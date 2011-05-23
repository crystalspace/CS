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

class csBulletCollisionSector;

class csBulletCollider:
  public scfImplementation1<csBulletCollider, CS::Collision::iCollider>
{
  friend class csBulletCollisionObject;
protected:
  btCollisionShape* shape;
  csVector3 scale;
  float margin;
  csBulletCollisionSector* collSector;

public:
  csBulletCollider ();
  virtual ~csBulletCollider();
  virtual ColliderType GetGeometryType () {return COLLIDER_INVALID;}
  virtual void SetLocalScale (const csVector3& scale);
  virtual const csVector3& GetLocalScale () const {return scale;}
  virtual void SetMargin (float margin);
  virtual float GetMargin () const;
  virtual void GenerateShape () = 0;
  void SetSector(csBulletCollisionSector* sector) {collSector = sector;}
};

class csBulletColliderBox: 
  public scfImplementationExt1<csBulletColliderBox,
  csBulletCollider, iColliderBox>
{
  csVector3 boxSize;

public:
  csBulletColliderBox (csBulletCollisionSector* sector, const csVector3& boxSize);
  virtual ~csBulletColliderBox ();
  virtual ColliderType GetGeometryType () {return COLLIDER_BOX;}
  virtual void GenerateShape ();
  virtual csVector3 GetBoxGeometry () {return boxSize;}
};

class csBulletColliderSphere:
  public scfImplementationExt1<csBulletColliderSphere,
  csBulletCollider, iColliderSphere>
{
  float radius;

public:
  csBulletColliderSphere (csBulletCollisionSector* sector, float radius);
  virtual ~csBulletColliderSphere ();
  virtual ColliderType GetGeometryType () {return COLLIDER_SPHERE;}
  virtual void GenerateShape ();
  //Lulu: Implement this for sphere? Bullet do not support local scale of sphere.
  virtual void SetLocalScale (const csVector3& scale);
  virtual float GetSphereGeometry () {return radius;}
};

class csBulletColliderCylinder:
  public scfImplementationExt1<csBulletColliderCylinder,
  csBulletCollider, iColliderCylinder>
{
  //why Z?
  float radius;
  float length;

public:
  csBulletColliderCylinder (csBulletCollisionSector* sector, float length, float radius);
  virtual ~csBulletColliderCylinder ();
  virtual ColliderType GetGeometryType () {return COLLIDER_CYLINDER;}
  virtual void GenerateShape ();
  virtual void GetCylinderGeometry (float& length, float& radius);
};

class csBulletColliderCapsule: 
  public scfImplementationExt1<csBulletColliderCapsule,
  csBulletCollider, iColliderCapsule>
{
  float radius;
  float length;

public:
  csBulletColliderCapsule (csBulletCollisionSector* sector, float length, float radius);
  virtual ~csBulletColliderCapsule ();
  virtual void GetCapsuleGeometry (float& length, float& radius);
  virtual void GenerateShape ();
};

class csBulletColliderCone:
  public scfImplementationExt1<csBulletColliderCone,
  csBulletCollider, iColliderCone>
{
  float radius;
  float length;

public:
  csBulletColliderCone (csBulletCollisionSector* sector, float length, float radius);
  virtual ~csBulletColliderCone ();
  virtual ColliderType GetGeometryType () {return COLLIDER_CONE;}
  virtual void GenerateShape ();
  virtual void GetConeGeometry (float& length, float& radius);
};

class csBulletColliderPlane:
  public scfImplementationExt1<csBulletColliderPlane,
  csBulletCollider, iColliderPlane>
{
  csPlane3 plane;

public:
  csBulletColliderPlane (csBulletCollisionSector* sector, const csPlane3& plane);
  virtual ~csBulletColliderPlane ();
  virtual ColliderType GetGeometryType () {return COLLIDER_PLANE;}
  virtual void GenerateShape ();
  virtual csPlane3 GetPlaneGeometry () {return plane;}
};

class csBulletColliderConvexMesh:
  public scfImplementationExt1<csBulletColliderConvexMesh,
  csBulletCollider, iColliderConvexMesh>
{
  iMeshWrapper* mesh;
  
public:
  csBulletColliderConvexMesh (csBulletCollisionSector* sector, iMeshWrapper* mesh);
  virtual ~csBulletColliderConvexMesh ();
  virtual ColliderType GetGeometryType () {return COLLIDER_CONVEX_MESH;}
  virtual void GenerateShape ();
  virtual iMeshWrapper* GetMesh () {return mesh;}
};

class csBulletColliderConcaveMesh:
  public scfImplementationExt1<csBulletColliderConcaveMesh, 
  csBulletCollider, iColliderConcaveMesh>
{
  friend class csBulletColliderConcaveMeshScaled;

  iMeshWrapper* mesh;

public:
  csBulletColliderConcaveMesh (csBulletCollisionSector* sector, iMeshWrapper* mesh);
  virtual ~csBulletColliderConcaveMesh ();
  virtual ColliderType GetGeometryType () {return COLLIDER_CONCAVE_MESH;}
  virtual void GenerateShape ();
  virtual iMeshWrapper* GetMeshFactory () {return mesh;}
};

class csBulletColliderConcaveMeshScaled:
  public scfImplementationExt1<csBulletColliderConcaveMeshScaled,
  csBulletCollider, iColliderConcaveMeshScaled>
{
  csBulletColliderConcaveMesh* originalCollider;

public:
  csBulletColliderConcaveMeshScaled (csBulletCollisionSector* sector, iColliderConcaveMesh* collider, csVector3 scale);
  virtual ~csBulletColliderConcaveMeshScaled() = 0;
  virtual ColliderType GetGeometryType () {return COLLIDER_CONCAVE_MESH_SCALED;}
  virtual void GenerateShape ();
  virtual iColliderConcaveMesh* GetCollider () {return originalCollider;}
};

class HeightMapCollider : public btHeightfieldTerrainShape
{
  
public:
  iTerrainCell* cell;
  csVector3 cellPosition;
  btVector3 localScale;
  float* heightData;

  HeightMapCollider (csBulletCollisionSector* sector,
    float* gridData,
    int gridWidth, int gridHeight,
    csVector3 gridSize,
    csVector3 position,
    float minHeight, float maxHeight);
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
  csBulletCollisionSector* collSector;
  iTerrainSystem* terrainSystem;
  float minimumHeight;
  float maximumHeight;
  bool unload;

  void LoadCellToCollider(iTerrainCell* cell);
public:
  csBulletColliderTerrain (csBulletCollisionSector* sector,
    iTerrainSystem* terrain,
    float minimumHeight,
    float maximumHeight);
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

  const csVector3& GetCellPosition (size_t index);
};
}
CS_PLUGIN_NAMESPACE_END(Bullet2)
#endif