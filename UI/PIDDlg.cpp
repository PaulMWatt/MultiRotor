/// @file PIDDlg.cpp 
///
/// Provides a peek into the PID values
///
//  ****************************************************************************
//  Includes *******************************************************************


#include "../stdafx.h"
#include <windows.h>
#include "PIDDlg.h"
#include "../resource.h"
#include "../drone/utility/util.h"
#include "AutoGdi.h"

#include <sstream>

#if defined(_UNICODE)
typedef std::wstringstream    tstringstream;

#else
typedef std::stringstream     tstringstream;

#endif

HBRUSH GetWhiteBrush();
HBRUSH GetNullBrush();
void AdjustGain(PIDDesc *roll, PIDDesc *pitch, PIDDesc *rotation);
void AdjustRateGain(PIDDesc *roll, PIDDesc *pitch, PIDDesc *rotation);

//  ****************************************************************************
void SetDoubleValue (double input, HWND hDlg, int id)
{
  tstringstream formatted;

  formatted.precision(4);
  formatted << input;
  ::SetDlgItemText(hDlg, id, formatted.str().c_str());
}




/* Default Contstructor ******************************************************/
PIDDlg::PIDDlg()
  : m_is_update_gain(true)
  , m_commanded({0})
  , m_state({0})
{
  
}

//  ****************************************************************************
// Message handler for Command and control of the drone.
INT_PTR CALLBACK PIDDlg::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  PIDDlg *pThis = NULL;
  switch (message)
  {
  case WM_INITDIALOG:
    {
      // Assign the input parameter pointer to user data.
      ::SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
      // The lparam is our "this" pointer.
      pThis = (PIDDlg*)lParam;
      if (pThis)
      {
        pThis->m_hDlg = hDlg;
      }
    }
    break;
  default:
    // Get the pointer to "this" object for processing.
    LONG_PTR lVal = ::GetWindowLongPtr(hDlg, GWLP_USERDATA);
    pThis = (PIDDlg*)lVal;
  }

  // Process the message if a proper object was extracted.
  if (pThis)
  {
    return pThis->MessageHandler(hDlg, message, wParam, lParam);
  }

  return (INT_PTR)FALSE;
}


//  ****************************************************************************
INT_PTR PIDDlg::MessageHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  UNREFERENCED_PARAMETER(lParam);
  switch (message)
  {
  case WM_INITDIALOG:
    {

      ::SendDlgItemMessage(m_hDlg, IDC_CONTROL_NAME, CB_ADDSTRING, 0, (LPARAM)_T("Roll"));
      ::SendDlgItemMessage(m_hDlg, IDC_CONTROL_NAME, CB_ADDSTRING, 0, (LPARAM)_T("Roll Rate"));
      ::SendDlgItemMessage(m_hDlg, IDC_CONTROL_NAME, CB_ADDSTRING, 0, (LPARAM)_T("Pitch"));
      ::SendDlgItemMessage(m_hDlg, IDC_CONTROL_NAME, CB_ADDSTRING, 0, (LPARAM)_T("Pitch Rate"));
      ::SendDlgItemMessage(m_hDlg, IDC_CONTROL_NAME, CB_ADDSTRING, 0, (LPARAM)_T("Rotate"));


      //LOGFONT lf; 
      //::memset(&lf, 0, sizeof(LOGFONT));

      //HDC hdc = ::GetDC(NULL);
  
      //// Create a set of rotated Tahoma fonts for special painting.
      //int ptSize = 14;
      //lf.lfWeight = FW_BOLD;
      //lf.lfHeight = -MulDiv(ptSize, ::GetDeviceCaps(hdc, LOGPIXELSY), 72);
      //::_tcscpy(lf.lfFaceName, _T("Tahoma"));
      //m_tahoma0.Attach(CreateFontIndirect(&lf));

      //::SendDlgItemMessage(m_hDlg, IDC_CONTROL_NAME, WM_SETFONT, WPARAM(HFONT(m_tahoma0)), 0);
      //::ReleaseDC(NULL, hdc);

      UpdateStatus();
    }
    return (INT_PTR)TRUE;

  case QC_UPDATE_STATUS:
    UpdateStatus();
    return (INT_PTR)TRUE;
  
  case WM_COMMAND:
    {
      UINT id     = LOWORD(wParam);
      UINT notify = HIWORD(wParam);
      
      if ( BN_CLICKED == notify
        && IDOK       == id)
      {
        AdjustGain();

      }
      else if (CBN_SELCHANGE == notify)
      {
        UINT result = ::SendDlgItemMessage(m_hDlg, IDC_CONTROL_NAME, CB_GETCURSEL, 0, 0);
        if (result != CB_ERR)
        {
          m_type = PIDType(result + 1);
        }

      }

      return (INT_PTR)TRUE;
    }

  case WM_CTLCOLORDLG:
  case WM_CTLCOLORSTATIC:
    return (INT_PTR)GetWhiteBrush();

  }

  return (INT_PTR)FALSE;
}

//  ****************************************************************************
void PIDDlg::UpdateStatus()
{
  GetDroneState(m_state);
  GetPIDState(m_PIDS);

  PIDState *pPID = nullptr;

  int16_t orientation = 0;
  if (k_roll == m_type)
  {
    pPID = &m_PIDS.roll;

    orientation = m_state.orientation.roll;
  }
  else if (k_pitch == m_type)
  {
    pPID = &m_PIDS.pitch; 

    orientation = m_state.orientation.pitch;
  }
  else if (k_rotate == m_type)
  {
    pPID = &m_PIDS.rotation;

    orientation = m_state.orientation.yaw;
  }

  if (!pPID)
    return;

  // Update the values.
  SetDoubleValue(to_normalized(pPID->current_error), m_hDlg, IDC_ERROR);
  SetDoubleValue(to_normalized(pPID->delta_error), m_hDlg, IDC_DELTA_ERROR);
  SetDoubleValue(to_normalized(pPID->integral_error), m_hDlg, IDC_INTEGRAL_ERROR);
  SetDoubleValue(to_normalized(pPID->set_point), m_hDlg, IDC_SET_POINT);
  SetDoubleValue(to_normalized(orientation), m_hDlg, IDC_ACTUAL);

  HWND hFocus = GetFocus();


  if (m_is_update_gain)
  {
    double Kp = decode_PID(pPID->desc.Kp);
    SetDoubleValue(Kp, m_hDlg, IDC_KP);

    double Ki = decode_PID(pPID->desc.Ki);
    SetDoubleValue(Ki, m_hDlg, IDC_KI);

    double Kd = decode_PID(pPID->desc.Kd);
    SetDoubleValue(Kd, m_hDlg, IDC_KD);

    if ( hFocus == GetDlgItem(m_hDlg, IDC_KP)
      || hFocus == GetDlgItem(m_hDlg, IDC_KI)
      || hFocus == GetDlgItem(m_hDlg, IDC_KD))
    {
      m_is_update_gain = false;
    }
  }
}


//  ****************************************************************************
void PIDDlg::AdjustGain()
{
  char bufferKp[32];
  char bufferKi[32];
  char bufferKd[32];

  ::GetDlgItemTextA(m_hDlg, IDC_KP, bufferKp, 32);
  ::GetDlgItemTextA(m_hDlg, IDC_KI, bufferKi, 32);
  ::GetDlgItemTextA(m_hDlg, IDC_KD, bufferKd, 32);

  double Kp = ::strtod(bufferKp, 0);
  double Ki = ::strtod(bufferKi, 0);
  double Kd = ::strtod(bufferKd, 0);

  PIDDesc desc;
  desc.Kp = encode_PID(Kp);
  desc.Ki = encode_PID(Ki);
  desc.Kd = encode_PID(Kd);

  desc.range_min = to_int64(-1.0);
  desc.range_max = to_int64( 1.0);

  switch (m_type)
  {
  case k_roll:
    ::AdjustGain(&desc, 0, 0);
    break;
  case k_roll_rate:
    ::AdjustRateGain(&desc, 0, 0);
    break;
  case k_pitch:
    ::AdjustGain(0, &desc, 0);
    break;
  case k_pitch_rate:
    ::AdjustRateGain(0, &desc, 0);
    break;
  case k_rotate:
    ::AdjustRateGain(0, 0, &desc);
    break;
  }
}
