/*
    Copyright (C) 1998 by Jorrit Tyberghein
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

#include "types.h"
#include "csobject/fakertti.h"
#include "csobject/objiter.h"
#include "csutil/csbase.h"
#include "csutil/util.h"

/**
 * A generic csObject class.
 * Any csObject can have any number of csObject children attached to it.
 *<p>
 * In order to make a class (let's call it newClass) that derives from 
 * csObject, the class must contain "CSOBJTYPE;" in the declaration.  The
 * corresponding .cpp file should contain the line:
 * <pre>
 * IMPLEMENT_CSOBJTYPE (newClass,parentClass);
 * </pre>
 */ 
class csObject : public csBase
{
private:
  friend class csObjIterator;
  /// Each object have a unique ID associated with it
  CS_ID csid;

  /// The array of child nodes
  csObjContainer *children;

  /**
   * Since most csObject's have names, we have a separate name
   * rather than inserting some kind of csNamedObject child.
   */
  char *Name;

  /// Set the parent csObject. Implemented in csPObject class.
  virtual void SetObjectParent (csObject *)
  { }

public:
  /// Initialize the csObject
  csObject ();
  /// Make a copy of another object
  csObject (csObject& iObject);
  /// Destroy this object and the associated children
  virtual ~csObject ();

  /// Set object name
  void SetName (const char *iName)
  { delete [] Name; Name = strnew (iName); }

  /// Query object name
  const char *GetName () const
  { return Name; }

  /// Get the unique ID associated with this object
  CS_ID GetID () const
  { return csid; }

  /// Returns the parent csObject. Implemented in csPObject class.
  virtual csObject* GetObjectParent () const
  { return NULL; }

  /// Return the first subobject instance of the given type
  csObject *GetChild (const csIdType& iType) const;

  /// Return an iterator referencing all objects of the given type
  csObjIterator GetIterator (const csIdType& iType) const
  { return csObjIterator (iType, *this); }

  /// Attach a new csObject to the tree
  void ObjAdd (csObject *obj);

  /// Removes the given object from the tree, without freeing the contents
  void ObjRelease (csObject *obj);

  /// Deletes the given object, removing it from the object tree
  void ObjRemove (csObject *obj);

  CSOBJTYPE;
};

#endif // __CSOBJ_H__
