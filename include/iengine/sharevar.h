/*
    Copyright (C) 2002 by Keith Fulton

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

#ifndef __CS_IENGINE_SHAREVAR_H__
#define __CS_IENGINE_SHAREVAR_H__

/**\file
 */
/**
 * \addtogroup engine3d
 * @{ */

#include "cstypes.h"
#include "csutil/scf.h"

SCF_VERSION (iSharedVariable, 0, 0, 1);

/**
 * iSharedVariable implements a refcounted float value which can
 * be shared across many objects and updated efficiently.
 */
struct iSharedVariable : public iBase
{
  virtual void Set(float val) = 0;
  virtual float Get() const = 0;
  virtual void SetName (const char *iName) = 0;
  virtual const char *GetName () const = 0;
};

SCF_VERSION (iSharedVariableList, 0, 0, 2);

/**
 * A list of shared variables.
 */
struct iSharedVariableList : public iBase
{
  /// Return the number of Shared Variables in this list.
  virtual int GetCount () const = 0;

  /// Return a SharedVariable by index.
  virtual iSharedVariable *Get (int n) const = 0;

  /// Add a SharedVariable.
  virtual int Add (iSharedVariable *obj) = 0;

  /// Remove a SharedVariable.
  virtual bool Remove (iSharedVariable *obj) = 0;

  /// Remove the nth SharedVariable.
  virtual bool Remove (int n) = 0;

  /// Remove all SharedVariables.
  virtual void RemoveAll () = 0;

  /// Find a SharedVariable and return its index.
  virtual int Find (iSharedVariable *obj) const = 0;

  /// Find a SharedVariable by name.
  virtual iSharedVariable *FindByName (const char *Name) const = 0;

  /// iSharedVariable Factory method.  This does not add the new var to the list.
  virtual iSharedVariable *New() const = 0;
};

#endif // __CS_IENGINE_SHAREVAR_H__
