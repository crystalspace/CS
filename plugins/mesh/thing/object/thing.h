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

#ifndef __CS_THING_H__
#define __CS_THING_H__

#include "csgeom/transfrm.h"
#include "parrays.h"
#include "csutil/csobject.h"
#include "csutil/nobjvec.h"
#include "csutil/util.h"
#include "csutil/flags.h"
#include "csutil/cscolor.h"
#include "csutil/csvector.h"
#include "csutil/garray.h"
#include "csutil/refarr.h"
#include "igeom/polymesh.h"
#include "igeom/objmodel.h"
#include "iengine/mesh.h"
#include "iengine/rview.h"
#include "iengine/shadcast.h"
#include "imesh/thing/thing.h"
#include "imesh/thing/polygon.h"
#include "imesh/object.h"
#include "imesh/lighting.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/config.h"
#include "lghtmap.h"

class csThing;
class csThingObjectType;
class csPolygon3D;
class csPolygon2D;
class csPoly2DPool;
class csLightPatchPool;
struct iShadowBlockList;
struct csVisObjInfo;
struct iGraphics3D;
struct iRenderView;
struct iMovable;
struct iFrustumView;
struct iMaterialWrapper;
struct iPolygonBuffer;

/**
 * This structure keeps the indices of the vertices which
 * define the bounding box of a csThing. It is calculated
 * by CreateBoundingBox() and stored with the csThing.<p>
 *
 * The following are six polygons which describe the faces
 * of the bounding box (clock-wise vertex order):<br>
 * <ul>
 *   <li>i2,i1,i3,i4
 *   <li>i6,i2,i4,i8
 *   <li>i5,i6,i8,i7
 *   <li>i1,i5,i7,i3
 *   <li>i1,i2,i6,i5
 *   <li>i7,i8,i4,i3
 * </ul>
 */
struct csThingBBox
{
  int i1, i2, i3, i4, i5, i6, i7, i8;
};

/**
 * This struct represents a line along which polygons are attached.
 * These polygons don't have to be adjacent but they usually are.
 * A thing has a list of these edges.
 */
struct csThingEdge
{
  int num_polygons;
  int* polygon_indices;
};

/**
 * A helper class for iPolygonMesh implementations used by csThing.
 */
class PolyMeshHelper : public iPolygonMesh
{
public:
  /**
   * Make a polygon mesh helper which will accept polygons which match
   * with the given flag (one of CS_POLY_COLLDET or CS_POLY_VISCULL).
   */
  PolyMeshHelper (uint32 flag) :
  	polygons (NULL), vertices (NULL), alloc_vertices (NULL),
	poly_flag (flag) { }
  virtual ~PolyMeshHelper () { Cleanup (); }

  void Setup ();
  void SetThing (csThing* thing) { PolyMeshHelper::thing = thing; }

  virtual int GetVertexCount ()
  {
    Setup ();
    return num_verts;
  }
  virtual csVector3* GetVertices ()
  {
    Setup ();
    return vertices;
  }
  virtual int GetPolygonCount ()
  {
    Setup ();
    return num_poly;
  }
  virtual csMeshedPolygon* GetPolygons ()
  {
    Setup ();
    return polygons;
  }
  virtual void Cleanup ();
  
  virtual bool IsDeformable () const { return false;  }
  virtual uint32 GetChangeNumber() const { return 0; }

private:
  csThing* thing;
  csMeshedPolygon* polygons;	// Array of polygons.
  csVector3* vertices;		// Array of vertices (points to alloc_vertices
  				// or else obj_verts of scfParent).
  csVector3* alloc_vertices;	// Optional copy of vertices from parent.
    				// This copy is used if there are curve
				// vertices.
  int num_poly;			// Total number of polygons.
  int curve_poly_start;		// Index of first polygon from curves.
    				// Polygons after this index need to be
				// deleted individually.
  int num_verts;		// Total number of vertices.
  uint32 poly_flag;		// Polygons must match with this flag.
};

/**
 * A Thing is a set of polygons. A thing can be used for the
 * outside of a sector or else to augment the sector with
 * features that are difficult to describe with convex sectors alone.<p>
 *
 * Every polygon in the set has a visible and an invisible face;
 * if the vertices of the polygon are ordered clockwise then the
 * polygon is visible. Using this feature it is possible to define
 * two kinds of things: in one kind the polygons are oriented
 * such that they are visible from within the hull. In other words,
 * the polygons form a sort of container or room where the camera
 * can be located. This kind of thing can be used for the outside
 * walls of a sector. In another kind the polygons are
 * oriented such that they are visible from the outside.<p>
 *
 * Things can also be used for volumetric fog. In that case
 * the Thing must be convex.
 */
class csThing : public csObject
{
  friend class PolyMeshHelper;

public:
  csThingObjectType* thing_type;

private:
  /// ID for this thing (will be >0).
  unsigned int thing_id;
  /// Last used ID.
  static int last_thing_id;
  /// Last used polygon ID.
  unsigned long last_polygon_id;
  /// Current visibility number.
  uint32 current_visnr;

  /// Number of vertices
  int num_vertices;
  /// Maximal number of vertices
  int max_vertices;

  /// Vertices in object space.
  csVector3* obj_verts;  

  /**
   * Vertices in world space.
   * It is possible that this array is equal to obj_verts. In that
   * case this is a thing that never moves.
   */
  csVector3* wor_verts;
  /// Vertices in camera space.
  csVector3* cam_verts;
  /// Number of vertices for cam_verts.
  int num_cam_verts;

  /// Normals in object space
  csVector3* obj_normals;

  /// Smooth flag
  bool smoothed;

  /// Camera number for which the above camera vertices are valid.
  long cameranr;

  /// Shape number.
  long shapenr;
  /// Object model listeners.
  csRefArray<iObjectModelListener> listeners;

  /**
   * This number indicates the last value of the movable number.
   * This thing can use this to check if the world space coordinates
   * need to be updated.
   */
  long movablenr;
  /**
   * The last movable used to move this object.
   */
  iMovable* cached_movable;
  /**
   * How is moving of this thing controlled? This is one of the
   * CS_THING_MOVE_... flags above.
   */
  int cfg_moving;

  /// The array of polygons forming the outside of the set
  csPolygonArray polygons;
  /**
   * A vector with polygons that contain portals (optimization).
   * csPolygon3D will make sure to call AddPortalPolygon() and
   * RemovePortalPolygon() when appropriate.
   */
  csVector portal_polygons;

#ifndef CS_USE_NEW_RENDERER
  /**
   * If we are a detail object then this will contain a reference to a
   * polygon buffer for the 3D renderer. This can be used by DrawPolygonMesh.
   */
  iPolygonBuffer* polybuf;
#endif // CS_USE_NEW_RENDERER
  /**
   * An array of materials that are used with the polygon buffer.
   */
  iMaterialWrapper** polybuf_materials;
  int polybuf_material_count;

  /// A vector with all edges in this thing.
  CS_DECLARE_GROWING_ARRAY (thing_edges, csThingEdge);
  /// If false then thing_edges is not valid and needs to be recalculated.
  bool thing_edges_valid;

  /// The array of curves forming the outside of the set
  csCurvesArray curves;
  /**
   * If true the transforms of the curves are set up (in case
   * CS_THING_MOVE_NEVER is used).
   */
  bool curves_transf_ok;

  /// Optional oriented bounding box.
  csThingBBox* bbox;

  /// Bounding box in object space.
  csBox3 obj_bbox;
  /**
   * Bounding box in world space.
   * This is a cache for GetBoundingBox(iMovable,csBox3) which
   * will recalculate this if the movable changes (by using movablenr).
   */
  csBox3 wor_bbox;
  /// Last movable number that was used for the bounding box in world space.
  long wor_bbox_movablenr;

  /// If true then the bounding box in object space is valid.
  bool obj_bbox_valid;

  /// Radius of object in object space.
  csVector3 obj_radius;
  /// Full radius of object in object space.
  float max_obj_radius;

#ifndef CS_USE_NEW_RENDERER
  /// Fog information.
  csFog fog;
#endif // CS_USE_NEW_RENDERER

  /// Dynamic ambient light assigned to this thing.
  csColor dynamic_ambient;
  /**
   * Version number of ambient light color determines whether recalc
   * is necessary in poly.
   */
  uint32 ambient_version;

  /// If convex, this holds the index to the center vertex.
  int center_idx;

  /// Pointer to the Thing Template which it derived from.
  csThing* ParentTemplate;
  /// Pointer to logical parent.
  iBase* logparent;

  /// If true then this thing has been prepared (Prepare() function).
  bool prepared;

  /**
   * Tesselation parameter:
   * Center of thing to determine distance from
   */
  csVector3 curves_center;
  /**
   * Scale param (the larger this param it, the more the curves are
   * tesselated).
   */
  float curves_scale;

  /// Curve vertices.
  csVector3* curve_vertices;
  /// Texture coords of curve vertices
  csVector2* curve_texels;

  /// Number of vertices.
  int num_curve_vertices;
  /// Maximum number of vertices.
  int max_curve_vertices;

  float current_lod;
  uint32 current_features;

public:
  /// Set of flags
  csFlags flags;

  /**
   * How many times are we busy drawing this thing (recursive).
   * This is an important variable as it indicates to
   * 'new_transformation' which set of camera vertices it should
   * use.
   */
  int draw_busy;

private:
  
  /// Calculates the interpolated normals of all vertices
  void CalculateNormals();

  /**
   * Prepare the polygon buffer for use by DrawPolygonMesh.
   * If the polygon buffer is already made then this function will do
   * nothing.
   */
  void PreparePolygonBuffer ();

  /**
   * Invalidate a thing. This has to be called when new polygons are
   * added or removed.
   */
  void InvalidateThing ();

  /**
   * Draw the given array of polygons in the current thing.
   * This version uses iGraphics3D->DrawPolygonMesh()
   * for more efficient rendering. WARNING! This version only works for
   * lightmapped polygons right now and is far from complete.
   */
  void DrawPolygonArrayDPM (csPolygon3D** polygon, int num,
	iRenderView* rview, iMovable* movable, csZBufMode zMode);

  /**
   * Draw the given array of polygons.
   */
  void DrawPolygonArray (csPolygon3D** polygon, int num,
	iRenderView* rview, csZBufMode zMode);

  /**
   * Draw one 3D/2D polygon combination. The 2D polygon is the transformed
   * and clipped version of the 3D polygon.
   */
  static void DrawOnePolygon (csPolygon3D* p, csPolygon2D* poly,
	iRenderView* d, csZBufMode zMode);

  /**
   * Utility function to be called whenever movable changes so the
   * object to world transforms in all the curves have to be updated.
   */
  void UpdateCurveTransform (const csReversibleTransform& movtrans);

  /**
   * Utility function to call when the thing never moves but the
   * curve transform has to be updated. The identity transformation
   * is used.
   */
  void UpdateCurveTransform ();

  /// Cleanup the thing edge table.
  void CleanupThingEdgeTable ();

  /// Compute table of thing edges if needed.
  void ComputeThingEdgeTable ();

  /// Generate a cachename based on geometry.
  char* GenerateCacheName ();  

public:
  /**
   * Create an empty thing.
   */
  csThing (iBase* parent, csThingObjectType* thing_type);

  /// Destructor.
  virtual ~csThing ();

  //----------------------------------------------------------------------
  // Vertex handling functions
  //----------------------------------------------------------------------

  /// Just add a new vertex to the thing.
  int AddVertex (const csVector3& v) { return AddVertex (v.x, v.y, v.z); }

  /// Just add a new vertex to the thing.
  int AddVertex (float x, float y, float z);

  /**
   * Add a vertex but first check if there is already
   * a vertex close to the wanted position. In that case
   * don't add a new vertex but return the index of the old one.
   * Note that this function is not very efficient. If you plan
   * to add a lot of vertices you should just use AddVertex()
   * and call CompressVertices() later.
   */
  int AddVertexSmart (const csVector3& v)
  {
    return AddVertexSmart (v.x, v.y, v.z);
  }

  /**
   * Add a vertex but first check if there is already
   * a vertex close to the wanted position. In that case
   * don't add a new vertex but return the index of the old one.
   * Note that this function is not very efficient. If you plan
   * to add a lot of vertices you should just use AddVertex()
   * and call CompressVertices() later.
   */
  int AddVertexSmart (float x, float y, float z);

  /**
   * Compress the vertex table so that all nearly identical vertices
   * are compressed. The polygons in the set are automatically adapted.
   * This function can be called at any time in the creation of the object
   * and it can be called multiple time but it normally only makes sense
   * to call this function after you have finished adding all polygons
   * and all vertices.<p>
   * Note that calling this function will make the camera vertex array
   * invalid.
   */
  void CompressVertices ();

  /**
   * Optimize the vertex table so that all unused vertices are deleted.
   * Note that calling this function will make the camera vertex array
   * invalid.
   */
  void RemoveUnusedVertices ();

  /// Return the object space vector for the vertex.
  const csVector3& Vobj (int idx) const { return obj_verts[idx]; }

  /**
   * Return the world space vector for the vertex.
   * Make sure you recently called WorUpdate(). Otherwise it is
   * possible that this coordinate will not be valid.
   */
  const csVector3& Vwor (int idx) const { return wor_verts[idx]; }

  /**
   * Return the camera space vector for the vertex.
   * Make sure you recently called UpdateTransformation(). Otherwise it is
   * possible that this coordinate will not be valid.
   */
  const csVector3& Vcam (int idx) const { return cam_verts[idx]; }

  /// Return the number of vertices.
  int GetVertexCount () const { return num_vertices; }

  /// Change a vertex.
  void SetVertex (int idx, const csVector3& vt);

  /// Delete a vertex.
  void DeleteVertex (int idx);

  /// Delete a range of vertices.
  void DeleteVertices (int from, int to);

  //----------------------------------------------------------------------
  // Polygon handling functions
  //----------------------------------------------------------------------

  /// Add a polygon to this thing.
  void AddPolygon (csPolygon3D* spoly);

  /// Create a new polygon in this thing and add it.
  csPolygon3D* NewPolygon (iMaterialWrapper* material);

  /// Get the number of polygons in this thing.
  int GetPolygonCount ()
  { return polygons.Length (); }

  /// Get the specified polygon from this set.
  csPolygon3D *GetPolygon3D (int idx)
  { return polygons.Get (idx); }

  /// Get the named polygon from this set.
  csPolygon3D *GetPolygon3D (const char* name);

  /// Get the entire array of polygons.
  csPolygonArray& GetPolygonArray () { return polygons; }

  /**
   * Get a new polygon ID. This is used by the polygon constructor.
   */
  unsigned long GetNewPolygonID ()
  {
    last_polygon_id++;
    return last_polygon_id;
  }

  /// Find a polygon index.
  int FindPolygonIndex (iPolygon3D* polygon) const;

  /// Remove a single polygon.
  void RemovePolygon (int idx);

  /// Remove all polygons.
  void RemovePolygons ();

  // Smoothing handling Functions

  /// Returns the smothing flag.
  bool GetSmoothingFlag () { return smoothed; }

  /// Sets the smoothing flag.
  void SetSmoothingFlag (bool smooth);

  /// Returns the normals array.
  csVector3* GetNormals () { return obj_normals; }

  //----------------------------------------------------------------------
  // Curve handling functions
  //----------------------------------------------------------------------

  /// Add a curve to this thing.
  void AddCurve (csCurve* curve);

  /// Get the number of curves in this thing.
  int GetCurveCount () const
  { return curves.Length (); }

  /// Get the specified curve from this set.
  csCurve* GetCurve (int idx) const
  { return curves.Get (idx); }

  /// Create a curve from a template.
  iCurve* CreateCurve (iCurveTemplate* tmpl);

  /// Find a curve index.
  int FindCurveIndex (iCurve* curve) const;

  /// Delete a curve given an index.
  void RemoveCurve (int idx);

  /// Delete all curves.
  void RemoveCurves ();

  /// Get the named curve from this set.
  csCurve* GetCurve (char* name) const;

  /// Get the number of curve vertices.
  int GetCurveVertexCount () const { return num_curve_vertices; }

  /// Get the specified curve vertex.
  csVector3& GetCurveVertex (int i) const { return curve_vertices[i]; }

  /// Get the curve vertices.
  csVector3* GetCurveVertices () const { return curve_vertices; }

  /// Get the specified curve texture coordinate (texel).
  csVector2& GetCurveTexel (int i) const { return curve_texels[i]; }

  /// Get the specified curve coordinate.
  void SetCurveVertex (int idx, const csVector3& vt);

  /// Set the specified curve texture coordinate (texel).
  void SetCurveTexel (int idx, const csVector2& vt);

  /// Clear the curve vertices.
  void ClearCurveVertices ();

  /// Add a curve vertex and return the index of the vertex.
  int AddCurveVertex (const csVector3& v, const csVector2& t);

  /// Get the curve scale.
  float GetCurvesScale () const { return curves_scale; }

  /// Set the curve scale.
  void SetCurvesScale (float f) { curves_scale = f; }

  /// Get the curves center.
  const csVector3& GetCurvesCenter () const { return curves_center; }

  /// Set the curves center.
  void SetCurvesCenter (csVector3& v) { curves_center = v; }

  //----------------------------------------------------------------------
  // Setup
  //----------------------------------------------------------------------

  /**
   * Prepare all polygons for use. This function MUST be called
   * AFTER the texture manager has been prepared. This function
   * is normally called by csEngine::Prepare() so you only need
   * to worry about this function when you add sectors or things
   * later.
   */
  void Prepare ();

  /**
   * Merge the given Thing into this one. The other polygons and
   * curves are removed from the other thing so that it is ready to
   * be removed. Warning! All Things are merged in world space
   * coordinates and not in object space as one could expect!
   */
  void Merge (csThing* other);

  /**
   * Add polygons and vertices from the specified thing (seen as template).
   */
  void MergeTemplate (iThingState* tpl,
  	iMaterialWrapper* default_material = NULL,
	csVector3* shift = NULL, csMatrix3* transform = NULL);

  /**
   * Replace the materials in this thing with new materials that are
   * prefixed by some name. For example, if a polygon in this thing uses
   * a material 'blabla' and the prefix is 'pref' then the new material
   * that will be used is called 'pref_blabla'. If that material cannot
   * be found then the original material will be used.
   */
  void ReplaceMaterials (iMaterialList* matList, const char* prefix);

  /// Set parent template.
  void SetTemplate (csThing *t)
  { ParentTemplate = t; }

  /// Query parent template.
  csThing *GetTemplate () const
  { return ParentTemplate; }

  void FireListeners ();
  void AddListener (iObjectModelListener* listener);
  void RemoveListener (iObjectModelListener* listener);

  //----------------------------------------------------------------------
  // Bounding information
  //----------------------------------------------------------------------

  /**
   * Create an oriented bounding box (currently not oriented yet@@@)
   * for this polygon set. This function will add the vertices for the
   * bounding box to the set itself so that they will get translated
   * together with the other vertices. The indices of the vertices
   * are added to the csThingBBox structure which is returned here.
   * Note that this creation is done in object space. The newly added
   * vertices will not have been translated to world/camera space yet.<p>
   */
  void CreateBoundingBox ();

  /// Get the oriented bounding box created by CreateBoundingBox().
  csThingBBox* GetBoundingBox ()
  {
    if (!bbox) CreateBoundingBox ();
    return bbox;
  }

  /**
   * Get bounding box given some transformation.
   */
  void GetTransformedBoundingBox (
	const csReversibleTransform& trans, csBox3& cbox);

  /**
   * Get bounding box in screen space.
   */
  float GetScreenBoundingBox (
	float fov, float sx, float sy,
	const csReversibleTransform& trans, csBox2& sbox, csBox3& cbox);

  /**
   * Get the bounding box in object space for this polygon set.
   * This is calculated based on the oriented bounding box.
   */
  void GetBoundingBox (csBox3& box);

  /**
   * Get the bounding box for this object given some transformation (movable).
   */
  void GetBoundingBox (iMovable* movable, csBox3& box);

  /**
   * Get the radius in object space for this polygon set.
   */
  void GetRadius (csVector3& rad, csVector3& cent);

  /**
   * Add a polygon to the list of polygons having a portal.
   * This function is called by a csPolygon3D in this thing
   * whenever it gets a portal.
   */
  void AddPortalPolygon (csPolygon3D* poly);

  /**
   * Remove a polygon from the list of polygons having a portal.
   */
  void RemovePortalPolygon (csPolygon3D* poly);

  /**
   * Get a write object for a vis culling system.
   */
  iPolygonMesh* GetWriteObject ();

  //----------------------------------------------------------------------
  // Drawing
  //----------------------------------------------------------------------

  /**
   * Test if this thing is visible or not.
   */
  bool DrawTest (iRenderView* rview, iMovable* movable);

  /**
   * Draw this thing given a view and transformation.
   */
  bool Draw (iRenderView* rview, iMovable* movable, csZBufMode zMode);

  /**
   * Draw all curves in this thing given a view and transformation.
   */
  bool DrawCurves (iRenderView* rview, iMovable* movable, csZBufMode zMode);

  /**
   * Draw this thing as a fog volume (only when fog is enabled for
   * this thing).
   */
  bool DrawFoggy (iRenderView* rview, iMovable* movable);

  //----------------------------------------------------------------------
  // Lighting
  //----------------------------------------------------------------------

  /**
   * Init the lightmaps for all polygons in this thing.
   */
  void InitializeDefault ();

  /**
   * Read the lightmaps from the cache.
   */
  bool ReadFromCache (iCacheManager* cache_mgr);

  /**
   * Cache the lightmaps for all polygons in this thing.
   */
  bool WriteToCache (iCacheManager* cache_mgr);

  /**
   * Prepare the lightmaps for all polys so that they are suitable
   * for the 3D rasterizer.
   */
  void PrepareLighting ();

  /// Marks the whole object as it is affected by any light.
  void MarkLightmapsDirty ();

  //----------------------------------------------------------------------
  // Utility functions
  //----------------------------------------------------------------------

  /**
   * Intersects object-space sphere with polygons of this set. Return
   * polygon it hits with (or NULL) and the intersection point
   * in object coordinates. It will also return the polygon with the
   * closest hit (the most nearby polygon).
   * If 'pr' != NULL it will also return the distance where the
   * intersection happened.
   */
  csPolygon3D* IntersectSphere (csVector3& center, float radius,
  	float* pr = NULL);

  /**
   * Intersect object-space segment with polygons of this set. Return
   * polygon it intersects with (or NULL) and the intersection point
   * in object coordinates.<p>
   *
   * If 'pr' != NULL it will also return a value between 0 and 1
   * indicating where on the 'start'-'end' vector the intersection
   * happened.<p>
   *
   * If only_portals == true then only portals are checked.
   */
  csPolygon3D* IntersectSegment (const csVector3& start,
	const csVector3& end, csVector3& isect,
	float* pr = NULL, bool only_portals = false);

  /**
   * Intersect object-space segment with polygons of this set. Return
   * polygon it intersects with (or NULL) and the intersection point
   * in object coordinates.<p>
   *
   * If 'pr' != NULL it will also return a value between 0 and 1
   * indicating where on the 'start'-'end' vector the intersection
   * happened.<p>
   *
   * This version is similar to IntersectSegment() but it will also test
   * polygons of all objects which are registered to the visibility culler
   * of this thing (if there is any). It also returns the mesh wrapper
   * that was hit. If this is NULL then this mesh was hit.
   */
  csPolygon3D* IntersectSegmentFull (const csVector3& start,
	const csVector3& end, csVector3& isect,
	float* pr = NULL, csMeshWrapper** p_mesh = NULL);

  /**
   * Check frustum visibility on this thing.
   * First initialize the 2D culler cube.
   */
  void CastShadows (iFrustumView* lview, iMovable* movable);

  /**
   * Append a list of shadow frustums which extend from
   * this thing. The origin is the position of the light.
   */
  void AppendShadows (iMovable* movable, iShadowBlockList* shadows,
  	const csVector3& origin);

  /**
   * Test a beam with this thing.
   */
  bool HitBeamOutline (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr);

  /**
   * Test a beam with this thing.
   */
  bool HitBeamObject (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr);

  //----------------------------------------------------------------------
  // Transformation
  //----------------------------------------------------------------------

  /**
   * Transform to the given camera if needed. This function will use
   * the camera number to avoid unneeded transformation.
   */
  void UpdateTransformation (const csTransform& c, long cam_cameranr);

  /// Make sure the world vertices are up-to-date.
  void WorUpdate ();

  /**
   * Do a hard transform of the object vertices.
   * This transformation and the original coordinates are not
   * remembered but the object space coordinates are directly
   * computed (world space coordinates are set to the object space
   * coordinates by this routine).
   */
  void HardTransform (const csReversibleTransform& t);

  /// Get the array of camera vertices.
  csVector3* GetCameraVertices (const csTransform& c, long cam_cameranr)
  {
    UpdateTransformation (c, cam_cameranr);
    return cam_verts;
  }

  //----------------------------------------------------------------------
  // Various
  //----------------------------------------------------------------------

  /**
   * Control how this thing will be moved.
   */
  void SetMovingOption (int opt);

  /**
   * Get the moving option.
   */
  int GetMovingOption () const { return cfg_moving; }

  /**
   * Set convexity flag of this thing. You should call this instead
   * of SetFlags (CS_ENTITY_CONVEX, CS_ENTITY_CONVEX) because this function
   * does some extra calculations.
   */
  void SetConvex (bool c);

  /**
   * If this thing is convex you can use getCenter to get the index
   * of the vertex holding the center of this thing. This center is
   * calculated by 'setConvex(true)'.
   */
  int GetCenter () { return center_idx; }

#ifndef CS_USE_NEW_RENDERER
  /// Return true if this has fog.
  bool HasFog () { return fog.enabled; }

  /// Return fog structure.
  csFog& GetFog () { return fog; }

  /// Conveniance function to set fog to some setting.
  void SetFog (float density, const csColor& color)
  {
    fog.enabled = true;
    fog.density = density;
    fog.red = color.red;
    fog.green = color.green;
    fog.blue = color.blue;
  }

  /// Disable fog.
  void DisableFog () { fog.enabled = false; }
#endif // CS_USE_NEW_RENDERER

  /// Sets dynamic ambient light for this thing
  void SetDynamicAmbientLight(const csColor& color)
  {
      dynamic_ambient = color;
      ambient_version++;
  }
  /// Gets dynamic ambient light for this thing
  const csColor& GetDynamicAmbientLight()
  {
      return dynamic_ambient;
  }

  /// Get dynamic ambient light version to test if needs to be recalculated
  uint32 GetDynamicAmbientVersion()
  {   return ambient_version; }

  void DynamicLightChanged (iDynLight* dynlight);
  void DynamicLightDisconnect (iDynLight* dynlight);
  void StaticLightChanged (iStatLight* statlight);

  SCF_DECLARE_IBASE_EXT (csObject);

  //------------------------- iThingState interface -------------------------
  struct ThingState : public iThingState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csThing);
    virtual void* GetPrivateObject () { return (void*)scfParent; }
    virtual iObject* QueryObject () { return scfParent; }
    virtual void CompressVertices () { scfParent->CompressVertices(); }

    virtual int GetPolygonCount () { return scfParent->polygons.Length (); }
    virtual iPolygon3D *GetPolygon (int idx);
    virtual iPolygon3D *GetPolygon (const char* name);
    virtual iPolygon3D *CreatePolygon (const char *iName);
    virtual int FindPolygonIndex (iPolygon3D* polygon) const
    { return scfParent->FindPolygonIndex (polygon); }
    virtual void RemovePolygon (int idx)
    { scfParent->RemovePolygon (idx); }
    virtual void RemovePolygons ()
    { scfParent->RemovePolygons (); }

    virtual int GetPortalCount () const;
    virtual iPortal* GetPortal (int idx) const;
    virtual iPolygon3D* GetPortalPolygon (int idx) const;

    virtual int GetVertexCount () const { return scfParent->num_vertices; }
    virtual const csVector3 &GetVertex (int i) const
    { return scfParent->obj_verts[i]; }
    virtual const csVector3* GetVertices () const
    { return scfParent->obj_verts; }
    virtual const csVector3 &GetVertexW (int i) const
    { return scfParent->wor_verts[i]; }
    virtual const csVector3* GetVerticesW () const
    { return scfParent->wor_verts; }
    virtual const csVector3 &GetVertexC (int i) const
    { return scfParent->cam_verts[i]; }
    virtual const csVector3* GetVerticesC () const
    { return scfParent->cam_verts; }
    virtual int CreateVertex (const csVector3 &iVertex)
    { return scfParent->AddVertex (iVertex.x, iVertex.y, iVertex.z); }
    virtual void SetVertex (int idx, const csVector3& vt)
    { scfParent->SetVertex (idx, vt); }
    virtual void DeleteVertex (int idx)
    { scfParent->DeleteVertex (idx); }
    virtual void DeleteVertices (int from, int to)
    { scfParent->DeleteVertices (from, to); }

    virtual csFlags& GetFlags () { return scfParent->flags; }
    virtual int GetMovingOption () const
    { return scfParent->GetMovingOption (); }
    virtual void SetMovingOption (int opt)
    { scfParent->SetMovingOption (opt); }

    virtual const csVector3& GetCurvesCenter () const
    { return scfParent->curves_center; }
    virtual void SetCurvesCenter (const csVector3& cen)
    { scfParent->curves_center = cen; }
    virtual float GetCurvesScale () const
    { return scfParent->curves_scale; }
    virtual void SetCurvesScale (float scale)
    { scfParent->curves_scale = scale; }
    virtual int GetCurveCount () const
    { return scfParent->GetCurveCount (); }
    virtual int GetCurveVertexCount () const
    { return scfParent->GetCurveVertexCount (); }
    virtual csVector3& GetCurveVertex (int i) const
    { return scfParent->GetCurveVertex (i); }
    virtual csVector3* GetCurveVertices () const
    { return scfParent->GetCurveVertices (); }
    virtual csVector2& GetCurveTexel (int i) const
    { return scfParent->GetCurveTexel (i); }
    virtual void SetCurveVertex (int idx, const csVector3& vt)
    { scfParent->SetCurveVertex (idx, vt); }
    virtual void SetCurveTexel (int idx, const csVector2& vt)
    { scfParent->SetCurveTexel (idx, vt); }
    virtual void ClearCurveVertices ()
    { scfParent->ClearCurveVertices (); }
    virtual iCurve* GetCurve (int idx) const;
    virtual iCurve* CreateCurve (iCurveTemplate* tmpl)
    { return scfParent->CreateCurve (tmpl); }
    virtual int FindCurveIndex (iCurve* curve) const
    { return scfParent->FindCurveIndex (curve); }
    virtual void RemoveCurve (int idx)
    { scfParent->RemoveCurve (idx); }
    virtual void RemoveCurves ()
    { scfParent->RemoveCurves (); }

    virtual void MergeTemplate (iThingState* tpl,
  	iMaterialWrapper* default_material = NULL,
	csVector3* shift = NULL, csMatrix3* transform = NULL)
    {
      scfParent->MergeTemplate (tpl, default_material, shift, transform);
    }
    virtual void ReplaceMaterials (iMaterialList* materials,
    	const char* prefix)
    {
      scfParent->ReplaceMaterials (materials, prefix);
    }
    virtual void AddCurveVertex (const csVector3& v, const csVector2& uv)
    {
      scfParent->AddCurveVertex (v, uv);
    }
#ifndef CS_USE_NEW_RENDERER
    virtual bool HasFog () const
    { return scfParent->HasFog (); }
    virtual csFog *GetFog () const
    { return &scfParent->GetFog (); }
#endif // CS_USE_NEW_RENDERER

    virtual iPolygon3D* IntersectSegment (const csVector3& start,
	const csVector3& end, csVector3& isect,
	float* pr = NULL, bool only_portals = false);

    // Normals Handling functions

    /// Sets the smoothing flag
    virtual void SetSmoothingFlag (bool smoothing)
    { 
      scfParent->SetSmoothingFlag (smoothing);
    }

    /// Sets the smoothing flag
    virtual bool GetSmoothingFlag ()
    { 
      return scfParent->GetSmoothingFlag ();
    }
  
    /// Sets the smoothing flag
    virtual csVector3* GetNormals ()
    {
      return scfParent->GetNormals ();
    }
  } scfiThingState;
  friend struct ThingState;

  //------------------------- iLightingInfo interface -------------------------
  /// iLightingInfo implementation.
  struct LightingInfo : public iLightingInfo
  {
    SCF_DECLARE_EMBEDDED_IBASE (csThing);
    virtual void InitializeDefault ()
    {
      scfParent->InitializeDefault ();
    }
    virtual bool ReadFromCache (iCacheManager* cache_mgr)
    {
      return scfParent->ReadFromCache (cache_mgr);
    }
    virtual bool WriteToCache (iCacheManager* cache_mgr)
    {
      return scfParent->WriteToCache (cache_mgr);
    }
    virtual void PrepareLighting ()
    {
      scfParent->PrepareLighting ();
    }
    virtual void SetDynamicAmbientLight (const csColor& color)
    { scfParent->SetDynamicAmbientLight (color); }
    virtual const csColor& GetDynamicAmbientLight ()
    { return scfParent->GetDynamicAmbientLight (); }
    virtual uint32 GetDynamicAmbientVersion () const
    { return scfParent->GetDynamicAmbientVersion (); }
    virtual void DynamicLightChanged (iDynLight* dynlight)
    { scfParent->DynamicLightChanged (dynlight); }
    virtual void DynamicLightDisconnect (iDynLight* dynlight)
    { scfParent->DynamicLightDisconnect (dynlight); }
    virtual void StaticLightChanged (iStatLight* statlight)
    { scfParent->StaticLightChanged (statlight); }
  } scfiLightingInfo;
  friend struct LightingInfo;

  //-------------------- iPolygonMesh interface implementation ---------------
  struct PolyMesh : public PolyMeshHelper
  {
    SCF_DECLARE_EMBEDDED_IBASE (csThing);
    PolyMesh () : PolyMeshHelper (CS_POLY_COLLDET) { }
  } scfiPolygonMesh;

  //------------------- Lower detail iPolygonMesh implementation ---------------
  struct PolyMeshLOD : public PolyMeshHelper
  {
    PolyMeshLOD ();
    // @@@ Not embedded because we can't have two iPolygonMesh implementations
    // in csThing.
    SCF_DECLARE_IBASE;
  } scfiPolygonMeshLOD;

  //-------------------- iShadowCaster interface implementation ----------
  struct ShadowCaster : public iShadowCaster
  {
    SCF_DECLARE_EMBEDDED_IBASE (csThing);
    virtual void AppendShadows (iMovable* movable, iShadowBlockList* shadows, const csVector3& origin)
    {
      scfParent->AppendShadows (movable, shadows, origin);
    }
  } scfiShadowCaster;
  friend struct ShadowCaster;

  //-------------------- iShadowReceiver interface implementation ----------
  struct ShadowReceiver : public iShadowReceiver
  {
    SCF_DECLARE_EMBEDDED_IBASE (csThing);
    virtual void CastShadows (iMovable* movable, iFrustumView* fview)
    {
      scfParent->CastShadows (fview, movable);
    }
  } scfiShadowReceiver;
  friend struct ShadowReceiver;

  //------------------------- iObjectModel implementation ----------------
  class ObjectModel : public iObjectModel
  {
    SCF_DECLARE_EMBEDDED_IBASE (csThing);
    virtual long GetShapeNumber () const { return scfParent->shapenr; }
    virtual iPolygonMesh* GetPolygonMeshColldet ()
    {
      return &(scfParent->scfiPolygonMesh);
    }
    virtual iPolygonMesh* GetPolygonMeshViscull ()
    {
      return &(scfParent->scfiPolygonMeshLOD);
    }
    virtual csPtr<iPolygonMesh> CreateLowerDetailPolygonMesh (float)
    { return 0; }
    virtual void GetObjectBoundingBox (csBox3& bbox,
    	int /*type = CS_BBOX_NORMAL*/)
    {
      scfParent->GetBoundingBox (bbox);
    }
    virtual void GetRadius (csVector3& rad, csVector3& cent)
    {
      scfParent->GetRadius (rad, cent);
    }
    virtual void AddListener (iObjectModelListener* listener)
    {
      scfParent->AddListener (listener);
    }
    virtual void RemoveListener (iObjectModelListener* listener)
    {
      scfParent->RemoveListener (listener);
    }
  } scfiObjectModel;
  friend class ObjectModel;

  //-------------------- iMeshObject interface implementation ----------
  struct MeshObject : public iMeshObject
  {
    SCF_DECLARE_EMBEDDED_IBASE (csThing);
    virtual iMeshObjectFactory* GetFactory () const;
    virtual bool DrawTest (iRenderView* rview, iMovable* movable)
    {
      return scfParent->DrawTest (rview, movable);
    }
    virtual void UpdateLighting (iLight** /*lights*/, int /*num_lights*/,
      	iMovable* /*movable*/) { }
    virtual bool Draw (iRenderView* rview, iMovable* movable,
    	csZBufMode zMode)
    {
      return scfParent->Draw (rview, movable, zMode);
    }
    virtual void SetVisibleCallback (iMeshObjectDrawCallback* /*cb*/) { }
    virtual iMeshObjectDrawCallback* GetVisibleCallback () const
    { return NULL; }
    virtual void NextFrame (csTicks /*current_time*/,const csVector3& /*pos*/) { }
    virtual bool WantToDie () const { return false; }
    virtual void HardTransform (const csReversibleTransform& t)
    {
      scfParent->HardTransform (t);
    }
    virtual bool SupportsHardTransform () const { return true; }
    virtual bool HitBeamOutline (const csVector3& start, const csVector3& end,
      csVector3& isect, float* pr)
    {
      return scfParent->HitBeamOutline (start, end, isect, pr);
    }
    virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr)
    {
      return scfParent->HitBeamObject (start, end, isect, pr);
    }
    virtual void SetLogicalParent (iBase* lp) { scfParent->logparent = lp; }
    virtual iBase* GetLogicalParent () const { return scfParent->logparent; }
    virtual iObjectModel* GetObjectModel ()
    {
      return &(scfParent->scfiObjectModel);
    }
    virtual bool SetColor (const csColor&) { return false; }
    virtual bool GetColor (csColor&) const { return false; }
    virtual bool SetMaterialWrapper (iMaterialWrapper*) { return false; }
    virtual iMaterialWrapper* GetMaterialWrapper () const { return NULL; }
  } scfiMeshObject;
  friend struct MeshObject;

  //-------------------- iMeshObjectFactory interface implementation ---------
  struct MeshObjectFactory : public iMeshObjectFactory
  {
    SCF_DECLARE_EMBEDDED_IBASE (csThing);
    virtual csPtr<iMeshObject> NewInstance ();
    virtual void HardTransform (const csReversibleTransform& t)
    {
      scfParent->HardTransform (t);
    }
    virtual bool SupportsHardTransform () const { return true; }
    virtual void SetLogicalParent (iBase* lp) { scfParent->logparent = lp; }
    virtual iBase* GetLogicalParent () const { return scfParent->logparent; }
  } scfiMeshObjectFactory;
  friend struct MeshObjectFactory;
};

/**
 * Thing type. This is the plugin you have to use to create instances
 * of csThing.
 */
class csThingObjectType : public iMeshObjectType
{
private:
  /**
   * List of planes. This vector contains objects of type
   * csPolyTxtPlane*. Note that this vector only contains named
   * planes. Default planes which are created for polygons
   * are not in this list.
   */
  csNamedObjVector planes;

  /**
   * List of curve templates (bezier templates). This vector contains objects of
   * type csCurveTemplate*.
   */
  csNamedObjVector curve_templates;

public:
  iObjectRegistry* object_reg;
  iEngine* engine;
  iGraphics3D* G3D;
  /// An object pool for 2D polygons used by the rendering process.
  csPoly2DPool* render_pol2d_pool;
  /// An object pool for lightpatches.
  csLightPatchPool* lightpatch_pool;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csThingObjectType (iBase*);

  /// Destructor.
  virtual ~csThingObjectType ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);
  void Clear ();

  void Warn (const char *description, ...);
  void Bug (const char *description, ...);
  void Notify (const char *description, ...);
  void Error (const char *description, ...);

  /// New Factory.
  virtual csPtr<iMeshObjectFactory> NewFactory ();

  csPtr<iPolyTxtPlane> CreatePolyTxtPlane (const char* name = NULL);
  iPolyTxtPlane* FindPolyTxtPlane (const char* name);
  csPtr<iCurveTemplate> CreateBezierTemplate (const char* name = NULL);
  iCurveTemplate* FindCurveTemplate (const char *iName);
  void RemovePolyTxtPlane (iPolyTxtPlane* pl);
  void RemoveCurveTemplate (iCurveTemplate* ct);
  void ClearPolyTxtPlanes ();
  void ClearCurveTemplates ();

  /// iThingEnvironment implementation.
  struct eiThingEnvironment : public iThingEnvironment
  {
    SCF_DECLARE_EMBEDDED_IBASE(csThingObjectType);
    virtual void Clear ()
    {
      scfParent->Clear ();
    }
    virtual csPtr<iPolyTxtPlane> CreatePolyTxtPlane (const char* name = NULL)
    {
      return scfParent->CreatePolyTxtPlane (name);
    }
    virtual iPolyTxtPlane* FindPolyTxtPlane (const char* name)
    {
      return scfParent->FindPolyTxtPlane (name);
    }
    virtual csPtr<iCurveTemplate> CreateBezierTemplate (const char* name = NULL)
    {
      return scfParent->CreateBezierTemplate (name);
    }
    virtual iCurveTemplate* FindCurveTemplate (const char* name)
    {
      return scfParent->FindCurveTemplate (name);
    }
    virtual void RemovePolyTxtPlane (iPolyTxtPlane* pl)
    {
      scfParent->RemovePolyTxtPlane (pl);
    }
    virtual void RemoveCurveTemplate (iCurveTemplate* ct)
    {
      scfParent->RemoveCurveTemplate (ct);
    }
    virtual void ClearPolyTxtPlanes ()
    {
      scfParent->ClearPolyTxtPlanes ();
    }
    virtual void ClearCurveTemplates ()
    {
      scfParent->ClearCurveTemplates ();
    }
    virtual int GetLightmapCellSize () const
    {
      return csLightMap::lightcell_size;
    }
    virtual void SetLightmapCellSize (int size)
    {
      csLightMap::lightcell_size = size;
    }
    virtual int GetDefaultLightmapCellSize () const
    {
      return csLightMap::default_lightmap_cell_size;
    }
  } scfiThingEnvironment;

  /// iComponent implementation.
  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csThingObjectType);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;

  /// iConfig implementation.
  struct eiConfig : public iConfig
  {
    SCF_DECLARE_EMBEDDED_IBASE(csThingObjectType);
    virtual bool GetOptionDescription (int idx, csOptionDescription *option);
    virtual bool SetOption (int id, csVariant* value);
    virtual bool GetOption (int id, csVariant* value);
  } scfiConfig;
};

#endif // __CS_THING_H__
