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

#ifndef __CS_OBJREG_H__
#define __CS_OBJREG_H__

#include <stdarg.h>
#include <stdio.h>
#include "csextern.h"
#include "iutil/objreg.h"
#include "scf.h"
#include "array.h"
#include "stringarray.h"
#include "scopedmutexlock.h"

/**
 * This is an implementation of iObjectRegistry.
 * Thread-safe!
 */
class CS_CRYSTALSPACE_EXPORT csObjectRegistry : public iObjectRegistry
{
private:
  csRef<csMutex> mutex;
  csArray<iBase*> registry;
  csStringArray tags;
  // True when this object is being cleared; prevents external changes.
  bool clearing;

public:
  csObjectRegistry ();
  /// Client must explicitly call Clear().
  virtual ~csObjectRegistry ();

  SCF_DECLARE_IBASE;
  /**
   * Clear the object registry and release all references.
   */
  virtual void Clear ();
  
  /**
   * Register an object with this registry. 
   */
  virtual bool Register (iBase* obj, char const* tag = 0);

  /**
   * Unregister an object with this registry. 
   */
  virtual void Unregister (iBase* obj, char const* tag = 0);
  
  /**
   * Get the registered object corresponding with the given tag.
   * This function will increase the ref count of the returned object.
   */
  virtual iBase* Get (char const* tag);

  /**
   * Get the registered object corresponding with the given tag and
   * implementing the specified interface. 
   */
  virtual iBase* Get (char const* tag, scfInterfaceID id, int version);

  /**
   * Get an iterator with all objects implementing the given interface.
   */
  virtual csPtr<iObjectRegistryIterator> Get (scfInterfaceID id, int version);

  /**
   * Get an iterator with all objects in this object registry.
   */
  virtual csPtr<iObjectRegistryIterator> Get ();
};

#endif // __CS_OBJREG_H__

