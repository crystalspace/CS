#ifndef __CS_BULLET_COLLIDERS_H__
#define __CS_BULLET_COLLIDERS_H__

#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"
#include "csgeom/plane3.h"
#include "imesh/terrain2.h"
#include "ivaria/collision2.h"
#include "common2.h"

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

//using namespace CS::Collision;
struct iTriangleMesh;

CS_PLUGIN_NAMESPACE_BEGIN(Bullet2)
{

using CS::Collision2::ColliderType;

class csBulletSector;
class csBulletSystem;

csRef<iTriangleMesh> FindColdetTriangleMesh (iMeshWrapper* mesh, 
                                             csStringID baseID, csStringID colldetID);

class csBulletCollider: public virtual CS::Collision2::iCollider
{
  friend class csBulletCollisionObject;
protected:
  btCollisionShape* shape;
  csVector3 scale;
  float margin;
  csBulletSystem* collSystem;

public:
  csBulletCollider ();
  virtual ~csBulletCollider() {}
  virtual ColliderType GetGeometryType () const = 0;
  virtual void SetLocalScale (const csVector3& scale);
  virtual const csVector3& GetLocalScale () const {return scale;}
  virtual void SetMargin (float margin);
  virtual float GetMargin () const;
  virtual float GetVolume () = 0;
};

class csBulletColliderBox: 
  public scfImplementation2<csBulletColliderBox,
  csBulletCollider, CS::Collision2::iColliderBox>
{
  csVector3 boxSize;

public:
  csBulletColliderBox (const csVector3& boxSize, csBulletSystem* sys);
  virtual ~csBulletColliderBox ();
  virtual ColliderType GetGeometryType () const {return ColliderType::COLLIDER_BOX;}
  virtual csVector3 GetBoxGeometry () {return boxSize;}
  virtual float GetVolume () {return boxSize.x * boxSize.y * boxSize.z;}
};

class csBulletColliderSphere:
  public scfImplementation2<csBulletColliderSphere,
  csBulletCollider, CS::Collision2::iColliderSphere>
{
  float radius;

public:
  csBulletColliderSphere (float radius, csBulletSystem* sys);
  virtual ~csBulletColliderSphere ();
  virtual ColliderType GetGeometryType () const {return ColliderType::COLLIDER_SPHERE;}
  //Lulu: Implement this for sphere? Bullet do not support local scale of sphere.
  virtual void SetLocalScale (const csVector3& scale);
  virtual float GetSphereGeometry () {return radius;}
  virtual float GetVolume () {return 1.333333f * PI * radius * radius * radius;}
};

class csBulletColliderCylinder:
  public scfImplementation2<csBulletColliderCylinder,
  csBulletCollider, CS::Collision2::iColliderCylinder>
{
  //why Z?
  float radius;
  float length;

public:
  csBulletColliderCylinder (float length, float radius, csBulletSystem* sys);
  virtual ~csBulletColliderCylinder ();
  virtual ColliderType GetGeometryType () const {return ColliderType::COLLIDER_CYLINDER;}
  virtual void GetCylinderGeometry (float& length, float& radius);
  virtual float GetVolume () {return PI * radius * radius * length;}
};

class csBulletColliderCapsule: 
  public scfImplementation2<csBulletColliderCapsule,
  csBulletCollider, CS::Collision2::iColliderCapsule>
{
  float radius;
  float length;

public:
  csBulletColliderCapsule (float length, float radius, csBulletSystem* sys);
  virtual ~csBulletColliderCapsule ();
  virtual ColliderType GetGeometryType () const {return ColliderType::COLLIDER_CAPSULE;}
  virtual void GetCapsuleGeometry (float& length, float& radius);
  virtual float GetVolume () {return PI * radius * radius * length 
    + 1.333333f * PI * radius * radius * radius;}
};

class csBulletColliderCone:
  public scfImplementation2<csBulletColliderCone,
  csBulletCollider, CS::Collision2::iColliderCone>
{
  float radius;
  float length;

public:
  csBulletColliderCone (float length, float radius, csBulletSystem* sys);
  virtual ~csBulletColliderCone ();
  virtual ColliderType GetGeometryType () const {return ColliderType::COLLIDER_CONE;}
  virtual void GetConeGeometry (float& length, float& radius);
  virtual float GetVolume () {return 0.333333f * PI * radius * radius * length;}
};

class csBulletColliderPlane:
  public scfImplementation2<csBulletColliderPlane,
  csBulletCollider, CS::Collision2::iColliderPlane>
{
  csPlane3 plane;

public:
  csBulletColliderPlane (const csPlane3& plane, csBulletSystem* sys);
  virtual ~csBulletColliderPlane ();
  virtual ColliderType GetGeometryType () const {return ColliderType::COLLIDER_PLANE;}
  virtual csPlane3 GetPlaneGeometry () {return plane;}
  virtual float GetVolume () {return FLT_MAX;}
};

class csBulletColliderConvexMesh:
  public scfImplementation2<csBulletColliderConvexMesh,
  csBulletCollider, CS::Collision2::iColliderConvexMesh>
{
  iMeshWrapper* mesh;
  
public:
  csBulletColliderConvexMesh (iMeshWrapper* mesh, csBulletSystem* sys);
  csBulletColliderConvexMesh (btConvexHullShape* shape, csBulletSystem* sys) 
    : scfImplementationType (this, sys)
  {this->shape = shape;}
  virtual ~csBulletColliderConvexMesh ();
  virtual ColliderType GetGeometryType () const {return ColliderType::COLLIDER_CONVEX_MESH;}
  virtual iMeshWrapper* GetMesh () {return mesh;}
  virtual float GetVolume ();
};

class csBulletColliderConcaveMesh:
  public scfImplementation2<csBulletColliderConcaveMesh, 
  csBulletCollider, CS::Collision2::iColliderConcaveMesh>
{
  friend class csBulletColliderConcaveMeshScaled;

  iMeshWrapper* mesh;

public:
  csBulletColliderConcaveMesh (iMeshWrapper* mesh, csBulletSystem* sys);
  virtual ~csBulletColliderConcaveMesh ();
  virtual ColliderType GetGeometryType () const {return ColliderType::COLLIDER_CONCAVE_MESH;}
  virtual iMeshWrapper* GetMesh () {return mesh;}
  virtual float GetVolume ();
};

class csBulletColliderConcaveMeshScaled:
  public scfImplementation2<csBulletColliderConcaveMeshScaled,
  csBulletCollider, CS::Collision2::iColliderConcaveMeshScaled>
{
  csBulletColliderConcaveMesh* originalCollider;

public:
  csBulletColliderConcaveMeshScaled (CS::Collision2::iColliderConcaveMesh* collider, csVector3 scale, csBulletSystem* sys);
  virtual ~csBulletColliderConcaveMeshScaled();
  virtual ColliderType GetGeometryType () const {return ColliderType::COLLIDER_CONCAVE_MESH_SCALED;}
  virtual CS::Collision2::iColliderConcaveMesh* GetCollider () 
  {return dynamic_cast<CS::Collision2::iColliderConcaveMesh*>(originalCollider);}
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
  public scfImplementation3<csBulletColliderTerrain, 
  csBulletCollider, CS::Collision2::iColliderTerrain, iTerrainCellLoadCallback>
{
  friend class csBulletSector;
  friend class csBulletCollisionObject;
  
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
  virtual ColliderType GetGeometryType () const {return ColliderType::COLLIDER_TERRAIN;}
  virtual iTerrainSystem* GetTerrain () const {return terrainSystem;}
  //Lulu: Will set scale/margin to all the height map collider.

  virtual void SetLocalScale (const csVector3& scale);
  virtual void SetMargin (float margin);

  virtual void OnCellLoad (iTerrainCell *cell);
  virtual void OnCellPreLoad (iTerrainCell *cell);
  virtual void OnCellUnload (iTerrainCell *cell);
  virtual float GetVolume () {return FLT_MAX;}

  btRigidBody* GetBulletObject (size_t index) {return bodies[index];}
  void RemoveRigidBodies ();
  void AddRigidBodies (csBulletSector* sector);
};
}
CS_PLUGIN_NAMESPACE_END(Bullet2)
#endif