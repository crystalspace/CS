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

#ifndef __IUTIL_OBJREG_H__
#define __IUTIL_OBJREG_H__

#include "csutil/scf.h"

#define CS_QUERY_REGISTRY_TAG(Reg,Tag)			\
  ((Reg)->Get (Tag))
#define CS_QUERY_REGISTRY(Reg,Interface)		\
  (Interface*)((Reg)->Get (#Interface, iSCF::SCF->GetInterfaceID (#Interface), \
  	VERSION_##Interface))

struct iObjectRegistryIterator;

SCF_VERSION (iObjectRegistry, 0, 0, 4);

/**
 * This interface serves as a registry of other objects.
 */
struct iObjectRegistry : public iBase
{
  /**
   * Clear the object registry and release all references.
   */
  virtual void Clear () = 0;

  /**
   * Register an object with this registry. The same object can
   * be registered multiple times but in that case it is probably
   * best to have different tags so they can be distinguished.
   * This function will increase the ref count of the given object.
   * <p>
   * Note that a given tag (if non-NULL) may only be registered once.
   * This function will return false otherwise.
   * <p>
   * This function will also fail if this object registry is being
   * cleared.
   */
  virtual bool Register (iBase*, char const* tag = NULL) = 0;

  /**
   * Unregister an object with this registry. If 'tag' is not given
   * then it will unregister all occurances of the given object
   * in the registry (i.e. for all tags). If 'tag' is given then only
   * the object that has that tag will be unregistered.
   * This function will decrease the ref count of the given object.
   */
  virtual void Unregister (iBase*, char const* tag = NULL) = 0;

  /**
   * Get the registered object corresponding with the given tag.
   * This function will NOT increase the ref count of the returned object.
   */
  virtual iBase* Get (char const* tag) = 0;

  /**
   * Get the registered object corresponding with the given tag and
   * implementing the specified interface.
   * This function will NOT increase the ref count of the returned object.
   */
  virtual iBase* Get (char const* tag, scfInterfaceID id, int version) = 0;

  /**
   * Get an iterator with all objects implementing the given interface.
   */
  virtual iObjectRegistryIterator* Get (scfInterfaceID id, int version) = 0;

  /**
   * Get an iterator with all objects in this object registry.
   */
  virtual iObjectRegistryIterator* Get () = 0;
};

SCF_VERSION (iObjectRegistryIterator, 0, 0, 1);

/**
 * Use an instance of this class to iterate over objects in the object
 * registry.
 */
struct iObjectRegistryIterator : public iBase
{
  /**
   * Restart the iterator. Returns false if there are no ellements
   * in it.
   */
  virtual bool Restart () = 0;

  /**
   * Get the current element. Return NULL if none (end of list).
   */
  virtual iBase* GetCurrent () = 0;

  /**
   * Return the current tag.
   */
  virtual const char* GetCurrentTag () = 0;

  /**
   * Proceed with next element. Return true if there is one.
   */
  virtual bool Next () = 0;
};

#endif

