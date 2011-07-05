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

//using namespace CS::Collision2;

CS_PLUGIN_NAMESPACE_BEGIN(Bullet2)
{

//using CS::Collision2::ColliderType;

class csBulletSector;
class csBulletSystem;

csRef<iTriangleMesh> FindColdetTriangleMesh (iMeshWrapper* mesh, 
                                             csStringID baseID, csStringID colldetID);

class csBulletCollider: public virtual CS::Collision2::iCollider
{
  friend class csBulletCollisionObject;
  friend class csBulletCollisionActor;
  friend class csBulletRigidBody;
protected:
  btCollisionShape* shape;
  csVector3 scale;
  float margin;
  float volume;
  csBulletSystem* collSystem;

public:
  csBulletCollider ();
  virtual ~csBulletCollider() {}
  virtual CS::Collision2::ColliderType GetGeometryType () const = 0;
  virtual void SetLocalScale (const csVector3& scale);
  virtual const csVector3& GetLocalScale () const {return scale;}
  virtual void SetMargin (float margin);
  virtual float GetMargin () const;
  virtual float GetVolume () const {return volume;}
};

class csBulletColliderBox: 
  public scfImplementation2<csBulletColliderBox,
  csBulletCollider, CS::Collision2::iColliderBox>
{
  csVector3 boxSize;

public:
  csBulletColliderBox (const csVector3& boxSize, csBulletSystem* sys);
  virtual ~csBulletColliderBox ();
  virtual CS::Collision2::ColliderType GetGeometryType () const
  {return CS::Collision2::COLLIDER_BOX;}
  virtual csVector3 GetBoxGeometry () {return boxSize;}
};

class csBulletColliderSphere:
  public scfImplementation2<csBulletColliderSphere,
  csBulletCollider, CS::Collision2::iColliderSphere>
{
  float radius;

public:
  csBulletColliderSphere (float radius, csBulletSystem* sys);
  virtual ~csBulletColliderSphere ();
  virtual CS::Collision2::ColliderType GetGeometryType () const
  {return CS::Collision2::COLLIDER_SPHERE;}
  virtual void SetMargin (float margin);
  virtual float GetSphereGeometry () {return radius;}
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
  virtual CS::Collision2::ColliderType GetGeometryType () const
  {return CS::Collision2::COLLIDER_CYLINDER;}
  virtual void GetCylinderGeometry (float& length, float& radius);
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
  virtual CS::Collision2::ColliderType GetGeometryType () const
  {return CS::Collision2::COLLIDER_CAPSULE;}
  virtual void GetCapsuleGeometry (float& length, float& radius);
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
  virtual CS::Collision2::ColliderType GetGeometryType () const
  {return CS::Collision2::COLLIDER_CONE;}
  virtual void GetConeGeometry (float& length, float& radius);
};

class csBulletColliderPlane:
  public scfImplementation2<csBulletColliderPlane,
  csBulletCollider, CS::Collision2::iColliderPlane>
{
  csPlane3 plane;

public:
  csBulletColliderPlane (const csPlane3& plane, csBulletSystem* sys);
  virtual ~csBulletColliderPlane ();
  virtual CS::Collision2::ColliderType GetGeometryType () const
  {return CS::Collision2::COLLIDER_PLANE;}
  virtual csPlane3 GetPlaneGeometry () {return plane;}
  virtual void SetLocalScale (const csVector3& scale) {}
};

class csBulletColliderConvexMesh:
  public scfImplementation2<csBulletColliderConvexMesh,
  csBulletCollider, CS::Collision2::iColliderConvexMesh>
{
  iMeshWrapper* mesh;
  
public:
  csBulletColliderConvexMesh (iMeshWrapper* mesh, csBulletSystem* sys);
  csBulletColliderConvexMesh (btConvexHullShape* shape, float volume, csBulletSystem* sys) 
    : scfImplementationType (this)
  {
    this->shape = shape; 
    collSystem = sys;
    this->volume = volume;
  }
  virtual ~csBulletColliderConvexMesh ();
  virtual CS::Collision2::ColliderType GetGeometryType () const
 {return CS::Collision2::COLLIDER_CONVEX_MESH;}
  virtual iMeshWrapper* GetMesh () {return mesh;}
};

class csBulletColliderConcaveMesh:
  public scfImplementation2<csBulletColliderConcaveMesh, 
  csBulletCollider, CS::Collision2::iColliderConcaveMesh>
{
  friend class csBulletColliderConcaveMeshScaled;
  btTriangleMesh* triMesh;
  iMeshWrapper* mesh;

public:
  csBulletColliderConcaveMesh (iMeshWrapper* mesh, csBulletSystem* sys);
  virtual ~csBulletColliderConcaveMesh ();
  virtual CS::Collision2::ColliderType GetGeometryType () const
 {return CS::Collision2::COLLIDER_CONCAVE_MESH;}
  virtual iMeshWrapper* GetMesh () {return mesh;}
};

class csBulletColliderConcaveMeshScaled:
  public scfImplementation2<csBulletColliderConcaveMeshScaled,
  csBulletCollider, CS::Collision2::iColliderConcaveMeshScaled>
{
  csBulletColliderConcaveMesh* originalCollider;

public:
  csBulletColliderConcaveMeshScaled (CS::Collision2::iColliderConcaveMesh* collider, csVector3 scale, csBulletSystem* sys);
  virtual ~csBulletColliderConcaveMeshScaled();
  virtual CS::Collision2::ColliderType GetGeometryType () const
  {return CS::Collision2::COLLIDER_CONCAVE_MESH_SCALED;}
  virtual CS::Collision2::iColliderConcaveMesh* GetCollider () 
  {return dynamic_cast<CS::Collision2::iColliderConcaveMesh*>(originalCollider);}
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
  void SetLocalScale (const csVector3& scale);
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
  csBulletCollisionObject* collBody;
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
  virtual CS::Collision2::ColliderType GetGeometryType () const {return CS::Collision2::COLLIDER_TERRAIN;}
  virtual iTerrainSystem* GetTerrain () const {return terrainSystem;}
  //Lulu: Will set scale/margin to all the height map collider.

  virtual void SetLocalScale (const csVector3& scale);
  virtual void SetMargin (float margin);

  virtual void OnCellLoad (iTerrainCell *cell);
  virtual void OnCellPreLoad (iTerrainCell *cell);
  virtual void OnCellUnload (iTerrainCell *cell);

  btRigidBody* GetBulletObject (size_t index) {return bodies[index];}
  void RemoveRigidBodies ();
  void AddRigidBodies (csBulletSector* sector, csBulletCollisionObject* body);
};
}
CS_PLUGIN_NAMESPACE_END(Bullet2)
#endif
