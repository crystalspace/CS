/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef _GENMESH_H_
#define _GENMESH_H_

#include "csgeom/vector3.h"
#include "csgeom/transfrm.h"
#include "csutil/cscolor.h"
#include "imesh/object.h"
#include "imesh/genmesh.h"
#include "ivideo/graph3d.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "ivideo/vbufmgr.h"

struct iMaterialWrapper;
struct iObjectRegistry;
class csGenmeshMeshObjectFactory;
class csColor;
class G3DFogInfo;

/**
 * Genmesh version of mesh object.
 */
class csGenmeshMeshObject : public iMeshObject
{
private:
  csGenmeshMeshObjectFactory* factory;
  iBase* logparent;
  iMaterialWrapper* material;
  uint MixMode;
  iMeshObjectDrawCallback* vis_cb;
  bool do_lighting;
  bool do_manual_colors;
  csColor color;
  float current_lod;
  uint32 current_features;

  csColor* lit_mesh_colors;

  bool initialized;
  long shapenr;

  /**
   * Camera space bounding box is cached here.
   * GetCameraBoundingBox() will check the current camera number from the
   * camera to see if it needs to recalculate this.
   */
  csBox3 camera_bbox;
  csBox3 world_bbox;

  /// Current camera number.
  long cur_cameranr;
  /// Current movable number.
  long cur_movablenr;

  /**
   * Setup this object. This function will check if setup is needed.
   */
  void SetupObject ();

public:
  /// Constructor.
  csGenmeshMeshObject (csGenmeshMeshObjectFactory* factory);

  /// Destructor.
  virtual ~csGenmeshMeshObject ();

  /// Get the bounding box in transformed space.
  void GetTransformedBoundingBox (long cameranr, long movablenr,
      const csReversibleTransform& trans, csBox3& cbox);
  /**
   * Get the coordinates of the mesh in screen coordinates.
   * Fills in the boundingBox with the X and Y locations of the mesh.
   * Returns the max Z location of the mesh, or -1 if not
   * on-screen. If the mesh is not on-screen, the X and Y values are not
   * valid.
   */
  float GetScreenBoundingBox (long cameranr, long movablenr, float fov,
  	float sx, float sy,
	const csReversibleTransform& trans, csBox2& sbox, csBox3& cbox);

  iMaterialWrapper* GetMaterialWrapper () const { return material; }
  uint GetMixMode () const { return MixMode; }
  void SetLighting (bool l) { do_lighting = l; }
  bool IsLighting () const { return do_lighting; }
  void SetColor (const csColor& col) { color = col; }
  csColor GetColor () const { return color; }
  void SetManualColors (bool m) { do_manual_colors = m; }
  bool IsManualColors () const { return do_manual_colors; }

  //----------------------- iMeshObject implementation ------------------------
  SCF_DECLARE_IBASE;

  virtual iMeshObjectFactory* GetFactory () const
  {
    return (iMeshObjectFactory*)factory;
  }
  virtual bool DrawTest (iRenderView* rview, iMovable* movable);
  virtual void UpdateLighting (iLight** lights, int num_lights,
      	iMovable* movable);
  virtual bool Draw (iRenderView* rview, iMovable* movable, csZBufMode mode);
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
  virtual void GetObjectBoundingBox (csBox3& bbox, int type = CS_BBOX_NORMAL);
  virtual void GetRadius (csVector3& rad, csVector3& cent);
  virtual void NextFrame (csTicks /*current_time*/) { }
  virtual bool WantToDie () const { return false; }
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
  virtual bool HitBeamOutline (const csVector3& start, const csVector3& end,
    csVector3& isect, float *pr);
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr);
  virtual long GetShapeNumber () const { return shapenr; }
  virtual void SetLogicalParent (iBase* lp) { logparent = lp; }
  virtual iBase* GetLogicalParent () const { return logparent; }
  virtual iPolygonMesh* GetWriteObject () { return NULL; }

  //------------------------- iGeneralMeshState implementation ----------------
  class GeneralMeshState : public iGeneralMeshState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csGenmeshMeshObject);
    virtual void SetMaterialWrapper (iMaterialWrapper* material)
    {
      scfParent->material = material;
    }
    virtual iMaterialWrapper* GetMaterialWrapper () const
    { return scfParent->material; }
    virtual void SetMixMode (uint mode) { scfParent->MixMode = mode; }
    virtual uint GetMixMode () const { return scfParent->MixMode; }
    virtual void SetLighting (bool l) { scfParent->SetLighting (l); }
    virtual bool IsLighting () const { return scfParent->IsLighting (); }
    virtual void SetColor (const csColor& col) { scfParent->SetColor (col); }
    virtual csColor GetColor () const { return scfParent->GetColor (); }
    virtual void SetManualColors (bool m)
    {
      scfParent->SetManualColors (m);
    }
    virtual bool IsManualColors () const
    {
      return scfParent->IsManualColors ();
    }
  } scfiGeneralMeshState;
  friend class GeneralMeshState;
};

/**
 * Factory for general meshes.
 */
class csGenmeshMeshObjectFactory : public iMeshObjectFactory
{
private:
  iVertexBuffer* vbuf;
  iVertexBufferManager* vbufmgr;

  iMaterialWrapper* material;
  G3DTriangleMesh top_mesh;
  csVector3* mesh_vertices;
  csVector2* mesh_texels;
  csVector3* mesh_normals;
  csColor* mesh_colors;
  int num_mesh_vertices;

  csVector3 radius;
  csBox3 object_bbox;
  bool object_bbox_valid;
  bool initialized;

  /// Calculate bounding box and radius.
  void CalculateBBoxRadius ();

  /// Retrieve a vertexbuffer from the manager if not done already.
  void SetupVertexBuffer ();

  /**
   * Setup this factory. This function will check if setup is needed.
   */
  void SetupFactory ();

  /// interface to receive state of vertexbuffermanager
  struct eiVertexBufferManagerClient : public iVertexBufferManagerClient
  {
    SCF_DECLARE_EMBEDDED_IBASE (csGenmeshMeshObjectFactory);
    virtual void ManagerClosing ();
  }scfiVertexBufferManagerClient;
  friend struct eiVertexBufferManagerClient;

public:
  iObjectRegistry* object_reg;
  iBase* logparent;

  /// Constructor.
  csGenmeshMeshObjectFactory (iBase *pParent, iObjectRegistry* object_reg);

  /// Destructor.
  virtual ~csGenmeshMeshObjectFactory ();

  void SetMaterialWrapper (iMaterialWrapper* material)
  {
    csGenmeshMeshObjectFactory::material = material;
  }
  iMaterialWrapper* GetMaterialWrapper () const { return material; }
  void SetVertexCount (int n);
  int GetVertexCount () const { return num_mesh_vertices; }
  csVector3* GetVertices () { SetupFactory (); return mesh_vertices; }
  csVector2* GetTexels () { SetupFactory (); return mesh_texels; }
  csVector3* GetNormals () { SetupFactory (); return mesh_normals; }
  csColor* GetColors () { SetupFactory (); return mesh_colors; }
  void SetTriangleCount (int n);
  int GetTriangleCount () const { return top_mesh.num_triangles; }
  csTriangle* GetTriangles () { SetupFactory (); return top_mesh.triangles; }
  void Invalidate ();
  void CalculateNormals ();
  void GenerateBox (const csBox3& box);
  const csBox3& GetObjectBoundingBox ();
  const csVector3& GetRadius ();

  iVertexBufferManager* GetVertexBufferManager ()
  {
    SetupFactory ();
    return vbufmgr;
  }
  iVertexBuffer* GetVertexBuffer ()
  {
    SetupFactory ();
    return vbuf;
  }
  G3DTriangleMesh& GetMesh ()
  {
    SetupFactory ();
    return top_mesh;
  }

  //------------------------ iMeshObjectFactory implementation --------------
  SCF_DECLARE_IBASE;

  virtual iMeshObject* NewInstance ();
  virtual void HardTransform (const csReversibleTransform&) {/*@@@ TODO*/ }
  virtual bool SupportsHardTransform () const { return false; }
  virtual void SetLogicalParent (iBase* lp) { logparent = lp; }
  virtual iBase* GetLogicalParent () const { return logparent; }

  //----------------------- iGeneralFactoryState implementation -------------
  class GeneralFactoryState : public iGeneralFactoryState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csGenmeshMeshObjectFactory);
    virtual void SetMaterialWrapper (iMaterialWrapper* material)
    {
      scfParent->SetMaterialWrapper (material);
    }
    virtual iMaterialWrapper* GetMaterialWrapper () const
    {
      return scfParent->GetMaterialWrapper ();
    }
    virtual void SetVertexCount (int n)
    {
      scfParent->SetVertexCount (n);
    }
    virtual int GetVertexCount () const
    {
      return scfParent->GetVertexCount ();
    }
    virtual csVector3* GetVertices ()
    {
      return scfParent->GetVertices ();
    }
    virtual csVector2* GetTexels ()
    {
      return scfParent->GetTexels ();
    }
    virtual csVector3* GetNormals ()
    {
      return scfParent->GetNormals ();
    }
    virtual void SetTriangleCount (int n)
    {
      scfParent->SetTriangleCount (n);
    }
    virtual int GetTriangleCount () const
    {
      return scfParent->GetTriangleCount ();
    }
    virtual csTriangle* GetTriangles ()
    {
      return scfParent->GetTriangles ();
    }
    virtual csColor* GetColors ()
    {
      return scfParent->GetColors ();
    }
    virtual void Invalidate ()
    {
      scfParent->Invalidate ();
    }
    virtual void CalculateNormals ()
    {
      scfParent->CalculateNormals ();
    }
    virtual void GenerateBox (const csBox3& box)
    {
      scfParent->GenerateBox (box);
    }
  } scfiGeneralFactoryState;
  friend class GeneralFactoryState;
};

/**
 * Genmesh type. This is the plugin you have to use to create instances
 * of csGenmeshMeshObjectFactory.
 */
class csGenmeshMeshObjectType : public iMeshObjectType
{
public:
  iObjectRegistry* object_reg;

  SCF_DECLARE_IBASE;

  /// Constructor.
  csGenmeshMeshObjectType (iBase*);
  /// Destructor.
  virtual ~csGenmeshMeshObjectType ();
  /// Draw.
  virtual iMeshObjectFactory* NewFactory ();
  /// Initialize.
  bool Initialize (iObjectRegistry* object_reg)
  {
    csGenmeshMeshObjectType::object_reg = object_reg;
    return true;
  }

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csGenmeshMeshObjectType);
    virtual bool Initialize (iObjectRegistry* object_reg)
    {
      return scfParent->Initialize (object_reg);
    }
  } scfiComponent;
};

#endif // _GENMESH_H_

