/// @file qcctrl.h
/// 
/// Captures controller input for transmission to the drone.
///
//  ****************************************************************************
#ifndef QCCTRL_H_INCLUDED
#define QCCTRL_H_INCLUDED

#include <cstdint>
#include <winsock2.h>
#include "../Common/qc_msg.h"
#include "XBoxCtrl/XBoxController.h"

using pbl::input::win32::xbox::Controller;
using pbl::input::win32::xbox::k_xboxCtrl_1;
using pbl::input::win32::xbox::k_xboxCtrl_2;
using pbl::input::win32::xbox::k_xboxCtrl_3;
using pbl::input::win32::xbox::k_xboxCtrl_4;


#define QC_DRONE_CONNECTED          (WM_USER + 101)
#define QC_DRONE_ARMED              (WM_USER + 102)
#define QC_DRONE_DISCONNECTED       (WM_USER + 103)
#define QC_PROCESS_INPUT            (WM_USER + 104)
#define QC_UPDATE_STATUS            (WM_USER + 105)

#define QC_DRONE_RECEIVE_ERROR      (WM_USER + 500)
#define QC_DRONE_CONNECT_ERROR      (WM_USER + 501)
#define QC_DRONE_CONNECT_ACK_ERROR  (WM_USER + 502)
#define QC_SEND_COMMAND_ERROR       (WM_USER + 503)
#define QC_SEND_GAIN_ERROR          (WM_USER + 504)
#define QC_DRONE_STATE_ERROR        (WM_USER + 505)


//  ****************************************************************************
enum TrimMode
{
  k_trim_none     = 0,
  k_trim_roll     = 1,
  k_trim_pitch    = 2,
  k_trim_yaw      = 3,
  k_trim_thrust   = 4,
  k_trim_motor_A  = 5,
  k_trim_motor_B  = 6,
  k_trim_motor_C  = 7,
  k_trim_motor_D  = 8,
  k_trim_max    
};

enum TrimAdjust
{
  k_decrement = -1,
  k_increment = 1
};

enum ControlSignal
{
  k_user_input_signal = 1,
  k_square_wave_signal,
  k_sine_wave_signal,
  k_one_sec_impulse
};


extern ControlSignal   g_control_signal;

extern float           g_control_signal_period;
extern float           g_control_signal_scalar;


//  ****************************************************************************

bool IsConnected();
bool IsArmed();

void StartTransmission(HWND hWnd, Controller &controller);
void HaltTransmission();
bool IsTransmitting();

void StartListening(HWND hWnd);
void HaltListening();
bool IsListening();

void GetCommandedInput(QCopter &commanded);
int16_t GetBaselineThrust();
int16_t GetCompositeThrust();

void GetDroneState(DroneState &state);
void GetPIDState(DronePIDs &state);
TrimMode GetTrimMode();

void AdjustGain(PIDDesc &roll, PIDDesc &pitch, PIDDesc &rotation);
void AdjustRateGain(PIDDesc &roll_rate, PIDDesc &pitch_rate, PIDDesc &rotation_rate);

float GetBaseLatitude();
float GetBaseLongitude();
float GetBaseAltitude();
float GetCurrentLatitude();
float GetCurrentLongitude();
float GetCurrentAltitude();
float GetRange();

void  ModifyControlMode(ControlMode mode, bool use_roll, bool use_pitch, bool use_yaw);
void  ModifyControlSignal(bool use_roll, bool use_pitch, bool use_yaw);

void  ArmDrone();


#endif
