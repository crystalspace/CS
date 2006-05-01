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

#ifndef __CS_BEZIERMESH_H__
#define __CS_BEZIERMESH_H__

#include "csgeom/transfrm.h"
#include "carrays.h"
#include "csutil/array.h"
#include "csutil/cscolor.h"
#include "csutil/csobject.h"
#include "csutil/flags.h"
#include "csutil/nobjvec.h"
#include "csutil/refarr.h"
#include "csutil/scf_implementation.h"
#include "csutil/util.h"
#include "csutil/weakref.h"
#include "cstool/rendermeshholder.h"
#include "cstool/framedataholder.h"
#include "csgfx/shadervarcontext.h"
#include "igeom/polymesh.h"
#include "cstool/objmodel.h"
#include "csgeom/pmtools.h"
#include "iengine/mesh.h"
#include "iengine/rview.h"
#include "iengine/shadcast.h"
#include "imesh/bezier.h"
#include "imesh/object.h"
#include "imesh/lighting.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/pluginconfig.h"
#include "clightmap.h"
#include "curvebase.h"

struct iShadowBlockList;
struct csVisObjInfo;
struct iGraphics3D;
struct iRenderView;
struct iMovable;
struct iFrustumView;
struct iMaterialWrapper;

CS_PLUGIN_NAMESPACE_BEGIN(Bezier)
{

class csBezierMesh;
class csBezierMeshObjectType;
class csBezierLightPatchPool;

/**
 * A helper class for iPolygonMesh implementations used by csBezierMesh.
 */
class BezierPolyMeshHelper : public scfImplementation1<BezierPolyMeshHelper, 
                                                       iPolygonMesh>
{
public:
  /**
   * Make a polygon mesh helper which will accept polygons which match
   * with the given flag (one of CS_POLY_COLLDET or CS_POLY_VISCULL).
   */
  BezierPolyMeshHelper () : scfImplementationType (this), polygons (0), 
    vertices (0), triangles (0) { }
  virtual ~BezierPolyMeshHelper () { Cleanup (); }
  void Cleanup ();

  void Setup ();
  void SetThing (csBezierMesh* thing) { BezierPolyMeshHelper::thing = thing; }

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
    Setup ();
    Triangulate ();
    return tri_count;
  }
  virtual csTriangle* GetTriangles ()
  {
    Setup ();
    Triangulate ();
    return triangles;
  }

  virtual void Lock () { }
  virtual void Unlock () { }
  
  virtual csFlags& GetFlags () { return flags;  }
  virtual uint32 GetChangeNumber() const { return 0; }

private:
  csBezierMesh* thing;
  csMeshedPolygon* polygons;	// Array of polygons.
  csVector3* vertices;		// Array of vertices.
  int num_poly;			// Total number of polygons.
  int num_verts;		// Total number of vertices.
  csFlags flags;
  csTriangle* triangles;
  int tri_count;

  void Triangulate ()
  {
    if (triangles) return;
    csPolygonMeshTools::Triangulate (this, triangles, tri_count);
  }
};

/**
 * The static data for a bezier.
 */
class csBezierMeshStatic
{
public:
  csBezierMeshObjectType* thing_type;

  /// Bounding box in object space.
  csBox3 obj_bbox;

  /// If true then the bounding box in object space is valid.
  bool obj_bbox_valid;

  /// Radius of object in object space.
  float obj_radius;

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

  /// If true then this thing has been prepared (Prepare() function).
  bool prepared;
  /**
   * Static data identifier. Mesh instances can compare their copy of this
   * number to see if they need to update local data when the static
   * data has changed.
   */
  uint32 static_data_nr;

  iBezierFactoryState* factory_state;

public:
  csBezierMeshStatic (csBezierMeshObjectType* thing_type,
  	iBezierFactoryState* factory_state);
  ~csBezierMeshStatic ();

  /**
   * Prepare the thing for use. This function MUST be called
   * AFTER the texture manager has been prepared. This function
   * is normally called by csEngine::Prepare() so you only need
   * to worry about this function when you add sectors or things
   * later.
   */
  void Prepare ();

  /// Get the factory state for this static data. @@@
  iBezierFactoryState* GetFactoryState () { return factory_state; }

  /**
   * Called if static data in some polygon has changed.
   */
  void StaticDataChanged () { static_data_nr++; }

  /**
   * Get the static data number.
   */
  uint32 GetStaticDataNumber () const { return static_data_nr; }

  /// Get the number of curve vertices.
  int GetCurveVertexCount () const { return num_curve_vertices; }

  /// Get the specified curve vertex.
  csVector3& GetCurveVertex (int i) const { return curve_vertices[i]; }

  /// Get the curve vertices.
  csVector3* GetCurveVertices () const { return curve_vertices; }

  /// Get the specified curve texture coordinate (texel).
  csVector2& GetCurveTexel (int i) const { return curve_texels[i]; }

  /// Get the curve scale.
  float GetCurvesScale () const { return curves_scale; }

  /// Set the curve scale.
  void SetCurvesScale (float f) { curves_scale = f; }

  /// Get the curves center.
  const csVector3& GetCurvesCenter () const { return curves_center; }

  /// Set the curves center.
  void SetCurvesCenter (csVector3& v) { curves_center = v; }
};

/* The inheritance of csBezierMesh is semi-odd since it exhibits both
 * iMeshObjectFactory and iMeshObject, but both have methods with the
 * same name and different return values and even methods with the same
 * name and different semantics. Isolate those conflicting interfaces into
 * two classes and use multi-inheritance to "pull" those implementations into
 * csBezierMesh.
 */

class csBezierMesh1 : public scfImplementationExt1<csBezierMesh1,
                                                   csObjectModel,
                                                   iMeshObjectFactory>
{
protected:
   csFlags factory_flags;
  /// Pointer to logical parent.
  iMeshFactoryWrapper* logparent_factory;
public:
  csBezierMesh1 (iBase* parent) : scfImplementationType (this, parent) {}
  /** \name iMeshObjectFactory interface implementation
   * @{ */
  virtual csFlags& GetFlags () { return factory_flags; }
  virtual csPtr<iMeshObjectFactory> Clone () { return 0; }
  virtual void SetMeshFactoryWrapper (iMeshFactoryWrapper* lp)
  { logparent_factory = lp; }
  virtual iMeshFactoryWrapper* GetMeshFactoryWrapper () const
  { return logparent_factory; }
  /** @} */
};

class csBezierMesh2 : public virtual iMeshObject
{
protected:
  csFlags object_flags;
public:
  virtual csPtr<iMeshObject> Clone () { return 0; }
  virtual csFlags& GetFlags () { return object_flags; }
};

/**
 * A bezier is a set of bezier curves.
 */
class csBezierMesh : public scfImplementationExt6<csBezierMesh,
                                                  csBezierMesh1,
                                                  iBezierFactoryState,
                                                  iBezierState,
                                                  iLightingInfo,
                                                  iShadowCaster,
                                                  iShadowReceiver,
                                                  scfFakeInterface<iMeshObject> >,
		     public csBezierMesh2
{
  friend class BezierPolyMeshHelper;

public:
  /**
   * Option variable: control how much the angle of the light with the polygon
   * it hits affects the final light value. Values ranges from -1 to 1.
   * With -1 the polygons will get no light at all. With 0 it will be perfect
   * cosine rule. With 1 the cosine is ignored and it will be like Crystal Space
   * was in the past. Note that changing this value at runtime only has an
   * effect on dynamic lights.
   */
  static float cfg_cosinus_factor;

private:
  /// Static data for this thing.
  csBezierMeshStatic* static_data;

  /// ID for this thing (will be >0).
  unsigned int thing_id;
  /// Last used ID.
  static int last_thing_id;
  /// Current visibility number.
  uint32 current_visnr;

  /**
   * This field describes how the light hitting polygons of this thing is
   * affected by the angle by which the beam hits the polygon. If this value is
   * equal to -1 (default) then the global csBezierMesh::cfg_cosinus_factor
   * will be used.
   */
  float cosinus_factor;

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

  /// The array of curves forming the outside of the set
  csCurvesArray curves;

  /**
   * If true the transforms of the curves are set up (in case
   * CS_BEZIERMESH_MOVE_NEVER is used).
   */
  bool curves_transf_ok;

  /**
   * Bounding box in world space.
   * This is a cache for GetBoundingBox(iMovable,csBox3) which
   * will recalculate this if the movable changes (by using movablenr).
   */
  csBox3 wor_bbox;
  /// Last movable number that was used for the bounding box in world space.
  long wor_bbox_movablenr;

  /**
   * Global sector wide dynamic ambient version.
   */
  uint dynamic_ambient_version;

  /**
   * Version number for dynamic/pseudo-dynamic light changes
   * and also for ambient.
   */
  uint32 light_version;

  /// Pointer to the Thing Template which it derived from.
  csBezierMesh* ParentTemplate;
  /// Pointer to logical parent.
  iMeshWrapper* logparent;
  /// Pointer to meshobjecttype.
  iMeshObjectType* beziermsh_type;

  /// If true then this thing has been prepared (Prepare() function).
  bool prepared;

  /// For clipping.
  int clip_portal, clip_plane, clip_z_plane;

  /**
   * This number is compared with the static_data_nr in the static data to
   * see if static data has changed and this thing needs to updated local
   * data
   */
  uint32 static_data_nr;

  float current_lod;
  uint32 current_features;

  csFrameDataHolder<csDirtyAccessArray<csRenderMesh*> > rmListHolder;
  csRenderMeshHolder rmHolder;

  struct PerFrameData
  {
    size_t vertCount;
    csRef<iRenderBuffer> vertices;
    csRef<iRenderBuffer> colors;
    csRef<iRenderBuffer> texcoords;
    csRef<iRenderBuffer> indices;
  };
  csFrameDataHolder<PerFrameData> frameBufferHolder;
  static csStringID vertex_name, texel_name, color_name, index_name;
private:
  /**
   * Invalidate a thing. This has to be called when new polygons are
   * added or removed.
   */
  void InvalidateThing ();

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

  /// Generate a cachename based on geometry.
  char* GenerateCacheName ();  

public:
  /**
   * Create an empty thing.
   */
  csBezierMesh (iBase* parent, csBezierMeshObjectType* thing_type);

  /// Destructor.
  virtual ~csBezierMesh ();

  /// Get the pointer to the static data.
  csBezierMeshStatic* GetStaticData () { return static_data; }

  //----------------------------------------------------------------------
  // Curve handling functions
  //----------------------------------------------------------------------

  /// Add a curve to this thing.
  void AddCurve (csCurve* curve);

  /// Get the number of curves in this thing.
  int GetCurveCount () const
  { return (int)curves.Length (); }

  /// Get the specified curve from this set.
  iCurve* GetCurve (int idx) const
  { return (iCurve*)curves.Get (idx); }

  /// Create a curve from a template.
  iCurve* CreateCurve ();

  /// Find a curve index.
  int FindCurveIndex (iCurve* curve) const;

  /// Delete a curve given an index.
  void RemoveCurve (int idx);

  /// Delete all curves.
  void RemoveCurves ();

  /// Get the named curve from this set.
  csCurve* GetCurve (char* name) const;

  /// Get the specified curve coordinate.
  void SetCurveVertex (int idx, const csVector3& vt);

  /// Set the specified curve texture coordinate (texel).
  void SetCurveTexel (int idx, const csVector2& vt);

  /// Clear the curve vertices.
  void ClearCurveVertices ();

  /// Add a curve vertex and return the index of the vertex.
  size_t AddCurveVertex (const csVector3& v, const csVector2& t);

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
  void Merge (csBezierMesh* other);

  /**
   * Add polygons and vertices from the specified thing (seen as template).
   */
  void MergeTemplate (iBezierFactoryState* tpl,
  	iMaterialWrapper* default_material = 0,
	csVector3* shift = 0, csMatrix3* transform = 0);

  /// Set parent template.
  void SetTemplate (csBezierMesh *t)
  { ParentTemplate = t; }

  /// Query parent template.
  csBezierMesh *GetTemplate () const
  { return ParentTemplate; }

  //----------------------------------------------------------------------
  // Bounding information
  //----------------------------------------------------------------------

  void WorUpdate ();

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
   * Set the bounding box in object space for this polygon set.
   */
  void SetBoundingBox (const csBox3& box);

  /**
   * Get the bounding box for this object given some transformation (movable).
   */
  void GetBoundingBox (iMovable* movable, csBox3& box);

  /**
   * Get the radius in object space for this polygon set.
   */
  void GetRadius (float& rad, csVector3& cent);

  /**
   * Get a write object for a vis culling system.
   */
  iPolygonMesh* GetWriteObject ();

  //----------------------------------------------------------------------
  // Drawing
  //----------------------------------------------------------------------

  csRenderMesh** GetRenderMeshes (int &n, iRenderView* rview, 
    iMovable* movable, uint32 frustum_mask);

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

  /// Get cosinus setting.
  float GetCosinusFactor () const { return cosinus_factor; }
  /// Set cosinus factor.
  void SetCosinusFactor (float c) { cosinus_factor = c; }

  //----------------------------------------------------------------------
  // Utility functions
  //----------------------------------------------------------------------

  /**
   * Check frustum visibility on this thing.
   * First initialize the 2D culler cube.
   */
  void CastShadows (iMovable* movable, iFrustumView* lview);

  /**
   * Append a list of shadow frustums which extend from
   * this thing. The origin is the position of the light.
   */
  void AppendShadows (iMovable* movable, iShadowBlockList* shadows,
  	const csVector3& origin);

  //----------------------------------------------------------------------
  // Transformation
  //----------------------------------------------------------------------

  /**
   * Do a hard transform of the object vertices.
   * This transformation and the original coordinates are not
   * remembered but the object space coordinates are directly
   * computed (world space coordinates are set to the object space
   * coordinates by this routine).
   */
  void HardTransform (const csReversibleTransform& t);

  //----------------------------------------------------------------------
  // Various
  //----------------------------------------------------------------------

  /// Get light version.
  uint32 GetLightVersion() const
  { return light_version; }

  void LightChanged (iLight* light);
  void LightDisconnect (iLight* light);
  void DisconnectAllLights ();

  csRef<BezierPolyMeshHelper> polygonMesh;
  csRef<BezierPolyMeshHelper> polygonMeshLOD;

  /** \name iBezierFactoryState implementation
   * @{ */
  virtual const csVector3& GetCurvesCenter () const
  { return static_data->curves_center; }
  virtual void SetCurvesCenter (const csVector3& cen)
  { static_data->curves_center = cen; }
  virtual float GetCurvesScale () const
  { return static_data->curves_scale; }
  virtual void SetCurvesScale (float scale)
  { static_data->curves_scale = scale; }
  virtual int GetCurveVertexCount () const
  { return static_data->GetCurveVertexCount (); }
  virtual csVector3& GetCurveVertex (int i) const
  { return static_data->GetCurveVertex (i); }
  virtual csVector3* GetCurveVertices () const
  { return static_data->GetCurveVertices (); }
  virtual csVector2& GetCurveTexel (int i) const
  { return static_data->GetCurveTexel (i); }
  /** @} */
  
  /** \name iBezierState implementation
   * @{ */
  virtual iBezierFactoryState* GetFactory ()
  { return (iBezierFactoryState*)this; }
  /** @} */

  /** \name iObjectModel implementation
   * @{ */
  void GetObjectBoundingBox (csBox3& box) { GetBoundingBox (box); }
  void SetObjectBoundingBox (const csBox3& box) { SetBoundingBox (box); }
  /** @} */

  /** \name iMeshObject interface implementation
   * @{ */
  virtual iMeshObjectFactory* GetFactory () const { return (iMeshObjectFactory*)this; }
  virtual void SetVisibleCallback (iMeshObjectDrawCallback* /*cb*/) { }
  virtual iMeshObjectDrawCallback* GetVisibleCallback () const
  { return 0; }
  virtual void NextFrame (csTicks /*current_time*/,const csVector3& /*pos*/)
  { }
  virtual bool SupportsHardTransform () const { return true; }
  virtual bool HitBeamOutline (const csVector3& /*start*/,
    const csVector3& /*end*/, csVector3& /*isect*/, float* /*pr*/)
  {
    return false;
  }
  virtual bool HitBeamObject (const csVector3& /*start*/,
    const csVector3& /*end*/, csVector3& /*isect*/, float* /*pr*/,
    int* /*polygon_idx*/ = 0, iMaterialWrapper** = 0)
  {
    return false;
  }
  virtual void SetMeshWrapper (iMeshWrapper* lp)
  { logparent = lp; }
  virtual iMeshWrapper* GetMeshWrapper () const
  { return logparent; }
  virtual iObjectModel* GetObjectModel ()
  {
    return (iObjectModel*)this;
  }
  virtual bool SetColor (const csColor&) { return false; }
  virtual bool GetColor (csColor&) const { return false; }
  virtual bool SetMaterialWrapper (iMaterialWrapper*) { return false; }
  virtual iMaterialWrapper* GetMaterialWrapper () const { return 0; }
  virtual void SetMixMode (uint) { }
  virtual uint GetMixMode () const { return CS_FX_COPY; }
  virtual void InvalidateMaterialHandles () { }
  /**
    * see imesh/object.h for specification. The default implementation
    * does nothing.
    */
  virtual void PositionChild (iMeshObject* /*child*/, csTicks /*current_time*/) { }
  /** @} */

  /** \name iMeshObjectFactory interface implementation
   * @{ */
  virtual csPtr<iMeshObject> NewInstance ();
  virtual iMeshObjectType* GetMeshObjectType () const
  { return beziermsh_type; }
  /** @} */
};

/**
 * Thing type. This is the plugin you have to use to create instances
 * of csBezierMesh.
 */
class csBezierMeshObjectType : public scfImplementation3<csBezierMeshObjectType,
                                                         iMeshObjectType,
                                                         iComponent,
                                                         iPluginConfig>
{
public:
  iObjectRegistry* object_reg;
  bool do_verbose;	// Verbose error reporting.
  iEngine* engine;
  /**
   * csBezierMeshObjectType must keep a reference to G3D because when polygons
   * are destructed they actually refer to G3D to clear the cache.
   */
  csWeakRef<iGraphics3D> G3D;
  /// An object pool for lightpatches.
  csBezierLightPatchPool* lightpatch_pool;

public:
  /// Constructor.
  csBezierMeshObjectType (iBase*);

  /// Destructor.
  virtual ~csBezierMeshObjectType ();

  /// Register plugin with the system driver
  virtual bool Initialize (iObjectRegistry *object_reg);
  void Clear ();

  void ReportV (int severity, const char *description, va_list args);
  void Warn (const char *description, ...);
  void Bug (const char *description, ...);
  void Notify (const char *description, ...);
  void Error (const char *description, ...);

  /// New Factory.
  virtual csPtr<iMeshObjectFactory> NewFactory ();

  virtual bool GetOptionDescription (int idx, csOptionDescription *option);
  virtual bool SetOption (int id, csVariant* value);
  virtual bool GetOption (int id, csVariant* value);
};

}
CS_PLUGIN_NAMESPACE_END(Bezier)

#endif // __CS_BEZIERMESH_H__
