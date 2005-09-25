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

/**\file
 * Input binder interface
 */

#include <csutil/scf.h>

struct iEvent;
struct iEventHandler;
struct iConfigFile;
class csInputDefinition;

SCF_VERSION (iInputBinder, 0, 1, 0);

/**
 * SCF interface for csInputBinder,
 * used to bind input events (keypress, button press, mouse move,
 * etc.) to commands which are represented by an unsigned integer. It is
 * up to the application to specify the meaning of a command value.
 * <p>
 * Example:
 * \code
 * enum MyCommand = { Walk, Shoot, Jump, LookX, LookY };
 * ...
 * csRef<iInputBinder> binder = ...;
 * binder->BindButton (csInputDefinition ("ctrl"), Shoot);
 * binder->BindAxis (csInputDefinition ("mousex"), LookX);
 * ...
 * if (binder->Button (Shoot))
 *   ...
 * else
 * {
 *   DoSomething (binder->Axis (LookX), binder->Axis (LookY));
 * }
 * \endcode
 */
struct iInputBinder : public iBase
{
  /**
   * Get a pointer to the embedded iEventHander.
   * \remarks This class has to be registered with the event queue:
   *   EventQueue->RegisterListener(InputBinder->QueryHandler (), CSMASK_Input);
   *   to get working Axis() and Button() methods.
   */
  virtual iEventHandler* QueryHandler () = 0;

  /// Returns the status of the given button command.
  virtual bool Button (unsigned cmd) = 0;

  /// Returns the position of the given axis command.
  virtual int Axis (unsigned cmd) = 0;

  /**
   * Bind a button event to a button command.
   * \param def Describes the physical button to bind to.
   * \param cmd The ID of the command to bind.
   * \param toggle If true, button status is only toggled on keydown events.
   * \remarks Note that cmd is used as an array index so the numbers you use
   *   should be consecutive, starting with 0.
   */
  virtual void BindButton (csInputDefinition const& def, unsigned int cmd,
    bool toggle = false) = 0;

  /**
   * Bind an axis motion event to an axis command.
   * \param def Describes the physical axis to bind to.
   * \param cmd The ID of the command to bind.
   * \param sensitivity A multiplier for the axis command.
   * \remarks Note that cmd is used as an array index so the numbers you use
   *   should be consecutive, starting with 0.
   */
  virtual void BindAxis (csInputDefinition const& def, unsigned int cmd,
    int sensitivity = 1) = 0;

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
