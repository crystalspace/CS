/*
    Copyright (C) 2003 by Mathew Sutcliffe <oktal@gmx.co.uk>

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

SCF_VERSION (iInputBinderPosition, 0, 0, 1);

/**
 * Represents the position of a mouse or joystick axis, shared between plugins.
 */
struct iInputBinderPosition : public iBase
{
  /// Set the position; called by csInputBinder.
  virtual void Set (int) = 0;
  /// Get the position; called by the application.
  virtual int Get () const = 0;
};

SCF_VERSION (iInputBinderBoolean, 0, 0, 1);

/**
 * Represents the up or down state of a keyboard key or a mouse or joystick
 * button, shared between plugins.
 */
struct iInputBinderBoolean : public iBase
{
  /// Set the state; called by csInputBinder.
  virtual void Set (bool) = 0;
  /// Get the state; called by the application.
  virtual bool Get () const = 0;
};

SCF_VERSION (iInputBinder, 0, 0, 1);

/**
 * Bind an input event to a pointer to a variable,
 * so that that variable will reflect the state of a given key, button or axis.
 */
struct iInputBinder : public iBase
{
  /**
   * Get a pointer to the embedded iEventHander
   * This class can be registered with the event queue:
   * EventQueue->RegisterListener(InputBinder->QueryHandler (), CSMASK_Input);
   */
  virtual iEventHandler* QueryHandler () = 0;

  /**
   * Bind a bool to a keyboard key or mouse or joystick button status.
   * If toggle is true, one press activates and the second deactivates.
   * Otherwise, keydown activates and keyup deactivates.
   */
  virtual void Bind (iEvent&, iInputBinderBoolean*, bool toggle = false) = 0;

  /**
   * Bind two int's to the x and y axes of a mouse or joystick.
   */
  virtual void Bind (iEvent&, iInputBinderPosition*) = 0;

  /**
   * Remove a binding.
   */
  virtual bool Unbind (iEvent&) = 0;

  /**
   * Remove all bindings.
   */
  virtual bool UnbindAll() = 0;
};

#endif // __CS_IUTIL_BINDER_H__
