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

#ifndef CSSPRITE2D_H
#define CSSPRITE2D_H

#include "csutil/cscolor.h"
#include "csutil/garray.h"
#include "csgeom/math3d.h"
#include "csgeom/math2d.h"
#include "csgeom/poly3d.h"
#include "csengine/rview.h"
#include "csengine/texture.h"
#include "igraph3d.h"

struct iTextureHandle;

TYPEDEF_GROWING_ARRAY (csColoredVertices, G3DTexturedVertex);

/**
 * A 2D sprite handled by the engine. It is actually a general
 * 2D polygon which has a 3D position and dimension but always
 * faces the camera.
 */
class csSprite2D : public csObject
{
public:
  /// List of sectors where this sprite is.
  csNamedObjVector sectors;

private:
  /// Mixmode.
  UInt MixMode;

  /// Position for sprite.
  csVector3 position;

  /**
   * Array of 3D vertices.
   */
  csColoredVertices vertices;

  /// The texture handle as returned by iTextureManager.
  csTextureHandle* cstxt;

public:
  ///
  csSprite2D ();
  ///
  virtual ~csSprite2D ();

  /// Set a new texture.
  void SetTexture (csTextureHandle* txt) { cstxt = txt; }

  /// Get the vertex array.
  csColoredVertices& GetVertices () { return vertices; }

  /// Sets the mode that is used, when drawing that sprite.
  void SetMixmode (UInt m) { MixMode = m; }

  /**
   * Set the transformation vector to move sprite to some position.
   */
  void SetMove (const csVector3& v) { position = v; }

  /**
   * Set the transformation vector to move sprite to some position.
   */
  void SetMove (float x, float y, float z)
  {
    position.x = x;
    position.y = y;
    position.z = z;
  }

  /**
   * Relative move
   */
  void Move (float dx, float dy, float dz)
  {
    position.x += dx;
    position.y += dy;
    position.z += dz;
  }

  /**
   * Relative move
   */
  void Move (csVector3& v) { position += v; }

  /**
   * Draw this sprite given a camera transformation.
   * If needed the skeleton state will first be updated.
   * Optionally update lighting if needed (DeferUpdateLighting()).
   */
  void Draw (csRenderView& rview);

  /// Get position of this sprite.
  inline csVector3 GetOrigin () const { return position; }

  /// Move this sprite to one sector (conveniance function).
  void MoveToSector (csSector* s);

  /// Remove this sprite from all sectors it is in (but not from the world).
  void RemoveFromSectors ();

  CSOBJTYPE;
};

#endif //CSSPRITE2D_H
