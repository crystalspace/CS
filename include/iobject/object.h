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

class csObject;

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
};

#endif
