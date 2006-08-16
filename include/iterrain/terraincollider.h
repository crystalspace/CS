/*
    Copyright (C) 2006 by Kapoulkine Arseny

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

#ifndef __CS_ITERRAIN_TERRAINCOLLIDER_H__
#define __CS_ITERRAIN_TERRAINCOLLIDER_H__

#include "csutil/scf.h"
#include "iutil/array.h"

#include "terrainarray.h"

struct iTerrainCellCollisionProperties;
struct iTerrainCell;
struct iCollider;

class csRect;
class csVector3;
class csReversibleTransform;

/// Provides an interface for custom collision
struct iTerrainCollider : public virtual iBase
{
  SCF_INTERFACE (iTerrainCollider, 1, 0, 0);

  /**
   * Create an object that implements iTerrainCellCollisionProperties
   * This object will be stored in the cell. This function gets invoked
   * at cells creation.
   *
   * \return properties object
   */
  virtual csPtr<iTerrainCellCollisionProperties> CreateProperties () = 0;
  
  /**
   * Collide segment with cell
   *
   * \param cell - cell
   * \param start - segment start (specified in object space)
   * \param end - segment end (specified in object space)
   * \param oneHit - if this is true, than stop on finding the first
   * intersection point (the closest to the segment start); otherwise, detect
   * all intersections
   * \param points - destination point array
   * 
   * \return true if there were any intersections, false if there were none
   */
  virtual bool CollideSegment (iTerrainCell* cell, const csVector3& start,
        const csVector3& end, bool oneHit, iTerrainVector3Array& points) = 0;

  /**
   * Collide set of triangles with cell
   *
   * \param cell - cell
   * \param vertices - vertex array
   * \param tri_count - triangle count
   * \param indices - vertex indices, 3 indices for each triangle
   * \param radius - radius of the bounding sphere surrounding the given set
   * of triangles (used for fast rejection)
   * \param trans - triangle set transformation (vertices' coordinates are
   * specified in the space defined by this transformation)
   * \param oneHit - if this is true, than stop on finding the first
   * collision pair; otherwise, detect all collisions
   * \param points - destination collision pair array
   * 
   * \return true if there were any collisions, false if there were none
   */
  virtual bool CollideTriangles (iTerrainCell* cell, const csVector3* vertices,
                       unsigned int tri_count,
                       const unsigned int* indices, float radius,
                       const csReversibleTransform* trans,
                       bool oneHit, iTerrainCollisionPairArray& pairs) = 0;

  /**
   * Collide collider with cell
   *
   * \param cell - cell
   * \param collider - collider
   * \param radius - radius of the bounding sphere surrounding the given set
   * of triangles (used for fast rejection)
   * \param trans - triangle set transformation (vertices' coordinates are
   * specified in the space defined by this transformation)
   * \param oneHit - if this is true, than stop on finding the first
   * collision pair; otherwise, detect all collisions
   * \param points - destination collision pair array
   * 
   * \return true if there were any collisions, false if there were none
   */
  virtual bool Collide (iTerrainCell* cell, iCollider* collider,
                       float radius, const csReversibleTransform* trans,
                       bool oneHit, iTerrainCollisionPairArray& pairs) = 0;

  /**
   * Indicates that the cell height data has been changed (while unlocking
   * the cell height data - either by a feeder or by a user-provided
   * functions), and that the collider should update its internal structures
   * to reflect the changes.
   *
   * \param cell - cell with the changed data
   * \param rectangle - rectangle that was updated
   * \param data - height data
   * \param pitch - data pitch
   */
  virtual void OnHeightUpdate (iTerrainCell* cell, const csRect& rectangle,
                               const float* data, unsigned int pitch) = 0;
};

#endif // __CS_ITERRAIN_TERRAINCOLLIDER_H__
