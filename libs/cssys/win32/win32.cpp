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

#ifdef OS_WIN32

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

#if defined(COMP_BC)
#include <dos.h> // For _argc & _argv
#endif

#define NAME "Crystal"

extern HINSTANCE DllHandle; //provided by the COM package
HINSTANCE gb_hInstance;
BOOL gb_bActive;
int gb_nCmdShow;

STDAPI DllInitialize ()
{
  //a Dummy function. required by the CS COM lib. This function would be
  //called, if the cspace lib woul be loaded as a COM module.
  return TRUE;
}

extern void cleanup();

void sys_fatalerror(char *s)
{
  SysSystemDriver::Printf (MSG_FATAL_ERROR, "%s", s);
  cleanup();
  exit(1);
}

extern csSystemDriver* System; // Global pointer to system that can be used by event handler

long FAR PASCAL WindowProc( HWND hWnd, UINT message,WPARAM wParam, LPARAM lParam );


////The System Driver////////////

void SysSystemDriver::SetSystemDefaults ()
{
  csSystemDriver::SetSystemDefaults ();
  
  System->FullScreen = 1; if (config) System->FullScreen = config->GetYesNo ("VideoDriver", "FULL_SCREEN", System->FullScreen);
}

void SysSystemDriver::Alert (const char* s)
{
  bool FullScreen = false;

  if (piGI)
  {
    //Check if we are in Fullscreenmode.
    piGI->GetIsFullScreen(FullScreen);
  }

  if (FullScreen)
  {
    //if fullscreen mode is active, we switch to default screen, because 
    //otherwise this message will not be seen. 
    ChangeDisplaySettings(NULL,0);
  }

  MessageBox(NULL, s, "Fatal Error", MB_OK | MB_ICONSTOP);
  debugprintf (true, s);
}

void SysSystemDriver::Warn (const char* s)
{
  //In windows there is no safe way to display a messagebox and then continue work when
  //you are in fullscreen mode. (If you know some way: You are welcome to change this)
  //For the moment we will display Warning as console messages.
  Printf(MSG_CONSOLE, "Warning:\n%s", s);
  debugprintf (true, s);
}

void SysSystemDriver::Close(void)
{
  csSystemDriver::Close();
  ChangeDisplaySettings(NULL,0);
}

////The Keyboard Driver////////////////

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

////The Mouse Driver///////////////////

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


// The System driver ////////////////

BEGIN_INTERFACE_TABLE(SysSystemDriver)
	IMPLEMENTS_COMPOSITE_INTERFACE(Win32SystemDriver)
	IMPLEMENTS_COMPOSITE_INTERFACE(System)
END_INTERFACE_TABLE()

IMPLEMENT_UNKNOWN_NODELETE(SysSystemDriver)

SysSystemDriver::SysSystemDriver() : csSystemDriver()
{
  WNDCLASS wc;

  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wc.lpfnWndProc = WindowProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = gb_hInstance;
  wc.hIcon = LoadIcon( NULL, IDI_APPLICATION );
  wc.hCursor = LoadCursor( NULL, IDC_ARROW );
  wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wc.lpszClassName = NAME;

  RegisterClass( &wc );
// Serguei 'Snaar' Narojnyi
// Following doesnt want to work on my system,
// it just thinks it's true anyway and i dont know what to do
// It may be VC++ 6.0 SP2 bug, but since someone else could use it
// I decided to make it in thins way.
//  if(!RegisterClass( &wc ))
//    sys_fatalerror("Cannot register CrystalSpace window class");
}

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


/// COM Implementation

IMPLEMENT_COMPOSITE_UNKNOWN_AS_EMBEDDED( SysSystemDriver, Win32SystemDriver )

STDMETHODIMP SysSystemDriver::XWin32SystemDriver::GetInstance(HINSTANCE* retval)
{
	*retval = gb_hInstance;
	return S_OK;
}

STDMETHODIMP SysSystemDriver::XWin32SystemDriver::GetIsActive()
{
	return gb_bActive ? S_OK : S_FALSE;
}

STDMETHODIMP SysSystemDriver::XWin32SystemDriver::GetCmdShow(int* retval)
{
	*retval = gb_nCmdShow;
	return S_OK;
}

// Windows input translator ////////////

inline void WinKeyTrans(csSystemDriver* pSystemDriver, WPARAM wParam, bool shift, bool alt, bool ctrl, bool down)
{
  if(!pSystemDriver) return;
  
  // handle standard alphabetical.
/*		if((wParam >= 'A') && (wParam <= 'z'))
    {
      if(down)
        pSystemDriver->Keyboard->do_keypress (SysGetTime (), wParam + ('a' - 'A')) ;
      else
        pSystemDriver->Keyboard->do_keyrelease (SysGetTime (), wParam + ('a' - 'A')) ;
      
    }
    else if((wParam >= '0') && (wParam <= '9'))
    {
      if(down)
        pSystemDriver->Keyboard->do_keypress(SysGetTime (), wParam);
      else 
        pSystemDriver->Keyboard->do_keyrelease(SysGetTime (), wParam);
    }
    else */
    {
      switch(wParam)
      {
      case VK_END:
        if(down) pSystemDriver->Keyboard->do_keypress (SysGetTime (), CSKEY_END) ;
        else pSystemDriver->Keyboard->do_keyrelease (SysGetTime (), CSKEY_END) ;
        break;
      case VK_ESCAPE:
        if(down) pSystemDriver->Keyboard->do_keypress (SysGetTime (), CSKEY_ESC) ;
        else pSystemDriver->Keyboard->do_keyrelease (SysGetTime (), CSKEY_ESC) ;
        break;
/*      case VK_SPACE:
        if(down) pSystemDriver->Keyboard->do_keypress (SysGetTime (), ' ') ;
        else pSystemDriver->Keyboard->do_keyrelease (SysGetTime (), ' ') ;
        break;
      case VK_TAB:
        if(down) pSystemDriver->Keyboard->do_keypress (SysGetTime (), '\t') ;
        else pSystemDriver->Keyboard->do_keyrelease (SysGetTime (), '\t');
        break; */
      case VK_RETURN:
        if(down) pSystemDriver->Keyboard->do_keypress (SysGetTime (), '\n') ;
        else pSystemDriver->Keyboard->do_keyrelease (SysGetTime (), '\n') ;
        break;
/*      case VK_BACK:
        if(down) pSystemDriver->Keyboard->do_keypress (SysGetTime (), '\b') ;
        else pSystemDriver->Keyboard->do_keyrelease (SysGetTime (), '\b') ;
        break;
      case VK_DELETE:
        if(down) pSystemDriver->Keyboard->do_keypress (SysGetTime (), '\b') ;
        else pSystemDriver->Keyboard->do_keyrelease (SysGetTime (), '\b') ;
        break; */
      case VK_UP:
        if(down) pSystemDriver->Keyboard->do_keypress (SysGetTime (), CSKEY_UP) ;
        else pSystemDriver->Keyboard->do_keyrelease (SysGetTime (), CSKEY_UP) ;
        break;
      case VK_DOWN:
        if(down) pSystemDriver->Keyboard->do_keypress (SysGetTime (), CSKEY_DOWN) ;
        else pSystemDriver->Keyboard->do_keyrelease (SysGetTime (), CSKEY_DOWN) ;
        break;
        
      case VK_LEFT:
        if(down) pSystemDriver->Keyboard->do_keypress (SysGetTime (), CSKEY_LEFT) ;
        else pSystemDriver->Keyboard->do_keyrelease (SysGetTime (), CSKEY_LEFT) ;
        break;
        
      case VK_RIGHT:
        if(down) pSystemDriver->Keyboard->do_keypress (SysGetTime (), CSKEY_RIGHT) ;
        else pSystemDriver->Keyboard->do_keyrelease (SysGetTime (), CSKEY_RIGHT) ;
        break;
        
      case VK_PRIOR:
        if(down) pSystemDriver->Keyboard->do_keypress (SysGetTime (), CSKEY_PGUP) ;
        else pSystemDriver->Keyboard->do_keyrelease (SysGetTime (), CSKEY_PGUP) ;
        break;
        
      case VK_NEXT:
        if(down) pSystemDriver->Keyboard->do_keypress (SysGetTime (), CSKEY_PGDN) ;
        else pSystemDriver->Keyboard->do_keyrelease (SysGetTime (), CSKEY_PGDN) ;
        break;
        
      case VK_MENU:
        if(down) pSystemDriver->Keyboard->do_keypress (SysGetTime (), CSKEY_ALT) ;
        else pSystemDriver->Keyboard->do_keyrelease (SysGetTime (), CSKEY_ALT) ;
        break;
        
      case VK_CONTROL:
        if(down) pSystemDriver->Keyboard->do_keypress (SysGetTime (), CSKEY_CTRL) ;
        else pSystemDriver->Keyboard->do_keyrelease (SysGetTime (), CSKEY_CTRL) ;
        break;

      case VK_SHIFT:
        if(down) pSystemDriver->Keyboard->do_keypress (SysGetTime (), CSKEY_SHIFT) ;
        else pSystemDriver->Keyboard->do_keyrelease (SysGetTime (), CSKEY_SHIFT);
        break;

//      default:
//        pSystemDriver->Keyboard->do_keypress (SysGetTime (), wParam) ;
      }
    }
}

long FAR PASCAL WindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
  bool shift=false, ctrl=false;
  
  if(GetAsyncKeyState(VK_CONTROL)) ctrl=true;
  if(GetAsyncKeyState(VK_SHIFT)) shift=true;
  
  switch( message )
  {
  case WM_ACTIVATEAPP:
    gb_bActive = wParam;
    break;

  case WM_ACTIVATE:
    if (System) System->Keyboard->Reset ();
    break;

  case WM_SETCURSOR:
    SetCursor(NULL);
    return TRUE;
    
  case WM_CREATE:
    break;
    
  case WM_DESTROY:
    PostQuitMessage( 0 );
    break;

  case WM_CHAR:
    if(System)
    {
	System->Keyboard->do_keypress (SysGetTime (), wParam) ;
	System->Keyboard->do_keyrelease (SysGetTime (), wParam) ;
    }
    break;
    
  case WM_KEYDOWN:
    WinKeyTrans(System, wParam, shift, false, ctrl, true);
    if(wParam==VK_MENU) return 0;
    break;
    
  case WM_SYSKEYDOWN:
    WinKeyTrans(System, wParam, shift, true, ctrl, true);
    if(wParam==VK_MENU) return 0;
    break;
    
  case WM_KEYUP:
    WinKeyTrans(System, wParam, shift, false, ctrl, false);
    if(wParam==VK_MENU) return 0;
    break;
    
  case WM_SYSKEYUP:
    WinKeyTrans(System, wParam, shift, true, ctrl, false);
    if(wParam==VK_MENU) return 0;
    break;
    
  case WM_LBUTTONDOWN:
    if (System)
      System->Mouse->do_buttonpress(SysGetTime (), 1, LOWORD(lParam), HIWORD(lParam), wParam & MK_SHIFT , GetAsyncKeyState(VK_MENU), wParam & MK_CONTROL);
    break;
    
  case WM_LBUTTONUP:
    if (System)
      System->Mouse->do_buttonrelease(SysGetTime (), 1, LOWORD(lParam), HIWORD(lParam));
    break;
    
  case WM_MBUTTONDOWN:
    if (System)
      System->Mouse->do_buttonpress(SysGetTime (), 3, LOWORD(lParam), HIWORD(lParam), wParam & MK_SHIFT , GetAsyncKeyState(VK_MENU), wParam & MK_CONTROL);
    break;
    
  case WM_MBUTTONUP:
    if (System)
      System->Mouse->do_buttonrelease(SysGetTime (), 3, LOWORD(lParam), HIWORD(lParam));
    break;
    
  case WM_RBUTTONDOWN:
    if (System)
      System->Mouse->do_buttonpress(SysGetTime (), 2, LOWORD(lParam), HIWORD(lParam), wParam & MK_SHIFT , GetAsyncKeyState(VK_MENU), wParam & MK_CONTROL);
    break;
    
  case WM_RBUTTONUP:
    if (System)
      System->Mouse->do_buttonrelease(SysGetTime (), 2, LOWORD(lParam), HIWORD(lParam));
    break;
    
  case WM_MOUSEMOVE:
    // Not yet ready for this!
    if (System)
      System->Mouse->do_mousemotion(SysGetTime (), LOWORD(lParam), HIWORD(lParam));
    break;
  }
  return DefWindowProc(hWnd, message, wParam, lParam);
}

extern int main (int argc, char* argv[]);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow )
{
  gb_hInstance = hInstance;
  DllHandle = gb_hInstance;

  gb_nCmdShow = nCmdShow;
  gb_bActive = true;
  
#ifdef COMP_BC
  main( _argc,  _argv);
#else
  main(__argc, __argv);
#endif

  return TRUE;
}

#endif //OS_WIN32

