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

SCF_VERSION (iNullMeshState, 0, 0, 2);

/**
 * This interface describes the API for the null mesh object.
 */
struct iNullMeshState : public iBase
{
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
};

SCF_VERSION (iNullFactoryState, 0, 0, 1);

/**
 * This interface describes the API for the null mesh object factory.
 */
struct iNullFactoryState : public iNullMeshState
{
};

/** @} */

#endif // __CS_IMESH_NULLMESH_H__

