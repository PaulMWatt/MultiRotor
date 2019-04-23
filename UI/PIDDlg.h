/// @file PIDDlg.h
///
/// Provides a peek into the PID values
///
//  ****************************************************************************
#ifndef PIDDLG_H_INCLUDED
#define PIDDLG_H_INCLUDED

//  Includes *******************************************************************
#include "ui_def.h"
#include "../resource.h"
#include "../Control/qcctrl.h"
#include "AutoGdi.h"

#include <string>

using namespace article;


//  ****************************************************************************
/// Sub-dialog that manages the display for the drone status/control feedback.
///
///
class PIDDlg
{ 
public:
  PIDDlg();

  static 
    INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

  HWND        m_hDlg;
  HINSTANCE   m_hInst;

  void name(const std::string& PIDName)
  {

    if (PIDName == "Roll")
    {
      m_type = k_roll;
    }
    else if (PIDName == "Roll Rate")
    {
      m_type = k_roll_rate;
    }
    else if (PIDName == "Pitch")
    {
      m_type = k_pitch;
    }
    else if (PIDName == "Pitch Rate")
    {
      m_type = k_pitch_rate;
    }
    else if (PIDName == "Rotate")
    {
      m_type = k_rotate;
    }

    ::SendDlgItemMessage(m_hDlg, IDC_CONTROL_NAME, CB_SETCURSEL, m_type - 1, 0);

  }

private:
  //  Members ******************************************************************
  bool        m_is_update_gain;

  QCopter     m_commanded;
  DroneState  m_state;
  DronePIDs   m_PIDS;

  AutoFont    m_tahoma0;

  PIDType     m_type;


  //  Methods ******************************************************************
  INT_PTR MessageHandler(HWND, UINT, WPARAM, LPARAM);

  void UpdateStatus();
  void AdjustGain();
};




#endif
