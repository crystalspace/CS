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

#ifndef __CSOSDEFS_H__
#  error csosdefs.h should be included before "cssys/win32.h"
#endif

#include "csutil/scf.h"

#define CS_WIN32_WINDOW_CLASS_NAME "CrystalSpaceWin32"

SCF_VERSION (iWin32Assistant, 0, 0, 3);

/**
 * This interface describes actions specific to the Windows platform.
 * An instance of this object will be registered to the object registry
 * with tag `iWin32Assistant'.
 */
struct iWin32Assistant : public iBase
{
  /// Returns the HINSTANCE of the program
  virtual HINSTANCE GetInstance () const = 0;
  /// Returns true if the program is 'active', false otherwise.
  virtual bool GetIsActive () const = 0;
  /// Gets the nCmdShow of the WinMain().
  virtual int GetCmdShow () const = 0;
  /// Set the mouse shape.
  virtual bool SetCursor (int cursor) = 0;
  /// Disable the console window (if present)
  virtual void DisableConsole () = 0;
};

// @@@ Delete everything below when the system driver is removed.
#include "cssys/system.h"
struct iObjectRegistry;
class SysSystemDriver : public csSystemDriver
{
public:
  SysSystemDriver(iObjectRegistry* r) : csSystemDriver(r) {}
};

#endif // __CS_WIN32_H__
