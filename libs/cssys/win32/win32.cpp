/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
  
    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "sysdef.h"
#include "csgeom/math3d.h"
#include "cssys/common/system.h"
#include "cssys/win32/win32.h"
#include "csutil/archive.h"
#include "csutil/inifile.h"
#include "igraph3d.h"
#include "cssys/win32/DirectDetection.h"
#include "cs2d/ddraw/ig2d.h"

#include <windows.h>
#include <windowsx.h>
#include <ddraw.h>
#include <stdio.h>
#include <time.h>

#if defined(COMP_WCC) || defined(COMP_VC)
#include <sys\timeb.h>
#endif

extern HINSTANCE ModuleHandle;
extern int ApplicationShow;
extern bool ApplicationActive;

extern csSystemDriver* System; // Global pointer to system that can be used by event handler

// The scan code -> character translation table
static unsigned short ScanCodeToChar[128] =
{
  CSKEY_ESC,27,       '1',      '2',      '3',      '4',      '5',      '6',    // 00..07
  '7',      '8',      '9',      '0',      '-',      '=',      '\b',     '\t',   // 08..0F
  'q',      'w',      'e',      'r',      't',      'y',      'u',      'i',    // 10..17
  'o',      'p',      '[',      ']',      '\n',     CSKEY_CTRL,'a',     's',    // 18..1F
  'd',      'f',      'g',      'h',      'j',      'k',      'l',      ';',    // 20..27
  39,       '`',      CSKEY_SHIFT,'\\',   'z',      'x',      'c',      'v',    // 28..2F
  'b',      'n',      'm',      ',',      '.',      '/',      CSKEY_SHIFT,'*',  // 30..37
  CSKEY_ALT,' ',      0,        CSKEY_F1, CSKEY_F2, CSKEY_F3, CSKEY_F4, CSKEY_F5,// 38..3F
  CSKEY_F6,  CSKEY_F7, CSKEY_F8, CSKEY_F9, CSKEY_F10,0,       0,        CSKEY_HOME,// 40..47
  CSKEY_UP,  CSKEY_PGUP,'-',    CSKEY_LEFT,CSKEY_CENTER,CSKEY_RIGHT,'+',CSKEY_END,// 48..4F
  CSKEY_DOWN,CSKEY_PGDN,CSKEY_INS,CSKEY_DEL,0,      0,        0,        CSKEY_F11,// 50..57
  CSKEY_F12,0,        0,        0,        0,        0,        0,        0,      // 58..5F
  0,        0,        0,        0,        0,        0,        0,        0,      // 60..67
  0,        0,        0,        0,        0,        0,        0,        0,      // 68..6F
  0,        0,        0,        0,        0,        0,        0,        0,      // 70..77
  0,        0,        0,        0,        0,        0,        0,        0       // 78..7F
};

//--------------------------------------------------// The System Driver //---//

SysSystemDriver::SysSystemDriver() : csSystemDriver()
{
  WNDCLASS wc;

  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wc.lpfnWndProc = csWindowProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = ModuleHandle;
  wc.hIcon = LoadIcon (NULL, IDI_APPLICATION);
  wc.hCursor = NULL;
  wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wc.lpszClassName = WINDOWCLASSNAME;

  RegisterClass (&wc);
  MouseCursor = NULL;
}

void SysSystemDriver::SetSystemDefaults ()
{
  csSystemDriver::SetSystemDefaults ();
  
  FullScreen = 1;
  HardwareCursor = true;
  if (config)
  {
    FullScreen = config->GetYesNo ("VideoDriver", "FULL_SCREEN", System->FullScreen);
    HardwareCursor = config->GetYesNo ("VideoDriver", "SYS_MOUSE_CURSOR", HardwareCursor);
  }
}

void SysSystemDriver::Alert (char* s)
{
  MessageBox(NULL, s, "Fatal Error", MB_OK | MB_ICONEXCLAMATION);
}

void SysSystemDriver::Warn (char* s)
{
  MessageBox(NULL, s, "Warning", MB_OK | MB_ICONEXCLAMATION);
}

// The System driver ////////////////

BEGIN_INTERFACE_TABLE(SysSystemDriver)
	IMPLEMENTS_COMPOSITE_INTERFACE(Win32SystemDriver)
	IMPLEMENTS_COMPOSITE_INTERFACE(System)
END_INTERFACE_TABLE()

IMPLEMENT_UNKNOWN_NODELETE(SysSystemDriver)

//// System loop ! 

void SysSystemDriver::Loop(void)
{
  MSG msg;
  IDDraw3GraphicsInfo* piG2Ddd3 = NULL;

  HRESULT hRes = System->piG2D->QueryInterface( IID_IDDraw3GraphicsInfo, (void**)&piG2Ddd3);

  if (SUCCEEDED(hRes))
  {
      piG2Ddd3->SetColorPalette();

      piG2Ddd3->Release();
      piG2Ddd3 = NULL;
  }

  while(!Shutdown && !ExitLoop)
  {
    if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
    {
      if( !GetMessage( &msg, NULL, 0, 0 ) )
        return;//msg.wParam;
      
      TranslateMessage(&msg); 
      DispatchMessage(&msg);
    }
    else
    {
      static long prev_time = -1;
      long elapsed = (prev_time == -1) ? 0 : Time () - prev_time;
      prev_time = Time ();
      NextFrame (elapsed, Time ());
    }
  }
}

//---------------------------------------------------// Window procedure //---//

LONG CALLBACK SysSystemDriver::csWindowProc (HWND Window, UINT Msg,
  WPARAM wParam, LPARAM lParam)
{
  int button;
  unsigned long time = SysGetTime ();
  
  switch (Msg)
  {
    case WM_ACTIVATEAPP:
      ApplicationActive = wParam;
      break;

    case WM_ACTIVATE:
      if (System)
        System->Keyboard->Reset ();
      break;

    case WM_SETCURSOR:
      SetCursor (((SysSystemDriver *)System)->MouseCursor);
      return TRUE;

    case WM_CREATE:
      break;

    case WM_DESTROY:
      PostQuitMessage (0);
      break;

    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
      if (System)
      {
        bool Down = (Msg == WM_KEYDOWN) || (Msg == WM_SYSKEYDOWN);
        int Key = ScanCodeToChar [(lParam >> 16) & 0x7f];

        if (Key)
          if (Down)
            System->Keyboard->do_keypress (time, Key);
          else
            System->Keyboard->do_keyrelease (time, Key);
      }
      return TRUE;
    
    case WM_LBUTTONDOWN:
      button = 1;
      goto button_down;
    case WM_RBUTTONDOWN:
      button = 2;
      goto button_down;
    case WM_MBUTTONDOWN:
      button = 3;
button_down:
      if (System)
        System->Mouse->do_buttonpress (SysGetTime (), button,
          LOWORD (lParam), HIWORD(lParam), wParam & MK_SHIFT,
          GetAsyncKeyState(VK_MENU), wParam & MK_CONTROL);
      break;
    
    case WM_LBUTTONUP:
      button = 1;
      goto button_up;
    case WM_RBUTTONUP:
      button = 2;
      goto button_up;
    case WM_MBUTTONUP:
      button = 3;
button_up:
      if (System)
        System->Mouse->do_buttonrelease (SysGetTime (), button,
          LOWORD (lParam), HIWORD (lParam));
      break;
    
    case WM_MOUSEMOVE:
      if (System)
        System->Mouse->do_mousemotion (SysGetTime (),
          LOWORD (lParam), HIWORD (lParam));
      break;
  }

  return DefWindowProc (Window, Msg, wParam, lParam);
}

//-------------------------------------------------// COM Implementation //---//

IMPLEMENT_COMPOSITE_UNKNOWN_AS_EMBEDDED (SysSystemDriver, Win32SystemDriver)

STDMETHODIMP SysSystemDriver::XWin32SystemDriver::GetInstance(HINSTANCE* retval)
{
  *retval = ModuleHandle;
  return S_OK;
}

STDMETHODIMP SysSystemDriver::XWin32SystemDriver::GetIsActive ()
{
  return ApplicationActive ? S_OK : S_FALSE;
}

STDMETHODIMP SysSystemDriver::XWin32SystemDriver::GetCmdShow (int* retval)
{
  *retval = ApplicationShow;
  return S_OK;
}

STDMETHODIMP SysSystemDriver::XWin32SystemDriver::GetHardwareCursor (bool *enabled)
{
  METHOD_PROLOGUE (SysSystemDriver, Win32SystemDriver);
  *enabled = pThis->HardwareCursor;
  return S_OK;
}

STDMETHODIMP SysSystemDriver::XWin32SystemDriver::SetMouseCursor (LPCTSTR cursorid)
{
  METHOD_PROLOGUE (SysSystemDriver, Win32SystemDriver);
  if (cursorid)
    pThis->MouseCursor = LoadCursor (NULL, cursorid);
  else
    pThis->MouseCursor = NULL;
  SetCursor (pThis->MouseCursor);
  return S_OK;
}

//------------------------------------------------// The Keyboard Driver //---//

SysKeyboardDriver::SysKeyboardDriver() : csKeyboardDriver ()
{
  // Create your keyboard interface
}

SysKeyboardDriver::~SysKeyboardDriver(void)
{
  // Destroy your keyboard interface
  Close();
}

bool SysKeyboardDriver::Open(csEventQueue *EvQueue)
{
  csKeyboardDriver::Open (EvQueue);
  return true;
}

void SysKeyboardDriver::Close(void)
{
  // Close your keyboard interface
}

//---------------------------------------------------// The Mouse Driver //---//

SysMouseDriver::SysMouseDriver() : csMouseDriver ()
{
}

SysMouseDriver::~SysMouseDriver(void)
{
  Close();
}

bool SysMouseDriver::Open(csEventQueue *EvQueue)
{
  csMouseDriver::Open (GetISystemFromSystem(System), EvQueue);
  // Open mouse system
  return true;
}

void SysMouseDriver::Close()
{
  // Close mouse system
}
