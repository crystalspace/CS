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

#ifndef __IUTIL_OBJREF_H__
#define __IUTIL_OBJREF_H__

#include "csutil/scf.h"

struct iReferencedObject;

SCF_VERSION (iReference, 0, 0, 1);

/**
 * This is a reference to some other object. Use this if you need
 * to keep track to references to some particular object. The object
 * that is being referenced is the iReferencedObject.
 */
struct iReference : public iBase
{
  /// Get the referenced object.
  virtual iReferencedObject* GetReferencedObject () const = 0;

  /// Set the referenced object. Can be NULL to clear the reference.
  virtual void SetReferencedObject (iReferencedObject*) = 0;
};

SCF_VERSION (iReferencedObject, 0, 0, 1);

/**
 * This is an object which is referenced by other objects.
 */
struct iReferencedObject : public iBase
{
  /// Add another reference.
  virtual void AddReference (iReference* ref) = 0;
  /// Remove a reference.
  virtual void RemoveReference (iReference* ref) = 0;
};

#endif

