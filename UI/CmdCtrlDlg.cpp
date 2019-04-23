/// @file CmdCtrlDlg.cpp 
///
/// Implements the Command and Control portion of the application that 
/// defines the interaction and feedback with the drone.
///
//  ****************************************************************************
//  Includes *******************************************************************


#include "../stdafx.h"
#include "ui_def.h"

#include "CmdCtrlDlg.h"
#include "../resource.h"
#include "AutoGdi.h"
#include "aa_ellipse.h"

#include <Ws2tcpip.h>
#include <sstream>

#if defined(_UNICODE)
typedef std::wstringstream    tstringstream;

#else
typedef std::stringstream     tstringstream;

#endif

// Forward declaration.
void UpdatePIDStatus();

// Convenience functions to return stock brushes and pens requested often.
HBRUSH GetBlackBrush()    { return HBRUSH(::GetStockObject(BLACK_BRUSH));}
HBRUSH GetWhiteBrush()    { return HBRUSH(::GetStockObject(WHITE_BRUSH));}
HBRUSH GetNullBrush()     { return HBRUSH(::GetStockObject(NULL_BRUSH));}
HPEN   GetBlackPen()      { return HPEN  (::GetStockObject(BLACK_PEN));}
HPEN   GetWhitePen()      { return HPEN  (::GetStockObject(WHITE_PEN));}

const
COLORREF k_empty_color = RGB(0x50,0x60,0x75);

COLORREF ScaleColor(COLORREF color, BYTE level);

void SetDoubleValue (double input, HWND hDlg, int id);

/* Default Contstructor ******************************************************/
CmdCtrlDlg::CmdCtrlDlg()
  : m_commanded({0})
  , m_state({0})
  , m_control_mode(angle_control)
  , m_disable_roll(false)
  , m_disable_pitch(false)
  , m_disable_yaw(false)
  , m_use_roll_signal(true)
  , m_use_pitch_signal(true)
  , m_use_yaw_signal(true)
{ }

//  ****************************************************************************
// Message handler for Command and control of the drone.
INT_PTR CALLBACK CmdCtrlDlg::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  CmdCtrlDlg *pThis = NULL;
  switch (message)
  {
  case WM_INITDIALOG:
    {
      // Assign the input parameter pointer to user data.
      ::SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
      // The lparam is our "this" pointer.
      pThis = (CmdCtrlDlg*)lParam;
      if (pThis)
      {
        pThis->m_hDlg = hDlg;
      }
    }
    break;
  default:
    // Get the pointer to "this" object for processing.
    LONG_PTR lVal = ::GetWindowLongPtr(hDlg, GWLP_USERDATA);
    pThis = (CmdCtrlDlg*)lVal;
  }

  // Process the message if a proper object was extracted.
  if (pThis)
  {
    return pThis->MessageHandler(hDlg, message, wParam, lParam);
  }

  return (INT_PTR)FALSE;
}


//  ****************************************************************************
INT_PTR CmdCtrlDlg::MessageHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  UNREFERENCED_PARAMETER(lParam);
  switch (message)
  {
  case WM_INITDIALOG:
    {
      CreateFonts();
      InitControls();
    }
    return (INT_PTR)TRUE;


  case WM_COMMAND:
    {
    
      if (HIWORD(wParam) == BN_CLICKED)
      {
        switch (wParam)
        {
        case IDC_ANGLE_CONTROL:
          m_control_mode = angle_control;
          ModifyControlMode(m_control_mode, 
                            !m_disable_roll, 
                            !m_disable_pitch, 
                            !m_disable_yaw);
          break;
        case IDC_RATE_CONTROL:
          m_control_mode = rate_control;
          ModifyControlMode(m_control_mode, 
                            !m_disable_roll, 
                            !m_disable_pitch, 
                            !m_disable_yaw);
          break;


        case IDC_USER_INPUT_SIGNAL:
          g_control_signal = k_user_input_signal;
          EnableCommandControls(false);
          break;
        case IDC_SQUARE_WAVE_SIGNAL:
          g_control_signal = k_square_wave_signal;
          EnableCommandControls(true);
          break;
        case IDC_SINE_WAVE_SIGNAL:
          g_control_signal = k_sine_wave_signal;
          EnableCommandControls(true);
          break;
        case IDC_IMPULSE:
          g_control_signal = k_one_sec_impulse;
          EnableCommandControls(true);
          break;


        case IDC_CONTROL_ROLL:
          m_disable_roll = BST_UNCHECKED == IsDlgButtonChecked(hDlg, IDC_CONTROL_ROLL);
          ModifyControlMode(m_control_mode, 
                            !m_disable_roll, 
                            !m_disable_pitch, 
                            !m_disable_yaw);
          break;
        case IDC_CONTROL_PITCH:
          m_disable_pitch = BST_UNCHECKED == IsDlgButtonChecked(hDlg, IDC_CONTROL_PITCH);
          ModifyControlMode(m_control_mode, 
                            !m_disable_roll, 
                            !m_disable_pitch, 
                            !m_disable_yaw);
          break;
        case IDC_CONTROL_YAW:
          m_disable_yaw = BST_UNCHECKED == IsDlgButtonChecked(hDlg, IDC_CONTROL_YAW);
          ModifyControlMode(m_control_mode, 
                            !m_disable_roll, 
                            !m_disable_pitch, 
                            !m_disable_yaw);
          break;

        case IDC_ROLL_SIGNAL:
          m_use_roll_signal = BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_ROLL_SIGNAL);
          ModifyControlSignal(m_use_roll_signal, 
                              m_use_pitch_signal, 
                              m_use_yaw_signal);
          break;
        case IDC_PITCH_SIGNAL:
          m_use_pitch_signal = BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_PITCH_SIGNAL);
          ModifyControlSignal(m_use_roll_signal, 
                              m_use_pitch_signal, 
                              m_use_yaw_signal);
         break;
        case IDC_YAW_SIGNAL:
          m_use_yaw_signal = BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_YAW_SIGNAL);
          ModifyControlSignal(m_use_roll_signal, 
                              m_use_pitch_signal, 
                              m_use_yaw_signal);
          break;

        case IDC_ARM_DRONE:
          ArmDrone();
          break;


        case IDC_APPLY_LIMITS:
          ApplyLimits();
          break;
        }
      }
    }
    return (INT_PTR)TRUE;


  case QC_DRONE_CONNECTED:
    DroneConnected((uint32_t)wParam);
    return (INT_PTR)TRUE;

  case QC_DRONE_ARMED:
    DroneArmed();
    return (INT_PTR)TRUE;

  case QC_PROCESS_INPUT:
    ProcessInput();
    return (INT_PTR)TRUE;
  
  case QC_UPDATE_STATUS:
    UpdateStatus();
    UpdatePIDStatus();
    return (INT_PTR)TRUE;
  
  case QC_SEND_COMMAND_ERROR:
    // TODO: Add an error reporting mechanism to the display
    return (INT_PTR)TRUE;
  
  case WM_ERASEBKGND:
    {
      HDC  hdc = (HDC)wParam;
      ProcessEraseBkgnd(hDlg, hdc);
    }
    return (INT_PTR)TRUE;

  case WM_CTLCOLORDLG:
  case WM_CTLCOLORSTATIC:
    return (INT_PTR)GetWhiteBrush();

  case WM_DRAWITEM:
    {
      DRAWITEMSTRUCT *pDrawInfo = (DRAWITEMSTRUCT*)lParam;
      if (ProcessDrawItem(hDlg, *pDrawInfo))
        return (INT_PTR)TRUE;
    }
    break;
  }

  return (INT_PTR)FALSE;
}


/*****************************************************************************/
void CmdCtrlDlg::EnableCommandControls(bool is_enable)
{
  EnableWindow(GetDlgItem(m_hDlg, IDC_CONTROL_PERIOD_LABEL), is_enable);
  EnableWindow(GetDlgItem(m_hDlg, IDC_CONTROL_PERIOD),       is_enable);
  EnableWindow(GetDlgItem(m_hDlg, IDC_CONTROL_SCALAR_LABEL), is_enable);
  EnableWindow(GetDlgItem(m_hDlg, IDC_CONTROL_SCALAR),       is_enable);
}

/*****************************************************************************/
void CmdCtrlDlg::CreateFonts()
{
  LOGFONT lf; 
  ::memset(&lf, 0, sizeof(LOGFONT));

  HDC hdc = ::GetDC(NULL);
  
  int ptSize = 14;
  lf.lfWeight = FW_BOLD;
  lf.lfHeight = -MulDiv(ptSize, ::GetDeviceCaps(hdc, LOGPIXELSY), 72);
  ::_tcscpy(lf.lfFaceName, _T("Tahoma"));
  m_tahoma0.Attach(CreateFontIndirect(&lf));

  ::SendDlgItemMessage(m_hDlg, IDC_STATUS,      WM_SETFONT, WPARAM(HFONT(m_tahoma0)), 0);
  ::SendDlgItemMessage(m_hDlg, IDC_IP_ADDRESS,  WM_SETFONT, WPARAM(HFONT(m_tahoma0)), 0);

  ::SendDlgItemMessage(m_hDlg, IDC_MOTOR_A, WM_SETFONT, WPARAM(HFONT(m_tahoma0)), 0);
  ::SendDlgItemMessage(m_hDlg, IDC_MOTOR_B, WM_SETFONT, WPARAM(HFONT(m_tahoma0)), 0);
  ::SendDlgItemMessage(m_hDlg, IDC_MOTOR_C, WM_SETFONT, WPARAM(HFONT(m_tahoma0)), 0);
  ::SendDlgItemMessage(m_hDlg, IDC_MOTOR_D, WM_SETFONT, WPARAM(HFONT(m_tahoma0)), 0);
  ::SendDlgItemMessage(m_hDlg, IDC_MOTOR_E, WM_SETFONT, WPARAM(HFONT(m_tahoma0)), 0);
  ::SendDlgItemMessage(m_hDlg, IDC_MOTOR_F, WM_SETFONT, WPARAM(HFONT(m_tahoma0)), 0);

  ::ReleaseDC(NULL, hdc);
}

/*****************************************************************************/
void CmdCtrlDlg::InitControls()
{
  m_orb_A.SetColorProfile(k_propOrangeOrb);
  m_orb_A.EnableClick(false);
  m_orb_A.EnableFocus(false);
  m_orb_A.SetText(_T("0"));

  m_orb_B.SetColorProfile(k_blueOrb);
  m_orb_B.EnableClick(false);
  m_orb_B.EnableFocus(false);
  m_orb_B.SetText(_T("0"));

  m_orb_C.SetColorProfile(k_blueOrb);
  m_orb_C.EnableClick(false);
  m_orb_C.EnableFocus(false);
  m_orb_C.SetText(_T("0"));

  m_orb_D.SetColorProfile(k_blueOrb);
  m_orb_D.EnableClick(false);
  m_orb_D.EnableFocus(false);
  m_orb_D.SetText(_T("0"));

  m_orb_E.SetColorProfile(k_blueOrb);
  m_orb_E.EnableClick(false);
  m_orb_E.EnableFocus(false);
  m_orb_E.SetText(_T("0"));

  m_orb_F.SetColorProfile(k_propOrangeOrb);
  m_orb_F.EnableClick(false);
  m_orb_F.EnableFocus(false);
  m_orb_F.SetText(_T("0"));

  //// Initialize all of the color fields.
  UpdateDisplay(IDC_COMMANDED_ROLL);
  UpdateDisplay(IDC_ACTUAL_ROLL);
  UpdateDisplay(IDC_COMMANDED_PITCH);
  UpdateDisplay(IDC_ACTUAL_PITCH);
  UpdateDisplay(IDC_COMMANDED_YAW);
  UpdateDisplay(IDC_ACTUAL_YAW);
  UpdateDisplay(IDC_COMMANDED_THRUST);
  UpdateDisplay(IDC_ACTUAL_THRUST);

  UpdateCommandStatus();

  SetDoubleValue(g_control_signal_period, m_hDlg, IDC_CONTROL_PERIOD);
  SetDoubleValue(g_control_signal_scalar, m_hDlg, IDC_CONTROL_SCALAR);

}

/*****************************************************************************/
void CmdCtrlDlg::ProcessEraseBkgnd(HWND hDlg, HDC hdc)
{
  article::AutoSaveDC checkPoint(hdc);

  const int k_nudgeSrcX =  2;
  const int k_nudgeSrcY =  2;
  const int k_nudgeDstX =  3;
  const int k_nudgeDstY =  2;

  // Clear the canvas back to a white slate.
  RECT  client;
  ::GetClientRect(hDlg, &client);
  ::FillRect(hdc, &client, GetWhiteBrush());

  AutoPen thinPen   (::CreatePen(PS_SOLID, 5, k_black));
  AutoPen outlinePen   (::CreatePen(PS_SOLID, 7, k_black));


  // The window rect for the dialog is used for 
  // calculating the offsets of its child controls.
  RECT dlgRect;
  ::GetWindowRect(hDlg, &dlgRect);


  ::SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE);
}

/*****************************************************************************/
bool CmdCtrlDlg::ProcessDrawItem(HWND hDlg, DRAWITEMSTRUCT& drawInfo)
{
  // Some of the controls can be filtered based on specific values:
  UINT id     = drawInfo.CtlID;
  UINT idMask = 0;
  HDC  hdc    = drawInfo.hDC;
  RECT &rc    = drawInfo.rcItem;

  switch (id)
  {
  case IDC_ACTUAL_ROLL:
    PaintRangeBar(hdc, rc, 
                  m_state.orientation.roll, 
                  RollColor(m_state.orientation.roll), 
                  false, false);
    break;
  case IDC_COMMANDED_ROLL:
    PaintRangeBar(hdc, rc, 
                  m_commanded.roll, 
                  RollColor(m_commanded.roll), 
                  false, false);
    break;
  case IDC_ACTUAL_PITCH:
    PaintRangeBar(hdc, rc, 
                  m_state.orientation.pitch, 
                  PitchColor(m_state.orientation.pitch), 
                  true, true);
    break;
  case IDC_COMMANDED_PITCH:
    PaintRangeBar(hdc, rc, 
                  m_commanded.pitch, 
                  PitchColor(m_commanded.pitch), 
                  true, true);
    break;
  case IDC_ACTUAL_YAW:
    PaintRangeBar(hdc, rc, 
                  m_state.orientation.yaw, 
                  YawColor(m_state.orientation.yaw), 
                  false, false);
    break;
  case IDC_COMMANDED_YAW:
    PaintRangeBar(hdc, rc, 
                  m_commanded.yaw, 
                  YawColor(m_commanded.yaw), 
                  false, false);
    break;
  case IDC_COMMANDED_THRUST:
    PaintRangeBar(hdc, rc, 
                  GetCompositeThrust(), 
                  ThrustColor(GetCompositeThrust()), 
                  true, false);
    break;
  default:
    idMask = 0xFF;
  }

  if (idMask)
  {
    // Update the color of the orb to correspond with
    // the level of activity commanded of the motor.
    article::OrbButton::ColorProfile profile;
    
    // The control has not been handled yet.
    // Determine if the control fits into one of the button types.
    id = id & idMask;


    BYTE scale = 0;
    switch (id)
    {
    case 0x10:
      profile = k_propOrangeOrb;

      profile.m_shadow = ScaleColor(RGB(255,255,0), m_state.motor.A >> 8);

      m_orb_A.SetColorProfile(profile);
      m_orb_A.OnDrawBtn(drawInfo);
      break;
    case 0x20:
      profile = k_blueOrb;

      profile.m_shadow = ScaleColor(RGB(255,255,0), m_state.motor.B >> 8);

      m_orb_B.SetColorProfile(profile);
      m_orb_B.OnDrawBtn(drawInfo);
      break;
    case 0x30:
      profile = k_blueOrb;

      profile.m_shadow = ScaleColor(RGB(0,255,255), m_state.motor.C >> 8);

      m_orb_C.SetColorProfile(profile);
      m_orb_C.OnDrawBtn(drawInfo);
      break;
    case 0x40:
      profile = k_blueOrb;

      profile.m_shadow = ScaleColor(RGB(0,255,255), m_state.motor.D >> 8);

      m_orb_D.SetColorProfile(profile);
      m_orb_D.OnDrawBtn(drawInfo);
      break;
    case 0x50:
      profile = k_blueOrb;

      profile.m_shadow = ScaleColor(RGB(0,255,255), m_state.motor.E >> 8);

      m_orb_E.SetColorProfile(profile);
      m_orb_E.OnDrawBtn(drawInfo);
      break;
    case 0x60:
      profile = k_propOrangeOrb;

      profile.m_shadow = ScaleColor(RGB(0,255,255), m_state.motor.F >> 8);

      m_orb_F.SetColorProfile(profile);
      m_orb_F.OnDrawBtn(drawInfo);
      break;
    default:
      idMask = 0xFFFFFFFF;
    }

  }

  return idMask != 0xFFFFFFFF;
}


/*****************************************************************************/
void CmdCtrlDlg::PaintRangeBar(HDC hdc, const RECT& rc, int16_t value, COLORREF color, bool isVertical, bool isReversed)
{
  AutoSaveDC checkPoint(hdc);

  if (isReversed)
  {
    value *= -1;
  }

  // Create a border with a bit of a highlight representing the channel color.
  SIZE sz = article::GetRectSize(rc);

  RECT lowRc = rc;
  RECT highRc= rc;

  if (isVertical)
  {
    lowRc.top     += sz.cy / 2;
    highRc.bottom -= sz.cy / 2;
    article::RectGradient(hdc, lowRc, k_empty_color, k_black, isVertical);
    article::RectGradient(hdc, highRc, k_black, k_empty_color, isVertical);
  }
  else
  {
    lowRc.right   -= sz.cx / 2;
    highRc.left   += sz.cx / 2; 
    article::RectGradient(hdc, lowRc, k_black, k_empty_color, isVertical);
    article::RectGradient(hdc, highRc, k_empty_color, k_black, isVertical);
  }

  AutoPen pen(::CreatePen(PS_SOLID, 1, color));
  ::SelectObject(hdc, pen);
  ::SelectObject(hdc, GetNullBrush());
  ::Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);

  RECT gradRc = (value < 0)
              ? lowRc
              : highRc;

  ::InflateRect(&gradRc, -2, -2);


  COLORREF low  = k_empty_color;
  COLORREF high = k_empty_color;

  // Paint the other half, based on the sign of the value.
  if (isVertical)
  {
    if (value < 0)
    {
      high = color;
    }
    else
    {
      low = color;
    }
  }
  else
  {
    if (value < 0)
    {
      low = color;
    }
    else
    {
      high = color;
    }
  }

  article::RectGradient(hdc, gradRc, low, high, isVertical);

  PaintIndicator(hdc, gradRc, value, k_black);
}

/******************************************************************************
Date:       8/23/2011
Purpose:    Paint an indicator line for the current level.
            The larger number is on the left, and it counts down.
            This is to help visualize giving a percentage of the pixel value 
            to the destination color on the right side.
Parameters: hdc
            rc
            alphaLevel
            color
*******************************************************************************/
void CmdCtrlDlg::PaintIndicator(HDC hdc, const RECT &rc, int16_t level, COLORREF color)
{
  RECT statusRect = rc;
  SIZE  sz = article::GetRectSize(statusRect);

  // Calculate the orientation of the meter, 
  // to determine the orientation of the indicator.
  bool isHorz = sz.cx > sz.cy;
  LONG &len = isHorz
              ? sz.cx
              : sz.cy;
  float normalized = level / 32767.0f;

  LONG  pos;
  
  if (isHorz)
  {
    pos = (normalized < 0)
        ? statusRect.right + LONG(len * normalized)
        : statusRect.left  + LONG(len * normalized);
  }
  else 
  {
    pos = (normalized < 0)
        ? statusRect.top    - LONG(len * normalized)
        : statusRect.bottom - LONG(len * normalized);
  }

  RECT rcInd;
  SIZE ind;
  if (isHorz)
  {
    ::SetRect(&rcInd, pos-2, statusRect.top, pos+3, statusRect.bottom);
    ind.cx = 2;
    ind.cy = 1;
  }
  else
  {
    ::SetRect(&rcInd, statusRect.left, pos-2, statusRect.right, pos+3);
    ind.cx = 1;
    ind.cy = 2;
  }

  article::AutoRgn indicator(::CreateRectRgnIndirect(&rcInd));
  ::FrameRgn(hdc, indicator, AutoBrush(::CreateSolidBrush(color)), ind.cx, ind.cy);
}

/*****************************************************************************/
void CmdCtrlDlg::UpdateDisplay(UINT id)
{
  UINT  textId = 0;

  tstringstream formatted;

  formatted.precision(4);

  // Apply the value to the appropriate color channel.
  switch (id)
  {
  case IDC_COMMANDED_ROLL:
    formatted << RollAngle(m_commanded.roll) << "°";
    textId = IDC_CMD_ROLL_VALUE;
    break;
  case IDC_ACTUAL_ROLL:
    formatted << RollOrientation(m_state.orientation.roll) << "°";
    textId = IDC_ACTUAL_ROLL_VALUE;
    break;
  case IDC_COMMANDED_PITCH:
    formatted << PitchAngle(m_commanded.pitch) << "°";
    textId = IDC_CMD_PITCH_VALUE;
    break;
  case IDC_ACTUAL_PITCH:
    formatted << PitchOrientation(m_state.orientation.pitch) << "°";
    textId = IDC_ACTUAL_PITCH_VALUE;
    break;
  case IDC_COMMANDED_YAW:
    formatted << YawAngle(m_commanded.yaw) << "°/s";
    textId = IDC_CMD_YAW_VALUE;
    break;
  case IDC_ACTUAL_YAW:
    formatted << YawOrientation(m_state.orientation.yaw) << "°";
    textId = IDC_ACTUAL_YAW_VALUE;
    break;
  case IDC_COMMANDED_THRUST:
    formatted << ThrustPercentage(GetCompositeThrust()) << "%";
    textId = IDC_CMD_THRUST_VALUE;
    break;

  // These items have an empty special processing,
  // but the last lines complete the status update.
  case IDC_MOTOR_A:
    break;
  case IDC_MOTOR_B:
    break;
  case IDC_MOTOR_C:
    break;
  case IDC_MOTOR_D:
    break;
  case IDC_MOTOR_E:
    break;
  case IDC_MOTOR_F:
    break;

  case IDC_BASE_LATITUDE:
    formatted << GetBaseLatitude();
    textId = IDC_BASE_LATITUDE;
    break;
  case IDC_BASE_LONGITUDE:
    formatted << GetBaseLongitude();
    textId = IDC_BASE_LONGITUDE;
    break;
  case IDC_BASE_ALTITUDE:
    formatted << GetBaseAltitude();
    textId = IDC_BASE_ALTITUDE;
    break;
  case IDC_CURRENT_LATITUDE:
    formatted << GetCurrentLatitude();
    textId = IDC_CURRENT_LATITUDE;
    break;
  case IDC_CURRENT_LONGITUDE:
    formatted << GetCurrentLongitude();
    textId = IDC_CURRENT_LONGITUDE;
    break;
  case IDC_CURRENT_ALTITUDE:
    formatted << GetCurrentAltitude();
    textId = IDC_CURRENT_ALTITUDE;
    break;
  case IDC_CURRENT_RANGE:
    formatted << GetRange();
    textId = IDC_CURRENT_RANGE;
    break;


  case IDC_STATUS:
    SetDlgItemText(m_hDlg, 
                   IDC_STATUS, 
                   IsConnected() ? _T("Connected") : _T("Not Connected"));
    break;
  default:
    // The value did not match any of the selectable color channels.
    return;
  }

  ::SetDlgItemText(m_hDlg, textId, formatted.str().c_str());
  ::InvalidateRect(::GetDlgItem(m_hDlg, id), 0, TRUE);
}

//  ****************************************************************************
void CmdCtrlDlg::DroneConnected(uint32_t ip_addr)
{
  ::SetDlgItemText(m_hDlg, IDC_STATUS, _T("Connected:"));
  ::InvalidateRect(GetDlgItem(m_hDlg, IDC_STATUS), 0, TRUE);

  if (0 == ip_addr)
  {
    ::SetDlgItemText(m_hDlg, IDC_IP_ADDRESS, _T("Serial"));
  }
  else
  {
    // now get it back and print it
    in_addr addr;
    addr.S_un.S_addr = ip_addr;

    TCHAR buffer[20];
    InetNtop(AF_INET, &ip_addr, buffer, 20);

    ::SetDlgItemText(m_hDlg, IDC_IP_ADDRESS, buffer);
  }
}

//  ****************************************************************************
void CmdCtrlDlg::DroneDisconnected()
{
  ::SetDlgItemText(m_hDlg, IDC_STATUS, _T("Disconnected:"));
  ::InvalidateRect(GetDlgItem(m_hDlg, IDC_STATUS), 0, TRUE);

  ::SetDlgItemText(m_hDlg, IDC_IP_ADDRESS, _T(""));

  ::SetDlgItemText(m_hDlg, IDC_ARM_DRONE, _T("Arm"));

}

//  ****************************************************************************
void CmdCtrlDlg::DroneArmed()
{
  ::SetDlgItemText(m_hDlg, IDC_STATUS, _T("Armed:"));
  ::InvalidateRect(GetDlgItem(m_hDlg, IDC_STATUS), 0, TRUE);

  ::SetDlgItemText(m_hDlg, IDC_ARM_DRONE, _T("Halt"));

  UpdateDisplay(IDC_BASE_LATITUDE);
  UpdateDisplay(IDC_BASE_LONGITUDE);
  UpdateDisplay(IDC_BASE_ALTITUDE);

}

//  ****************************************************************************
void CmdCtrlDlg::ProcessInput()
{
  QCopter last = m_commanded;
  GetCommandedInput(m_commanded);

  // Invalidate the commanded controls so they are repainted.
  if (last.roll != m_commanded.roll)
  {
    UpdateDisplay(IDC_COMMANDED_ROLL);
  }

  if (last.pitch != m_commanded.pitch)
  {
    UpdateDisplay(IDC_COMMANDED_PITCH);
  }

  if (last.yaw != m_commanded.yaw)
  {
    UpdateDisplay(IDC_COMMANDED_YAW);
  }

  if (last.thrust != GetCompositeThrust())
  {
    UpdateDisplay(IDC_COMMANDED_THRUST);
  }

}

//  ****************************************************************************
void CmdCtrlDlg::UpdateStatus()
{
  DroneState    last = m_state;

  GetDroneState(m_state);

  // Invalidate the status controls so they are repainted.
  if (last.orientation.roll != m_state.orientation.roll)
  {
    UpdateDisplay(IDC_ACTUAL_ROLL);
  }

  if (last.orientation.pitch != m_state.orientation.pitch)
  {
    UpdateDisplay(IDC_ACTUAL_PITCH);
  }

  if (last.orientation.yaw != m_state.orientation.yaw)
  {
    UpdateDisplay(IDC_ACTUAL_YAW);
  }

  double percent = 0;
  if (last.motor.A != m_state.motor.A)
  {
    percent = uint16_t((m_state.motor.A / (double)std::numeric_limits<uint16_t>::max()) * 1000.0) / 10.0;
    SetDoubleValue(percent, m_hDlg, IDC_A_THRUST);

    tstringstream formatted;
    formatted << (int)percent;

    m_orb_A.SetText(formatted.str().c_str());

    UpdateDisplay(IDC_MOTOR_A);
  }

  if (last.motor.B != m_state.motor.B)
  {
    percent = uint16_t((m_state.motor.B / (double)std::numeric_limits<uint16_t>::max()) * 1000.0) / 10.0;
    SetDoubleValue(percent, m_hDlg, IDC_B_THRUST);

    tstringstream formatted;
    formatted << (int)percent;

    m_orb_B.SetText(formatted.str().c_str());

    UpdateDisplay(IDC_MOTOR_B);
  }

  if (last.motor.C != m_state.motor.C)
  {
    percent = uint16_t((m_state.motor.C / (double)std::numeric_limits<uint16_t>::max()) * 1000.0) / 10.0;
    SetDoubleValue(percent, m_hDlg, IDC_C_THRUST);

    tstringstream formatted;
    formatted << (int)percent;

    m_orb_C.SetText(formatted.str().c_str());

    UpdateDisplay(IDC_MOTOR_C);
  }

  if (last.motor.D != m_state.motor.D)
  {
    percent = uint16_t((m_state.motor.D / (double)std::numeric_limits<uint16_t>::max()) * 1000.0) / 10.0;
    SetDoubleValue(percent, m_hDlg, IDC_D_THRUST);

    tstringstream formatted;
    formatted << (int)percent;

    m_orb_D.SetText(formatted.str().c_str());

    UpdateDisplay(IDC_MOTOR_D);
  }

  if (last.motor.E != m_state.motor.E)
  {
    percent = uint16_t((m_state.motor.E / (double)std::numeric_limits<uint16_t>::max()) * 1000.0) / 10.0;

    tstringstream formatted;
    formatted << (int)percent;

    m_orb_E.SetText(formatted.str().c_str());

    UpdateDisplay(IDC_MOTOR_E);
  }

  if (last.motor.F != m_state.motor.F)
  {
    percent = uint16_t((m_state.motor.F / (double)std::numeric_limits<uint16_t>::max()) * 1000.0) / 10.0;

    tstringstream formatted;
    formatted << (int)percent;

    m_orb_F.SetText(formatted.str().c_str());

    UpdateDisplay(IDC_MOTOR_F);
  }


  UpdateDisplay(IDC_CURRENT_LATITUDE);
  UpdateDisplay(IDC_CURRENT_LONGITUDE);
  UpdateDisplay(IDC_CURRENT_ALTITUDE);
  UpdateDisplay(IDC_CURRENT_RANGE);


  tstringstream battery1;
  battery1 << BatteryVoltage(m_state.batteries.battery[0].cell_level[0]);

  SetDlgItemText(m_hDlg, IDC_BATTERY_1_VALUE, (battery1.str().c_str()));


  // TODO: Revisit with multiple cell-levels.
  tstringstream battery2;

  battery2 << BatteryVoltage(m_state.batteries.battery[1].cell_level[0]);
  SetDlgItemText(m_hDlg, IDC_BATTERY_2A_VALUE, (battery2.str().c_str()));

  battery2.clear();
  battery2 << BatteryVoltage(m_state.batteries.battery[1].cell_level[1]);
  SetDlgItemText(m_hDlg, IDC_BATTERY_2B_VALUE, (battery2.str().c_str()));

  battery2.clear();
  battery2 << BatteryVoltage(m_state.batteries.battery[1].cell_level[2]);
  SetDlgItemText(m_hDlg, IDC_BATTERY_2C_VALUE, (battery2.str().c_str()));

  battery2.clear();
  battery2 << BatteryVoltage(m_state.batteries.battery[1].cell_level[3]);
  SetDlgItemText(m_hDlg, IDC_BATTERY_2D_VALUE, (battery2.str().c_str()));


}


/*****************************************************************************/
void CmdCtrlDlg::UpdateCommandStatus()
{
  CheckDlgButton(m_hDlg, IDC_ANGLE_CONTROL, m_control_mode == angle_control ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(m_hDlg, IDC_RATE_CONTROL,  m_control_mode == rate_control  ? BST_CHECKED : BST_UNCHECKED);

  CheckDlgButton(m_hDlg, IDC_USER_INPUT_SIGNAL,  g_control_signal == k_user_input_signal  ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(m_hDlg, IDC_SQUARE_WAVE_SIGNAL, g_control_signal == k_square_wave_signal ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(m_hDlg, IDC_SINE_WAVE_SIGNAL,   g_control_signal == k_sine_wave_signal   ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(m_hDlg, IDC_IMPULSE,            g_control_signal == k_one_sec_impulse    ? BST_CHECKED : BST_UNCHECKED);

  CheckDlgButton(m_hDlg, IDC_CONTROL_ROLL,  m_disable_roll  ? BST_UNCHECKED : BST_CHECKED);
  CheckDlgButton(m_hDlg, IDC_CONTROL_PITCH, m_disable_pitch ? BST_UNCHECKED : BST_CHECKED);
  CheckDlgButton(m_hDlg, IDC_CONTROL_YAW,   m_disable_yaw   ? BST_UNCHECKED : BST_CHECKED);

  EnableCommandControls(g_control_signal != k_user_input_signal);

  CheckDlgButton(m_hDlg, IDC_ROLL_SIGNAL,  m_use_roll_signal  ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(m_hDlg, IDC_PITCH_SIGNAL, m_use_pitch_signal ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(m_hDlg, IDC_YAW_SIGNAL,   m_use_yaw_signal   ? BST_CHECKED : BST_UNCHECKED);

}


/*****************************************************************************/
void CmdCtrlDlg::ApplyLimits( )
{
  char bufferPeriod[32] = {0};
  char bufferScalar[32] = {0};

  GetDlgItemTextA(m_hDlg, IDC_CONTROL_PERIOD, bufferPeriod, 32);
  GetDlgItemTextA(m_hDlg, IDC_CONTROL_SCALAR, bufferScalar, 32);
   
  g_control_signal_period = ::strtod(bufferPeriod, 0);

  // Set limits of the scalar between 0.0 and 1.0
  float scalar = ::strtod(bufferScalar, 0);
  g_control_signal_scalar = std::max(std::min(scalar, 1.0f), 0.0f);

  if (g_control_signal_scalar != scalar)
  {
    SetDoubleValue(g_control_signal_scalar, m_hDlg, IDC_CONTROL_SCALAR);
  }
}


/*****************************************************************************/
float CmdCtrlDlg::RollAngle(int16_t value)
{
  float result  = value < 0
                ? value / (float)std::numeric_limits<int16_t>::min() * -1.0f
                : value / (float)std::numeric_limits<int16_t>::max();
  return result * k_roll_limit;
}

/*****************************************************************************/
float CmdCtrlDlg::PitchAngle(int16_t value)
{
  float result  = value < 0
                ? value / (float)std::numeric_limits<int16_t>::min() * -1.0f
                : value / (float)std::numeric_limits<int16_t>::max();
  return result * k_pitch_limit;
}

/*****************************************************************************/
float CmdCtrlDlg::YawAngle(int16_t value)
{
  float result  = value < 0
                ? value / (float)std::numeric_limits<int16_t>::min() * -1.0f
                : value / (float)std::numeric_limits<int16_t>::max();

  return result * k_yaw_limit;
}

/*****************************************************************************/
float CmdCtrlDlg::ThrustPercentage(int16_t value)
{
  float result  = value < 0
                ? value / (float)std::numeric_limits<int16_t>::min()
                : value / (float)std::numeric_limits<int16_t>::max();

  // We only allow thrust in positive direction.
  return std::fabs(result) * k_thrust_limit;
}

/*****************************************************************************/
float CmdCtrlDlg::ToFloat(uint64_t value)
{
  float result  = value / (float)std::numeric_limits<uint64_t>::max();

  return std::fabs(result);
}

/*****************************************************************************/
float CmdCtrlDlg::BatteryVoltage(int16_t value)
{
  // Currently based upon 12-bit ADC

  float result  = value < 0
                ? value / (float)2048 * -1.0f
                : value / (float)2048;

  return result;
}

/*****************************************************************************/
float CmdCtrlDlg::RollOrientation(int16_t value)
{
  float result  = value < 0
                ? value / (float)std::numeric_limits<int16_t>::min() * -1.0f
                : value / (float)std::numeric_limits<int16_t>::max();
  return result * 90.0f;
}

/*****************************************************************************/
float CmdCtrlDlg::PitchOrientation(int16_t value)
{
  float result  = value < 0
                ? value / (float)std::numeric_limits<int16_t>::min() * -1.0f
                : value / (float)std::numeric_limits<int16_t>::max();
  
  
  return result * 180.0f;
}

/*****************************************************************************/
float CmdCtrlDlg::YawOrientation(int16_t value)
{
  float result  = value < 0
                ? value / (float)std::numeric_limits<int16_t>::min() * -1.0f
                : value / (float)std::numeric_limits<int16_t>::max();

  if (result < 0.0)
  {
    result += 2.0;
  }

  return result * 180.0f;
}

/*****************************************************************************/
COLORREF CmdCtrlDlg::RollColor(int16_t value)
{
  float result  = value < 0
                ? value / (float)std::numeric_limits<int16_t>::min()
                : value / (float)std::numeric_limits<int16_t>::max();
  return RGB(0, (result * 255), 0);
}

/*****************************************************************************/
COLORREF CmdCtrlDlg::PitchColor(int16_t value)
{
  float result  = value < 0
                ? value / (float)std::numeric_limits<int16_t>::min()
                : value / (float)std::numeric_limits<int16_t>::max();
  return RGB((result * 255), 0, 0);
}

/*****************************************************************************/
COLORREF CmdCtrlDlg::YawColor(int16_t value)
{
  float result  = value < 0
                ? value / (float)std::numeric_limits<int16_t>::min()
                : value / (float)std::numeric_limits<int16_t>::max();
  return RGB(0, 0, (result * 255));
}

/*****************************************************************************/
COLORREF CmdCtrlDlg::ThrustColor(int16_t value)
{
  float result  = value < 0
                ? value / (float)std::numeric_limits<int16_t>::min()
                : value / (float)std::numeric_limits<int16_t>::max();

  // We only allow thrust in positive direction.
  return RGB((result * 255), (result * 255), (result * 255));
}


/*****************************************************************************/
COLORREF ScaleColor(COLORREF color, BYTE level)
{
  float scale = level / 255.0f;

  return RGB((GetRValue(color)*scale), 
             (GetGValue(color)*scale),
             (GetBValue(color)*scale));
}
