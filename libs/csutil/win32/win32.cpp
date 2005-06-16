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
#include <ctype.h>
#include "csutil/sysfunc.h"
#include "csutil/syspath.h"
#include "csutil/win32/win32.h"
#include "csutil/event.h"
#include "iutil/cfgmgr.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/cmdline.h"
#include "iutil/objreg.h"
#include "ivideo/natwin.h"
#include "ivideo/cursor.h"
#include <stdarg.h>

#include "csutil/win32/wintools.h"
#include "win32kbd.h"
#include "cachedll.h"

#include <windows.h>

#include <stdio.h>
#include <time.h>

#if defined(CS_COMPILER_MSVC)
#include <sys/timeb.h>
#endif

#if defined(CS_COMPILER_BCC)
#include <dos.h> // For _argc & _argv
#endif

#if defined(__CYGWIN__)
// Cygwin doesn't understand _argc or _argv, so we define them here.
// These are borrowed from Mingw32 includes (see stdlib.h)
// Cygwin Purists, forgive the corruption, Cygwin means Cygnus for Win32.
extern int CS_WIN32_ARGC;
extern char** CS_WIN32_ARGV;
#endif

void SystemFatalError (const char *s)
{
  ChangeDisplaySettings (0, 0);  // doesn't hurt
  csPrintfErr ("FATAL: %s\n", s);
  MessageBoxW (0, csCtoW (s), L"Fatal Error", MB_OK | MB_ICONSTOP);
}

#define MAX_SCANCODE 0x100

class Win32Assistant :
  public iWin32Assistant,
  public iEventPlug,
  public iEventHandler
{
private:
  bool ApplicationActive;
  HINSTANCE ModuleHandle;
  int ApplicationShow;
  /// Handle of the main window.
  HWND ApplicationWnd;

  iObjectRegistry* registry;
  /// is a console window to be displayed?
  bool console_window;
  /// is the binary linked as GUI or console app?
  bool is_console_app;
  /// is command line help requested?
  bool cmdline_help_wanted;
  /// use our own message loop
  bool use_own_message_loop;

  HCURSOR m_hCursor;
  csRef<iEventOutlet> EventOutlet;
  csRef<csWin32KeyboardDriver> kbdDriver;

  int mouseButtons;

  // @@@ The following aren't thread-safe. Prolly use TLS.
  /// The "Ok" button text passsed to Alert(),
  const char* msgOkMsg;
  /// Hook that will change the OK button text.
  HHOOK msgBoxOkChanger;

  /// Handle to the exception handler DLL.
  HANDLE exceptHandlerDLL;

  /// The console codepage that was set on program startup. WIll be restored on exit.
  //UINT oldCP;

  static LRESULT CALLBACK WindowProc (HWND hWnd, UINT message,
    WPARAM wParam, LPARAM lParam);
  static LRESULT CALLBACK CBTProc (int nCode, WPARAM wParam, LPARAM lParam);
public:
  SCF_DECLARE_IBASE;
  Win32Assistant (iObjectRegistry*);
  virtual ~Win32Assistant ();
  virtual void Shutdown();
  virtual HINSTANCE GetInstance () const;
  virtual bool GetIsActive () const;
  virtual int GetCmdShow () const;
  virtual bool SetCursor (int cursor);
  virtual bool SetHCursor (HCURSOR);
  virtual bool HandleEvent (iEvent&);
  virtual unsigned GetPotentiallyConflictingEvents ();
  virtual unsigned QueryEventPriority (unsigned);
  virtual void DisableConsole ();
  void AlertV (HWND window, int type, const char* title, 
    const char* okMsg, const char* msg, va_list args);
  virtual HWND GetApplicationWindow();

  virtual void UseOwnMessageLoop(bool ownmsgloop);
  virtual bool HasOwnMessageLoop();

  iEventOutlet* GetEventOutlet();

  /**
   * Handle a keyboard-related Windows message.
   */
  bool HandleKeyMessage (HWND hWnd, UINT message,
    WPARAM wParam, LPARAM lParam);
};

static Win32Assistant* GLOBAL_ASSISTANT = 0;

SCF_IMPLEMENT_IBASE (Win32Assistant)
  SCF_IMPLEMENTS_INTERFACE (iWin32Assistant)
  SCF_IMPLEMENTS_INTERFACE (iEventPlug)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

static inline void ToLower (char *dst, const char *src) 
{
  char *d=dst;
  const char *s=src;
  for(; *s; s++, d++) {
    *d = (char)tolower(*s);
  }
  *d=0;
}

static inline bool AddToPathEnv (csString dir, char **pathEnv)
{
  // check if installdir is in the path.
  bool gotpath = false;

  size_t dlen = dir.Length();
  // csGetInstallDir() might return "" (current dir)
  if (dlen != 0)
  {
    dir.Downcase();
  
    if (*pathEnv)
    {
      char *mypath = new char[strlen(*pathEnv) + 1];
      ToLower (mypath, *pathEnv);

      char* ppos = strstr (mypath, dir);
      while (!gotpath && ppos)
      {
        char* npos = strchr (ppos, ';');
        if (npos) *npos = 0;

        if ((strlen (ppos) == dlen) || (strlen (ppos) == dlen+1))
        {
	  if (ppos[dlen] == '\\') ppos[dlen] = 0;
	  if (!strcmp (ppos, dir))
	  {
	    // found it
	    gotpath = true;
	  }
        }
        ppos = npos ? strstr (npos+1, dir) : 0;
      }
      delete[] mypath;
    }

    if (!gotpath)
    {
      // put CRYSTAL path into PATH environment.
      char *newpath = new char[(*pathEnv?strlen(*pathEnv):0) + strlen(dir) + 2];
      strcpy (newpath, dir);
      strcat (newpath, ";");
      if (*pathEnv) strcat (newpath, *pathEnv);
      delete[] *pathEnv;
      *pathEnv = newpath;
      return true;
    }
  }
  return false;
}

typedef void (WINAPI * LPFNSETDLLDIRECTORYA)(LPCSTR lpPathName);

bool csPlatformStartup(iObjectRegistry* r)
{
  csRef<iCommandLineParser> cmdline (CS_QUERY_REGISTRY (r, iCommandLineParser));

  csPathsList* pluginpaths = csGetPluginPaths (cmdline->GetAppPath());

  /*
    When it isn't already in the PATH environment,
    the CS directory will be put there in front of all
    other paths.
    The idea is that DLLs required by plugins (e.g. zlib)
    which reside in the CS directory can be found by the
    OS even if the application is somewhere else.
   */
  bool needPATHpatch = true;

#if 0
  // @@@ doesn't seem to work in some cases?
  /*
    WinXP SP 1 has a nice function that does exactly that: setting
    a number of search paths for DLLs. However, it's WinXP SP 1+,
    so we have to check if it's available, and if not, patch the PATH
    env var.
   */
  cswinCacheDLL hKernel32 ("kernel32.dll");
  if (hKernel32 != 0)
  {
    LPFNSETDLLDIRECTORYA SetDllDirectoryA =
      (LPFNSETDLLDIRECTORYA)GetProcAddress (hKernel32, "SetDllDirectoryA");

    if (SetDllDirectoryA)
    {
      csString path;

      for (int i = 0; i < pluginpaths->Length(); i++)
      {
	if (((*pluginpaths)[i].path != 0) && (*((*pluginpaths)[i].path) != 0))
	{
	  path << (*pluginpaths)[i].path;
	  path << ';';
	}
      }

      if (path.Length () > 0) SetDllDirectoryA (path.GetData ());
      needPATHpatch = false;
    }
  }
#endif

  if (needPATHpatch)
  {
    char* pathEnv = csStrNew (getenv("PATH"));
    bool pathChanged = false;

    for (size_t i = 0; i < pluginpaths->Length(); i++)
    {
      // @@@ deal with path recursion here?
      if (AddToPathEnv ((*pluginpaths)[i].path, &pathEnv)) pathChanged = true;
    }
    if (pathChanged) SetEnvironmentVariable ("PATH", pathEnv);
    delete[] pathEnv;
  }

  delete pluginpaths;


  csRef<Win32Assistant> a = csPtr<Win32Assistant> (new Win32Assistant(r));
  bool ok = r->Register (static_cast<iWin32Assistant*>(a), "iWin32Assistant");
  if (ok)
  {
    a->IncRef ();
    GLOBAL_ASSISTANT = a;
  }
  else
  {
    SystemFatalError ("Failed to register iWin32Assistant!");
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

BOOL WINAPI ConsoleHandlerRoutine (DWORD dwCtrlType)
{
  switch (dwCtrlType)
  {
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
      {
	if (GLOBAL_ASSISTANT != 0)
	{
	  GLOBAL_ASSISTANT->GetEventOutlet()->ImmediateBroadcast (
	    cscmdQuit, 0);
	  return TRUE;
	}
      }
  }
  return FALSE;
}

Win32Assistant::Win32Assistant (iObjectRegistry* r) :
  ApplicationActive (true),
  ModuleHandle (0),
  ApplicationWnd (0),
  console_window (false),  
  is_console_app(false),
  cmdline_help_wanted(false),
  EventOutlet (0),
  mouseButtons(0)
{
  SCF_CONSTRUCT_IBASE(0);

  /*
    Load the exception handler DLL. In case of an OS exception
    (e.g. Access Violation) this DLL creates a report about
    where the crash happened. After that, the "Application Error"
    boy is shown as usual.

    The DLL is contained in the "mingw-utils" package and works
    for both MinGW and VC compiled binaries.
   */
  // @@@ Potentially useful, but seems to interfere with "real" debugging
  //exceptHandlerDLL = LoadLibraryEx ("exchndl.dll", 0, 
  //  LOAD_WITH_ALTERED_SEARCH_PATH);
  exceptHandlerDLL = 0;

  ModuleHandle = GetModuleHandle(0);
  STARTUPINFO startupInfo;
  GetStartupInfo (&startupInfo);
  if (startupInfo.dwFlags & STARTF_USESHOWWINDOW)
    ApplicationShow = startupInfo.wShowWindow;
  else
    ApplicationShow = SW_SHOWNORMAL;

// Cygwin has problems with freopen()
#if defined(CS_DEBUG) || defined(__CYGWIN__)
  console_window = true;
#else
  console_window = false;
#endif

  use_own_message_loop = true;

  csRef<iCommandLineParser> cmdline (CS_QUERY_REGISTRY (r, iCommandLineParser));
  console_window = cmdline->GetBoolOption ("console", console_window);

  cmdline_help_wanted = cmdline->GetOption ("help");

  /*
     to determine if we are actually a console app we look up
     the subsystem field in the PE header.
   */
  PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)ModuleHandle;
  PIMAGE_NT_HEADERS NTheader = (PIMAGE_NT_HEADERS)((uint8*)dosHeader + dosHeader->e_lfanew);
  if (NTheader->Signature == 0x00004550) // check for PE sig
  {
    is_console_app = 
      (NTheader->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI);
  }

  /*
    - console apps won't do anything about their console... yet. 
    - GUI apps will open a console window if desired.
   */
  if (!is_console_app)
  {
    if (console_window || cmdline_help_wanted)
    {
      AllocConsole ();
      freopen("CONOUT$", "a", stderr);
      freopen("CONOUT$", "a", stdout);
      freopen("CONIN$", "a", stdin);
    }
  }

  /*
    In case we're a console app, set up a control handler so
    console window closing, user logoff and system shutdown
    cause CS to properly shut down.
   */
  if (is_console_app || console_window || cmdline_help_wanted)
  {
    SetConsoleCtrlHandler (&ConsoleHandlerRoutine, TRUE);
  }

  // experimental UC stuff.
  // Retrieve old codepage.
  //oldCP = GetConsoleOutputCP ();
  // Try to set console codepage to UTF-8.
  // @@@ The drawback of UTF8 is that it requires a TrueType console
  // font to work properly. However, default is "raster font" :/
  // In this case, text output w/ non-ASCII chars will appear broken, tho
  // this is really a Windows issue. (see MS KB 99795)
  // @@@ Maybe a command line param to set a different con cp could be useful.
  // * Don't change the codepage, for now.
  //SetConsoleOutputCP (CP_UTF8);
  //

  registry = r;
  registry->IncRef();

  HICON appIcon;

  // try the app icon...
  appIcon = LoadIcon (ModuleHandle, MAKEINTRESOURCE (1));
  // not? maybe executable.ico?
  if (!appIcon) 
  {
    char apppath[MAX_PATH];
    GetModuleFileName (0, apppath, sizeof(apppath));

    char *dot = strrchr (apppath, '.');
    if (dot) 
    {
      strcpy (dot, ".ico");
    }
    else
    {
      strcat (apppath, ".ico");
    }
    appIcon = (HICON)LoadImage (ModuleHandle, apppath, IMAGE_ICON,
      0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
  }
  // finally the default one
  if (!appIcon) appIcon = LoadIcon (0, IDI_APPLICATION);

  bool bResult = false;
  HBRUSH backBrush = (HBRUSH)::GetStockObject (BLACK_BRUSH);
  if (cswinIsWinNT ())
  {
    WNDCLASSEXW wc;

    wc.cbSize	      = sizeof (wc);
    wc.hCursor        = 0;
    wc.hIcon          = appIcon;
    wc.lpszMenuName   = 0;
    wc.lpszClassName  = CS_WIN32_WINDOW_CLASS_NAMEW;
    wc.hbrBackground  = backBrush;
    wc.hInstance      = ModuleHandle;
    wc.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc    = WindowProc;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = 0;
    wc.hIconSm	      = 0;
    bResult = RegisterClassExW (&wc) != 0;
  }
  else
  {
    WNDCLASSEXA wc;

    wc.cbSize	      = sizeof (wc);
    wc.hCursor        = 0;
    wc.hIcon          = appIcon;
    wc.lpszMenuName   = 0;
    wc.lpszClassName  = CS_WIN32_WINDOW_CLASS_NAME;
    wc.hbrBackground  = backBrush;
    wc.hInstance      = ModuleHandle;
    wc.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc    = WindowProc;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = 0;
    wc.hIconSm	      = 0;
    bResult = RegisterClassExA (&wc) != 0;
  }

  if (!bResult)
  {
    SystemFatalError ("Failed to register window class!");
  }

  m_hCursor = LoadCursor (0, IDC_ARROW);

  csRef<iEventQueue> q (CS_QUERY_REGISTRY (registry, iEventQueue));
  CS_ASSERT (q != 0);
  q->RegisterListener (this, CSMASK_Nothing | CSMASK_Broadcast);

  // Put our own keyboard driver in place.
  kbdDriver.AttachNew (new csWin32KeyboardDriver (r));
  if (kbdDriver == 0)
  {
    SystemFatalError ("Failed to create keyboard driver!");
  }

  csRef<iBase> currentKbd = 
    CS_QUERY_REGISTRY_TAG (r, "iKeyboardDriver");
  if (currentKbd != 0)
  {
    // Bit hacky: remove old keyboard driver
    csRef<iEventHandler> eh = SCF_QUERY_INTERFACE (currentKbd, 
      iEventHandler);
    q->RemoveListener (eh);
    r->Unregister (currentKbd, "iKeyboardDriver");
  }
  r->Register (kbdDriver, "iKeyboardDriver");
}

Win32Assistant::~Win32Assistant ()
{
  registry->DecRef();
  //SetConsoleOutputCP (oldCP);
  if (!is_console_app && (console_window || cmdline_help_wanted))
    FreeConsole();
  if (exceptHandlerDLL != 0)
    FreeLibrary ((HMODULE)exceptHandlerDLL);
  if (cswinIsWinNT ())
    UnregisterClassW (CS_WIN32_WINDOW_CLASS_NAMEW, ModuleHandle);
  else
    UnregisterClassA (CS_WIN32_WINDOW_CLASS_NAME, ModuleHandle); 
  SCF_DESTRUCT_IBASE();
}

void Win32Assistant::Shutdown()
{
  csRef<iEventQueue> q (CS_QUERY_REGISTRY (registry, iEventQueue));
  if (q != 0)
    q->RemoveListener(this);
  if (!is_console_app && (cmdline_help_wanted || console_window))
  {
    csPrintf ("\nPress a key to close this window...");
    fflush (stdout);
    HANDLE hConsole;
    hConsole = CreateFile ("CONIN$", GENERIC_READ, FILE_SHARE_READ, 0, 
      OPEN_EXISTING, 0, 0);
    if (hConsole != 0)
    {
      INPUT_RECORD ir;
      DWORD events_read;
      do 
      {
	ReadConsoleInput (hConsole, &ir, 1, &events_read);
      } while ((events_read == 0) || (ir.EventType != KEY_EVENT));
      CloseHandle (hConsole);
    }
  }
}

bool Win32Assistant::SetHCursor (HCURSOR cur)
{
  m_hCursor = cur;
  ::SetCursor (cur);
  return true;
}

unsigned Win32Assistant::GetPotentiallyConflictingEvents ()
  { return CSEVTYPE_Keyboard | CSEVTYPE_Mouse; }
unsigned Win32Assistant::QueryEventPriority (unsigned /*iType*/)
  { return 100; }

iEventOutlet* Win32Assistant::GetEventOutlet()
{
  if (!EventOutlet.IsValid())
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY(registry, iEventQueue));
    if (q != 0)
      EventOutlet = q->CreateEventOutlet(this);
  }
  return EventOutlet;
}

bool Win32Assistant::HandleEvent (iEvent& e)
{
  if (e.Type != csevBroadcast)
    return false;

  if (csCommandEventHelper::GetCode (&e) == cscmdPreProcess)
  {
    if(use_own_message_loop)
    {
      MSG msg;
      /*
        [res] *W versions of the message queue functions exist,
        but they don't seem to make a difference.
      */
      while (PeekMessage (&msg, 0, 0, 0, PM_NOREMOVE))
      {
        if (!GetMessage (&msg, 0, 0, 0))
        {
          iEventOutlet* outlet = GetEventOutlet();
          outlet->Broadcast (cscmdQuit);
          return true;
        }
        TranslateMessage (&msg);
        DispatchMessage (&msg);
      }
    }
    return true;
  }
  else if (csCommandEventHelper::GetCode (&e) == cscmdSystemOpen)
  {
    return true;
  }
  else if (csCommandEventHelper::GetCode (&e) == cscmdSystemClose)
  {
  } 
  else if (csCommandEventHelper::GetCode (&e) == cscmdCommandLineHelp)
  {

   #ifdef CS_DEBUG 
    const char *defcon = "yes";
   #else
    const char *defcon = "no";
   #endif

    csPrintf ("Win32-specific options:\n");
    csPrintf ("  -[no]console       Create a debug console (default = %s)\n",     
      defcon);
  }
  return false;
}

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

//----------------------------------------// Windows input translator //------//

#ifndef WM_MOUSEWHEEL
#define	WM_MOUSEWHEEL	0x020a
#endif

#ifndef WM_XBUTTONDOWN
#define	WM_XBUTTONDOWN	0x020b
#endif

#ifndef WM_XBUTTONUP
#define	WM_XBUTTONUP	0x020c
#endif

LRESULT CALLBACK Win32Assistant::WindowProc (HWND hWnd, UINT message,
  WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    case WM_ACTIVATEAPP:
      if ((GLOBAL_ASSISTANT != 0))
      {
        if (wParam) 
	{ 
	  GLOBAL_ASSISTANT->ApplicationActive = true; 
	} 
	else 
	{ 
	  GLOBAL_ASSISTANT->ApplicationActive = false; 
	}
      }
      break;
    case WM_ACTIVATE:
      if ((GLOBAL_ASSISTANT != 0))
      {
	iEventOutlet* outlet = GLOBAL_ASSISTANT->GetEventOutlet();
        outlet->Broadcast (cscmdFocusChanged, LOWORD(wParam) != WA_INACTIVE);
      }
      break;
    case WM_CREATE:
      if (GLOBAL_ASSISTANT != 0)
      {
	GLOBAL_ASSISTANT->ApplicationWnd = hWnd;
	// a window is created. Hide the console window, if requested.
	if (GLOBAL_ASSISTANT->is_console_app && 
	  !GLOBAL_ASSISTANT->console_window)
	{
	  GLOBAL_ASSISTANT->DisableConsole ();
	}
      }
      break;
    case WM_SYSCOMMAND:
      if (wParam == SC_CLOSE)
      {
	PostQuitMessage (0);
	return TRUE;
      }
      break;
    case WM_SYSCHAR:
    case WM_CHAR:
    case WM_UNICHAR:
    case WM_DEADCHAR:
    case WM_SYSDEADCHAR:
    case WM_IME_COMPOSITION:
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
      if (GLOBAL_ASSISTANT != 0)
      {	  
	if (GLOBAL_ASSISTANT->HandleKeyMessage (hWnd, message, wParam,
	  lParam))
	{
	  return 0;
	}
      }
      break;
    }
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    {
      if (GLOBAL_ASSISTANT != 0)
      {
	const int buttonNum = (message == WM_LBUTTONDOWN) ? csmbLeft :
          (message == WM_RBUTTONDOWN) ? csmbRight : csmbMiddle;
        if (GLOBAL_ASSISTANT->mouseButtons == 0) SetCapture (hWnd);
	GLOBAL_ASSISTANT->mouseButtons |= 1 << (buttonNum - csmbLeft);

        iEventOutlet* outlet = GLOBAL_ASSISTANT->GetEventOutlet();
        outlet->Mouse (buttonNum, true,
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
	const int buttonNum = (message == WM_LBUTTONUP) ? csmbLeft :
          (message == WM_RBUTTONUP) ? csmbRight : csmbMiddle;
	GLOBAL_ASSISTANT->mouseButtons &= ~(1 << (buttonNum - csmbLeft));
        if (GLOBAL_ASSISTANT->mouseButtons == 0) ReleaseCapture ();

        iEventOutlet* outlet = GLOBAL_ASSISTANT->GetEventOutlet();
        outlet->Mouse (buttonNum, false,
          short (LOWORD (lParam)), short (HIWORD (lParam)));
      }
      return TRUE;
    }
    case WM_MOUSEWHEEL:
    {
      if (GLOBAL_ASSISTANT != 0)
      {
        iEventOutlet* outlet = GLOBAL_ASSISTANT->GetEventOutlet();
	int wheelDelta = (short)HIWORD (wParam);
	// @@@ Only emit events when WHEEL_DELTA wheel ticks accumulated?
	outlet->Mouse (wheelDelta > 0 ? csmbWheelUp : csmbWheelDown, true,
	  short (LOWORD (lParam)), short (HIWORD (lParam)));
	//outlet->Mouse (wheelDelta > 0 ? csmbWheelUp : csmbWheelDown, false,
	  //short (LOWORD (lParam)), short (HIWORD (lParam))); 
      }
      return 0;
    }
    case WM_XBUTTONUP:
    case WM_XBUTTONDOWN:
    {
      if (GLOBAL_ASSISTANT != 0)
      {
	bool down = (message == WM_XBUTTONDOWN);
	const int maxXButtons = 16; 
	  // XButton flags are stored in high word of lparam
	const int mbFlagsOffs = csmbMiddle; 
	  // Offset of bit num of mouseButtons

	int XButtons = HIWORD(wParam);

	if (down && (GLOBAL_ASSISTANT->mouseButtons == 0)) SetCapture (hWnd);

        iEventOutlet* outlet = GLOBAL_ASSISTANT->GetEventOutlet();
	for (int x = 0; x < maxXButtons; x++)
	{
	  if (XButtons & (1 << x))
	  {
	    int mbFlag = 1 << (x + mbFlagsOffs);
	    if (down && !(GLOBAL_ASSISTANT->mouseButtons & mbFlag))
	    {
	      GLOBAL_ASSISTANT->mouseButtons |= mbFlag;
	      outlet->Mouse (csmbExtra1 + x, true,
		short (LOWORD (lParam)), short (HIWORD (lParam)));
	    }
	    else if (!down && (GLOBAL_ASSISTANT->mouseButtons & mbFlag))
	    {
	      GLOBAL_ASSISTANT->mouseButtons &= ~mbFlag;
	      outlet->Mouse (csmbExtra1 + x, false,
		short (LOWORD (lParam)), short (HIWORD (lParam)));
	    }
	  }
	}
        if (!down && (GLOBAL_ASSISTANT->mouseButtons == 0)) ReleaseCapture ();
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
    case WM_SIZE:
    {
      if (GLOBAL_ASSISTANT != 0)
      {
	if ( (wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED) )
	{
          iEventOutlet* outlet = GLOBAL_ASSISTANT->GetEventOutlet();
	  outlet->Broadcast (cscmdCanvasExposed, 0);
	} 
	else if (wParam == SIZE_MINIMIZED) 
	{
          iEventOutlet* outlet = GLOBAL_ASSISTANT->GetEventOutlet();
	  outlet->Broadcast (cscmdCanvasHidden, 0);
	}
      }
      return TRUE;
    }
    case WM_SHOWWINDOW:
    {
      if (GLOBAL_ASSISTANT != 0)
      {
	if (wParam)
	{
          iEventOutlet* outlet = GLOBAL_ASSISTANT->GetEventOutlet();
	  outlet->Broadcast (cscmdCanvasExposed, 0);
	} 
	else
	{
          iEventOutlet* outlet = GLOBAL_ASSISTANT->GetEventOutlet();
	  outlet->Broadcast (cscmdCanvasHidden, 0);
	}
      }
      break;
    }
  }
  if (IsWindowUnicode (hWnd))
  {
    return DefWindowProcW (hWnd, message, wParam, lParam);
  }
  else
  {
    return DefWindowProcA (hWnd, message, wParam, lParam);
  }
}

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
    cur = ((CursorID != (char *)-1) ? LoadCursor (0, CursorID) : 0);
    success = true;
  }
  else
  {
    cur = 0;
    success = false;
  }
  SetHCursor (cur);
  return success;
}

void Win32Assistant::DisableConsole ()
{
  if (console_window)
  {
    console_window = false;
  }

  char apppath[MAX_PATH];

  GetModuleFileName (0, apppath, sizeof(apppath));

  char *dot = strrchr (apppath, '.');
  if (dot) 
  {
    strcpy (dot, ".txt");
  }
  else
  {
    strcat (apppath, ".txt");
  }

  freopen(apppath, "a", stderr);
  freopen(apppath, "a", stdout);
  FreeConsole();

  struct tm *now;
  time_t aclock;
  time( &aclock );
  now = localtime( &aclock );
  csPrintf("====== %s", asctime(now));
}

LRESULT Win32Assistant::CBTProc (int nCode, WPARAM wParam, LPARAM lParam)
{
  switch (nCode)
  {
    // when the MB is first activated we change the button text
  case HCBT_ACTIVATE:
    {
      // The MBs we request always have just one button (OK)
      HWND Button = FindWindowEx ((HWND)wParam, 0, "Button", 0);
      if (Button)
      {
	if (cswinIsWinNT ())
	{
          SetWindowTextW (Button, csCtoW (GLOBAL_ASSISTANT->msgOkMsg));
	}
	else
	{
          SetWindowTextA (Button, cswinCtoA (GLOBAL_ASSISTANT->msgOkMsg));
	}
      }
      LRESULT ret = CallNextHookEx (GLOBAL_ASSISTANT->msgBoxOkChanger,
	nCode, wParam, lParam);
      // The work is done, remove the hook.
      UnhookWindowsHookEx (GLOBAL_ASSISTANT->msgBoxOkChanger);
      GLOBAL_ASSISTANT->msgBoxOkChanger = 0;
      return ret;
    }
    break;
  }
  return CallNextHookEx (GLOBAL_ASSISTANT->msgBoxOkChanger,
    nCode, wParam, lParam);
}

void Win32Assistant::AlertV (HWND window, int type, const char* title, 
    const char* okMsg, const char* msg, va_list args)
{
  UINT style = MB_OK;

  if (type == CS_ALERT_ERROR)
    style |= MB_ICONERROR;
  else if (type == CS_ALERT_WARNING)
    style |= MB_ICONWARNING;
  else if (type == CS_ALERT_NOTE)
    style |= MB_ICONINFORMATION;

  msgBoxOkChanger = 0;
  /*
    To change the text of the "OK" button, we somehow have to get
    a handle to it, preferably before the user sees anything of
    the message. So a hook is set up.
   */
  if (okMsg != 0)
  {
    msgOkMsg = okMsg;
    msgBoxOkChanger = SetWindowsHookEx (WH_CBT,
      (HOOKPROC)&CBTProc, ModuleHandle, GetCurrentThreadId());
  }

  csString buf;
  buf.FormatV (msg, args);

  // No need to juggle with string conversions here, 
  // MessageBoxW() also exists on Win9x
  MessageBoxW (window, csCtoW (buf), csCtoW (title), style);

  /*
    Clean up in case it isn't already.
   */
  if (msgBoxOkChanger != 0)
  {
    UnhookWindowsHookEx (msgBoxOkChanger);
  }
}

HWND Win32Assistant::GetApplicationWindow()
{
  return ApplicationWnd;
}

void Win32Assistant::UseOwnMessageLoop(bool ownmsgloop)
{
  use_own_message_loop = ownmsgloop;
}

bool Win32Assistant::HasOwnMessageLoop()
{
  return use_own_message_loop;
}

bool Win32Assistant::HandleKeyMessage (HWND hWnd, UINT message, 
				       WPARAM wParam, LPARAM lParam)
{
  return kbdDriver->HandleKeyMessage (hWnd, message, wParam, lParam);
}
