/// @file VLCDisplay.h
///
/// Displays streaming video.
///
//  ****************************************************************************
//  Includes *******************************************************************
#ifndef VLCDISPLAY_H_INCLUDED
#define VLCDISPLAY_H_INCLUDED

#include "ui_def.h"
#include "../resource.h"

//#include "vlc/VLCWrapper.h"

//  ****************************************************************************
class VLCDisplay
{
public:
  VLCDisplay()
  { }

  //static 
  //  void HandleVLCEvents(const VLCEvent*, void*);

  static 
    INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

  //void UpdatePosition();

  HWND        m_hDlg;
  HINSTANCE   m_hInst;


private:

  //VLCWrapper       m_vlcPlayer;
  bool             m_created;

  //  Methods ******************************************************************
  INT_PTR MessageHandler(HWND, UINT, WPARAM, LPARAM);

	void OnPaint();
};

#endif
