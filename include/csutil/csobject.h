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

#ifndef __CSOBJ_H__
#define __CSOBJ_H__

#include "cstypes.h"
#include "csutil/csbase.h"
#include "csutil/util.h"
#include "iutil/object.h"

/**
 * A generic csObject class. Any csObject can have any number of iObject
 * children attached to it. You can use QUERY_INTERFACE to get interfaces
 * from the child objects.
 */ 
class csObject : public iObject
{
protected:
  friend class csObjectIterator;
  /// Each object have a unique ID associated with it
  CS_ID csid;

  /// The array of child nodes
  class csObjectContainer *Children;

  /// Object's name or NULL if unnamed.
  char *Name;

  /// Parent object
  iObject *ParentObject;

public:
  /// Initialize the csObject
  csObject (iBase* pParent = NULL);
  /// Destroy this object and the associated children
  virtual ~csObject ();

  /// Set object name
  virtual void SetName (const char *iName);

  /// Query object name
  virtual const char *GetName () const;

  /// Get the unique ID associated with this object
  virtual CS_ID GetID () const;

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
    const char *Name = NULL, bool FirstName = false) const;

  /// Return the first child object with the given name
  virtual iObject *GetChild (const char *Name) const;

  /**
   * Return an iterator for all child objects. Note that you should not
   * remove child objects while iterating.
   */
  virtual iObjectIterator *GetIterator ();

  DECLARE_IBASE;

  // @@@ temporary fix
  virtual void ObjReleaseOld (iObject *obj);
};

#endif // __CSOBJ_H__
