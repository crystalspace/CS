/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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

#include "cssysdef.h"
#include "cssys/win32/win32.h"
#include "iutil/cfgmgr.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/cmdline.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include <stdarg.h>
#include <windows.h>

#ifdef DO_DINPUT_KEYBOARD
#include <dinput.h>
#endif

#include <stdio.h>
#include <time.h>

#if defined(COMP_VC)
#include <sys/timeb.h>
#endif

#if defined(COMP_BC)
#include <dos.h> // For _argc & _argv
#endif

#if defined(__CYGWIN__)
// Cygwin doesn't understand _argc or _argv, so we define them here.
// These are borrowed from Mingw32 includes (see stdlib.h)
// Cygwin Purists, forgive the corruption, Cygwin means Cygnus for Win32.
extern int _argc;
extern char** _argv;
#endif

bool ApplicationActive = true;
extern HINSTANCE ModuleHandle;
extern int ApplicationShow;
extern bool need_console;

void SystemFatalError (char *s)
{
  MessageBox(NULL, s, "Fatal Error", MB_OK | MB_ICONSTOP);
  exit (1);
}

#ifdef DO_DINPUT_KEYBOARD

/*
 * This table performs the translation from keycode to Crystal Space key code.
 */
static unsigned short ScanCodeToChar [256] =
{
  0,              CSKEY_ESC,      '1',            '2',
  '3',            '4',            '5',            '6',		// 00..07
  '7',            '8',            '9',            '0',
  '-',            '=',            CSKEY_BACKSPACE,CSKEY_TAB,	// 08..0F
  'q',            'w',            'e',            'r',
  't',            'y',            'u',            'i',		// 10..17
  'o',            'p',            '[',            ']',
  CSKEY_ENTER,    CSKEY_CTRL,     'a',            's',		// 18..1F
  'd',            'f',            'g',            'h',
  'j',            'k',            'l',            ';',		// 20..27
  39,             '`',            CSKEY_SHIFT,    '\\',
  'z',            'x',            'c',            'v',		// 28..2F
  'b',            'n',            'm',            ',',
  '.',            '/',            CSKEY_SHIFT,    CSKEY_PADMULT,// 30..37
  CSKEY_ALT,      ' ',            0,              CSKEY_F1,
  CSKEY_F2,       CSKEY_F3,       CSKEY_F4,       CSKEY_F5,	// 38..3F
  CSKEY_F6,       CSKEY_F7,       CSKEY_F8,       CSKEY_F9,
  CSKEY_F10,      0,              0,              CSKEY_HOME,	// 40..47
  CSKEY_UP,       CSKEY_PGUP,     CSKEY_PADMINUS, CSKEY_LEFT,
  CSKEY_CENTER,   CSKEY_RIGHT,    CSKEY_PADPLUS,  CSKEY_END,	// 48..4F
  CSKEY_DOWN,     CSKEY_PGDN,     CSKEY_INS,      CSKEY_DEL,
  0,              0,              0,              CSKEY_F11,	// 50..57
  CSKEY_F12,      0,              0,              0,
  0,              0,              0,              0,		// 58..5F
  0,              0,              0,              0,
  0,              0,              0,              0,		// 60..67
  0,              0,              0,              0,
  0,              0,              0,              0,		// 68..6F
  0,              0,              0,              0,
  0,              0,              0,              0,		// 70..77
  0,              0,              0,              0,
  0,              0,              0,              0,		// 78..7F
  0,              0,              0,              0,
  0,              0,              0,              0,		// 80..87
  0,              0,              0,              0,
  0,              0,              0,              0,		// 88..8F
  0,              0,              0,              0,
  0,              0,              0,              0,		// 90..97
  0,              0,              0,              0,
  CSKEY_ENTER,    CSKEY_CTRL,     0,              0,		// 98..9F
  0,              0,              0,              0,
  0,              0,              0,              0,		// A0..A7
  0,              0,              0,              0,
  0,              0,              0,              0,		// A8..AF
  0,              0,              0,              ',',
  0,              CSKEY_PADDIV,   0,              0,		// B0..B7
  CSKEY_ALT,      0,              0,              0,
  0,              0,              0,              0,		// B8..BF
  0,              0,              0,              0,
  0,              0,              0,              CSKEY_HOME,	// C0..C7
  CSKEY_UP,       CSKEY_PGUP,     0,              CSKEY_LEFT,
  0,              CSKEY_RIGHT,    0,              CSKEY_END,	// C8..CF
  CSKEY_DOWN,     CSKEY_PGDN,     CSKEY_INS,      CSKEY_DEL,
  0,              0,              0,              0,		// D0..D7
  0,              0,              0,              0,
  0,              0,              0,              0,		// D8..DF
  0,              0,              0,              0,
  0,              0,              0,              0,		// E0..E7
  0,              0,              0,              0,
  0,              0,              0,              0,		// E8..EF
  0,              0,              0,              0,
  0,              0,              0,              0,		// F0..F7
  0,              0,              0,              0,
  0,              0,              0,              0		// F8..FF
};

// This macro is for COM calls. If it fails, it shows a MessageBox then
// kills the whole process. It's brutal, but as I use another thread,
// it's safer this way
#define CHK_FAILED(x) \
  { if(FAILED(x)) { MessageBox(NULL, #x " Failed!", NULL,MB_OK|MB_ICONERROR); ::ExitProcess(1); } }
// This macro is for COM Release calls
#define CHK_RELEASE(x) \
  { if((x) != NULL) { (x)->Release(); (x)=NULL; } }

/*
 * The thread entry point. Called by ::Open()
 */

#define AUTOREPEAT_WAITTIME 1000 // 1 seconde
#define AUTOREPEAT_TIME      100 // 10 keystroke/seconds

DWORD WINAPI s_threadroutine (LPVOID param)
{
  iEventOutlet *EventOutlet = (iEventOutlet *)param;
  HRESULT hr;
  DWORD dwWait = INFINITE;
  char *buffer;
  int i,lastkey = -1;
#ifndef DI_USEGETDEVICEDATA
  char *oldbuffer = NULL;
#endif
  LPDIRECTINPUT lpdi = NULL;
  LPDIRECTINPUTDEVICE lpKbd = NULL;
  //Setup for directinput mouse code
  LPDIRECTINPUTDEVICE lpMouse = NULL;

  HANDLE hEvent [2];

  CHK_FAILED (DirectInputCreate (ModuleHandle, DIRECTINPUT_VERSION, &lpdi, NULL));
  CHK_FAILED (lpdi->CreateDevice (GUID_SysKeyboard, &lpKbd, NULL));
  CHK_FAILED (lpKbd->SetDataFormat (&c_dfDIKeyboard));
  CHK_FAILED (lpKbd->SetCooperativeLevel (FindWindow (WINDOWCLASSNAME, NULL),
    DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));

  //Setup for directinput mouse code
#if 0
  CHK_FAILED(lpdi->CreateDevice (GUID_SysMouse, &lpMouse, NULL));
  CHK_FAILED(lpMouse->SetDataFormat(&c_dfDIMouse));
  CHK_FAILED(lpMouse->SetCooperativeLevel(FindWindow (WINDOWCLASSNAME, NULL),
    DISCL_EXCLUSIVE | DISCL_FOREGROUND));
  hevtMouse = CreateEvent(0, 0, 0, 0);
  CHK_FAILED(lpMouse->SetEventNotification(g_hevtMouse));
  DIPROPDWORD dipdw =
      {
          {
              sizeof(DIPROPDWORD),        // diph.dwSize
              sizeof(DIPROPHEADER),       // diph.dwHeaderSize
              0,                          // diph.dwObj
              DIPH_DEVICE,                // diph.dwHow
          },
          DINPUT_BUFFERSIZE,              // dwData
      };
  CHK_FAILED(lpMouse->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph));
#endif

#ifdef DI_USEGETDEVICEDATA
  {
    DIPROPDWORD dpd;
    dpd.diph.dwSize=sizeof(DIPROPDWORD);
    dpd.diph.dwHeaderSize=sizeof(DIPROPHEADER);
    dpd.diph.dwObj=0;
    dpd.diph.dwHow=DIPH_DEVICE;
    dpd.dwData=10; // The size of the buffer (should be more than sufficient)
#if DIRECTINPUT_VERSION < 0x0700
    CHK_FAILED (lpKbd->SetProperty (DIPROP_BUFFERSIZE, &dpd));
#else
    //For incomprehensible reason, SetProperty() parameters type has
    //changed between DX6.1 and DX7 SDK
    CHK_FAILED (lpKbd->SetProperty (DIPROP_BUFFERSIZE, &dpd.diph));
#endif
  }
#endif
  hEvent [0] = CreateEvent (NULL, FALSE, FALSE, NULL);
  if (hEvent [0] == NULL)
  {
    MessageBox (NULL, "CreateEvent() Failed!", NULL, MB_OK|MB_ICONERROR);
    ExitProcess (1);
  }
  if (!DuplicateHandle (GetCurrentProcess(), ((SysSystemDriver*)System)->m_hEvent, 
                        GetCurrentProcess (), &hEvent [1], 0, FALSE, DUPLICATE_SAME_ACCESS))
  {
    MessageBox (NULL, "DuplicateEvent() Failed!", NULL, MB_OK|MB_ICONERROR);
    ExitProcess (1);
  }

  hr = lpKbd->SetEventNotification (hEvent [0]);
  switch (hr)
  {
    case DI_OK:
      break;
    default:
      MessageBox (NULL, "lpKbd->SetEventNotification(hEvent) Failed!", NULL,
        MB_OK|MB_ICONERROR);
      ExitProcess (1);
      break;
  }

  while (1)
  {
    hr = lpKbd->Acquire ();
    if (SUCCEEDED (hr))
      break;
    if (WaitForSingleObject (hEvent [1], 0) == WAIT_OBJECT_0 + 1)
    {
      CloseHandle (hEvent [0]);
      CloseHandle (hEvent [1]);
      CHK_RELEASE (lpKbd);
      CHK_RELEASE (lpdi);
#ifndef DI_USEGETDEVICEDATA
      if (oldbuffer) delete[] oldbuffer;
#endif
      return 0;
    }
  }

#ifndef DI_USEGETDEVICEDATA
  oldbuffer = new char [256];
  hr = lpKbd->GetDeviceState (256, oldbuffer);
#endif
  while (1)
  {
    switch (WaitForMultipleObjects (2, hEvent, FALSE, dwWait))
    {
      case WAIT_OBJECT_0:
#ifndef DI_USEGETDEVICEDATA
        buffer = new char [256];
        do
        {
          hr = lpKbd->GetDeviceState (256, buffer);
          switch (hr)
          {
            case DIERR_NOTACQUIRED:
            case DIERR_INPUTLOST:
              lpKbd->Acquire ();
              break;
            case DI_OK:
              break;
            default:
              MessageBox (NULL, "lpKbd->GetDeviceState(hEvent) Failed!",
                NULL, MB_OK|MB_ICONERROR);
              ExitProcess (1);
              break;
          }
        } while (hr != DI_OK);
        for (i = 0; i < 256; i++)
          if (oldbuffer [i] != buffer [i])
          {
            if (buffer [i] & 0X80)
            {
              lastkey = i;
              dwWait = AUTOREPEAT_WAITTIME;
              EventOutlet->Key (ScanCodeToChar [i], -1, true);
            }
            else
            {
              lastkey = -1;
              dwWait = INFINITE;
              EventOutlet->Key (ScanCodeToChar[i], -1, false);
            }
            break;
          }
        delete [] oldbuffer;
        oldbuffer = buffer;
#else
        DIDEVICEOBJECTDATA *lpdidod;
        DWORD dwNb;
        do
        {
          dwNb = INFINITE;
          hr = lpKbd->GetDeviceData (sizeof (DIDEVICEOBJECTDATA), NULL, &dwNb,DIGDD_PEEK);
          switch(hr)
          {
            case DIERR_NOTACQUIRED:
            case DIERR_INPUTLOST:
              lpKbd->Acquire ();
              break;
            case DI_OK:
              break;
            case DI_BUFFEROVERFLOW:
              hr = DI_OK;
              break;
            default:
              MessageBox(NULL, "lpKbd->GetDeviceState(hEvent) Failed!",
                NULL, MB_OK|MB_ICONERROR);
              ExitProcess (1);
              break;
          }
        } while (hr != DI_OK);
        if (!dwNb)
          continue;
        lpdidod = new DIDEVICEOBJECTDATA [dwNb];
        CHK_FAILED (lpKbd->GetDeviceData (sizeof (DIDEVICEOBJECTDATA),
          lpdidod, &dwNb, 0));
        for (i = 0; i < dwNb; i++)
        {
          if (lpdidod [i].dwData & 0X80)
          {
            lastkey = lpdidod [i].dwOfs;
            dwWait = AUTOREPEAT_WAITTIME:
            EventOutlet->Key (ScanCodeToChar [lpdidod [i].dwOfs], -1, true);
          }
          else
          {
            lastkey = -1;
            dwWait = INFINITE;
            EventOutlet->Key (ScanCodeToChar [lpdidod [i].dwOfs], -1, false);
          }
        }
        delete[] lpdidod;
#endif
        break;
      case WAIT_TIMEOUT:  // HANDLE key autorepeat
        buffer = new char [256];
        do
        {
          hr = lpKbd->GetDeviceState (256, buffer);
          switch (hr)
          {
            case DIERR_NOTACQUIRED:
            case DIERR_INPUTLOST:
              lpKbd->Acquire ();
              break;
            case DI_OK:
              break;
            default:
              MessageBox (NULL, "lpKbd->GetDeviceState(hEvent) Failed!",
                NULL, MB_OK|MB_ICONERROR);
              ExitProcess (1);
              break;
          }
        } while (hr != DI_OK);
        // The lastkey is still pressed
        if ((lastkey >= 0) && (buffer [lastkey] & 0X80))
        {
          dwWait = AUTOREPEAT_TIME;
          EventOutlet->Key (ScanCodeToChar [lastkey], -1, true);
        }
        else
        { // Strange.. we didn't get the message that the key was released !
          lastkey = -1;
          dwWait = INFINITE;
        }
        delete [] buffer;
        break;
      case WAIT_OBJECT_0 + 1:
        lpKbd->Unacquire ();
        CloseHandle (hEvent [0]);
        CloseHandle (hEvent [1]);
        CHK_RELEASE (lpKbd);
        CHK_RELEASE (lpdi);
#ifndef DI_USEGETDEVICEDATA
        if (oldbuffer)
          delete [] oldbuffer;
#endif
        return 0;
    }
  }
}

#undef CHK_RELEASE
#undef CHK_FAILED
#endif // DO_DINPUT_KEYBOARD

class Win32Assistant :
  public iWin32Assistant,
  public iEventPlug,
  public iEventHandler
{
private:
  iObjectRegistry* registry;
  bool need_console;
  HCURSOR m_hCursor;
  iEventOutlet* EventOutlet;
  void SetWinCursor (HCURSOR);
  iEventOutlet* GetEventOutlet();
  static long FAR PASCAL WindowProc (HWND hWnd, UINT message,
    WPARAM wParam, LPARAM lParam);
#ifdef DO_DINPUT_KEYBOARD
  HANDLE m_hEvent;
  HANDLE m_hThread;
  friend DWORD WINAPI s_threadroutine(LPVOID param);
#endif
public:
  SCF_DECLARE_IBASE;
  Win32Assistant (iObjectRegistry*);
  virtual ~Win32Assistant ();
  virtual void Shutdown();
  virtual HINSTANCE GetInstance () const;
  virtual bool GetIsActive () const;
  virtual int GetCmdShow () const;
  virtual bool SetCursor (int cursor);
  virtual bool HandleEvent (iEvent&);
  virtual unsigned GetPotentiallyConflictingEvents ();
  virtual unsigned QueryEventPriority (unsigned);
};

static Win32Assistant* GLOBAL_ASSISTANT = 0;

SCF_IMPLEMENT_IBASE (Win32Assistant)
  SCF_IMPLEMENTS_INTERFACE (iWin32Assistant)
  SCF_IMPLEMENTS_INTERFACE (iEventPlug)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

bool csPlatformStartup(iObjectRegistry* r)
{
  Win32Assistant* a = new Win32Assistant(r);
  bool ok = r->Register (static_cast<iWin32Assistant*>(a), "iWin32Assistant");
  if (ok)
    GLOBAL_ASSISTANT = a;
  else
  {
    a->DecRef();
    fprintf (stderr, "FATAL: Failed to register iWin32Assistant!\n");
  }
  return ok;
}

bool csPlatformShutdown(iObjectRegistry* r)
{
  if (GLOBAL_ASSISTANT != 0)
  {
    r->Unregister(
      static_cast<iWin32Assistant*>(GLOBAL_ASSISTANT), "iWin32Assistant");
    GLOBAL_ASSISTANT->Shutdown();
    GLOBAL_ASSISTANT->DecRef();
    GLOBAL_ASSISTANT = 0;
  }
  return true;
}

Win32Assistant::Win32Assistant (iObjectRegistry* r) :
#ifdef CS_DEBUG
  need_console(true), 
#else
  need_console(false), 
#endif
  EventOutlet(0)
{
  SCF_CONSTRUCT_IBASE(0);

  iCommandLineParser* cmdline = CS_QUERY_REGISTRY (r,
						   iCommandLineParser);
  if (cmdline->GetOption ("console")) need_console = true;
  if (cmdline->GetOption ("noconsole")) need_console = false;
  cmdline->DecRef ();

  if (need_console)
  {
    // @@@ if we are started from the command prompt, is there a way
    //     to use that console ?
    AllocConsole();
    freopen("CONOUT$", "a", stderr); // Redirect stderr to console   
    freopen("CONOUT$", "a", stdout); // Redirect stdout to console   
  }

  registry = r;
  registry->IncRef();

  if (ModuleHandle == NULL)
    ModuleHandle = GetModuleHandle(NULL);

  WNDCLASS wc;
  wc.hCursor        = NULL;
  // try the app icon...
  wc.hIcon          = LoadIcon (ModuleHandle, MAKEINTRESOURCE(1));
  // not?
  if (!wc.hIcon) wc.hIcon = LoadIcon (NULL, IDI_APPLICATION);
  wc.lpszMenuName   = NULL;
  wc.lpszClassName  = CS_WIN32_WINDOW_CLASS_NAME;
  wc.hbrBackground  = 0;
  wc.hInstance      = ModuleHandle;
  wc.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wc.lpfnWndProc    = WindowProc;
  wc.cbClsExtra     = 0;
  wc.cbWndExtra     = 0;

  bool bResult;
  bResult = RegisterClass (&wc);
  ASSERT (bResult);
  m_hCursor = LoadCursor (0, IDC_ARROW);

  iEventQueue* q = CS_QUERY_REGISTRY (registry, iEventQueue);
  CS_ASSERT (q != NULL);
  q->RegisterListener (this, CSMASK_Nothing | CSMASK_Broadcast);
  q->DecRef ();
}

Win32Assistant::~Win32Assistant ()
{
  if (EventOutlet != 0)
    EventOutlet->DecRef();
  if (need_console)
    FreeConsole();
}

void Win32Assistant::Shutdown()
{
  iEventQueue* q = CS_QUERY_REGISTRY (registry, iEventQueue);
  if (q != 0)
  {
    q->RemoveListener(this);
    q->DecRef ();
  }
}

void Win32Assistant::SetWinCursor (HCURSOR cur)
{
  m_hCursor = cur;
  ::SetCursor (cur);
}

unsigned Win32Assistant::GetPotentiallyConflictingEvents ()
  { return CSEVTYPE_Keyboard | CSEVTYPE_Mouse; }
unsigned Win32Assistant::QueryEventPriority (unsigned /*iType*/)
  { return 100; }

iEventOutlet* Win32Assistant::GetEventOutlet()
{
  if (EventOutlet == 0)
  {
    iEventQueue* q = CS_QUERY_REGISTRY(registry, iEventQueue);
    if (q != 0)
    {
      EventOutlet = q->CreateEventOutlet(this);
      q->DecRef ();
    }
  }
  return EventOutlet;
}

bool Win32Assistant::HandleEvent (iEvent& e)
{
  if (e.Type != csevBroadcast)
    return false;

  if (e.Command.Code == cscmdPreProcess)
  {
    MSG msg;
    while (PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
    {
      if (!GetMessage (&msg, NULL, 0, 0))
      {
        iEventOutlet* outlet = GetEventOutlet();
        outlet->Broadcast (cscmdQuit);
        return true;
      }
      TranslateMessage (&msg);
      DispatchMessage (&msg);
    }
    return true;
  }
  else if (e.Command.Code == cscmdSystemOpen)
  {
#   ifdef DO_DINPUT_KEYBOARD
    DWORD dwThreadId;
    m_hEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
    m_hThread =
      CreateThread (NULL, 0, s_threadroutine, EventOutlet, 0, &dwThreadId);
    if (!m_hEvent || !m_hThread)
    {
      MessageBox (NULL, "CreateEvent() Failed!", NULL, MB_OK|MB_ICONERROR);
      ExitProcess (1);
    }
#   endif
    return true;
  }
  else if (e.Command.Code == cscmdSystemClose)
  {
    ChangeDisplaySettings (NULL, 0);

#   ifdef DO_DINPUT_KEYBOARD
    if (m_hEvent)
    {
      SetEvent (m_hEvent);
      CloseHandle (m_hEvent);
      m_hEvent = NULL;
      WaitForSingleObject (m_hThread, 1000);
      CloseHandle (m_hThread);
      m_hThread = NULL;
    }
#   endif
  }
  return false;
}

#if 0
// @@@ MOVE THIS CODE TO THE WINDOWS CANVASES!
void Win32Assistant::Alert (const char* s)
{
  bool FullScreen = false;
  //int width, height, depth;

// @@@ IMPORTANT: FIX ME: HAVE TO GO TO CANVAS FOR THIS!!!
  //GetSettings(width, height, depth, FullScreen);

  if (FullScreen)
  {
    // If fullscreen mode is active, we switch to default screen, because
    // otherwise this message will not be seen.
    ChangeDisplaySettings (NULL, 0);
  }

  MessageBox (NULL, s, "Fatal Error", MB_OK | MB_ICONSTOP);
  DebugTextOut (true, s);
}
#endif

HINSTANCE Win32Assistant::GetInstance () const
{
  return ModuleHandle;
}

bool Win32Assistant::GetIsActive () const
{
  return ApplicationActive;
}

int Win32Assistant::GetCmdShow () const
{
  return ApplicationShow;
}

#ifndef DO_DINPUT_KEYBOARD

//----------------------------------------// Windows input translator //------//

#define MAX_SCANCODE 0x3c

/*
    This table does not contain special key codes, since those are
    handled by the switch() in WinKeyTrans().
*/

static unsigned char ScanCodeToChar [MAX_SCANCODE] =
{
  0,              0,              '1',            '2',
  '3',            '4',            '5',            '6',		// 00..07
  '7',            '8',            '9',            '0',
  '-',            '=',            0,              0,		// 08..0F
  'q',            'w',            'e',            'r',
  't',            'y',            'u',            'i',		// 10..17
  'o',            'p',            '[',            ']',
  0,              0,              'a',            's',		// 18..1F
  'd',            'f',            'g',            'h',
  'j',            'k',            'l',            ';',		// 20..27
  39,             '`',            0,              '\\',
  'z',            'x',            'c',            'v',		// 28..2F
  'b',            'n',            'm',            ',',
  '.',            '/',            0,              0,            // 30..37
  0,              ' ',            0,              0
};

static unsigned char LastCharCode [MAX_SCANCODE];

static void WinKeyTrans (WPARAM wParam, bool down, iEventOutlet* outlet)
{
  int key = 0, chr = 0;
  switch (wParam)
  {
    case VK_MENU:     key = CSKEY_ALT; break;
    case VK_CONTROL:  key = CSKEY_CTRL; break;
    case VK_SHIFT:    key = CSKEY_SHIFT; break;
    case VK_UP:       key = CSKEY_UP; break;
    case VK_DOWN:     key = CSKEY_DOWN; break;
    case VK_LEFT:     key = CSKEY_LEFT; break;
    case VK_RIGHT:    key = CSKEY_RIGHT; break;
    case VK_CLEAR:    key = CSKEY_CENTER; break;
    case VK_INSERT:   key = CSKEY_INS; break;
    case VK_DELETE:   key = CSKEY_DEL; break;
    case VK_PRIOR:    key = CSKEY_PGUP; break;
    case VK_NEXT:     key = CSKEY_PGDN; break;
    case VK_HOME:     key = CSKEY_HOME; break;
    case VK_END:      key = CSKEY_END; break;
    case VK_RETURN:   key = CSKEY_ENTER; chr = '\n'; break;
    case VK_BACK:     key = CSKEY_BACKSPACE; chr = '\b'; break;
    case VK_TAB:      key = CSKEY_TAB; chr = '\t'; break;
    case VK_ESCAPE:   key = CSKEY_ESC; chr = 27; break;
    case VK_F1:       key = CSKEY_F1; break;
    case VK_F2:       key = CSKEY_F2; break;
    case VK_F3:       key = CSKEY_F3; break;
    case VK_F4:       key = CSKEY_F4; break;
    case VK_F5:       key = CSKEY_F5; break;
    case VK_F6:       key = CSKEY_F6; break;
    case VK_F7:       key = CSKEY_F7; break;
    case VK_F8:       key = CSKEY_F8; break;
    case VK_F9:       key = CSKEY_F9; break;
    case VK_F10:      key = CSKEY_F10; break;
    case VK_F11:      key = CSKEY_F11; break;
    case VK_F12:      key = CSKEY_F12; break;
    case VK_ADD:      key = CSKEY_PADPLUS; chr = '+'; break;
    case VK_SUBTRACT: key = CSKEY_PADMINUS; chr = '-'; break;
    case VK_MULTIPLY: key = CSKEY_PADMULT; chr = '*'; break;
    case VK_DIVIDE:   key = CSKEY_PADDIV; chr = '/'; break;
  }

  if (key)
    outlet->Key (key, chr, down);
}
#endif

long FAR PASCAL Win32Assistant::WindowProc (HWND hWnd, UINT message,
  WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    case WM_ACTIVATEAPP:
      ApplicationActive = wParam;
#ifndef DO_DINPUT_KEYBOARD
      memset (&LastCharCode, 0, sizeof (LastCharCode));
#endif
      break;
    case WM_ACTIVATE:
      if (GLOBAL_ASSISTANT != 0)
      {
	iEventOutlet* outlet = GLOBAL_ASSISTANT->GetEventOutlet();
        outlet->Broadcast (cscmdFocusChanged,
          (void *)(LOWORD (wParam) != WA_INACTIVE));
      }
      break;
    case WM_DESTROY:
      PostQuitMessage (0);
      break;
#ifndef DO_DINPUT_KEYBOARD
    case WM_SYSCHAR:
    case WM_CHAR:
    {
      if (GLOBAL_ASSISTANT != 0)
      {
        int scancode = (lParam >> 16) & 0xff;
        int key = (scancode < MAX_SCANCODE) ? ScanCodeToChar [scancode] : 0;
        if (key || (wParam >= ' '))
        {
	  iEventOutlet* outlet = GLOBAL_ASSISTANT->GetEventOutlet();
          outlet->Key (key, wParam, true);
          LastCharCode [scancode] = wParam;
        }
      }
      break;
    }
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    {
      if (GLOBAL_ASSISTANT != 0)
      {
        iEventOutlet* outlet = GLOBAL_ASSISTANT->GetEventOutlet();
        WinKeyTrans (wParam, true, outlet);
        if (wParam == VK_MENU) return 0;
      }
      break;
    }
    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
      if (GLOBAL_ASSISTANT != 0)
      {
        iEventOutlet* outlet = GLOBAL_ASSISTANT->GetEventOutlet();
        WinKeyTrans (wParam, false, outlet);
        if (wParam == VK_MENU) return 0;
        // Check if this is the keyup event for a former WM_CHAR
        int scancode = (lParam >> 16) & 0xff;
        if ((scancode < MAX_SCANCODE) && LastCharCode [scancode])
        {
          int key = (scancode < MAX_SCANCODE) ? ScanCodeToChar [scancode] : 0;
          outlet->Key (key, LastCharCode [scancode], false);
          LastCharCode [scancode] = 0;
        }
      }
      break;
    }
#endif
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    {
      if (GLOBAL_ASSISTANT != 0)
      {
        SetCapture (hWnd);
        iEventOutlet* outlet = GLOBAL_ASSISTANT->GetEventOutlet();
        outlet->Mouse ((message == WM_LBUTTONDOWN) ? 1 :
          (message == WM_RBUTTONDOWN) ? 2 : 3, true,
          short (LOWORD (lParam)), short (HIWORD (lParam)));
      }
      return TRUE;
    }
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    {
      if (GLOBAL_ASSISTANT != 0)
      {
        iEventOutlet* outlet = GLOBAL_ASSISTANT->GetEventOutlet();
        ReleaseCapture ();
        outlet->Mouse ((message == WM_LBUTTONUP) ? 1 :
          (message == WM_RBUTTONUP) ? 2 : 3, false,
          short (LOWORD (lParam)), short (HIWORD (lParam)));
      }
      return TRUE;
    }
    case WM_MOUSEMOVE:
    {
      if (GLOBAL_ASSISTANT != 0)
      {
        iEventOutlet* outlet = GLOBAL_ASSISTANT->GetEventOutlet();
        ::SetCursor (GLOBAL_ASSISTANT->m_hCursor);
        outlet->Mouse (0, false, short(LOWORD(lParam)), short(HIWORD(lParam)));
      }
      return TRUE;
    }
  }
  return DefWindowProc (hWnd, message, wParam, lParam);
}

void csSleep (int SleepTime)
{
  Sleep (SleepTime);
}

#if 0
//@@@ THIS PART OF CONFIG HELP IS CURRENTLY BROKEN!!!
void Win32Assistant::Win32Assistant (iConfigManager *Config)
{
#ifdef CS_DEBUG
  need_console = true;
  if (need_console)
  {
    AllocConsole();
    freopen("CONOUT$", "a", stderr); // Redirect stderr to console   
    freopen("CONOUT$", "a", stdout); // Redirect stdout to console   
  }
#endif
}
#endif

#if 0
//@@@ THIS PART OF COMMANDLINE HELP IS CURRENTLY BROKEN!!!
void Win32Assistant::Help ()
{
  printf ("  -[no]console       Create a debug console (default = %s)\n",
    need_console ? "yes" : "no");
}
#endif

bool Win32Assistant::SetCursor (int cursor)
{
  char *CursorID;
  switch (cursor)
  {
    case csmcNone:     CursorID = (char *)-1;   break;
    case csmcArrow:    CursorID = IDC_ARROW;    break;
    case csmcCross:    CursorID = IDC_CROSS;	break;
    //case csmcPen:      CursorID = IDC_PEN;	break;
    case csmcPen:      CursorID = MAKEINTRESOURCE(32631);	break;
    case csmcMove:     CursorID = IDC_SIZEALL;  break;
    case csmcSizeNWSE: CursorID = IDC_SIZENWSE; break;
    case csmcSizeNESW: CursorID = IDC_SIZENESW; break;
    case csmcSizeNS:   CursorID = IDC_SIZENS;   break;
    case csmcSizeEW:   CursorID = IDC_SIZEWE;   break;
    case csmcStop:     CursorID = IDC_NO;       break;
    case csmcWait:     CursorID = IDC_WAIT;     break;
    default:           CursorID = 0;            break;
  }

  bool success;
  HCURSOR cur;
  if (CursorID)
  {
    cur = ((CursorID != (char *)-1)
    	? LoadCursor (NULL, CursorID)
	: NULL);
    success = true;
  }
  else
  {
    cur = NULL;
    success = false;
  }
  SetWinCursor (cur);
  return success;
}
