/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __CS_IMESH_NULLMESH_H__
#define __CS_IMESH_NULLMESH_H__

/**\file
 * Null mesh object
 */ 

#include "csutil/scf.h"

/**\addtogroup meshplugins
 * @{ */

class csVector3;
class csBox3;
struct iMeshObject;
struct iObjectModel;

/**
 * This interface describes the API for the null mesh object.
 */
struct iNullMeshState : public virtual iBase
{
  SCF_INTERFACE (iNullMeshState, 1, 0, 0);

  /**
   * Set the radius of the sphere represented by this object.
   * If you call this function then the box will be calculated from the
   * radius.
   */
  virtual void SetRadius (float radius) = 0;
  /// Get the radius of the sphere represented by this object.
  virtual float GetRadius () const = 0;
  /**
   * Set the bounding box represented by this object.
   * If you call this function then the radius will be calculated based
   * on this box.
   */
  virtual void SetBoundingBox (const csBox3& box) = 0;
  /// Get the bounding box represented by this object.
  virtual void GetBoundingBox (csBox3& box) = 0;

  /**
   * Sets a mesh to perform hitbeam calculations on. Useful where you 
   * want to use a different shape for these calculations to what you render.
   * An example of this is having a nullmesh to represent an object entity
   * while using pseudo-instancing on a different mesh object for the render.
   */
  virtual void SetHitBeamMeshObject (iMeshObject* mesh) = 0;
};

/**
 * This interface describes the API for the null mesh object factory.
 */
struct iNullFactoryState : public virtual iNullMeshState
{
  SCF_INTERFACE (iNullFactoryState, 1, 0, 0);

  /**
   * Sets the object model required by csColliderHelper for retrieving
   * collision data. Useful when you want to have collision on a null mesh,
   * e.g. Using a nullmesh to represent the position of an instanced mesh.
   */
  virtual void SetCollisionMeshData (iObjectModel* data) = 0;
};

/** @} */

#endif // __CS_IMESH_NULLMESH_H__

