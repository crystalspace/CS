/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
    csObject library (C) 1999 by Ivan Avramovic <ivan@avramovic.com>

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

#ifndef __CS_CSOBJECT_H__
#define __CS_CSOBJECT_H__

#include "csextern.h"
#include "cstypes.h"
#include "util.h"
#include "csutil/leakguard.h"
#include "iutil/object.h"
#include "refarr.h"

typedef csRefArray<iObject> csObjectContainer;

/**
 * A generic csObject class. Any csObject can have any number of iObject
 * children attached to it. You can use SCF_QUERY_INTERFACE to get interfaces
 * from the child objects.
 */
class CS_CSUTIL_EXPORT csObject : public iObject
{
protected:
  friend class csObjectIterator;
  /// Each object has a unique ID associated with it
  uint csid;

  /// The array of child nodes
  csObjectContainer *Children;

  /// Object's name or 0 if unnamed.
  char *Name;

  /// Parent object
  iObject *ParentObject;

  /// private initialization function
  void InitializeObject ();

public:
  CS_LEAKGUARD_DECLARE (csObject);

  /// Initialize the csObject.
  csObject (iBase* pParent = 0);

  /**
   * Copy constructor. The copied object contains all children of the original
   * object, but has a new ID and is not automatically added to the original
   * object's parent.
   */
  csObject (csObject &o);

  /// Destroy this object and the associated children
  virtual ~csObject ();

  /// Set object name
  virtual void SetName (const char *iName);

  /// Query object name
  virtual const char *GetName () const;

  /// Get the unique ID associated with this object
  virtual uint GetID () const;

  /// Set the parent csObject.
  virtual void SetObjectParent (iObject *);

  /// Returns the parent iObject.
  virtual iObject* GetObjectParent () const;

  /// Attach a new iObject to the tree
  virtual void ObjAdd (iObject *obj);

  /// Deletes the given object, removing it from the object tree
  virtual void ObjRemove (iObject *obj);

  /// Deletes all objects, removing them from the object tree
  virtual void ObjRemoveAll ();

  /// Add all child objects of the given object
  virtual void ObjAddChildren (iObject *Parent);

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
    const char *Name = 0, bool FirstName = false) const;

  /// Return the first child object with the given name
  virtual iObject *GetChild (const char *Name) const;

  /**
   * Return an iterator for all child objects. Note that you should not
   * remove child objects while iterating.
   */
  virtual csPtr<iObjectIterator> GetIterator ();

  SCF_DECLARE_IBASE;

  // @@@ temporary fix
  virtual void ObjReleaseOld (iObject *obj);
};

#endif // __CS_CSOBJECT_H__
