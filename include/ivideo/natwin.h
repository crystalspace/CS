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

#ifndef __CS_IVIDEO_NATWIN_H__
#define __CS_IVIDEO_NATWIN_H__

/**\file
 * Native window manager interface
 */

/**
 * \addtogroup gfx2d
 * @{ */
 
#include "csutil/scf.h"

/**\name Alert types
 * @{ */
/// Display an error
#define CS_ALERT_ERROR 1
/// Display a warning
#define CS_ALERT_WARNING 2
/// Display a note
#define CS_ALERT_NOTE 3
/** @} */

/**
 * This interface represents the native window manager system.
 * At this moment this interface is nearly empty. In the future it
 * will be extended with more functionality to manage windows and so on.
 */
struct iNativeWindowManager : public virtual iBase
{
  SCF_INTERFACE (iNativeWindowManager, 2, 0, 0);
  
  /**
   * Show an alert.
   * Type is one of CS_ALERT_???.
   * \remarks All strings are expected to be UTF-8 encoded.
   * \sa #CS_ALERT_ERROR
   * \sa \ref FormatterNotes
   */
  virtual void Alert (int type, const char* title, const char* okMsg,
  	const char* msg, ...) CS_GNUC_PRINTF (5, 6) = 0;
  /**
   * Show an alert.
   * Type is one of CS_ALERT_???.
   * \remarks All strings are expected to be UTF-8 encoded.
   * \sa #CS_ALERT_ERROR
   * \sa \ref FormatterNotes
   */
  virtual void AlertV (int type, const char* title, const char* okMsg,
  	const char* msg, va_list arg) CS_GNUC_PRINTF (5, 0) = 0;
};

/**
 * This interface represents a native window.
 */
struct iNativeWindow : public virtual iBase
{
  SCF_INTERFACE (iNativeWindow, 2, 0, 0);
  
  /**
   * Set the title for this window.
   * \remarks \p title is expected to be UTF-8 encoded.
   */
  virtual void SetTitle (const char* title) = 0;
};

/** @} */

#endif // __CS_IVIDEO_NATWIN_H__

