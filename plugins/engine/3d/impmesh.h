/*
  Copyright (C) 2002 by Keith Fulton and Jorrit Tyberghein
  Rewritten during Summer of Code 2006 by Christoph "Fossi" Mewes
  Rewritten during Summer of Code 2009 by Michael Gist

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
#ifndef __CS_IMPMESH_H__
#define __CS_IMPMESH_H__

#include "cstool/objmodel.h"
#include "csutil/nobjvec.h"
#include "iengine/camera.h"
#include "iengine/impman.h"
#include "iengine/movable.h"
#include "iengine/sector.h"
#include "csgeom/poly3d.h"
#include "csgeom/box.h"
#include "ivideo/rendermesh.h"
#include "cstool/rendermeshholder.h"
#include "iengine/mesh.h"

class csVector3;
class csMatrix3;
class csMovable;
class csMeshFactoryWrapper;
class csImposterMesh;
class csEngine;
struct iGraphics3D;
struct iRenderView;

CS_PLUGIN_NAMESPACE_BEGIN(Engine)
{
  class csMeshWrapper;
}
CS_PLUGIN_NAMESPACE_END(Engine)

/**
 * Class representing the mesh/imposter relation.
 */
class csImposterMesh : public scfImplementationExt2<
	csImposterMesh, csObjectModel, iImposterMesh, iMeshObject>
{
private:
  // Imposter instance data.
  struct Instance
  {
    iMeshWrapper* mesh;
    csRef<csShaderVariable> transformVar;
    csRef<csShaderVariable> fadeFactor;

    Instance(iMeshWrapper* mesh) : mesh(mesh)
    {
    }
  };

  // Array of imposter instances.
  csArray<Instance*> instances;

  // Mesh billboard for this imposter.
  csRef<iMeshWrapper> mesh;

  // Array of transform vars for the imposter instances.
  csRef<csShaderVariable> transformVars;

  // Array of fade factors for the imposter instances.
  csRef<csShaderVariable> fadeFactors;

  // Factory that we're created from.
  iImposterFactory* fact;

  csRef<iImposterManager> impman;

  // Convenience shortcut
  csEngine *engine;

  iSector* sector;

  // Whether or not we're instancing.
  bool instance;

  // True if we're ready to be removed.
  bool removeMe;

  // Shader to use on the imposter material.
  csString shader;

  // Flags that indicate that we have been updated.
  bool materialUpdateNeeded;
  bool matDirty;
  bool meshDirty;

  // Saved values for update checking.
  csVector3 meshLocalDir;
  csVector3 cameraLocalDir;

  // Rendermesh holder for this mesh
  csRenderMeshHolder rmHolder;

  // Rect for billboard
  csPoly3D vertices;

  // Normals.
  csVector3 normals;

  // Imposter material.
  csRef<iMaterialWrapper> mat;

  // Imposter meshes (for batched rendering).
  csRefArray<csImposterMesh> imposterMeshes;

  // Number of meshes at last update.
  size_t numImposterMeshes;

  // Texture coordinates.
  csBox2 texCoords;

  // The camera this mesh is being viewed through.
  csWeakRef<iCamera> camera;

  // The distance to the closest instance of this imposter.
  float closestInstance;

  // The closest instance of the mesh.
  csWeakRef<iMeshWrapper> closestInstanceMesh;

  // True if currently awaiting an update.
  bool isUpdating;

  // Flags for iMeshObject.
  csFlags flags;

  // Bounding box.
  csBox3 bbox;

  csRef<iGraphics3D> g3d;

  // True if r2t has been performed for this imposter.
  bool rendered;

  // Current mesh being updated.
  uint currentMesh;

  // Number of meshes to update (for batching).
  uint updatePerFrame;

  void AddSVToMesh(iMeshWrapper* mesh, csShaderVariable* sv);

  void CreateInstance(iMeshWrapper* pmesh);

  void DestroyInstance(Instance* instance);

  void InitMesh();

  bool WithinTolerance(iRenderView *rview, iMeshWrapper* pmesh);

  void SetupRenderMeshes(csRenderMesh*& mesh, bool rmCreated, iRenderView* rview);

  void SetupRenderMeshesInstance(csRenderMesh*& mesh,  bool rmCreated, iCamera* camera);

  // For batched imposter sorting.
  static int ImposterMeshSort(csImposterMesh* const& f, csImposterMesh* const& s);

  friend class csImposterManager;

public:
  csImposterMesh (csEngine* engine, iSector* sector);

  csImposterMesh (csEngine* engine, iImposterFactory* fact,
    iMeshWrapper* mesh, iRenderView* rview, bool instance, const char* shader);
  virtual ~csImposterMesh () {}

  ///////////////////// iImposterMesh /////////////////////

  /**
   * Whether this imposter is currently instancing any meshes.
   */
  virtual bool IsInstancing()
  { return (!instance && !removeMe) || instances.GetSize() != 0; }

  /**
   * Add an instance of the passed mesh.
   * Returns true if able to add an instance for this mesh.
   * Returns false otherwise.
   */
  virtual bool Add(iMeshWrapper* mesh, iRenderView* rview);

  /**
   * Update the instance of the passed mesh.
   * Returns false if the mesh instance was removed (no longer valid for this imposter).
   * Returns true otherwise.
   */
  virtual bool Update(iMeshWrapper* mesh, iRenderView* rview);

  /**
   * Remove the instance of the passed mesh.
   * Returns false if not currently instancing this mesh.
   * Returns true otherwise.
   */
  virtual bool Remove(iMeshWrapper* mesh);

  /**
   * Destroy this imposter.
   */
  virtual void Destroy();

  /**
   * Query whether the r2t has been performed for this imposter.
   */
  virtual bool Rendered() const
  {
    return rendered;
  }

  ///////////////////// iObjectModel /////////////////////
  virtual const csBox3& GetObjectBoundingBox()
  {
    return bbox;
  }

  virtual void SetObjectBoundingBox(const csBox3& box)
  {
      bbox = box;
  }

  virtual void GetRadius (float& rad, csVector3& cent)
  {
    instances[0]->mesh->GetMeshObject()->GetObjectModel()->GetRadius(rad, cent);
  }

  ///////////////////// iMeshObject /////////////////////
  virtual csRenderMesh** GetRenderMeshes (int& num, iRenderView* rview, 
    iMovable* movable, uint32 frustum_mask);

  virtual iMeshWrapper* GetMeshWrapper () const
  {
    return mesh;
  }

  virtual bool SetMaterialWrapper (iMaterialWrapper* material)
  {
    mat = material;
    return true;
  }

  virtual iMaterialWrapper* GetMaterialWrapper () const
  {
    return mat;
  }

  virtual bool SupportsHardTransform () const
  {
    return false;
  }

  virtual iObjectModel* GetObjectModel ()
  {
    return static_cast<iObjectModel*>(this);
  }

  virtual csFlags& GetFlags () { return flags; }

  /// All of these are unsupported.
  virtual iMeshObjectFactory* GetFactory () const { return 0; }
  virtual csPtr<iMeshObject> Clone () { return csPtr<iMeshObject>(0); }
  virtual void SetVisibleCallback (iMeshObjectDrawCallback* cb) {}
  virtual iMeshObjectDrawCallback* GetVisibleCallback () const { return 0; }
  virtual void NextFrame (csTicks current_time,const csVector3& pos,
    uint currentFrame) {}
  virtual void HardTransform (const csReversibleTransform& t) {}
  virtual bool HitBeamOutline (const csVector3& start,
  	const csVector3& end, csVector3& isect, float* pr) { return false; }
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr, int* polygon_idx = 0,
	iMaterialWrapper** material = 0, csArray<iMaterialWrapper*>* materials = 0) { return false; }
  virtual void SetMeshWrapper (iMeshWrapper* logparent) {}
  virtual bool SetColor (const csColor& color) { return false; }
  virtual bool GetColor (csColor& color) const { return false; }
  virtual void SetMixMode (uint mode) {}
  virtual uint GetMixMode () const { return 0; }
  virtual void PositionChild (iMeshObject* child,csTicks current_time) {}
  virtual void BuildDecal(const csVector3* pos, float decalRadius,
    iDecalBuilder* decalBuilder) {}
};

#endif // __CS_IMPMESH_H__
