/// @file InputDefs.h
/// Contains common definition code values for input related ids and events.
///
/// @copyright 2011 Paul Watt
//  ****************************************************************************
#ifndef INPUTDEFS_H_INCLUDED
#define INPUTDEFS_H_INCLUDED

#include "compiler.h"

namespace pbl
{
namespace convert
{

//  ****************************************************************************
/// Extracts the lower 2 bytes from a unsigned long value.
///
inline uint16_t LoWord(uint32_t input)
{
  return static_cast<uint16_t>(input & 0xFFFF);
}

}

namespace input
{

// Button State Enumeration Key ************************************************
enum ButtonState
{
  k_btnNone   = 0x00,
  k_btnDown   = 0x01,
  k_btnUp     = 0x02,  
  k_dblClick  = 0x03, // Only valid for mouse events.
  k_repeat    = 0x04
};

//  Typedefs *******************************************************************
#ifdef _WIN32
  typedef HWND              window_t;
  typedef HANDLE            device_t;
  typedef RID_DEVICE_INFO   DeviceInfo;
#else
# error "Platform not yet implemented"
#endif

//  Device Type Enumeration Key ************************************************
enum DeviceType
{
#ifdef _WIN32
  k_typeHid       = RIM_TYPEHID, 
  k_typeKeyboard  = RIM_TYPEKEYBOARD,
  k_typeMouse     = RIM_TYPEMOUSE,
  k_typeGamepad
#else
  k_typeHid       = 0,
  k_typeKeyboard,
  k_typeMouse,
  k_typeGamepad
#endif
};

//  Device Classification ******************************************************
typedef uint32_t UsageType;
typedef uint16_t UsagePage;
typedef uint16_t UsageId;

//  The constants below indicate: **********************************************
//  Usage Type: contains both Page and Id.
//  Usage Page: The High order word
//  Usage Id:   The Low  order word
//
const UsageType   k_pointer         = 0x00010001;
const UsageType   k_mouse           = 0x00010002;
const UsageType   k_joystick        = 0x00010004;
const UsageType   k_gamepad         = 0x00010005;
const UsageType   k_keyboard        = 0x00010006;
const UsageType   k_keypad          = 0x00010007;
const UsageType   k_multiAxisCtrl   = 0x00010008;
const UsageType   k_tabletPcCtrl    = 0x00010009;
const UsageType   k_sysCtrl         = 0x00010080;
const UsageType   k_consumerAudio   = 0x000C0001;


//  ****************************************************************************
inline 
uint16_t GetUsagePage(UsageType type)  
{ 
  return static_cast<uint16_t>(type >> 16);
}

//  ****************************************************************************
inline 
uint16_t GetUsageId(UsageType type)  
{ 
  return pbl::convert::LoWord(type);
}

//  Typedefs *******************************************************************
typedef uint32_t KeyId;
typedef uint32_t EventId;

//  Constants ******************************************************************
const EventId     k_mouseMove       = 0x00;

const KeyId       k_leftMouseBtn    = 0x01;
const KeyId       k_rightMouseBtn   = 0x02;
const KeyId       k_middleMouseBtn  = 0x04;
const KeyId       k_x1MouseBtn      = 0x10;
const KeyId       k_x2MouseBtn      = 0x20;

#ifdef _WIN32

// Unassigned or reserved keys in windows.
// 0x07 : unassigned
// 0x0A : reserved
// 0x0B : reserved
// 0x40 : unassigned
// 0x5E : reserved
// 0x88 - 0x8F : unassigned
// 0x97 - 0x9F : unassigned
// 0xB8 - 0xB9 : reserved
// 0xC1 - 0xD7 : reserved
// 0xD8 - 0xDA : unassigned
// 0xE0 : reserved
// 0xE8 : unassigned
// 0xFF : reserved

//  Virtual Key Constant Definitions *******************************************
const KeyId       vk_lButton        = VK_LBUTTON;             // 0x01
const KeyId       vk_rButton        = VK_RBUTTON;             // 0x02
const KeyId       vk_cancel         = VK_CANCEL;              // 0x03
const KeyId       vk_mButton        = VK_MBUTTON;             // 0x04
const KeyId       vk_xButton1       = VK_XBUTTON1;            // 0x05
const KeyId       vk_xButton2       = VK_XBUTTON2;            // 0x06
const KeyId       vk_back           = VK_BACK;                // 0x08
const KeyId       vk_tab            = VK_TAB;                 // 0x09
const KeyId       vk_clear          = VK_CLEAR;               // 0x0C
const KeyId       vk_return         = VK_RETURN;              // 0x0D
const KeyId       vk_shift          = VK_SHIFT;               // 0x10
const KeyId       vk_control        = VK_CONTROL;             // 0x11
const KeyId       vk_menu           = VK_MENU;                // 0x12
const KeyId       vk_pause          = VK_PAUSE;               // 0x13
const KeyId       vk_capital        = VK_CAPITAL;             // 0x14
const KeyId       vk_kana           = VK_KANA;                // 0x15
const KeyId       vk_hangul         = VK_HANGUL;              // 0x15
const KeyId       vk_junja          = VK_JUNJA;               // 0x17
const KeyId       vk_final          = VK_FINAL;               // 0x18
const KeyId       vk_kanji          = VK_KANJI;               // 0x19
const KeyId       vk_escape         = VK_ESCAPE;              // 0x1B
const KeyId       vk_convert        = VK_CONVERT;             // 0x1C
const KeyId       vk_nonconvert     = VK_NONCONVERT;          // 0x1D
const KeyId       vk_accept         = VK_ACCEPT;              // 0x1E
const KeyId       vk_modechange     = VK_MODECHANGE;          // 0x1F
const KeyId       vk_space          = VK_SPACE;               // 0x20
const KeyId       vk_prior          = VK_PRIOR;               // 0x21
const KeyId       vk_next           = VK_NEXT;                // 0x22
const KeyId       vk_end            = VK_END;                 // 0x23
const KeyId       vk_home           = VK_HOME;                // 0x24
const KeyId       vk_left           = VK_LEFT;                // 0x25
const KeyId       vk_up             = VK_UP;                  // 0x26
const KeyId       vk_right          = VK_RIGHT;               // 0x27
const KeyId       vk_down           = VK_DOWN;                // 0x28
const KeyId       vk_select         = VK_SELECT;              // 0x29
const KeyId       vk_print          = VK_PRINT;               // 0x2A
const KeyId       vk_execute        = VK_EXECUTE;             // 0x2B
const KeyId       vk_snapshot       = VK_SNAPSHOT;            // 0x2C
const KeyId       vk_insert         = VK_INSERT;              // 0x2D
const KeyId       vk_delete         = VK_DELETE;              // 0x2E
const KeyId       vk_help           = VK_HELP;                // 0x2F
// VK_0 - VK_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
const KeyId       vk_0              = '0';                    // 0x30
const KeyId       vk_1              = '1';                    // 0x31
const KeyId       vk_2              = '2';                    // 0x32
const KeyId       vk_3              = '3';                    // 0x33
const KeyId       vk_4              = '4';                    // 0x34
const KeyId       vk_5              = '5';                    // 0x35
const KeyId       vk_6              = '6';                    // 0x36
const KeyId       vk_7              = '7';                    // 0x37
const KeyId       vk_8              = '8';                    // 0x38
const KeyId       vk_9              = '9';                    // 0x39
// VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
const KeyId       vk_a              = 'A';                    // 0x41
const KeyId       vk_b              = 'B';                    // 0x42
const KeyId       vk_c              = 'C';                    // 0x43
const KeyId       vk_d              = 'D';                    // 0x44
const KeyId       vk_e              = 'E';                    // 0x45
const KeyId       vk_f              = 'F';                    // 0x46
const KeyId       vk_g              = 'G';                    // 0x47
const KeyId       vk_h              = 'H';                    // 0x48
const KeyId       vk_i              = 'I';                    // 0x49
const KeyId       vk_j              = 'J';                    // 0x4A
const KeyId       vk_k              = 'K';                    // 0x4B
const KeyId       vk_l              = 'L';                    // 0x4C
const KeyId       vk_m              = 'M';                    // 0x4D
const KeyId       vk_n              = 'N';                    // 0x4E
const KeyId       vk_o              = 'O';                    // 0x4F
const KeyId       vk_p              = 'P';                    // 0x50
const KeyId       vk_q              = 'Q';                    // 0x51
const KeyId       vk_r              = 'R';                    // 0x52
const KeyId       vk_s              = 'S';                    // 0x53
const KeyId       vk_t              = 'T';                    // 0x54
const KeyId       vk_u              = 'U';                    // 0x55
const KeyId       vk_v              = 'V';                    // 0x56
const KeyId       vk_w              = 'W';                    // 0x57
const KeyId       vk_x              = 'X';                    // 0x58
const KeyId       vk_y              = 'Y';                    // 0x59
const KeyId       vk_z              = 'Z';                    // 0x5A

const KeyId       vk_lwin           = VK_LWIN;                // 0x5B
const KeyId       vk_rwin           = VK_RWIN;                // 0x5C
const KeyId       vk_apps           = VK_APPS;                // 0x5D
const KeyId       vk_sleep          = VK_SLEEP;               // 0x5F
const KeyId       vk_numpad0        = VK_NUMPAD0;             // 0x60
const KeyId       vk_numpad1        = VK_NUMPAD1;             // 0x61
const KeyId       vk_numpad2        = VK_NUMPAD2;             // 0x62
const KeyId       vk_numpad3        = VK_NUMPAD3;             // 0x63
const KeyId       vk_numpad4        = VK_NUMPAD4;             // 0x64
const KeyId       vk_numpad5        = VK_NUMPAD5;             // 0x65
const KeyId       vk_numpad6        = VK_NUMPAD6;             // 0x66
const KeyId       vk_numpad7        = VK_NUMPAD7;             // 0x67
const KeyId       vk_numpad8        = VK_NUMPAD8;             // 0x68
const KeyId       vk_numpad9        = VK_NUMPAD9;             // 0x69
const KeyId       vk_multiply       = VK_MULTIPLY;            // 0x6A
const KeyId       vk_add            = VK_ADD;                 // 0x6B
const KeyId       vk_separator      = VK_SEPARATOR;           // 0x6C
const KeyId       vk_subtract       = VK_SUBTRACT;            // 0x6D
const KeyId       vk_decimal        = VK_DECIMAL;             // 0x6E
const KeyId       vk_divide         = VK_DIVIDE;              // 0x6F
const KeyId       vk_f1             = VK_F1;                  // 0x70
const KeyId       vk_f2             = VK_F2;                  // 0x71
const KeyId       vk_f3             = VK_F3;                  // 0x72
const KeyId       vk_f4             = VK_F4;                  // 0x73
const KeyId       vk_f5             = VK_F5;                  // 0x74
const KeyId       vk_f6             = VK_F6;                  // 0x75
const KeyId       vk_f7             = VK_F7;                  // 0x76
const KeyId       vk_f8             = VK_F8;                  // 0x77
const KeyId       vk_f9             = VK_F9;                  // 0x78
const KeyId       vk_f10            = VK_F10;                 // 0x79
const KeyId       vk_f11            = VK_F11;                 // 0x7A
const KeyId       vk_f12            = VK_F12;                 // 0x7B
const KeyId       vk_f13            = VK_F13;                 // 0x7C
const KeyId       vk_f14            = VK_F14;                 // 0x7D
const KeyId       vk_f15            = VK_F15;                 // 0x7E
const KeyId       vk_f16            = VK_F16;                 // 0x7F
const KeyId       vk_f17            = VK_F17;                 // 0x80
const KeyId       vk_f18            = VK_F18;                 // 0x81
const KeyId       vk_f19            = VK_F19;                 // 0x82
const KeyId       vk_f20            = VK_F20;                 // 0x83
const KeyId       vk_f21            = VK_F21;                 // 0x84
const KeyId       vk_f22            = VK_F22;                 // 0x85
const KeyId       vk_f23            = VK_F23;                 // 0x86
const KeyId       vk_f24            = VK_F24;                 // 0x87
const KeyId       vk_numlock        = VK_NUMLOCK;             // 0x90
const KeyId       vk_scroll         = VK_SCROLL;              // 0x91

//
// VK_L* & VK_R* - left and right Alt, Ctrl and Shift virtual keys.
// Used only as parameters to GetAsyncKeyState() and GetKeyState().
// No other API or message will distinguish left and right keys in this way.
//
const KeyId       vk_lShift         = VK_LSHIFT;              // 0xA0
const KeyId       vk_rShift         = VK_RSHIFT;              // 0xA1
const KeyId       vk_lControl       = VK_LCONTROL;            // 0xA2
const KeyId       vk_rControl       = VK_RCONTROL;            // 0xA3
const KeyId       vk_lMenu          = VK_LMENU;               // 0xA4
const KeyId       vk_rMenu          = VK_RMENU;               // 0xA5
const KeyId       vk_browserBack    = VK_BROWSER_BACK;        // 0xA6
const KeyId       vk_browserForward = VK_BROWSER_FORWARD;     // 0xA7
const KeyId       vk_browserRefresh = VK_BROWSER_REFRESH;     // 0xA8
const KeyId       vk_browserStop    = VK_BROWSER_STOP;        // 0xA9
const KeyId       vk_browserSearch  = VK_BROWSER_SEARCH;      // 0xAA
const KeyId       vk_browserFav     = VK_BROWSER_FAVORITES;   // 0xAB
const KeyId       vk_browserHome    = VK_BROWSER_HOME;        // 0xAC
const KeyId       vk_volumeMute     = VK_VOLUME_MUTE;         // 0xAD
const KeyId       vk_volumeDown     = VK_VOLUME_DOWN;         // 0xAE
const KeyId       vk_volumeUp       = VK_VOLUME_UP;           // 0xAF
const KeyId       vk_mediaNextTrack = VK_MEDIA_NEXT_TRACK;    // 0xB0
const KeyId       vk_mediaPrevTrack = VK_MEDIA_PREV_TRACK;    // 0xB1
const KeyId       vk_mediaStop      = VK_MEDIA_STOP;          // 0xB2
const KeyId       vk_mediaPause     = VK_MEDIA_PLAY_PAUSE;    // 0xB3
const KeyId       vk_launchMail     = VK_LAUNCH_MAIL;         // 0xB4
const KeyId       vk_launchMediaSel = VK_LAUNCH_MEDIA_SELECT; // 
const KeyId       vk_launchApp1     = VK_LAUNCH_APP1;         // 0xB6
const KeyId       vk_launchApp2     = VK_LAUNCH_APP2;         // 0xB7
const KeyId       vk_oem1           = VK_OEM_1;               // 0xBA   // ';:' for US
const KeyId       vk_colon          = VK_OEM_1;               // VK_OEM_1
const KeyId       vk_plus           = VK_OEM_PLUS;            // VK_OEM_PLUS
const KeyId       vk_comma          = VK_OEM_COMMA;           // VK_OEM_COMMA
const KeyId       vk_minus          = VK_OEM_MINUS;           // 0xBD   // '-' any country
const KeyId       vk_period         = VK_OEM_PERIOD;          // 0xBE   // '.' any country
const KeyId       vk_oem2           = VK_OEM_2;               // 0xBF   // '/?' for US
const KeyId       vk_fSlash         = VK_OEM_2;               // VK_OEM_2
const KeyId       vk_question       = VK_OEM_2;               // VK_OEM_2
const KeyId       vk_oem3           = VK_OEM_3;               // 0xC0   // '`~' for US
const KeyId       vk_tilde          = VK_OEM_3;               // VK_OEM_3
const KeyId       vk_oem4           = VK_OEM_4;               // 0xDB  //  '[{' for US
const KeyId       vk_lParen         = VK_OEM_4;               // VK_OEM_4
const KeyId       vk_oem5           = VK_OEM_5;               // 0xDC  //  '\|' for US
const KeyId       vk_bSlash         = VK_OEM_5;               // VK_OEM_5
const KeyId       vk_pipe           = VK_OEM_5;               // VK_OEM_5
const KeyId       vk_oem6           = VK_OEM_6;               // 0xDD  //  ']}' for US
const KeyId       vk_rParen         = VK_OEM_6;               // VK_OEM_6
const KeyId       vk_oem7           = VK_OEM_7;               // 0xDE  //  ''"' for US
const KeyId       vk_quote          = VK_OEM_7;               // VK_OEM_7
const KeyId       vk_oem8           = VK_OEM_8;               // 0xDF

//  OEM Keyboard Definitions ***************************************************
// NEC PC-9800 kbd definitions
//const KeyId       vk_ = VK_OEM_NEC_EQUAL;         // 0x92   // '=' key on numpad
//// Fujitsu/OASYS kbd definitions
//const KeyId       vk_ = VK_OEM_FJ_JISHO;          // 0x92   // 'Dictionary' key
//const KeyId       vk_ = VK_OEM_FJ_MASSHOU;        // // 'Unregister word' key
//const KeyId       vk_ = VK_OEM_FJ_TOUROKU;        // // 'Register word' key
//const KeyId       vk_ = VK_OEM_FJ_LOYA;           // 0x95   // 'Left OYAYUBI' key
//const KeyId       vk_ = VK_OEM_FJ_ROYA;           // 0x96   // 'Right OYAYUBI' key
//// Various extended or enhanced keyboards
//const KeyId       vk_ = VK_OEM_AX;                // 0xE1   //  'AX' key on Japanese AX kbd
//const KeyId       vk_ = VK_OEM_102;               // 0xE2   //  "<>" or "\|" on RT 102-key kbd.
//const KeyId       vk_ = VK_ICO_HELP;              // 0xE3   //  Help key on ICO
//const KeyId       vk_ = VK_ICO_00;                // 0xE4   //  00 key on ICO
//const KeyId       vk_ = VK_PROCESSKEY;            // 0xE5
//const KeyId       vk_ = VK_ICO_CLEAR;             // 0xE6
//const KeyId       vk_ = VK_PACKET;                // 0xE7
//// Nokia/Ericsson definitions
//const KeyId       vk_ = VK_OEM_RESET;             // 0xE9
//const KeyId       vk_ = VK_OEM_JUMP;              // 0xEA
//const KeyId       vk_ = VK_OEM_PA1;               // 0xEB
//const KeyId       vk_ = VK_OEM_PA2;               // 0xEC
//const KeyId       vk_ = VK_OEM_PA3;               // 0xED
//const KeyId       vk_ = VK_OEM_WSCTRL;            // 0xEE
//const KeyId       vk_ = VK_OEM_CUSEL;             // 0xEF
//const KeyId       vk_ = VK_OEM_ATTN;              // 0xF0
//const KeyId       vk_ = VK_OEM_FINISH;            // 0xF1
//const KeyId       vk_ = VK_OEM_COPY;              // 0xF2
//const KeyId       vk_ = VK_OEM_AUTO;              // 0xF3
//const KeyId       vk_ = VK_OEM_ENLW;              // 0xF4
//const KeyId       vk_ = VK_OEM_BACKTAB;           // 0xF5
//const KeyId       vk_ = VK_ATTN;                  // 0xF6
//const KeyId       vk_ = VK_CRSEL;                 // 0xF7
//const KeyId       vk_ = VK_EXSEL;                 // 0xF8
//const KeyId       vk_ = VK_EREOF;                 // 0xF9
//const KeyId       vk_ = VK_PLAY;                  // 0xFA
//const KeyId       vk_ = VK_ZOOM;                  // 0xFB
//const KeyId       vk_ = VK_NONAME;                // 0xFC
//const KeyId       vk_ = VK_PA1;                   // 0xFD
//const KeyId       vk_ = VK_OEM_CLEAR;             // 0xFE

#else
# error Virtual Key codes have not been defined for this platform.
#endif

namespace detail
{
  extern bool RequestDeviceChanges        (UsageType type);
  extern bool CancelDeviceChangeRequest   (UsageType type);
  extern bool IsNotifyDeviceChange        (UsageType type);
} // namespace detail
} // namespace input
} // namespace pbl

#endif
