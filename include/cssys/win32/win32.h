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

#ifndef __CS_WIN32_H__
#define __CS_WIN32_H__

#ifdef COMP_BC
#  ifndef __CSOSDEFS_H__
#     error csosdefs.h should be included before win32.h
#  endif
#endif

#include "csutil/scf.h"
#include "cssys/win32/winhelp.h"
#include "iutil/event.h"
#include <objbase.h>

#include "cssys/system.h"
#include "csutil/csinput.h"

class SysSystemDriver;
struct iObjectRegistry;

/**
 * Implementation class for iWin32Helper.
 */
class Win32Helper : public iWin32Helper
{
private:
  SysSystemDriver* sys;

public:
  Win32Helper (SysSystemDriver* sys);
  virtual ~Win32Helper ();

  SCF_DECLARE_IBASE;
  virtual bool SetCursor (int cursor);
  virtual HINSTANCE GetInstance () const;
  virtual bool GetIsActive () const;
  virtual int GetCmdShow () const;
};

/// Windows system driver
class SysSystemDriver : public csSystemDriver, public iEventPlug
{
public:
  SysSystemDriver (iObjectRegistry* object_reg);
  virtual ~SysSystemDriver ();
  
  virtual bool HandleEvent(iEvent&);

  /// Returns the HINSTANCE of the program
  HINSTANCE GetInstance() const;
  /// Returns true if the program is 'active', false otherwise.
  bool GetIsActive() const;
  /// Gets the nCmdShow of the WinMain().
  int GetCmdShow() const;

  virtual bool Open ();
  virtual void Close ();

  /// @@@ CURRENTLY BROKEN!!!
  virtual void SetSystemDefaults (iConfigManager *Config);

  /// @@@ CURRENTLY BROKEN!!!
  virtual void Help ();

  /// Function for Win32Helper to set the cursor.
  void SetWinCursor (HCURSOR cur)
  {
    m_hCursor = cur;
    SetCursor (cur);
  }

  //------------------------ iEventPlug interface ---------------------------//
  SCF_DECLARE_IBASE_EXT (csSystemDriver);

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
