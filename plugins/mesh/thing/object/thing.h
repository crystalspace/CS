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

#include "csgeom/csrect.h"
#include "csgeom/objmodel.h"
#include "csgeom/pmtools.h"
#include "csgeom/subrec2.h"
#include "csgeom/transfrm.h"
#include "csgfx/memimage.h"
#include "csgfx/shadervar.h"
#include "csutil/array.h"
#include "csutil/blockallocator.h"
#include "csutil/cscolor.h"
#include "csutil/csobject.h"
#include "csutil/flags.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/nobjvec.h"
#include "csutil/parray.h"
#include "csutil/refarr.h"
#include "csutil/util.h"
#include "csutil/weakref.h"
#include "csutil/leakguard.h"
#include "cstool/rendermeshholder.h"
#include "iengine/mesh.h"
#include "iengine/rview.h"
#include "iengine/shadcast.h"
#include "imesh/lighting.h"
#include "imesh/object.h"
#include "imesh/thing.h"
#include "iutil/comp.h"
#include "iutil/config.h"
#include "iutil/dbghelp.h"
#include "iutil/eventh.h"
#include "ivideo/shader/shader.h"
#include "ivideo/txtmgr.h"
#include "lghtmap.h"
#include "parrays.h"
#include "polygon.h"

class csThing;
class csThingStatic;
class csThingObjectType;
class csLightPatchPool;
class csPolyTexLightMap;
struct iShadowBlockList;
struct csVisObjInfo;
struct iGraphics3D;
struct iRenderView;
struct iMovable;
struct iFrustumView;
struct iMaterialWrapper;
struct iPolygonBuffer;

/**
 * A structure used to replace materials.
 */
struct RepMaterial
{
  iMaterialWrapper* old_mat;
  iMaterialWrapper* new_mat;
  RepMaterial (iMaterialWrapper* o, iMaterialWrapper* n) :
  	old_mat (o), new_mat (n) { }
};

/**
 * A helper class for iPolygonMesh implementations used by csThing.
 */
class PolyMeshHelper : public iPolygonMesh
{
public:
  SCF_DECLARE_IBASE;

  /**
   * Make a polygon mesh helper which will accept polygons which match
   * with the given flag (one of CS_POLY_COLLDET or CS_POLY_VISCULL).
   */
  PolyMeshHelper (uint32 flag) :
  	polygons (0), vertices (0), poly_flag (flag), triangles (0),
	locked (0)
  {
    SCF_CONSTRUCT_IBASE (0);
  }
  virtual ~PolyMeshHelper ()
  {
    SCF_DESTRUCT_IBASE ();
    Cleanup ();
  }

  void Setup ();
  void SetThing (csThingStatic* thing);

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
  virtual int GetTriangleCount ()
  {
    Triangulate ();
    return tri_count;
  }
  virtual csTriangle* GetTriangles ()
  {
    Triangulate ();
    return triangles;
  }
  virtual void Lock () { locked++; }
  virtual void Unlock ();

  virtual csFlags& GetFlags () { return flags;  }
  virtual uint32 GetChangeNumber() const { return 0; }

  void Cleanup ();
  void ForceCleanup ();

private:
  csThingStatic* thing;
  uint32 static_data_nr;	// To see if the static thing has changed.
  csMeshedPolygon* polygons;	// Array of polygons.
  csVector3* vertices;		// Array of vertices (points to obj_verts).
  int num_poly;			// Total number of polygons.
  int num_verts;		// Total number of vertices.
  uint32 poly_flag;		// Polygons must match with this flag.
  csFlags flags;
  csTriangle* triangles;
  int tri_count;
  int locked;

  void Triangulate ()
  {
    if (triangles) return;
    csPolygonMeshTools::Triangulate (this, triangles, tri_count);
  }
};

/**
 * The static data for a thing.
 */
class csThingStatic : public iThingFactoryState, public iMeshObjectFactory
{
public:
  CS_LEAKGUARD_DECLARE (csThingStatic);
  
  csRef<csThingObjectType> thing_type;
  /// Pointer to logical parent.
  iBase* logparent;
  iMeshObjectType* thingmesh_type;
  /// Set of flags
  csFlags flags;
  csFlags internalFlags;

  /// If true then this thing has been prepared (Prepare() function).
  bool IsPrepared() { return internalFlags.Check (1); }
  void SetPrepared (bool b) { internalFlags.SetBool (1, b); }
  bool IsLmPrepared() { return internalFlags.Check (2); }
  void SetLmPrepared (bool b) { internalFlags.SetBool (2, b); }
  /// Smooth flag
  bool IsSmoothed() { return internalFlags.Check (4); }
  void SetSmoothed (bool b) { internalFlags.SetBool (4, b); }
  /// If true then the bounding box in object space is valid.
  bool IsObjBboxValid() { return internalFlags.Check (8); }
  void SetObjBboxValid (bool b) { internalFlags.SetBool (8, b); }

  /// Number of vertices
  int num_vertices;
  /// Maximal number of vertices
  int max_vertices;
  /// Vertices in object space.
  csVector3* obj_verts;
  /// Normals in object space
  csVector3* obj_normals;

  /// Last used range.
  csPolygonRange last_range;
  /**
   * Function to calculate real start and end polygon indices based
   * on last_range and the given input range.
   */
  void GetRealRange (const csPolygonRange& requested_range, int& start,
  	int& end);
  /**
   * Function to calculate real individual polygon index.
   */
  int GetRealIndex (int requested_index) const;

  /// Bounding box in object space.
  csBox3 obj_bbox;

  /// Radius of object in object space.
  csVector3 obj_radius;
  /// Full radius of object in object space.
  float max_obj_radius;

  /// Static polys which share the same material
  struct csStaticPolyGroup
  {
    iMaterialWrapper* material;
    csArray<int> polys;

    int numLitPolys;
    int totalLumels;
  };

  struct StaticSuperLM;

  /**
   * Static polys which share the same material and fit on the same SLM
   * template.
   */
  struct csStaticLitPolyGroup : public csStaticPolyGroup
  {
    csArray<csRect> lmRects;
    StaticSuperLM* staticSLM;
  };

  /// SLM template
  struct StaticSuperLM
  {
    int width, height;
    csSubRectangles2* rects;
    int freeLumels;

    StaticSuperLM (int w, int h) : width(w), height(h)
    {
      rects = 0;
      freeLumels = width * height;
    }
    ~StaticSuperLM()
    {
      delete rects;
    }

    csSubRectangles2* GetRects ()
    {
      if (rects == 0)
      {
	rects = new csSubRectangles2 (csRect (0, 0, width, height));
      }
      return rects;
    }

    void Grow (int newWidth, int newHeight)
    {
      int usedLumels = (width * height) - freeLumels;

      width = newWidth; height = newHeight;
      if (rects != 0)
	rects->Grow (width, height);

      freeLumels = (width * height) - usedLumels;
    }
  };

  /// The array of static polygon data (csPolygon3DStatic).
  csPolygonStaticArray static_polygons;
  csPDelArray<csStaticLitPolyGroup> litPolys;
  csPDelArray<csStaticPolyGroup> unlitPolys;
  csArray<StaticSuperLM*> superLMs;

  /** 
   * Used to verify whether poly-specific renderbuffers have the right
   * properties.
   */
  csUserRenderBufferManager polyBufferTemplates;

  /**
   * This field describes how the light hitting polygons of this thing is
   * affected by the angle by which the beam hits the polygon. If this value is
   * equal to -1 (default) then the global csPolyTexture::cfg_cosinus_factor
   * will be used.
   */
  float cosinus_factor;

  csWeakRef<iGraphics3D> r3d;

  csRefArray<iPolygonRenderer> polyRenderers;

  static csStringID texLightmapName;

public:
  csThingStatic (iBase* parent, csThingObjectType* thing_type);
  virtual ~csThingStatic ();

  /**
   * Prepare the thing for use. This function MUST be called
   * AFTER the texture manager has been prepared. This function
   * is normally called by csEngine::Prepare() so you only need
   * to worry about this function when you add sectors or things
   * later.
   */
  void Prepare (iBase* thing_logparent);
  /**
   * Sets up a layout of lightmaps on a super lightmap that all instances
   * of this factory can reuse.
   */
  void PrepareLMLayout ();
  /// Do the actual distribution.
  void DistributePolyLMs (const csStaticPolyGroup& inputPolys,
    csPDelArray<csStaticLitPolyGroup>& outputPolys,
    csStaticPolyGroup* rejectedPolys);
  /// Delete LM distro information. Needed when adding/removing polys.
  void UnprepareLMLayout ();

  /// Calculates the interpolated normals of all vertices
  void CalculateNormals ();

  /**
   * Get the static data number.
   */
  uint32 GetStaticDataNumber () const
  {
    return scfiObjectModel.GetShapeNumber ();
  }

  /// Get the specified polygon from this set.
  csPolygon3DStatic *GetPolygon3DStatic (int idx)
  { return static_polygons.Get (idx); }

  /// Clone this static data in a separate instance.
  csPtr<csThingStatic> CloneStatic ();

  /**
   * Get the bounding box in object space for this polygon set.
   * This is calculated based on the oriented bounding box.
   */
  void GetBoundingBox (csBox3& box);

  /**
   * Set the bounding box in object space for this polygon set.
   */
  void SetBoundingBox (const csBox3& box);

  /**
   * Get the radius in object space for this polygon set.
   */
  void GetRadius (csVector3& rad, csVector3& cent);

  //----------------------------------------------------------------------
  // Vertex handling functions
  //----------------------------------------------------------------------

  /// Just add a new vertex to the thing.
  int AddVertex (const csVector3& v) { return AddVertex (v.x, v.y, v.z); }

  /// Just add a new vertex to the thing.
  int AddVertex (float x, float y, float z);

  virtual int CreateVertex (const csVector3 &iVertex)
  { return AddVertex (iVertex.x, iVertex.y, iVertex.z); }

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
  virtual void CompressVertices ();

  /**
   * Optimize the vertex table so that all unused vertices are deleted.
   * Note that calling this function will make the camera vertex array
   * invalid.
   */
  void RemoveUnusedVertices ();

  /// Change a vertex.
  virtual void SetVertex (int idx, const csVector3& vt);

  /// Delete a vertex.
  virtual void DeleteVertex (int idx);

  /// Delete a range of vertices.
  virtual void DeleteVertices (int from, int to);

  /// Return the object space vector for the vertex.
  const csVector3& Vobj (int idx) const { return obj_verts[idx]; }

  /// Return the number of vertices.
  virtual int GetVertexCount () const { return num_vertices; }

  virtual const csVector3 &GetVertex (int i) const
  { return obj_verts[i]; }
  virtual const csVector3* GetVertices () const
  { return obj_verts; }

  /// Add a polygon to this thing and return index.
  int AddPolygon (csPolygon3DStatic* spoly);

  /**
   * Intersect object-space segment with polygons of this set. Return
   * polygon index it intersects with (or -1) and the intersection point
   * in object coordinates.<p>
   *
   * If 'pr' != 0 it will also return a value between 0 and 1
   * indicating where on the 'start'-'end' vector the intersection
   * happened.
   */
  int IntersectSegmentIndex (
    const csVector3 &start, const csVector3 &end,
    csVector3 &isect,
    float *pr);

  SCF_DECLARE_IBASE;

  virtual int GetPolygonCount () { return static_polygons.Length (); }
  virtual void RemovePolygon (int idx);
  virtual void RemovePolygons ();

  virtual void SetSmoothingFlag (bool smoothing) { SetSmoothed (smoothing); }
  virtual bool GetSmoothingFlag () { return IsSmoothed(); }
  virtual csVector3* GetNormals () { return obj_normals; }

  virtual float GetCosinusFactor () const { return cosinus_factor; }
  virtual void SetCosinusFactor (float c) { cosinus_factor = c; }

  virtual int FindPolygonByName (const char* name);
  virtual int AddEmptyPolygon ();
  virtual int AddTriangle (const csVector3& v1, const csVector3& v2,
  	const csVector3& v3);
  virtual int AddQuad (const csVector3& v1, const csVector3& v2,
  	const csVector3& v3, const csVector3& v4);
  virtual int AddPolygon (csVector3* vertices, int num);
  virtual int AddPolygon (int num, ...);
  virtual int AddOutsideBox (const csVector3& bmin, const csVector3& bmax);
  virtual int AddInsideBox (const csVector3& bmin, const csVector3& bmax);
  virtual void SetPolygonName (const csPolygonRange& range,
  	const char* name);
  virtual const char* GetPolygonName (int polygon_idx);
  virtual csPtr<iPolygonHandle> CreatePolygonHandle (int polygon_idx);
  virtual void SetPolygonMaterial (const csPolygonRange& range,
  	iMaterialWrapper* material);
  virtual iMaterialWrapper* GetPolygonMaterial (int polygon_idx);
  virtual void AddPolygonVertex (const csPolygonRange& range,
  	const csVector3& vt);
  virtual void AddPolygonVertex (const csPolygonRange& range, int vt);
  virtual void SetPolygonVertexIndices (const csPolygonRange& range,
  	int num, int* indices);
  virtual int GetPolygonVertexCount (int polygon_idx);
  virtual const csVector3& GetPolygonVertex (int polygon_idx, int vertex_idx);
  virtual int* GetPolygonVertexIndices (int polygon_idx);
  virtual bool SetPolygonTextureMapping (const csPolygonRange& range,
  	const csMatrix3& m, const csVector3& v);
  virtual bool SetPolygonTextureMapping (const csPolygonRange& range,
  	const csVector2& uv1, const csVector2& uv2, const csVector2& uv3);
  virtual bool SetPolygonTextureMapping (const csPolygonRange& range,
  	const csVector3& p1, const csVector2& uv1,
  	const csVector3& p2, const csVector2& uv2,
  	const csVector3& p3, const csVector2& uv3);
  virtual bool SetPolygonTextureMapping (const csPolygonRange& range,
  	const csVector3& v_orig, const csVector3& v1, float len1);
  virtual bool SetPolygonTextureMapping (const csPolygonRange& range,
  	const csVector3& v_orig,
	const csVector3& v1, float len1,
	const csVector3& v2, float len2);
  virtual bool SetPolygonTextureMapping (const csPolygonRange& range,
  	float len1);
  virtual void GetPolygonTextureMapping (int polygon_idx,
  	csMatrix3& m, csVector3& v);
  virtual void SetPolygonTextureMappingEnabled (const csPolygonRange& range,
  	bool enabled);
  virtual bool IsPolygonTextureMappingEnabled (int polygon_idx) const;
  virtual bool PointOnPolygon (int polygon_idx, const csVector3& v);
  virtual void SetPolygonFlags (const csPolygonRange& range, uint32 flags);
  virtual void SetPolygonFlags (const csPolygonRange& range, uint32 mask,
  	uint32 flags);
  virtual void ResetPolygonFlags (const csPolygonRange& range, uint32 flags);
  virtual csFlags& GetPolygonFlags (int polygon_idx);
  virtual const csPlane3& GetPolygonObjectPlane (int polygon_idx);
  virtual bool IsPolygonTransparent (int polygon_idx);

  virtual bool AddPolygonRenderBuffer (int polygon_idx, const char* name,
    iRenderBuffer* buffer);

  //-------------------- iMeshObjectFactory interface implementation ----------

  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> NewInstance ();
  virtual csPtr<iMeshObjectFactory> Clone ();
  virtual void HardTransform (const csReversibleTransform& t);
  virtual bool SupportsHardTransform () const { return true; }
  virtual void SetLogicalParent (iBase* lp) { logparent = lp; }
  virtual iMeshObjectType* GetMeshObjectType () const { return thingmesh_type; }
  virtual iBase* GetLogicalParent () const { return logparent; }

  //-------------------- iPolygonMesh interface implementation ----------------
  PolyMeshHelper scfiPolygonMesh;
  //-------------------- CD iPolygonMesh implementation -----------------------
  PolyMeshHelper scfiPolygonMeshCD;
  //-------------------- Lower detail iPolygonMesh implementation -------------
  PolyMeshHelper scfiPolygonMeshLOD;

  //-------------------- iObjectModel implementation --------------------------
  class ObjectModel : public csObjectModel
  {
    SCF_DECLARE_EMBEDDED_IBASE (csThingStatic);
    virtual void GetObjectBoundingBox (csBox3& bbox)
    {
      scfParent->GetBoundingBox (bbox);
    }
    virtual void SetObjectBoundingBox (const csBox3& bbox)
    {
      scfParent->SetBoundingBox (bbox);
    }
    virtual void GetRadius (csVector3& rad, csVector3& cent)
    {
      scfParent->GetRadius (rad, cent);
    }
  } scfiObjectModel;
  friend class ObjectModel;

  void FillRenderMeshes (csDirtyAccessArray<csRenderMesh*>& rmeshes,
    const csArray<RepMaterial>& repMaterials, uint mixmode);

  virtual iObjectModel* GetObjectModel () { return &scfiObjectModel; }
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
 * oriented such that they are visible from the outside.
 */
class csThing : public iBase
{
  friend class PolyMeshHelper;
  friend class csPolygon3D;

private:
  /// Static data for this thing.
  csRef<csThingStatic> static_data;

  /// ID for this thing (will be >0).
  unsigned int thing_id;
  /// Last used ID.
  static int last_thing_id;
  /// Current visibility number.
  uint32 current_visnr;

  /**
   * Vertices in world space.
   * It is possible that this array is equal to obj_verts. In that
   * case this is a thing that never moves.
   */
  csVector3* wor_verts;

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

  /// The array of dynamic polygon data (csPolygon3D).
  csPolygonArray polygons;
  /// World space planes (if movable is not identity).
  csPlane3* polygon_world_planes;
  size_t polygon_world_planes_num;

  /// Optional array of materials to replace.
  csArray<RepMaterial> replace_materials;

  /**
   * An array of materials that must be visited before use.
   */
  csArray<iMaterialWrapper*> materials_to_visit;

  /**
   * Bounding box in world space.
   * This is a cache for GetBoundingBox(iMovable,csBox3) which
   * will recalculate this if the movable changes (by using movablenr).
   */
  csBox3 wor_bbox;
  /// Last movable number that was used for the bounding box in world space.
  long wor_bbox_movablenr;

  /// Dynamic ambient light assigned to this thing.
  csColor dynamic_ambient;
  /**
   * Version number for dynamic/pseudo-dynamic light changes
   * and also for ambient.
   */
  uint32 light_version;

  /// Pointer to logical parent.
  iBase* logparent;

  /**
   * This number is compared with the static_data_nr in the static data to
   * see if static data has changed and this thing needs to updated local
   * data
   */
  int32 static_data_nr;

  float current_lod;

  csFlags flags;
  csFlags internalFlags;

  /// If true then this thing has been prepared (Prepare() function).
  bool IsPrepared() { return internalFlags.Check (1); }
  void SetPrepared (bool b) { internalFlags.SetBool (1, b); }
#ifdef __USE_MATERIALS_REPLACEMENT__
  /// If true then a material has been added/removed from
  /// the replace_materials array, and the polygon
  /// buffer of the thing needs to be recalculated.
  bool IsReplaceMaterialChanged() { return internalFlags.Check (2); }
  void SetReplaceMaterialChanged (bool b) { internalFlags.SetBool (2, b); }
#endif
  bool IsLmPrepared() { return internalFlags.Check (4); }
  void SetLmPrepared (bool b) { internalFlags.SetBool (4, b); }
  bool IsLmDirty() { return internalFlags.Check (8); }
  void SetLmDirty (bool b) { internalFlags.SetBool (8, b); }

  csRenderMeshHolderMultiple rmHolder;

  void PrepareRenderMeshes (csDirtyAccessArray<csRenderMesh*>& renderMeshes);

  // Mixmode for rendering.
  uint mixmode;

  /// A group of polys that share the same material.
  struct csPolyGroup
  {
    iMaterialWrapper* material;
    csArray<size_t> polys;
  };

  /// Polys with the same material and the same SLM
  struct csLitPolyGroup : public csPolyGroup
  {
    csRefArray<iRendererLightmap> lightmaps;
    csRef<iSuperLightmap> SLM;
  };

  csPDelArray<csLitPolyGroup> litPolys;
  csPDelArray<csPolyGroup> unlitPolys;

  void PreparePolygons ();
  void PrepareLMs ();
  void ClearLMs ();
  void UpdateDirtyLMs ();

private:
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
   * 't' is the movable transform.
   */
  void DrawPolygonArrayDPM (iRenderView* rview, iMovable* movable,
  	csZBufMode zMode);

  /// Generate a cachename based on geometry.
  char* GenerateCacheName ();

public:
  CS_LEAKGUARD_DECLARE (csThing);

  /// Option variable: quality for lightmap calculation.
  static int lightmap_quality;

  /// Option variable: enable/disable lightmapping.
  static bool lightmap_enabled;

  /**
   * Create an empty thing.
   */
  csThing (iBase* parent, csThingStatic* static_data);

  /// Destructor.
  virtual ~csThing ();

  /// Get the pointer to the static data.
  csThingStatic* GetStaticData () { return static_data; }

  /// Get the cached movable.
  iMovable* GetCachedMovable () const { return cached_movable; }

  //----------------------------------------------------------------------
  // Vertex handling functions
  //----------------------------------------------------------------------

  /// Make sure the world vertices are up-to-date.
  void WorUpdate ();

  /**
   * Return the world space vector for the vertex.
   * Make sure you recently called WorUpdate(). Otherwise it is
   * possible that this coordinate will not be valid.
   */
  const csVector3& Vwor (int idx) const { return wor_verts[idx]; }

  /**
   * Get the world plane for a polygon. This function does NOT
   * check if the world plane is valid. Call WorUpdate() to make sure
   * it is valid.
   */
  const csPlane3& GetPolygonWorldPlaneNoCheck (int polygon_idx) const;

  //----------------------------------------------------------------------
  // Polygon handling functions
  //----------------------------------------------------------------------

  /// Get the number of polygons in this thing.
  int GetPolygonCount ()
  { return polygons.Length (); }

  /// Get the specified polygon from this set.
  csPolygon3DStatic *GetPolygon3DStatic (int idx)
  { return static_data->GetPolygon3DStatic (idx); }

  /// Get the specified polygon from this set.
  csPolygon3D *GetPolygon3D (int idx)
  { return &polygons.Get (idx); }

  /// Get the named polygon from this set.
  csPolygon3D *GetPolygon3D (const char* name);

  /// Get the entire array of polygons.
  csPolygonArray& GetPolygonArray () { return polygons; }

  /// Remove a single polygon.
  void RemovePolygon (int idx);

  /// Remove all polygons.
  void RemovePolygons ();

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

  /** Reset the prepare flag so that this Thing can be re-prepared.
   * Among other things this will allow cached lightmaps to be
   * recalculated.
   */
  void Unprepare ();

  /// Find the real material to use if it was replaced (or 0 if not).
  iMaterialWrapper* FindRealMaterial (iMaterialWrapper* old_mat);

  void ReplaceMaterial (iMaterialWrapper* oldmat, iMaterialWrapper* newmat);
  void ClearReplacedMaterials ();

  void InvalidateMaterialHandles ();

  void PrepareForUse ();

  //----------------------------------------------------------------------
  // Bounding information
  //----------------------------------------------------------------------

  /**
   * Get the bounding box for this object given some transformation (movable).
   */
  void GetBoundingBox (iMovable* movable, csBox3& box);

  /**
   * Get a write object for a vis culling system.
   */
  iPolygonMesh* GetWriteObject ();

  //----------------------------------------------------------------------
  // Lighting
  //----------------------------------------------------------------------

  /**
   * Init the lightmaps for all polygons in this thing.
   */
  void InitializeDefault (bool clear);

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
  	csVector3& isect, float* pr, int* polygon_idx = 0);

  //----------------------------------------------------------------------
  // Various
  //----------------------------------------------------------------------

  /**
   * Do a hardtransform. This will make a clone of the factory
   * to avoid other meshes using this factory to be hard transformed too.
   */
  void HardTransform (const csReversibleTransform& t);

  /**
   * Control how this thing will be moved.
   */
  void SetMovingOption (int opt);

  /**
   * Get the moving option.
   */
  int GetMovingOption () const { return cfg_moving; }

  /// Sets dynamic ambient light for this thing
  void SetDynamicAmbientLight(const csColor& color)
  {
      dynamic_ambient = color;
      light_version++;
      MarkLightmapsDirty ();
  }
  /// Gets dynamic ambient light for this thing
  const csColor& GetDynamicAmbientLight()
  {
      return dynamic_ambient;
  }

  /// Get light version.
  uint32 GetLightVersion() const
  { return light_version; }

  void LightChanged (iLight* light);
  void LightDisconnect (iLight* light);

  void SetMixMode (uint mode)
  {
    mixmode = mode;
  }
  uint GetMixMode () const
  {
    return mixmode;
  }

  csPtr<iPolygonHandle> CreatePolygonHandle (int polygon_idx);
  const csPlane3& GetPolygonWorldPlane (int polygon_idx);

  SCF_DECLARE_IBASE;

  //------------------------- iThingState interface -------------------------
  struct ThingState : public iThingState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csThing);
    virtual iThingFactoryState* GetFactory ()
    {
      return (iThingFactoryState*)(scfParent->static_data);
    }

    virtual const csVector3 &GetVertexW (int i) const
    { return scfParent->wor_verts[i]; }
    virtual const csVector3* GetVerticesW () const
    { return scfParent->wor_verts; }

    virtual int GetMovingOption () const
    { return scfParent->GetMovingOption (); }
    virtual void SetMovingOption (int opt)
    { scfParent->SetMovingOption (opt); }

    /// Prepare.
    virtual void Prepare ()
    { scfParent->PrepareForUse (); }

    /// Unprepare.
    virtual void Unprepare ()
    {
      scfParent->Unprepare ();
    }

    virtual void ReplaceMaterial (iMaterialWrapper* oldmat,
  	iMaterialWrapper* newmat)
    {
      scfParent->ReplaceMaterial (oldmat, newmat);
    }
    virtual void ClearReplacedMaterials ()
    {
      scfParent->ClearReplacedMaterials ();
    }
    virtual void SetMixMode (uint mode)
    {
      scfParent->SetMixMode (mode);
    }
    virtual uint GetMixMode () const
    {
      return scfParent->GetMixMode ();
    }
    virtual csPtr<iPolygonHandle> CreatePolygonHandle (int polygon_idx)
    {
      return scfParent->CreatePolygonHandle (polygon_idx);
    }
    virtual const csPlane3& GetPolygonWorldPlane (int polygon_idx)
    {
      return scfParent->GetPolygonWorldPlane (polygon_idx);
    }
  } scfiThingState;
  friend struct ThingState;

  //------------------------- iLightingInfo interface -------------------------
  /// iLightingInfo implementation.
  struct LightingInfo : public iLightingInfo
  {
    SCF_DECLARE_EMBEDDED_IBASE (csThing);
    virtual void InitializeDefault (bool clear)
    {
      scfParent->InitializeDefault (clear);
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
    virtual void LightChanged (iLight* light)
    { scfParent->LightChanged (light); }
    virtual void LightDisconnect (iLight* light)
    { scfParent->LightDisconnect (light); }
  } scfiLightingInfo;
  friend struct LightingInfo;

  //-------------------- iPolygonMesh interface implementation ----------------
  PolyMeshHelper scfiPolygonMesh;
  //-------------------- CD iPolygonMesh implementation -----------------------
  PolyMeshHelper scfiPolygonMeshCD;
  //-------------------- Lower detail iPolygonMesh implementation -------------
  PolyMeshHelper scfiPolygonMeshLOD;

  //-------------------- iShadowCaster interface implementation ---------------
  struct ShadowCaster : public iShadowCaster
  {
    SCF_DECLARE_EMBEDDED_IBASE (csThing);
    virtual void AppendShadows (iMovable* movable, iShadowBlockList* shadows,
    	const csVector3& origin)
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

  csRenderMesh **GetRenderMeshes (int &num, iRenderView* rview, 
    iMovable* movable, uint32 frustum_mask);

  //-------------------- iMeshObject interface implementation ----------
  struct MeshObject : public iMeshObject
  {
    SCF_DECLARE_EMBEDDED_IBASE (csThing);
    virtual iMeshObjectFactory* GetFactory () const;
    virtual csFlags& GetFlags () { return scfParent->flags; }
    virtual csPtr<iMeshObject> Clone () { return 0; }
    virtual csRenderMesh** GetRenderMeshes (int &n, iRenderView* rview, 
      iMovable* movable, uint32 frustum_mask)
    {
      return scfParent->GetRenderMeshes (n, rview, movable, frustum_mask);
    }
    virtual void SetVisibleCallback (iMeshObjectDrawCallback* /*cb*/) { }
    virtual iMeshObjectDrawCallback* GetVisibleCallback () const
    { return 0; }
    virtual void NextFrame (csTicks /*current_time*/,const csVector3& /*pos*/)
    { }
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
  	csVector3& isect, float* pr, int* polygon_idx = 0)
    {
      return scfParent->HitBeamObject (start, end, isect, pr, polygon_idx);
    }
    virtual void SetLogicalParent (iBase* lp) { scfParent->logparent = lp; }
    virtual iBase* GetLogicalParent () const { return scfParent->logparent; }
    virtual iObjectModel* GetObjectModel ()
    {
      return scfParent->static_data->GetObjectModel ();
    }
    virtual bool SetColor (const csColor&) { return false; }
    virtual bool GetColor (csColor&) const { return false; }
    virtual bool SetMaterialWrapper (iMaterialWrapper*) { return false; }
    virtual iMaterialWrapper* GetMaterialWrapper () const { return 0; }
    virtual void InvalidateMaterialHandles ()
    {
      scfParent->InvalidateMaterialHandles ();
    }
    /**
     * see imesh/object.h for specification. The default implementation
     * does nothing.
     */
    virtual void PositionChild (iMeshObject* child,csTicks current_time) { }
  } scfiMeshObject;
  friend struct MeshObject;
};

struct intar2 { int ar[2]; };
struct intar3 { int ar[3]; };
struct intar4 { int ar[4]; };
struct intar5 { int ar[5]; };
struct intar6 { int ar[6]; };
struct intar20 { int ar[20]; };
struct intar60 { int ar[60]; };

/**
 * Thing type. This is the plugin you have to use to create instances
 * of csThing.
 */
class csThingObjectType : public iMeshObjectType
{
public:
  iObjectRegistry* object_reg;
  static bool do_verbose;	// Verbose error reporting.
  iEngine* engine;
  /**
   * csThingObjectType must keep a reference to G3D because when polygons
   * are destructed they actually refer to G3D to clear the cache.
   */
  csWeakRef<iGraphics3D> G3D;
  /// An object pool for lightpatches.
  csLightPatchPool* lightpatch_pool;
  csRef<iStringSet> stringset;

  /**
   * Block allocators for various types of objects in thing.
   */
  csBlockAllocator<csPolygon3DStatic> blk_polygon3dstatic;
  csBlockAllocator<csPolyTextureMapping> blk_texturemapping;
  csBlockAllocator<csLightMap> blk_lightmap;
  csBlockAllocator<intar3> blk_polidx3;
  csBlockAllocator<intar4> blk_polidx4;
  csBlockAllocator<intar5>* blk_polidx5;
  csBlockAllocator<intar6>* blk_polidx6;
  csBlockAllocator<intar20>* blk_polidx20;
  csBlockAllocator<intar60>* blk_polidx60;

  csLightingScratchBuffer lightingScratch;

  int maxLightmapW, maxLightmapH;
  float maxSLMSpaceWaste;
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

  /// Execute a debug command.
  virtual bool DebugCommand (const char* cmd);

  /// iThingEnvironment implementation.
  struct eiThingEnvironment : public iThingEnvironment
  {
    SCF_DECLARE_EMBEDDED_IBASE(csThingObjectType);
    virtual void Clear ()
    {
      scfParent->Clear ();
    }
    virtual int GetLightmapCellSize () const
    {
      return csLightMap::lightcell_size;
    }
    virtual void SetLightmapCellSize (int size)
    {
      csLightMap::SetLightCellSize (size);
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

  /// iDebugHelper implementation
  struct eiDebugHelper : public iDebugHelper
  {
    SCF_DECLARE_EMBEDDED_IBASE(csThingObjectType);
    virtual int GetSupportedTests () const
    { return 0; }
    virtual csPtr<iString> UnitTest ()
    { return 0; }
    virtual csPtr<iString> StateTest ()
    { return 0; }
    virtual csTicks Benchmark (int num_iterations)
    { return 0; }
    virtual csPtr<iString> Dump ()
    { return 0; }
    virtual void Dump (iGraphics3D* g3d)
    { }
    virtual bool DebugCommand (const char* cmd)
    { return scfParent->DebugCommand (cmd); }
  } scfiDebugHelper;
};

#endif // __CS_THING_H__

