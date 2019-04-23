/// @file CmdCtrlDlg.ch
///
/// Provides feedbak on the command and control of the quad copter.
///
//  ****************************************************************************
#ifndef CNDCTRLDLG_H_INCLUDED
#define CNDCTRLDLG_H_INCLUDED

//  Includes *******************************************************************
#include "ui_def.h"
#include "../resource.h"
#include "AutoGdi.h"
#include "aa_ellipse.h"
#include "OrbButton.h"
#include "GlossButton.h"
#include "BitBlender.h"
#include "../Control/qcctrl.h"

using namespace article;




//  ****************************************************************************
/// Sub-dialog that manages the display for the drone status/control feedback.
///
///
class CmdCtrlDlg
{
public:
  CmdCtrlDlg();

  static 
    INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

  HWND        m_hDlg;
  HINSTANCE   m_hInst;

private:
  //  Members ******************************************************************
  static const 
    UINT_PTR  k_timerSampleInput = 1001;

  const 
    float     k_roll_limit  = 30.0f;
  const 
    float     k_pitch_limit = 30.0f;
  const 
    float     k_yaw_limit   = 90.0f;
  const 
    float     k_thrust_limit= 100.0f;


  QCopter     m_commanded;
  DroneState  m_state;
  TrimMode    m_trim_mode;

  ControlMode     m_control_mode;

  bool            m_disable_roll;
  bool            m_disable_pitch;
  bool            m_disable_yaw;

  bool            m_use_roll_signal;
  bool            m_use_pitch_signal;
  bool            m_use_yaw_signal;

  OrbButton   m_orb_A;
  OrbButton   m_orb_B;
  OrbButton   m_orb_C;
  OrbButton   m_orb_D;
  OrbButton   m_orb_E;
  OrbButton   m_orb_F;



  // A few fonts to enhance the display.
  AutoFont    m_symbol;
  AutoFont    m_symbolBig;

  AutoFont    m_tahoma0;
  AutoFont    m_tahoma90;
  AutoFont    m_tahoma270;

  static const int   k_scale = 8;
  //  Methods ******************************************************************
  INT_PTR MessageHandler(HWND, UINT, WPARAM, LPARAM);

  void EnableCommandControls(bool is_enable);

  void CreateFonts();
  void InitControls();

  void ProcessEraseBkgnd(HWND, HDC);
  bool ProcessDrawItem(HWND, DRAWITEMSTRUCT&);

  void UpdateDisplay(UINT id);

  void PaintRangeBar(HDC hdc, const RECT& rc, int16_t value, COLORREF color, bool isVertical, bool isReversed);
  void PaintIndicator(HDC hdc, const RECT &rc, int16_t value, COLORREF color);

  void DroneConnected(uint32_t ip_addr);
  void DroneDisconnected();
  void DroneArmed    ();
  void ProcessInput();
  void UpdateStatus();
  void UpdateCommandStatus();
  void ApplyLimits( );

  float RollAngle         (int16_t value);
  float PitchAngle        (int16_t value);
  float YawAngle          (int16_t value);
  float ThrustPercentage  (int16_t value);
  float ToFloat           (uint64_t value);
  float BatteryVoltage    (int16_t value);


  float RollOrientation   (int16_t value);
  float PitchOrientation  (int16_t value);
  float YawOrientation    (int16_t value);

  COLORREF RollColor      (int16_t value);
  COLORREF PitchColor     (int16_t value);
  COLORREF YawColor       (int16_t value);
  COLORREF ThrustColor    (int16_t value);
};




#endif
