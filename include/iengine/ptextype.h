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

#ifndef __IENGINE_PTEXTYPE_H__
#define __IENGINE_PTEXTYPE_H__

#include "csutil/scf.h"

class csColor;
class csVector2;
struct iPolygon3D;

/**
 * Polygon has no texture mapping.
 * This flag is meaningful only for flat-colored polygons
 * since it saves a bit of memory.
 */
#define POLYTXT_NONE		0

/**
 * Flat shaded texture.
 * Polygons with this texturing type will always have same lighting
 * value across entire polygon. If the CS_POLY_LIGHTING flag is not
 * set in parent polygon object, the polygon will be painted using
 * the original texture or flat color; otherwise a single lighting
 * value will be computed (depending on the angle the light falls
 * on the polygon) and will be applied to every pixel.
 */
#define POLYTXT_FLAT		1

/**
 * Gouraud shaded texture.
 * With software rendering these textures will be painted without
 * perspective correction. Instead you can defined a color (with
 * r/g/b values in range 0..2) for every polygon vertex, and these
 * colors will be interpolated across scanlines.
 */
#define POLYTXT_GOURAUD		2

/**
 * Texture type is lightmapped.
 * These polygons are painted perspective-correct even with software
 * rendering and are used usually for walls and big objects.
 */
#define POLYTXT_LIGHTMAP	3

SCF_VERSION (iPolyTexType, 0, 0, 1);

/**
 * Kind of texturing that is used for a 3D polygon.
 * This is the base interface for all texture types.
 */
struct iPolyTexType : public iBase
{
};

SCF_VERSION (iPolyTexNone, 0, 0, 1);

/**
 * Kind of texturing that is used for a 3D polygon which
 * has no texture mapping.
 */
struct iPolyTexNone : public iBase
{
  /// Sets the mode that is used for DrawPolygonFX.
  virtual void SetMixmode (UInt m) = 0;
  /// Gets the mode that is used for DrawPolygonFX.
  virtual UInt GetMixmode () = 0;
};

SCF_VERSION (iPolyTexFlat, 0, 0, 1);

/**
 * Kind of texturing that is used for a 3D polygon which
 * has flat-shaded texture mapping.
 */
struct iPolyTexFlat : public iBase
{
  /**
   * Setup this lighting structure with the right number of vertices,
   * taken from parent object. The contents of U/V array are not destroyed,
   * if it was previously allocated.
   */
  virtual void Setup (iPolygon3D *iParent) = 0;
  /**
   * Set an (u,v) texture coordinate for the specified vertex
   * of this polygon. This function may only be called after all
   * vertices have been added. As soon as this function is used
   * this polygon will be rendered using a different technique
   * (perspective incorrect texture mapping and Gouroud shading).
   * This is useful for triangulated objects for which the triangles
   * are very small and also for drawing very far away polygons
   * for which perspective correctness is not needed (sky polygons for
   * example).
   */
  virtual void SetUV (int i, float u, float v) = 0;

  /**
   * Clear all (u,v) coordinates.
   */
  virtual void ClearUV () = 0;

  /// Get the pointer to the vertex uv coordinates.
  virtual csVector2 *GetUVCoords () = 0;


};

SCF_VERSION (iPolyTexGouraud, 0, 0, 1);

/**
 * Kind of texturing that is used for a 3D polygon which
 * has gouraud-shaded texture mapping.
 */
struct iPolyTexGouraud : public iBase
{
  /**
   * Setup this lighting structure with the rignt number of vertices,
   * taken from parent object. The contents of U/V array are not destroyed,
   * if it was previously allocated.
   */
  virtual void Setup (iPolygon3D *iParent) = 0;
  /**
   * Clear all color information.
   */
  virtual void ClearColors () = 0;

  /// Get the pointer to the vertex color table.
  virtual csColor *GetColors () = 0;

  /// Get the pointer to the static vertex color table.
  virtual csColor *GetStaticColors () = 0;

  /**
   * Reset a dynamic color to the static values.
   */
  virtual void ResetDynamicColor (int i) = 0;

  /**
   * Set a color in the dynamic array.
   */
  virtual void SetDynamicColor (int i, const csColor& c) = 0;

  /**
   * Set a color in the static array.
   */
  virtual void SetColor (int i, const csColor& c) = 0;
};

SCF_VERSION (iPolyTexLightMap, 0, 0, 1);

/**
 * Kind of texturing that is used for a 3D polygon which
 * has lightmaps.
 */
struct iPolyTexLightMap : public iBase
{
  /// Get the poly texture plane.
  virtual iPolyTxtPlane* GetPolyTxtPlane () const = 0;
};

#endif // __IENGINE_PTEXTYPE_H__
