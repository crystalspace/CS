/*
    Copyright (C) 2003 by Keith Fulton

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

#ifndef __CS_SPRCAL3D_H__
#define __CS_SPRCAL3D_H__

// This include looks horrible, but assumes that cal3d 
// is set up in the usual way and shares the same parent
// directory as CS.
#include "cal3d/cal3d.h"

#include "cssys/sysfunc.h"
#include "csutil/cscolor.h"
#include "csutil/parray.h"
#include "csutil/randomgen.h"
#include "csutil/refarr.h"
#include "csgeom/math3d.h"
#include "csgeom/math2d.h"
#include "csgeom/poly2d.h"
#include "csgeom/poly3d.h"
#include "csgeom/box.h"
#include "csgeom/objmodel.h"
#include "imesh/spritecal3d.h"
#include "imesh/object.h"
#include "iengine/material.h"
#include "iengine/lod.h"
#include "iutil/config.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/virtclk.h"
#include "ivideo/graph3d.h"
#ifndef CS_USE_NEW_RENDERER
#include "ivideo/vbufmgr.h"
#else
#include "ivideo/rendermesh.h"
#include "ivideo/rndbuf.h"
#include "cstool/anonrndbuf.h"
#endif // CS_USE_NEW_RENDERER
#include "ivideo/material.h"
#include "qint.h"


struct iObjectRegistry;
struct iEngine;

#define ALL_LOD_FEATURES (CS_LOD_TRIANGLE_REDUCTION|CS_LOD_DISTANCE_REDUCTION)


class csSpriteCal3DMeshObject;

/**
 * A Cal3D sprite based on a triangle mesh with a single texture.
 * Animation is done with frames.
 * This class represents a template from which a csSpriteCal3D
 * class can be made.
 */
class csSpriteCal3DMeshObjectFactory : public iMeshObjectFactory
{
private:
  friend class csSpriteCal3DMeshObject;

  /// Material handle as returned by iTextureManager.
  iBase* logparent;

  /// If true then this factory has been initialized.
  bool initialized;

  /// This is the factory equivalent class in cal3d.
  CalCoreModel calCoreModel;
  csArray<int> animationIDs;
  csString     basePath;
  float	       renderScale;

public:
  iObjectRegistry* object_reg;
  iVirtualClock* vc;

  csRef<iGraphics3D> g3d;

  /**
   * Reference to the engine (optional because sprites can also be
   * used for the isometric engine).
   */
  iEngine* engine;

  /// Create the sprite template.
  csSpriteCal3DMeshObjectFactory (iBase *pParent);
  /// Destroy the template.
  virtual ~csSpriteCal3DMeshObjectFactory ();

  void Report (int severity, const char* msg, ...);

  //------------------------iSpriteCal3dFactoryState implementation --------
  /// Create a new core object.
  bool Create(const char *name);
  void ReportLastError ();
  void SetBasePath(const char *path);
  void SetRenderScale(float scale);
  bool LoadCoreSkeleton(const char *filename);
  int  LoadCoreAnimation(const char *filename);
  bool LoadCoreMesh(const char *filename);
  bool AddCoreMaterial(iMaterialWrapper *mat);
  void BindMaterials();

  //------------------------ iMeshObjectFactory implementation --------------
  SCF_DECLARE_IBASE;

  virtual csPtr<iMeshObject> NewInstance ();
  virtual void HardTransform (const csReversibleTransform& t) { };
  virtual bool SupportsHardTransform () const { return true; }
  virtual void SetLogicalParent (iBase* lp) { logparent = lp; }
  virtual iBase* GetLogicalParent () const { return logparent; }


  //------------------ iPolygonMesh interface implementation ----------------//

  int GetVertexCount() { return 0; }
  csVector3* GetVertices () { return 0; }
  int GetTriangleCount () { return 0; }
  csMeshedPolygon* GetPolygons () { return 0; }

  struct PolyMesh : public iPolygonMesh
  {
  private:
    csSpriteCal3DMeshObjectFactory* factory;
  public:
    SCF_DECLARE_IBASE;

    void SetFactory (csSpriteCal3DMeshObjectFactory* Factory)
    {
      factory = Factory;
    }

    /// Get the number of vertices for this mesh.
    virtual int GetVertexCount ()
    {
      return factory->GetVertexCount ();
    }
    /// Get the pointer to the array of vertices.
    virtual csVector3* GetVertices ()
    {
      return factory->GetVertices ();
    }
    /// Get the number of polygons for this mesh.
    virtual int GetPolygonCount ()
    {
      return factory->GetTriangleCount ();
    }

    /// Get the pointer to the array of polygons.
    virtual csMeshedPolygon* GetPolygons ()
    {
      return factory->GetPolygons();
    }

    /// Cleanup.
    virtual void Cleanup () { } //  delete[] polygons; polygons = 0; }
    
    virtual bool IsDeformable () const { return false;  }
    virtual uint32 GetChangeNumber() const { return 0; }

    PolyMesh () : polygons (0)
    {
      SCF_CONSTRUCT_IBASE (0);
    }
    virtual ~PolyMesh () { Cleanup (); }

    csMeshedPolygon* polygons;
  } scfiPolygonMesh;
  friend struct PolyMesh;

  //------------------------- iObjectModel implementation ----------------
  void GetObjectBoundingBox (csBox3& bbox, int type, csVector3 *verts,int vertCount);
  void GetObjectBoundingBox (csBox3& bbox, int type = CS_BBOX_NORMAL);
  void GetRadius (csVector3& rad, csVector3& cent);

  class ObjectModel : public csObjectModel
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObjectFactory);
    virtual void GetObjectBoundingBox (csBox3& bbox, int type = CS_BBOX_NORMAL)
    {
      scfParent->GetObjectBoundingBox (bbox, type);
    }
    virtual void GetRadius (csVector3& rad, csVector3& cent)
    {
      scfParent->GetRadius (rad, cent);
    }
  } scfiObjectModel;
  friend class ObjectModel;

  virtual iObjectModel* GetObjectModel () { return &scfiObjectModel; }

  //--------------------- iSpriteCal3DFactoryState implementation -------------//
  struct SpriteCal3DFactoryState : public iSpriteCal3DFactoryState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObjectFactory);

    virtual bool Create(const char *name)
    { return scfParent->Create(name); }

    virtual void ReportLastError ()
    { scfParent->ReportLastError(); }

    virtual void SetBasePath(const char *path)
    { scfParent->SetBasePath(path); }

    virtual void SetRenderScale(float scale)
    { scfParent->SetRenderScale(scale); }

    virtual bool LoadCoreSkeleton(const char *filename)
    { return scfParent->LoadCoreSkeleton(filename); }

    virtual int  LoadCoreAnimation(const char *filename)
    { return scfParent->LoadCoreAnimation(filename); }

    virtual bool LoadCoreMesh(const char *filename)
    { return scfParent->LoadCoreMesh(filename); }

    virtual bool AddCoreMaterial(iMaterialWrapper *mat)
    { return scfParent->AddCoreMaterial(mat); }

    virtual void BindMaterials()
    { scfParent->BindMaterials(); }

  } scfiSpriteCal3DFactoryState;
  struct LODControl : public iLODControl
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObjectFactory);
    virtual uint32 GetLODFeatures () const
    {
      return ALL_LOD_FEATURES;
    }
    virtual void SetLODFeatures (uint32 mask, uint32 value)
    {
      (void)mask; (void)value;
      // @@@ TODO
    }
    virtual void SetLOD (float lod)
    {
      //csSpriteCal3DMeshObject::global_lod_level = lod;
    }
    virtual float GetLOD () const
    {
      return 0; // csSpriteCal3DMeshObject::global_lod_level;
    }
    virtual int GetLODPolygonCount (float /*lod*/) const
    {
      return 0;
    }
    virtual uint32 GetAvailableLODFeatures () const
    {
      return ALL_LOD_FEATURES;
    }
    virtual uint32 GetAvailableDistanceFeatures () const
    {
      return CS_LOD_TRIANGLE_REDUCTION;
    }
    virtual uint32 GetDistanceReduction () const
    {
      return CS_LOD_TRIANGLE_REDUCTION;
    }
    virtual void SetDistanceReduction (uint32 mask, uint32 value)
    {
      (void)mask; (void)value;
      // @@@ TODO
    }
    virtual void SetLODPriority (uint16 group)
    {
      (void)group;
      // @@@ TODO
    }
    virtual uint16 GetLODPriority () const
    {
      return 0;
    }
    virtual void SetMinLODThreshold (float level, bool turnOff)
    {
      (void)level; (void)turnOff;
      // @@@ TODO
    }
  } scfiLODControl;
  friend struct LODControl;

};

/**
 * A Cal3D sprite based on a triangle mesh with a single texture.
 * Animation is done with frames (a frame may be controlled by
 * a skeleton).
 */
class csSpriteCal3DMeshObject : public iMeshObject
{
private:
  iMeshObjectDrawCallback* vis_cb;
  uint32 current_features;  // LOD Control thing
  iBase* logparent;
  CalModel calModel;
  float last_update_time;
#ifndef CS_USE_NEW_RENDERER
  iVertexBufferManager* vbufmgr;
  csRef<iVertexBuffer> vbuf;
//  csArray<G3DTriangleMesh>  mesh;
//  csArray<bool>             initialized;
//  csArray<csColor*>         mesh_colors;
  bool arrays_initialized;
  csArray<G3DTriangleMesh>  *meshes;
  csArray<bool>             *is_initialized;
  csArray<csColor*>         *meshes_colors;
  csBox3 object_bbox;

#endif

  void SetupObject ();
  void SetupVertexBuffer (int mesh,int submesh,int num_vertices,int num_triangles,csTriangle *triangles);
  bool DrawSubmesh (iGraphics3D* g3d,iRenderView* rview,CalRenderer *pCalRenderer,int mesh,int submesh);
  void UpdateLightingSubmesh (iLight** lights, int num_lights,iMovable* movable,CalRenderer *pCalRenderer,int mesh, int submesh);

public:

  /// The parent.
  csSpriteCal3DMeshObjectFactory* factory;

  /// Constructor.
  csSpriteCal3DMeshObject (iBase *pParent, CalCoreModel& calCoreModel);
  /// Destructor.
  virtual ~csSpriteCal3DMeshObject ();

  /// Set the factory.
  void SetFactory (csSpriteCal3DMeshObjectFactory* factory);

  /// Get the factory.
  csSpriteCal3DMeshObjectFactory* GetFactory3D () const { return factory; }


  ///------------------------ iMeshObject implementation ----------------------
  SCF_DECLARE_IBASE;
  virtual bool HitBeamOutline (const csVector3& start, const csVector3& end,
      csVector3& intersect, float* pr) { return false; }
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
      csVector3& intersect, float* pr) { return false; }

  virtual bool SetColor (const csColor& col)
  {
    //SetBaseColor (col);
    return true;
  }
  virtual bool GetColor (csColor& col) const
  {
    //GetBaseColor (col);
    return true;
  }
  virtual bool SetMaterialWrapper (iMaterialWrapper* mat)
  {
    //SetMaterial (mat);
    return true;
  }
  virtual iMaterialWrapper* GetMaterialWrapper () const
  {
    return NULL; //GetMaterial ();
  }
  virtual int GetPortalCount () const { return 0; }
  virtual iPortal* GetPortal (int) const { return 0; }


  virtual iMeshObjectFactory* GetFactory () const
  {
    csRef<iMeshObjectFactory> ifact (SCF_QUERY_INTERFACE (factory,
    	iMeshObjectFactory));
    return ifact;	// DecRef is ok here.
  }
  virtual bool DrawTest (iRenderView* rview, iMovable* movable);
  virtual void UpdateLighting (iLight** lights, int num_lights,
      iMovable* movable);
  virtual bool Draw (iRenderView* rview, iMovable* movable, csZBufMode mode);
#ifdef CS_USE_NEW_RENDERER
  virtual csRenderMesh **GetRenderMeshes (int &n);
#endif // CS_USE_NEW_RENDERER
  virtual void SetVisibleCallback (iMeshObjectDrawCallback* cb)
  {
    if (cb) cb->IncRef ();
    if (vis_cb) vis_cb->DecRef ();
    vis_cb = cb;
  }
  virtual iMeshObjectDrawCallback* GetVisibleCallback () const
  {
    return vis_cb;
  }
  virtual bool Advance (csTicks current_time);

  virtual void NextFrame (csTicks current_time,const csVector3& new_pos)
  {   
    Advance (current_time);
  }
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
  virtual void SetLogicalParent (iBase* lp) { logparent = lp; }
  virtual iBase* GetLogicalParent () const { return logparent; }

  //------------------ iPolygonMesh interface implementation ----------------//
  struct PolyMesh : public iPolygonMesh
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObject);

    /// Get the number of vertices for this mesh.
    virtual int GetVertexCount ()
    {
      csSpriteCal3DMeshObjectFactory* fact = scfParent->GetFactory3D ();
      return fact->GetVertexCount ();
    }
    /// Get the pointer to the array of vertices.
    virtual csVector3* GetVertices ()
    {
      csSpriteCal3DMeshObjectFactory* fact = scfParent->GetFactory3D ();
      return fact->GetVertices ();
    }
    /// Get the number of polygons for this mesh.
    virtual int GetPolygonCount ()
    {
      csSpriteCal3DMeshObjectFactory* fact = scfParent->GetFactory3D ();
      return fact->GetTriangleCount ();
    }

    /// Get the pointer to the array of polygons.
    virtual csMeshedPolygon* GetPolygons ();

    /// Cleanup.
    virtual void Cleanup () { delete[] polygons; polygons = 0; }
    
    virtual bool IsDeformable () const { return false;  }
    virtual uint32 GetChangeNumber() const { return 0; }

    PolyMesh () : polygons (0) { }
    virtual ~PolyMesh () { Cleanup (); }

    csMeshedPolygon* polygons;
  } scfiPolygonMesh;
  friend struct PolyMesh;

  virtual iObjectModel* GetObjectModel () { return &scfiObjectModel; }

  //--------------------- iSpriteCal3DState implementation -------------//
  struct SpriteCal3DState : public iSpriteCal3DState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObject);

  } scfiSpriteCal3DState;

  //--------------------- iLODControl implementation -------------//
  int GetLODPolygonCount (float lod)
  { return 0; }

  struct LODControl : public iLODControl
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObject);
    virtual uint32 GetLODFeatures () const
    {
      return scfParent->current_features;
    }
    virtual void SetLODFeatures (uint32 mask, uint32 value)
    {
      mask &= ALL_LOD_FEATURES;
      scfParent->current_features = (scfParent->current_features & ~mask)
      	| (value & mask);
    }
    virtual void SetLOD (float lod) {  }
    virtual float GetLOD () const { return 1; }
    virtual int GetLODPolygonCount (float lod) const
    {
      return scfParent->GetLODPolygonCount (lod);
    }
    virtual uint32 GetAvailableLODFeatures () const
    {
      return ALL_LOD_FEATURES;
    }
    virtual uint32 GetAvailableDistanceFeatures () const
    {
      return CS_LOD_TRIANGLE_REDUCTION;
    }
    virtual uint32 GetDistanceReduction () const
    {
      return CS_LOD_TRIANGLE_REDUCTION;
    }
    virtual void SetDistanceReduction (uint32 mask, uint32 value)
    {
      (void)mask; (void)value;
      // @@@ TODO
    }
    virtual void SetLODPriority (uint16 group)
    {
      (void)group;
      // @@@ TODO
    }
    virtual uint16 GetLODPriority () const
    {
      return 0;
    }
    virtual void SetMinLODThreshold (float level, bool turnOff)
    {
      (void)level; (void)turnOff;
      // @@@ TODO
    }
  } scfiLODControl;
  friend struct LODControl;

  /// interface to receive state of vertexbuffermanager
  struct eiVertexBufferManagerClient : public iVertexBufferManagerClient
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObject);
    virtual void ManagerClosing ();
  }scfiVertexBufferManagerClient;
  friend struct eiVertexBufferManagerClient;

  void GetObjectBoundingBox (csBox3& bbox, int type, csVector3 *verts,int vertCount);
  void GetObjectBoundingBox (csBox3& bbox, int type = CS_BBOX_NORMAL);
  void GetRadius (csVector3& rad, csVector3& cent);

  class ObjectModel : public csObjectModel
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObject);
    virtual void GetObjectBoundingBox (csBox3& bbox, int type = CS_BBOX_NORMAL)
    {
      scfParent->GetObjectBoundingBox (bbox, type);
    }
    virtual void GetRadius (csVector3& rad, csVector3& cent)
    {
      scfParent->GetRadius (rad, cent);
    }
  } scfiObjectModel;
  friend class ObjectModel;
};

/**
 * Sprite Cal3D type. This is the plugin you have to use to create instances
 * of csSpriteCal3DMeshObjectFactory.
 */
class csSpriteCal3DMeshObjectType : public iMeshObjectType
{
private:
  iObjectRegistry* object_reg;
  csRef<iVirtualClock> vc;
  iEngine* engine;

public:
  /// Constructor.
  csSpriteCal3DMeshObjectType (iBase*);

  /// Destructor.
  virtual ~csSpriteCal3DMeshObjectType ();

  bool Initialize (iObjectRegistry* p);

  //------------------------ iMeshObjectType implementation --------------
  SCF_DECLARE_IBASE;

  /// New Factory.
  virtual csPtr<iMeshObjectFactory> NewFactory ();

  //------------------- iConfig interface implementation -------------------
  struct csSpriteCal3DConfig : public iConfig
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObjectType);
    virtual bool GetOptionDescription (int idx, csOptionDescription *option) { return false; }
    virtual bool SetOption (int id, csVariant* value) { return false; }
    virtual bool GetOption (int id, csVariant* value) { return false; }
  } scfiConfig;
  friend struct csSpriteCal3DConfig;

  //--------------------- iComponent interface implementation
  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSpriteCal3DMeshObjectType);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
  friend struct eiComponent;

  //--------------------- iLODControl implementation -------------//
  struct LODControl : public iLODControl
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSpriteCal3DMeshObjectType);
    virtual uint32 GetLODFeatures () const
    {
      return ALL_LOD_FEATURES;
    }
    virtual void SetLODFeatures (uint32 mask, uint32 value)
    {
      (void)mask; (void)value;
      // @@@ TODO
    }
    virtual void SetLOD (float lod)
    {
      //csSpriteCal3DMeshObject::global_lod_level = lod;
    }
    virtual float GetLOD () const
    {
      return 0; // csSpriteCal3DMeshObject::global_lod_level;
    }
    virtual int GetLODPolygonCount (float /*lod*/) const
    {
      return 0;
    }
    virtual uint32 GetAvailableLODFeatures () const
    {
      return ALL_LOD_FEATURES;
    }
    virtual uint32 GetAvailableDistanceFeatures () const
    {
      return CS_LOD_TRIANGLE_REDUCTION;
    }
    virtual uint32 GetDistanceReduction () const
    {
      return CS_LOD_TRIANGLE_REDUCTION;
    }
    virtual void SetDistanceReduction (uint32 mask, uint32 value)
    {
      (void)mask; (void)value;
      // @@@ TODO
    }
    virtual void SetLODPriority (uint16 group)
    {
      (void)group;
      // @@@ TODO
    }
    virtual uint16 GetLODPriority () const
    {
      return 0;
    }
    virtual void SetMinLODThreshold (float level, bool turnOff)
    {
      (void)level; (void)turnOff;
      // @@@ TODO
    }
  } scfiLODControl;
  friend struct LODControl;
};

#endif // __CS_SPRCAL3D_H__

