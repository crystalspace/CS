/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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

#ifndef __CS_THING_POLYGON_H__
#define __CS_THING_POLYGON_H__

#include "csutil/scf.h"
#include "csgeom/plane3.h"
#include "csgeom/vector3.h"
#include "csgeom/matrix3.h"
#include "csutil/flags.h"

struct iMaterialHandle;
struct iMaterialWrapper;
struct iPolygon3D;
struct iPolygonTexture;
struct iPolyTexLightMap;
struct iLight;
struct iLightMap;
struct iSector;
struct iThingState;
struct iThingFactoryState;
struct iPolyTexType;
struct iRendererLightmap;
struct csPolyLightMapMapping;
struct csPolyTextureMapping;

class csReversibleTransform;
class csPlane3;
class csPolygon3D;
class csPolygon3DStatic;
class csVector3;
class csVector2;
class csMatrix3;
class csColor;

/**
 * If CS_POLY_LIGHTING is set for a polygon then the polygon will be lit.
 * It is set by default.
 */
#define CS_POLY_LIGHTING 0x00000001

/**
 * If this flag is set then this polygon is used for collision detection.
 */
#define CS_POLY_COLLDET	0x00000002

/**
 * If this flag is set then this polygon is used for visibility culling.
 */
#define CS_POLY_VISCULL	0x00000004


SCF_VERSION (iPolygon3DStatic, 0, 2, 0);

/**
 * This is the interface to the static part of a 3D polygon.
 */
struct iPolygon3DStatic : public iBase
{
  /// @@@ UGLY! Used by engine to retrieve internal object structure
  virtual csPolygon3DStatic *GetPrivateObject () = 0;

  /// Get the name of this polygon.
  virtual const char* GetName () const = 0;
  /// Set the name of this polygon.
  virtual void SetName (const char* name) = 0;

  /**
   * Get the thing (container) that this polygon belongs to.
   * The reference counter on iThingFactoryState is NOT incremented.
   */
  virtual iThingFactoryState *GetParent () = 0;
  /// Get the material handle for the texture manager.
  virtual iMaterialHandle *GetMaterialHandle () = 0;
  /**
   * Set the material for this polygon. WARNING: if you initially
   * created the polygon with a material without texture then
   * texture mapping will be disabled for this polygon.
   * You need to call EnableTextureMapping(true) again to
   * really enable texture mapping.
   */
  virtual void SetMaterial (iMaterialWrapper* mat) = 0;
  /// Get the material for this polygon.
  virtual iMaterialWrapper* GetMaterial () = 0;

  /// Query number of vertices in this polygon
  virtual int GetVertexCount () = 0;
  /// Get the vertex indices array.
  virtual int* GetVertexIndices () = 0;
  /// Get the given polygon vertex coordinates in object space
  virtual const csVector3 &GetVertex (int idx) const = 0;
  /// Create a polygon vertex given his index in parent polygon set
  virtual int CreateVertex (int idx) = 0;
  /// Create a polygon vertex and add it to parent object
  virtual int CreateVertex (const csVector3 &iVertex) = 0;

  /// Get the alpha transparency value for this polygon.
  virtual int GetAlpha () = 0;
  /**
   * Set the alpha transparency value for this polygon.
   * Not all renderers support all possible values. 0, 25, 50,
   * 75, and 100 will always work but other values may give
   * only the closest possible to one of the above.
   */
  virtual void SetAlpha (int iAlpha) = 0;

  /// Create a private polygon texture mapping plane
  virtual void CreatePlane (const csVector3 &iOrigin,
    const csMatrix3 &iMatrix) = 0;

  /// Set polygon flags (see CS_POLY_... values above)
  virtual csFlags& GetFlags () = 0;

  /**
   * Set the texture space transformation given three vertices and
   * their uv coordinates.
   */
  virtual void SetTextureSpace (
  	const csVector3& p1, const csVector2& uv1,
  	const csVector3& p2, const csVector2& uv2,
  	const csVector3& p3, const csVector2& uv3) = 0;

  /**
   * Calculate the matrix using two vertices (which are preferably on the
   * plane of the polygon and are possibly (but not necessarily) two vertices
   * of the polygon). The first vertex is seen as the origin and the second
   * as the u-axis of the texture space coordinate system. The v-axis is
   * calculated on the plane of the polygon and orthogonal to the given
   * u-axis. The length of the u-axis and the v-axis is given as the 'len1'
   * parameter.
   *<p>
   * For example, if 'len1' is equal to 2 this means that texture will be
   * tiled exactly two times between vertex 'v_orig' and 'v1'.
   *<p>
   * I hope this explanation is clear since I can't seem to make it
   * any clearer :-)
   */
  virtual void SetTextureSpace (const csVector3& v_orig,
    const csVector3& v1, float l1) = 0;

  /**
   * Calculate the matrix using 'v1' and 'len1' for the u-axis and
   * 'v2' and 'len2' for the v-axis.
   */
  virtual void SetTextureSpace (
    const csVector3& v_orig,
    const csVector3& v1, float len1,
    const csVector3& v2, float len2) = 0;

  /**
   * The most general function. With these you provide the matrix
   * directly.
   */
  virtual void SetTextureSpace (const csMatrix3&, const csVector3&) = 0;

  /**
   * Get the texture space information.
   */
  virtual void GetTextureSpace (csMatrix3&, csVector3&) = 0;

  /**
   * Disable or enable texture mapping. Doesn't do anything if nothing
   * has changed.
   */
  virtual void EnableTextureMapping (bool enable) = 0;
  /**
   * Check if texture mapping is enabled.
   */
  virtual bool IsTextureMappingEnabled () const = 0;
  /**
   * Copy texture type settings from another polygon.
   * (this will not copy the actual material that is used, just the
   * information on how to apply that material to the polygon).
   */
  virtual void CopyTextureType (iPolygon3DStatic* other_polygon) = 0;

  /// Get object space plane.
  virtual const csPlane3& GetObjectPlane () = 0;

  /**
   * Return true if this polygon or the texture it uses is transparent.
   */
  virtual bool IsTransparent () = 0;

  /// Sets the mode that is used for drawing.
  virtual void SetMixMode (uint m) = 0;
  /// Gets the mode that is used for drawing.
  virtual uint GetMixMode () = 0;

  /**
   * Intersect object-space segment with this polygon. Return
   * true if it intersects and the intersection point in object coordinates.
   */
  virtual bool IntersectSegment (const csVector3& start, const csVector3& end,
                          csVector3& isect, float* pr = 0) = 0;

  /**
   * Intersect object-space ray with this polygon. This function
   * is similar to IntersectSegment except that it doesn't keep the lenght
   * of the ray in account. It just tests if the ray intersects with the
   * interior of the polygon. Note that this function also does back-face
   * culling.
   */
  virtual bool IntersectRay (const csVector3& start, const csVector3& end) = 0;

  /**
   * Intersect object-space ray with this polygon. This function
   * is similar to IntersectSegment except that it doesn't keep the lenght
   * of the ray in account. It just tests if the ray intersects with the
   * interior of the polygon. Note that this function doesn't do
   * back-face culling.
   */
  virtual bool IntersectRayNoBackFace (const csVector3& start,
    const csVector3& end) = 0;

  /**
   * Intersect object space ray with the plane of this polygon and
   * returns the intersection point. This function does not test if the
   * intersection is inside the polygon. It just returns the intersection
   * with the plane (in or out). This function returns false if the ray
   * is parallel with the plane (i.e. there is no intersection).
   */
  virtual bool IntersectRayPlane (const csVector3& start, const csVector3& end,
  	csVector3& isect) = 0;

  /**
   * This is a given point is on (or very nearly on) this polygon.
   * Test happens in object space.
   */
  virtual bool PointOnPolygon (const csVector3& v) = 0;

  virtual csPolyLightMapMapping* GetLightMapMapping () const = 0;
  virtual csPolyTextureMapping* GetTextureMapping () const = 0;
};

SCF_VERSION (iPolygon3D, 0, 3, 1);

/**
 * This is the interface to dynamic part of a 3D polygon.
 */
struct iPolygon3D : public iBase
{
  /// @@@ UGLY! Used by engine to retrieve internal object structure
  virtual csPolygon3D *GetPrivateObject () = 0;

  /**
   * Get the thing (container) that this polygon belongs to.
   * The reference counter on iThingState is NOT incremented.
   */
  virtual iThingState *GetParent () = 0;
  /// Get the lightmap associated with this polygon
  virtual iLightMap *GetLightMap () = 0;
  /// Get the handle to the polygon texture object
  virtual iPolygonTexture *GetTexture () = 0;

  /**
   * Get the static polygon.
   */
  virtual iPolygon3DStatic* GetStaticData () const = 0;

  /// Get the given polygon vertex coordinates in world space
  virtual const csVector3 &GetVertexW (int idx) const = 0;
  /// Get the given polygon vertex coordinates in camera space
  virtual const csVector3 &GetVertexC (int idx) const = 0;

  /// Get world space plane.
  virtual const csPlane3& GetWorldPlane () = 0;
  /// Compute the camera plane based on the camera transform.
  virtual void ComputeCameraPlane (const csReversibleTransform& t,
  	csPlane3& pl) = 0;

  /**
   * Set the material for this polygon. This material must have
   * the same size as the material given in the factory!
   * If 0 then the factory material will be used.
   */
  virtual void SetMaterial (iMaterialWrapper* mat) = 0;
  /// Get the material for this polygon.
  virtual iMaterialWrapper* GetMaterial () = 0;
};

/**
 * This structure holds mapping information to map the texture on
 * a polygon. You can get it from the iPolygonTexture below.
 */
struct csPolyTextureMapping
{
  /// Transformation from object to texture space.
  csMatrix3 m_obj2tex;
  /// Translation from object to texture space.
  csVector3 v_obj2tex;

  // @@@ some entries really rather belong to csPolyLightMapMapping.
  float fdu, fdv;

  /**
   * Bounding box of corresponding polygon in 2D texture space.
   * Note that the u-axis of this bounding box is made a power of 2 for
   * efficiency reasons.
   */
  int Imin_u, Imin_v;

  /// fp bounding box (0..1 texture space)
  float Fmin_u, Fmin_v, Fmax_u, Fmax_v;

  ///
  uint16 shf_u;

  /**
   * Get the power of the lowest power of 2 that is not smaller than the
   * texture bounding box' width.
   * that is: 2^shift_u >= texbbox-width > 2^(shift_u-1)
   */
  int GetShiftU () const { return shf_u; }

  /// Get the rounded u-value of the textures bounding box' lower left corner.
  int GetIMinU () const { return Imin_u; }
  /// Get the rounded v-value of the textures bounding box' lower left corner.
  int GetIMinV () const { return Imin_v; }
  /// Get texture box.
  void GetTextureBox (float& fMinU, float& fMinV,
    float& fMaxU, float& fMaxV) const
  {
    fMinU = Fmin_u;
    fMaxU = Fmax_u;
    fMinV = Fmin_v;
    fMaxV = Fmax_v;
  }

  /// Get the u-value of the textures bounding box' lower left corner.
  float GetFDU () const { return fdu; }
  /// Get the v-value of the textures bounding box' lower left corner.
  float GetFDV () const { return fdv; }
};

/**
 * This structure holds mapping information to map the lightmap on
 * a polygon. You can get it from the iPolygonTexture below.
 */
struct csPolyLightMapMapping
{
  /*
   * New texture data with lighting added. This is an untiled texture
   * so it is more efficient to draw. This texture data is allocated
   * and maintained by the texture cache. If a PolyTexture is in the
   * cache it will be allocated, otherwise it won't.
   */

  /// Width of lighted texture ('w' is a power of 2).
  int w; //@@@ renderer specific

  /// Height of lighted texture.
  int h; //@@@ renderer specific 

  /// Original width (may not be a power of 2) (w_orig <= w).
  int w_orig;  //@@@ renderer specific
  
  /// Light cell size
  //int lightCellSize;
  /// Light cell shift
  //int lightCellShift;

  /// Get width of lit texture (power of 2).
  int GetWidth () const { return w; }
  /// Get height of lit texture.
  int GetHeight () const { return h; }

  /// Get original width.
  int GetOriginalWidth () const { return w_orig; }
};

SCF_VERSION (iPolygonTexture, 1, 3, 0);

/**
 * This is a interface to an object responsible for containing
 * the data required for a lighted texture on a polygon.
 */
struct iPolygonTexture : public iBase
{
  /// Get the material handle associated with this polygon
  virtual iMaterialHandle *GetMaterialHandle () = 0;
  /// Get the mapping to use for this texture
  virtual csPolyTextureMapping* GetTMapping () const = 0;
  /// Get the mapping to use for this lightmap.
  virtual csPolyLightMapMapping* GetLMapping () const = 0;

  /// Get light map.
  virtual iLightMap *GetLightMap () = 0;
  /// Query the size of one light cell (always a power of two)
  virtual int GetLightCellSize () = 0;
  /// Query log2 (cell size)
  virtual int GetLightCellShift () = 0;

  /// Get data used internally by texture cache
  virtual void *GetCacheData (int idx) = 0;
  /// Set data used internally by texture cache
  virtual void SetCacheData (int idx, void *d) = 0;
};

#endif // __CS_THING_POLYGON_H__

