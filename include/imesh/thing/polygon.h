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

#ifndef __IENGINE_POLYGON_H__
#define __IENGINE_POLYGON_H__

#include "csutil/scf.h"
#include "csgeom/plane3.h"
#include "csutil/flags.h"

struct iMaterialHandle;
struct iMaterialWrapper;
struct iPolygon3D;
struct iPolyTxtPlane;
struct iPolygonTexture;
struct iLight;
struct iLightMap;
struct iPortal;
struct iSector;
struct iThingState;
struct iPolyTexType;
struct iObject;

class csPolygon3D;
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


SCF_VERSION (iPolygon3D, 0, 1, 14);

/**
 * This is the interface to 3D polygons.
 */
struct iPolygon3D : public iBase
{
  /// @@@ UGLY! Used by engine to retrieve internal object structure
  virtual csPolygon3D *GetPrivateObject () = 0;

  /// Get the iObject for this polygon.
  virtual iObject *QueryObject() = 0;
  /**
   * Get the thing (container) that this polygon belongs to.
   * The reference counter on iThingState is NOT incremented.
   */
  virtual iThingState *GetParent () = 0;
  /// Get the lightmap associated with this polygon
  virtual iLightMap *GetLightMap () = 0;
  /// Get the handle to the polygon texture object
  virtual iPolygonTexture *GetTexture () = 0;
  /// Get the material handle for the texture manager.
  virtual iMaterialHandle *GetMaterialHandle () = 0;
  /// Set the material for this polygon.
  virtual void SetMaterial (iMaterialWrapper* mat) = 0;
  /// Get the material for this polygon.
  virtual iMaterialWrapper* GetMaterial () = 0;

  /// Query number of vertices in this polygon
  virtual int GetVertexCount () = 0;
  /// Get the vertex indices array.
  virtual int* GetVertexIndices () = 0;
  /// Get the given polygon vertex coordinates in object space
  virtual csVector3 &GetVertex (int idx) = 0;
  /// Get the given polygon vertex coordinates in world space
  virtual csVector3 &GetVertexW (int idx) = 0;
  /// Get the given polygon vertex coordinates in camera space
  virtual csVector3 &GetVertexC (int idx) = 0;
  /// Create a polygon vertex given his index in parent polygon set
  virtual int CreateVertex (int idx) = 0;
  /// Create a polygon vertex and add it to parent object
  virtual int CreateVertex (const csVector3 &iVertex) = 0;

  /// Get the alpha transparency value for this polygon.
  virtual int GetAlpha () = 0;
  /**
   * Set the alpha transparency value for this polygon (only if
   * it is a portal).
   * Not all renderers support all possible values. 0, 25, 50,
   * 75, and 100 will always work but other values may give
   * only the closest possible to one of the above.
   */
  virtual void SetAlpha (int iAlpha) = 0;

  /// Create a private polygon texture mapping plane
  virtual void CreatePlane (const csVector3 &iOrigin,
    const csMatrix3 &iMatrix) = 0;
  /// Set polygon texture mapping plane
  virtual bool SetPlane (const char *iName) = 0;

  /// Set polygon flags (see CS_POLY_... values above)
  virtual csFlags& GetFlags () = 0;

  /// Set Gouraud vs lightmap polygon lighting
  virtual void SetLightingMode (bool iGouraud) = 0;

  /**
   * Create a null pointer pointing to no sector.
   * It is preferably to set a missing sector callback on the
   * returned portal.
   */
  virtual iPortal* CreateNullPortal () = 0;
  /// Create a portal object pointing to given sector
  virtual iPortal *CreatePortal (iSector *iTarget) = 0;
  /**
   * Return the pointer to the portal if there is one.
   */
  virtual iPortal* GetPortal () = 0;

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
  virtual void SetTextureSpace (csMatrix3 const&, csVector3 const&) = 0;

  /**
   * With this function you let this polygon share the given plane.
   */
  virtual void SetTextureSpace (iPolyTxtPlane* plane) = 0;

  /**
   * Set type of texturing to use for this polygon (one of
   * the POLYTXT_??? flags). POLYTXT_LIGHTMAP is default.
   * This function is guaranteed not to do anything if the type is
   * already correct.
   */
  virtual void SetTextureType (int type) = 0;
  /**
   * Copy texture type settings from another polygon.
   * (this will not copy the actual material that is used, just the
   * information on how to apply that material to the polygon).
   */
  virtual void CopyTextureType (iPolygon3D* other_polygon) = 0;

  /**
   * Get the polygon texture type (one of POLYTXT_XXX values).
   */
  virtual int GetTextureType () = 0;

  /// Get world space plane.
  virtual const csPlane3& GetWorldPlane () = 0;
  /// Get object space plane.
  virtual const csPlane3& GetObjectPlane () = 0;
  /// Get camera space plane.
  virtual const csPlane3& GetCameraPlane () = 0;

  /**
   * Return true if this polygon or the texture it uses is transparent.
   */
  virtual bool IsTransparent () = 0;

  /**
   * Get cosinus factor.
   */
  virtual float GetCosinusFactor () = 0;
  /**
   * Set cosinus factor.
   */
  virtual void SetCosinusFactor (float cosfact) = 0;
  /**
   * Get the polygon texture type.
   */
  virtual iPolyTexType* GetPolyTexType () = 0;

  /**
   * Update vertex lighting for this polygon. Only works if the
   * polygon uses gouraud shading or is flat-shaded.
   * 'dynamic' is true for a dynamic light.
   * 'reset' is true if the light values need to be reset to 0.
   * 'lcol' is the color of the light. It is given seperately
   * because the color of the light may be modified by portals and
   * other effects.<br>
   * 'light' can be NULL in which case this function is useful
   * for resetting dynamic light values to the static lights ('reset'
   * must be equal to true then).
   * @@@ TEMPORARY FUNCTION: it is better to use the mesh object lighting
   * system for this!
   */
  virtual void UpdateVertexLighting (iLight* light, const csColor& lcol,
  	bool dynamic, bool reset) = 0;
  /**
   * Get a unique ID for this polygon. This is only unique relative
   * to the polygon parent.
   */
  virtual unsigned long GetPolygonID () = 0;

  /**
   * Intersect object-space segment with this polygon. Return
   * true if it intersects and the intersection point in world coordinates.
   */
  virtual bool IntersectSegment (const csVector3& start, const csVector3& end,
                          csVector3& isect, float* pr = NULL) = 0;

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
};

SCF_VERSION (iPolygonTexture, 1, 0, 0);

/**
 * This is a interface to an object responsible for containing
 * the data required for a lighted texture on a polygon.
 */
struct iPolygonTexture : public iBase
{
  /// Get the material handle associated with this polygon
  virtual iMaterialHandle *GetMaterialHandle () = 0;
  /// Get the u-value of the textures bounding box' lower left corner
  virtual float GetFDU () = 0;
  /// Get the v-value of the textures bounding box' lower left corner
  virtual float GetFDV () = 0;
  /// Get width of lighted texture (power of 2)
  virtual int GetWidth () = 0;
  /// Get height of lighted texture.
  virtual int GetHeight () = 0;
  ///Get the degree of the lowest power of 2 that is not smaller than the texture bounding box' width
  /// that is: 2^shift_u >= texbbox-width > 2^(shift_u-1)
  virtual int GetShiftU () = 0;

  /// Get the rounded u-value of the textures bounding box' lower left corner
  virtual int GetIMinU () = 0;
  /// Get the rounded v-value of the textures bounding box' lower left corner
  virtual int GetIMinV () = 0;
  /// Get texture box.
  virtual void GetTextureBox (float& fMinU, float& fMinV,
    float& fMaxU, float& fMaxV) = 0;
  /// Get original width.
  virtual int GetOriginalWidth () = 0;

  /// Get polygon.
  virtual iPolygon3D *GetPolygon () = 0;
  /// Check if dynamic lighting information should be recalculated
  virtual bool DynamicLightsDirty () = 0;
  /**
   * Recalculate all pseudo and real dynamic lights if the
   * texture is dirty. The function returns true if there
   * was a recalculation (then the texture needs to be removed
   * from the texture cache).
   */
  virtual bool RecalculateDynamicLights () = 0;

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

#endif // __IENGINE_POLYGON_H__
