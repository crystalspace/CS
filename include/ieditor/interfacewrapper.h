/*
    Copyright (C) 2007 by Seth Yastrov

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

#ifndef __IEDITOR_INTERFACEWRAPPER_H__
#define __IEDITOR_INTERFACEWRAPPER_H__

#include <csutil/ref.h>

#include <csutil/scf.h>
#include <csutil/scf_implementation.h>

#include "ieditor/editorobject.h"

namespace CS {
namespace EditorApp {

struct iInterfaceWrapperFactory;

/**
 * Wraps an SCF interface.
 */
struct iInterfaceWrapper : public virtual iBase
{
  SCF_INTERFACE (iInterfaceWrapper, 0, 0, 1);

  /// Get a pointer to the factory that created this.
  virtual iInterfaceWrapperFactory* GetFactory () = 0;
  
  /// Get the object's name.
  virtual const char* GetObjectName () const = 0;

  /// Set the name of the object.
  virtual void SetObjectName (const char* name) = 0;

  /// Get the object's parent in the hierarchy.
  virtual iBase* GetObjectParent () = 0;

  /// Set the object's parent in the hierarchy.
  virtual bool SetObjectParent (iBase* parent) = 0;

  /// Get an iterator over the child objects.
  virtual csPtr<iBaseIterator> GetIterator () = 0;

  /// TODO: GetProperties() function
};

/**
 * Factory for interface wrappers.
 */
struct iInterfaceWrapperFactory : public virtual iBase
{
  SCF_INTERFACE (iInterfaceWrapperFactory, 0, 0, 1);

  /**
   * Instantiate an interface wrapper around the specified object.
   * \note Will not check that it implements this interface.
   */
  virtual csPtr<iInterfaceWrapper> CreateInstance (iBase* object) = 0;

  /// Get the scfInterfaceID of the wrapped interface.
  virtual scfInterfaceID GetInterfaceID () const = 0;
  
  /// Get the type of object this interface might represent.
  virtual EditorObjectType GetInterfaceType () const = 0;

  /// Get whether this interface has a name attribute.
  virtual bool HasNameAttribute () const = 0;

  /// Get whether this interface has a parent attribute.
  virtual bool HasParentAttribute () const = 0;

  /// TODO: Property metadata function(s)
};

} // namespace EditorApp
} // namespace CS

#endif
