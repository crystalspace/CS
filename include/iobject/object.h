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
#include "iobject/rtti.h"

struct iObjectIterator;

/// You can use this macro to get a child object from a csObject
#define GET_CHILD_OBJECT(object,type)				\
  ((type*)(object)->GetChild (OBJECT_TYPE_ID(type)))

/**
 * You can use this macro to get a child object with
 * the given name from a csObject.
 */
#define GET_NAMED_CHILD_OBJECT(object,type,name)				\
  ((type*)(object)->GetChild (OBJECT_TYPE_ID(type), name))

/**
 * same as GET_CHILD_OBJECT, but stops at the first object with the given
 * name, even if it does not implement the requested type.
 */
#define GET_FIRST_NAMED_CHILD_OBJECT(object,type,name)				\
  ((type*)(object)->GetChild (OBJECT_TYPE_ID(type), name, true))


SCF_VERSION (iObject, 0, 0, 2);

/**
 * This interface is an SCF interface for encapsulating csObject.
 */
struct iObject : public iBase
{
  /// Set object name
  virtual void SetName (const char *iName) = 0;

  /// Query object name
  virtual const char *GetName () const = 0;

  /// Returns the parent iObject.
  virtual iObject* GetObjectParentI () const = 0;

  /// Attach a new iObject to the tree
  virtual void ObjAdd (iObject *obj) = 0;

  /// Removes the given object from the tree, without freeing the contents
  virtual void ObjRelease (iObject *obj) = 0;

  /// Deletes the given object, removing it from the object tree
  virtual void ObjRemove (iObject *obj) = 0;

  /// Removes all objects from the tree, without freeing the contents
  virtual void ObjReleaseAll () = 0;

  /// Deletes all objects, removing them from the object tree
  virtual void ObjRemoveAll () = 0;

  /**
   * Look for a child object that implements the given type. You can
   * optionally pass a name to look for. If FirstName is true then the
   * method will stop at the first object with the requested name, even
   * if it did not implement the requested type. Note that the returned
   * object may only be cast to the requested type, no other type, not
   * even iObject!
   */
  virtual void* GetChild (int TypeID, const char *Name = NULL,
    bool FirstName = false) const = 0;

  /// Return the first child object with the given name
  virtual iObject *GetChild (const char *Name) const = 0;

  /**
   * Return an iterator for all child objects. You may optionally
   * request only objects with a given type. Note that you should not
   * remove child objects while iterating.
   */
  virtual iObjectIterator *GetIterator (int TypeID = -1) = 0;

  DECLARE_ABSTRACT_OBJECT_INTERFACE;
};


SCF_VERSION (iObjectIterator, 0, 0, 1);

/// This is an iterator for child objects of a csObject.
struct iObjectIterator : public iBase
{
  /// Move forward
  virtual bool Next() = 0;

  /// Reset the iterator to the beginning
  virtual void Reset() = 0;

  /**
   * Get the object we are pointing at. You should cast this pointer to the
   * type you requested when you created the iterator. Don't cast it to
   * another type, not even iObject!
   */
  virtual void* GetTypedObj() const = 0;

  /// Get the object we are pointing at as a iObject
  virtual iObject *GetiObject () const = 0;

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

#endif
