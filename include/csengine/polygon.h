/*
    Copyright (C) 1998 by Jorrit Tyberghein

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
#include "csgeom/transfrm.h"
#include "csgeom/polyclip.h"
#include "csgeom/polyidx.h"
#include "csengine/polyint.h"
#include "csengine/polyplan.h"
#include "csengine/polyset.h"
#include "csengine/portal.h"
#include "csengine/polytext.h"
#include "csengine/octree.h"
#include "ipolygon.h"
#include "csengine/material.h"

class csSector;
class StatLight;
class CLights;
class csTextureHandle;
class csPolyPlane;
class csPolyTxtPlane;
class csPolygon2D;
class csPolygon3D;
class csLightMap;
class csLightPatch;
class Dumper;
class csPolyTexture;
class csPolygonSet;
struct iGraphics2D;
struct iGraphics3D;

/**
 * Additional polygon flags. These flags are private,
 * unlike those defined in ipolygon.h
 */

/**
 * If this flag is set this portal was allocated by this polygon
 * and it should also be deleted by it.
 */
#define CS_POLY_DELETE_PORTAL	0x80000000

/**
 * If this flag is true then this polygon will never be drawn.
 * This is useful for polygons which have been split. The original
 * unsplit polygon is still kept because it holds the shared
 * information about the lighted texture and lightmaps (all split
 * children refer to the original polygon for that).
 */
#define CS_POLY_NO_DRAW		0x40000000

/// Texture type is lightmapped
#define POLYTXT_LIGHTMAP	1
/// Gouraud Shaded texture
#define POLYTXT_GOURAUD		2

/**
 * Structure containing lighting information valid
 * for all types of polygons.
 */
struct csPolygonLightInfo
{
  /**
   * This field describes how the light hitting this polygon is affected
   * by the angle by which the beam hits the polygon. If this value is
   * equal to -1 (default) then the global csPolyTexture::cfg_cosinus_factor
   * will be used.
   */
  float cosinus_factor;

  /**
   * Flat shading color.
   */
  csColor flat_color;

  /**
   * List of light patches for this polygon.
   */
  csLightPatch* lightpatches;

  /**
   * True if we have a dirty polygon due to dynamic lighting.
   */
  bool dyn_dirty;
};

/**
 * Kind of texturing that is used for a 3D polygon.
 * This is the base class with subclasses depending on the kind
 * of texturing that is used for a polygon.
 */
class csPolygonTextureType
{
private:
  /// Reference counter
  uint32 ref_count;

protected:
  /// Common constructor for derived classes
  csPolygonTextureType () { ref_count = 1; }
  /// Destructor is virtual to be able to delete derived objects
  virtual ~csPolygonTextureType () { }

public:
  /// Return a type for the kind of texturing used.
  virtual int GetTextureType () = 0;
  /// Maintain a reference counter for texture type objects
  void IncRef () { ref_count++; }
  /// Decrement usage counter
  void DecRef () { if (!--ref_count) delete this; }
};

/**
 * Texture class for lightmapped polygons.
 */
class csLightMapped : public csPolygonTextureType
{
  friend class csPolygon3D;

private:
  /// The transformed texture for this polygon.
  csPolyTexture* tex;

  /**
   * The csPolyTxtPlane for this polygon.
   */
  csPolyTxtPlane* txt_plane;

  /**
   * This bool indicates if the lightmap is up-to-date (read from the
   * cache). If set to false the polygon still needs to be recalculated.
   */
  bool lightmap_up_to_date;

  /**
   * A uniform dynamic light for this poly.
   * This is used for flashing the polygon in a uniform way
   * (unlike positional dynamic lights).
   */
  CLights* theDynLight;

  /**
   * 0 is no alpha, 25 is 25% see through and
   * 75% texture, ... Possible values are 0, 25, 50, and 75.
   */
  int cfg_alpha;

private:
  /// Constructor.
  csLightMapped ();

  /// Destructor.
  virtual ~csLightMapped ();

public:
  /// Setup for the given polygon and texture.
  void Setup (csPolygon3D* poly3d, csTextureHandle* texh);

  /// Return a type for the kind of texturing used.
  virtual int GetTextureType () { return POLYTXT_LIGHTMAP; }

  ///
  int GetAlpha () { return cfg_alpha; }
  ///
  void SetAlpha (int a) { cfg_alpha = a; }

  /// Get the polytexture (lighted texture)
  csPolyTexture* GetPolyTex ();

  /**
   * Return the texture plane of this polygon.
   */
  csPolyTxtPlane* GetTxtPlane () const { return txt_plane; }

  /**
   * Set the texture plane.
   */
  void SetTxtPlane (csPolyTxtPlane* txt_pl);

  /**
   * Create a new texture plane.
   */
  void NewTxtPlane ();

  /**
   * Get the lightmap belonging with this polygon.
   */
  iLightMap* GetLightMap () { return tex->GetLightMap (); }

  /**
   * Set the uniform dynamic light.
   */
  void SetUniformDynLight (CLights* l) { theDynLight = l; }
};

/**
 * Lighting class for gouraud shaded polygons.
 */
class csGouraudShaded : public csPolygonTextureType
{
  friend class csPolygon3D;

private:
  /**
   * If the following arrays is non-NULL then this polygon is drawn
   * using a different technique (all previous lightmap stuff
   * is ignored). The polygon will be drawn with perspective incorrect
   * texture mapping and (in the future) Gouroud shading. The following
   * array specifies the u,v coordinates for every vertex of the polygon.
   */
  csVector2* uv_coords;

  /**
   * If uv_coords is given then this can be an optional array of vertex
   * colors. If this array is given then gouraud shading is used. Otherwise
   * flatshading.
   */
  csColor* colors;

  /**
   * This array contains the static colors. It is used in combination with
   * 'colors'.  'colors=static_colors+dynamic_lights'.
   */
  csColor* static_colors;

  /// Number of vertices.
  int num_vertices;

  /// Mixmode to use for drawing this polygon.
  UInt MixMode;

private:
  /// Constructor.
  csGouraudShaded ();

  /// Destructor.
  virtual ~csGouraudShaded ();

public:
  /// Return a type for the kind of texturing used.
  virtual int GetTextureType () { return POLYTXT_GOURAUD; }

  /**
   * Setup this lighting structure with the rignt number of
   * vertices. If use_gouraud is true we will also use gouraud shading.
   * This function is guaranteed not to do anything if the number of
   * vertices is already correct.
   */
  void Setup (int num_vertices);

  /// Sets the mode that is used for DrawPolygonFX.
  void SetMixmode (UInt m) { MixMode = m; }

  /// Gets the mode that is used for DrawPolygonFX.
  UInt GetMixmode () { return MixMode; }

  /**
   * Enable/disable gouraud.
   * This function is guaranteed not to do anything if gouraud shading
   * was already enabled and it is enabled again.
   */
  void EnableGouraud (bool g);

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
  void SetUV (int i, float u, float v);

  /**
   * Clear all (u,v) and color information.
   */
  void Clear ();

  /// Get the pointer to the vertex uv coordinates.
  csVector2* GetUVCoords () { return uv_coords; }

  /// Get the pointer to the vertex color table.
  csColor* GetColors () { return colors; }

  /// Get the pointer to the static vertex color table.
  csColor* GetStaticColors () { return static_colors; }

  /**
   * Add a color to a static array color entry.
   */
  void AddColor (int i, float r, float g, float b);

  /**
   * Add a color to a dynamic array color entry.
   */
  void AddDynamicColor (int i, float r, float g, float b);

  /**
   * Set a color in the dynamic array.
   */
  void SetDynamicColor (int i, float r, float g, float b);

  /**
   * Reset a dynamic color to the static values.
   */
  void ResetDynamicColor (int i);

  /**
   * Set a color in the dynamic array.
   */
  void SetDynamicColor (int i, csColor& c) { SetDynamicColor (i, c.red, c.green, c.blue); }

  /**
   * Set a color in the static array.
   */
  void SetColor (int i, float r, float g, float b);

  /**
   * Set a color in the static array.
   */
  void SetColor (int i, csColor& c) { SetColor (i, c.red, c.green, c.blue); }
};

/**
 * This is our main 3D polygon class. Polygons are used to construct the
 * outer hull of sectors and the faces of 3D things.
 * Polygons can be transformed in 3D (usually they are transformed so
 * that the camera position is at (0,0,0) and the Z-axis is forward).
 * Polygons cannot be transformed in 2D. That's what csPolygon2D is for.
 * It is possible to convert a csPolygon3D to a csPolygon2D though, at
 * which point processing continues with the csPolygon2D object.
 *<p>
 * Polygons have a texture and lie on a plane. The plane does not
 * define the orientation of the polygon but is derived from it. The plane
 * does define how the texture is scaled and translated accross the surface
 * of the polygon (in case we are talking about lightmapped polygons, gouraud
 * shaded polygons have u,v coordinates at every vertex).
 * Several planes can be shared for different polygons. As a result of this
 * their textures will be correctly aligned.
 *<p>
 * If a polygon is part of a sector it can be a portal to another sector.
 * A portal-polygon is a see-through polygon that defines a view to another
 * sector. Normally the texture for a portal-polygon is not drawn unless
 * the texture is filtered in which case it is drawn on top of the other
 * sector.
 */
class csPolygon3D : public iBase, public csObject, public csPolygonInt
{
  friend class Dumper;
  friend class csPolyTexture;

private:
  /// A table of indices into the vertices of the parent csPolygonSet (container).
  csPolyIndexed vertices;

  /**
   * The following two fields are somewhat related. 'poly_set' is the real
   * parent (container) of this polygon. It is either a 3D thing or a sector.
   * 'sector' is always a sector. If this polygon belongs to a sector ('poly_set'
   * is a sector) then 'sector' will have the same value as 'poly_set'. If
   * this polygon belongs to a thing then 'sector' will be set to the sector
   * containing the thing.<p>
   *
   * @@@ Note! We have to reconsider this. If a thing moves to another sector
   * we would have to update this variable for all polygons of the thing.
   */
  csPolygonSet* poly_set;
  /// The csSector that this polygon is in.
  csSector* sector;

  /**
   * If not-null, this polygon is a portal.
   */
  csPortal* portal;

  /**
   * The PolygonPlane for this polygon.
   */
  csPolyPlane* plane;

  /**
   * The material, this contains the texturehandle, the 3D engine texture 
   * reference (contains the handle as returned
   * by iTextureManager interface). And more.
   */
  csMaterial* material;

  /**
   * General lighting information for this polygon.
   */
  csPolygonLightInfo light_info;

  /**
   * Texture type specific information for this polygon. Can be either
   * csLightMapped or csGouraudShaded.
   */
  csPolygonTextureType *txt_info;

  /**
   * The original polygon. This is useful when a BSP tree
   * has split a polygon. In that case, orig_poly will indicate the
   * original polygon that this polygon was split from.
   * If not split then this will be NULL.
   */
  csPolygon3D *orig_poly;

  /**
   * The texture share list. All polygons that share a single texture
   * are linked using this field. For example, if some polygon was split
   * multiple times by the BSP algorithm, all polygons that results
   * from that split are linked through this field. The start of list is
   * in "orig_poly" polygon.
   */
  csPolygon3D *txt_share_list;

  /**
   * Visibility number. If equal to csOctreeNode::pvs_cur_vis_nr then
   * this object is visible.
   */
  ULong pvs_vis_nr;

  /**
   * Precompute the plane normal. Normally this is done automatically by
   * set_texture_space but if needed you can call this function again when
   * something has changed.
   */
  void ComputeNormal ();

  /**
   * Return twice the signed area of the polygon in world space coordinates using
   * the yz, zx, and xy components. In effect this calculates the (P,Q,R) or the
   * plane normal of the polygon.
   */
  void PlaneNormal (float* yz, float* zx, float* xy);

#ifdef DO_HW_UVZ
  /// Precompute the (u,v) values for all vertices of the polygon
  void SetupHWUV();
#endif
  
  /**
   * Same as CalculateLighting but called before light view destruction
   * through callbacks and csPolyTexture::ProcessDelayedLightmaps ().
   * Called only for lightmapped polygons with shared lightmap.
   */
  void CalculateDelayedLighting (csFrustumView *lview);

public:
  /// Set of flags
  csFlags flags;

public:
  /// Option variable: cache lightmaps.
  static bool do_cache_lightmaps;

#ifdef DO_HW_UVZ
  csVector3 *uvz;
  bool isClipped;
#endif

  /**
   * Construct a new polygon with the given material. 
   */
  csPolygon3D (csMaterial *mat);

  /**
   * Construct a new polygon with the given texture.
   * This will create a new material specifically for this polygon.
   * If the texture is NULL the polygon is untextured (or you can
   * set the texture later using the SetTexture method).
   */
  csPolygon3D (csTextureHandle* texture);

  /**
   * Construct a new polygon and copy from the given polygon.
   * Note! Several fields will reflect that a copy was made!
   * New polytextures will be allocated. This is mainly used when
   * a BSP tree splits a polygon.
   */
  csPolygon3D (csPolygon3D& poly);

  /**
   * Delete everything related to this polygon. Less is
   * deleted if this polygon is a copy of another one (because
   * some stuff is shared).
   */
  virtual ~csPolygon3D ();

  /**
   * Set type of texturing to use for this polygon (one of
   * the POLYTXT_??? flags). POLYTXT_LIGHTMAP is default.
   * This function is guaranteed not to do anything if the type is
   * already correct.
   */
  void SetTextureType (int type);

  /**
   * Short-hand to get the texture type from the csPolygonTextureType
   * structure.
   */
  int GetTextureType () { return txt_info->GetTextureType (); }

  /**
   * Get the general texture type information structure.
   */
  csPolygonTextureType* GetTextureTypeInfo () { return txt_info; }

  /**
   * This is a conveniance function to get the lightmap information
   * structure. If this polygon is gouraud shaded it will return NULL.
   */
  csLightMapped* GetLightMapInfo ()
  {
    if (txt_info && txt_info->GetTextureType () == POLYTXT_LIGHTMAP)
      return (csLightMapped*)txt_info;
    else
      return NULL;
  }

  /**
   * This is a conveniance function to get the gouraud information
   * structure. If this polygon is lightmapped it will return NULL.
   */
  csGouraudShaded* GetGouraudInfo ()
  {
    if (txt_info && txt_info->GetTextureType () == POLYTXT_GOURAUD)
      return (csGouraudShaded*)txt_info;
    else
      return NULL;
  }

  /**
   * Clear the polygon (remove all vertices).
   */
  void Reset ();

  /**
   * Add a vertex from the container (polygonset) to the polygon.
   */
  int AddVertex (int v);

  /**
   * Add a vertex to the polygon (and containing csPolygonSet).
   * Note that it will not check if the vertex is already there.
   * After adding all vertices/polygons you should call
   * csPolygonSet::CompressVertices() to safe space and gain
   * efficiency.
   */
  int AddVertex (const csVector3& v);

  /**
   * Add a vertex to the polygon (and containing csPolygonSet).
   * Note that it will not check if the vertex is already there.
   * After adding all vertices/polygons you should call
   * csPolygonSet::CompressVertices() to safe space and gain
   * efficiency.
   */
  int AddVertex (float x, float y, float z);

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

  /// Get the flat color for this polygon.
  csColor& GetFlatColor () { return light_info.flat_color; }

  /// Set the flat color for this polygon.
  void SetFlatColor (float r, float g, float b)
  {
    light_info.flat_color.red = r;
    light_info.flat_color.green = g;
    light_info.flat_color.blue = b;
    flags.Set (CS_POLY_FLATSHADING, CS_POLY_FLATSHADING);
  }

  /// Set the flat color for this polygon.
  void SetFlatColor (csColor& fc)
  { light_info.flat_color = fc; flags.Set (CS_POLY_FLATSHADING, CS_POLY_FLATSHADING); }

  /// Reset flat color (i.e. use texturing again).
  void ResetFlatColor () { flags.Set (CS_POLY_FLATSHADING, 0); }

  /**
   * If the polygon is a portal this will set the sector
   * that this portal points to. If this polygon has no portal
   * a Crystal Space portal will be created.
   */
  void SetCSPortal (csSector* sector);

  /**
   * Set a pre-created portal on this polygon.
   */
  void SetPortal (csPortal* prt);

  /**
   * Get the portal structure (if there is one).
   */
  csPortal* GetPortal () { return portal; }

  /**
   * Set the polygonset (container) that this polygon belongs to.
   */
  void SetParent (csPolygonSet* poly_set) { csPolygon3D::poly_set = poly_set; }

  /**
   * Get the polygonset (container) that this polygons belongs to.
   */
  csPolygonSet* GetParent () { return poly_set; }

  /**
   * Get the sector that this polygon belongs to.
   */
  csSector* GetSector () { return sector; }

  /**
   * Set the sector that this polygon belongs to.
   */
  void SetSector (csSector* sector) { csPolygon3D::sector = sector; }

  /**
   * Return the plane of this polygon. This function returns
   * a 3D engine type csPolyPlane which encapsulates object,
   * world, and camera space planes as well as the texture
   * transformation.
   */
  csPolyPlane* GetPlane () { return plane; }

  /**
   * Return the world-space plane of this polygon.
   */
  csPlane3* GetPolyPlane () { return &plane->GetWorldPlane (); }

  /**
   * Get the vertices.
   */
  csPolyIndexed& GetVertices () { return vertices; }

  /**
   * Get number of vertices (required for csPolygonInt).
   */
  virtual int GetNumVertices () { return vertices.GetNumVertices (); }

  /**
   * Get vertex index table (required for csPolygonInt).
   */
  virtual int* GetVertexIndices () { return vertices.GetVertexIndices (); }

  /**
   * Set the warping transformation for the portal.
   * If there is no portal this function does nothing.
   */
  void SetWarp (const csTransform& t) { if (portal) portal->SetWarp (t); }

  /**
   * Set the warping transformation for the portal.
   * If there is no portal this function does nothing.
   */
  void SetWarp (const csMatrix3& m_w, const csVector3& v_w_before,
  	const csVector3& v_w_after)
  {
    if (portal) portal->SetWarp (m_w, v_w_before, v_w_after);
  }

  /**
   * 'idx' is a local index into the vertices table of the polygon.
   * This index is translated to the index in the parent container and
   * a reference to the vertex in world-space is returned.
   */
  csVector3& Vwor (int idx)
  { return poly_set->Vwor (vertices.GetVertexIndices ()[idx]); }

  /**
   * 'idx' is a local index into the vertices table of the polygon.
   * This index is translated to the index in the parent container and
   * a reference to the vertex in object-space is returned.
   */
  csVector3& Vobj (int idx)
  { return poly_set->Vobj (vertices.GetVertexIndices ()[idx]); }

  /**
   * 'idx' is a local index into the vertices table of the polygon.
   * This index is translated to the index in the parent container and
   * a reference to the vertex in camera-space is returned.
   */
  csVector3& Vcam (int idx)
  { return poly_set->Vcam (vertices.GetVertexIndices ()[idx]); }

  /**
   * Before calling a series of Vcam() you should call
   * CamUpdate() first to make sure that the camera vertex set
   * is up-to-date.
   */
  void CamUpdate () { poly_set->CamUpdate (); }

  /**
   * Set the texture for this polygon.
   * This texture handle will only be used as soon as 'Finish()'
   * is called. So you can safely wait preparing the textures
   * until finally csWorld::Prepare() is called (which in the end
   * calls Finish() for every polygon).
   */
  void SetTexture (csTextureHandle* texture);

  /**
   * Get the texture.
   */
  csTextureHandle* GetCsTextureHandle () {return material->GetTextureHandle();}

  /**
   * Set the material
   */
  void SetMaterial (csMaterial *mat);

  /**
   * Get the material
   */
  inline csMaterial* GetMaterial () const { return material; }

  /**
   * Return true if this polygon or the texture it uses is transparent.
   */
  bool IsTransparent ();

  /// Return true if node is visible according to PVS.
  bool IsVisible () { return pvs_vis_nr == csOctreeNode::pvs_cur_vis_nr; }

  /// Mark visible (used by PVS).
  void MarkVisible () { pvs_vis_nr = csOctreeNode::pvs_cur_vis_nr; }

  /// Calculates the area of the polygon.
  float GetArea();

  /**
   * Get the cosinus factor. This factor is used
   * for lighting.
   */
  float GetCosinusFactor () { return light_info.cosinus_factor; }

  /**
   * Set the cosinus factor. This factor is used
   * for lighting.
   */
  void SetCosinusFactor (float f) { light_info.cosinus_factor = f; }

  /**
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
  void SetTextureSpace (csPolygon3D* copy_from);

  /**
   * This version takes the given plane. Using this function you
   * can use the same plane for several polygons. This polygon
   * is not responsible for cleaning this plane.
   */
  void SetTextureSpace (csPolyTxtPlane* txt_pl);

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
  void SetTextureSpace (csVector3& v_orig, csVector3& v1, float len1);

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
  void SetTextureSpace (const csMatrix3& tx_matrix, const csVector3& tx_vector);

  /**
   * Prepare the lightmaps for use.
   * This function also converts the lightmaps to the correct
   * format required by the 3D driver. This function does NOT
   * create the first lightmap. This is done by the precalculated
   * lighting process (using CalculateLighting()).
   */
  void CreateLightMaps (iGraphics3D* g3d);

  /// Return the pointer to the original polygon (before any BSP splits).
  csPolygonInt* GetUnsplitPolygon () { return orig_poly; }

  /**
   * Return the pointer to the original polygon (before any BSP splits).
   * If polygon was not split this will return current poly.
   */
  csPolygon3D* GetBasePolygon ()
  { return orig_poly ? (csPolygon3D*)orig_poly : this; }

  /**
   * A dynamic light has changed (this can be either an
   * intensity/color change of a pseudo-dynamic light or else
   * a real dynamic light change).
   */
  void MakeDirtyDynamicLights ();

  /// Return true if polygon is dirty for dynamic lights.
  bool IsDirty () { return light_info.dyn_dirty; }

  /// Make clean again.
  void MakeCleanDynamicLights () { light_info.dyn_dirty = false; }

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
  csLightPatch* GetLightpatches () { return light_info.lightpatches; }

  /**
   * Clip a polygon against a frustum in camera space.
   * The frustum is defined with (0,0,0) as one of the
   * points of the plane.<p>
   *
   * If the frustum == NULL then it is considered infinite.
   * This function returns false (and does not allocate the
   * clipped polygon) if the polygon is completely clipped
   * away. Otherwise it will allocated a new array
   * of csVector3 in dest.
   */
  bool ClipPoly (csVector3* frustum, int num_frustum,
	bool mirror, csVector3** dest, int* num_dest);

  /**
   * Clip a polygon against a plane (in camera space).
   * The plane is defined as going through v1, v2, and (0,0,0).
   * The 'verts' array is modified and 'num' is also modified if needed.
   */
  void ClipPolyPlane (csVector3* verts, int* num, bool mirror,
  	csVector3& v1, csVector3& v2);

  /**
   * See if a polygon is visible from the given center (in world space
   * using backface culling) and then clip against the frustum and return
   * a new frustum (in camera space with the frustum center at (0,0,0)).
   * This function returns false if the polygon is completely clipped away
   * or if it is not visible. No new_frustum will be allocated in that case.
   * If 'mirror' is true the given frustum is mirrored (vertices in
   * anti-clockwise order). This function correctly handles that case and
   * will return a new frustum that is also mirrored.
   */
  bool ClipFrustum (csVector3& center, csVector3* frustum,
  	int num_frustum, bool mirror,
  	csVector3** new_frustum, int* new_num_frustum);

  /**
   * Initialize the lightmaps for this polygon.
   * Should be called before calling CalculateLighting() and before
   * calling CacheLightMaps().<p>
   *
   * This function will try to read the lightmap from the
   * cache in the level archive.
   * If do_cache == false this function will not try to read the lightmap
   * from the cache.
   * Index is the index of this polygon into the containing object. This
   * is used to identify the lightmap data on disk.
   */
  void InitLightMaps (csPolygonSet* owner, bool do_cache, int index);

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
  void FillLightMap (csFrustumView& lview);

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
   */
  void UpdateVertexLighting (csLight* light, const csColor& lcol,
  	bool dynamic, bool reset);

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
  bool MarkRelevantShadowFrustums (csFrustumView& lview, csPlane3& plane);

  /**
   * Same as above but takes polygon plane as 'plane' argument.
   */
  bool MarkRelevantShadowFrustums (csFrustumView& lview);

  /**
   * Check visibility of this polygon with the given csFrustumView
   * and fill the lightmap if needed (this function calls FillLightMap ()).
   * This function will also traverse through a portal if so needed.
   */
  void CalculateLighting (csFrustumView* lview);

  /**
   * Call after calling InitLightMaps and CalculateLighting to cache
   * the calculated lightmap to the level archive. This function does
   * nothing if the cached lightmap was already up-to-date.
   */
  void CacheLightMaps (csPolygonSet* owner, int index);

  /**
   * Transform the plane of this polygon from object space to world space.
   * This is mainly used for things since sectors currently have
   * identical object and world space coordinates.
   */
  void ObjectToWorld (const csReversibleTransform& t);

  /**
   * Clip this camera space polygon to the given plane. 'plane' can be NULL
   * in which case no clipping happens.<p>
   *
   * If this function returns false then the polygon is not visible (backface
   * culling, no visible vertices, ...) and 'verts' will be NULL. Otherwise
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
  bool DoPerspective (const csTransform& trans, csVector3* source,
  	int num_verts, csPolygon2D* dest, csVector2* orig_triangle,
	bool mirror);

  /**
   * Classify this polygon with regards to a plane (in world space). If this poly
   * is on same plane it returns POL_SAME_PLANE. If this poly is
   * completely in front of the given plane it returnes POL_FRONT. If this poly
   * is completely back of the given plane it returnes POL_BACK. Otherwise it
   * returns POL_SPLIT_NEEDED.
   */
  int Classify (const csPlane3& pl);

  /// Same as Classify() but for X plane only.
  int ClassifyX (float x);

  /// Same as Classify() but for Y plane only.
  int ClassifyY (float y);

  /// Same as Classify() but for Z plane only.
  int ClassifyZ (float z);

  /**
   * Split this polygon with the given plane (A,B,C,D) and return the
   * two resulting new polygons in 'front' and 'back'. The new polygons will
   * mimic the behaviour of the parent polygon as good as possible.
   * This function is mainly used by the BSP splitter.
   */
  void SplitWithPlane (csPolygonInt** front, csPolygonInt** back,
  	const csPlane3& plane);

  /**
   * Check if this polygon (partially) overlaps the other polygon
   * from some viewpoint in space.
   */
  bool Overlaps (csPolygonInt* overlapped);

  /**
   * Return 1 to indicate to the BSP tree routines that
   * this is a csPolygon3D.
   */
  int GetType () { return 1; }

  /**
   * Intersect world-space segment with this polygon. Return
   * true if it intersects and the intersection point in world coordinates.
   */
  bool IntersectSegment (const csVector3& start, const csVector3& end,
                          csVector3& isect, float* pr = NULL);

  /**
   * Intersect world-space ray with this polygon. This function
   * is similar to intersect_segment except that it doesn't keep the lenght
   * of the ray in account. It just tests if the ray intersects with the
   * interior of the polygon. Note that this function also does back-face
   * culling.
   */
  bool IntersectRay (const csVector3& start, const csVector3& end);

  /**
   * Intersect world-space ray with this polygon. This function
   * is similar to intersect_segment except that it doesn't keep the lenght
   * of the ray in account. It just tests if the ray intersects with the
   * interior of the polygon. Note that this function doesn't do
   * back-face culling.
   */
  bool IntersectRayNoBackFace (const csVector3& start, const csVector3& end);

  /**
   * Intersect ray with the plane of this polygon and returns the intersection
   * point. This function does not test if the intersection is inside the
   * polygon. It just returns the intersection with the plane (in or out).
   * This function returns false if the ray is parallel with the plane
   * (i.e. there is no intersection).
   */
  bool IntersectRayPlane (const csVector3& start, const csVector3& end,
  	csVector3& isect);

  /**
   * This is a given point is on (or very nearly on) this polygon.
   */
  bool PointOnPolygon (const csVector3& v);

  /// Get the alpha transparency value for this polygon.
  virtual int GetAlpha ()
  { return GetLightMapInfo () ? GetLightMapInfo ()->GetAlpha () : 0; }

  /**
   * Set the alpha transparency value for this polygon (only if
   * it is a portal).
   * Not all renderers support all possible values. 0, 25, 50,
   * 75, and 100 will always work but other values may give
   * only the closest possible to one of the above.
   */
  virtual void SetAlpha (int iAlpha)
  { if (GetLightMapInfo ()) GetLightMapInfo ()->SetAlpha (iAlpha); }

  /// Get the texture handle for the texture manager.
  virtual iTextureHandle *GetTextureHandle ();
  /// Get the handle to the polygon texture object
  virtual iPolygonTexture *GetTexture ()
  {
    csLightMapped *lm = GetLightMapInfo ();
    return lm ? lm->GetPolyTex () : NULL;
  }

  /// Get next polygon in texture share list
  csPolygon3D *GetNextShare ()
  { return txt_share_list; }
  /// Set next polygon in texture share list
  void SetNextShare (csPolygon3D *next)
  { txt_share_list = next; }

  CSOBJTYPE;
  DECLARE_IBASE;

  //-------------------- iPolygon3D interface implementation -------------------

  struct Poly3D : public iPolygon3D
  {
    DECLARE_EMBEDDED_IBASE (csPolygon3D);

    /// Get polygon name
    virtual const char *GetName () const
    { return scfParent->GetName (); }
    /// Set polygon name
    virtual void SetName (const char *iName)
    { scfParent->SetName (iName); }

    /// Get the polygonset (container) that this polygons belongs to.
    virtual iPolygonSet *GetContainer ()
    { return (iPolygonSet *)scfParent->GetParent (); }
    /// Get the lightmap associated with this polygon
    virtual iLightMap *GetLightMap ()
    {
      csLightMapped *lm = scfParent->GetLightMapInfo ();
      return lm ? lm->GetLightMap () : NULL;
    }
    /// Get the handle to the polygon texture object
    virtual iPolygonTexture *GetTexture ()
    { return scfParent->GetTexture(); }
    /// Get the texture handle for the texture manager.
    virtual iTextureHandle *GetTextureHandle ()
    { return scfParent->GetTextureHandle (); }

    /// Query number of vertices in this polygon
    virtual int GetVertexCount ()
    { return scfParent->vertices.GetNumVertices (); }
    /// Get the given polygon vertex coordinates in object space
    virtual csVector3 &GetVertex (int idx)
    { return GetContainer ()->GetVertex (scfParent->vertices.GetVertex (idx)); }
    /// Get the given polygon vertex coordinates in world space
    virtual csVector3 &GetVertexW (int idx)
    { return GetContainer ()->GetVertexW (scfParent->vertices.GetVertex (idx)); }
    /// Get the given polygon vertex coordinates in camera space
    virtual csVector3 &GetVertexC (int idx)
    { return GetContainer ()->GetVertexC (scfParent->vertices.GetVertex (idx)); }
    /// Create a polygon vertex given his index in parent polygon set
    virtual int CreateVertex (int idx)
    { return scfParent->AddVertex (idx); }
    /// Create a polygon vertex and add it to parent object
    virtual int CreateVertex (const csVector3 &iVertex)
    { return scfParent->AddVertex (iVertex); }

    /// Get the alpha transparency value for this polygon.
    virtual int GetAlpha ()
    { return scfParent->GetAlpha (); }
    /// Set the alpha transparency value for this polygon.
    virtual void SetAlpha (int iAlpha)
    { scfParent->SetAlpha (iAlpha); }

    /// Create a private polygon texture mapping plane
    virtual void CreatePlane (const csVector3 &iOrigin, const csMatrix3 &iMatrix);
    /// Set polygon texture mapping plane
    virtual bool SetPlane (const char *iName);
  } scfiPolygon3D;
  friend struct Poly3D;
};

#endif // __CS_POLYGON_H__
