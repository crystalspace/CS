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

#ifdef OS_WIN32

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
#ifdef DO_DINPUT_KEYBOARD //New keyboard handling by Xavier Trochu (xtrochu@yahoo.com)
#include <dinput.h>
#endif
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

#ifdef DO_DINPUT_KEYBOARD

/*
 * This table performs the translation from keycode to character.
 * It use this english keyboard layout.
 * ----
 * PS: They may be some errors, it's quite difficult to check. So if you
 * have some keys that are not handled correctly, You'd better check this first.
 *
*/
static int s_KeyTable[257]= {
	0,
	CSKEY_ESC,'1','2','3','4','5','6','7','8','9','0','-','=',CSKEY_BACKSPACE,CSKEY_TAB,
	'q','w','e','r','t','y','u','i','o','p','[',']','\n',
	CSKEY_CTRL,'a','s','d','f','g','h','j','k','l',';','\'','`',CSKEY_SHIFT,'\\',
	'z','x','c','v','b','n','m',',','.','/',CSKEY_SHIFT,'*',CSKEY_ALT,
	' ',0/*DIK_CAPITAL*/,
	CSKEY_F1,CSKEY_F2,CSKEY_F3,CSKEY_F4,CSKEY_F5,CSKEY_F6,CSKEY_F7,CSKEY_F8,CSKEY_F9,CSKEY_F10,
	0/*DIK_NUMLOCK*/,0/*DIK_SCROLL*/,
	CSKEY_HOME,CSKEY_UP,CSKEY_PGUP,'-',CSKEY_LEFT,CSKEY_CENTER,CSKEY_RIGHT,'+',CSKEY_END,CSKEY_DOWN,CSKEY_PGDN,CSKEY_INS,'.',
	0/*DIK_OEM_102*/,CSKEY_F11,CSKEY_F12,0,0,0,0,0,0,0,0,0,0,0,0/*DIK_F13*/,0/*DIK_F14*/,0/*DIK_F15*/,0,0,0,0,0,0,0,0,0,
	0/*DIK_KANA*/,0,0,0/*DIK_ABNT_C1*/,0,0,0,0,0,0/*DIK_CONVERT*/,0,0/*DIK_NOCONVERT*/,
	0,0/*DIK_YEN*/,0/*DIK_ABNT_C2*/,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'=',0,0,0/*DIK_CIRCUMFLEX*/,0/*DIK_AT*/,
	0/*DIK_COLON*/,0/*DIK_UNDERLINE*/,0/*DIK_KANJI*/,0/*DIK_STOP*/,0/*DIK_AX*/,0/*DIK_UNLABELED*/,0,0,0,0,
	'\n',CSKEY_CTRL,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	'.',0,'/',0,0/*DIK_SYSRQ*/,CSKEY_ALT,0,0,0,0,0,0,0,0,0,0,0,0,0/*DIK_PAUSE*/,0,
	CSKEY_HOME,CSKEY_UP,CSKEY_PGUP,0,CSKEY_LEFT,0,CSKEY_RIGHT,0,CSKEY_END,CSKEY_DOWN,CSKEY_PGDN,CSKEY_INS,CSKEY_DEL
};

// This macro is for COM calls. If it fails, it shows a MessageBox then Kills the whole process. It's brutal, but as I use another thread, it's safer this way
#define CHK_FAILED(x)  { if(FAILED(x)) { ::MessageBox(::GetFocus(), #x " Failed!", NULL,MB_OK|MB_ICONERROR); ::ExitProcess(1); } }
// This macro is for COM Release calls
#define CHK_RELEASE(x)  { if((x) != NULL) { (x)->Release(); (x)=NULL; } }

/*
 * The thread entry point. Called by CsKeyboardDriver::Open()
 *
*/
DWORD WINAPI s_threadroutine(LPVOID param)
{
	SysKeyboardDriver * kbd=(SysKeyboardDriver*)param;
	HRESULT hr;
	char * buffer, *oldbuffer=NULL;
	LPDIRECTINPUT lpdi=NULL;
	LPDIRECTINPUTDEVICE lpKbd=NULL; 
	HANDLE hEvent[2];
	int i;

	CHK_FAILED(::DirectInputCreate(gb_hInstance, 0X300, &lpdi, NULL));  // 0X300 instead of DIRECTINPUT_VERSION allow the binaries to stay compatible with NT4
	CHK_FAILED(lpdi->CreateDevice(GUID_SysKeyboard, &lpKbd, NULL));
	CHK_FAILED(lpKbd->SetDataFormat(&c_dfDIKeyboard)); 
	CHK_FAILED(lpKbd->SetCooperativeLevel(::FindWindow(NAME,NULL), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)); 
	hEvent[0]=::CreateEvent(NULL,FALSE,FALSE,NULL);
	if(hEvent[0]==NULL)
	{
		::MessageBox(::GetFocus(), "CreateEvent() Failed!", NULL,MB_OK|MB_ICONERROR);
		::ExitProcess(1);
	}
	if(!::DuplicateHandle(::GetCurrentProcess(),kbd->m_hEvent,
						 ::GetCurrentProcess(),&hEvent[1],
						 0,FALSE,DUPLICATE_SAME_ACCESS))
	{
		::MessageBox(::GetFocus(), "DuplicateEvent() Failed!", NULL,MB_OK|MB_ICONERROR);
		::ExitProcess(1);
	}
	hr=lpKbd->SetEventNotification(hEvent[0]);
	switch(hr) {
	case DI_OK:
		break;
	default:
		::MessageBox(::GetFocus(), "lpKbd->SetEventNotification(hEvent) Failed!", NULL,MB_OK|MB_ICONERROR);
		::ExitProcess(1);
		break;
	}
	CHK_FAILED(lpKbd->Acquire());
	oldbuffer=new char[256];
	hr=lpKbd->GetDeviceState(256,oldbuffer);
	while(1) {
		switch(::WaitForMultipleObjects(2,hEvent,FALSE,INFINITE))
		{
		case WAIT_OBJECT_0:
			buffer=new char[256];
			do {
				hr=lpKbd->GetDeviceState(256,buffer);
				switch(hr)
				{
				case DIERR_NOTACQUIRED:
				case DIERR_INPUTLOST:
					lpKbd->Acquire();
					break;
				case DI_OK:
					break;
				default:
					::MessageBox(::GetFocus(), "lpKbd->GetDeviceState(hEvent) Failed!", NULL,MB_OK|MB_ICONERROR);
					::ExitProcess(1);
					break;
				}
			} while(hr!=DI_OK);
			for(i=0;i<256;i++)
				if(oldbuffer[i]!=buffer[i])
				{
					if(buffer[i]&0X80)
						kbd->do_keypress(SysGetTime(),s_KeyTable[i]);
					else
						kbd->do_keyrelease(SysGetTime(),s_KeyTable[i]);
					break;
				}
			delete[] oldbuffer;
			oldbuffer=buffer;
			break;
		case WAIT_OBJECT_0+1:
		case WAIT_TIMEOUT:
			lpKbd->Unacquire();
			CloseHandle(hEvent[0]);
			CloseHandle(hEvent[1]);
			CHK_RELEASE(lpKbd);
			CHK_RELEASE(lpdi);
			return 0;
		}
	}
}

#undef CHK_RELEASE
#undef CHK_FAILED
#endif	// DO_DINPUT_KEYBOARD

bool SysKeyboardDriver::Open(csEventQueue *EvQueue)
{
  csKeyboardDriver::Open (EvQueue);
#ifdef DO_DINPUT_KEYBOARD
  DWORD dwThreadId;
  m_hEvent=::CreateEvent(NULL,FALSE,FALSE,NULL);
  m_hThread=::CreateThread(NULL,0,s_threadroutine,this,0,&dwThreadId);
  if(m_hEvent==NULL||m_hThread==NULL)
  {
	::MessageBox(::GetFocus(), "CreateEvent() Failed!", NULL,MB_OK|MB_ICONERROR);
	::ExitProcess(1);
  }
#endif
  return true;
}

void SysKeyboardDriver::Close(void)
{
#ifdef DO_DINPUT_KEYBOARD
	::SetEvent(m_hEvent);
	::CloseHandle(m_hEvent);
	::WaitForSingleObject(m_hThread,1000);
	::CloseHandle(m_hThread);
#endif
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

#ifndef DO_DINPUT_KEYBOARD
inline void WinKeyTrans(csSystemDriver* pSystemDriver, WPARAM wParam, 
                        bool /*shift*/, bool /*alt*/, bool /*ctrl*/, bool down)
{
  if(!pSystemDriver) return;
  
  // handle standard alphabetical.
/*    if((wParam >= 'A') && (wParam <= 'z'))
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
#endif

long FAR PASCAL WindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
#ifndef DO_DINPUT_KEYBOARD
  bool shift=false, ctrl=false;
  
  if(GetAsyncKeyState(VK_CONTROL)) ctrl=true;
  if(GetAsyncKeyState(VK_SHIFT)) shift=true;
#endif
  
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

#ifndef DO_DINPUT_KEYBOARD
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
#endif
    
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, 
                   LPSTR /*lpCmdLine*/, int nCmdShow )
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

