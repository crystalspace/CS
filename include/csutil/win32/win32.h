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

/**\file
 * Win32-specific interfaces
 */

#ifndef __CS_CSOSDEFS_H__
#  error csosdefs.h should be included before "csutil/win32.h"
#endif

#include "csutil/scf.h"

/// CrystalSpace window class
#define CS_WIN32_WINDOW_CLASS_NAME  "CrystalSpaceWin32"
/// CrystalSpace window class (Unicode version)
#define CS_WIN32_WINDOW_CLASS_NAMEW L"CrystalSpaceWin32"

SCF_VERSION (iWin32Assistant, 0, 2, 0);

/**
 * This interface describes actions specific to the Windows platform.
 * \remarks As the name suggests, this interface provides functionality
 *  specific to the Win32 platform. It can be retrieved from the object
 *  registry via CS_QUERY_REGISTRY(), as an instance of this object will be 
 *  registered with the tag `iWin32Assistant'. To ensure that code using this 
 *  functionality compiles properly on all other platforms, the use of the
 *  interface and inclusion of the header file should be surrounded by
 *  appropriate `#if defined(CS_PLATFORM_WIN32) ... #endif' statements.
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
  /// Set the mouse shape.
  virtual bool SetHCursor (HCURSOR cursor) = 0;
  /// Disable the console window (if present)
  virtual void DisableConsole () = 0;
  /**
   * Display a nice message box.
   * \param window Parent window of the message box.
   * \param type Type of the message box, one of CS_ALERT_*.
   * \param title Title used for the message box.
   * \param okMsg Label of the "OK" button. 0 means default.
   * \param msg Message. Can contain cs_snprintf()-like format
   *  specifiers.
   * \param args Message formatting arguments
   * \sa CS_ALERT_ERROR
   */
  virtual void AlertV (HWND window, int type, const char* title, 
    const char* okMsg, const char* msg, va_list args) = 0;
  /**
   * Returns the handle to the main application window.
   */
  virtual HWND GetApplicationWindow() = 0;

  /**
   * Sets wether CS should get Messages on it's own
   */
  virtual void UseOwnMessageLoop (bool ownmsgloop) = 0;
  /**
   * Gets whether CS should get Messages on it's own
   */
  virtual bool HasOwnMessageLoop () = 0;
};

#endif // __CS_WIN32_H__
