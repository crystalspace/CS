/*
    Copyright (C) 2003, 04 by Mathew Sutcliffe <oktal@gmx.co.uk>

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

#ifndef __CS_IUTIL_BINDER_H__
#define __CS_IUTIL_BINDER_H__

#include <csutil/scf.h>

struct iEvent;
struct iEventHandler;
struct iConfigFile;
class csInputDefinition;

SCF_VERSION (iInputBinder, 0, 1, 0);

/// SCF interface for csInputBinder.
struct iInputBinder : public iBase
{
  /**
   * Get a pointer to the embedded iEventHander.
   *
   * This class can be registered with the event queue:
   * EventQueue->RegisterListener(InputBinder->QueryHandler (), CSMASK_Input);
   */
  virtual iEventHandler* QueryHandler () = 0;

  /// Returns the status of the given button command.
  virtual bool Button (unsigned cmd) = 0;

  /// Returns the position of the given axis command.
  virtual int Axis (unsigned cmd) = 0;

  /**
   * Bind a button event to a button command.
   *
   * E.g. pass a keyboard, mouse button or joystick button definition to this
   * method to bind to those particular buttons.
   *
   * Note that cmd is used as an array index so the numbers you use should be
   * consecutive, starting with 0.
   *
   * If toggle is true, the status is toggled on keydown events. If it is
   * false, status is set to 0 on keyup and 1 on keydown.
   */
  virtual void BindButton (const csInputDefinition &def, unsigned cmd,
    bool toggle = false) = 0;

  /**
   * Bind an axis motion event to an axis command.
   *
   * E.g. pass a mouse or joystick movement defintion to this method to bind to
   * that particular axis.
   *
   * Note that cmd is used as an array index so the numbers you use should be
   * consecutive, starting with 0.
   *
   * Movements will be multiplied by the sensitivity values. Remember you can
   * use negative sensitivites to invert the mouse. The default values (~0) for
   * the min and max parameters mean there will be no limit imposed on the
   * cumulative movements.
   *
   * The wrap parameter specifies whether the value will jump to the other end
   * of the range if it goes beyond the minimum or maximum value.
   */
  virtual void BindAxis (const csInputDefinition &def, unsigned cmd,
    int sensitivity = 1, int min = ~0, int max = ~0, bool wrap = true) = 0;

  /// Remove a binding.
  virtual bool UnbindButton (unsigned cmd) = 0;

  /// Remove a binding.
  virtual bool UnbindAxis (unsigned cmd) = 0;

  /// Remove all bindings.
  virtual void UnbindAll() = 0;

  /// Load bindings from a configuration file.
  virtual void LoadConfig (iConfigFile *, const char *subsection = 0) = 0;

  /// Save bindings to a configuration file.
  virtual void SaveConfig (iConfigFile *, const char *subsection = 0) = 0;
};

#endif // __CS_IUTIL_BINDER_H__
