/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __IVIDEO_NATWIN_H__
#define __IVIDEO_NATWIN_H__

#include <stdarg.h>
#include "csutil/scf.h"

#define CS_ALERT_ERROR 1
#define CS_ALERT_WARNING 2
#define CS_ALERT_NOTE 3

SCF_VERSION (iNativeWindowManager, 0, 0, 1);

/**
 * This interface represents the native window manager system.
 * At this moment this interface is nearly empty. In the future it
 * will be extended with more functionality to manage windows and so on.
 */
struct iNativeWindowManager : public iBase
{
  /**
   * Show an alert.
   * Type is one of CS_ALERT_???.
   */
  virtual void Alert (int type, const char* title, const char* okMsg,
  	const char* msg, ...) CS_GNUC_PRINTF (5, 6) = 0;
  /**
   * Show an alert.
   * Type is one of CS_ALERT_???.
   */
  virtual void AlertV (int type, const char* title, const char* okMsg,
  	const char* msg, va_list arg) CS_GNUC_PRINTF (5, 0) = 0;
};

SCF_VERSION (iNativeWindow, 0, 0, 1);

/**
 * This interface represents a native window.
 */
struct iNativeWindow : public iBase
{
  /**
   * Set the title for this window.
   */
  virtual void SetTitle (const char* title) = 0;
};

#endif // __IVIDEO_NATWIN_H__

