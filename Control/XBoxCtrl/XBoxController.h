/// @file XBoxController.h 
/// XBox Controller device implementation to interact with the PBL
/// device abstraction input interface.
/// 
/// The controller and this class implementation are only supported on
/// Windows platforms that have the device drivers installed for the 
/// XBox360 controller.
/// 
/// Copyright 2011 Paul Watt
///
//  ****************************************************************************
//  ****************************************************************************
#ifndef XBOXCONTROLLER_H_INCLUDED
#define XBOXCONTROLLER_H_INCLUDED  
// Includes ********************************************************************
#ifdef _WIN32

#include "InputDevice.h"
#include "BLMisc.h"

// Include the controller library and link to the DLL.
#include <XInput.h>
#pragma comment(lib, "XInput.lib")

namespace pbl
{

namespace input
{

namespace win32
{

namespace xbox
{

//  Typedefs *******************************************************************
typedef uint32_t                  CtrlId;
typedef pbl::battery::BatteryType BatteryType;

//  Constants ******************************************************************
//  Controller ID Names 
const CtrlId  k_xboxCtrl_1            = 0;
const CtrlId  k_xboxCtrl_2            = 1;
const CtrlId  k_xboxCtrl_3            = 2;
const CtrlId  k_xboxCtrl_4            = 3;

//  Game pad control names
const KeyId   k_padDPadUp             = 0x00000001;
const KeyId   k_padDPadDown           = 0x00000002;
const KeyId   k_padDPadLeft           = 0x00000004;
const KeyId   k_padDPadRight          = 0x00000008;
const KeyId   k_padStart              = 0x00000010;
const KeyId   k_padBack               = 0x00000020;
const KeyId   k_padLThumb             = 0x00000040;
const KeyId   k_padRThumb             = 0x00000080;
const KeyId   k_padLShoulder          = 0x00000100;
const KeyId   k_padRShoulder          = 0x00000200;
const KeyId   k_padA                  = 0x00001000;
const KeyId   k_padB                  = 0x00002000;
const KeyId   k_padX                  = 0x00004000;
const KeyId   k_padY                  = 0x00008000;

const KeyId   k_gamePadLTrigger       = 0x00010000;
const KeyId   k_gamePadRTrigger       = 0x00020000;
const KeyId   k_gamePadThumbLX        = 0x00100000;
const KeyId   k_gamePadThumbLY        = 0x00200000;
const KeyId   k_gamePadThumbRX        = 0x00400000;
const KeyId   k_gamePadThumbRY        = 0x00800000;

const KeyId   k_gamePadThumbL         = k_gamePadThumbLX | k_gamePadThumbLY;
const KeyId   k_gamePadThumbR         = k_gamePadThumbRX | k_gamePadThumbRY;

//  Virtual Key Codes
const KeyId   vk_padA                 = VK_PAD_A;               // 0x5800
const KeyId   vk_padB                 = VK_PAD_B;               // 0x5801
const KeyId   vk_padX                 = VK_PAD_X;               // 0x5802
const KeyId   vk_padY                 = VK_PAD_Y;               // 0x5803
const KeyId   vk_padRShoulder         = VK_PAD_RSHOULDER;       // 0x5804
const KeyId   vk_padLShoulder         = VK_PAD_LSHOULDER;       // 0x5805
const KeyId   vk_padLTrigger          = VK_PAD_LTRIGGER;        // 0x5806
const KeyId   vk_padRTrigger          = VK_PAD_RTRIGGER;        // 0x5807

const KeyId   vk_padDPadUp            = VK_PAD_DPAD_UP;         // 0x5810
const KeyId   vk_padDPadDown          = VK_PAD_DPAD_DOWN;       // 0x5811
const KeyId   vk_padDPadLeft          = VK_PAD_DPAD_LEFT;       // 0x5812
const KeyId   vk_padDPadRight         = VK_PAD_DPAD_RIGHT;      // 0x5813
const KeyId   vk_padStart             = VK_PAD_START;           // 0x5814
const KeyId   vk_padBack              = VK_PAD_BACK;            // 0x5815
const KeyId   vk_padLThumbPress       = VK_PAD_LTHUMB_PRESS;    // 0x5816
const KeyId   vk_padRThumbPress       = VK_PAD_RTHUMB_PRESS;    // 0x5817

const KeyId   vk_padLThumbUp          = VK_PAD_LTHUMB_UP;       // 0x5820
const KeyId   vk_padLThumbDown        = VK_PAD_LTHUMB_DOWN;     // 0x5821
const KeyId   vk_padLThumbRight       = VK_PAD_LTHUMB_RIGHT;    // 0x5822
const KeyId   vk_padLThumbLeft        = VK_PAD_LTHUMB_LEFT;     // 0x5823
const KeyId   vk_padLThumbUpLeft      = VK_PAD_LTHUMB_UPLEFT;   // 0x5824
const KeyId   vk_padLThumbUpRight     = VK_PAD_LTHUMB_UPRIGHT;  // 0x5825
const KeyId   vk_padLThumbDownRight   = VK_PAD_LTHUMB_DOWNRIGHT;// 0x5826
const KeyId   vk_padLThumbDownLeft    = VK_PAD_LTHUMB_DOWNLEFT; // 0x5827

const KeyId   vk_padRThumbUp          = VK_PAD_RTHUMB_UP;       // 0x5830
const KeyId   vk_padRThumbDown        = VK_PAD_RTHUMB_DOWN;     // 0x5831
const KeyId   vk_padRThumbRight       = VK_PAD_RTHUMB_RIGHT;    // 0x5832
const KeyId   vk_padRThumbLeft        = VK_PAD_RTHUMB_LEFT;     // 0x5833
const KeyId   vk_padRThumbUpLeft      = VK_PAD_RTHUMB_UPLEFT;   // 0x5834
const KeyId   vk_padRThumbUpRight     = VK_PAD_RTHUMB_UPRIGHT;  // 0x5835
const KeyId   vk_padRThumbDownRight   = VK_PAD_RTHUMB_DOWNRIGHT;// 0x5836
const KeyId   vk_padRThumbDownLeft    = VK_PAD_RTHUMB_DOWNLEFT; // 0x5837

//  ****************************************************************************
/// Control pad event processing.
///
class GamePad
{
public:
  //  Typedefs ****************************************************************
  struct inputdata_t
  {
    KeyId             virtKey;
    ButtonState       state;
    uint16_t          flags;
    byte_t            extra;
    byte_t            userIndex;
    BLPoint           pt;
    bool              isAnalog;
  };

  typedef HANDLE                device_t;
  typedef XINPUT_CAPABILITIES   DeviceInfo;
  typedef XINPUT_KEYSTROKE      rawdata_t;

  //  Constants ****************************************************************
  static const UsageType  k_typeHid       = k_gamepad;

  //  Construction *************************************************************
  GamePad()  
    : m_isEnabled(false)
    , m_isConnected(false)                      { }

  //  Status *******************************************************************
  CtrlId      GetId             () const        { return m_ctrlId;}
  bool        IsConnected       () const        { return m_isConnected;}
  bool        IsEnabled         () const        { return m_isEnabled;}
  void        SetEnableState    (bool isEnable) { m_isEnabled = isEnable;
                                                  ::XInputEnable(isEnable);
                                                }

  uint32_t    GetType           () const        { return m_deviceInfo.Type;}
  uint32_t    GetSubType        () const        { return m_deviceInfo.SubType;}
  uint32_t    IsVoiceCaps       () const        { return m_deviceInfo.Flags   & XINPUT_CAPS_VOICE_SUPPORTED;}

  //  **************************************************************************
  /// Type of battery is returned, indication of a wired device.
  ///
  BatteryType GetBatteryType    () const        { return m_batteryType;}
  float       GetBatteryLevel   () const;

  //  Methods ******************************************************************
  //  **************************************************************************
  bool RegisterDevice(CtrlId index)             { m_ctrlId = index;
                                                  return UpdateDeviceInfo();
                                                }

  //  **************************************************************************
  void Decode(const rawdata_t &raw,
              inputdata_t     &data)
  {
    data.isAnalog= false;
    data.virtKey = raw.VirtualKey;
    if (XINPUT_KEYSTROKE_REPEAT & raw.Flags)
      data.state   = ButtonState(XINPUT_KEYSTROKE_REPEAT);
    else if (XINPUT_KEYSTROKE_KEYDOWN & raw.Flags)
      data.state   = ButtonState(XINPUT_KEYSTROKE_KEYDOWN);
    else if (XINPUT_KEYSTROKE_KEYUP & raw.Flags)
      data.state   = ButtonState(XINPUT_KEYSTROKE_KEYUP);

    // If one of the analog controls has been activated, 
    // update the cached control status, and return the value of the analog input.
    if (IsVkLeftTrigger(raw.VirtualKey))
    {
      UpdateInputState();
      data.extra    = LeftTrigger();
      data.isAnalog = true;
    }
    else if (IsVkRightTrigger(raw.VirtualKey))
    {
      UpdateInputState();
      data.extra    = RightTrigger();
      data.isAnalog = true;
    }
    else if (IsVkLeftThumb(raw.VirtualKey))
    {
      UpdateInputState();
      data.pt.x     = LeftThumbX();
      data.pt.y     = LeftThumbY();
      data.isAnalog = true;
    }
    else if (IsVkRightThumb(raw.VirtualKey))
    {
      UpdateInputState();
      data.pt.x     = RightThumbX();
      data.pt.y     = RightThumbY();
      data.isAnalog = true;
    }

    // TODO: It is unknown how this flag works, if the values are mutually exclusive.
    // For now assert to test if more than one flag type can appear.  
    // If so, something new will need to be devised.
    //BLASSERT( 0 == xInput.Flags
    //       || 1 == xInput.Flags
    //       || 2 == xInput.Flags
    //       || 4 == xInput.Flags);

    data.flags = raw.UserIndex;
  }

  //  **************************************************************************
  /// This function must be called periodically to update the 
  /// known status for the controller.  
  /// 
  /// @return  true if the controller is connected.
  ///         false if the controller is no longer detected.
  ///
  bool Refresh()                                { UpdateInputState();
                                                  return UpdateDeviceInfo();
                                                }

  /* Controller Control Status ************************************************/
  bool      DPadUpPressed   () const            { return !!(m_curState.Gamepad.wButtons & k_padDPadUp);}
  bool      DPadDownPressed () const            { return !!(m_curState.Gamepad.wButtons & k_padDPadDown);}
  bool      DPadLeftPressed () const            { return !!(m_curState.Gamepad.wButtons & k_padDPadLeft);}
  bool      DPadRightPressed() const            { return !!(m_curState.Gamepad.wButtons & k_padDPadRight);}

  bool      StartPressed    () const            { return !!(m_curState.Gamepad.wButtons & k_padStart);}
  bool      BackPressed     () const            { return !!(m_curState.Gamepad.wButtons & k_padBack);}

  bool      LThumbPressed   () const            { return !!(m_curState.Gamepad.wButtons & k_padLThumb);}
  bool      RThumbPressed   () const            { return !!(m_curState.Gamepad.wButtons & k_padRThumb);}
  bool      LShoulderPressed() const            { return !!(m_curState.Gamepad.wButtons & k_padLShoulder);}
  bool      RShoulderPressed() const            { return !!(m_curState.Gamepad.wButtons & k_padRShoulder);}

  bool      APressed        () const            { return !!(m_curState.Gamepad.wButtons & k_padA);}
  bool      BPressed        () const            { return !!(m_curState.Gamepad.wButtons & k_padB);}
  bool      XPressed        () const            { return !!(m_curState.Gamepad.wButtons & k_padX);}
  bool      YPressed        () const            { return !!(m_curState.Gamepad.wButtons & k_padY);}

  byte_t    LeftTrigger     () const            { return FilterTriggerThreshold(m_curState.Gamepad.bLeftTrigger);}
  byte_t    RightTrigger    () const            { return FilterTriggerThreshold(m_curState.Gamepad.bRightTrigger);}

  int16_t   LeftThumbX      () const            { return FilterLeftThumbStickThreshold(m_curState.Gamepad.sThumbLX);}
  int16_t   LeftThumbY      () const            { return FilterLeftThumbStickThreshold(m_curState.Gamepad.sThumbLY);}
  int16_t   RightThumbX     () const            { return FilterRightThumbStickThreshold(m_curState.Gamepad.sThumbRX);}
  int16_t   RightThumbY     () const            { return FilterRightThumbStickThreshold(m_curState.Gamepad.sThumbRY);}

  /* Controller Vibration Status **********************************************/
  float     GetLowVibrate   () const            { return m_deviceInfo.Vibration.wLeftMotorSpeed / 65535.0f;}
  bool      SetLowVibrate   (float level)       { return Vibrate(level, GetHighVibrate());}

  float     GetHighVibrate  () const            { return m_deviceInfo.Vibration.wRightMotorSpeed / 65535.0f;}
  bool      SetHighVibrate  (float level)       { return Vibrate(GetLowVibrate(), level);}

  bool      Vibrate         (float low,
                             float high);

  //  **************************************************************************
  static 
    bool IsVkLeftTrigger  (KeyId vkCode);
  static
    bool IsVkRightTrigger (KeyId vkCode);
  static
    bool IsVkLeftThumb    (KeyId vkCode);
  static
    bool IsVkRightThumb   (KeyId vkCode);

private:
  /* Members ******************************************************************/
  CtrlId        m_ctrlId;                   // The User Index of the controller.

  bool          m_isConnected;
  bool          m_isEnabled;                // Indicates if this object has
                                            // requested to receive notifications
                                            // regarding the state of the controller.
  DeviceInfo    m_deviceInfo;
  BatteryType   m_batteryType;     

  XINPUT_STATE  m_lastState;                // Previous state of controller.
  XINPUT_STATE  m_curState;                 // Current known state of controller.


  /* Methods ******************************************************************/
  bool    UpdateDeviceInfo();
  bool    UpdateInputState();
  byte_t  FilterTriggerThreshold(byte_t level) const;
  int16_t FilterLeftThumbStickThreshold(int16_t level) const;
  int16_t FilterRightThumbStickThreshold(int16_t level) const;

  //  **************************************************************************
  //  Many of the XInput functions return result values, which either
  //  indicate success or device not connected.  This Helper will
  //  process the result and update the status.
  //
  //  @param retVal[in]   The return value from any of the XInput calls.
  //  @return             The connected status of the controller is returned.
  //
  bool ProcessResult(DWORD retVal)             
  { 
    m_isConnected = (ERROR_DEVICE_NOT_CONNECTED != retVal);
    return m_isConnected;
  }
};


//  **************************************************************************
/// Returns the charge left in the battery of the device.
/// Reporting resolution depends upon the device.
/// 
/// @return   A value from 0.0 -> 1.0.  
///           0.0 = empty, 
///           1.0 = full charge.
///
inline 
float GamePad::GetBatteryLevel() const        
{ 
  XINPUT_BATTERY_INFORMATION battInfo;
  ::XInputGetBatteryInformation(m_ctrlId, 
                                BATTERY_DEVTYPE_GAMEPAD, 
                                &battInfo);
  return  float(battInfo.BatteryLevel)
        / float(BATTERY_LEVEL_FULL);
}


//  **************************************************************************
/// Updates basic information regarding the device.
/// &return    The connected status of the controller is returned.
///
inline 
bool GamePad::UpdateDeviceInfo()
{ 
  DWORD retVal = ::XInputGetCapabilities( m_ctrlId, 
                                          XINPUT_FLAG_GAMEPAD, 
                                          &m_deviceInfo);
  return ProcessResult(retVal);
}

//  **************************************************************************
/// Queries the controller driver for the current state of the controls.
/// @return true if any changes have occurred.
///         false if there have not been any recorded changes since the last call.
///
inline 
bool GamePad::UpdateInputState()
{ 
  m_lastState = m_curState;

  DWORD retVal = ::XInputGetState( m_ctrlId, &m_curState);
  ProcessResult(retVal);

  // Determine if there has been a change in controller state.
  return m_curState.dwPacketNumber != m_lastState.dwPacketNumber;
}

//  **************************************************************************
/// Filters the trigger buttons values if they do not reach a minimum
/// threshold.
/// @param level[in]: The level to test against the threshold.
/// @return           The level value if it is greater than the threshold, or 0.
///
inline 
byte_t GamePad::FilterTriggerThreshold(byte_t level) const
{
  return  level > XINPUT_GAMEPAD_TRIGGER_THRESHOLD 
          ? level
          : 0;
}

//  **************************************************************************
/// Filters the left thumb-stick values if they do not reach a minimum
/// threshold.
/// @param level[in]: The level to test against the threshold.
/// @return           The level value if it is greater than the threshold, or 0.
///
inline 
int16_t GamePad::FilterLeftThumbStickThreshold(int16_t level) const
{
  return  abs(level) > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE 
          ? level
          : 0;
}

//  **************************************************************************
/// Filters the left thumb-stick values if they do not reach a minimum
/// threshold.
/// @param level[in]: The level to test against the threshold.
/// @return           The level value if it is greater than the threshold, or 0.
///
inline 
int16_t GamePad::FilterRightThumbStickThreshold(int16_t level) const
{
  return  abs(level) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 
          ? level
          : 0;
}

//  **************************************************************************
/// Indicates if the current vk code is a Left Trigger command.
/// @param vkCode[in]: Virtual Key code to test.
/// @return             true  The code is the left trigger.
///                     false The code is not the left trigger.
///
inline 
bool GamePad::IsVkLeftTrigger(KeyId vkCode)
{
  return vkCode == vk_padLTrigger;
}

//  **************************************************************************
/// Indicates if the current vk code is a Right Trigger command.
/// @param  vkCode[in]: Virtual Key code to test.
/// @return   true  The code is the right trigger.
///           false The code is not the right trigger.
///
inline 
bool GamePad::IsVkRightTrigger(KeyId vkCode)
{
  return vkCode == vk_padRTrigger;
}

//  **************************************************************************
/// Indicates if the current vk code is a Left Analog Stick.
/// @param vkCode[in]: Virtual Key code to test.
/// @return true  The code is the left analog stick.
///         false The code is not the left analog stick.
///
inline 
bool GamePad::IsVkLeftThumb(KeyId vkCode)
{
  return vkCode >= vk_padLThumbUp
      && vkCode <= vk_padLThumbDownLeft;
}

//  **************************************************************************
/// Indicates if the current vk code is a Right Analog Stick.
/// @param vkCode[in]: Virtual Key code to test.
/// @return true  The code is the right analog stick.
///         false The code is not the right analog stick.
///
inline 
bool GamePad::IsVkRightThumb(KeyId vkCode)
{
  return vkCode >= vk_padRThumbUp
      && vkCode <= vk_padRThumbDownLeft;
}

//  **************************************************************************
/// Set the power level of the vibration motors in the controller.
/// The values range from:
///               0.0%   or (min): no power
///               100.0% or (max): 100% power
/// 
/// @param low[in]: The power-level to set for the low frequency motor.
///       high[in]: The power-level to set for the high frequency motor.
/// @return         The connected status of the controller is returned.
///
inline 
bool GamePad::Vibrate(float low, float high)    
{ 
  // Cap the input to 1.0 to insure the motor speed calculation is in range.
  low = std::max(low, 1.0f);
  high= std::max(high, 1.0f);

  // Convert these percentages into the integer range,
  // then use the integer version to set the vibration level.
  XINPUT_VIBRATION vibeLevel;
  vibeLevel.wLeftMotorSpeed = uint16_t(low  * 65535);
  vibeLevel.wRightMotorSpeed= uint16_t(high * 65535);

  DWORD retVal = ::XInputSetState( m_ctrlId, &vibeLevel);
  Refresh();

  return ProcessResult(retVal);
}

//  **************************************************************************
/// Queries the XBox Controller Drivers for the next available event
/// generated by the specified XBox controller.
/// @param index[in]: The index of the controller to be processed:
///                     k_xboxCtrl_1
///                     k_xboxCtrl_2
///                     k_xboxCtrl_3
///                     k_xboxCtrl_4
/// @param event[out]: A buffer that will accept the details regarding the 
///                    controller event to be processed.
/// @return      true if there are more messages to be processed.
///             false if there are no other controller messages.
///
inline 
bool GetNextControllerEvent(CtrlId index, GamePad::rawdata_t &event)
{
  DWORD retVal = ::XInputGetKeystroke(index, XINPUT_FLAG_GAMEPAD, &event);
  return retVal != ERROR_EMPTY;
}

//  **************************************************************************
/// Queries the XBox Controller Drivers for the next available event
/// generated by any of the active controllers.
///
/// @param event[out]: A buffer that will accept the details regarding the 
/// controller event to be processed.
/// The index for the controller message is returned in this buffer.
/// @return  true if there are more messages to be processed.
///         false if there are no other controller messages.
///
inline 
bool GetNextControllerEvent(GamePad::rawdata_t &event)
{
  return GetNextControllerEvent(XUSER_INDEX_ANY, event);
}

typedef BasicInputDevice<GamePad>          Controller;

} // namespace xbox
} // namespace win32
} // namespace input
} // namespace pbl

#endif // _WIN32

#endif
