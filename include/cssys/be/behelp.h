/*
    Copyright (C) 1999-2000 by Eric Sunshine <sunshine@sunshineco.com>

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

#ifndef __CS_BEHELP_H__
#define __CS_BEHELP_H__

#include "csutil/scf.h"

struct iGraphics2D;

SCF_VERSION (iBeHelper, 0, 0, 1);

/**
 * This interface describes actions specific to the BeOS platform.
 * An instance of this object will be registered to the object registry
 * with tag 'SystemHelper'. But it is recommended to query this from the
 * object registry using the iBeHelper interface.
 */
struct iBeHelper : public iBase
{
  /**
   * A user action (probably sent from the subthread), such as keypress
   * or mouse action, in the form of a BMessage.  The BMessage is
   * placed in a thread-safe message queue and later processed by the
   * Crystal Space event-loop running in the main thread.
   */
  virtual bool UserAction (BMessage*) = 0;

  /**
   * Set the mouse pointer to one of the pre-defined Crystal Space
   * mouse shapes.
   */
  virtual bool SetCursor(csMouseCursorID) = 0;

  /**
   * Spawn a thread in which to run the BApplication and invokes its
   * Run() method.  If the application is already running in the
   * subthread, then this method does nothing.  This extension is
   * requested by 2D driver modules when they are about to place a
   * window on-screen, at which point the BeOS event-loop should be
   * running (independently of whether or not the Crystal Space
   * event-loop is running) so that they can respond to user actions.
   */
  virtual bool BeginUI() = 0;

  /**
   * Ask the Crystal Space event-loop to terminate.
   */
  virtual bool Quit() = 0;

  /**
   * Notify Crystal Space that a 2D graphics context is closing.
   */
  virtual bool ContextClose(iGraphics2D*) = 0;
};

#endif // __CS_BEHELP_H__
