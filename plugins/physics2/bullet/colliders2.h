#ifndef __CS_BULLET_COLLIDERS_H__
#define __CS_BULLET_COLLIDERS_H__

#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"
#include "csgeom/plane3.h"
#include "imesh/terrain2.h"
#include "ivaria/collision2.h"
#include "csutil/csobject.h"
//#include "bulletcollision.h"
#include "common.h"

class btBoxShape;
class btSphereShape;
class btCylinderShapeZ;
class btCapsuleShapeZ;
class btConeShapeZ;
class btStaticPlaneShape;
class btConvexHullShape;
class btCollisionShape;
class btScaledBvhTriangleMeshShape;
class btBvhTriangleMeshShape;
class btGImpactMeshShape;
class btTriangleMesh;
struct csLockedHeightData;
struct iTerrainSystem;

using namespace CS::Collision;

CS_PLUGIN_NAMESPACE_BEGIN(Bullet2)
{

class csBulletSector;

csRef<iTriangleMesh> FindColdetTriangleMesh (iMeshWrapper* mesh, 
                                             csStringID baseID, csStringID colldetID);

class csBulletCollider:
  public scfImplementation1<csBulletCollider, CS::Collision::iCollider>
{
  friend class csBulletCollisionObject;
protected:
  btCollisionShape* shape;
  csVector3 scale;
  float margin;
  csBulletSystem* collSystem;

public:
  csBulletCollider (csBulletSystem* sys) {collSystem = sys;}
  virtual ~csBulletCollider();
  virtual ColliderType GetGeometryType () {return COLLIDER_INVALID;}
  virtual void SetLocalScale (const csVector3& scale);
  virtual const csVector3& GetLocalScale () const {return scale;}
  virtual void SetMargin (float margin);
  virtual float GetMargin () const;
  virtual float GetVolume () = 0;
};

class csBulletColliderBox: 
  public scfImplementationExt1<csBulletColliderBox,
  csBulletCollider, iColliderBox>
{
  csVector3 boxSize;

public:
  csBulletColliderBox (const csVector3& boxSize, csBulletSystem* sys);
  virtual ~csBulletColliderBox ();
  virtual ColliderType GetGeometryType () {return COLLIDER_BOX;}
  virtual void GenerateShape ();
  virtual csVector3 GetBoxGeometry () {return boxSize;}
  virtual float GetVolume () {return boxSize.x * boxSize.y * boxSize.z;}
};

class csBulletColliderSphere:
  public scfImplementationExt1<csBulletColliderSphere,
  csBulletCollider, iColliderSphere>
{
  float radius;

public:
  csBulletColliderSphere (float radius, csBulletSystem* sys);
  virtual ~csBulletColliderSphere ();
  virtual ColliderType GetGeometryType () {return COLLIDER_SPHERE;}
  virtual void GenerateShape ();
  //Lulu: Implement this for sphere? Bullet do not support local scale of sphere.
  virtual void SetLocalScale (const csVector3& scale);
  virtual float GetSphereGeometry () {return radius;}
  virtual float GetVolume () {1.333333f * PI * radius * radius * radius;}
};

class csBulletColliderCylinder:
  public scfImplementationExt1<csBulletColliderCylinder,
  csBulletCollider, iColliderCylinder>
{
  //why Z?
  float radius;
  float length;

public:
  csBulletColliderCylinder (float length, float radius, csBulletSystem* sys);
  virtual ~csBulletColliderCylinder ();
  virtual ColliderType GetGeometryType () {return COLLIDER_CYLINDER;}
  virtual void GenerateShape ();
  virtual void GetCylinderGeometry (float& length, float& radius);
  virtual float GetVolume () {return PI * radius * radius * length;}
};

class csBulletColliderCapsule: 
  public scfImplementationExt1<csBulletColliderCapsule,
  csBulletCollider, iColliderCapsule>
{
  float radius;
  float length;

public:
  csBulletColliderCapsule (float length, float radius, csBulletSystem* sys);
  virtual ~csBulletColliderCapsule ();
  virtual void GetCapsuleGeometry (float& length, float& radius);
  virtual void GenerateShape ();
  virtual float GetVolume () {return PI * radius * radius * length 
    + 1.333333f * PI * radius * radius * radius;}
};

class csBulletColliderCone:
  public scfImplementationExt1<csBulletColliderCone,
  csBulletCollider, iColliderCone>
{
  float radius;
  float length;

public:
  csBulletColliderCone (float length, float radius, csBulletSystem* sys);
  virtual ~csBulletColliderCone ();
  virtual ColliderType GetGeometryType () {return COLLIDER_CONE;}
  virtual void GenerateShape ();
  virtual void GetConeGeometry (float& length, float& radius);
  virtual float GetVolume () {0.333333f * PI * radius * radius * length;}
};

class csBulletColliderPlane:
  public scfImplementationExt1<csBulletColliderPlane,
  csBulletCollider, iColliderPlane>
{
  csPlane3 plane;

public:
  csBulletColliderPlane (const csPlane3& plane, csBulletSystem* sys);
  virtual ~csBulletColliderPlane ();
  virtual ColliderType GetGeometryType () {return COLLIDER_PLANE;}
  virtual void GenerateShape ();
  virtual csPlane3 GetPlaneGeometry () {return plane;}
  virtual float GetVolume () {return FLT_MAX;}
};

class csBulletColliderConvexMesh:
  public scfImplementationExt1<csBulletColliderConvexMesh,
  csBulletCollider, iColliderConvexMesh>
{
  iMeshWrapper* mesh;
  
public:
  csBulletColliderConvexMesh (iMeshWrapper* mesh, csBulletSystem* sys);
  csBulletColliderConvexMesh (btConvexHullShape* shape) {this->shape = shape;}
  virtual ~csBulletColliderConvexMesh ();
  virtual ColliderType GetGeometryType () {return COLLIDER_CONVEX_MESH;}
  virtual void GenerateShape ();
  virtual iMeshWrapper* GetMesh () {return mesh;}
  virtual float GetVolume ();
};

class csBulletColliderConcaveMesh:
  public scfImplementationExt1<csBulletColliderConcaveMesh, 
  csBulletCollider, iColliderConcaveMesh>
{
  friend class csBulletColliderConcaveMeshScaled;

  iMeshWrapper* mesh;

public:
  csBulletColliderConcaveMesh (iMeshWrapper* mesh, csBulletSystem* sys);
  virtual ~csBulletColliderConcaveMesh ();
  virtual ColliderType GetGeometryType () {return COLLIDER_CONCAVE_MESH;}
  virtual void GenerateShape ();
  virtual iMeshWrapper* GetMeshFactory () {return mesh;}
  virtual float GetVolume ();
};

class csBulletColliderConcaveMeshScaled:
  public scfImplementationExt1<csBulletColliderConcaveMeshScaled,
  csBulletCollider, iColliderConcaveMeshScaled>
{
  csBulletColliderConcaveMesh* originalCollider;

public:
  csBulletColliderConcaveMeshScaled (iColliderConcaveMesh* collider, csVector3 scale, csBulletSystem* sys);
  virtual ~csBulletColliderConcaveMeshScaled() = 0;
  virtual ColliderType GetGeometryType () {return COLLIDER_CONCAVE_MESH_SCALED;}
  virtual void GenerateShape ();
  virtual iColliderConcaveMesh* GetCollider () {return originalCollider;}
  virtual float GetVolume () {return originalCollider->GetVolume () * scale.x * scale.y * scale.z;}
};

//TODO: modify the terrain collider.
class HeightMapCollider : public btHeightfieldTerrainShape
{
  
public:
  iTerrainCell* cell;
  btVector3 localScale;
  float* heightData;

  HeightMapCollider (float* gridData,
    int gridWidth, int gridHeight, 
    csVector3 gridSize,
    float minHeight, float maxHeight,
    float internalScale);
  virtual ~HeightMapCollider();
  void UpdataMinHeight (float minHeight);
  void UpdateMaxHeight (float maxHeight);
  void SetLocalScale (csVector3& scale);
};

class csBulletColliderTerrain:
  public scfImplementationExt2<csBulletColliderTerrain, 
  csBulletCollider, iColliderTerrain, iTerrainCellLoadCallback>
{
  
  csArray<HeightMapCollider*> colliders;
  csArray<btRigidBody*> bodies;
  csBulletSector* collSector;
  csBulletSystem* collSystem;
  iTerrainSystem* terrainSystem;
  csOrthoTransform terrainTransform;
  float minimumHeight;
  float maximumHeight;
  bool unload;

  void LoadCellToCollider(iTerrainCell* cell);
public:
  csBulletColliderTerrain (iTerrainSystem* terrain,
    float minimumHeight, float maximumHeight,
    csBulletSystem* sys);
  virtual ~csBulletColliderTerrain ();
  virtual ColliderType GetGeometryType () {return COLLIDER_TERRAIN;}
  virtual void GenerateShape ();
  virtual iTerrainSystem* GetTerrain () const {return terrainSystem;}
  //Lulu: Will set scale/margin to all the height map collider.

  virtual void SetLocalScale (const csVector3& scale);
  virtual void SetMargin (float margin);

  virtual void OnCellLoad (iTerrainCell *cell);
  virtual void OnCellPreLoad (iTerrainCell *cell);
  virtual void OnCellUnload (iTerrainCell *cell);
  virtual float GetVolume () {return FLT_MAX;}

  btRigidBody* GetBulletObject (size_t index);
  const csVector3& GetCellPosition (size_t index);
  void RemoveRigidBodies ();
  void AddRigidBodies (csBulletSector* sector);
};
}
CS_PLUGIN_NAMESPACE_END(Bullet2)
#endif