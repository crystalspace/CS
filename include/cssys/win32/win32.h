/*
    Copyright (C) 1998,1999 by Jorrit Tyberghein
  
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

#ifndef __CS_WIN32_H__
#define __CS_WIN32_H__

#if defined (OS_WIN32)
// Windows compilers has a different idea of program entry point behaviour,
// compared to other platform compilers.
#define main csMain
#endif

#ifdef COMP_BC
#  ifndef __CSOSDEFS_H__
#     error csosdefs.h should be included before win32.h
#  endif
#endif

#include "csutil/scf.h"
#include "cssys/win32/win32itf.h"
#include <objbase.h>

#if defined(COMP_VC)
#include "cssys/csinput.h"
#include "igraph2D.h"
#endif

#include "cssys/system.h"
#include "cssys/csinput.h"

/// Windows system driver
class SysSystemDriver :
  public csSystemDriver, public iWin32SystemDriver, public iEventPlug
{
public:
  SysSystemDriver ();
  virtual ~SysSystemDriver ();
  
  virtual void NextFrame ();
  virtual void Alert (const char* s);

  /// Implementation of iWin32SystemDriver interface.

  /// Returns the HINSTANCE of the program
  HINSTANCE GetInstance() const;
  /// Returns true if the program is 'active', false otherwise.
  bool GetIsActive() const;
  /// Gets the nCmdShow of the WinMain().
  int GetCmdShow() const;

  virtual bool Open (const char *Title);
  virtual void Close ();

  /// Perform extension function
  bool SystemExtension (const char *iCommand, ...);

  /// The system is idle: we can sleep for a while
  virtual void Sleep (int SleepTime);

  ///
  virtual void SetSystemDefaults (iConfigManager *Config);

  ///
  virtual void Help ();

  /// Get the installation path.
  virtual bool GetInstallPath (char *oInstallPath, size_t iBufferSize);

  //------------------------ iEventPlug interface ---------------------------//
  DECLARE_IBASE_EXT (csSystemDriver);

  virtual unsigned GetPotentiallyConflictingEvents ()
  { return CSEVTYPE_Keyboard | CSEVTYPE_Mouse; }
  virtual unsigned QueryEventPriority (unsigned /*iType*/)
  { return 100; }

private:
  static long FAR PASCAL WindowProc (HWND hWnd, UINT message,
    WPARAM wParam, LPARAM lParam);

  HCURSOR m_hCursor;
#ifdef DO_DINPUT_KEYBOARD
  HANDLE m_hEvent;
  HANDLE m_hThread;
  friend DWORD WINAPI s_threadroutine(LPVOID param);
#endif

  bool need_console;
};

#endif // __CS_WIN32_H__
