#ifndef __CS_IVARIA_COLLISION2_H__
#define __CS_IVARIA_COLLISION2_H__

#include "csutil/scf.h"
#include "csutil/csstring.h"
#include "csgeom/vector3.h"

class csMatrix3;
class csOrthoTransform;
class csPlane3;
class csString;
struct iTerrainSystem;
struct iSector;
struct iMeshWrapper;
struct iMovable;
struct iPortal;
struct iView;

namespace CS
{
namespace Collision
{

struct csConvexResult;
struct iCollisionCallback;
struct iCollisionObject;
typedef size_t CollisionGroupMask;

enum ColliderType
{
COLLIDER_INVALID = 0,
COLLIDER_BOX,
COLLIDER_SPHERE,
COLLIDER_CYLINDER,
COLLIDER_CAPSULE,
COLLIDER_CONE,
COLLIDER_PLANE,
COLLIDER_CONVEX_MESH,
COLLIDER_CONCAVE_MESH,
COLLIDER_CONCAVE_MESH_SCALED,        
COLLIDER_TERRAIN, 
};

enum CollisionObjectType
{
COLLISION_OBJECT_BASE = 0,
COLLISION_OBJECT_GHOST,
COLLISION_OBJECT_ACTOR,
COLLISION_OBJECT_TERRAIN,
};

struct CollisionGroup
{
  csString name;
  CollisionGroupMask value;
};

struct MoveResult
{
  bool hasHit;
  csVector3 hitNormalWorld;
  csVector3 hitPointWorld;
  csVector3 hitNormalLocal;
  csVector3 hitPointLocal;
};

struct HitBeamResult
{
  HitBeamResult ()
  : hasHit (false), object (0), isect (0.0f), normal (0.0f), vertexIndex (0)
  {}

  /// Whether the beam has hit a body or not.
  bool hasHit;

  /// The collision object that was hit, or \a nullptr if no object was hit.
  iCollisionObject* object;

  /// Intersection point in world space.
  csVector3 isect;

  /// Normal to the surface of the body at the intersection point.
  csVector3 normal;

  /**
  * The index of the closest vertex of the soft body to be hit. This is only valid
  * if it is a soft body which is hit.
  */
  size_t vertexIndex;
};

struct CollisionData
{
  csVector3 position; // in world coordinates? in object coordinates for both bodies? 
  csVector3 penetration; 
};

struct iCollisionCallback: public virtual iBase
{
  SCF_INTERFACE (iCollisionCallback, 1, 0, 0);

  /**
  * A collision occurred.
  * \param thisbody The body that received a collision.
  * \param otherbody The body that collided with \a thisBody.
  * \param collisions The list of collisions between the two bodies. 
  * \param timesteps Since how many simulation time steps this collision occured. 
  */
  virtual void OnCollision (iCollisionObject *thisbody, iCollisionObject *otherbody, 
      const csArray<CollisionData>& collisions, size_t timesteps) = 0; 
};

/**
* A base interface for colliders. 
* Other colliders will be derived from this one.
*/
struct iCollider : public virtual iBase
{
  SCF_INTERFACE (CS::Collision::iCollider, 1, 0, 0);

  /// Get the geometry type of this collider. 
  virtual ColliderType GetGeometryType () const = 0;

  virtual void SetLocalScale (const csVector3& scale) = 0;

  virtual const csVector3& GetLocalScale () const = 0;
  
  /// Set the margin of collision shape.
  virtual void SetMargin (float margin) = 0;

  virtual float GetMargin () const = 0; 
};

/**
* A box collider.
*
* Main creators of instances implementing this interface:
* - iCollisionSystem::CreateColliderBox()
* 
* Main ways to get pointers to this interface:
* - iCollisionObject::GetCollider()
* 
* Main users of this interface:
* - iCollisionObject
*/
struct iColliderBox : public virtual iCollider
{
  SCF_INTERFACE (CS::Collision::iColliderBox, 1, 0, 0);

  /// Get the box geometry of this collider.
  virtual csVector3 GetBoxGeometry ()  = 0;
};

/**
* A sphere collider.
*
* Main creators of instances implementing this interface:
* - iCollisionSystem::CreateColliderSphere()
* 
* Main ways to get pointers to this interface:
* - iCollisionObject::GetCollider()
* 
* Main users of this interface:
* - iCollisionObject
*/
struct iColliderSphere : public virtual iCollider
{
  SCF_INTERFACE (CS::Collision::iColliderSphere, 1, 0, 0);

  /// Get the sphere geometry of this collider.
  virtual float GetSphereGeometry () = 0;
};

/**
* A cylinder collider.
*
* Main creators of instances implementing this interface:
* - iCollisionSystem::CreateColliderCylinder()
* 
* Main ways to get pointers to this interface:
* - iCollisionObject::GetCollider()
* 
* Main users of this interface:
* - iCollisionObject
*/
struct iColliderCylinder : public virtual iCollider
{
  SCF_INTERFACE (CS::Collision::iColliderCylinder, 1, 0, 0);

  /// Get the cylinder geometry of this collider.
  virtual void GetCylinderGeometry (float& length, float& radius) = 0;
};

/**
* A capsule collider.
*
* Main creators of instances implementing this interface:
* - iCollisionSystem::CreateColliderCapsule()
* 
* Main ways to get pointers to this interface:
* - iCollisionObject::GetCollider()
* 
* Main users of this interface:
* - iCollisionObject
*/
struct iColliderCapsule : public virtual iCollider
{
  SCF_INTERFACE (CS::Collision::iColliderCapsule, 1, 0, 0);

  /// Get the capsule geometry of this collider.
  virtual void GetCapsuleGeometry (float& length, float& radius) = 0;
};

/**
* A cone collider.
*
* Main creators of instances implementing this interface:
* - iCollisionSystem::CreateColliderCone()
* 
* Main ways to get pointers to this interface:
* - iCollisionObject::GetCollider()
* 
* Main users of this interface:
* - iCollisionObject
*/
struct iColliderCone : public virtual iCollider
{
  SCF_INTERFACE (CS::Collision::iColliderCone, 1, 0, 0);

  /// Get the capsule geometry of this collider.
  virtual void GetConeGeometry (float& length, float& radius) = 0;
};

/**
* A plane collider.
*
* Main creators of instances implementing this interface:
* - iCollisionSystem::CreateColliderPlane()
* 
* Main ways to get pointers to this interface:
* - iCollisionObject::GetCollider()
* 
* Main users of this interface:
* - iCollisionObject
*/
struct iColliderPlane : public virtual iCollider
{
  SCF_INTERFACE (CS::Collision::iColliderPlane, 1, 0, 0);

  /// Get the plane geometry of this collider.
  virtual csPlane3 GetPlaneGeometry () = 0;
};

/**
* A convex mesh collider.
*
* Main creators of instances implementing this interface:
* - iCollisionSystem::CreateColliderConvexMesh()
* 
* Main ways to get pointers to this interface:
* - iCollisionObject::GetCollider()
* 
* Main users of this interface:
* - iCollisionObject
*/
struct iColliderConvexMesh : public virtual iCollider
{
  SCF_INTERFACE (CS::Collision::iColliderConvexMesh, 1, 0, 0);

  /// Get the mesh factory of this collider.
  virtual iMeshWrapper* GetMesh () = 0;
};

/**
* A compound shape collider.
*
* Main creators of instances implementing this interface:
* - iCollisionSystem::CreateColliderCompound()
* 
* Main ways to get pointers to this interface:
* - iCollisionObject::GetCollider()
* 
* Main users of this interface:
* - iCollisionObject
*/
// kickvb: to be removed
// Lulu: I don't know whether I'm thinking in a correct way...
//       So for concave object, if we call CreateColliderConcaveMesh(...), 
//       it will always return a iCollisionConcaveMesh?
//       In this case, we call SetAutoDecompose(true), iColliderConcaveMesh will 
//       create a btCompoundShape. User cannot add more collision shape into the
//       collider because iColliderConcaveMesh doesn't have this functionality.
//
//       And we also have a compound collider class, it also create a btCompoundShape.
//       This class support adding colliders and it's used internal. When user add more
//       than 1 colliders into a collision object this compound collider will be used.
//       
//       Is it correct?
//struct iCompoundCollider : public virtual iCollider
//{
//    SCF_INTERFACE (CS::Collision::iCompoundCollider, 1, 0, 0);
//
//    /**
//    * Add a child collider to the compound collider.
//    */
//    virtual bool AddChildCollider(iCollider* child,
//        const csOrthoTransform& trans); = 0;
//
//    /**
//    * Get a child collider by index.
//    */
//    virtual csRef<iCollider> GetChildCollider(size_t index) = 0;
//
//    /**
//    * Get the count of child colliders.
//    */
//    virtual int GetChildColliderCount() = 0;
//
//    /**
//    * Remove a child collider by index.
//    */
//    virtual bool RemoveChildCollider(size_t index) = 0;
//};

/**
* A concave mesh collider.
*
* Main creators of instances implementing this interface:
* - iCollisionSystem::CreateColliderConcaveMesh()
* 
* Main ways to get pointers to this interface:
* - iCollisionObject::GetCollider()
* 
* Main users of this interface:
* - iCollisionObject
*/
struct iColliderConcaveMesh : public virtual iCollider
{
  SCF_INTERFACE (CS::Collision::iColliderConcaveMesh, 1, 0, 0);

  /// Get the mesh factory of this collider.
  virtual iMeshWrapper* GetMesh () = 0;
};

/**
* A scaled concave mesh collider.
*
* Main creators of instances implementing this interface:
* - iCollisionSystem::CreateColliderConcaveMeshScaled()
* 
* Main ways to get pointers to this interface:
* - iCollisionObject::GetCollider()
* 
* Main users of this interface:
* - iCollisionObject
*/
struct iColliderConcaveMeshScaled : public virtual iCollider
{
  SCF_INTERFACE (CS::Collision::iColliderConcaveMeshScaled, 1, 0, 0);

  /// Get the concave collider scaled by this collider.
  virtual iColliderConcaveMesh* GetCollider () = 0;
};

/**
* A terrain collider.
*
* Main creators of instances implementing this interface:
* - iCollisionSystem::CreateColliderTerrain()
* 
* Main ways to get pointers to this interface:
* - iCollisionObject::GetCollider()
* 
* Main users of this interface:
* - iCollisionObject
*/
struct iColliderTerrain : public virtual iCollider
{
  SCF_INTERFACE (CS::Collision::iColliderTerrain, 1, 0, 0);

  /// Get the terrain system.
  virtual iTerrainSystem* GetTerrain () const = 0;
};

/**
* This is the interface of a collision object. 
* It contains the collision information of the object.
*
* Main creators of instances implementing this interface:
* - iCollisionSystem::CreateCollisionObject()
* 
* Main ways to get pointers to this interface:
* - iCollisionSystem::GetCollisionObject()
* 
* Main users of this interface:
* - iCollisionSystem
*/
struct iCollisionObject : public virtual iBase
{
  SCF_INTERFACE (CS::Collision::iCollisionObject, 1, 0, 0);

  /// Set the type of the collision object.
  virtual void SetObjectType (CollisionObjectType type) = 0;

  /// Return the type of the collision object.
  virtual CollisionObjectType GetObjectType () = 0;

  /**
  * Set the movable attached to this collision object. Its position will be updated
  * automatically when this object is moved.
  */
  virtual void SetAttachedMovable (iMovable* movable) = 0;

  /// Get the movable attached to this collision object.
  virtual iMovable* GetAttachedMovable () = 0;
  
  /// Set the transform.
  virtual void SetTransform (const csOrthoTransform& trans) = 0;

  /// Get the transform.
  virtual csOrthoTransform GetTransform () = 0;

  /// Add a collider to this collision body.
  virtual void AddCollider (iCollider* collider, const csOrthoTransform& relaTrans) = 0;

  /// Remove the given collider from this collision object.
  virtual void RemoveCollider (iCollider* collider) = 0;

  /// Remove the collider with the given index from this collision object.
  virtual void RemoveCollider (size_t index) = 0;

  /// Get the collider with the given index.
  virtual iCollider* GetCollider (size_t index) = 0;

  /// Get the count of colliders in this collision object.
  virtual size_t GetColliderCount () = 0;

  /// Rebuild this collision object.
  virtual void RebuildObject () = 0;

  /// Set the collision group this object belongs to.
  virtual void SetCollisionGroup (const char* name);

  /// Get the collision group this object belongs to.
  virtual const char* GetCollisionGroup () const;

  /**
  * Set a callback to be executed when this body collides with another.
  * If 0, no callback is executed.
  */
  virtual void SetCollisionCallback (iCollisionCallback* cb) = 0;

  /// Get the collision response callback.
  virtual iCollisionCallback* GetCollisionCallback () = 0;

  /// Test collision with another collision objects.
  virtual bool Collide (iCollisionObject* otherObject) = 0;

  /// Follow a beam from start to end and return whether this body was hit.
  virtual HitBeamResult HitBeam (
      const csVector3& start, const csVector3& end) = 0;
};

/**
* A iCollisionActor is a kinematic collision object. It has a faster collision detection and response.
* You can use it to create a player or character model with gravity handling.
*
* Main creators of instances implementing this interface:
* - iCollisionSystem::CreateCollisionActor()
* 
* Main ways to get pointers to this interface:
* - iCollisionSystem::GetCollisionActor()
* 
* Main users of this interface:
* - iCollisionSystem
*/
// kickvb: most of this would have to be redesigned, let's do it later
struct iCollisionActor : public virtual iCollisionObject
{
  SCF_INTERFACE (CS::Collision::iCollisionActor, 1, 0, 0);

  /// Check if we are on the ground.
  virtual bool IsOnGround () = 0;

  /// Set the onground status.
  //virtual void SetOnGround (bool og) = 0;

  /// Get current rotation in angles around every axis.
  virtual csVector3 GetRotation () const = 0;

  /**
  * Set current rotation in angles around every axis and set to actor.
  * If a camera is used, set it to camera too.
  */
  virtual void SetRotation (const csVector3& rot) = 0;

  /// Move the actor.
  virtual void UpdateAction (float delta) = 0;

  /// Set the up axis of the actor.
  virtual void SetUpAxis (int axis) = 0;

  /// Set the walking velocity of the actor.
  virtual void SetVelocity (const csVector3& dir) = 0;

  /**
  * Set the walking velocity with which the character should move for
  * the given time period. After the time period, velocity is reset to zero.
  * Negative time intervals will result in no motion.
  */
  virtual void SetVelocityForTimeInterval (const csVector3& velo,
      float timeInterval) = 0;

  /**
  * This is used by UpdateAction() but you can also call it manually.
  * It will adjust the new position to match with collision
  * detection.
  */
  virtual void PreStep () = 0;

  /**
  * This is used by UpdateAction() but you can also call it manually.
  * Move the actor to proper target position.
  */
  virtual void PlayerStep (float delta) = 0;

  /// Set the falling speed.
  virtual void SetFallSpeed (float fallSpeed) = 0;

  /// Set the jumping speed.
  virtual void SetJumpSpeed (float jumpSpeed) = 0;

  /// Set the max jump height an actor can have.
  virtual void setMaxJumpHeight (float maxJumpHeight) = 0;

  /// Let the actor jump.
  virtual void Jump () = 0;

  /**
  * The max slope determines the maximum angle that the actor can walk up.
  * The slope angle is measured in radians.
  */
  virtual void SetMaxSlope (float slopeRadians) = 0;
  
  /// Get the max slope.
  virtual float GetMaxSlope () const = 0;
};

struct iCollisionSector : public virtual iBase
{
  SCF_INTERFACE (CS::Collision::iCollisionSector, 1, 0, 0);

  virtual void SetInternalScale (float scale) = 0;

  /// Set the global gravity.
  virtual void SetGravity (const csVector3& v) = 0;

  /// Get the global gravity.
  virtual csVector3 GetGravity () const = 0;

  /**
  * Add a collision object into the sector.
  * The collision object has to be initialized.
  */
  virtual void AddCollisionObject (iCollisionObject* object) = 0;

  /// Remove a collision object by pointer.
  virtual void RemoveCollisionObject (iCollisionObject* object) = 0;

  /// Add a portal into the sector. Collision objects crossing a portal will be switched from iCollisionSector's.
  virtual void AddPortal (iPortal* portal);

  /// Remove the given portal from this sector.
  virtual void RemovePortal (iPortal* portal);

  /**
  * Set the engine iSector related to this collision sector. The iMovable that are 
  * attached to a iCollisionObject present in this collision sector will be put automatically in the given engine sector.
  */
  virtual void SetSector (iSector* sector) = 0;

  /// Get the engine iSector related to this collison sector.
  virtual iSector* GetSector () = 0;

  /// Follow a beam from start to end and return the first body that is hit.
  virtual HitBeamResult HitBeam (
      const csVector3& start, const csVector3& end) = 0;

  /**
  * Follow a beam from start to end and return the first body that is hit.
  */
  //Lulu: What's this? return the first portal that is hit?
  virtual HitBeamResult HitBeamPortal (
      const csVector3& start, const csVector3& end) = 0;

  /**
  * Performs a discrete collision test against all objects in this iCollisionSector.
  * it reports one or more contact points for every overlapping object
  */
  virtual bool CollisionTest (iCollisionObject* object, csArray<CollisionData>& collisions) = 0;

  /**
  * Try to move the given object from \a fromWorld to \a toWorld and return the first collision occured if any.
  */
  // kickvb: a test only on convex colliders is probably not interesting, so remove this method if not possible to do on any collision object
  virtual MoveResult MoveTest (iCollisionObject* object,
      const csOrthoTransform& fromWorld, const csOrthoTransform& toWorld) = 0;

};

/**
* This is the Collision plug-in. This plugin is a factory for creating
* iCollider, iCollisionObject, iCollisionSector and iCollisionActor
* entities. 
*
* Main creators of instances implementing this interface:
* - OPCODE plugin (crystalspace.collisiondetection.opcode)
* - Bullet plugin (crystalspace.dynamics.bullet)
*
* Main ways to get pointers to this interface:
* - csQueryRegistry()
*/
struct iCollisionSystem : public virtual iBase
{
  SCF_INTERFACE (CS::Collision::iCollisionSystem, 2, 2, 2);

  /// Create a convex mesh collider.
  virtual csPtr<iColliderConvexMesh> CreateColliderConvexMesh (iMeshWrapper* mesh) = 0;

  /// Create a concave mesh collider.
  virtual csPtr<iColliderConcaveMesh> CreateColliderConcaveMesh (
    iMeshWrapper* mesh, bool isStatic = false) = 0;

  /// Create a scaled concave mesh collider.
  virtual csPtr<iColliderConcaveMeshScaled> CreateColliderConcaveMeshScaled (
    iColliderConcaveMesh* collider, float scale) = 0;

  /// Create a cylinder collider.
  virtual csPtr<iColliderCylinder> CreateColliderCylinder (float length, float radius) = 0;

  /// Create a box collider.
  virtual csPtr<iColliderBox> CreateColliderBox (const csVector3& size) = 0;

  /// Create a sphere collider.
  virtual csPtr<iColliderSphere> CreateColliderSphere (float radius) = 0;

  /// Create a capsule collider.
  virtual csPtr<iColliderCapsule> CreateColliderCapsule (float length, float radius) = 0;

  /// Create a plane collider.
  virtual csPtr<iColliderPlane> CreateColliderPlane (const csPlane3& plain) = 0;

  /// Create a terrain collider.
  virtual csPtr<iColliderTerrain> CreateColliderTerrain (const iTerrainSystem* terrain,
      float minHeight = 0, float maxHeight = 0) = 0;

  /**
  * Create a collision object. Without any initialization.
  * Need to call iCollisionObject::RebuildObject.
  */
  virtual csPtr<iCollisionObject> CreateCollisionObject (
    CollisionObjectType type = COLLISION_OBJECT_BASE) = 0;

  /**
  * Create a collision actor.
  * Need to call iCollisionObject::RebuildObject.
  */
  virtual csPtr<iCollisionActor> CreateCollisionActor () = 0;
  
  /// Create a collision sector.
  virtual csPtr<iCollisionSector> CreateCollisionSector () = 0;

  /// Create a collision group.
  virtual CollisionGroup& CreateCollisionGroup (const char* name) = 0;

  /// Find a collision group by name.
  virtual CollisionGroup& FindCollisionGroup (const char* name) = 0;

  virtual void SetGroupCollision (CollisionGroup& group1,
      CollisionGroup& group2, bool collide) = 0;

  virtual bool GetGroupCollision (CollisionGroup& group1,
      CollisionGroup& group2) = 0;

  /**
  * Decompose a concave mesh in convex parts. Each convex part will be added to
  * the collision object as a separate iColliderConvexMesh.
  */
  virtual void DecomposeConcaveMesh (iCollisionObject* object, iMeshWrapper* mesh) = 0;
};
}
}

#endif