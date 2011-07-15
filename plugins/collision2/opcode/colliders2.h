#ifndef __CS_OPCODE_COLLIDER_H__
#define __CS_OPCODE_COLLIDER_H__

#include "imesh/terrain2.h"
#include "ivaria/collision2.h"
#include "Opcode.h"
#include "csOpcode2.h"

CS_PLUGIN_NAMESPACE_BEGIN (Opcode2)
{
class csOpcodeCollider : public scfImplementation1<
  csOpcodeCollider, CS::Collision2::iColliderConcaveMesh>
{
  friend class csOpcodeCollisionObject;
  friend class csOpcodeCollisionSector;
private:
  float volume;
  Opcode::Model* model;
  iMeshWrapper* mesh;
  Opcode::MeshInterface opcMeshInt;
  unsigned int* indexholder;
  Point *vertholder;
  csVector3 scale;
  csBox3 aabbox;
  csOpcodeCollisionSystem* system;

  static void MeshCallback (udword triangle_index, 
    Opcode::VertexPointers& triangle, void* user_data);
public:
  csOpcodeCollider (iMeshWrapper* mesh, csOpcodeCollisionSystem* sys);
  virtual ~csOpcodeCollider();
  virtual CS::Collision2::ColliderType GetGeometryType () const {return CS::Collision2::COLLIDER_CONCAVE_MESH;}
  virtual iMeshWrapper* GetMesh () {return mesh;}
  virtual void SetLocalScale (const csVector3& scale) {}
  virtual const csVector3& GetLocalScale () const {return scale;}
  virtual void SetMargin (float margin) {}
  virtual float GetMargin () const {return 0;}
  virtual float GetVolume () const {return volume;}
};

/*
class csOpcodeColliderBox: public scfImplementation2<
  csOpcodeColliderBox, csOpcodeCollider, iColliderBox>
{
  IceMaths::AABB shape;
  csVector3 boxSize;

public:
  csOpcodeColliderBox (const csVector3& boxSize);
  virtual ~csOpcodeColliderBox();
  virtual ColliderType GetGeometryType() const {return COLLIDER_BOX;}
  virtual csVector3 GetBoxGeometry() {return boxSize};
};

class csOPCODEColliderSphere: public scfImplementation2<
  csOPCODEColliderSphere, csOpcodeCollider, iColliderSphere>
{
  IceMaths::Sphere shape;
  float radius;

public:
  csOPCODEColliderSphere(float radius);
  virtual ~csOPCODEColliderSphere();
  virtual ColliderType GetGeometryType() const {return COLLIDER_SPHERE;}
  virtual float GetSphereGeometry() {return radius;}
};

class csOpcodeColliderPlane:
  public scfImplementation2<csOpcodeColliderPlane,
  csOpcodeCollider, CS::Collision2::iColliderPlane>
{
  IceMaths::Plane opcodePlane;
  csPlane3 plane;

public:
  csBulletColliderPlane (const csPlane3& plane, csBulletSystem* sys);
  virtual ~csBulletColliderPlane ();
  virtual CS::Collision2::ColliderType GetGeometryType () const
  {return CS::Collision2::COLLIDER_PLANE;}
  virtual csPlane3 GetPlaneGeometry () {return plane;}
};


class csOPCODEColliderCapsule:
  public scfImplementation1<csOPCODEColliderCapsule, iColliderCapsule>
{
  IceMaths::LSS shape;
  float radius;
  float length;

public:
  csOPCODEColliderCapsule (float length, float radius);
  virtual ~csOPCODEColliderCapsule();
  virtual ColliderType GetGeometryType() const {return COLLIDER_CAPSULE;}
  virtual void SetMargin(float margin) {}
  virtual bool GetCapsuleGeometry(float& length, float& radius);
};

class csOPCODEColliderConcaveMesh:
  public scfImplementation1<csOPCODEColliderConcaveMesh, iColliderConcaveMesh>
{
  iMeshWrapper* meshFactory;
  Opcode::MeshInterface opcMeshInt;
  Opcode::OPCODECREATE OPCC;
  size_t* indexHolder;
  csDirtyAccessArray<Point> vertices;
public:
  csOPCODEColliderConcaveMesh(iMeshWrapper* mesh);
  virtual ~csOPCODEColliderConcaveMesh ();
  virtual ColliderType GetGeometryType () const {return COLLIDER_CONCAVE_MESH;}
  virtual iMeshFactoryWrapper* GetMeshFactory () {return meshFactory;}
};*/


class TerrainCellCollider
{
  friend class csOpcodeCollisionObject;
public:
  Opcode::MeshInterface opcMeshInt;
  Opcode::Model* model;
  iTerrainCell* cell;
  unsigned int* indexholder;
  Point *vertholder;
  csOrthoTransform cellTransform;
  TerrainCellCollider(iTerrainCell* cell, csOrthoTransform trans);
  virtual ~TerrainCellCollider();
  //Do not support update.
};

class csOpcodeColliderTerrain:
  public scfImplementation2<csOpcodeColliderTerrain,
  CS::Collision2::iColliderTerrain, iTerrainCellLoadCallback>
{
  friend class csOpcodeCollisionObject;
  friend class csOpcodeCollisionSector;

  csArray<TerrainCellCollider*> colliders;
  iTerrainSystem* terrainSystem;
  csOpcodeCollisionSystem* system;
  csOrthoTransform terrainTransform;
  float volume;
  bool unload;
  csVector3 scale;

public:
  csOpcodeColliderTerrain(iTerrainSystem* terrain, csOpcodeCollisionSystem* sys);
  virtual ~csOpcodeColliderTerrain();
  virtual CS::Collision2::ColliderType GetGeometryType() const {return CS::Collision2::COLLIDER_TERRAIN;}
  virtual iTerrainSystem* GetTerrain() const {return terrainSystem;}
  virtual void SetLocalScale (const csVector3& scale) {}
  virtual const csVector3& GetLocalScale () const {return scale;}
  virtual void SetMargin (float margin) {}
  virtual float GetMargin () const {return 0;}
  virtual float GetVolume () const {return volume;}

  virtual void OnCellLoad (iTerrainCell *cell);
  virtual void OnCellPreLoad (iTerrainCell *cell);
  virtual void OnCellUnload (iTerrainCell *cell);
  Opcode::Model* GetColliderModel (size_t index) {return colliders[index]->model;}
  csOrthoTransform GetColliderTransform (size_t index) {return colliders[index]->cellTransform;}
  Point* GetVertexHolder (size_t index) {return colliders[index]->vertholder;}
  udword* GetIndexHolder (size_t index) {return colliders[index]->indexholder;}
  void LoadCellToCollider (iTerrainCell* cell);
};
}
CS_PLUGIN_NAMESPACE_END (Opcode2)
#endif