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
 * Shared variables
 */
/**
 * \addtogroup engine3d
 * @{ */

#include "csutil/scf.h"


struct iObject;
struct iSharedVariableListener;

class csColor;
class csVector3;


/**
 * This class implements a refcounted value which can
 * be shared across many objects and updated efficiently.
 *
 * Main creators of instances implementing this interface:
 * - iSharedVariableList::New()
 * 
 * Main ways to get pointers to this interface:
 * - iEngine::GetVariableList()
 * - iSharedVariableList::Get()
 */
struct iSharedVariable : public virtual iBase
{
  SCF_INTERFACE(iSharedVariable, 3,0,0);
  /// Get the private object interface
  virtual iObject* QueryObject () = 0;

  /// iSharedVariables are referenced by name. Here is where you set it.
  virtual void SetName (const char *name) = 0;

  /// Get the name of this variable.
  virtual const char *GetName () const = 0;

  /// Set the variable to a floating pt value.
  virtual void Set(float val) = 0;

  /// Get the floating point version of the var value.
  virtual float Get () const = 0;

  /// Set the variable to store a csColor.
  virtual void SetColor (const csColor& col) = 0;

  /// Get the csColor from the variable.
  virtual const csColor& GetColor () const = 0;

  /// Set the variable to store a csVector3.
  virtual void SetVector (const csVector3& v) = 0;

  /// Get the vector from the variable.
  virtual const csVector3& GetVector () const = 0;

  /// Set the string from the variable.
  virtual void SetString (const char* str) = 0;

  /// Get the string from the variable.
  virtual const char* GetString () const = 0;

  /// Possible types stored by this class.
  enum SharedVariableType
  {
    SV_UNKNOWN = 0,    /*!< Undefined variable type. */
    SV_FLOAT   = 1,    /*!< 'float' variable type. */
    SV_COLOR   = 2,    /*!< csColor variable type. */
    SV_VECTOR  = 3,    /*!< csVector3 variable type. */
    SV_STRING  = 4     /*!< 'const char*' variable type. */
  };

  /// Get the type currently stored by this variable.
  virtual int GetType () const = 0;

  /// Add a listener to variables.
  virtual void AddListener (iSharedVariableListener* listener) = 0;

  /// Remove a listener.
  virtual void RemoveListener (iSharedVariableListener* listener) = 0;
};


/**
 * A listener so that you can get notified when a variable is
 * changed.
 *
 * This callback is used by:
 * - iSharedVariable
 */
struct iSharedVariableListener : public virtual iBase
{
  SCF_INTERFACE(iSharedVariableListener,2,0,0);
  /**
   * A variable has changed.
   */
  virtual void VariableChanged (iSharedVariable* var) = 0;
};


/**
 * A list of shared variables.
 *
 * Main ways to get pointers to this interface:
 * - iEngine::GetVariableList()
 */
struct iSharedVariableList : public virtual iBase
{
  SCF_INTERFACE(iSharedVariableList, 2,0,0);
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

  /// iSharedVariable Factory method. This does not add the new var to the list.
  virtual csPtr<iSharedVariable> New() = 0;
};

/** @} */

#endif // __CS_IENGINE_SHAREVAR_H__
