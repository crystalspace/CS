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

#ifndef __CS_MESHOBJ_H__
#define __CS_MESHOBJ_H__

#include "csobject/pobject.h"
#include "csengine/cssprite.h"
#include "csengine/movable.h"
#include "imeshobj.h"

class Dumper;

/**
 * The holder class for all implementations of iMeshObject.
 */
class csMeshObject : public csSprite
{
  friend class Dumper;

private:
  /// Bounding box for polygon trees.
  csPolyTreeBBox bbox;

  /// Mesh object corresponding with this csMeshObject.
  iMeshObject* mesh;

protected:
  /**
   * Update this sprite in the polygon trees.
   */
  virtual void UpdateInPolygonTrees ();

public:
  /// Constructor.
  csMeshObject (csObject* theParent, iMeshObject* mesh);
  /// Destructor.
  virtual ~csMeshObject ();

  /// Scale the mesh object by scaling the diagonal of the transform.
  virtual void ScaleBy (float factor);

  /**
   * Rotate the mesh object in some way, angle in radians.
   * currently first z-rotates angle then x-rotates angle.
   */
  virtual void Rotate (float angle);

  /// Set color for all vertices.
  virtual void SetColor (const csColor& col);

  /// Add color to all vertices.
  virtual void AddColor (const csColor& col);

  /**
   * Light sprite according to the given array of lights (i.e.
   * fill the vertex color array).
   */
  virtual void UpdateLighting (csLight** lights, int num_lights);

  /**
   * Do some initialization needed for visibility testing.
   * i.e. clear camera transformation.
   */
  virtual void VisTestReset ()
  {
    bbox.ClearTransform ();
  }

  /**
   * Draw this mesh object given a camera transformation.
   * If needed the skeleton state will first be updated.
   * Optionally update lighting if needed (DeferUpdateLighting()).
   */
  virtual void Draw (csRenderView& rview);

  /**
   * Check if this sprite is hit by this object space vector.
   * Return the collision point in object space coordinates.
   */
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr);

  CSOBJTYPE;
};

#endif // __CS_MESHOBJ_H__
