/*
    Copyright (C) 1998,2000 by Jorrit Tyberghein

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

#ifndef __CS_CSSPRITE2D_H__
#define __CS_CSSPRITE2D_H__

#include "csutil/cscolor.h"
#include "csutil/garray.h"
#include "csgeom/math3d.h"
#include "csgeom/math2d.h"
#include "csgeom/poly3d.h"
#include "csgeom/box.h"
#include "csengine/rview.h"
#include "csengine/texture.h"
#include "csengine/cssprite.h"
#include "igraph3d.h"

class csMaterialWrapper;
class csPolyTreeObject;

struct csSprite2DVertex
{
  csVector2 pos;
  csColor color_init;
  csColor color;
  float u, v;
};

TYPEDEF_GROWING_ARRAY (csColoredVertices, csSprite2DVertex);

/**
 * A 2D sprite handled by the engine. It is actually a general
 * 2D polygon which has a 3D position and dimension but always
 * faces the camera.
 */
class csSprite2D : public csSprite
{
private:
  /**
   * Array of 3D vertices.
   */
  csColoredVertices vertices;

  /// The material handle as returned by iTextureManager.
  csMaterialWrapper* cstxt;

  /**
   * If false then we don't do lighting but instead use
   * the given colors.
   */
  bool lighting;

protected:
  /**
   * Update this sprite in the polygon trees.
   */
  virtual void UpdateInPolygonTrees ();

public:
  ///
  csSprite2D (csObject* theParent);
  ///
  virtual ~csSprite2D ();

  /// Set a new material.
  void SetMaterial (csMaterialWrapper* mat) { cstxt = mat; }

  /// Get the vertex array.
  csColoredVertices& GetVertices () { return vertices; }

  /** 
   * Set vertices to form a regular n-polygon around (0,0),
   * optionally also set u,v to corresponding coordinates in a texture.
   * Large n approximates a circle with radius 1. n must be > 2. 
   */
  void CreateRegularVertices (int n, bool setuv);

  /**
   * Set true if this sprite needs lighting (default).
   * Otherwise the given colors are used.
   * If lighting is disabled then the color_init array
   * is copied to the color array.
   */
  void SetLighting (bool l);

  /// Return the value of the lighting flag.
  bool HasLighting () { return lighting; }

  /**
   * Move the sprite.
   */
  virtual void SetPosition (const csVector3& v) { movable.SetPosition (v); }

  /**
   * Relative move
   */
  virtual void MovePosition (const csVector3& rel)
  { movable.MovePosition (rel); }

  /// Get position of this sprite.
  virtual const csVector3& GetPosition () const
  { return movable.GetPosition (); }

  /**
   * Scale the vertices of the sprite by factor.
   */
  virtual void ScaleBy (float factor);

  /**
   * Rotate the vertices of the sprite by angle, angle in radians.
   */
  virtual void Rotate (float angle);

  /**
   * Shift the vertices of the sprite by delta x and y.
   * The sprite will be drawn displaced from the center position.
   */
  void Shift (float dx, float dy);

  /**
   * Shift the vertices of the sprite by vector delta.
   * The sprite will be drawn displaced from the center position.
   */
  void Shift (const csVector2& delta) { Shift (delta.x, delta.y); }

  /**
   * Set the color of the sprite for all vertices
   */
  virtual void SetColor(const csColor& col);

  /** 
   * Add a color to all vertices init_color.
   */
  virtual void AddColor(const csColor&col);

  /**
   * Light sprite according to the given array of lights (i.e.
   * fill the vertex color array).
   */
  virtual void UpdateLighting (csLight** lights, int num_lights);

  /**
   * Draw this sprite given a camera transformation.
   * If needed the skeleton state will first be updated.
   * Optionally update lighting if needed (DeferUpdateLighting()).
   */
  virtual void Draw (csRenderView& rview);

  /**
   * Check if this sprite is hit by this object space vector.
   * Return the collision point in object space coordinates.
   * @@@ TO BE IMPLEMENTED!
   */
  virtual bool HitBeamObject (const csVector3& /*start*/,
    const csVector3& /*end*/, csVector3& /*isect*/, float* /*pr*/)
    { return false; }

  CSOBJTYPE;
  // Dummy SCF goop to work around compiler bug.  See csspr2d.cpp for details.
  DECLARE_IBASE_EXT (csSprite);
};

#endif // __CS_CSSPRITE2D_H__
