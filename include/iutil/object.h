/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

#ifndef __IOBJECT_OBJECT_H__
#define __IOBJECT_OBJECT_H__

#include "csutil/scf.h"

struct iObjectIterator;

/**
 * You can use this macro to get a child object from a csObject. The returned
 * object will be IncRef'ed.
 */
#define CS_GET_CHILD_OBJECT(object,Interface)				\
  ((Interface*)(object)->GetChild (					\
    iSCF::SCF->GetInterfaceID (#Interface), VERSION_##Interface))

/**
 * You can use this macro to get a child object from a csObject. The returned
 * object will be IncRef'ed. This version requires a correctly set-up interface
 * ID variable.
 */
#define CS_GET_CHILD_OBJECT_FAST(object,Interface)			\
  ((Interface*)(object)->GetChild(scfGetID_##Interface(), VERSION_##Interface))

/**
 * You can use this macro to get a child object with the given name and
 * interface from a csObject. The returned object will be IncRef'ed.
 */
#define CS_GET_NAMED_CHILD_OBJECT(object,Interface,name)		\
  ((Interface*)(object)->GetChild (iSCF::SCF->GetInterfaceID		\
    (#Interface), VERSION_##Interface, name))

/**
 * You can use this macro to get a child object with the given name and
 * interface from a csObject. The returned object will be IncRef'ed. This
 * version requires a correctly set-up interface ID variable.
 */
#define CS_GET_NAMED_CHILD_OBJECT_FAST(object,Interface,name)		\
  ((Interface*)(object)->GetChild					\
  (scfGetID_##Interface (), VERSION_##Interface, name))

/**
 * This is the same as CS_GET_CHILD_OBJECT, but stops at the first object with
 * the given name, even if it does not implement the requested interface.
 */
#define CS_GET_FIRST_NAMED_CHILD_OBJECT(object,Interface,name)		\
  ((Interface*)(object)->GetChild (iSCF::SCF->GetInterfaceID		\
    (#Interface), VERSION_##Interface, name, true))

/**
 * This is the same as CS_GET_CHILD_OBJECT_FAST, but stops at the first object
 * with the given name, even if it does not implement the requested interface.
 */
#define CS_GET_FIRST_NAMED_CHILD_OBJECT_FAST(object,Interface,name)	\
  ((Interface*)(object)->GetChild					\
  (scfGetID_##Interface (), VERSION_##Interface, name, true))


SCF_VERSION (iObject, 0, 3, 0);

/**
 * This interface is an SCF interface for encapsulating csObject.
 */
struct iObject : public iBase
{
  /// Set object name
  virtual void SetName (const char *iName) = 0;

  /// Query object name
  virtual const char *GetName () const = 0;

  /// Get the unique ID associated with this object.
  virtual CS_ID GetID () const = 0;

  /**
   * Set the parent iObject. Note that this only sets the 'parent' pointer but
   * does not add the object as a child object.
   */
  virtual void SetObjectParent (iObject *obj) = 0;

  /// Returns the parent iObject.
  virtual iObject* GetObjectParent () const = 0;

  /// Attach a new iObject to the tree
  virtual void ObjAdd (iObject *obj) = 0;

  /// Remove an iObject from the tree.
  virtual void ObjRemove (iObject *obj) = 0;

  /// Remove all child objects.
  virtual void ObjRemoveAll () = 0;

  /// Add all child objects of the given object
  virtual void ObjAddChildren (iObject *Parent) = 0;

  /**
   * Look for a child object that implements the given interface. You can
   * optionally pass a name to look for. If FirstName is true then the
   * method will stop at the first object with the requested name, even
   * if it did not implement the requested type. Note that the returned
   * object may only be cast to the requested type, no other type, not
   * even iObject! <p>
   *
   * Note that the returned object will be IncRef'ed.
   */
  virtual void* GetChild (int iInterfaceID, int iVersion,
    const char *Name = NULL, bool FirstName = false) const = 0;

  /// Return the first child object with the given name
  virtual iObject *GetChild (const char *Name) const = 0;

  /**
   * Return an iterator for all child objects. Note that you should not
   * remove child objects while iterating.
   */
  virtual iObjectIterator *GetIterator () = 0;

  // @@@ temporary fix
  virtual void ObjReleaseOld (iObject *obj) = 0;
};


SCF_VERSION (iObjectIterator, 0, 0, 1);

/**
 * This is an iterator for child objects of a csObject. Note that this
 * iterator only contains type-independent functionality and is therefore
 * a bit complicated to use (i.e. you'll have to do a lot of
 * SCF_QUERY_INTERFACE_CALLS if you use it directly). Check out typed object
 * iterators instead.
 */
struct iObjectIterator : public iBase
{
  /// Move forward
  virtual bool Next() = 0;

  /// Reset the iterator to the beginning
  virtual void Reset() = 0;

  /// Get the object we are pointing at
  virtual iObject *GetObject () const = 0;

  /// Get the parent object
  virtual iObject* GetParentObj() const = 0;

  /// Check if we have any children of requested type
  virtual bool IsFinished () const = 0;

  /**
   * traverses all csObjects and looks for an object with the given name
   * returns true, if found, false if not found. You can then get the
   * object by calling GetObj, and can continue search by calling Next and
   * then do an other FindName, if you like.
   */
  virtual bool FindName (const char* name) = 0;
};


SCF_VERSION (iDataObject, 0, 0, 1);

/// This is an iObject that contains a type-less data buffer.
struct iDataObject : public iBase
{
  /// Query the iObject pointer
  virtual iObject *QueryObject () = 0;

  /// Return the data buffer
  virtual void* GetData () = 0;
};

#endif
