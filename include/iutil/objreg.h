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

#ifndef __CS_IUTIL_OBJREG_H__
#define __CS_IUTIL_OBJREG_H__

/**\file
 */
 
/**
 * \addtogroup scf
 * @{ */

#include "csutil/scf.h"
#include "csutil/ref.h"

#define CS_QUERY_REGISTRY_TAG(Reg, Tag)       \
  csPtr<iBase> ((iBase*) (CS_IMPLICIT_PTR_CAST(iObjectRegistry, Reg)->Get (Tag)))
#define CS_QUERY_REGISTRY(Reg,Interface)      \
  csPtr<Interface> ((Interface*)(CS_IMPLICIT_PTR_CAST(iObjectRegistry, Reg)->Get (#Interface, \
    scfInterface<Interface>::GetID(), scfInterface<Interface>::GetVersion())))
#define CS_QUERY_REGISTRY_TAG_INTERFACE(Reg, Tag, Interface)  \
  csPtr<Interface> ((Interface*)(CS_IMPLICIT_PTR_CAST(iObjectRegistry, Reg)->Get (Tag,    \
    scfInterface<Interface>::GetID(), scfInterface<Interface>::GetVersion())))

struct iObjectRegistryIterator;

SCF_VERSION (iObjectRegistry, 0, 0, 4);

/**
 * This interface serves as a registry of other objects.
 * The object registry is thread-safe.
 */
struct iObjectRegistry : public iBase
{
  // Allow implicit casts through static function.
  CS_IMPLEMENT_IMPLICIT_PTR_CAST (iObjectRegistry);

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
   * Note that a given tag (if non-0) may only be registered once.
   * This function will return false otherwise.
   * <p>
   * This function will also fail if this object registry is being
   * cleared.
   */
  virtual bool Register (iBase*, char const* tag = 0) = 0;

  /**
   * Unregister an object with this registry. If 'tag' is not given
   * then it will unregister all occurrences of the given object
   * in the registry (i.e. for all tags). If 'tag' is given then only
   * the object that has that tag will be unregistered.
   * This function will decrease the ref count of the given object.
   */
  virtual void Unregister (iBase*, char const* tag = 0) = 0;

  /**
   * Get the registered object corresponding with the given tag.
   * This function will increase the ref count of the returned object.
   */
  virtual iBase* Get (char const* tag) = 0;

  /**
   * Get the registered object corresponding with the given tag and
   * implementing the specified interface. The iBase pointers returned
   * by the iterator will be the requested interface itself so there is
   * no need to do further QueryInterface().
   * This function will increase the ref count of the returned object.
   */
  virtual iBase* Get (char const* tag, scfInterfaceID id, int version) = 0;

  /**
   * Get an iterator with all objects implementing the given interface.
   * Note that the iterator iterates over a copy of the elements in the
   * object registry so no thread-locking on the object registry happens except
   * at the time the iterator is created.
   */
  virtual csPtr<iObjectRegistryIterator> Get (
  	scfInterfaceID id, int version) = 0;

  /**
   * Get an iterator with all objects in this object registry.
   * Note that the iterator iterates over a copy of the elements in the
   * object registry so no thread-locking on the object registry happens except
   * at the time the iterator is created.
   */
  virtual csPtr<iObjectRegistryIterator> Get () = 0;
};

SCF_VERSION (iObjectRegistryIterator, 0, 1, 0);

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
  virtual bool Reset () = 0;

  /**
   * Return the current tag.
   */
  virtual const char* GetCurrentTag () = 0;

  /**
   * Return true if there are more elements.
   */
  virtual bool HasNext () = 0;

  /**
   * Proceed with next element. Return the element is there is one.
   */
  virtual iBase* Next () = 0;
};

/** @} */

#endif // __CS_IUTIL_OBJREG_H__

