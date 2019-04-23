/// @file QCopter.cpp
///
/// HW8 - Flight
/// Paul Watt
/// EN605.715 - Software Development for Real Time Embedded Systems
/// 
/// This is the main program file for the Quad-copter's ground control station.
/// This program has a number of central repsonisbilities:
///   - reads and transmits the controls to the drone
///   - receives and displays the current status of the drone
///   - provides controls to tune parameters such as trims and PID gains
///
//  ****************************************************************************
// QuadCopter.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <objbase.h>
#include "QuadCopter.h"
#include "UI/CmdCtrlDlg.h"
#include "UI/PIDDlg.h"
#include "UI/VLCDisplay.h"
#include "Control/qcctrl.h"

#pragma comment(lib, "Ws2_32.lib")

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

HWND  hCmdDlg       = NULL;
HWND  hPidDlg       = NULL;

CmdCtrlDlg      cmdCtrlDlg;
PIDDlg          activePIDDlg;

// The initialized instance of the XBOX controller.
Controller      g_controller;  


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void        SetupDroneWindows(HWND hParent);


//  ****************************************************************************
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  // Initialize global strings
  LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
  LoadStringW(hInstance, IDC_QUADCOPTER, szWindowClass, MAX_LOADSTRING);
  MyRegisterClass(hInstance);

  // Perform application initialization:
  if (!InitInstance (hInstance, nCmdShow))
  {
      return FALSE;
  }

  HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_QUADCOPTER));

  MSG msg;

  // Main message loop:
  while (GetMessage(&msg, nullptr, 0, 0))
  {
      if ( !TranslateAccelerator(msg.hwnd, hAccelTable, &msg)
        && !IsDialogMessage(hPidDlg,&msg))
      {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
      }
  }

  // Free resources.
  WSACleanup();

  return (int) msg.wParam;
}



//  ****************************************************************************
//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_QUADCOPTER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_QUADCOPTER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
  // Initialize WinSock
  WSADATA wsaData;

  int result = WSAStartup(MAKEWORD(2,2), &wsaData);
  if (result != 0) 
  {
    printf("WSAStartup failed with error: %d\n", result);
    return -1;
  }

  hInst = hInstance; // Store instance handle in our global variable

  HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
     CW_USEDEFAULT, 0, 1100, 650, nullptr, nullptr, hInstance, nullptr);

  if (!hWnd)
  {
     return FALSE;
  }

  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

  return TRUE;
}

//  ****************************************************************************
//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
      {
        SetupDroneWindows(hWnd);
      }
    break;
  case WM_ERASEBKGND:
    // Disable background erase functionality.
    // This will be taken care of in the WM_PAINT message.
    // The PAINTSTRUCT::fErase value will be TRUE by returning a non-zero value.
    return 1;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

//  ****************************************************************************
// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

//  ****************************************************************************
void SetupDroneWindows(HWND hParent)
{
  if ( !g_controller.RegisterDevice(k_xboxCtrl_1)
    && !g_controller.RegisterDevice(k_xboxCtrl_2)
    && !g_controller.RegisterDevice(k_xboxCtrl_3)
    && !g_controller.RegisterDevice(k_xboxCtrl_4))
  {
    MessageBox(hParent, 
                _T("Could not detect XBox Controller.\nCheck the connection and restart the application."), 
                _T("Error"), 
                MB_ICONEXCLAMATION | MB_OK);
  }

  cmdCtrlDlg.m_hInst = hInst;
  hCmdDlg = ::CreateDialogParam(hInst, 
                                MAKEINTRESOURCE(IDD_HEX), 
                                hParent, 
                                CmdCtrlDlg::DlgProc,
                                (LPARAM)&cmdCtrlDlg);


  RECT ctrlRect;
  ::GetClientRect(hCmdDlg, &ctrlRect);

  hPidDlg = ::CreateDialogParam(hInst, 
                                MAKEINTRESOURCE(IDD_PID), 
                                hParent, 
                                PIDDlg::DlgProc,
                                (LPARAM)&activePIDDlg);
  RECT pidRect;
  SIZE szPID;
  ::GetClientRect(hPidDlg, &pidRect);
  szPID = article::GetRectSize(pidRect);
  ::MoveWindow(hPidDlg, ctrlRect.right - 12, ctrlRect.bottom - szPID.cy - 10, szPID.cx, szPID.cy, TRUE);
  activePIDDlg.name("Roll Rate");


  // Resize the parent to fit the dialog.
  ctrlRect.right += szPID.cx;
  SIZE szCtrl = article::GetRectSize(ctrlRect);

  RECT rect;
  RECT client;
  ::GetWindowRect(hParent, &rect);
  ::GetClientRect(hParent, &client);

  int vborder = (rect.bottom - rect.top) - (client.bottom - client.top);
  int hborder = (rect.right - rect.left) - (client.right - client.left);

  ::SetWindowPos(hParent, 
                 NULL, 
                 0,0,
                 szCtrl.cx + hborder,
                 szCtrl.cy + vborder,
                 SWP_NOMOVE | SWP_NOOWNERZORDER);

  StartListening(hCmdDlg);

  StartTransmission(hCmdDlg, g_controller);
}

//  **************************************************************************
void UpdatePIDStatus()
{
  ::PostMessage(activePIDDlg.m_hDlg,   QC_UPDATE_STATUS, 0, 0);
}

