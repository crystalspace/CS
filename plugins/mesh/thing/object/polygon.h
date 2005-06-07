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

#ifndef __CS_POLYGON_H__
#define __CS_POLYGON_H__

#include "csutil/scf.h"
#include "csutil/cscolor.h"
#include "csutil/flags.h"
#include "csutil/util.h"
#include "csgeom/transfrm.h"
#include "csgeom/polyclip.h"
#include "csgeom/polyidx.h"
#include "cstool/userrndbuf.h"
#include "polytext.h"
#include "iengine/movable.h"
#include "iengine/sector.h"
#include "ivideo/polyrender.h"

class csFrustumView;
class csFrustumContext;
class csPolyTxtPlane;
class csPolygon3D;
class csLightMap;
class csLightPatch;
class csPolyTexture;
class csThing;
class csThingStatic;
struct iFile;
struct iLight;
struct iGraphics2D;
struct iGraphics3D;
struct iCacheManager;
struct iMaterialWrapper;

/*---------------------------------------------------------------------------*/

/*
 * Additional polygon flags. These flags are private,
 * unlike those defined in ipolygon.h
 */

/**
 * This flag is set if the renderer can't handle the lightmap.
 * Lighting is still calculated, but the lightmap isn't passed to the
 * renderer.
 */
#define CS_POLY_LM_REFUSED	0x10000000

/**
 * This specifies the minimum allowed determinant for UV coordinate mapping
 * on the polygon.  Lower this to allow polygon that use a very small portion
 * of a texture map.  This cannot be made zero, otherwise division errors
 * will occur.
 */
#define CS_POLY_MIN_UV_DET  0.0001f

/**
 * This is our main static 3D polygon class. Polygons are used to construct the
 * faces of things.
 *<p>
 * Polygons have a texture and lie on a plane. The plane does not
 * define the orientation of the polygon but is derived from it. The plane
 * does define how the texture is scaled and translated accross the surface
 * of the polygon (in case we are talking about lightmapped polygons).
 */
class csPolygon3DStatic
{
  friend class csPolyTexture;
  friend class csPolygon3D;
  friend class csThingStatic;
  friend class csThing;

private:
  /// Name of this polygon.
  char* name;			// @@@ Avoid this?

  /**
   * The physical parent of this polygon.
   */
  csThingStatic* thing_static;	// @@@ Can probably be removed easily!!!

  /**
   * Render data for the renderer. This contains information like
   * object plane, the vertices (in index form) and texture mapping information.
   */
  csPolygonRenderData polygon_data;

  /**
   * The material, this contains the texture handle,
   * the flat color (if no texture) and other parameters.
   */
  csRef<iMaterialWrapper> material;

  csUserRenderBufferManager polyBuffers;

  /**
   * Return twice the signed area of the polygon in world space coordinates
   * using the yz, zx, and xy components.  In effect this calculates the
   * (P,Q,R) or the plane normal of the polygon.
   */
  void PlaneNormal (float* yz, float* zx, float* xy);

public:
  /// Set of flags
  csFlags flags;

public:
  /**
   * Construct a new polygon with the given material.
   * Warning! Objects of this type are allocated on
   * csThingObjectType->blk_polygon3dstatic.
   */
  csPolygon3DStatic ();

  /**
   * Delete everything related to this polygon. Less is
   * deleted if this polygon is a copy of another one (because
   * some stuff is shared).
   */
  ~csPolygon3DStatic ();

  ///
  void MappingSetTextureSpace (const csVector3& v_orig,
			const csVector3& v1, float len1,
			const csVector3& v2, float len2);
  ///
  void MappingSetTextureSpace (const csPlane3& plane_wor,
  			float xo, float yo, float zo,
			float x1, float y1, float z1,
			float len);
  ///
  void MappingSetTextureSpace (const csPlane3& plane_wor,
  			const csVector3& v_orig,
  			const csVector3& v1, float len);
  ///
  void MappingSetTextureSpace (const csVector3& v_orig,
  			const csVector3& v_u,
			const csVector3& v_v);
  ///
  void MappingSetTextureSpace (float xo, float yo, float zo,
			float xu, float yu, float zu,
			float xv, float yv, float zv);
  ///
  void MappingSetTextureSpace (float xo, float yo, float zo,
			float xu, float yu, float zu,
			float xv, float yv, float zv,
			float xw, float yw, float zw);
  ///
  void MappingSetTextureSpace (const csMatrix3& tx_matrix,
  			const csVector3& tx_vector);

  /// Get the transformation from object to texture space.
  void MappingGetTextureSpace (csMatrix3& tx_matrix, csVector3& tx_vector)
  {
    tx_matrix = polygon_data.tmapping->GetO2T ();
    tx_vector = polygon_data.tmapping->GetO2TTranslation ();
  }

  /**
   * Enable or disable texture mapping.
   */
  void EnableTextureMapping (bool enabled);
  bool IsTextureMappingEnabled () const { return polygon_data.tmapping != 0; }

  /**
   * Get the lightmap mapping information.
   */
  csPolyTextureMapping* GetTextureMapping () const
  {
    return polygon_data.tmapping;
  }

  /**
   * Clear the polygon (remove all vertices).
   */
  void Reset ();

  /**
   * Set the number of vertices. This will properly reallocate the vertices
   * table into the right block allocator.
   */
  void SetNumVertices (int count);

  /**
   * Add a vertex from the container (polygonset) to the polygon.
   */
  int AddVertex (int v);

  /**
   * Set a vertex from the container (polygonset) to the polygon.
   */
  void SetVertex (int idx, int v);

  /**
   * Add a vertex to the polygon (and containing thing).
   * Note that it will not check if the vertex is already there.
   * After adding all vertices/polygons you should call
   * CompressVertices() to safe space and gain
   * efficiency.
   */
  int AddVertex (const csVector3& v);

  /**
   * Set a vertex to the polygon (and containing thing).
   */
  void SetVertex (int idx, const csVector3& v);

  /**
   * Add a vertex to the polygon (and containing thing).
   * Note that it will not check if the vertex is already there.
   * After adding all vertices/polygons you should call
   * CompressVertices() to safe space and gain
   * efficiency.
   */
  int AddVertex (float x, float y, float z);

  /**
   * Precompute the plane normal. Normally this is done automatically by
   * set_texture_space but if needed you can call this function again when
   * something has changed.
   */
  void ComputeNormal ();

  /**
   * Calculate the bounding box in (u,v) space for the lighted texture.
   * This is used in case of lightmapping.
   * <br>
   * This function returns false if the texture handle was not present yet.
   * In that case the initialization is not completely correct yet and has
   * to be redone later when the texture is prepared.
   */
  bool CreateBoundingTextureBox ();

  /**
   * After the plane normal and the texture matrices have been set
   * up this routine makes some needed pre-calculations for this polygon.
   * It will create a texture space bounding box that
   * is going to be used for lighting and the texture cache.
   * Then it will allocate the light map tables for this polygons.
   * You also need to call this function if you make a copy of a
   * polygon (using the copy constructor) or if you change the vertices
   * in a polygon.
   * <br>
   * This function returns false if the texture handle was not present yet.
   * In that case the initialization is not completely correct yet and has
   * to be redone later when the texture is prepared.
   */
  bool Finish (iBase* thing_logparent);

  /**
   * Set the thing that this polygon belongs to.
   */
  void SetParent (csThingStatic* thing_static);

  /**
   * Get the polygonset (container) that this polygons belongs to.
   */
  csThingStatic* GetParent () { return thing_static; }

  /// Name handling.
  const char* GetName () const { return name; }
  void SetName (const char* n)
  {
    delete[] name;
    if (n)
      name = csStrNew (n);
    else
      name = 0;
  }

  /**
   * Return the object-space plane of this polygon.
   */
  const csPlane3& GetObjectPlane () const
  { 
    return polygon_data.plane_obj;
  }

  /**
   * Set the object-space plane.
   */
  void SetObjectPlane (const csPlane3& p)
  {
    polygon_data.plane_obj = p;
  }

  /**
   * Get number of vertices.
   */
  int GetVertexCount () const { return polygon_data.num_vertices; }

  /**
   * Get vertex index table.
   */
  int* GetVertexIndices () { return polygon_data.vertices; }

  /**
   * 'idx' is a local index into the vertices table of the polygon.
   * This index is translated to the index in the parent container and
   * a reference to the vertex in object-space is returned.
   */
  const csVector3& Vobj (int idx) const;

  /**
   * Set the material for this polygon.
   * This material handle will only be used as soon as 'Finish()'
   * is called. So you can safely wait preparing the materials
   * until finally csEngine::Prepare() is called (which in the end
   * calls Finish() for every polygon).
   */
  void SetMaterial (iMaterialWrapper* material);

  /**
   * Get the material.
   */
  iMaterialWrapper* GetMaterialWrapper () { return material; }

  /**
   * Return true if this polygon or the texture it uses is transparent.
   */
  bool IsTransparent ();

  /*
   * One of the SetTextureSpace functions should be called after
   * adding all vertices to the polygon (not before) and before
   * doing any processing on the polygon (not after)!
   * It makes sure that the plane normal is correctly computed and
   * the texture and plane are correctly initialized.
   *<p>
   * Internally the transformation from 3D to texture space is
   * represented by a matrix and a vector. You can supply this
   * matrix directly or let it be calculated from other parameters.
   * If you supply another Polygon or a csPolyPlane to this function
   * it will automatically share the plane.
   *<p>
   * This version copies the plane from the other polygon. The plane
   * is shared with that other plane and this allows the engine to
   * do some optimizations. This polygon is not responsible for
   * cleaning this plane.
   */

  /**
   * Set the texture space transformation given three vertices and
   * their uv coordinates.
   */
  bool SetTextureSpace (
  	const csVector3& p1, const csVector2& uv1,
  	const csVector3& p2, const csVector2& uv2,
  	const csVector3& p3, const csVector2& uv3);

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
  void SetTextureSpace (const csVector3& v_orig,
    const csVector3& v1, float len1);

  /**
   * Calculate the matrix using two vertices (which are preferably on the
   * plane of the polygon and are possibly (but not necessarily) two vertices
   * of the polygon). The first vertex is seen as the origin and the second
   * as the u-axis of the texture space coordinate system. The v-axis is
   * calculated on the plane of the polygon and orthogonal to the given
   * u-axis. The length of the u-axis and the v-axis is given as the 'len1'
   * parameter.
   */
  void SetTextureSpace (
    float xo, float yo, float zo,
    float x1, float y1, float z1, float len1);

  /**
   * Calculate the matrix using 'v1' and 'len1' for the u-axis and
   * 'v2' and 'len2' for the v-axis.
   */
  void SetTextureSpace (
    const csVector3& v_orig,
    const csVector3& v1, float len1,
    const csVector3& v2, float len2);

  /**
   * The same but all in floats.
   */
  void SetTextureSpace (
    float xo, float yo, float zo,
    float x1, float y1, float z1, float len1,
    float x2, float y2, float z2, float len2);

  /**
   * The most general function. With these you provide the matrix
   * directly.
   */
  void SetTextureSpace (const csMatrix3&, const csVector3&);
  /**
   * Get txt mapping info.
   */
  void GetTextureSpace (csMatrix3&, csVector3&);

  /**
   * Hard transform the plane of this polygon and also the
   * lightmap info. This does a hard transform of the object space
   * planes.
   */
  void HardTransform (const csReversibleTransform& t);

  /**
   * Classify this polygon with regards to a plane (in object space).  If this
   * poly is on same plane it returns CS_POL_SAME_PLANE.  If this poly is
   * completely in front of the given plane it returnes CS_POL_FRONT.  If this
   * poly is completely back of the given plane it returnes CS_POL_BACK.
   * Otherwise it returns CS_POL_SPLIT_NEEDED.
   */
  int Classify (const csPlane3& pl);

  /// Same as Classify() but for X plane only.
  int ClassifyX (float x);

  /// Same as Classify() but for Y plane only.
  int ClassifyY (float y);

  /// Same as Classify() but for Z plane only.
  int ClassifyZ (float z);

  /**
   * Check if this polygon (partially) overlaps the other polygon
   * from some viewpoint in space. This function works in object space.
   */
  bool Overlaps (csPolygon3DStatic* overlapped);

  /**
   * Intersect object-space segment with the plane of this polygon. Return
   * true if it intersects and the intersection point in world coordinates.
   */
  bool IntersectSegmentPlane (const csVector3& start, const csVector3& end,
                          csVector3& isect, float* pr = 0) const;

  /**
   * Intersect object-space segment with this polygon. Return
   * true if it intersects and the intersection point in world coordinates.
   */
  bool IntersectSegment (const csVector3& start, const csVector3& end,
                          csVector3& isect, float* pr = 0);

  /**
   * Intersect object-space ray with this polygon. This function
   * is similar to IntersectSegment except that it doesn't keep the lenght
   * of the ray in account. It just tests if the ray intersects with the
   * interior of the polygon. Note that this function also does back-face
   * culling.
   */
  bool IntersectRay (const csVector3& start, const csVector3& end);

  /**
   * Intersect object-space ray with this polygon. This function
   * is similar to IntersectSegment except that it doesn't keep the lenght
   * of the ray in account. It just tests if the ray intersects with the
   * interior of the polygon. Note that this function doesn't do
   * back-face culling.
   */
  bool IntersectRayNoBackFace (const csVector3& start, const csVector3& end);

  /**
   * Intersect object space ray with the plane of this polygon and
   * returns the intersection point. This function does not test if the
   * intersection is inside the polygon. It just returns the intersection
   * with the plane (in or out). This function returns false if the ray
   * is parallel with the plane (i.e. there is no intersection).
   */
  bool IntersectRayPlane (const csVector3& start, const csVector3& end,
  	csVector3& isect);

  /**
   * This is a given point is on (or very nearly on) this polygon.
   * Test happens in object space.
   */
  bool PointOnPolygon (const csVector3& v);

  /// Get the material handle for the texture manager.
  iMaterial* GetMaterial ();

  /// Make a clone of this static polygon.
  csPolygon3DStatic* Clone (csThingStatic* new_parent);
};

/**
 * This is our main 3D polygon class. Polygons are used to construct the
 * faces of things.
 */
class csPolygon3D
{
  friend class csPolyTexture;

private:
  /**
   * @@@@@@@@@@@@@ DO WE NEED THIS HERE?
   * The physical parent of this polygon.
   */
  csThing* thing;

  /**
   * List of light patches for this polygon.
   */
  csLightPatch *lightpatches;

  /**
   * Texture type specific information for this polygon.
   */
  csPolyTexture txt_info;

public:
  /**
   * Construct a new polygon with the given material.
   * Warning! Objects of this type are allocated on
   * csThingObjectType->blk_polygon3d.
   */
  csPolygon3D ();

  /**
   * Delete everything related to this polygon. Less is
   * deleted if this polygon is a copy of another one (because
   * some stuff is shared).
   */
  ~csPolygon3D ();

  /// Remove the polygon texture.
  void RemovePolyTexture ();

  /**
   * Get the lightmap information.
   */
  csPolyTexture* GetPolyTexture () { return &txt_info; }

  /**
   * Get index of this polygon.
   */
  int GetPolyIdx () const;

  /**
   * Get static polygon.
   */
  csPolygon3DStatic* GetStaticPoly () const;

  /**
   * After the plane normal and the texture matrices have been set
   * up this routine makes some needed pre-calculations for this polygon.
   * It will create a texture space bounding box that
   * is going to be used for lighting and the texture cache.
   * Then it will allocate the light map tables for this polygons.
   * You also need to call this function if you make a copy of a
   * polygon (using the copy constructor) or if you change the vertices
   * in a polygon.
   */
  void Finish (csPolygon3DStatic* spoly);

  /**
   * Refresh texture mapping and other info from static polygon.
   */
  void RefreshFromStaticData ();

  /**
   * @@@@@ NEEDED?
   * Set the thing that this polygon belongs to.
   */
  void SetParent (csThing* thing);

  /**
   * @@@@@ NEEDED?
   * Get the polygonset (container) that this polygons belongs to.
   */
  csThing* GetParent () { return thing; }

  /**
   * Disconnect a dynamic light from this polygon.
   */
  void DynamicLightDisconnect (iLight* dynlight);

  /**
   * Disconnect a static light from this polygon.
   * Only works for pseudo-dynamic lights.
   */
  void StaticLightDisconnect (iLight* statlight);

  /**
   * Unlink a light patch from the light patch list.
   * Warning! This function does not test if the light patch
   * is really on the list!
   */
  void UnlinkLightpatch (csLightPatch* lp);

  /**
   * Add a light patch to the light patch list.
   */
  void AddLightpatch (csLightPatch *lp);

  /**
   * Get the list of light patches for this polygon.
   */
  csLightPatch* GetLightpatches () { return lightpatches; }

  /**
   * Initialize the lightmaps for this polygon.
   * Should be called before calling CalculateLighting() and before
   * calling WriteToCache().
   */
  void InitializeDefault (bool clear);

  /**
   * This function will try to read the lightmap from the given file.
   * Return 0 on success or else an error message.
   */
  const char* ReadFromCache (iFile* file, csPolygon3DStatic* spoly);

  /**
   * Call after calling InitializeDefault() and CalculateLighting to cache
   * the calculated lightmap to the file.
   */
  bool WriteToCache (iFile* file, csPolygon3DStatic* spoly);

  /**
   * Fill the lightmap of this polygon according to the given light and
   * the frustum. The light is given in world space coordinates. The
   * view frustum is given in camera space (with (0,0,0) the origin
   * of the frustum). The camera space used is just world space translated
   * so that the center of the light is at (0,0,0).
   * If the lightmaps were cached in the level archive this function will
   * do nothing.
   * The "frustum" parameter defines the original light frustum (not the
   * one bounded by this polygon as given by "lview").
   * This function returns true if the light actually affected the
   * polygon. False otherwise.
   */
  bool FillLightMapDynamic (iFrustumView* lview, csFrustum* frustum);

  /**
   * Check all shadow frustums and mark all relevant ones. A shadow
   * frustum is relevant if it is (partially) inside the light frustum
   * and if it is not obscured by other shadow frustums.
   * In addition to the checking above this routine will return false
   * if it can find a shadow frustum which totally obscures the light
   * frustum. In this case it makes no sense to continue lighting the
   * polygon.<br>
   * This function will also discard all shadow frustums which start at
   * the same plane as the given plane.
   */
  bool MarkRelevantShadowFrustums (iFrustumView* lview, csPlane3& plane,
  	csPolygon3DStatic* spoly);

  /**
   * Check visibility of this polygon with the given csFrustumView
   * and update the light patches if needed.
   * This version is for dynamic lighting.
   * This function returns true if the light actually affected the
   * polygon. False otherwise.
   */
  bool CalculateLightingDynamic (iFrustumView* lview, iMovable* movable,
  	const csPlane3& world_plane, csPolygon3DStatic* spoly);

  /**
   * Check visibility of this polygon with the given csFrustumView
   * and fill the lightmap if needed (this function calls FillLightMap ()).
   * If 'vis' == false this means that the lighting system already discovered
   * that the polygon is totally shadowed.
   * This version is for static lighting.
   * This function returns true if the light actually affected the
   * polygon. False otherwise.
   */
  bool CalculateLightingStatic (iFrustumView* lview, iMovable* movable,
  	csLightingPolyTexQueue* lptq, bool vis,
	const csMatrix3& m_world2tex,
	const csVector3& v_world2tex,
	const csPlane3& world_plane,
	csPolygon3DStatic* spoly);
};

#endif // __CS_POLYGON_H__
