/// @file InputDevice.h
/// Defines a BasicInputDevice template to create a standard interface
/// for processing all types of input devices.
///
/// @copyright 2011 Paul Watt
//  ****************************************************************************
#ifndef INPUTDEVICE_H_INCLUDED
#define INPUTDEVICE_H_INCLUDED
//  Includes *******************************************************************
#include "compiler.h"
#include "InputDefs.h"

namespace pbl
{
namespace input
{

//  ****************************************************************************
/// BasicInputDevice class template provides functionality that is 
/// common to all types of input devices.
///
/// DeviceT must derive from InputDeviceBase defined below.
///
template<typename DeviceT>
class BasicInputDevice
  : public DeviceT
{
public:
  //  Typedefs *****************************************************************
  typedef typename DeviceT::rawdata_t           rawdata_t;
  typedef typename DeviceT::inputdata_t         inputdata_t;

  //  Construction *************************************************************
  BasicInputDevice()
    : DeviceT()                                 { }
  ~BasicInputDevice()                           { }

  //  Status *******************************************************************
  DeviceType GetDeviceType()                    { return k_typeHid;}


  //  Methods ******************************************************************
  void Decode(const rawdata_t &raw, inputdata_t &data)
  {
    DeviceT::Decode(raw, data);
  }

  //  **************************************************************************
  void Refresh()
  {
    DeviceT::Refresh();
  }

protected:
  //  Members ******************************************************************
#ifdef _WIN32
  bool OnRegisterDevice(RAWINPUTDEVICE &rid) 
  { 
    return DeviceT::OnRegisterDevice(rid);
  }
#endif

};

//  ****************************************************************************
/// An InputDevice object that is a general purpose implementation 
/// for the HID standard type devices.  This class itself will provide
/// common functionality such as registration implementation.
/// 
/// For specific input handling, a new object should be derived 
/// from the HIDInputDevice.
/// 
/// DeviceT must derive from InputDeviceBase defined below.
///
template<typename DeviceT>
class HIDInputDevice
  : public BasicInputDevice<DeviceT>
{
public:

  //  Construction *************************************************************
  HIDInputDevice()
    : BasicInputDevice<DeviceT>()               
  { }

  //  **************************************************************************
  bool RegisterDevice(window_t hWnd, bool isNotifyOnChange)
  {
#ifdef _WIN32
    TODO("Add some sort of traits or configuration that will allow customization of flags.");
    //       RIDEV_INPUTSINK: This will capture keyboard input even when the app does not have focus.
           
    RAWINPUTDEVICE rid;
    rid.usUsagePage = GetUsagePage(k_type); 
    rid.usUsage     = GetUsageId(k_type); 
    rid.dwFlags     =  0;

    if (isNotifyOnChange)
      rid.dwFlags  |= k_devNotify;

    rid.hwndTarget  = hWnd;

    // Call the derived version to allow it 
    // to update any flags for configuration.
    OnRegisterDevice_(rid);

    return RegisterRawInputDevices(&rid, 1, sizeof(rid)) != FALSE;
#else
# error "Platform not implemented, support required."
#endif
  }

  //  **************************************************************************
  bool UnregisterDevice(window_t hWnd)
  {
#ifdef _WIN32
    RAWINPUTDEVICE rid;
    rid.usUsagePage = GetUsagePage(k_type); 
    rid.usUsage     = GetUsageId(k_type); 
    rid.dwFlags     = k_remove;
    rid.hwndTarget  = hWnd;
    return RegisterRawInputDevices(&rid, 1, sizeof(rid)) != FALSE;
#else
# error "Platform not implemented, support required."
#endif
  }

  //  **************************************************************************
  bool EnableChangeNotify(bool isEnable)        
  { 
    return  isEnable 
            ? detail::RequestDeviceChanges_(k_type)
            : detail::CancelDeviceChangeRequest_(k_type);
  }

  bool IsNotifyChange() const                   
  { 
    return detail::IsNotifyDeviceChange_(k_type);
  }

};



//  ****************************************************************************
/// Base set of constants and defined types common to all types of 
/// input devices.
/// Specific device implementations should derive from this class
/// to provide a common set of type definitions and constants.
/// 
class InputDeviceBase
{
public:

  //  Constants ****************************************************************
#if defined(_WIN32) 
  static const uint32_t   k_appKeys       = RIDEV_APPKEYS;
  static const uint32_t   k_captureMouse  = RIDEV_CAPTUREMOUSE;
  static const uint32_t   k_exclude       = RIDEV_EXCLUDE;
  static const uint32_t   k_catchSink     = RIDEV_EXINPUTSINK;
  static const uint32_t   k_bkgndSink     = RIDEV_INPUTSINK;
  static const uint32_t   k_noHotKeys     = RIDEV_NOHOTKEYS;
  static const uint32_t   k_noLegacy      = RIDEV_NOLEGACY;
  static const uint32_t   k_pageOnly      = RIDEV_PAGEONLY;
  static const uint32_t   k_remove        = RIDEV_REMOVE;
  static const uint32_t   k_devNotify     = RIDEV_DEVNOTIFY;
#else
# error "Platform not implemented, support required."
#endif

  //  Typedefs *****************************************************************
  typedef uint32_t Id;
  typedef uint32_t Type;
  typedef uint32_t Subtype;

protected:
  //  Methods ******************************************************************
  //  Hide the destructor to prevent creation at this level.
  ~InputDeviceBase()                            
  { }
};

} // namespace input
} // namespace pbl


#endif
