/// @file VLCDisplay.cpp
///
/// Displays streaming video.
///
//  ****************************************************************************
//  Includes *******************************************************************
#include "../stdafx.h"
#include "ui_def.h"
#include "../resource.h"
#include "AutoGdi.h"
#include "VLCDisplay.h"
#include <vlc/vlc.h>

//  ****************************************************************************
//void VLCDisplay::HandleVLCEvents(const VLCEvent* pEvt, void* pUserData)
//{       
//    VLCDisplay* pDlg = reinterpret_cast<VLCDisplay*>(pUserData); 
//}

//  ****************************************************************************
// Message handler for Command and control of the drone.
INT_PTR CALLBACK VLCDisplay::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  VLCDisplay *pThis = NULL;
  switch (message)
  {
  case WM_INITDIALOG:
    {
      // Assign the input parameter pointer to user data.
      ::SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
      // The lparam is our "this" pointer.
      pThis = (VLCDisplay*)lParam;
      if (pThis)
      {
        pThis->m_hDlg = hDlg;
      }
    }
    break;
  default:
    // Get the pointer to "this" object for processing.
    LONG_PTR lVal = ::GetWindowLongPtr(hDlg, GWLP_USERDATA);
    pThis = (VLCDisplay*)lVal;
  }

  // Process the message if a proper object was extracted.
  if (pThis)
  {
    return pThis->MessageHandler(hDlg, message, wParam, lParam);
  }

  return (INT_PTR)FALSE;
}

//  ****************************************************************************
INT_PTR VLCDisplay::MessageHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static libvlc_instance_t * inst;
  static libvlc_media_player_t *mp;

  UNREFERENCED_PARAMETER(lParam);
  switch (message)
  {
  case WM_INITDIALOG:
    {
     libvlc_media_t *m;
     
     /* Load the VLC engine */
     const char * const vlc_args[] = {
		   "-I", "dumy",      // No special interface
		   //"--ignore-config", // Don't use VLC's config
		   "--plugin-path=\"C:/Program Files (x86)/VideoLAN/VLC/plugins" };

     // Required copying the plugin directory to the same location
     // the executable is located. The plug-in path value is ignored.
	   // init vlc modules, should be done only once
     inst = 0;
 	   //inst = libvlc_new (sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);

     //inst = libvlc_new (0, NULL);
  
     if (inst)
     {
       /* Create a new item */
       m = libvlc_media_new_location (inst, "UDP://@:8090");
       //m = libvlc_media_new_path (inst, "/path/to/test.mov");
        
       /* Create a media player playing environement */
       mp = libvlc_media_player_new_from_media (m);
     
       /* No need to keep the media now */
       libvlc_media_release (m);


       libvlc_media_player_set_hwnd (mp, GetDlgItem(hDlg, IDC_VLC));

       /* play the media_player */
       libvlc_media_player_play (mp);

       m_created = true;
     }
    }
    return (INT_PTR)TRUE;


  case WM_DESTROY:
    {
      if (inst)
      {
        /* Stop playing */
        libvlc_media_player_stop (mp);
 
        /* Free the media_player */
        libvlc_media_player_release (mp);
 
        libvlc_release (inst);
      }
    }
    return (INT_PTR)TRUE;

  }

  return (INT_PTR)FALSE;
}

//  ****************************************************************************
void VLCDisplay::OnPaint()
{
  PAINTSTRUCT ps;

  HDC hDC = ::BeginPaint(m_hDlg, &ps);
	::RedrawWindow( ::GetDlgItem(m_hDlg, IDC_VLC),
                  NULL, NULL, 
                  RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
  ::EndPaint(m_hDlg, &ps);
}



//  ****************************************************************************
//void VLCDisplay::UpdatePosition()
//{
//
//}
