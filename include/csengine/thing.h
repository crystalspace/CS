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
#include "csobject/pobject.h"
#include "csengine/bsp.h"
#include "csengine/movable.h"
#include "csutil/flags.h"
#include "csutil/cscolor.h"
#include "csutil/csvector.h"
#include "csutil/garray.h"
#include "iengine/thing.h"
#include "iengine/movable.h"
#include "iengine/polymesh.h"
#include "iengine/viscull.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/rview.h"

class csSector;
class csEngine;
class csStatLight;
class csMaterialWrapper;
class csMaterialList;
class csThing;
class csPolygon3D;
class csPolygonInt;
class csPolygonTree;
class csPolygon2D;
class csPolygon2DQueue;
class csFrustumList;
class csFrustumView;
struct iShadowBlockList;
struct csVisObjInfo;
struct iGraphics3D;
struct iRenderView;
struct iMovable;
class Dumper;

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
 * If CS_ENTITY_CONVEX is set then this entity is convex (what did
 * you expect :-)
 * This means the 3D engine can do various optimizations.
 * If you set 'convex' to true the center vertex will also be calculated.
 * It is unset by default (@@@ should be calculated).
 */
#define CS_ENTITY_CONVEX 1

/**
 * If CS_ENTITY_DETAIL is set then this entity is a detail
 * object. A detail object is treated as a single object by
 * the engine. The engine can do several optimizations on this.
 * In general you should use this flag for small and detailed
 * objects. Detail objects are not included in BSP or octrees.
 */
#define CS_ENTITY_DETAIL 2

/**
 * If CS_ENTITY_CAMERA is set then this entity will be always
 * be centerer around the same spot relative to the camera. This
 * is useful for skyboxes or skydomes.
 */
#define CS_ENTITY_CAMERA 4

/**
 * If CS_ENTITY_ZFILL is set then this thing will be rendered with
 * ZFILL instead of fully using the Z-buffer. This is useful for
 * things that make the outer walls of a sector.
 */
#define CS_ENTITY_ZFILL 8

/**
 * If CS_ENTITY_INVISIBLE is set then this thing will not be rendered.
 * It will still cast shadows and be present otherwise. Use the
 * CS_ENTITY_NOSHADOWS flag to disable shadows.
 */
#define CS_ENTITY_INVISIBLE 16

/**
 * If CS_ENTITY_NOSHADOWS is set then this thing will not cast
 * shadows. Lighting will still be calculated for it though. Use the
 * CS_ENTITY_NOLIGHTING flag to disable that.
 */
#define CS_ENTITY_NOSHADOWS 32

/**
 * If CS_ENTITY_NOLIGHTING is set then this thing will not be lit.
 * It may still cast shadows though. Use the CS_ENTITY_NOSHADOWS flag
 * to disable that.
 */
#define CS_ENTITY_NOLIGHTING 64

/**
 * If CS_ENTITY_VISTREE is set then an octree will be calculated for the
 * polygons in this thing. In this case the thing will implement a
 * fully working iVisibilityCuller which the sector can use.
 */
#define CS_ENTITY_VISTREE 128

/**
 * If CS_ENTITY_BACK2FRONT is set then all objects with the same
 * render order as this one and which also have this flag set will
 * be rendered in roughly back to front order. All objects with
 * the same render order but which do not have this flag set will
 * be rendered later. This flag is important if you want to have
 * alpha transparency rendered correctly.
 */
#define CS_ENTITY_BACK2FRONT 256

/**
 * The following flags affect movement options for a thing. See
 * SetMovingOption() for more info.
 */
#define CS_THING_MOVE_NEVER 0
#define CS_THING_MOVE_OFTEN 1
#define CS_THING_MOVE_OCCASIONAL 2


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
 * the Thing must be convex.<p>
 *
 * If you add a static tree (octree/BSP trees) to a thing it can
 * be used as a visibility culler (i.e. it implements iVisibilityCuller).
 */
class csThing : public csPObject
{
  friend class csMovable;
  friend class Dumper;

private:
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
  /// Camera number for which the above camera vertices are valid.
  long cameranr;

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

  /// The array of curves forming the outside of the set
  csCurvesArray curves;

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

  /**
   * Light frame number. Using this number one can see if gouraud shaded
   * vertices have been initialized already.
   */
  long light_frame_number;

  /// Fog information.
  csFog fog;

  /**
   * Vector with visibility objects. Only useful if this thing has a
   * static tree and thus can do visibility testing.
   */
  csVector visobjects;

  /**
   * If this variable is not NULL then it is a BSP or octree for this
   * thing.
   */
  csPolygonTree* static_tree;

  /// Engine handle.
  csEngine* engine;

  /// If convex, this holds the index to the center vertex.
  int center_idx;

  /// Pointer to the Thing Template which it derived from.
  csThing* ParentTemplate;

  /// If true this thing is visible.
  bool is_visible;

  /// If true this thing is a 'sky' object. @@@ Obsolete when 'render order' is implemented
  bool is_sky;

  /// If true this thing is a 'template' object.
  bool is_template;

  /// Position in the world.
  csMovable movable;

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

public:
  /// Set of flags
  csFlags flags;

  /**
   * Current light frame number. This is used for seeing
   * if gouraud shading should be recalculated for this thing.
   * If there is a mismatch between the frame number of this set
   * and the global number then the gouraud shading is not up-to-date.
   */
  static long current_light_frame_number;

  /**
   * How many times are we busy drawing this thing (recursive).
   * This is an important variable as it indicates to
   * 'new_transformation' which set of camera vertices it should
   * use.
   */
  int draw_busy;

private:
  /**
   * Draw the given array of polygons in the current thing.
   * This version uses iGraphics3D->DrawPolygonMesh()
   * for more efficient rendering. WARNING! This version only works for
   * lightmapped polygons right now and is far from complete.
   */
  void DrawPolygonArrayDPM (csPolygonInt** polygon, int num,
	iRenderView* rview, bool use_z_buf);

  /**
   * Draw the given array of polygons in the current csPolygonSet.
   */
  static void DrawPolygonArray (csPolygonInt** polygon, int num,
	iRenderView* rview, bool use_z_buf);

  /**
   * Test a number of polygons against the c-buffer and insert them to the
   * c-buffer if visible and also add them to a queue.
   * If 'pvs' is true then the PVS is used (polygon->IsVisible()).
   */
  static void* TestQueuePolygonArray (csPolygonInt** polygon, int num,
	iRenderView* d, csPolygon2DQueue* poly_queue, bool pvs);

  /**
   * Draw one 3D/2D polygon combination. The 2D polygon is the transformed
   * and clipped version of the 3D polygon.
   */
  static void DrawOnePolygon (csPolygon3D* p, csPolygon2D* poly,
	iRenderView* d, bool use_z_buf);

  /**
   * This function is called by the BSP tree traversal routine
   * to test polygons against the C buffer and add them to a queue if needed.
   */
  static void* TestQueuePolygons (csThing*, csPolygonInt** polygon,
  	int num, bool same_plane, void* data);

  /**
   * Draw a number of polygons from a queue (used with C buffer processing).
   */
  void DrawPolygonsFromQueue (csPolygon2DQueue* queue, iRenderView* rview);

  /**
   * This function is called by the BSP tree traversal routine
   * to draw a number of polygons.
   */
  static void* DrawPolygons (csThing*, csPolygonInt** polygon,
  	int num, bool same_plane, void* data);

  /**
   * Check if some object needs updating in the visibility information
   * (static tree) and do the update in that case.
   */
  void CheckVisUpdate (csVisObjInfo* vinf);

  /**
   * Utility function to be called whenever movable changes so the
   * object to world transforms in all the curves have to be updated.
   */
  void UpdateCurveTransform ();

  /// Internal draw function.
  bool DrawInt (iRenderView* rview, iMovable* movable);

  /// Move this thing to the specified sector. Can be called multiple times.
  void MoveToSector (csSector* s);

  /// Remove this thing from all sectors it is in (but not from the engine).
  void RemoveFromSectors ();

  /**
   * Update transformations after the thing has moved
   * (through updating the movable instance).
   * This MUST be done after you change the movable otherwise
   * some of the internal data structures will not be updated
   * correctly. This function is called by movable.UpdateMove().
   */
  void UpdateMove ();

public:
  /**
   * Create an empty thing.
   */
  csThing (csEngine*, bool is_sky = false, bool is_template = false);

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

  /// Return the object space vector for the vertex.
  csVector3& Vobj (int idx) { return obj_verts[idx]; }

  /**
   * Return the world space vector for the vertex.
   * Make sure you recently called WorUpdate(). Otherwise it is
   * possible that this coordinate will not be valid.
   */
  csVector3& Vwor (int idx) { return wor_verts[idx]; }

  /**
   * Return the camera space vector for the vertex.
   * Make sure you recently called UpdateTransformation(). Otherwise it is
   * possible that this coordinate will not be valid.
   */
  csVector3& Vcam (int idx) { return cam_verts[idx]; }

  /// Return the number of vertices.
  int GetNumVertices () { return num_vertices; }

  //----------------------------------------------------------------------
  // Polygon handling functions
  //----------------------------------------------------------------------

  /// Add a polygon to this thing.
  void AddPolygon (csPolygonInt* spoly);

  /// Create a new polygon in this thing and add it.
  csPolygon3D* NewPolygon (csMaterialWrapper* material);

  /// Get the number of polygons in this thing.
  int GetNumPolygons ()
  { return polygons.Length (); }

  /// Get a csPolygonInt with the index.
  csPolygonInt* GetPolygonInt (int idx);

  /// Get the specified polygon from this set.
  csPolygon3D *GetPolygon3D (int idx)
  { return polygons.Get (idx); }

  /// Get the named polygon from this set.
  csPolygon3D *GetPolygon3D (const char* name);

  /// Get the entire array of polygons.
  csPolygonArray& GetPolygonArray () { return polygons; }

  //----------------------------------------------------------------------
  // Curve handling functions
  //----------------------------------------------------------------------

  /// Add a curve to this thing.
  void AddCurve (csCurve* curve);

  /// Get the number of curves in this thing.
  int GetNumCurves ()
  { return curves.Length (); }

  /// Get the specified curve from this set.
  csCurve* GetCurve (int idx)
  { return curves.Get (idx); }

  /// Get the named curve from this set.
  csCurve* GetCurve (char* name);

  /// Get the number of curve vertices.
  int GetNumCurveVertices () { return num_curve_vertices; }

  /// Get the specified curve vertex.
  csVector3& CurveVertex (int i) { return curve_vertices[i]; }

  /// Get the curve vertices.
  csVector3* GetCurveVertices () { return curve_vertices; }

  /// Get the specified curve texture coordinate (texel).
  csVector2& CurveTexel (int i) { return curve_texels[i]; }

  /// Add a curve vertex and return the index of the vertex.
  int AddCurveVertex (csVector3& v, csVector2& t);

  /// Get the curve scale.
  float GetCurvesScale () { return curves_scale; }

  /// Set the curve scale.
  void SetCurvesScale (float f) { curves_scale = f; }

  /// Get the curves center.
  const csVector3& GetCurvesCenter () { return curves_center; }

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
  void Prepare (csSector* sector);

  /**
   * Merge the given Thing into this one. The other polygons and
   * curves are removed from the other thing so that it is ready to
   * be removed. Warning! All Things are merged in world space
   * coordinates and not in object space as one could expect!
   */
  void Merge (csThing* other);

  /**
   * Add polygons and vertices from the specified template. Replace the
   * materials if they match one in the matList.
   */
  void MergeTemplate (csThing* tpl, csSector* sector, csMaterialList* matList,
  	const char* prefix, 
	csMaterialWrapper* default_material = NULL,
  	float default_texlen = 1,
	csVector3* shift = NULL, csMatrix3* transform = NULL);

  /**
   * Add polygons and vertices from the specified thing (seen as template).
   */
  void MergeTemplate (csThing* tpl, csSector* sector,
  	csMaterialWrapper* default_material = NULL,
  	float default_texlen = 1,
	csVector3* shift = NULL, csMatrix3* transform = NULL);

  /// Set parent template.
  void SetTemplate (csThing *t)
  { ParentTemplate = t; }

  /// Query parent template.
  csThing *GetTemplate () const
  { return ParentTemplate; }

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
  const csVector3& GetRadius ();

  //----------------------------------------------------------------------
  // Visibility culler
  //----------------------------------------------------------------------

  /**
   * Get the static polygon tree.
   */
  csPolygonTree* GetStaticTree () { return static_tree; }

  /**
   * Call this function to generate a polygon tree for this csThing.
   * This might make drawing more efficient because
   * this thing can then be drawn using Z-fill instead of Z-buffer.
   * Also the c-buffer requires a tree of this kind.
   * If 'octree' is true this function will create an octree with mini-bsp
   * trees instead of a BSP tree alone.
   */
  void BuildStaticTree (int mode = BSP_MINIMIZE_SPLITS, bool octree = false);

  /// Register a visibility object with this culler.
  void RegisterVisObject (iVisibilityObject* visobj);
  /// Unregister a visibility object with this culler.
  void UnregisterVisObject (iVisibilityObject* visobj);

  /**
   * Do the visibility test from a given viewpoint. This will first
   * clear the visible flag on all registered objects and then it will
   * mark all visible objects.
   */
  bool VisTest (iRenderView* irview);

  //----------------------------------------------------------------------
  // Shadow System
  //----------------------------------------------------------------------

  /// Register a shadow receiver.
  void RegisterShadowReceiver (iShadowReceiver* receiver);
  /// Unregister a shadow receiver.
  void UnregisterShadowReceiver (iShadowReceiver* receiver);

  /**
   * Start casting shadows from a given point in space.
   */
  void CastShadows (iFrustumView* fview);

  //----------------------------------------------------------------------
  // Drawing
  //----------------------------------------------------------------------
  
  /**
   * Draw this thing given a view and transformation.
   */
  bool Draw (iRenderView* rview, iMovable* movable);

  /**
   * Draw all curves in this thing given a view and transformation.
   */
  bool DrawCurves (iRenderView* rview, iMovable* movable);

  /**
   * Draw this thing as a fog volume (only when fog is enabled for
   * this thing).
   */
  bool DrawFoggy (iRenderView* rview, iMovable* movable);

  //----------------------------------------------------------------------
  // Lighting
  //----------------------------------------------------------------------
  
  /**
   * Prepare the lightmaps for all polys so that they are suitable
   * for the 3D rasterizer.
   */
  void CreateLightMaps (iGraphics3D* g3d);

  /**
   * Init the lightmaps for all polygons in this thing.
   */
  void InitLightMaps (bool do_cache = true);

  /**
   * Cache the lightmaps for all polygons in this thing.
   */
  void CacheLightMaps ();

  /// Mark this thing as visible.
  void MarkVisible () { is_visible = true; }

  /// Mark this thing as invisible.
  void MarkInvisible () { is_visible = false; }

  /// Return if this thing is visible.
  bool IsVisible () { return is_visible; }

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
   * happened.
   */
  csPolygon3D* IntersectSegment (const csVector3& start, 
                                       const csVector3& end, csVector3& isect,
				       float* pr = NULL);

  /**
   * Check frustum visibility on this thing.
   */
  void RealCheckFrustum (csFrustumView& lview);

  /**
   * Check frustum visibility on this thing.
   * First initialize the 2D culler cube.
   */
  void CheckFrustum (csFrustumView& lview);

  /**
   * Return a list of shadow frustums which extend from
   * this thing. The origin is the position of the light.
   * Note that this function uses camera space coordinates and
   * thus assumes that this thing is transformed to the
   * origin of the light.
   */
  void AppendShadows (iShadowBlockList* shadows, csVector3& origin);

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
   * Get the movable instance for this thing.
   * It is very important to call GetMovable().UpdateMove()
   * after doing any kind of modification to this movable
   * to make sure that internal data structures are
   * correctly updated.
   */
  csMovable& GetMovable () { return movable; }

  /**
   * Control how this thing will be moved.
   * There are currently three options.
   * <ul>
   *   <li>CS_THING_MOVE_NEVER: this option is set for a thing that cannot
   *       move at all. In this case the movable will be ignored and only
   *       hard transforms can be used to move a thing with this flag. This
   *       setting is both efficient for memory (object space coordinates are
   *       equal to world space coordinates so only one array is kept) and
   *       render speed (only the camera transform is needed). This option
   *       is very useful for static geometry like walls.
   *       This option is default.
   *   <li>CS_THING_MOVE_OCCASIONAL: this option is set for a thing that
   *       is movable but doesn't move all the time usually. Setting this
   *       option means that the world space vertices will be cached (taking
   *       up more memory that way) but the coordinates will be recalculated
   *       only at rendertime (and cached at that time). This option has
   *       the same speed efficiency as MOVE_NEVER when the object doesn't
   *       move but more memory is used as all the vertices are duplicated.
   *       Use this option for geometry that is not too big (in number of
   *       vertices) and only moves occasionally like doors of elevators.
   *   <li>CS_THING_MOVE_OFTEN: this option is set for a thing that moves
   *       very often (i.e. almost every frame). Setting this option means
   *       that the object->world and camera transformations will be combined
   *       at render time. It has the same memory efficiency as MOVE_NEVER
   *       but the transforms need to be combined every frame (if the object
   *       is visible). Use this option for geometry that moves a lot. Also
   *       very useful for objects that often move and have lots of vertices
   *       since in that case combining the transforms ones is a lot more
   *       efficient than doing two transforms on every vertex.
   * </ul>
   */
  void SetMovingOption (int opt);

  /**
   * Get the moving option.
   */
  int GetMovingOption () { return cfg_moving; }

  /**
   * Return true if this thing is a sky object.
   */
  bool IsSky () { return is_sky; }

  /**
   * Return true if this thing is a template.
   */
  bool IsTemplate () { return is_template; }

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

  /// Get the engine for this thing.
  csEngine* GetEngine () { return engine; }

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

  CSOBJTYPE;
  DECLARE_IBASE_EXT (csPObject);

  //------------------------- iThing interface -------------------------------
  struct eiThing : public iThing
  {
    DECLARE_EMBEDDED_IBASE (csThing);
 
    virtual csThing *GetPrivateObject () { return scfParent; }
    virtual const char *GetName () const { return scfParent->GetName (); }
    virtual void SetName (const char *iName) { scfParent->SetName (iName); }
    virtual void CompressVertices () { scfParent->CompressVertices(); }
    virtual int GetPolygonCount () { return scfParent->polygons.Length (); }
    virtual iPolygon3D *GetPolygon (int idx);
    virtual iPolygon3D *CreatePolygon (const char *iName);
    virtual int GetVertexCount () { return scfParent->num_vertices; }
    virtual csVector3 &GetVertex (int idx) { return scfParent->obj_verts [idx]; }
    virtual csVector3 &GetVertexW (int idx) { return scfParent->wor_verts [idx]; }
    virtual csVector3 &GetVertexC (int idx) { return scfParent->cam_verts [idx]; }
    virtual int CreateVertex (csVector3 &iVertex)
    { return scfParent->AddVertex (iVertex.x, iVertex.y, iVertex.z); }
    virtual bool CreateKey (const char *iName, const char *iValue);
    virtual iMovable* GetMovable () { return &scfParent->GetMovable ().scfiMovable; }
  } scfiThing;
  friend struct eiThing;
 
  //-------------------- iPolygonMesh interface implementation ---------------
  struct PolyMesh : public iPolygonMesh
  {
    DECLARE_EMBEDDED_IBASE (csThing);

    virtual int GetNumVertices () { return scfParent->GetNumVertices (); }
    virtual csVector3* GetVertices () { return scfParent->wor_verts; }
    virtual int GetNumPolygons ()
    {
      GetPolygons ();	// To make sure our count is ok.
      return num;
    }
    virtual csMeshedPolygon* GetPolygons ();

    PolyMesh () { polygons = NULL; }
    virtual ~PolyMesh () { delete[] polygons; }

    csMeshedPolygon* polygons;
    int num;
  } scfiPolygonMesh;
  friend struct PolyMesh;
 
  //-------------------- iVisibilityCuller interface implementation ----------
  struct VisCull : public iVisibilityCuller
  {
    DECLARE_EMBEDDED_IBASE (csThing);
    virtual void RegisterVisObject (iVisibilityObject* visobj)
    {
      scfParent->RegisterVisObject (visobj);
    }
    virtual void UnregisterVisObject (iVisibilityObject* visobj)
    {
      scfParent->UnregisterVisObject (visobj);
    }
    virtual bool VisTest (iRenderView* irview)
    {
      return scfParent->VisTest (irview);
    }
    virtual bool SupportsShadowCasting ()
    {
      return true;
    }
    virtual void RegisterShadowReceiver (iShadowReceiver* receiver)
    {
      scfParent->RegisterShadowReceiver (receiver);
    }
    virtual void UnregisterShadowReceiver (iShadowReceiver* receiver)
    {
      scfParent->UnregisterShadowReceiver (receiver);
    }
    virtual void CastShadows (iFrustumView* fview)
    {
      scfParent->CastShadows (fview);
    }
  } scfiVisibilityCuller;
  friend struct VisCull;

  //-------------------- iMeshObject interface implementation ----------
  struct MeshObject : public iMeshObject
  {
    DECLARE_EMBEDDED_IBASE (csThing);
    virtual iMeshObjectFactory* GetFactory ();
    virtual bool DrawTest (iRenderView* /*rview*/, iMovable* /*movable*/)
    {
      //@@@ For now!
      return true;
    }
    virtual void UpdateLighting (iLight** /*lights*/, int /*num_lights*/,
      	iMovable* /*movable*/) { }
    virtual bool Draw (iRenderView* rview, iMovable* movable)
    {
      return scfParent->Draw (rview, movable);
    }
    virtual void SetVisibleCallback (csMeshCallback* /*cb*/, void* /*cbData*/) { }
    virtual csMeshCallback* GetVisibleCallback () { return NULL; }
    virtual void GetObjectBoundingBox (csBox3& /*bbox*/, int /*type = CS_BBOX_NORMAL*/)
    {
    }
    virtual csVector3 GetRadius () { return csVector3 (0); }
    virtual void NextFrame (cs_time /*current_time*/) { }
    virtual bool WantToDie () { return false; }
    virtual void HardTransform (const csReversibleTransform& t)
    {
      scfParent->HardTransform (t);
    }
    virtual bool SupportsHardTransform () { return true; }
    virtual bool HitBeamObject (const csVector3& /*start*/, const csVector3& /*end*/,
  	csVector3& /*isect*/, float* /*pr*/) { return false; }
    virtual long GetShapeNumber () { return 0; /*@@@*/ }
  } scfiMeshObject;
  friend struct MeshObject;

  //-------------------- iMeshObjectFactory interface implementation ---------
  struct MeshObjectFactory : public iMeshObjectFactory
  {
    DECLARE_EMBEDDED_IBASE (csThing);
    virtual iMeshObject* NewInstance ();
    virtual void HardTransform (const csReversibleTransform& t)
    {
      scfParent->HardTransform (t);
    }
    virtual bool SupportsHardTransform () { return true; }
  } scfiMeshObjectFactory;
  friend struct MeshObjectFactory;

  //-------------------- iVisibilityObject interface implementation ----------
  struct VisObject : public iVisibilityObject
  {
    DECLARE_EMBEDDED_IBASE (csThing);
    virtual iMovable* GetMovable () { return &scfParent->GetMovable ().scfiMovable; }
    virtual long GetShapeNumber () { return scfParent->scfiMeshObject.GetShapeNumber (); }
    virtual void GetBoundingBox (csBox3& bbox)
    {
      scfParent->GetBoundingBox (bbox);
    }
    virtual void MarkVisible () { scfParent->MarkVisible (); }
    virtual void MarkInvisible () { scfParent->MarkInvisible (); }
    virtual bool IsVisible () { return scfParent->IsVisible (); }
  } scfiVisibilityObject;
  friend struct VisObject;
};

#endif // __CS_THING_H__
