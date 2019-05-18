/// @file drone.cpp
///
/// Abstraction for the entire drone and its components.
///
 
//  TODO: Add thrust compensation for the tilt of the frame.
//  TODO: Add CRCs to the communication protocol.
//  TODO: Send a MAVLink Heartbeat message to the radio to get the radios reported RSSI with the ground station.
//        Report this value in the Drone State message.
//  TODO: Incorporate the vertical range finder to assist with landing
//  TODO: Incorporate the barometer to help assess the current height
//        in the absense of GPS.
//  TODO: Calculate the thrust level for hover. Incorporate the barometer
//        and GPS altimeter to regulate this value dynamically at flight.
//  TODO: Enable the anti-windup logic.
//  TODO: Move the logging logic to a separate source file, 
//        and hopefully write to the file in a different thread.
//  TODO: Move the configuration settings into a file that is not compiled into the program.
//  TODO: Add test-mode settings that allow disabling and enabling specific controllers.

//  ****************************************************************************

#include "drone.h"
#include "utility/util.h"

#include <cstdlib>

#include <array>
#include <iosfwd>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <cerrno>

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>


//  Forward declarations *******************************************************
// TODO: Clean up the integration of the range sensor information.
void  start_range_sensor();
void  shutdown_range_sensor();
float get_range();



using std::ifstream;
using std::ofstream;
using std::cout;
using std::cin;
using std::endl;


namespace // unnamed
{

Drone *p_drone_instance = nullptr;

}


// TODO: Incorporate this into a logging class, 
//       which is composited into the other classes.
ofstream g_outfile;

// TODO: Need to move these values into configurable settings loaded from a file.
float g_hover_level             = 0.1f;

float g_roll_Kp                 = 1.25; 
float g_roll_Ki                 = 0.325;
float g_roll_Kd                 = 0.077; 
float g_roll_scalar             = 1.0;
float g_roll_windup_limit       = to_radians(10);

float g_pitch_Kp                = 1.08;
float g_pitch_Ki                = 0.65;
float g_pitch_Kd                = 0.1625;
float g_pitch_scalar            = 1.0;
float g_pitch_windup_limit      = to_radians(10);

float g_roll_rate_Kp            = 0.9678; 
float g_roll_rate_Ki            = 1.526;  
float g_roll_rate_Kd            = 0.02405;
float g_roll_rate_windup_limit  = to_radians(20);
float g_roll_rate_cutoff        = 41.0;
float g_roll_rate_scalar        = 1.0;

float g_pitch_rate_Kp           = 0.375;
float g_pitch_rate_Ki           = 1.545;
float g_pitch_rate_Kd           = 0.0225;
float g_pitch_rate_windup_limit = to_radians(20);
float g_pitch_rate_cutoff       = 41.0;
float g_pitch_rate_scalar       = 1.0;

float g_yaw_Kp                  = 0.825;
float g_yaw_Ki                  = 0.5;
float g_yaw_Kd                  = 0.0035;
float g_yaw_windup_limit        = to_radians(20);
float g_yaw_cutoff              = 41.0;
float g_yaw_scalar              = 1.0;


float g_motor_constraint_min    = 0.0f;
float g_motor_constraint_max    = 1.0f;


// Constants *******************************************************************
const
  int   k_i2c_bus             = 2;

const
  float k_dT                  = 0.005f;               ///< 200 Hz, size of time slice.

const
  float k_rp_command_limit    = k_pi_6;               // 30° in radians;

const
  float k_yaw_command_limit   = k_pi_3;               // 60° in radians;

const 
  float k_yaw_comp_limit      = 1.0f;

const
  float k_critical_limit      = k_pi_3;               // 60° in radians;

const
  float k_thrust_error_angle  = k_pi_6;               // 30° in radians;


const
  float k_throttle_limit_max  = 0.8f;                 ///< Throttle can account for up to
                                                      ///  80% of the command signal. 
                                                      ///  Reserve the remaining 20% for
                                                      ///  attitude control.

const
  float k_acceleration_max    = to_radians(720.0f);   ///< Maximum angular acceleration
                                                      ///  deg/sec^2  ->  radians/sec^2


const 
  float k_ratio_limit         = 0.5f;


//  ****************************************************************************
// Calculate the raw levels for each of the output levels.
// Where the caret is the forward face of the drone,
// the orientation of the motors are as follows:
//
// 4-motor configuration
//
//    A ^ B
//    D   C
//
// The rules break-down grouping each motor into
// one of two sets, positive or negative. For each
// command, a balanced number of motors should be assigned
// to each set:
//
//          +     -
// Roll:  (A,D) (B,C)
// Pitch: (A,B) (C,D)
// Yaw:   (A,C) (B,D)
//
const 
  Arm_thrust_ratio  k_quad_arms[4] = 
{ 
  { cosf(k_pi_4), sinf(k_pi_4),  k_ratio_limit },               // Arm A - 45°
  { cosf(k_pi_4), sinf(k_pi_4), -k_ratio_limit },               // Arm B - 315°
  { cosf(k_pi_4), sinf(k_pi_4),  k_ratio_limit },               // Arm C - 225°
  { cosf(k_pi_4), sinf(k_pi_4), -k_ratio_limit }                // Arm D - 135°
};

//  ****************************************************************************
// 6-motor configuration
//
//           0°
//        F    A
// 90°  E   ^^   B  270°
//        D    C
//         180°
//
//          +     -
// Roll:  (D,E,F) (A,B,C)
// Pitch: (A,F)   (C,D)
// Yaw:   (B,D,F) (A,C,E)
//
// The roll calculations are rotated by 90° and cos is used,
// otherwise there is a sign shift that is required with sin calculations
// for the aft motors of the drone.
//
// Additionally, the maximum amount of thrust for all control axes are limited.
// 
const 
  Arm_thrust_ratio  k_hex_arms[6] = 
{
  { cosf(to_radians(-30.0f  - 90))  * k_ratio_limit, 
    cosf(to_radians(-30.0f))        * k_ratio_limit,
    -k_yaw_comp_limit * k_ratio_limit},                    // Arm A: -30°
  { cosf(to_radians(-90.0f  - 90))  * k_ratio_limit, 
    cosf(to_radians(-90.0f))        * k_ratio_limit,
    k_yaw_comp_limit * k_ratio_limit},                     // Arm B: -90°
  { cosf(to_radians(-150.0f - 90))  * k_ratio_limit, 
    cosf(to_radians(-150.0f))       * k_ratio_limit,
    -k_yaw_comp_limit * k_ratio_limit},                    // Arm C: -150°
  { cosf(to_radians(150.0f  - 90))  * k_ratio_limit, 
    cosf(to_radians( 150.0f))       * k_ratio_limit,
    k_yaw_comp_limit * k_ratio_limit},                     // Arm D: 150°
  { cosf(to_radians(90.0f   - 90))  * k_ratio_limit, 
    cosf(to_radians( 90.0f))        * k_ratio_limit,
    -k_yaw_comp_limit * k_ratio_limit},                    // Arm E: 90°
  { cosf(to_radians(30.0f   - 90))  * k_ratio_limit, 
    cosf(to_radians( 30.0f))        * k_ratio_limit,
    k_yaw_comp_limit * k_ratio_limit}                      // Arm F: 30°
};



//  ****************************************************************************
inline
float normalize_throttle(int16_t thrust)
{
  float throttle = to_normalized(thrust);


  // Throttle is centered at the neutral hover-point.
  if (throttle >= 0.0f)
  {
    // Positive values increase from the hover level to the maximum.
    // Scale this normalized throttle to be within the
    // allowed range.

// TODO: Switch to this code before flight.
    //float range = k_throttle_limit_max - g_hover_level;
    //throttle    = g_hover_level + (throttle * k_throttle_limit_max);  
    throttle *= k_throttle_limit_max;  
  }
  else
  {
    // Negative values reduce the thrust from hover down to a minimum level.
    throttle = (1.0 + throttle) * g_hover_level;
  }

  return throttle;
}

//  ****************************************************************************
inline
float normalize_roll_angle(float roll)
{
  return (roll) / k_half_pi;
}

//  ****************************************************************************
inline 
float normalize_pitch_angle(float pitch)
{
  return (pitch) / k_pi;
}

//  ****************************************************************************
//  Normalize to be between -1 (-180) and 1 (180)
//
inline
float normalize_yaw_angle(float yaw)
{
  float normalized = yaw / k_pi;

  return  normalized > 1.0
          ? normalized - 2.0
          : normalized;
}

//  ****************************************************************************
inline 
float normalize_latitude(float value)
{
  return (value) / 90.0f;
}

//  ****************************************************************************
inline 
float normalize_longitude(float value)
{
  return (value) / 180.0f;
}

//  ****************************************************************************
inline 
float normalize_altitude(float value)
{
  return (value) / 10000.0f;
}


//  ****************************************************************************
float YawOrientation(int16_t value)
{
  float result  = value < 0
                ? value / (float)std::numeric_limits<int16_t>::min() * -1.0f
                : value / (float)std::numeric_limits<int16_t>::max();

  if (result < 0.0)
  {
    result += 2.0;
  }

  return result * k_pi;
}



//  ****************************************************************************
/// Limits the growth of the velocity by a maximum acceleration.
///
float accelerate_angular_vel(float target, float velocity, float accel_max)
{
  // TODO: Revisit
  float delta_ang_vel = accel_max * k_dT * 10;
  velocity += constrain(target - velocity, -delta_ang_vel, delta_ang_vel);

  return velocity;
}


//  ****************************************************************************
PIDState to_PIDState(const PID& pid)
{
  PIDState state;

  state.set_point       = to_int64(pid.target());
  state.delta_time      = to_int64(pid.dt());
  state.current_error   = to_int64(pid.error());
  state.delta_error     = to_int64(pid.dError());
  state.integral_error  = to_int64(pid.integrator());
  state.windup_limit    = to_int64(pid.windup_limit());

  state.desc.Kp         = encode_PID(pid.Kp());
  state.desc.Ki         = encode_PID(pid.Ki());
  state.desc.Kd         = encode_PID(pid.Kd());

  state.desc.range_min  = to_int64(pid.min());
  state.desc.range_max  = to_int64(pid.max());

  return state;
}


//  ****************************************************************************
void Drone::IMU_interrupt_handler()
{
  if (!p_drone_instance)
  {
    return;
  }

//  int index = (p_drone_instance->m_imu_index + 1) % 2;

  // Update the IMU data, and signal the update thread.
  p_drone_instance->m_imu_update = p_drone_instance->m_imu_data;

  p_drone_instance->update( );

  //// Wait while the update thread is processing logic.
  //while (true == p_drone_instance->m_IMU_ready);

  //// Update the 
  //p_drone_instance->m_imu_index = index;

  //p_drone_instance->m_IMU_ready = true;
}


//  ****************************************************************************
Drone::Drone()
  : m_motors{1,2,3,4,5,6,7,8}
  , mp_arm_thrust(nullptr)
  , m_motor_count(4)
  , m_imu_data{0}
  , m_imu_index(0)
  , m_imu_update{0}
  , m_control_mode(angle_control)
  , m_critical_angle(false)
  , m_roll(0.0f)
  , m_pitch(0.0f)
  , m_yaw(0.0f)
  , m_throttle(0.0f)
  , m_last_state{0}
  , m_last_PIDS{0}
  , m_base_location{0}
  , m_IMU_ready(false)
  , m_is_exit(false)
{ 
  // Associate this drone object with the interrupt routines.
  p_drone_instance = this;

  // TODO: Add code to dynamically load the motor count and initialize the thrust table.
  m_motor_count = 6;
  mp_arm_thrust = k_hex_arms;
  //control_mode(rate_control);

}

//  ****************************************************************************
Drone::~Drone()
{
  m_gps.term( );

  rc_mpu_power_off();
  rc_adc_cleanup();
  rc_bmp_power_off();
  rc_servo_cleanup();

  // TODO: REenable for range calculations
  //shutdown_range_sensor();

  p_drone_instance = nullptr;
}


//  ****************************************************************************
bool Drone::init()
{
  // TODO: Add code to read coefficients from a configuration file.
  // Set the command limits for each of the PID controllers.
  adjust_gain_roll        ( g_roll_Kp,       g_roll_Ki,        g_roll_Kd);
  m_roll_stabilize.min    (-k_rp_command_limit);
  m_roll_stabilize.max    ( k_rp_command_limit);
  m_roll_stabilize.scalar ( g_roll_scalar);
  m_roll_stabilize.windup_limit(g_roll_windup_limit);

  adjust_gain_pitch       ( g_pitch_Kp,      g_pitch_Ki,       g_pitch_Kd);
  m_pitch_stabilize.min   (-k_rp_command_limit);
  m_pitch_stabilize.max   ( k_rp_command_limit);
  m_pitch_stabilize.scalar( g_pitch_scalar);
  m_pitch_stabilize.windup_limit(g_pitch_windup_limit);

  adjust_gain_roll_rate   ( g_roll_rate_Kp,  g_roll_rate_Ki,   g_roll_rate_Kd);
  m_roll_rate.min         (-k_critical_limit);
  m_roll_rate.max         ( k_critical_limit);
  m_roll_rate.scalar      ( g_roll_rate_scalar);
  m_roll_rate.lowpass_freq( g_roll_rate_cutoff);
  m_roll_rate.windup_limit( g_roll_rate_windup_limit);

  adjust_gain_pitch_rate  (g_pitch_rate_Kp, g_pitch_rate_Ki,   g_pitch_rate_Kd);
  m_pitch_rate.min        (-k_critical_limit);
  m_pitch_rate.max        ( k_critical_limit);
  m_pitch_rate.scalar     ( g_pitch_rate_scalar);
  m_pitch_rate.lowpass_freq(g_pitch_rate_cutoff);
  m_pitch_rate.windup_limit(g_pitch_rate_windup_limit);

  adjust_gain_rotation    ( g_yaw_Kp,        g_yaw_Ki,         g_yaw_Kd);
  m_rotation.min          (-k_yaw_command_limit);
  m_rotation.max          ( k_yaw_command_limit);
  m_rotation.scalar       ( g_yaw_scalar);
  m_rotation.lowpass_freq ( g_yaw_cutoff);
  m_rotation.windup_limit ( g_yaw_windup_limit);

  // Initialize the servo motor and power levels.
  if (0 != rc_servo_init( ))
  {
    cout  << "Call to initialize servo (motors) failed." << endl;
  }

  //if (0 != rc_servo_set_esc_range(1000, 2000))
  //{
  //  cout  << "Call to initialize the ESC range failed." << endl;
  //}

  // Disable the power rail for the servo signals.
  rc_servo_power_rail_en(0);


  clear_motor_levels();

  g_outfile.open("./data.csv", std::ios_base::app);
  if (!g_outfile.is_open())
  {
    cout  << "Could not open file to log behavior." << endl;
  }

  // TODO: Need to figure out what needs to be done to enable the PRU range sensor code.
  // Start the ultrasonic range sensor for high-resolution
  // distance measurements near the ground.
  //start_range_sensor();
  

  // Initialize battery read support.
  rc_adc_init( );

  // Initialize the IMU to trigger our handler with the interrupt handler.
  rc_mpu_config_t conf = rc_mpu_default_config();

  // We want the magnetometer data fused with the calculations
  // to help us determine absolute orientation.

  conf.i2c_bus                    = k_i2c_bus;
  conf.gpio_interrupt_pin_chip    = 3;
  conf.gpio_interrupt_pin         = 21;
  conf.enable_magnetometer        = 1;
  conf.dmp_sample_rate            = 200; // Hz
  conf.dmp_interrupt_priority     = 50;
  conf.dmp_interrupt_sched_policy = SCHED_FIFO;
  conf.orient                     = ORIENTATION_X_BACK;

  conf.dmp_fetch_accel_gyro       = 1;
  conf.show_warnings              = 1;

  int status = rc_mpu_initialize_dmp(&m_imu_data, 
                                     conf);
  if (0 != status)
  {
    cout  << "Error: " << status << "\n"
          << "IMU initialization failed in: rc_initialize_imu_dmp()\n";
    return false;
  }

  rc_mpu_set_dmp_callback(&IMU_interrupt_handler);

  // TODO: Have issues if we attempt to enable the barometer at the same time as the IMU.
  //rc_bmp_init(BMP_OVERSAMPLE_1, 
  //            BMP_FILTER_16);

  m_critical_angle = false;

  m_gps.init( );

  // Waiting for an ARM command from the GCS.
  halt( );

  //m_is_exit = false;
  //m_update_thread = std::thread(thread_proc, this);

  //return m_update_thread.joinable();
  return true;
}

//  ****************************************************************************
void Drone::activate()
{
  // Reset the commanded levels.
  m_roll      = 0.0f;
  m_pitch     = 0.0f;
  m_yaw       = 0.0f;
  m_throttle  = 0.0f;


  m_roll_stabilize.clear();
  m_pitch_stabilize.clear();
  m_roll_rate.clear();
  m_pitch_rate.clear();
  m_rotation.clear();

  clear_motor_levels();

  for (size_t index = 0; index < k_max_motor_count; ++index)
  {
    m_motors[index].arm(); 
  }

  // Record the starting location before take-off.
  m_base_location = m_gps.location( );
  if (m_base_location.is_valid)
  {
    cout  << "The base location is:\n"
          << "  Latitude:  " << m_base_location.latitude
          << "  Longitude: " << m_base_location.longitude
          << "  Altitude:  " << m_base_location.altitude << endl;
  }
  else
  {
    cout  << "Warning!!!\n"
          << "A valid base location has not been recorded for the drone.\n"
          << "Current recorded values are: \n"
          << "  Latitude:  " << m_base_location.latitude
          << "  Longitude: " << m_base_location.longitude
          << "  Altitude:  " << m_base_location.altitude << endl;
  }

  m_last_state.is_armed = 1;
  cout << "The drone is now armed!" << endl;
}

//  ****************************************************************************
void Drone::halt()
{
  for (size_t index = 0; index < k_max_motor_count; ++index)
  {
    m_motors[index].unarm(); 
  }

  m_last_state.is_armed = 0;
  cout << "The drone is disarmed..." << endl;


  GPS::location_t loc = m_gps.location( );
  if (loc.is_valid)
  {
    cout  << "Warning!!!\n"
          << "A GPS location is not valid.\n";
  }

  cout  << "The most recent location is:\n"
        << "  Latitude:  " << loc.latitude
        << "  Longitude: " << loc.longitude
        << "  Altitude:  " << loc.altitude << endl;
  

  m_is_exit = true;

  if (m_update_thread.joinable())
  {
    m_update_thread.join();
  }
}

//  ****************************************************************************
void Drone::command(const QCopter &cmd)
{
  // Normalize each commanded value and configure
  // the setpoint for each PID.
  // The PIDs enforce the allowed range for command values.
  m_roll  = to_normalized(cmd.roll)  * k_rp_command_limit;
  m_pitch = to_normalized(cmd.pitch) * k_rp_command_limit;

  // Prevent the overall commanded angle from exceeding the critical angle.
  float combined_angle = euclid_distance(m_roll, 
                                         m_pitch);
  if (combined_angle > k_rp_command_limit)
  {
    float ratio = k_rp_command_limit / combined_angle;
    m_roll   *= ratio;
    m_pitch  *= ratio;  
  }

  m_roll_stabilize.setpoint  (m_roll);
  m_pitch_stabilize.setpoint (m_pitch);

  m_yaw = to_normalized(cmd.yaw) * k_yaw_command_limit;
  m_rotation.setpoint(m_yaw);


  m_throttle = normalize_throttle(cmd.thrust);
}


//  ****************************************************************************
void Drone::thread_proc(Drone *p_this)
{
  if (!p_this)
    return;

  cout << "Starting Drone Update Thread." << endl;

  while (!p_this->m_is_exit)
  {
    if (p_this->m_IMU_ready)
    {
      p_this->update();
      p_this->m_IMU_ready = false;
    }
  }

  cout << "Terminating Drone Update Thread." << endl;

}


//  ****************************************************************************
void Drone::update( )
{
  GPS::location_t cur = current_location( );

  m_last_state.position.is_valid  = cur.is_valid;

  m_last_state.position.latitude  = to_int32(normalize_latitude(cur.latitude));
  m_last_state.position.longitude = to_int32(normalize_longitude(cur.longitude));
  m_last_state.position.altitude  = to_int32(normalize_altitude(cur.altitude));
  m_last_state.position.height    = 0;


  // TODO: Address when the drone is on the ground, do not let the PID integrals wind-up.
  //       For now, do not update with zero thrust.
  if (m_throttle == 0.0f)
  {
    clear_motor_levels();
    return;
  }
  else if (m_throttle < g_hover_level)
  {
    // This is a make shift adjustment until other components 
    // are tuned to keep the integral from winding up.
    m_roll_stabilize.reset_integral();
    m_pitch_stabilize.reset_integral();
  }


  uint64_t timestamp  = timestamp_ms( );

  // TODO: Enable the barometer once the issue with simultaneous usage is solved for the IMU.
  //rc_bmp_data_t bmp_data;
  //rc_bmp_read(&bmp_data);
  //
  //m_baro_altitude     = bmp_data.alt_m;
  //m_baro_temperature  = bmp_data.temp_c;

  // TODO: Enable the range sensor logic once the PRU is solved.
  //m_range_altitude    = get_range();

  //static int cycles = 0;
  //cycles++;
  //if (!(cycles % 50))
  //{
  //  cout << " Range: " << m_range_altitude << endl;
  //}

  double distance = distance_from_base(cur);
  if (distance > 20.0)
  {
    // Perform an emergency action to prevent the drone from drifting away.
    // We force the throttle down to 25%.
    cout << "ALERT!!! The drone has moved outside of the test area (" << distance << ")" << endl;

    m_throttle = 0.5 * g_hover_level;
  }

  // Safety check the stability of the drone
  m_critical_angle = (roll( )  >  k_critical_limit
                   || roll( )  < -k_critical_limit
                   || pitch( ) >  k_critical_limit
                   || pitch( ) < -k_critical_limit);

    // Update the stabilization PID controllers. *********************
  float roll_error      = 0.0f;
  float pitch_error     = 0.0f;
  if (angle_control == control_mode( ))
  {
    roll_error          = m_roll_stabilize.update (roll( ), timestamp);
    pitch_error         = m_pitch_stabilize.update(pitch( ), timestamp);
  }
  else // expecting rate control
  {
    // Simply use the target setpoint for each axis
    roll_error          = m_roll_stabilize.target( );
    pitch_error         = m_pitch_stabilize.target( );
  }

  // Update the rate PID controllers. ******************************

  // The outputs from the stabilization control PIDs,
  // become the new set-points for the rate control PIDs.
  m_roll_rate.setpoint (accelerate_angular_vel(roll_error, roll_rate( ), k_acceleration_max));
  m_pitch_rate.setpoint(accelerate_angular_vel(pitch_error, pitch_rate( ), k_acceleration_max));

  float roll_output       = m_roll_rate.update (roll_rate( ), timestamp);
  float pitch_output      = m_pitch_rate.update(pitch_rate( ), timestamp);
  float yaw_output        = m_rotation.update  (yaw_rate( ), timestamp);

  roll_output   = constrain(roll_output, -k_critical_limit, k_critical_limit);
  pitch_output  = constrain(pitch_output, -k_critical_limit, k_critical_limit);

  // Log statistics.
  g_outfile << m_roll_stabilize.target()  << ","    // 1
            << roll()                     << ","    // 2
            << roll_error                 << ","    // 3
            << m_roll_rate.target()       << ","    // 4
            << roll_rate()                << ","    // 5
            << roll_output                << ","    // 6
            << m_pitch_stabilize.target() << ","    // 7
            << pitch()                    << ","    // 8
            << pitch_error                << ","    // 9
            << m_pitch_rate.target()      << ","    // 10
            << pitch_rate()               << ","    // 11
            << pitch_output               << ","    // 12
            << m_rotation.target( )       << ","    // 13
            << yaw_rate( )                << ","    // 14
            << yaw_output                 << ","    // 15
            << yaw( )                     << "\n";  // 16
   
  // Record the orientation.
  m_last_state.orientation.roll_rate  = to_int16(normalize_roll_angle(roll_rate( )));
  m_last_state.orientation.roll       = to_int16(normalize_roll_angle(roll( )));
  m_last_state.orientation.pitch_rate = to_int16(normalize_pitch_angle(pitch_rate( )));
  m_last_state.orientation.pitch      = to_int16(normalize_pitch_angle(pitch( )));
  m_last_state.orientation.yaw        = to_int16(normalize_yaw_angle(yaw( )));


  if (!is_critical( )
      && m_throttle > 0.0f)
  {
    // Convert the modified command vectors into
    // actual commands directed to the motors.
    //
    // This data is equivalent to u(t) in the PID graph.
    //
    process_plant(roll_output,
                  pitch_output,
                  yaw_output);
    rc_led_set(RC_LED_RED, 0);
  }
  else
  {
    // Turn motors off if past the critical angle.
    clear_motor_levels( );

    // Reset the state of the PIDs.
    m_roll_stabilize.clear( );
    m_pitch_stabilize.clear( );

    m_roll_rate.clear( );
    m_pitch_rate.clear( );
    m_rotation.clear( );

    // Indicates beyond the critical angle.
    rc_led_set(RC_LED_RED, 1);
  }

  // Record PID states.
  m_last_PIDS.roll_rate  = to_PIDState(m_roll_rate);
  m_last_PIDS.roll       = to_PIDState(m_roll_stabilize);
  m_last_PIDS.pitch_rate = to_PIDState(m_pitch_rate);
  m_last_PIDS.pitch      = to_PIDState(m_pitch_stabilize);
  m_last_PIDS.rotation   = to_PIDState(m_rotation);

  // Update the drone's recorded state for the motors.
  m_last_state.motor.A = to_uint16(get_motor_level(m_motors[0]));
  m_last_state.motor.B = to_uint16(get_motor_level(m_motors[1]));
  m_last_state.motor.C = to_uint16(get_motor_level(m_motors[2]));
  m_last_state.motor.D = to_uint16(get_motor_level(m_motors[3]));
  m_last_state.motor.E = to_uint16(get_motor_level(m_motors[4]));
  m_last_state.motor.F = to_uint16(get_motor_level(m_motors[5]));
  m_last_state.motor.G = to_uint16(get_motor_level(m_motors[6]));
  m_last_state.motor.H = to_uint16(get_motor_level(m_motors[7]));

}
  

//  ****************************************************************************
bool Drone::process_plant(float roll, float pitch, float yaw)
{
  float attitude[k_max_motor_count] = {0.0f};
  float rate[k_max_motor_count]     = {0.0f};

  // These coefficients account for the offset angle 
  // of the motor placement on the body frame.
  for (size_t index = 0; index < motor_count(); ++index)
  {
    attitude[index] = mp_arm_thrust[index].pitch * pitch 
                    + mp_arm_thrust[index].roll  * roll
                    + mp_arm_thrust[index].yaw   * yaw;

    rate[index]     = m_throttle + attitude[index];
  }


  // Normalize the thrust level to between 0 and 1.
  float lowest_rate  = *std::min_element(&rate[0], &rate[0] + motor_count());
  float highest_rate = *std::max_element(&rate[0], &rate[0] + motor_count());

  // Offset by the inverse to raise to zero.
  float offset  =  lowest_rate < 0.0
                ? -lowest_rate        
                : 0.0;

  highest_rate += offset;
  float ratio   = highest_rate > 1.0
                ? 1.0 / highest_rate
                : 1.0;

  // Assign the normalized rate to each corresponding motor.
  for (size_t index = 0; index < motor_count(); ++index)
  {
    float motor_rate  = (rate[index] + offset) * ratio;

    set_motor_level(m_motors[index], motor_rate);
  }

  return false;
}

//  ****************************************************************************
void Drone::reset_motors()
{
  halt();

  // Initialize each of the PWMs.
  for (size_t index = 0; index < k_max_motor_count; ++index)
  {
    m_motors[index].period(k_period);
    m_motors[index].polarity(PWM::k_active_high);
  }

  clear_motor_levels();

  // Allow the motors to spin.
  activate();
}

//  ****************************************************************************
void Drone::set_motor_level(PWM &motor, float level)
{
  float cur_rate = constrain(level, 
                             g_motor_constraint_min, 
                             g_motor_constraint_max);

  motor.duty_cycle_percent(cur_rate);
}

//  ****************************************************************************
float Drone::get_motor_level(PWM &motor) const
{
  return motor.duty_cycle_percent();
}

//  **************************************************************************
void Drone::clear_motor_levels()
{
  for (size_t index = 0; index < k_max_motor_count; ++index)
  {
    m_motors[index].duty_cycle_percent(0);
  }
}


//  **************************************************************************
void Drone::adjust_gain_roll(float Kp, float Ki, float Kd)
{
  cout << "Adjust Gain - Roll Stabilize.\n" << endl;

  // Update the gain parameters and reset to remove
  // any accumulated integral error.
  m_roll_stabilize.Kp(Kp);
  m_roll_stabilize.Ki(Ki);
  m_roll_stabilize.Kd(Kd);
}

//  **************************************************************************
void Drone::adjust_gain_pitch(float Kp, float Ki, float Kd)
{
  cout << "Adjust Gain - Pitch Stabilize.\n" << endl;

  // Update the gain parameters and reset to remove
  // any accumulated integral error.
  m_pitch_stabilize.Kp(Kp);
  m_pitch_stabilize.Ki(Ki);
  m_pitch_stabilize.Kd(Kd);
}

//  **************************************************************************
void Drone::adjust_gain_roll_rate(float Kp, float Ki, float Kd)
{
  cout << "Adjust Gain - Roll Rate.\n" << endl;

  // Update the gain parameters and reset to remove
  // any accumulated integral error.
  m_roll_rate.Kp(Kp);
  m_roll_rate.Ki(Ki);
  m_roll_rate.Kd(Kd);
}

//  **************************************************************************
void Drone::adjust_gain_pitch_rate(float Kp, float Ki, float Kd)
{
  cout << "Adjust Gain - Pitch Rate.\n" << endl;

  // Update the gain parameters and reset to remove
  // any accumulated integral error.
  m_pitch_rate.Kp(Kp);
  m_pitch_rate.Ki(Ki);
  m_pitch_rate.Kd(Kd);
}

//  **************************************************************************
void Drone::adjust_gain_rotation(float Kp, float Ki, float Kd)
{
  cout << "Adjust Gain - Rotate.\n" << endl;

  // Update the gain parameters and reset to remove
  // any accumulated integral error.
  m_rotation.Kp(Kp);
  m_rotation.Ki(Ki);
  m_rotation.Kd(Kd);
}


//  **************************************************************************
void Drone::read_battery_levels(DroneState &last_state)
{
  last_state.batteries.count = 1;

  Battery &computer   = last_state.batteries.battery[0];
  computer.cell_count = 2;

  last_state.batteries.count = 1;

  // Read the computer's battery level.
  float board = rc_adc_batt();

  
  computer.cell_level[0] = int(board * 2048) / 2;
  computer.cell_level[1] = int(board * 2048) / 2;

  // TODO: Ready the motor's battery level
  //Battery &motors   = last_state.batteries.battery[1];
  //motors.cell_count = 4;

}


//  **************************************************************************
double Drone::distance_from_base(const GPS::location_t &cur) const
{
  // TODO: This calculation does not currently account for altitude.

  // Verify both distances are valid.
  // Report 0.0 distance if not.
  if ( !cur.is_valid
    || !m_base_location.is_valid)
  { 
    return 0.0;
  }

  const double k_R  = 6379.137;    // Radius of the earth in km.

  double lat_1  = m_base_location.latitude * k_pi / 180.0;
  double lat_2  = cur.latitude * k_pi / 180.0;

  double long_1 = m_base_location.longitude * k_pi / 180.0;
  double long_2 = cur.longitude * k_pi / 180.0;

  double lat_diff   = lat_2 - lat_1;
  double long_diff  = long_2 - long_1;

  double sin_lat_sqr  = (sin(lat_diff/2)  * sin(lat_diff/2));
  double sin_long_sqr = (sin(long_diff/2) * sin(long_diff/2));

  double A = sin_lat_sqr + (cos(lat_1) * cos(lat_2) * sin_long_sqr);
  double C = 2 * atan2(sqrt(A), sqrt(1.0 - A));
  double D = k_R * C;

  // Convert distance to meters.
  return D * 1000.0;
}

