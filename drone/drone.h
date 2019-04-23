/// @file drone.h
///
/// Abstraction for the entire drone and its components.
///
//  ****************************************************************************
#ifndef DRONE_H_INCLUDED
#define DRONE_H_INCLUDED

#include <string>
#include <atomic>
#include <thread>

#include "GPS.h"
#include "PWM.h"
#include "PID.h"
#include "qcrecv.h"

#include "utility/robotics.h"

const float k_epsilon = 1e-5;


extern float g_roll_scalar;
extern float g_pitch_scalar;
extern float g_yaw_scalar;


// TODO: Move these values to configurable fields.

const float k_roll_bias   = -0.0033;  
const float k_pitch_bias  = 0.0; 
const float k_yaw_bias    = -0.0038;    


// TODO: Utility Functions that can be moved to more common location.

//  ****************************************************************************
inline
float to_degrees(float radians)
{
  return radians * 180.0 / k_pi;
}

//  ****************************************************************************
inline
float to_radians(float degrees)
{
  return degrees * k_pi / 180.0;
}


//  ****************************************************************************
template <typename T>
T constrain(T value, T lower, T upper)
{
  T result = value;
  if (result < lower)
  {
    result = lower;
  }
  else if (result > upper)
  {
    result = upper;
  }

  return result;
}

//  ****************************************************************************
inline
float is_zero(float value)
{
  return ( value < k_epsilon
        && value > -k_epsilon);
}



//  ****************************************************************************
/// Provides the amount of thrust the specified arm provides for both roll and pitch.
///
struct Arm_thrust_ratio
{
  float roll;
  float pitch;
  float yaw;
};




//  ****************************************************************************
/// The single container through which all components of the drone are accessed.
///
class Drone
{
public:
  //  **************************************************************************
  static const 
    size_t k_max_motor_count = 8;

  static const
    unsigned int k_base_rate_ns = 1000000;          // 1000us

  static const
    unsigned int k_rate_range_ns= 1000000;          // 1000us

  static const
    unsigned int k_max_rate_ns  = k_base_rate_ns 
                                + k_rate_range_ns;  // 2000us

  static const
    unsigned int k_period       = 20000000;         // 20ms



  //  **************************************************************************
  Drone();
  ~Drone();

  //  **************************************************************************
  /// Interrupt handler for IMU events.
  ///
  static
    void IMU_interrupt_handler();

  //  **************************************************************************
  /// Initializes the components of the drone and resets its state.
  ///
  bool init();


  //  **************************************************************************
  /// Returns the current control mode for the drone.
  ///
  ControlMode control_mode() const
  {
    return m_control_mode;
  }

  //  **************************************************************************
  /// Sets the desired control mode-type for the drone.
  ///
  void control_mode(ControlMode mode)
  {
    m_control_mode = mode;
  }


  //  **************************************************************************
  /// Returns the current control state of the drone.
  ///
  DroneState state() const
  {
    return m_last_state;
  }

  //  **************************************************************************
  /// Configures control of the roll axis
  ///
  void use_roll_control(bool enable)
  {
    m_use_roll_control = enable;

    g_roll_scalar = enable ? 1.0 : 0.0;
  }

  //  **************************************************************************
  /// Indicates if control of the roll axis is enabled.
  ///
  bool use_roll_control() const
  {
    return m_use_roll_control;
  }

  //  **************************************************************************
  /// Configures control of the pitch axis
  ///
  void use_pitch_control(bool enable)
  {
    m_use_pitch_control = enable;

    g_pitch_scalar = enable ? 1.0 : 0.0;
  }

  //  **************************************************************************
  /// Indicates if control of the pitch axis is enabled.
  ///
  bool use_pitch_control() const
  {
    return m_use_pitch_control;
  }

  //  **************************************************************************
  /// Configures control of the yaw axis
  ///
  void use_yaw_control(bool enable)
  {
    m_use_yaw_control = enable;

    g_yaw_scalar = enable ? 1.0 : 0.0;
  }

  //  **************************************************************************
  /// Indicates if control of the yaw axis is enabled.
  ///
  bool use_yaw_control() const
  {
    return m_use_yaw_control;
  }

  //  **************************************************************************
  /// Reports the current roll value for the drone's orientation.
  ///
  float roll() const
  {
    return m_imu_update.fused_TaitBryan[TB_ROLL_Y];
  }

  //  **************************************************************************
  /// Reports the current pitch value for the drone's orientation.
  ///
  float pitch() const
  {
    return m_imu_update.fused_TaitBryan[TB_PITCH_X];  
  }

  //  **************************************************************************
  /// Reports the current yaw value for the drone's orientation.
  ///
  float yaw() const
  {
    return m_imu_update.fused_TaitBryan[TB_YAW_Z];
  }

  //  **************************************************************************
  /// Reports the current roll rate of change for the drone's orientation.
  /// Reported in radians / second.
  ///
  float roll_rate() const
  {
    return to_radians(raw_roll_rate()) - k_roll_bias;
  }

  //  **************************************************************************
  /// Reports the current pitch rate of change for the drone's orientation.
  /// Reported in radians / second.
  ///
  float pitch_rate() const
  {
    return to_radians(raw_pitch_rate()) - k_pitch_bias;
  }

  //  **************************************************************************
  /// Reports the current yaw rate of change for the drone's orientation.
  /// Reported in radians / second.
  ///
  float yaw_rate() const
  {
    return to_radians(raw_yaw_rate()) - k_yaw_bias;
  }

  //  **************************************************************************
  /// Reports the unfiltered roll rate of change for the drone's orientation.
  /// Reported in radians / second.
  ///
  float raw_roll_rate() const
  {
    // Includes measured bias.
    return m_imu_update.gyro[1];
  }

  //  **************************************************************************
  /// Reports the unfiltered pitch rate of change for the drone's orientation.
  /// Reported in radians / second.
  ///
  float raw_pitch_rate() const
  {
    // Includes measured bias.
    return m_imu_update.gyro[0];
  }

  //  **************************************************************************
  /// Reports the unfiltered yaw rate of change for the drone's orientation.
  /// Reported in radians / second.
  ///
  float raw_yaw_rate() const
  {
    // Includes measured bias.
    return m_imu_update.gyro[2];
  }

  //  **************************************************************************
  /// Updates the commanded settings for the drone.
  ///
  void command(const QCopter &cmd);

  //  **************************************************************************
  /// For the moment, the update event is externally driven.
  /// This function triggers a resample of the state of the system, 
  /// and iterates command of the PID.
  /// 
  void update();

  //  **************************************************************************
  /// Arms the motors for movement.
  ///
  void activate();

  //  **************************************************************************
  /// Immediately stops all motor activity.
  ///
  void halt();


  //  **************************************************************************
  /// Tests the motors by ramping from min to max and back to min over 20 seconds.
  ///
  void test_ramp_motors();

  //  **************************************************************************
  /// Reports if the system was shutdown due to a critical error.
  ///
  bool is_critical() const
  {
    return m_critical_angle;
  }

  //  **************************************************************************
  /// Command allows the Roll PID gain to be adjusted
  ///
  void adjust_gain_roll(float Kp, float Ki, float Kd);

  //  **************************************************************************
  /// Command allows the pitch PID gain to be adjusted
  ///
  void adjust_gain_pitch(float Kp, float Ki, float Kd);

  //  **************************************************************************
  /// Command allows the Roll rate PID gain to be adjusted
  ///
  void adjust_gain_roll_rate(float Kp, float Ki, float Kd);

  //  **************************************************************************
  /// Command allows the pitch rate PID gain to be adjusted
  ///
  void adjust_gain_pitch_rate(float Kp, float Ki, float Kd);

  //  **************************************************************************
  /// Command allows the rotation PID gain to be adjusted
  ///
  void adjust_gain_rotation(float Kp, float Ki, float Kd);

  //  **************************************************************************
  /// Returns the configured number of motors on the drone.
  ///
  size_t motor_count() const
  {
    return m_motor_count;
  }


  //  **************************************************************************
  /// Reports the distance of the drone from the base location.
  /// Calculations are performed using the haversine formula for great circles.
  ///
  double distance_from_base(const GPS::location_t &cur) const;

  //  **************************************************************************
  /// Reports the base location of takeoff.
  ///
  void base_location(Location &base) const
  {
    base.is_valid   = m_base_location.is_valid ? 1 : 0;
    base.latitude   = m_base_location.latitude;
    base.longitude  = m_base_location.longitude;
    base.altitude   = m_base_location.altitude;
    base.height     = 0;
  }

  //  **************************************************************************
  /// Reports the base location of takeoff.
  ///
  GPS::location_t current_location() const
  {
    return m_gps.location( );
  }



private:
  //  **************************************************************************
  PWM           m_motors[8];          ///< The motors that provide thrust for the
                                      ///  The multi-rotor copter.
                                      ///  We can support up to 8 motors
                                      ///  in 4,6, or 8 motor configuration.
  const Arm_thrust_ratio*
                mp_arm_thrust;        ///< Table with vectors that indicate
                                      ///  the ratio forces apply in both the
                                      ///  roll and pitch per arm.
  size_t        m_motor_count;        ///< The configured number of motors
                                      ///  on the drone.


  rc_mpu_data_t m_imu_data;           ///< Contains the latest data updated 
                                      ///  in the background by the IMU.     

  int           m_imu_index;          ///< The index for the active channel of
                                      ///  the IMU data's ping-pong buffer.
  rc_mpu_data_t m_imu_update;         ///< IMU data used by the update thread. 
                                         


  ControlMode   m_control_mode;       ///< The control mode-type used to control
                                      ///  the drone's motion.
  bool          m_use_roll_control;   ///< Indicates of control of the roll axis is enabled.
  bool          m_use_pitch_control;  ///< Indicates of control of the pitch axis is enabled.
  bool          m_use_yaw_control;    ///< Indicates of control of the yaw axis is enabled.

  PID           m_roll_stabilize;     ///  These PIDs control the stabilized
  PID           m_pitch_stabilize;    ///  orientation of the UAV.
                                                                                                    
  PID           m_roll_rate;          ///  These PIDs control the rate of change
  PID           m_pitch_rate;         ///  along each of the three control axis.
  PID           m_rotation;                                         

  bool          m_critical_angle;     ///< Indicates if a critical angle
                                      ///  was reached for the drone's orientation.

  // The most recent commands stored as normalized command values
  float         m_baro_altitude;      ///< Most recent altitude calculated from
                                      ///  the barometric pressure.
  float         m_baro_temperature;   ///< Most recent temperature reported
                                      ///  by the barometer.
  float         m_range_altitude;     ///< Ultra-sonic high-resolution altitude 
                                      ///  measurements near the ground.

  float         m_roll;               ///< normalized roll value
  float         m_pitch;              ///< normalized pitch value
  float         m_yaw;                ///< normalized yaw value
  float         m_throttle;           ///< normalized throttle value

  float         m_neutral_thrust;     ///< The thrust level required to remain
                                      ///  at a constant height.

  DroneState    m_last_state;         ///< The last set of command and control
                                      ///  values based on the most recent
                                      ///  update cycle to the drone's controls.
  DronePIDs     m_last_PIDS;          ///< The last set of status values recorded
                                      ///  for each of the drone's PIDs.


  GPS::location_t m_base_location;    ///< This is the starting location for
                                      ///  the drone. If a problem occurs 
                                      ///  during flight, the drone will attempt
                                      ///  to return to this location and land.

  GPS::Sensor   m_gps;                ///< GPS module instance.

  std::atomic_bool   m_IMU_ready;     ///< Flag indicates when the IMU interrupt has
                                      ///  been triggered, signaling new data.

  std::thread        m_update_thread;

  bool               m_is_exit;

  //  **************************************************************************
  //  Update processing thread for when the IMU has new data.
  //
  //  @p_this   Context pointer to this drone instance.
  //
  static
    void thread_proc(Drone *p_this);


  //  **************************************************************************
  //  Processes the current values and properly distributes the commands to 
  //  the motors that control the drone.
  //
  //  @param roll   The calculated roll to be commanded of the plant.
  //  @param pitch  The calculated pitch to be commanded of the plant.
  //  @param yaw    The calculated yaw to be commanded of the plant.
  //
  //  @return   true  if any of the motors are saturated.
  //            false if all of the plant's motors are commanded within range.
  // 
  bool process_plant(float roll, float pitch, float yaw);

  //  **************************************************************************
  //  Resets the motors to the initial activated state.
  //
  void reset_motors();

  //  **************************************************************************
  //  Commands the fan motor to the appropriate level from 0.0 to 1.0.
  //
  void set_motor_level(PWM &motor, float level);

  //  **************************************************************************
  float get_motor_level(PWM &motor) const;

  //  **************************************************************************
  void clear_motor_levels();

  //  **************************************************************************
  void read_battery_levels(DroneState &last_state);


};


#endif

