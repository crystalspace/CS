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
#include "thing.h"
#include "polytext.h"
#include "iengine/sector.h"
#include "imesh/thing/polygon.h"

class csFrustumView;
class csFrustumContext;
class csPolyTxtPlane;
class csPolygon2D;
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
 * This is our main static 3D polygon class. Polygons are used to construct the
 * faces of things.
 *<p>
 * Polygons have a texture and lie on a plane. The plane does not
 * define the orientation of the polygon but is derived from it. The plane
 * does define how the texture is scaled and translated accross the surface
 * of the polygon (in case we are talking about lightmapped polygons).
 */
class csPolygon3DStatic : public iBase
{
  friend class csPolyTexture;
  friend class csPolygon3D;

private:
  /// Name of this polygon.
  char* name;

  /*
   * A table of indices into the vertices of the parent csThing
   * (container).
   */
  csPolyIndexed vertices;

  /**
   * The physical parent of this polygon.
   */
  csThingStatic* thing_static;

  /// The object space plane equation (this is fixed).
  csPlane3 plane_obj;
  bool plane_obj_need_update;

  /**
   * The material, this contains the texture handle,
   * the flat color (if no texture) and other parameters.
   */
  iMaterialWrapper* material;

  /**
   * How to map the lightmap on the polygon.
   * Warning! Objects of this type are allocated on
   * csThingObjectType->blk_lightmapmapping.
   */
  //csLightMapMapping* mapping;
  csPolyLightMapMapping* mapping;
  csPolyTextureMapping* tmapping;

  /**
   * 0 is no alpha, 25 is 25% see through and 75% texture and so on.
   * Valid values are from 0 to 100; some renderers in some modes will
   * approximate it (and some values like 25/50/75 are optimized for speed).
   * Note that alpha is in range 0..255, 0 for 0% and 255 for 100%.
   */
  uint16 Alpha;

  /**
   * MixMode to use for drawing this polygon (plus alpha value
   * which is stored separately). The GetMixMode() function will
   * overlap both variables to get one compound value.
   */
  uint MixMode;

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
  virtual ~csPolygon3DStatic ();

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
  void MappingGetTextureSpace (csMatrix3& tx_matrix, csVector3& tx_vector);

  /**
   * Enable or disable texture mapping.
   */
  void EnableTextureMapping (bool enabled);
  bool IsTextureMappingEnabled () const { return mapping != 0; }

  /**
   * Copy texture type settings from another polygon.
   * (this will not copy the actual material that is used, just the
   * information on how to apply that material to the polygon).
   */
  void CopyTextureType (iPolygon3DStatic* other_polygon);

  /**
   * Get the lightmap mapping information.
   */
  csPolyLightMapMapping* GetLightMapMapping () const { return mapping; }
  csPolyTextureMapping* GetTextureMapping () const { return tmapping; }

  /**
   * Clear the polygon (remove all vertices).
   */
  void Reset ();

  /**
   * Add a vertex from the container (polygonset) to the polygon.
   */
  int AddVertex (int v);

  /**
   * Add a vertex to the polygon (and containing thing).
   * Note that it will not check if the vertex is already there.
   * After adding all vertices/polygons you should call
   * CompressVertices() to safe space and gain
   * efficiency.
   */
  int AddVertex (const csVector3& v);

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
  bool Finish ();

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
  csPlane3& GetObjectPlane ()
  { 
    //if (plane_obj_need_update) ComputeNormal();
    return plane_obj;
  }

  /**
   * Get the vertices.
   */
  csPolyIndexed& GetVertices () { return vertices; }

  /**
   * Get number of vertices.
   */
  int GetVertexCount () { return vertices.GetVertexCount (); }

  /**
   * Get vertex index table.
   */
  int* GetVertexIndices () { return vertices.GetVertexIndices (); }

  /**
   * 'idx' is a local index into the vertices table of the polygon.
   * This index is translated to the index in the parent container and
   * a reference to the vertex in object-space is returned.
   */
  const csVector3& Vobj (int idx) const
  { return thing_static->Vobj (vertices.GetVertexIndices ()[idx]); }

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

  /// Calculates the area of the polygon in object space.
  float GetArea ();

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
  void SetTextureSpace (
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
   * lightmap info. This is similar to ObjectToWorld
   * but it does a hard transform of the object space planes
   * instead of keeping a transformation.
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

  /// Get the alpha transparency value for this polygon.
  int GetAlpha () const
  {
    return Alpha;
  }

  /**
   * Set the alpha transparency value for this polygon.
   * Not all renderers support all possible values. 0, 25, 50,
   * 75, and 100 will always work but other values may give
   * only the closest possible to one of the above.
   */
  void SetAlpha (int alpha)
  {
    Alpha = (uint16) alpha;
  }

  /// Get the material handle for the texture manager.
  iMaterialHandle *GetMaterialHandle ();

  /// Sets the mode that is used for DrawPolygonFX.
  void SetMixMode (uint m)
  {
    MixMode = m & ~CS_FX_MASK_ALPHA;
  }

  /// Gets the mode that is used for DrawPolygonFX.
  uint GetMixMode () const
  {
    return (MixMode | Alpha);
  }

  /// Make a clone of this static polygon.
  csPolygon3DStatic* Clone ();

  SCF_DECLARE_IBASE;

  //--------------- iPolygon3DStatic interface implementation ---------------

  struct eiPolygon3DStatic : public iPolygon3DStatic
  {
    SCF_DECLARE_EMBEDDED_IBASE (csPolygon3DStatic);

    virtual csPolygon3DStatic *GetPrivateObject () { return scfParent; }
    virtual const char* GetName () const { return scfParent->GetName (); }
    virtual void SetName (const char* name) { scfParent->SetName (name); }
    virtual iThingFactoryState *GetParent ();
    virtual iMaterialHandle *GetMaterialHandle ()
    { return scfParent->GetMaterialHandle (); }
    virtual void SetMaterial (iMaterialWrapper* mat)
    {
      scfParent->SetMaterial (mat);
    }
    virtual iMaterialWrapper* GetMaterial ()
    {
      return scfParent->GetMaterialWrapper ();
    }

    virtual int GetVertexCount ()
    { return scfParent->vertices.GetVertexCount (); }
    virtual int* GetVertexIndices ()
    { return scfParent->vertices.GetVertexIndices (); }
    virtual const csVector3 &GetVertex (int idx) const
    { return scfParent->Vobj (idx); }
    virtual int CreateVertex (int idx)
    { return scfParent->AddVertex (idx); }
    virtual int CreateVertex (const csVector3 &vertex)
    { return scfParent->AddVertex (vertex); }

    virtual int GetAlpha ()
    { return scfParent->GetAlpha (); }
    virtual void SetAlpha (int iAlpha)
    { scfParent->SetAlpha (iAlpha); }

    virtual void CreatePlane (const csVector3 &origin,
      const csMatrix3 &matrix);

    virtual csFlags& GetFlags ()
    { return scfParent->flags; }

    virtual void SetTextureSpace (
  	const csVector3& p1, const csVector2& uv1,
  	const csVector3& p2, const csVector2& uv2,
  	const csVector3& p3, const csVector2& uv3)
    {
      scfParent->SetTextureSpace (p1, uv1, p2, uv2, p3, uv3);
    }
    virtual void SetTextureSpace (const csVector3& v_orig,
      const csVector3& v1, float l1)
    {
      scfParent->SetTextureSpace (v_orig, v1, l1);
    }
    virtual void SetTextureSpace (
        const csVector3& v_orig,
        const csVector3& v1, float len1,
        const csVector3& v2, float len2)
    {
      scfParent->SetTextureSpace (v_orig, v1, len1, v2, len2);
    }
    virtual void SetTextureSpace (const csMatrix3& m, const csVector3& v)
    {
      scfParent->SetTextureSpace (m, v);
    }
    virtual void GetTextureSpace (csMatrix3& m, csVector3& v)
    {
      scfParent->GetTextureSpace (m, v);
    }
    virtual void EnableTextureMapping (bool enabled)
    {
      scfParent->EnableTextureMapping (enabled);
    }
    virtual bool IsTextureMappingEnabled () const
    {
      return scfParent->IsTextureMappingEnabled ();
    }
    virtual void CopyTextureType (iPolygon3DStatic* other_polygon)
    {
      scfParent->CopyTextureType (other_polygon);
    }

    virtual const csPlane3& GetObjectPlane ()
    {
      return scfParent->GetObjectPlane();
    }
    virtual bool IsTransparent ()
    {
      return scfParent->IsTransparent ();
    }
    virtual void SetMixMode (uint m)
    {
      scfParent->SetMixMode (m);
    }
    virtual uint GetMixMode ()
    {
      return scfParent->GetMixMode ();
    }
    virtual bool IntersectSegment (const csVector3& start, const csVector3& end,
                          csVector3& isect, float* pr = 0)
    {
      return scfParent->IntersectSegment (start, end, isect, pr);
    }
    virtual bool IntersectRay (const csVector3& start, const csVector3& end)
    {
      return scfParent->IntersectRay (start, end);
    }
    virtual bool IntersectRayNoBackFace (const csVector3& start,
      const csVector3& end)
    {
      return scfParent->IntersectRayNoBackFace (start, end);
    }
    virtual bool IntersectRayPlane (const csVector3& start,
      const csVector3& end, csVector3& isect)
    {
      return scfParent->IntersectRayPlane (start, end, isect);
    }
    virtual bool PointOnPolygon (const csVector3& v)
    {
      return scfParent->PointOnPolygon (v);
    }

    virtual csPolyLightMapMapping* GetLightMapMapping () const 
    { 
      return scfParent->GetLightMapMapping ();
    }
    csPolyTextureMapping* GetTextureMapping () const 
    { 
      return scfParent->GetTextureMapping (); 
    }
  } scfiPolygon3DStatic;
  friend struct eiPolygon3DStatic;
};

/**
 * This is our main 3D polygon class. Polygons are used to construct the
 * faces of things.
 */
class csPolygon3D : public iBase
{
  friend class csPolyTexture;

private:
  /// Pointer to static data.
  csPolygon3DStatic* static_data;

  /**
   * @@@@@@@@@@@@@ DO WE NEED THIS HERE?
   * The physical parent of this polygon.
   */
  csThing* thing;

  iMaterialWrapper* material;

  /// The world space plane equation.
  csPlane3 plane_wor;

  /**
   * List of light patches for this polygon.
   */
  csLightPatch *lightpatches;

  /**
   * Texture type specific information for this polygon.
   * Warning! Objects of this type are allocated on
   * csThingObjectType->blk_polytex.
   */
  csPolyTexture *txt_info;

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
  virtual ~csPolygon3D ();

  /**
   * Get the lightmap information.
   */
  csPolyTexture* GetPolyTexture () const { return txt_info; }

  /**
   * Set the static data.
   */
  void SetStaticData (csPolygon3DStatic* static_data);

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
  void Finish ();

  /**
   * Refresh texture mapping and other info from static polygon.
   */
  void RefreshFromStaticData ();

  void SetMaterial (iMaterialWrapper* mat) { material = mat; }
  iMaterialWrapper* GetMaterial () { return material; }
  /**
   * Get the real material to use for rendering this polygon.
   * This will first check the local material and if that is 0
   * return the one from the factory.
   */
  iMaterialWrapper* GetRealMaterial ()
  {
    return material ? material : static_data->GetMaterialWrapper ();
  }

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
   * Get static data.
   */
  csPolygon3DStatic* GetStaticData () const { return static_data; }

  /**
   * Return the world-space plane of this polygon.
   */
  const csPlane3& GetPolyPlane () const { return plane_wor; }

  /**
   * Return the world-space plane of this polygon.
   */
  csPlane3& GetWorldPlane () { return plane_wor; }

  /**
   * Transform the plane of this polygon from world space to camera space using
   * the given matrices. One vertex on the plane is also given so
   * that we can more easily recompute the 'D' component of the plane.
   * The given vertex should be in camera space.
   */
  void WorldToCameraPlane (
  	const csReversibleTransform& t,
	const csVector3& vertex1,
	csPlane3& camera_plane);

  /**
   * Other version which computes the camera space vertex itself.
   */
  void ComputeCameraPlane (const csReversibleTransform& t,
  	csPlane3& pl);

  /**
   * 'idx' is a local index into the vertices table of the polygon.
   * This index is translated to the index in the parent container and
   * a reference to the vertex in world-space is returned.
   */
  const csVector3& Vwor (int idx) const
  { return thing->Vwor (static_data->vertices.GetVertexIndices ()[idx]); }

  /**
   * 'idx' is a local index into the vertices table of the polygon.
   * This index is translated to the index in the parent container and
   * a reference to the vertex in camera-space is returned.
   */
  const csVector3& Vcam (int idx) const
  { return thing->Vcam (static_data->vertices.GetVertexIndices ()[idx]); }

  /**
   * Before calling a series of Vcam() you should call
   * UpdateTransformation() first to make sure that the camera vertex set
   * is up-to-date.
   */
  void UpdateTransformation (const csTransform& c, long cam_cameranr)
  {
    thing->UpdateTransformation (c, cam_cameranr);
  }

  /**
   * Before calling a series of Vwor() you should call
   * WorUpdate() first to make sure that the world vertex set
   * is up-to-date.
   */
  void WorUpdate () { thing->WorUpdate (); }

  /**
   * Disconnect a dynamic light from this polygon.
   */
  void DynamicLightDisconnect (iDynLight* dynlight);

  /**
   * Disconnect a static light from this polygon.
   * Only works for pseudo-dynamic lights.
   */
  void StaticLightDisconnect (iStatLight* statlight);

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
   * Clip a polygon against a plane (in camera space).
   * The plane is defined as going through v1, v2, and (0,0,0).
   * The 'verts' array is modified and 'num' is also modified if needed.
   */
  void ClipPolyPlane (csVector3* verts, int* num, bool mirror,
  	csVector3& v1, csVector3& v2);

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
  const char* ReadFromCache (iFile* file);

  /**
   * Call after calling InitializeDefault() and CalculateLighting to cache
   * the calculated lightmap to the file.
   */
  bool WriteToCache (iFile* file);

  /**
   * Prepare the lightmaps for use.
   * This function also converts the lightmaps to the correct
   * format required by the 3D driver. This function does NOT
   * create the first lightmap. This is done by the precalculated
   * lighting process (using CalculateLighting()).
   */
  void PrepareLighting ();

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
   */
  void FillLightMapDynamic (iFrustumView* lview);

  /**
   * Fill the lightmap of this polygon according to the given light and
   * the frustum. The light is given in world space coordinates. The
   * view frustum is given in camera space (with (0,0,0) the origin
   * of the frustum). The camera space used is just world space translated
   * so that the center of the light is at (0,0,0).
   * If the lightmaps were cached in the level archive this function will
   * do nothing.<p>
   *
   * The "frustum" parameter defines the original light frustum (not the
   * one bounded by this polygon as given by "lview").<p>
   *
   * If 'vis' == false this means that the lighting system already discovered
   * that the polygon is totally shadowed.
   */
  void FillLightMapStatic (iFrustumView* lview, csLightingPolyTexQueue* lptq,
  	bool vis);

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
  bool MarkRelevantShadowFrustums (iFrustumView* lview, csPlane3& plane);

  /**
   * Same as above but takes polygon plane as 'plane' argument.
   */
  bool MarkRelevantShadowFrustums (iFrustumView* lview);

  /**
   * Check visibility of this polygon with the given csFrustumView
   * and update the light patches if needed.
   * This version is for dynamic lighting.
   */
  void CalculateLightingDynamic (iFrustumView* lview, iMovable* movable);

  /**
   * Check visibility of this polygon with the given csFrustumView
   * and fill the lightmap if needed (this function calls FillLightMap ()).
   * If 'vis' == false this means that the lighting system already discovered
   * that the polygon is totally shadowed.
   * This version is for static lighting.
   */
  void CalculateLightingStatic (iFrustumView* lview, iMovable* movable,
  	csLightingPolyTexQueue* lptq, bool vis);

  /**
   * Transform the plane of this polygon from object space to world space.
   * 'vt' is a vertex of this polygon in world space.
   */
  void ObjectToWorld (const csReversibleTransform& t, const csVector3& vwor);

  /**
   * Clip this camera space polygon to the given plane. 'plane' can be 0
   * in which case no clipping happens.<p>
   *
   * If this function returns false then the polygon is not visible (backface
   * culling, no visible vertices, ...) and 'verts' will be 0. Otherwise
   * this function will return true and 'verts' will point to the new
   * clipped polygon (this is a pointer to a static table of vertices.
   * WARNING! Because of this you cannot do new ClipToPlane calls until
   * you have processed the 'verts' array!).<p>
   *
   * If 'cw' is true the polygon has to be oriented clockwise in order to be
   * visible. Otherwise it is the other way around.
   */
  bool ClipToPlane (csPlane3* portal_plane, const csVector3& v_w2c,
  	csVector3*& pverts, int& num_verts, bool cw = true);

  /**
   * This is the link between csPolygon3D and csPolygon2D (see below for
   * more info about csPolygon2D). It should be used after the parent
   * container has been transformed from world to camera space.
   * It will fill the given csPolygon2D with a perspective corrected
   * polygon that is also clipped to the view plane (Z=SMALL_Z).
   * If all vertices are behind the view plane the polygon will not
   * be visible and it will return false.
   * 'do_perspective' will also do back-face culling and returns false
   * if the polygon is not visible because of this.
   * If the polygon is deemed to be visible it will return true.
   */
  bool DoPerspective (csVector3* source,
  	int num_verts, csPolygon2D* dest, bool mirror,
	int fov, float shift_x, float shift_y,
	const csPlane3& plane_cam);

  SCF_DECLARE_IBASE;

  //------------------- iPolygon3D interface implementation -------------------

  struct eiPolygon3D : public iPolygon3D
  {
    SCF_DECLARE_EMBEDDED_IBASE (csPolygon3D);

    virtual csPolygon3D *GetPrivateObject () { return scfParent; }
    virtual iThingState *GetParent ();
    virtual iPolygon3DStatic* GetStaticData () const
    {
      return &(scfParent->GetStaticData ()->scfiPolygon3DStatic);
    }

    virtual iLightMap *GetLightMap ()
    {
      csPolyTexture *lm = scfParent->txt_info;
      return lm ? (iLightMap*)(lm->GetLightMapFast) () : (iLightMap*)0;
    }
    virtual iPolygonTexture *GetTexture ()
    {
      return scfParent->GetPolyTexture ();
    }
    virtual const csVector3 &GetVertexW (int idx) const
    { return scfParent->Vwor (idx); }
    virtual const csVector3 &GetVertexC (int idx) const
    { return scfParent->Vcam (idx); }

    virtual const csPlane3& GetWorldPlane ()
    {
      return scfParent->plane_wor;
    }
    virtual void ComputeCameraPlane (const csReversibleTransform& t,
  	csPlane3& pl)
    {
      scfParent->ComputeCameraPlane (t, pl);
    }
    virtual void SetMaterial (iMaterialWrapper* mat)
    {
      scfParent->SetMaterial (mat);
    }
    virtual iMaterialWrapper* GetMaterial ()
    {
      return scfParent->GetMaterial ();
    }
  } scfiPolygon3D;
  friend struct eiPolygon3D;
};

#endif // __CS_POLYGON_H__
