#ifndef __CS_SOFTBODYANIM2_H__
#define __CS_SOFTBODYANIM2_H__

#include "csutil/scf_implementation.h"
#include "csutil/dirtyaccessarray.h"
#include "iutil/comp.h"
#include "csutil/weakref.h"
#include "ivaria/physical2.h"
#include "iengine/movable.h"
#include "imesh/animesh.h"

CS_PLUGIN_NAMESPACE_BEGIN(SoftAnim2)
{
class SoftBodyControlType : public scfImplementation2<SoftBodyControlType,
  CS::Physics2::iSoftBodyAnimationControlType, iComponent>
{
  friend class SoftBodyControlFactory;
  friend class SoftBodyControl;
public:
  CS_LEAKGUARD_DECLARE(SoftBodyControlType);

  SoftBodyControlType (iBase* parent);

  //-- iGenMeshAnimationControlType
  virtual csPtr<iGenMeshAnimationControlFactory> CreateAnimationControlFactory ();

  //-- iComponent
  virtual bool Initialize (iObjectRegistry*);

  // error reporting
  void Report (int severity, const char* msg, ...) const;

private:
  iObjectRegistry* object_reg;
};

class SoftBodyControlFactory : public scfImplementation2<SoftBodyControlFactory, 
  scfFakeInterface<iGenMeshAnimationControlFactory>,
  CS::Physics2::iSoftBodyAnimationControlFactory>
{
  friend class SoftBodyControl;
public:
  CS_LEAKGUARD_DECLARE(CS::Physics2::iSoftBodyAnimationControlFactory);

  SoftBodyControlFactory (SoftBodyControlType* type);

  //-- iGenMeshAnimationControlFactory
  virtual csPtr<iGenMeshAnimationControl> CreateAnimationControl (iMeshObject* mesh);
  virtual const char* Load (iDocumentNode* node);
  virtual const char* Save (iDocumentNode* parent);

private:
  SoftBodyControlType* type;

};

class SoftBodyControl : public scfImplementation2<SoftBodyControl, 
  scfFakeInterface<iGenMeshAnimationControl>, CS::Physics2::iSoftBodyAnimationControl>
{
public:
  CS_LEAKGUARD_DECLARE(CS::Physics2::iSoftBodyAnimationControl);

  SoftBodyControl (SoftBodyControlFactory* factory, iMeshObject* mesh);

  //-- CS::Animation::iSoftBodyAnimationControl
  virtual void SetSoftBody (CS::Physics2::iSoftBody* body, bool doubleSided = false);
  virtual CS::Physics2::iSoftBody* GetSoftBody ();

  virtual void CreateAnimatedMeshAnchor (CS::Mesh::iAnimatedMesh* animesh,
    CS::Physics2::iRigidBody* body,
    size_t bodyVertexIndex,
    size_t animeshVertexIndex = (size_t) ~0);
  virtual size_t GetAnimatedMeshAnchorVertex (size_t bodyVertexIndex);
  virtual void RemoveAnimatedMeshAnchor (size_t bodyVertexIndex);

  //-- iGenMeshAnimationControl
  virtual bool AnimatesColors () const;
  virtual bool AnimatesNormals () const;
  virtual bool AnimatesTexels () const;
  virtual bool AnimatesVertices () const;
  virtual bool AnimatesBBoxRadius () const;
  virtual void Update (csTicks current, int num_verts, uint32 version_id);
  virtual const csColor4* UpdateColors (csTicks current, const csColor4* colors,
    int num_colors, uint32 version_id);
  virtual const csVector3* UpdateNormals (csTicks current, const csVector3* normals,
    int num_normals, uint32 version_id);
  virtual const csVector2* UpdateTexels (csTicks current, const csVector2* texels,
    int num_texels, uint32 version_id);
  virtual const csVector3* UpdateVertices (csTicks current, const csVector3* verts,
    int num_verts, uint32 version_id);
  virtual const csBox3& UpdateBoundingBox (csTicks current, uint32 version_id,
    const csBox3& factoryBbox);
  virtual const float UpdateRadius (csTicks current, uint32 version_id,
    const float factoryRadius);
  virtual const csBox3* UpdateBoundingBoxes (csTicks current, uint32 version_id);

private:
  SoftBodyControlFactory* factory;
  csWeakRef<iMeshObject> mesh;
  csRef<CS::Physics2::iSoftBody> softBody;
  bool doubleSided;
  csDirtyAccessArray<csVector3> vertices;
  csDirtyAccessArray<csVector3> normals;
  csBox3 bbox;
  csDirtyAccessArray<csBox3> bboxes;
  csTicks lastTicks;
  csVector3 meshPosition;

  struct Anchor
  {
    csWeakRef<CS::Mesh::iAnimatedMesh> animesh;
    csWeakRef<CS::Physics2::iRigidBody> body;
    size_t bodyVertexIndex;
    size_t animeshVertexIndex;
  };
  csArray<Anchor> anchors;
};
}
CS_PLUGIN_NAMESPACE_END(SoftAnim2)
#endif