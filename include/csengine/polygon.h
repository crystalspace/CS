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
#include "csgeom/transfrm.h"
#include "csgeom/polyclip.h"
#include "csgeom/polyidx.h"
#include "csengine/polyint.h"
#include "csengine/polyplan.h"
#include "csengine/thing.h"
#include "csengine/portal.h"
#include "csengine/polytext.h"
#include "csengine/octree.h"
#include "csengine/material.h"
#include "iengine/sector.h"
#include "imesh/thing/polygon.h"
#include "imesh/thing/ptextype.h"

class csSector;
class csFrustumView;
class csFrustumContext;
class csMaterialWrapper;
class csPolyPlane;
class csPolyTxtPlane;
class csPolygon2D;
class csPolygon3D;
class csLightMap;
class csLightPatch;
class csPolyTexture;
class csThing;
struct iLight;
struct iGraphics2D;
struct iGraphics3D;

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
   * List of light patches for this polygon.
   */
  csLightPatch *lightpatches;

  /**
   * True if we have a dirty polygon due to dynamic lighting.
   */
  bool dyn_dirty;
};

/*---------------------------------------------------------------------------*/

/**
 * Kind of texturing that is used for a 3D polygon.
 * This is the base class with subclasses depending on the kind
 * of texturing that is used for a polygon. Also this class contains
 * all the information required for POLYTXT_NONE texture type.
 */
class csPolyTexType : public iPolyTexType
{
  friend class csPolygon3D;

protected:
  /**
   * 0 is no alpha, 25 is 25% see through and 75% texture and so on.
   * Valid values are from 0 to 100; some renderers in some modes will
   * approximate it (and some values like 25/50/75 are optimized for speed).
   * Note that alpha is in range 0..255, 0 for 0% and 255 for 100%.
   */
  ushort Alpha;

  /**
   * MixMode to use for drawing this polygon (plus alpha value
   * which is stored separately). The GetMixMode() function will
   * overlap both variables to get one compound value.
   */
  uint MixMode;

  /// Common constructor for derived classes
  csPolyTexType ();
  /// Destructor is virtual to be able to delete derived objects
  virtual ~csPolyTexType ();

public:
  /// Return a type for the kind of texturing used.
  virtual int GetTextureType () { return POLYTXT_NONE; }

  /// Get the alpha value for this polygon
  int GetAlpha () { return Alpha; }
  /// Set the alpha value for this polygon
  void SetAlpha (int a) { Alpha = a; }

  /// Sets the mode that is used for DrawPolygonFX.
  virtual void SetMixMode (UInt m) { MixMode = m & ~CS_FX_MASK_ALPHA; }

  /// Gets the mode that is used for DrawPolygonFX.
  virtual UInt GetMixMode () { return (MixMode | Alpha); }

  SCF_DECLARE_IBASE;
};

/**
 * This structure contains all required information for
 * flat-shaded (do not mix with flat-colored!) texture mapped
 * (or flat-shaded) polygons.
 */
class csPolyTexFlat : public csPolyTexType
{
  friend class csPolygon3D;

private:
  /**
   * The following array specifies the u,v coordinates for every vertex
   * of the polygon.
   */
  csVector2* uv_coords;

protected:
  /// Constructor.
  csPolyTexFlat () : csPolyTexType ()
  { SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPolyTexFlat); uv_coords = NULL; }

  /// Destructor.
  virtual ~csPolyTexFlat ();

public:
  /// Return a type for the kind of texturing used.
  virtual int GetTextureType () { return POLYTXT_FLAT; }

  /**
   * Setup this lighting structure with the right number of vertices,
   * taken from parent object. The contents of U/V array are not destroyed,
   * if it was previously allocated.
   */
  void Setup (csPolygon3D *iParent);

  /**
   * Setup this lighting structure with the right number of vertices,
   * taken from parent object. The contents of U/V array are not destroyed,
   * if it was previously allocated.
   */
  virtual void Setup (iPolygon3D *iParent);

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
  virtual void SetUV (int i, float u, float v);

  /**
   * Clear all (u,v) coordinates.
   */
  virtual void ClearUV ();

  /// Get the pointer to the vertex uv coordinates.
  virtual csVector2 *GetUVCoords () { return uv_coords; }

  SCF_DECLARE_IBASE_EXT (csPolyTexType);

  struct eiPolyTexFlat : public iPolyTexFlat
  {
    SCF_DECLARE_EMBEDDED_IBASE(csPolyTexFlat);
    virtual void Setup (iPolygon3D *p) { scfParent->Setup(p); }
    virtual void SetUV (int i, float u, float v) { scfParent->SetUV(i,u,v); }
    virtual void ClearUV () { scfParent->ClearUV(); }
    virtual csVector2 *GetUVCoords () { return scfParent->GetUVCoords(); }
  } scfiPolyTexFlat;
};

/**
 * Structure containing information about texture mapping
 * and vertex colors for Gouraud-shaded polygons.
 */
class csPolyTexGouraud : public csPolyTexFlat
{
  friend class csPolygon3D;

private:
  /**
   * If uv_coords is given then this can be an optional array of vertex
   * colors. If this array is given then gouraud shading is used. Otherwise
   * flatshading.
   */
  csColor *colors;

  /**
   * This array contains the static colors. It is used in combination with
   * 'colors'.  'colors=static_colors+dynamic_lights'.
   */
  csColor *static_colors;

protected:
  /// Constructor.
  csPolyTexGouraud () : csPolyTexFlat ()
  { SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPolyTexGouraud); colors = static_colors = 0; }

  /// Destructor.
  virtual ~csPolyTexGouraud ();

public:
  /// Return a type for the kind of texturing used.
  virtual int GetTextureType () { return POLYTXT_GOURAUD; }

  /**
   * Setup this lighting structure with the rignt number of vertices,
   * taken from parent object. The contents of U/V array are not destroyed,
   * if it was previously allocated.
   */
  void Setup (csPolygon3D *iParent);

  /**
   * Setup this lighting structure with the rignt number of vertices,
   * taken from parent object. The contents of U/V array are not destroyed,
   * if it was previously allocated.
   */
  virtual void Setup (iPolygon3D *iParent);

  /**
   * Clear all color information.
   */
  virtual void ClearColors ();

  /// Get the pointer to the vertex color table.
  virtual csColor *GetColors () { return colors; }

  /// Get the pointer to the static vertex color table.
  virtual csColor *GetStaticColors () { return static_colors; }

  /**
   * Add a color to the static color array.
   */
  void AddColor (int i, float r, float g, float b);

  /**
   * Add a color to the dynamic color array.
   */
  void AddDynamicColor (int i, float r, float g, float b);

  /**
   * Set a color in the dynamic array.
   */
  void SetDynamicColor (int i, float r, float g, float b);

  /**
   * Reset a dynamic color to the static values.
   */
  virtual void ResetDynamicColor (int i);

  /**
   * Set a color in the dynamic array.
   */
  virtual void SetDynamicColor (int i, const csColor& c)
  { SetDynamicColor (i, c.red, c.green, c.blue); }

  /**
   * Set a color in the static array.
   */
  void SetColor (int i, float r, float g, float b);

  /**
   * Set a color in the static array.
   */
  virtual void SetColor (int i, const csColor& c)
  { SetColor (i, c.red, c.green, c.blue); }

  SCF_DECLARE_IBASE_EXT (csPolyTexFlat);

  struct eiPolyTexGouraud : public iPolyTexGouraud
  {
    SCF_DECLARE_EMBEDDED_IBASE(csPolyTexGouraud);
    virtual void Setup (iPolygon3D *p) { scfParent->Setup(p); }
    virtual void ClearColors () { scfParent->ClearColors(); }
    virtual csColor *GetColors () { return scfParent->GetColors(); }
    virtual csColor *GetStaticColors() { return scfParent->GetStaticColors(); }
    virtual void ResetDynamicColor (int i) { scfParent->ResetDynamicColor(i); }
    virtual void SetDynamicColor (int i, const csColor& c)
    { scfParent->SetDynamicColor(i,c); }
    virtual void SetColor(int i,const csColor& c) { scfParent->SetColor(i,c); }
  } scfiPolyTexGouraud;
};

/**
 * Structure containing all required information
 * for lightmapped polygons.
 */
class csPolyTexLightMap : public csPolyTexType
{
  friend class csPolygon3D;

private:
  /// The transformed texture for this polygon.
  csPolyTexture *tex;

  /**
   * The csPolyTxtPlane for this polygon.
   */
  csPolyTxtPlane *txt_plane;

  /**
   * This bool indicates if the lightmap is up-to-date (read from the
   * cache). If set to false the polygon still needs to be recalculated.
   */
  bool lightmap_up_to_date;

private:
  /// Constructor.
  csPolyTexLightMap ();

  /// Destructor.
  virtual ~csPolyTexLightMap ();

public:
  /// Setup for the given polygon and material.
  void Setup (csPolygon3D* poly3d, csMaterialWrapper* math);

  /// Return a type for the kind of texturing used.
  virtual int GetTextureType () { return POLYTXT_LIGHTMAP; }

  /// Get the polytexture (lighted texture)
  csPolyTexture* GetPolyTex ();

  /**
   * Return the texture plane of this polygon.
   */
  csPolyTxtPlane* GetTxtPlane () const { return txt_plane; }
  /**
   * Return the texture plane of this polygon.
   */
  virtual iPolyTxtPlane* GetPolyTxtPlane () const;

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

  SCF_DECLARE_IBASE_EXT (csPolyTexType);

  struct eiPolyTexLightMap : public iPolyTexLightMap
  {
    SCF_DECLARE_EMBEDDED_IBASE(csPolyTexLightMap);
    virtual iPolyTxtPlane* GetPolyTxtPlane () const
    { return scfParent->GetPolyTxtPlane(); }
  } scfiPolyTexLightMap;
};

/*---------------------------------------------------------------------------*/

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

/**
 * If this flag is set then this polygon has been split (BSP tree
 * or other reason). Depending on the engine mode this polygon will
 * not be used anymore for rendering.
 */
#define CS_POLY_SPLIT		0x20000000

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
class csPolygon3D : public csPolygonInt, public csObject
// NOTE: DO NOT MOVE csPolygonInt FROM FIRST PLACE! THERE ARE LOTS OF PLACES
// WHERE CODE BLATANTLY SWITCHES BETWEEN csPolygon3D AND csPolygonInt TYPES
// WITHOUT GIVING TO THE C++ COMPILER EVEN A CHANCE TO ADJUST THE POINTER!!!
{
  friend class csPolyTexture;

private:
  /// ID for this polygon relative to the parent (will be >0).
  unsigned long polygon_id;

  /*
   * A table of indices into the vertices of the parent csThing
   * (container).
   */
  csPolyIndexed vertices;

  /**
   * The physical parent of this polygon.
   * Important note for CS developers. If the parent of a polygon
   * is changed in any way and this polygon has a portal then the
   * portal needs to be removed from the old thing and added to the
   * new thing (things keep a list of all polygons having a portal
   * on them).
   */
  csThing* thing;

  /**
   * If not-null, this polygon is a portal.
   * Important note for CS developers. If the portal is changed
   * in any way (deleted or set) then the parent thing has to
   * be notified (csThing::AddPortalPolygon() and
   * csThing::RemovePortalPolygon()) so that it can update its list
   * of portal polygons.
   */
  csPortal* portal;

  /**
   * The PolygonPlane for this polygon.
   */
  csPolyPlane* plane;

  /**
   * The material, this contains the texture handle,
   * the flat color (if no texture) and other parameters.
   */
  csMaterialWrapper* material;

  /**
   * General lighting information for this polygon.
   */
  csPolygonLightInfo light_info;

  /**
   * Texture type specific information for this polygon. Can be either
   * csPolyTexLightMap or csPolyTexGouraud.
   */
  csPolyTexType *txt_info;

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
   * Return twice the signed area of the polygon in world space coordinates
   * using the yz, zx, and xy components.  In effect this calculates the
   * (P,Q,R) or the plane normal of the polygon.
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
  void CalculateDelayedLighting (csFrustumView *lview, csFrustumContext* ctxt);

public:
  /// Set of flags
  csFlags flags;

public:
#ifdef DO_HW_UVZ
  csVector3 *uvz;
  bool isClipped;
#endif

  /**
   * Construct a new polygon with the given material. 
   */
  csPolygon3D (csMaterialWrapper *mat);

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

  /// Get the ID of this polygon relative to the parent (will be >0).
  unsigned long GetPolygonID ()
  {
    CS_ASSERT (polygon_id != 0);
    return polygon_id;
  }

  /**
   * Set type of texturing to use for this polygon (one of
   * the POLYTXT_??? flags). POLYTXT_LIGHTMAP is default.
   * This function is guaranteed not to do anything if the type is
   * already correct.
   */
  void SetTextureType (int type);

  /**
   * Get the polygon texture type (one of POLYTXT_XXX values).
   */
  int GetTextureType ()
  { return txt_info->GetTextureType (); }

  /**
   * Copy texture type settings from another polygon.
   * (this will not copy the actual material that is used, just the
   * information on how to apply that material to the polygon).
   */
  void CopyTextureType (iPolygon3D* other_polygon);

  /**
   * Get the general texture type information structure.
   */
  csPolyTexType *GetTextureTypeInfo () { return txt_info; }

  /**
   * This is a conveniance function to get the lightmap information
   * structure. If this polygon is not POLYTXT_LIGHTMAP it will return NULL.
   */
  csPolyTexLightMap *GetLightMapInfo ()
  {
    if (txt_info && txt_info->GetTextureType () == POLYTXT_LIGHTMAP)
      return (csPolyTexLightMap *)txt_info;
    else
      return NULL;
  }

  /**
   * This is a conveniance function to get the gouraud information
   * structure. If this polygon is not POLYTXT_GOURAUD it will return NULL.
   */
  csPolyTexGouraud *GetGouraudInfo ()
  {
    if (txt_info && txt_info->GetTextureType () == POLYTXT_GOURAUD)
      return (csPolyTexGouraud*)txt_info;
    else
      return NULL;
  }

  /**
   * This is a conveniance function to get the flat shaded information
   * structure. If this polygon is not POLYTXT_FLAT or POLYTXT_GOURAUD
   * it will return NULL, GOURAUD is derived from the FLAT structure.
   */
  csPolyTexFlat *GetFlatInfo ()
  {
    if (txt_info
     && (txt_info->GetTextureType () == POLYTXT_FLAT
      || txt_info->GetTextureType () == POLYTXT_GOURAUD))
      return (csPolyTexFlat*)txt_info;
    else
      return NULL;
  }

  /**
   * This is a conveniance function to get the "type" texturing type
   * information structure. It returns NULL only if the polygon texture
   * type is lightmapped, because all other texturing types are subclassed
   * from NONE.
   */
  csPolyTexType *GetNoTexInfo ()
  {
    if (txt_info && txt_info->GetTextureType () != POLYTXT_LIGHTMAP)
      return (csPolyTexType *)txt_info;
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
   * If the polygon is a portal this will set the sector
   * that this portal points to. If this polygon has no portal
   * one will be created.
   * If 'null' is true and sector == 'NULL' then a NULL portal
   * is created.
   */
  void SetCSPortal (csSector* sector, bool null = false);

  /**
   * Set a pre-created portal on this polygon.
   */
  void SetPortal (csPortal* prt);

  /**
   * Get the portal structure (if there is one).
   */
  csPortal* GetPortal () { return portal; }

  /**
   * Set the thing that this polygon belongs to.
   */
  void SetParent (csThing* thing);

  /**
   * Get the polygonset (container) that this polygons belongs to.
   */
  csThing* GetParent () { return thing; }

  /**
   * Return the plane of this polygon.  This function returns a 3D engine type
   * csPolyPlane which encapsulates object, world, and camera space planes as
   * well as the texture transformation.
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
  virtual int GetVertexCount () { return vertices.GetVertexCount (); }

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
  { return thing->Vwor (vertices.GetVertexIndices ()[idx]); }

  /**
   * 'idx' is a local index into the vertices table of the polygon.
   * This index is translated to the index in the parent container and
   * a reference to the vertex in object-space is returned.
   */
  csVector3& Vobj (int idx)
  { return thing->Vobj (vertices.GetVertexIndices ()[idx]); }

  /**
   * 'idx' is a local index into the vertices table of the polygon.
   * This index is translated to the index in the parent container and
   * a reference to the vertex in camera-space is returned.
   */
  csVector3& Vcam (int idx)
  { return thing->Vcam (vertices.GetVertexIndices ()[idx]); }

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
   * Set the material for this polygon.
   * This material handle will only be used as soon as 'Finish()'
   * is called. So you can safely wait preparing the materials
   * until finally csEngine::Prepare() is called (which in the end
   * calls Finish() for every polygon).
   */
  void SetMaterial (csMaterialWrapper* material);

  /**
   * Get the material.
   */
  csMaterialWrapper* GetMaterialWrapper () { return material; }

  /**
   * Return true if this polygon or the texture it uses is transparent.
   */
  bool IsTransparent ();

  /// Calculates the area of the polygon in object space.
  float GetArea ();

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
  void SetTextureSpace (csMatrix3 const&, csVector3 const&);

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
  void InitializeDefault ();

  /**
   * This function will try to read the lightmap from the
   * cache in the level archive.
   * If do_cache == false this function will not try to read the lightmap
   * from the cache.
   */
  bool ReadFromCache (int id);

  /**
   * Call after calling InitializeDefault() and CalculateLighting to cache
   * the calculated lightmap to the level archive. This function does
   * nothing if the cached lightmap was already up-to-date.
   */
  bool WriteToCache (int id);

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
  void FillLightMapDynamic (csFrustumView& lview);

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
  void FillLightMapStatic (csFrustumView* lview, bool vis);

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
  void UpdateVertexLighting (iLight* light, const csColor& lcol,
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
   * and update the light patches if needed.
   * This function will also traverse through a portal if so needed.
   * This version is for dynamic lighting.
   */
  void CalculateLightingDynamic (csFrustumView* lview);

  /**
   * Check visibility of this polygon with the given csFrustumView
   * and fill the lightmap if needed (this function calls FillLightMap ()).
   * This function will also traverse through a portal if so needed.
   * If 'vis' == false this means that the lighting system already discovered
   * that the polygon is totally shadowed.
   * This version is for static lighting.
   */
  void CalculateLightingStatic (csFrustumView* lview, bool vis);

  /**
   * Transform the plane of this polygon from object space to world space.
   * 'vt' is a vertex of this polygon in world space.
   */
  void ObjectToWorld (const csReversibleTransform& t, const csVector3& vwor);

  /**
   * Hard transform the plane of this polygon and also the
   * portal and lightmap info. This is similar to ObjectToWorld
   * but it does a hard transform of the object space planes
   * instead of keeping a transformation.
   */
  void HardTransform (const csReversibleTransform& t);

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
   * Classify this polygon with regards to a plane (in object space).  If this
   * poly is on same plane it returns POL_SAME_PLANE.  If this poly is
   * completely in front of the given plane it returnes POL_FRONT.  If this
   * poly is completely back of the given plane it returnes POL_BACK.
   * Otherwise it returns POL_SPLIT_NEEDED.
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
   * This function is mainly used by the BSP splitter. Note that splitting
   * happens in object space.
   */
  void SplitWithPlane (csPolygonInt** front, csPolygonInt** back,
  	const csPlane3& plane);

  /**
   * Check if this polygon (partially) overlaps the other polygon
   * from some viewpoint in space. This function works in object space.
   */
  bool Overlaps (csPolygonInt* overlapped);

  /**
   * Return 1 to indicate to the BSP tree routines that
   * this is a csPolygon3D.
   */
  int GetType () { return 1; }

  /**
   * Intersect object-space segment with this polygon. Return
   * true if it intersects and the intersection point in world coordinates.
   */
  bool IntersectSegment (const csVector3& start, const csVector3& end,
                          csVector3& isect, float* pr = NULL);

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
  virtual int GetAlpha ()
  { return txt_info->GetAlpha (); }

  /**
   * Set the alpha transparency value for this polygon (only if
   * it is a portal).
   * Not all renderers support all possible values. 0, 25, 50,
   * 75, and 100 will always work but other values may give
   * only the closest possible to one of the above.
   */
  virtual void SetAlpha (int iAlpha)
  { txt_info->SetAlpha (iAlpha); }

  /// Get the material handle for the texture manager.
  virtual iMaterialHandle *GetMaterialHandle ();
  /// Get the handle to the polygon texture object
  virtual iPolygonTexture *GetTexture ()
  {
    csPolyTexLightMap *lm = GetLightMapInfo ();
    return lm ? lm->GetPolyTex () : (iPolygonTexture*)NULL;
  }

  /// Get next polygon in texture share list
  csPolygon3D *GetNextShare ()
  { return txt_share_list; }
  /// Set next polygon in texture share list
  void SetNextShare (csPolygon3D *next)
  { txt_share_list = next; }

  SCF_DECLARE_IBASE_EXT (csObject);

  //------------------- iPolygon3D interface implementation -------------------

  struct eiPolygon3D : public iPolygon3D
  {
    SCF_DECLARE_EMBEDDED_IBASE (csPolygon3D);

    virtual csPolygon3D *GetPrivateObject () { return scfParent; }
    virtual iObject *QueryObject() {return scfParent;}
    virtual iThingState *GetParent ();
    virtual iLightMap *GetLightMap ()
    {
      csPolyTexLightMap *lm = scfParent->GetLightMapInfo ();
      return lm ? lm->GetLightMap () : (iLightMap*)NULL;
    }
    virtual iPolygonTexture *GetTexture () { return scfParent->GetTexture(); }
    virtual iMaterialHandle *GetMaterialHandle ()
    { return scfParent->GetMaterialHandle (); }
    virtual void SetMaterial (iMaterialWrapper* mat)
    {
      scfParent->SetMaterial (
        ((csMaterialWrapper::MaterialWrapper*)(mat))->scfParent);
    }
    virtual iMaterialWrapper* GetMaterial ()
    {
      // @@@ Not efficient. In future we need to store iMaterialWrapper
      // directly.
      if (!scfParent->GetMaterialWrapper ()) return NULL;
      else
      {
        iMaterialWrapper* wrap = SCF_QUERY_INTERFACE (
	  scfParent->GetMaterialWrapper (), iMaterialWrapper);
        wrap->DecRef ();
	return wrap;
      }
    }

    virtual int GetVertexCount ()
    { return scfParent->vertices.GetVertexCount (); }
    virtual int* GetVertexIndices ()
    { return scfParent->vertices.GetVertexIndices (); }
    virtual csVector3 &GetVertex (int idx)
    { return scfParent->Vobj (idx); }
    virtual csVector3 &GetVertexW (int idx)
    { return scfParent->Vwor (idx); }
    virtual csVector3 &GetVertexC (int idx)
    { return scfParent->Vcam (idx); }
    virtual int CreateVertex (int idx)
    { return scfParent->AddVertex (idx); }
    virtual int CreateVertex (const csVector3 &iVertex)
    { return scfParent->AddVertex (iVertex); }

    virtual int GetAlpha ()
    { return scfParent->GetAlpha (); }
    virtual void SetAlpha (int iAlpha)
    { scfParent->SetAlpha (iAlpha); }

    virtual void CreatePlane (const csVector3 &iOrigin,
      const csMatrix3 &iMatrix);
    virtual bool SetPlane (const char *iName);

    virtual csFlags& GetFlags ()
    { return scfParent->flags; }

    virtual void SetLightingMode (bool iGouraud)
    { scfParent->SetTextureType(iGouraud ? POLYTXT_GOURAUD:POLYTXT_LIGHTMAP); }

    virtual iPortal* CreateNullPortal ()
    {
      scfParent->SetCSPortal (NULL, true);
      return &(scfParent->GetPortal ()->scfiPortal);
    }
    virtual iPortal* CreatePortal (iSector *iTarget)
    {
      scfParent->SetCSPortal (iTarget->GetPrivateObject ());
      return &(scfParent->GetPortal ()->scfiPortal);
    }
    virtual iPortal* GetPortal ()
    {
      csPortal* prt = scfParent->GetPortal ();
      if (prt)
        return &(prt->scfiPortal);
      else
        return NULL;
    }

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
    virtual void SetTextureSpace (csMatrix3 const& m, csVector3 const& v)
    {
      scfParent->SetTextureSpace (m, v);
    }
    virtual void SetTextureSpace (iPolyTxtPlane* plane);

    virtual void SetTextureType (int type)
    {
      scfParent->SetTextureType (type);
    }
    virtual int GetTextureType ()
    {
      return scfParent->GetTextureType ();
    }
    virtual void CopyTextureType (iPolygon3D* other_polygon)
    {
      scfParent->CopyTextureType (other_polygon);
    }

    virtual const csPlane3& GetWorldPlane ()
    {
      return scfParent->plane->GetWorldPlane ();
    }
    virtual const csPlane3& GetObjectPlane ()
    {
      return scfParent->plane->GetObjectPlane ();
    }
    virtual const csPlane3& GetCameraPlane ()
    {
      return scfParent->plane->GetCameraPlane ();
    }
    virtual bool IsTransparent ()
    {
      return scfParent->IsTransparent ();
    }
    virtual float GetCosinusFactor ()
    {
      return scfParent->GetCosinusFactor ();
    }
    virtual void SetCosinusFactor (float cosfact)
    {
      scfParent->SetCosinusFactor (cosfact);
    }
    virtual iPolyTexType* GetPolyTexType ();
    virtual void UpdateVertexLighting (iLight* light, const csColor& lcol,
  	bool dynamic, bool reset)
    {
      scfParent->UpdateVertexLighting (light, lcol, dynamic, reset);
    }
    virtual unsigned long GetPolygonID ()
    {
      return scfParent->GetPolygonID ();
    }
  } scfiPolygon3D;
  friend struct eiPolygon3D;
};

#endif // __CS_POLYGON_H__
