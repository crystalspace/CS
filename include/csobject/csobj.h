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

#ifndef __CSOBJ_H_
#define __CSOBJ_H_

#define __USE_CS_ID_CODE

#include "types.h"
#include "csobject/fakertti.h"
#include "csobject/treeitr.h"
#include "csutil/csbase.h"

// Note: this implementation of csObject doesn't include the 'name' field.
// if a name is necessary, then a csNameObject can be created and attached.

/**
 * A generic csObject class.
 * Any csObject can have any number of csObject children attached to it
 * via the csObject tree.  
 * In order to make a class (let's call it newClass) that derives from 
 * csObject, the class must contain "CSOBJTYPE;" in the declaration.  The
 * corresponding .cpp file should contain "CSOBJTYPE(newClass,parentClass);"
 */ 
class csObject : public csBase //, IObject
{
private:
  ///
  csObjTree* objtree;

#ifdef __USE_CS_ID_CODE
  ///
  CS_ID csid_value;
#endif

  /// Set the parent csObject.  Not required.
  virtual void SetObjectParent (csObject* parent) { (void)parent; }

public:
  ///
  csObject ();
  ///
  virtual ~csObject ();
  ///
  csObject (csObject& csobj);

  /// Returns the parent csObject.  Not required.
  virtual csObject* GetObjectParent () { return NULL; }

  /// Return the first subobject instance of the given type
  csObject* GetObj (const csIdType& objtype);

  /// Return an iterator referencing all objects of the given type
  csObjIterator ObjGet (const csIdType& objtype);

  /// Attach a new csObject to the tree
  void ObjAdd (csObject* obj);

  /// Removes the given object from the tree, without freeing the contents
  void ObjRelease (csObject* obj);

  /// Deletes the given object, removing it from the object tree
  void ObjRemove (csObject* obj);

  /// Set object name
  void SetName (const char *Name);

  /// Query object name
  const char *GetName ();

#ifdef __USE_CS_ID_CODE
  ///
  CS_ID GetID () const { return csid_value; }
#endif

  CSOBJTYPE;
};

#endif /* __CSOBJ_H_ */
