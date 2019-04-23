/// @file qcctrl.cpp
/// 
/// Captures controller input for transmission to the drone.
///
//  ****************************************************************************

#include "../stdafx.h"
#include "qcctrl.h"
#include "../drone/utility/util.h"

#include <thread>
#include <chrono>

ControlSignal   g_control_signal = k_user_input_signal;

float           g_control_signal_period    = 10.0f;
float           g_control_signal_scalar    = 0.5f;

namespace  // unnamed 
{

const 
  int16_t     k_command_percent = std::numeric_limits<int16_t>::max() / 100;

uint32_t      g_drone_address   = 0;
uint32_t      g_drone_port      = 8100;


bool          g_is_transmitting = false;
bool          g_is_listening    = false;
bool          g_is_connected    = false;
bool          g_is_armed        = false;


QCopter       g_commanded       = {0};
int16_t       g_baseline_thrust = {0};
PIDDesc       g_PID_roll        = {0};
PIDDesc       g_PID_roll_rate   = {0};
PIDDesc       g_PID_pitch       = {0};
PIDDesc       g_PID_pitch_rate  = {0};
PIDDesc       g_PID_rotation    = {0};

bool          g_has_gain_adjustment   = false;
bool          g_has_roll_adjustment   = false;
bool          g_has_roll_rate_adjustment   = false;
bool          g_has_pitch_adjustment  = false;
bool          g_has_pitch_rate_adjustment  = false;
bool          g_has_rotate_adjustment = false;

bool          g_has_control_mode_change = false;
ControlMode   g_stage_control_mode      = angle_control;
bool          g_stage_disable_roll          = true;
bool          g_stage_disable_pitch         = true;
bool          g_stage_disable_yaw           = true;

bool          g_stage_use_roll_signal       = true;
bool          g_stage_use_pitch_signal      = true;
bool          g_stage_use_yaw_signal        = true;

DroneState    g_drone_state     = {0};
DronePIDs     g_PID_state       = {0};
Location      g_base_location   = {0};


uint16_t      g_next_sequence   = 0;

uint16_t      g_last_recv_seq   = 0;

std::thread*  gp_transmitter    = nullptr;
std::thread*  gp_receiver       = nullptr;

uint32_t      g_connect_cookie  = 0x600DC0DE;
uint32_t      g_beacon_cookie   = 0;

HANDLE        g_hCom            = INVALID_HANDLE_VALUE;


}

//  ****************************************************************************
HANDLE OpenConnection(HWND hWnd)
{
   HANDLE hCom;
   TCHAR *pcCommPort = TEXT("COM3"); 

   //  Open a handle to the specified com port.
   hCom = CreateFile( pcCommPort,
                      GENERIC_READ | GENERIC_WRITE,
                      0,              //  must be opened with exclusive-access
                      NULL,           //  default security attributes
                      OPEN_EXISTING,  //  must use OPEN_EXISTING
                      0,              //  not overlapped I/O
                      NULL );         

   if (hCom == INVALID_HANDLE_VALUE) 
   {
     MessageBox(hWnd, 
                _T("Failed to open serial port."), 
                _T("Error"), 
                MB_ICONEXCLAMATION | MB_OK);
     return INVALID_HANDLE_VALUE;
   }

   DCB dcb = {0};
   dcb.DCBlength = sizeof(DCB);

   GetCommState(hCom, &dcb);

   dcb.BaudRate = CBR_57600;
   dcb.ByteSize = 8;
   dcb.Parity   = NOPARITY;
   dcb.StopBits = ONESTOPBIT;

   SetCommState(hCom, &dcb);

   //PurgeComm(hCom, PURGE_TXCLEAR | PURGE_RXCLEAR);

   return hCom;
}


//  ****************************************************************************
void CloseConnection(HANDLE hCom)
{
  if (INVALID_HANDLE_VALUE != g_hCom)
  {
    // Free resources.
    ::CloseHandle(g_hCom);
    g_hCom = INVALID_HANDLE_VALUE;
  }
}


//  ****************************************************************************
bool IsTransmitting()
{
  return g_is_transmitting;
}

//  ****************************************************************************
bool IsListening()
{
  return g_is_listening;
}

//  ****************************************************************************
bool IsConnected()
{
  return g_is_connected;
}

//  ****************************************************************************
bool IsArmed()
{
  return g_is_armed;
}

//  ****************************************************************************
void GetCommandedInput(QCopter &commanded)
{
  commanded = g_commanded;
}

//  ****************************************************************************
int16_t GetBaselineThrust()
{
  return g_baseline_thrust;
}

//  ****************************************************************************
int16_t GetCompositeThrust()
{
  int32_t composite = g_baseline_thrust + g_commanded.thrust;

  if (composite > std::numeric_limits<int16_t>::max())
    composite = std::numeric_limits<int16_t>::max();
  else if (composite < std::numeric_limits<int16_t>::min())
    composite = std::numeric_limits<int16_t>::min();

  return composite;
}

//  ****************************************************************************
float GetBaseLatitude()
{
  return to_normalized(g_base_location.latitude) * 90.0f;
}

//  ****************************************************************************
float GetBaseLongitude()
{
  return to_normalized(g_base_location.longitude) * 180.0f;
}

//  ****************************************************************************
float GetBaseAltitude()
{
  return to_normalized(g_base_location.altitude) * 10000.0f;
}

//  ****************************************************************************
float GetCurrentLatitude()
{
  return to_normalized(g_drone_state.position.latitude) * 90.0f;
}

//  ****************************************************************************
float GetCurrentLongitude()
{
  return to_normalized(g_drone_state.position.longitude) * 180.0f;
}

//  ****************************************************************************
float GetCurrentAltitude()
{
  return to_normalized(g_drone_state.position.altitude) * 10000.0f;
}

//  ****************************************************************************
float GetRange()
{
  return 0.f;
}


//  ****************************************************************************
void GetDroneState(DroneState &state)
{
  state = g_drone_state;
}

//  ****************************************************************************
void GetPIDState(DronePIDs &state)
{
  state = g_PID_state;
}

//  ****************************************************************************
uint16_t GetSequenceId()
{
  return g_next_sequence++;
}

//  ****************************************************************************
bool HasGainAdjustment()
{
  return  g_has_roll_adjustment 
       || g_has_roll_rate_adjustment
       || g_has_pitch_adjustment
       || g_has_pitch_rate_adjustment
       || g_has_rotate_adjustment;
}

//  ****************************************************************************
bool UpdateLatestSeqId(uint16_t seq)
{
  if ( seq < g_last_recv_seq
    && g_last_recv_seq != std::numeric_limits<uint16_t>::max())
  {
    return false;
  }

  g_last_recv_seq = seq;

  return true;
}

//  ****************************************************************************
void ProcessThrustHold(Controller &controller)
{
  static bool is_thrust_hold_pressed   = false; // Mapped to right shoulder button
  static bool is_thrust_reset_pressed  = false; // Mapped to left shoulder button

  // Process each of the thrust hold buttons.
  // They are organized in a priority order so only 
  // one button can operate at a time.
  if (is_thrust_hold_pressed != controller.RShoulderPressed())
  {
    // If the button is being released, 
    // set a baseline thrust level.
    if (is_thrust_hold_pressed)
    {
      g_baseline_thrust = g_commanded.thrust;
    }

    is_thrust_hold_pressed = !is_thrust_hold_pressed;
  }
  else if (is_thrust_reset_pressed != controller.LShoulderPressed())
  {
    // If the button is being released, 
    // clear the baseline thrust level, set back to 0.
    if (is_thrust_reset_pressed)
    {
      g_baseline_thrust = 0;
    }

    is_thrust_reset_pressed = !is_thrust_reset_pressed;
  }
}

//  ****************************************************************************
void ProcessThrustNudge(Controller &controller)
{
  static bool is_thrust_increase_pressed  = false; // Mapped to D-Pad Up
  static bool is_thrust_decrease_pressed  = false; // Mapped to D-Pad Down

  // Process each of the buttons.
  // They are organized in a priority order so only 
  // one button can operate at a time.
  if (is_thrust_decrease_pressed != controller.DPadDownPressed())
  {
    // If the button is being released, 
    // increase the baseline thrust level by 2%.
    if (is_thrust_decrease_pressed)
    {
      int16_t adjusted = g_baseline_thrust - 2*k_command_percent;
      g_baseline_thrust = adjusted > g_baseline_thrust
                          ? std::numeric_limits<int16_t>::min()
                          : adjusted;
    }

    is_thrust_decrease_pressed = !is_thrust_decrease_pressed;
  }
  else if (is_thrust_increase_pressed != controller.DPadUpPressed())
  {
    // If the button is being released, 
    // increase the baseline thrust level by 2%.
    if (is_thrust_increase_pressed)
    {
      int16_t adjusted = g_baseline_thrust + 2*k_command_percent;
      g_baseline_thrust = adjusted < g_baseline_thrust
                          ? std::numeric_limits<int16_t>::max()
                          : adjusted;
    }

    is_thrust_increase_pressed = !is_thrust_increase_pressed;
  }
}

//  ****************************************************************************
int16_t ScaleInput(int16_t raw_value, int16_t threshold, float scale)
{
  if (raw_value < 0)
  {
    return int16_t((raw_value + threshold) / scale);
  }
  else if (raw_value > 0)
  {
    return int16_t((raw_value - threshold) / scale); 
  }
  else
  {
    return 0;
  }
}


//  ****************************************************************************
void ProcessInput(HWND hWnd, Controller &controller)
{
  static const 
    float k_left_scale = 
      (std::numeric_limits<int16_t>::max() - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
      / (float)std::numeric_limits<int16_t>::max();

  static const 
    float k_right_scale = 
      (std::numeric_limits<int16_t>::max() - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
      / (float)std::numeric_limits<int16_t>::max();

  static const 
    float k_trigger_scale = 
      (std::numeric_limits<int8_t>::max() - XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
      / (float)std::numeric_limits<int8_t>::max();

  // Capture the latest state for the controller.
  controller.Refresh();

  QCopter last = g_commanded;

  if (k_user_input_signal == g_control_signal)
  {
    // Calculate the control values.
    int16_t raw_roll    = controller.LeftThumbX();
    int16_t raw_pitch   = controller.LeftThumbY();
    int16_t raw_thrust  = controller.RightThumbY();
    int16_t raw_yaw     = controller.RightTrigger() - controller.LeftTrigger();

    // Invert the pitch and roll commands.
    g_commanded.roll    = ScaleInput(raw_roll,   XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, k_left_scale);
    g_commanded.pitch   = ScaleInput(raw_pitch,  XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, k_left_scale);
    g_commanded.thrust  = ScaleInput(raw_thrust, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE, k_right_scale);
    g_commanded.yaw     = raw_yaw;

    if (g_commanded.pitch == std::numeric_limits<int16_t>::min())
      g_commanded.pitch = std::numeric_limits<int16_t>::max();
    else
      g_commanded.pitch = -g_commanded.pitch;
  }
  else if (k_square_wave_signal == g_control_signal)
  {
    uint32_t period= g_control_signal_period * 1000;

    uint32_t T        = GetTickCount() / period;
    uint32_t command  = T % 2
                        ? 8000 * g_control_signal_scalar
                        : -8000 * g_control_signal_scalar;

    if (g_stage_use_roll_signal)
      g_commanded.roll  = command;

    if (g_stage_use_pitch_signal)
      g_commanded.pitch = command;

    // TODO: Need to scale at a different resolution
    //if (g_stage_use_yaw_signal)
    //  g_commanded.yaw   = command;
  }
  else if (k_sine_wave_signal == g_control_signal)
  {
    uint32_t period= g_control_signal_period * 1000;

    const 
      float k_2pi     = 2 * 3.1415926535897932384626433832795f;
    const 
      uint16_t range  = (1 << 13) * g_control_signal_scalar;  // ~7.5°

    uint32_t T     = GetTickCount() % period;
    float    theta = k_2pi * T / float(period);
    uint32_t command  = int16_t(sinf(theta) * range);

    if (g_stage_use_roll_signal)
      g_commanded.roll  = command;

    if (g_stage_use_pitch_signal)
      g_commanded.pitch = command;

    // TODO: Need to scale at a different resolution
    //if (g_stage_use_yaw_signal)
    //  g_commanded.yaw   = command;
  }
  else if (k_one_sec_impulse == g_control_signal)
  {
    uint32_t period = g_control_signal_period * 1000;

    uint32_t T        = (GetTickCount() % period) / 1000;
    uint32_t command  = 0;

    switch (T)
    {
    case 0:
      command = 8000 * g_control_signal_scalar;
      break;
    case 5:
      command = -8000 * g_control_signal_scalar;
      break;
    default:
      command = 0;
    }

    if (g_stage_use_roll_signal)
      g_commanded.roll  = command;

    if (g_stage_use_pitch_signal)
      g_commanded.pitch = command;
  }



  // The trigger buttons only report to 255. 
  // Therefore, yaw must be shifted to the left 7-bits to use
  // the same range as the other 3 control parameters.
  g_commanded.yaw <<= 7;

  // Process the thrust hold/reset pair.
  ProcessThrustHold(controller);
  ProcessThrustNudge(controller);

  ::PostMessage(hWnd, QC_PROCESS_INPUT, 0, 0);
}


//  ****************************************************************************
int DroneConnect(HANDLE hCom)
{
  QCArmMsg data_out = {0};

  PopulateQCHeader(data_out);
  data_out.cookie = htonl(g_connect_cookie);

  const   size_t k_data_len = sizeof(QCArmMsg);
  uint8_t buffer[k_data_len] = {0};

  size_t offset = 0;

  offset = Serialize(data_out.header, buffer, k_data_len);

  // Encode the type of PID:
  ::memcpy(buffer + offset, (char*)&data_out.cookie, sizeof(data_out.cookie));
  offset += sizeof(data_out.cookie);

  return write_message(hCom, 
                       buffer, 
                       sizeof(QCArmMsg));
}

//  ****************************************************************************
int SendCommands(HANDLE hCom)
{
  QCControlMsg data_out = {0};

  PopulateQCHeader(data_out);

  // Serialize the structure:
  const   size_t k_data_len = sizeof(QCControlMsg);
  uint8_t buffer[k_data_len] = {0};

  size_t offset = 0;
  int16_t value = 0;

  offset = Serialize(data_out.header, buffer, k_data_len);

  // Serialize the command values.
  uint8_t *p_cur = buffer + offset;

  offset += Serialize_int16(g_commanded.roll,     &p_cur);
  offset += Serialize_int16(g_commanded.pitch,    &p_cur);
  offset += Serialize_int16(g_commanded.yaw,      &p_cur);
  offset += Serialize_int16(GetCompositeThrust(), &p_cur);


  return write_message(hCom, buffer, offset);
}

//  ****************************************************************************
int SendGainAdjustment(HANDLE hCom, const PIDDesc &desc, PIDType type)
{
  QCAdjustGainMsg data_out;

  PopulateQCHeader(data_out);

  // Serialize the structure:
  const   size_t k_data_len = sizeof(QCAdjustGainMsg);
  uint8_t buffer[k_data_len] = {0};

  size_t offset = 0;

  offset = Serialize(data_out.header, buffer, k_data_len);

  // Encode the type of PID:
  data_out.type = PIDType(htonl(type));
  ::memcpy(buffer + offset, (char*)&data_out.type, sizeof(data_out.type));
  offset += sizeof(data_out.type);

  // Encode the PID Description values.
  data_out.desc = to_network(desc);
  ::memcpy(buffer + offset, (char*)&data_out.desc.Kp, sizeof(uint64_t));
  offset += sizeof(uint64_t);

  ::memcpy(buffer + offset, (char*)&data_out.desc.Ki, sizeof(uint64_t));
  offset += sizeof(uint64_t);

  ::memcpy(buffer + offset, (char*)&data_out.desc.Kd, sizeof(uint64_t));
  offset += sizeof(uint64_t);

  ::memcpy(buffer + offset, (char*)&data_out.desc.range_min, sizeof(int64_t));
  offset += sizeof(int64_t);

  ::memcpy(buffer + offset, (char*)&data_out.desc.range_max, sizeof(int64_t));
  offset += sizeof(int64_t);


  return write_message(hCom, buffer, sizeof(QCAdjustGainMsg));
}


//  ****************************************************************************
int SendGainAdjustment(HANDLE hCom)
{
  int result = 0;

  if (g_has_roll_adjustment)
  {
    result = SendGainAdjustment(hCom, g_PID_roll, k_roll);
    g_has_roll_adjustment = false;
  }

  if (g_has_roll_rate_adjustment)
  {
    result = SendGainAdjustment(hCom, g_PID_roll_rate, k_roll_rate);
    g_has_roll_rate_adjustment = false;
  }

  if (g_has_pitch_adjustment)
  {
    result = SendGainAdjustment(hCom, g_PID_pitch, k_pitch);
    g_has_pitch_adjustment = false;
  }

  if (g_has_pitch_rate_adjustment)
  {
    result = SendGainAdjustment(hCom, g_PID_pitch_rate, k_pitch_rate);
    g_has_pitch_rate_adjustment = false;
  }

  if (g_has_rotate_adjustment)
  {
    result = SendGainAdjustment(hCom, g_PID_rotation, k_rotate);
    g_has_rotate_adjustment = false;
  }

  return result;
}



//  ****************************************************************************
int SendControlMode(HANDLE hCom)
{
  QCControlModeMsg data;

  PopulateQCHeader(data);
                        
  // Encode the controlmode fields:
  data.control_mode   = static_cast<uint8_t>(g_stage_control_mode);
  data.disable_roll   = g_stage_disable_roll  ? 0 : 1;
  data.disable_pitch  = g_stage_disable_pitch ? 0 : 1;
  data.disable_yaw    = g_stage_disable_yaw   ? 0 : 1;

  // Serialize the structure:
  const   size_t k_data_len = sizeof(QCControlModeMsg);
  uint8_t buffer[k_data_len] = {0};

  size_t offset = 0;

  offset = Serialize(data.header, buffer, k_data_len);

  uint8_t* p_cur = buffer + offset;

  p_cur[0] = data.control_mode;
  p_cur[1] = data.disable_roll;
  p_cur[2] = data.disable_pitch;
  p_cur[3] = data.disable_yaw;

  offset += 4;

  return write_message(hCom, buffer, sizeof(QCControlModeMsg));
}


//  ****************************************************************************
int SendHalt(HANDLE hCom, uint32_t status)
{
  QCHaltMsg data_out;

  PopulateQCHeader(data_out);

  // Serialize the structure:
  const   size_t k_data_len = sizeof(QCHaltMsg);
  uint8_t buffer[k_data_len] = {0};

  size_t offset = 0;

  offset = Serialize(data_out.header, buffer, k_data_len);

  uint32_t value = htonl(status);
  ::memcpy(buffer + offset, (char*)&value, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  int bytes = write_message(hCom, buffer, offset);

  g_is_armed     = false;
  g_is_connected = false;

  return bytes;

}

//  ****************************************************************************
void SendBeaconResponse(HWND hWnd, HANDLE hCom)
{
  // TODO: Add a windows notification to indicate a beacon was received and differentiate from the arming state.

  QCBeaconAckMsg data_out;

  PopulateQCHeader(data_out);

  // Serialize the structure:
  const   size_t k_data_len = sizeof(QCBeaconAckMsg);
  uint8_t buffer[k_data_len] = {0};

  size_t offset = 0;

  offset = Serialize(data_out.header, buffer, k_data_len);

  // Encode the response:
  data_out.cookie = htonl(g_beacon_cookie);
  ::memcpy(buffer + offset, (char*)&data_out.cookie, sizeof(data_out.cookie));
  offset += sizeof(data_out.cookie);

  data_out.status = 0;
  ::memcpy(buffer + offset, (char*)&data_out.status, sizeof(data_out.status));
  offset += sizeof(data_out.status);


  int bytes = write_message(hCom, buffer, sizeof(data_out));

  if (bytes > 0)
  {
    g_beacon_cookie = 0;
  }
}

//  ****************************************************************************
void ProcessDroneConnect(HWND hWnd, HANDLE hCom, Controller &controller)
{
  static bool is_start_pressed   = false; // Mapped to the start button

  if (is_start_pressed != controller.StartPressed())
  {
    // If the button is being released, 
    // set the connect (start) command to the drone.
    if (is_start_pressed)
    {
      int result = DroneConnect(hCom);
      if (result < 0)
      {
        ::PostMessage(hWnd, QC_DRONE_CONNECT_ERROR, result, 0);
      }
    }

    is_start_pressed = !is_start_pressed;
  }

}

//  ****************************************************************************
bool ProcessDroneHalt(HANDLE hCom, Controller &controller)
{
  static bool is_halt_pressed   = false; // Mapped to the back button

  bool result = false;
  if (is_halt_pressed != controller.BackPressed())
  {
    // If the button is being released, 
    // set the halt command to the drone.
    if (is_halt_pressed)
    {
      SendHalt(hCom, 0);
      result = true;
    }

    is_halt_pressed = !is_halt_pressed;
  }

  return result;
}

//  ****************************************************************************
void DroneTransmitter(HWND hWnd, Controller &controller)
{
  //if (INVALID_HANDLE_VALUE == g_hCom)
  //{
  //  g_hCom = OpenConnection(hWnd);
  //}

  // Process input at 100Hz.
  while (IsTransmitting())
  {
    // Refresh and update the input state.
    ProcessInput(hWnd, controller);
    
    // Test for the "Kill Switch" call
    // to shutdown the drones motors and disconnect controls.
    if (ProcessDroneHalt(g_hCom, controller))
    {
      ::PostMessage(hWnd, QC_DRONE_DISCONNECTED, 0, 0);
      break;
    }

    if (IsArmed())
    {
      int result = SendCommands(g_hCom);
      if (result < 0)
      {
        ::PostMessage(hWnd, QC_SEND_COMMAND_ERROR, result, 0);
      }

      // If a gain adjustment has been queued, 
      // send that as well.
      if (HasGainAdjustment())
      {
        result = SendGainAdjustment(g_hCom);
        if (result < 0)
        {
          ::PostMessage(hWnd, QC_SEND_GAIN_ERROR, result, 0);
        }
      }

      if (g_has_control_mode_change)
      {
        SendControlMode(g_hCom);
        g_has_control_mode_change = false;
      }
    }
    else 
    {
      if (g_beacon_cookie != 0)
      {
        SendBeaconResponse(hWnd, g_hCom);
      }
      else      
      {
        ProcessDroneConnect(hWnd, g_hCom, controller);    
      }
    }    

    // Pause; sample input at 100Hz.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  CloseConnection(g_hCom);
}

//  ****************************************************************************
void ProcessBeacon(HWND hWnd, const uint8_t* p_buffer, int len)
{
  if (len != sizeof(QCBeaconMsg))
    return;

  QCBeaconMsg &data = *(QCBeaconMsg*)p_buffer;
  if (!UpdateLatestSeqId(ntohs(data.header.seq_id)))
  {
    // This is stale state.
    return;
  }

  g_beacon_cookie = ntohl(data.cookie);

  //// Extract the address from the sender.
  // This code was for socket communication, we're now using serial.
  //g_drone_address = ntohl(addr.sin_addr.S_un.S_addr);

  //// Update the IP Address on the control display.
  //::PostMessage(hWnd, QC_DRONE_CONNECTED, addr.sin_addr.S_un.S_addr, 0);
  ::PostMessage(hWnd, QC_DRONE_CONNECTED, 0, 0);

  g_is_connected = true;
}

//  ****************************************************************************
void DroneArmed(HWND hWnd, const uint8_t* p_buffer, int len)
{
  if (len != sizeof(QCArmAck))
    return;

  QCArmAck &data = *(QCArmAck*)p_buffer;
  if (!UpdateLatestSeqId(ntohs(data.header.seq_id)))
  {
    // This is stale state.
    return;
  }

  size_t   offset = sizeof(QCHeader);

  const uint8_t* p_cur = p_buffer + offset;

  offset += Deserialize_uint32(data.cookie, &p_cur);
  offset += Deserialize_uint32(data.status, &p_cur);

  // Read the base (starting) location of the drone before takeoff.
  data.base_position.is_valid = p_cur[0];
  p_cur++;
  offset++;

  offset += Deserialize_int32(data.base_position.latitude,   &p_cur);
  offset += Deserialize_int32(data.base_position.longitude,  &p_cur);
  offset += Deserialize_int32(data.base_position.altitude,   &p_cur);
  offset += Deserialize_int32(data.base_position.height,     &p_cur);

  g_base_location = data.base_position;

  // Update the IP Address on the control display.
  ::PostMessage(hWnd, QC_DRONE_ARMED, 0, 0);

  g_is_armed = true;
}

//  ****************************************************************************
void UpdateDroneState(HWND hWnd, const uint8_t* p_buffer, int len)
{
  if ( !p_buffer
    || len != sizeof(QCDroneStateMsg))
    return;

  QCDroneStateMsg  drone_data;

  // Extract the values from the receive buffer.
  uint16_t value = 0;
  int      offset = 0;

  // Read the header fields.
  offset += Deserialize(drone_data.header, p_buffer, len);

  if (!UpdateLatestSeqId(drone_data.header.seq_id))
  {
    // Stale data, ignore this message.
    return;
  }

  const uint8_t *p_cur = p_buffer + offset;

  // Update the drone's armed status.
  drone_data.state.is_armed = *p_cur;
  p_cur++;
  offset++;

  bool armed_status = drone_data.state.is_armed != 0;

  if (armed_status != g_is_armed)
  {
    if (armed_status)
      ::PostMessage(hWnd, QC_DRONE_ARMED, 0, 0);
    else
      ::PostMessage(hWnd, QC_DRONE_CONNECTED, 0, 0);

    g_is_armed = armed_status;
  }

  // Read the drone's orientation.
  offset += Deserialize_int16(drone_data.state.orientation.roll,        &p_cur);
  offset += Deserialize_int16(drone_data.state.orientation.pitch,       &p_cur);
  offset += Deserialize_int16(drone_data.state.orientation.yaw,         &p_cur);
  offset += Deserialize_int16(drone_data.state.orientation.roll_rate,   &p_cur);
  offset += Deserialize_int16(drone_data.state.orientation.pitch_rate,  &p_cur);
  offset += Deserialize_int16(drone_data.state.orientation.yaw_rate,    &p_cur);

  drone_data.state.position.is_valid = p_cur[0];
  p_cur++;
  offset++;

  offset += Deserialize_int32(drone_data.state.position.latitude,   &p_cur);
  offset += Deserialize_int32(drone_data.state.position.longitude,  &p_cur);
  offset += Deserialize_int32(drone_data.state.position.altitude,   &p_cur);
  offset += Deserialize_int32(drone_data.state.position.height,     &p_cur);


  // Populate the commanded level of each motor.
  offset += Deserialize_uint16(drone_data.state.motor.A,  &p_cur);
  offset += Deserialize_uint16(drone_data.state.motor.B,  &p_cur);
  offset += Deserialize_uint16(drone_data.state.motor.C,  &p_cur);
  offset += Deserialize_uint16(drone_data.state.motor.D,  &p_cur);
  offset += Deserialize_uint16(drone_data.state.motor.E,  &p_cur);
  offset += Deserialize_uint16(drone_data.state.motor.F,  &p_cur);
  offset += Deserialize_uint16(drone_data.state.motor.G,  &p_cur);
  offset += Deserialize_uint16(drone_data.state.motor.H,  &p_cur);


  // Deserialize the battery-level information
  offset += Deserialize(drone_data.state.batteries, (uint8_t*)p_buffer + offset, len-offset);


  g_drone_state = drone_data.state;
  

  ::PostMessage(hWnd, QC_UPDATE_STATUS, 0, 0);
}

//  ****************************************************************************
void UpdatePIDState(HWND hWnd, const uint8_t* p_buffer, int len)
{
  if ( !p_buffer
    || len != sizeof(QCPIDStateMsg))
    return;

  QCPIDStateMsg  PID_data;

  // Extract the values from the receive buffer.
  uint16_t value = 0;
  int      offset = 0;

  // Read the header fields.
  offset += Deserialize(PID_data.header, p_buffer, len);

  if (!UpdateLatestSeqId(PID_data.header.seq_id))
  {
    // Stale data, ignore this message.
    return;
  }

  const uint8_t *p_cur = p_buffer + offset;

  // Read the TYPE of PID.
  uint32_t type = k_none;
  offset += Deserialize_uint32(type, &p_cur);

  PIDState state = { 0 };
  offset += Deserialize(state,   (uint8_t*)p_buffer + offset, len-offset);

  // Read the PID state.
  switch (type)
  {
  case k_roll_rate:
    g_PID_state.roll_rate = state;
    break;
  case k_roll:
    g_PID_state.roll = state;
    break;
  case k_pitch_rate:
    g_PID_state.pitch_rate = state;
    break;
  case k_pitch:
    g_PID_state.pitch = state;
    break;
  case k_rotate:
    g_PID_state.rotation = state;
    break;
  }

  ::PostMessage(hWnd, QC_UPDATE_STATUS, 0, 0);
}

//  ****************************************************************************
void DroneDisconnected(HWND hWnd, const uint8_t* p_buffer, int len)
{
  if (len != sizeof(QCDisarmMsg))
    return;

  QCDisarmMsg &data = *(QCDisarmMsg*)p_buffer;
  if (!UpdateLatestSeqId(ntohs(data.header.seq_id)))
  {
    // This is stale state.
    return;
  }

  //g_is_armed     = false;
  //g_is_connected = false;
}

//  ****************************************************************************
void DispatchDroneMessage(HWND hWnd, const uint8_t* p_buffer, int len)
{
  if (len > 0)
  {
    // Identify the type of message.
    uint16_t type = DecodeMessageType(p_buffer, len);

    switch (type)
    {
    case k_qc_msg_beacon:
      ProcessBeacon(hWnd, p_buffer, len);
      break;
    case k_qc_msg_arm_ack:
      DroneArmed(hWnd, p_buffer, len);
      break;
    case k_qc_msg_drone_state:
      UpdateDroneState(hWnd, p_buffer, len);

      // We most likely missed the ACK, or we reconnected.
      // Update the status to indicate we are connected.
      // TODO: This will require more consideration to ensure this sequence is secure and we do not cross signals with additional drones.
      if (!IsConnected())
      {
        // Update the IP Address on the control display.
//        ::PostMessage(hWnd, QC_DRONE_CONNECTED, addr.sin_addr.S_un.S_addr, 0);
        ::PostMessage(hWnd, QC_DRONE_CONNECTED, 0, 0);
        g_is_connected = true;
      }

      break;
    case k_qc_msg_disarm:
      DroneDisconnected(hWnd, p_buffer, len);
      break;
    default:
      ::PostMessage(hWnd, QC_DRONE_RECEIVE_ERROR, -1, 0);
    }
  }
  else
  {
    ::PostMessage(hWnd, QC_DRONE_RECEIVE_ERROR, WSAGetLastError(), 0);
  }
}

//  ****************************************************************************
void DroneReceiver(HWND hWnd)
{
  const size_t k_buffer_size = 2048;

  if (INVALID_HANDLE_VALUE == g_hCom)
  {
    g_hCom = OpenConnection(hWnd);
  }

  // Read any messages waiting on the queue.
  while (IsListening())
  {
    uint8_t buffer[k_buffer_size];     
    
    int bytes = read_message(g_hCom, buffer, k_buffer_size);

    DispatchDroneMessage(hWnd, buffer, bytes);
  }

  CloseConnection(g_hCom);
}


//  ****************************************************************************
void StartTransmission(HWND hWnd, Controller &controller)
{
  if (g_is_transmitting)
    return;

  // Release the previous instance.
  delete gp_transmitter;
  gp_transmitter = nullptr;

  g_is_transmitting = true;
  gp_transmitter = new std::thread(DroneTransmitter,
                                   hWnd,
                                   controller);
}

//  ****************************************************************************
void HaltTransmission()
{
  g_is_transmitting = false;
}

//  ****************************************************************************
void StartListening(HWND hWnd)
{
  if (g_is_listening)
    return;

  // Release the previous instance.
  delete gp_receiver;
  gp_receiver = nullptr;

  g_is_listening = true;
  gp_receiver = new std::thread(DroneReceiver,
                                hWnd);
}

//  ****************************************************************************
void HaltListening()
{
  g_is_listening = false;
}


//  ****************************************************************************
void AdjustGain(PIDDesc *roll, PIDDesc *pitch, PIDDesc *rotation)
{
  if (roll)
  {
    g_PID_roll              = *roll;  
    g_has_roll_adjustment   = true;
  }

  if (pitch)
  {
    g_PID_pitch             = *pitch;  
    g_has_pitch_adjustment  = true;
  }

  //if (rotation)
  //{
  //  g_PID_rotation          = *rotation;  
  //  g_has_rotate_adjustment = true;
  //}
}

//  ****************************************************************************
void AdjustRateGain(PIDDesc *roll_rate, PIDDesc *pitch_rate, PIDDesc *rotation_rate)
{
  if (roll_rate)
  {
    g_PID_roll_rate         = *roll_rate;  
    g_has_roll_rate_adjustment   = true;
  }

  if (pitch_rate)
  {
    g_PID_pitch_rate        = *pitch_rate;  
    g_has_pitch_rate_adjustment  = true;
  }

  if (rotation_rate)
  {
    g_PID_rotation          = *rotation_rate;  
    g_has_rotate_adjustment = true;
  }
}

//  ****************************************************************************
void  ModifyControlMode(ControlMode mode, bool use_roll, bool use_pitch, bool use_yaw)
{
  g_stage_control_mode      = mode;
  g_stage_disable_roll      = use_roll;
  g_stage_disable_pitch     = use_pitch;
  g_stage_disable_yaw       = use_yaw;

  g_has_control_mode_change = true;
}

//  ****************************************************************************
void  ModifyControlSignal(bool use_roll, bool use_pitch, bool use_yaw)
{
  g_stage_use_roll_signal      = use_roll;
  g_stage_use_pitch_signal     = use_pitch;
  g_stage_use_yaw_signal       = use_yaw;
}

//  ****************************************************************************
void ArmDrone()
{
  if (IsArmed())
  {
    SendHalt(g_hCom, 0);
  }
  else
  {
    DroneConnect(g_hCom);
  }
}
