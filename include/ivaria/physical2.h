#ifndef __IVARIA_PHYSIH__
#define __IVARIA_PHYSIH__

#include "csutil/scf.h"
#include "csutil/scf_interface.h"
#include "iutil/objreg.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "imesh/genmesh.h"
#include "csgeom/tri.h"
#include "cstool/primitives.h"
#include "ivaria/collision2.h"

namespace CS 
{
namespace Mesh 
{
struct iAnimatedMesh;
} 
}

namespace CS
{
namespace Collision2
{
struct iCollisionCallback;
struct iCollisionObject;
struct CollisionGroup;
struct iCollisionObject;
}
}

namespace CS
{
namespace Physics2
{

struct iJoint;
struct iObject;
struct iRigidBody;
struct iSoftBody;
struct iKinematicCallback;
struct iPhysicalSystem;

enum PhysicalBodyType
{
  BODY_RIGID = 0,
  BODY_SOFT
};

enum RigidBodyState
{
  STATE_STATIC = 0,
  STATE_DYNAMIC,
  STATE_KINEMATIC
};

/**
* A base interface of physical bodies. 
* iRigidBody and iSoftBody will be derived from this one.
*/
struct iPhysicalBody : public virtual CS::Collision2::iCollisionObject
{
  SCF_INTERFACE (CS::Physics2::iPhysicalBody, 1, 0, 0);

  virtual PhysicalBodyType GetBodyType () const = 0;

  virtual iRigidBody* QueryRigidBody () = 0;

  virtual iSoftBody* QuerySoftBody () = 0;

  /// Disable this collision object.
  virtual bool Disable () = 0;

  /// Enable this collision object.
  virtual bool Enable () = 0;

  /// Check if the collision object is enabled.
  virtual bool IsEnabled () = 0;

  // move "virtual iRigidBody::RigidBodyState GetState() = 0;" here and implement it for soft bodies by switching to rigid bodies when static/dynamic?
  //Lulu: If soft body can switch to rigid body, what about the parameters? Dose user have to call functions to set the parameters of rigid body? 
  //      If iPhysicalSystem create a new rigidbody, it's based on the softbody's current shape or original shape? 
  //      And the collision system will create a collision shape for the mesh? How to decide which type of collider is appropriate?

  /// Set the total mass of this body.
  virtual void SetMass (float mass) = 0;

  /// Get the mass of this body.
  virtual float GetMass () = 0;

  /// Set the mass of this body.
  /*virtual void SetMass (float mass) = 0;*/

  virtual float GetDensity () const = 0;

  virtual void SetDensity (float density) = 0;

  /// Return the volume of this body.
  virtual float GetVolume () = 0;
  
  /// Add a force to the whole body.
  virtual void AddForce (const csVector3& force) = 0;
  
  /// Get the linear velocity (movement).
  virtual csVector3 GetLinearVelocity (size_t index = 0) const = 0;

  /**
  * Set the friction of this body.
  * [0,1] for soft body.
  */
  virtual void SetFriction (float friction) = 0;

  /// Get the friction of this rigid body.
  virtual float GetFriction () = 0;
};

/**
* This is the interface for a rigid body.
* It keeps all properties for the body.
* It can also be attached to a movable or a bone,
* to automatically update it.
*
* Main creators of instances implementing this interface:
* - iPhysicalSystem::CreateRigidBody()
* 
* Main ways to get pointers to this interface:
* - iPhysicalSystem::GetRigidBody()
* 
* Main users of this interface:
* - iPhysicalSystem
*
* \sa iSoftBody
*/
struct iRigidBody : public virtual iPhysicalBody
{
  SCF_INTERFACE (CS::Physics2::iRigidBody, 1, 0, 0);

  /// Get the current state of the body.
  virtual RigidBodyState GetState () = 0;
  
  /// Set the current state of the body.
  virtual bool SetState (RigidBodyState state) = 0;


  /// Set the elasticity of this rigid body.
  virtual void SetElasticity (float elasticity) = 0;

  /// Get the elasticity of this rigid body.
  virtual float GetElasticity () = 0;

  /// Set the linear velocity (movement).
  virtual void SetLinearVelocity (const csVector3& vel) = 0;

  /// Set the angular velocity (rotation).
  virtual void SetAngularVelocity (const csVector3& vel) = 0;

  /// Get the angular velocity (rotation)
  virtual csVector3 GetAngularVelocity () const = 0;

  /// Add a torque (world space) (active for one timestep).
  virtual void AddTorque (const csVector3& torque) = 0;

  /// Add a force (local space) (active for one timestep).
  virtual void AddRelForce (const csVector3& force) = 0;

  /// Add a torque (local space) (active for one timestep).
  virtual void AddRelTorque (const csVector3& torque) = 0;

  /**
  * Add a force (world space) at a specific position (world space)
  * (active for one timestep)
  */
  virtual void AddForceAtPos (const csVector3& force,
      const csVector3& pos) = 0;

  /**
  * Add a force (world space) at a specific position (local space)
  * (active for one timestep)
  */
  virtual void AddForceAtRelPos (const csVector3& force,
      const csVector3& pos) = 0;

  /**
  * Add a force (local space) at a specific position (world space)
  * (active for one timestep)
  */
  virtual void AddRelForceAtPos (const csVector3& force,
      const csVector3& pos) = 0;

  /**
  * Add a force (local space) at a specific position (local space)
  * (active for one timestep)
  */
  virtual void AddRelForceAtRelPos (const csVector3& force,
      const csVector3& pos) = 0;

  /// Get total force (world space).
  virtual csVector3 GetForce () const = 0;

  /// Get total torque (world space).
  virtual csVector3 GetTorque () const = 0;

  /**
  * Set the callback to be used to update the transform of the kinematic body.
  * If no callback are provided then the dynamic system will use a default one.
  */
  virtual void SetKinematicCallback (iKinematicCallback* cb) = 0;

  /// Get the callback used to update the transform of the kinematic body.
  virtual iKinematicCallback* GetKinematicCallback () = 0;

  /**
  * Set the linear dampener for this rigid body. The dampening correspond to
  * how much the movements of the objects will be reduced. It is a value
  * between 0 and 1, giving the ratio of speed that will be reduced
  * in one second. 0 means that the movement will not be reduced, while
  * 1 means that the object will not move.
  * The default value is 0.
  * \sa iDynamicSystem::SetLinearDampener()
  */
  virtual void SetLinearDampener (float d) = 0;

  /// Get the linear dampener for this rigid body.
  virtual float GetLinearDampener () = 0;

  /**
  * Set the angular dampener for this rigid body. The dampening correspond to
  * how much the movements of the objects will be reduced. It is a value
  * between 0 and 1, giving the ratio of speed that will be reduced
  * in one second. 0 means that the movement will not be reduced, while
  * 1 means that the object will not move.
  * The default value is 0.
  * \sa iDynamicSystem::SetRollingDampener()
  */
  virtual void SetRollingDampener (float d) = 0;

  /// Get the angular dampener for this rigid body.
  virtual float GetRollingDampener () = 0;
};

/**
 * This class can be implemented in order to update the position of an anchor of a
 * CS::Physics2::Bullet::iSoftBody. This can be used to try to control manually the
 * position of a vertex of a soft body.
 *
 * \warning This feature uses a hack around the physical simulation of soft bodies
 * and may not always be stable. Use it at your own risk.
 * \sa CS::Physics2::iSoftBody::AnchorVertex(size_t,iAnchorAnimationControl)
 */
struct iAnchorAnimationControl : public virtual iBase
{
  SCF_INTERFACE(CS::Physics2::iAnchorAnimationControl, 1, 0, 0);

  /**
   * Return the new position of the anchor, in world coordinates.
   */
  virtual csVector3 GetAnchorPosition () const = 0;
};

/**
* A soft body is a physical body that can be deformed by the physical
* simulation. It can be used to simulate eg ropes, clothes or any soft
* volumetric object.
*
* A soft body does not have a positional transform by itself, but the
* position of every vertex of the body can be queried through GetVertexPosition().
*
* A soft body can neither be static or kinematic, it is always dynamic.
* \sa iRigidBody 
*/
struct iSoftBody : public virtual iPhysicalBody
{
  SCF_INTERFACE (CS::Physics2::iSoftBody, 1, 0, 0);

  /// Set the mass of a node by index.
  virtual void SetVertexMass (float mass, size_t index) = 0;

  /// Get the mass of a node by index.
  virtual float GetVertexMass (size_t index) = 0;

  /// Return the count of vertices of this soft body.
  virtual size_t GetVertexCount () = 0;

  /// Return the position in world coordinates of the given vertex.
  virtual csVector3 GetVertexPosition (size_t index) const = 0;

  /// Anchor the given vertex to its current position. This vertex will no more move.
  virtual void AnchorVertex (size_t vertexIndex) = 0;

  /**
  * Anchor the given vertex to the given rigid body. The relative position of the
  * vertex and the body will remain constant.
  */
  virtual void AnchorVertex (size_t vertexIndex,
      iRigidBody* body) = 0;

  /**
  * Anchor the given vertex to the given controller. The relative position of the
  * vertex and the controller will remain constant.
  */
  virtual void AnchorVertex (size_t vertexIndex,
      iAnchorAnimationControl* controller) = 0;

  /**
  * Update the position of the anchor of the given vertex relatively to the anchored
  * rigid body. This can be used to have a finer control of the anchor position
  * relatively to the rigid body.
  *
  * This would work only if you called AnchorVertex(size_t,iRigidBody*) before.
  * The position to be provided is in world coordinates.
  *
  * \warning The stability of the simulation can be lost if you move the position too far
  * from the previous position.
  * \sa CS::Animation::iSoftBodyAnimationControl::CreateAnimatedMeshAnchor()
  */
  virtual void UpdateAnchor (size_t vertexIndex,
      csVector3& position) = 0;

  /**
  * Remove the given anchor. This won't work if you anchored the vertex to a rigid body, due
  * to a limitation in the Bullet library.
  */
  virtual void RemoveAnchor (size_t vertexIndex) = 0;

  /**
  * Set the rigidity of this body. The value should be in the 0 to 1 range, with
  * 0 meaning soft and 1 meaning rigid.
  */
  virtual void SetRigidity (float rigidity) = 0;

  /// Get the rigidity of this body.
  virtual float GetRidigity () = 0;

  /// Set the linear velocity (movement).
  virtual void SetLinearVelocity (const csVector3& vel) = 0;

  /// Set the linear velocity of the given vertex of the body.
  virtual void SetLinearVelocity (const csVector3& velocity,
      size_t vertexIndex) = 0;

  /**
  * Set the wind velocity of the whole body.
  */
  virtual void SetWindVelocity (const csVector3& velocity) = 0;

  /// Get the wind velocity of the whole body.
virtual const csVector3 GetWindVelocity () const = 0;

  /// Add a force at the given vertex of the body.
  virtual void AddForce (const csVector3& force, size_t vertexIndex) = 0;

  /// Return the count of triangles of this soft body.
  virtual size_t GetTriangleCount () = 0;

  /// Return the triangle with the given index.
  virtual csTriangle GetTriangle (size_t index) const = 0;

  /// Return the normal vector in world coordinates for the given vertex.
  virtual csVector3 GetVertexNormal (size_t index) const = 0;

  /**
  * Currently Blender set this to 0 when creating a soft body.
  * Used to create a btTriangleMesh for soft body.
  */
  //virtual void SetWelding(float welding) = 0;
};

/**
 * General helper class for CS::Physics2::Bullet::iSoftBody.
 */
struct SoftBodyHelper
{
  /**
   * Create a genmesh from the given cloth soft body.
   * The genmesh will be double-sided, in order to have correct normals on both
   * sides of the cloth (ie the vertices of the soft body will be duplicated for the
   * genmesh).
   * \warning Don't forget to use doubleSided = true in
   * CS::Animation::iSoftBodyAnimationControl::SetSoftBody()
   */
  static csPtr<iMeshFactoryWrapper> CreateClothGenMeshFactory
  (iObjectRegistry* object_reg, const char* factoryName, iSoftBody* cloth)
  {
    csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);

    // Create the cloth mesh factory.
    csRef<iMeshFactoryWrapper> clothFact = engine->CreateMeshFactory
      ("crystalspace.mesh.object.genmesh", factoryName);
    if (!clothFact)
      return 0;

    csRef<iGeneralFactoryState> gmstate = scfQueryInterface<iGeneralFactoryState>
      (clothFact->GetMeshObjectFactory ());

    // Create the vertices of the genmesh
    size_t vertexCount = cloth->GetVertexCount ();
    gmstate->SetVertexCount (vertexCount * 2);
    csVector3* vertices = gmstate->GetVertices ();
    for (size_t i = 0; i < vertexCount; i++)
    {
      vertices[i] = cloth->GetVertexPosition (i);
      vertices[i + vertexCount] = cloth->GetVertexPosition (i);
    }

    // Create the triangles of the genmesh
    gmstate->SetTriangleCount (cloth->GetTriangleCount () * 2);
    csTriangle* triangles = gmstate->GetTriangles ();
    for (size_t i = 0; i < cloth->GetTriangleCount (); i++)
    {
      csTriangle triangle = cloth->GetTriangle (i);
      triangles[i * 2] = triangle;
      triangles[i * 2 + 1] = csTriangle (triangle[2] + vertexCount,
					 triangle[1] + vertexCount,
					 triangle[0] + vertexCount);
    }

    gmstate->CalculateNormals ();

    // Set up the texels of the genmesh
    csVector2* texels = gmstate->GetTexels ();
    csVector3* normals = gmstate->GetNormals ();
    CS::Geometry::TextureMapper* mapper = new CS::Geometry::DensityTextureMapper (1.0f);
    for (size_t i = 0; i < vertexCount * 2; i++)
      texels[i] = mapper->Map (vertices[i], normals[i], i);

    gmstate->Invalidate ();

    return csPtr<iMeshFactoryWrapper> (clothFact);
  }
};

/**
* A joint that can constrain the relative motion between two iRigidBody.
* For instance if all motion in along the local X axis is constrained
* then the bodies will stay motionless relative to each other
* along an x axis rotated and positioned by the joint's transform.
*
* Main creators of instances implementing this interface:
* - iPhysicalSystem::CreateJoint()
* 
* Main users of this interface:
* - iPhysicalSystem
*/
struct iJoint : public virtual iBase
{
  SCF_INTERFACE (CS::Physics2::iJoint, 1, 0, 0);

  /**
  * Set the rigid bodies that will be affected by this joint. Set force_update to true if 
  * you want to apply the changes right away.
  */
  virtual void Attach (iPhysicalBody* body1, iPhysicalBody* body2,
      bool forceUpdate = true) = 0;

  /// Get the attached body with the given index (valid values for body are 0 and 1).
  virtual iPhysicalBody* GetAttachedBody (int index) = 0;

  /**
  * Set the world transformation of the joint.
  *
  * Set force_update to true if you want to apply the changes right away.
  */
  virtual void SetTransform (const csOrthoTransform& trans,
      bool forceUpdate = false) = 0;

  /// Get the world transformation of the joint.
  virtual csOrthoTransform GetTransform () const = 0;

  /// Set the new position of the joint, in world coordinates.
  virtual void SetPosition (const csVector3& position,
    bool forceUpdate = false) = 0;

  /// Get the current position of the joint, in world coordinates.
  virtual csVector3 GetPosition () const = 0;

  /**
  * Set the translation constraints on the 3 axes. If true is
  * passed for an axis then the Joint will constrain all motion along
  * that axis (ie no motion will be allowed). If false is passed in then all motion along that
  * axis is free, but bounded by the minimum and maximum distance
  * if set. Set force_update to true if you want to apply the changes 
  * right away.
  */
  virtual void SetTransConstraints (bool X, 
      bool Y, bool Z, 
      bool forceUpdate = false) = 0;

  /// True if this axis' translation is constrained.
  virtual bool IsXTransConstrained () = 0;

  /// True if this axis' translation is constrained.
  virtual bool IsYTransConstrained () = 0;

  /// True if this axis' translation is constrained.
  virtual bool IsZTransConstrained () = 0;

  /**
  * Set the minimum allowed distance between the two bodies. Set force_update to true if 
  * you want to apply the changes right away.
  */
  virtual void SetMinimumDistance (const csVector3& dist,
      bool forceUpdate = false) = 0;

  /// Get the minimum allowed distance between the two bodies.
  virtual csVector3 GetMinimumDistance () const = 0;

  /**
  * Set the maximum allowed distance between the two bodies. Set force_update to true if 
  * you want to apply the changes right away.
  */
  virtual void SetMaximumDistance (const csVector3& dist,
      bool forceUpdate = false) = 0;

  /// Get the maximum allowed distance between the two bodies.
  virtual csVector3 GetMaximumDistance () const = 0;

  /**
  * Set the rotational constraints on the 3 axes. If true is
  * passed for an axis then the Joint will constrain all rotation around
  * that axis (ie no motion will be allowed). If false is passed in then all rotation around that
  * axis is free, but bounded by the minimum and maximum angle
  * if set. Set force_update to true if you want to apply the changes 
  * right away.
  */
  virtual void SetRotConstraints (bool X, 
      bool Y, bool Z, 
      bool forceUpdate = false) = 0;

  /// True if this axis' rotation is constrained.
  virtual bool IsXRotConstrained () = 0;

  /// True if this axis' rotation is constrained.
  virtual bool IsYRotConstrained () = 0;

  /// True if this axis' rotation is constrained.
  virtual bool IsZRotConstrained () = 0;

  /**
  * Set the minimum allowed angle between the two bodies, in radian. Set force_update to true if 
  * you want to apply the changes right away.
  */
  virtual void SetMinimumAngle (const csVector3& angle,
      bool forceUpdate = false) = 0;

  /// Get the minimum allowed angle between the two bodies (in radian).
  virtual csVector3 GetMinimumAngle () const = 0;

  /**
  * Set the maximum allowed angle between the two bodies (in radian). Set force_update to true if 
  * you want to apply the changes right away.
  */
  virtual void SetMaximumAngle (const csVector3& angle,
      bool forceUpdate = false) = 0;

  /// Get the maximum allowed angle between the two bodies (in radian).
  virtual csVector3 GetMaximumAngle () const = 0;

  /** 
  * Set the restitution of the joint's stop point (this is the 
  * elasticity of the joint when say throwing open a door how 
  * much it will bounce the door back closed when it hits).
  */
  virtual void SetBounce (const csVector3& bounce,
      bool forceUpdate = false) = 0;

  /// Get the joint restitution.
  virtual csVector3 GetBounce () const = 0;

  /**
  * Apply a motor velocity to joint (for instance on wheels). Set force_update to true if 
  * you want to apply the changes right away.
  */
  virtual void SetDesiredVelocity (const csVector3& velo,
      bool forceUpdate = false) = 0;

  /// Get the desired velocity of the joint motor.
  virtual csVector3 GetDesiredVelocity () const = 0;

  /**
  * Set the maximum force that can be applied by the joint motor to reach the desired velocity.
  * Set force_update to true if  you want to apply the changes right away.
  */
  virtual void SetMaxForce (const csVector3& force,
      bool forceUpdate = false) = 0;

  /// Get the maximum force that can be applied by the joint motor to reach the desired velocity.
  virtual csVector3 GetMaxForce () const = 0;

  /**
  * Rebuild the joint using the current setup. Return true if the rebuilding operation was successful
  * (otherwise the joint won't be active).
  */
  virtual bool RebuildJoint () = 0;

  /// Set this joint to a spring joint.
  virtual void SetSpring(bool isSpring, bool forceUpdate = false) = 0;

  /// Set the stiffness of the spring.
  virtual void SetLinearStiffness (csVector3 stiff, bool forceUpdate = false) = 0;

  virtual csVector3 GetLinearStiffness () const = 0;

  virtual void SetAngularStiffness (csVector3 stiff, bool forceUpdate = false) = 0;

  virtual csVector3 GetAngularStiffness () const = 0;

  /// Set the damping of the spring.
  virtual void SetLinearDamping (csVector3 damp, bool forceUpdate = false) = 0;

  virtual csVector3 GetLinearDamping () const = 0;

  /// Set the damping of the spring.
  virtual void SetAngularDamping (csVector3 damp, bool forceUpdate = false) = 0;

  virtual csVector3 GetAngularDamping () const = 0;
  
  /// Set the value to an equilibrium point for translation.
  virtual void SetLinearEquilibriumPoint (csVector3 point, bool forceUpdate = false) = 0;

  /// Set the value to an equilibrium point for rotation.
  virtual void SetAngularEquilibriumPoint (csVector3 point, bool forceUpdate = false) = 0;

  virtual void SetBreakingImpulseThreshold (float threshold, bool forceUpdate = false) = 0;

  virtual float GetBreakingImpulseThreshold () = 0;
};

/**
 * A callback to be implemented when you are using kinematic bodies. If no
 * callback are provided then the dynamic system will use a default one which
 * will update the transform of the body from the position of the attached
 * movable (see iRigidBody::AttachMovable()).
 * \sa CS::Physics2::iRigidBody::SetKinematicCallback()
 */
struct iKinematicCallback : public virtual iBase
{
  SCF_INTERFACE (CS::Physics2::iKinematicCallback, 1, 0, 0);

  /**
   * Update the new transform of the rigid body.
   */
  virtual void GetBodyTransform (iRigidBody* body,
				 csOrthoTransform& transform) const = 0;
};

struct iPhysicalSystem : public virtual iBase
{
  SCF_INTERFACE (CS::Physics2::iPhysicalSystem, 1, 0, 0);

  /**
  * Create a rigid body, if there's an iCollisionObject pointer, 
  * Need to call iCollisionObject::RebuildObject.
  */
  virtual csRef<iRigidBody> CreateRigidBody () = 0;

  /// Create a general 6DOF joint.
  virtual csRef<iJoint> CreateJoint () = 0;

  /// Create a P2P joint for rigid bodies.
  virtual csRef<iJoint> CreateRigidP2PJoint (const csVector3 position) = 0;

  /// Create a slide joint for rigid bodies.
  virtual csRef<iJoint> CreateRigidSlideJoint (const csOrthoTransform trans,
    float minDist, float maxDist, float minAngle, float maxAngle, int axis) = 0;

  /// Create a hinge joint for rigid bodies.
  virtual csRef<iJoint> CreateRigidHingeJoint (const csVector3 position,
    float minAngle, float maxAngle, int axis) = 0;

  /// Create a linear joint for soft body.
  virtual csRef<iJoint> CreateSoftLinearJoint (const csVector3 position) = 0;

  /// Create a angular joint for soft body.
  virtual csRef<iJoint> CreateSoftAngularJoint (int axis) = 0;

  virtual csRef<iJoint> CreateRigidPivotJoint (iRigidBody* body, const csVector3 position) = 0;
  
  /**
  * Create a soft body rope.
  * \param start Start position of the rope.
  * \param end End position of the rope.
  * \param segmentCount Number of segments in the rope.
  * \remark You must call SetSoftBodyWorld() prior to this.
  */
  virtual csRef<iSoftBody> CreateRope (csVector3 start,
      csVector3 end, size_t segmentCount) = 0;

  /**
  * Create a soft body rope with explicit positions of the vertices.
  * \param vertices The array of positions to use for the vertices.
  * \param vertexCount The amount of vertices for the rope.
  * \remark You must call SetSoftBodyWorld() prior to this.
  */
  virtual csRef<iSoftBody> CreateRope (csVector3* vertices, size_t vertexCount) = 0;

  /**
  * Create a soft body cloth.
  * \param corner1 The position of the top left corner.
  * \param corner2 The position of the top right corner.
  * \param corner3 The position of the bottom left corner.
  * \param corner4 The position of the bottom right corner.
  * \param segmentCount1 Number of horizontal segments in the cloth.
  * \param segmentCount2 Number of vertical segments in the cloth.
  * \param withDiagonals Whether there must be diagonal segments in the cloth
  * or not. Diagonal segments will make the cloth more rigid.
  * \remark You must call SetSoftBodyWorld() prior to this.
  */
  virtual csRef<iSoftBody> CreateCloth (csVector3 corner1, csVector3 corner2,
      csVector3 corner3, csVector3 corner4,
      size_t segmentCount1, size_t segmentCount2,
      bool withDiagonals = false) = 0;

  /**
  * Create a volumetric soft body from a genmesh.
  * \param genmeshFactory The genmesh factory to use.
  * \param if there's an iCollisionObject pointer, attach the iCollisionObject to it.
  * \remark You must call SetSoftBodyWorld() prior to this.
  */
  virtual csRef<iSoftBody> CreateSoftBody (iGeneralFactoryState* genmeshFactory, 
    const csOrthoTransform& bodyTransform) = 0;

  /**
  * Create a custom volumetric soft body.
  * \param vertices The vertices of the soft body. The position is absolute.
  * \param vertexCount The count of vertices of the soft body.
  * \param triangles The faces of the soft body.
  * \param triangleCount The count of faces of the soft body.
  \param if there's an iCollisionObject pointer, attach the iCollisionObject to it.
  * \remark You must call SetSoftBodyWorld() prior to this.
  */
  virtual csRef<iSoftBody> CreateSoftBody (csVector3* vertices,
      size_t vertexCount, csTriangle* triangles,
      size_t triangleCount, const csOrthoTransform& bodyTransform) = 0;
};

struct iPhysicalSector : public virtual iBase
{
  SCF_INTERFACE (CS::Physics2::iPhysicalSector, 1, 0, 0);

  /**
  * Set the simulation speed. A value of 0 means that the simulation is not made
  * automatically (but it can still be made manually through Step())
  */
  virtual void SetSimulationSpeed (float speed) = 0;

  virtual void SetStepParameters (float timeStep, size_t maxSteps,
    size_t iterations) = 0;  

  /// Step the simulation forward by the given duration, in second
  virtual void Step (float duration) = 0;

  /**
  * Set the global linear dampener. The dampening correspond to how
  * much the movements of the objects will be reduced. It is a value
  * between 0 and 1, giving the ratio of speed that will be reduced
  * in one second. 0 means that the movement will not be reduced, while
  * 1 means that the object will not move.
  * The default value is 0.
  * \sa CS::Physics2::Bullet::iRigidBody::SetLinearDampener()
  */
  virtual void SetLinearDampener (float d) = 0;

  /**
  * Get the global linear dampener setting.
  */
  virtual float GetLinearDampener () const = 0;

  /**
  * Set the global angular dampener. The dampening correspond to how
  * much the movements of the objects will be reduced. It is a value
  * between 0 and 1, giving the ratio of speed that will be reduced
  * in one second. 0 means that the movement will not be reduced, while
  * 1 means that the object will not move.
  * The default value is 0.
  * \sa CS::Physics2::Bullet::iRigidBody::SetRollingDampener()
  */
  virtual void SetRollingDampener (float d) = 0;

  /// Get the global rolling dampener setting.
  virtual float GetRollingDampener () const = 0;

  /**
  * Turn on/off AutoDisable functionality.
  * AutoDisable will stop moving objects if they are stable in order
  * to save processing time. By default this is enabled.
  */
  // always enabled?
  //virtual void EnableAutoDisable(bool enable) = 0;
 
  /**
  * Return whether the AutoDisable is on or off.
  */
  //virtual bool AutoDisableEnabled() = 0;
  
  /**
  * Set the parameters for AutoDisable.
  * \param linear Maximum linear movement to disable a body. Default value is 0.8.
  * \param angular Maximum angular movement to disable a body. Default value is 1.0.
  * \param steps Minimum number of steps the body meets linear and angular
  * requirements before it is disabled. Default value is 0.
  * \param time Minimum time the body needs to meet linear and angular
  * movement requirements before it is disabled. Default value is 0.0.
  * \remark With the Bullet plugin, the 'steps' parameter is ignored.
  * \remark With the Bullet plugin, calling this method will not affect bodies already
  * created.
  */
  virtual void SetAutoDisableParams (float linear, float angular,
      float time) = 0;

  /**
  * Add a rigid body into the sector.
  * The rigid body has to be initialized.
  */
  virtual void AddRigidBody (iRigidBody* body) = 0;

  /// Remove a rigid body by pointer.
  virtual void RemoveRigidBody (iRigidBody* body) = 0;

  /// Get the count of rigid bodies.
  virtual size_t GetRigidBodyCount () = 0;

  /// Get the rigid body by index.
  virtual iRigidBody* GetRigidBody (size_t index) = 0;

  /**
  * Add a soft body into the sector.
  * The soft body has to be initialized.
  */
  virtual void AddSoftBody (iSoftBody* body) = 0;

  /// Remove a soft body by pointer.
  virtual void RemoveSoftBody (iSoftBody* body) = 0;

  /// Get the count of soft bodies.
  virtual size_t GetSoftBodyCount () = 0;

  /// Get the soft body by index.
  virtual iSoftBody* GetSoftBody (size_t index) = 0;

  /// Remove a joint by pointer.
  virtual void RemoveJoint (iJoint* joint) = 0;

  /**
  * Set whether this dynamic world can handle soft bodies or not.
  * \warning You have to call this method before adding any objects in the
  * dynamic world.
  */
  virtual void SetSoftBodyEnabled (bool enabled) = 0; 

  /**
  * Return whether this dynamic world can handle soft bodies or not.
  */
  virtual bool GetSoftBodyEnabled () = 0;
};

/**
 * Animation control type for a genmesh animated by a CS::Physics2::iSoftBody.
 *
 * Main ways to get pointers to this interface:
 * - csQueryPluginClass()
 * - csLoadPlugin()
 *
 * Main users of this interface:
 * - Genmesh plugin (crystalspace.mesh.object.genmesh) 
 */
struct iSoftBodyAnimationControlType : public iGenMeshAnimationControlType
{
  SCF_INTERFACE (CS::Physics2::iSoftBodyAnimationControlType, 1, 0, 0);
};

/**
 * Animation control factory for a genmesh animated by a CS::Physics2::iSoftBody.
 *
 * Main creators of instances implementing this interface:
 * - CS::Physics2::iSoftBodyAnimationControlType::CreateAnimationControlFactory()
 *
 * Main ways to get pointers to this interface:
 * - iGeneralFactoryState::GetAnimationControlFactory()
 *
 * Main users of this interface:
 * - Genmesh plugin (crystalspace.mesh.object.genmesh) 
 */
struct iSoftBodyAnimationControlFactory : public iGenMeshAnimationControlFactory
{
  SCF_INTERFACE (CS::Physics2::iSoftBodyAnimationControlFactory, 1, 0, 0);
};

/**
 * Animation control for a genmesh animated by a CS::Physics2::iSoftBody. This class will
 * animate the vertices of the genmesh depending on the physical simulation of the
 * soft body. It will also update automatically the position of the genmesh.
 *
 * The soft body controlling the animation of the genmesh can also be attached precisely to a
 * given vertex of an animesh. This allows to have the soft body following precisely the vertices
 * of the animesh, even when it is deformed by the skinning and morphing processes.
 *
 * Main creators of instances implementing this interface:
 * - CS::Physics2::iSoftBodyAnimationControlFactory::CreateAnimationControl()
 *
 * Main ways to get pointers to this interface:
 * - iGeneralMeshState::GetAnimationControl()
 *
 * Main users of this interface:
 * - Genmesh plugin (crystalspace.mesh.object.genmesh) 
 */
struct iSoftBodyAnimationControl : public iGenMeshAnimationControl
{
  SCF_INTERFACE (CS::Physics2::iSoftBodyAnimationControl, 1, 0, 0);

  /**
   * Set the soft body to be used to animate the genmesh. You can switch this soft body
   * at any time, the animation of the genmesh will just be adapted to the new soft body.
   * \param body The soft body that will be used to animate this genmesh.
   * \param doubleSided True if the genmesh is double-sided (ie this is a cloth
   * soft body), false otherwise. If the genmesh is double-sided, then the duplicated
   * vertices must be added at the end of the vertex array, so that a vertex of index
   * 'i' is duplicated at index 'i + body->GetVertexCount ()'.
   */
  virtual void SetSoftBody (iSoftBody* body, bool doubleSided = false) = 0;

  /**
   * Get the soft body used to animate the genmesh.
   */
  virtual iSoftBody* GetSoftBody () = 0;

  /**
   * Create an anchor between the soft body and an animesh. The position of the anchor
   * will be updated accordingly when the vertex is moved by the skinning and morphing
   * processes of the animesh.
   *
   * This anchor is only effective if the vertex of the animesh is influenced by more
   * than one bone or by some morph targets. If it is not the case then it is more
   * efficient to simply use CS::Physics2::iSoftBody::AnchorVertex(size_t,iRigidBody*).
   *
   * You have to provide a rigid body attached to the animesh as a main physical anchor
   * point. The main way to do that is to use a CS::Animation::iSkeletonRagdollNode
   * animation node.
   *
   * Note also that you may anchor a same soft body to different animeshes, for example
   * to create a cloth hold by several avatars.
   *
   * \param animesh The CS::Mesh::iAnimatedMesh to attach the soft body to.
   * \param body The rigid body used as the main physical anchor point.
   * \param bodyVertexIndex The index of the vertex on the soft body which will be anchored.
   * \param animeshVertexIndex The index of the vertex on the animesh which will be anchored.
   * If no values are provided then the system will compute the vertex on the animesh which is
   * the closest to the given vertex of the soft body. This vertex can be queried afterwards
   * through GetAnimatedMeshAnchorVertex().
   */
  virtual void CreateAnimatedMeshAnchor (CS::Mesh::iAnimatedMesh* animesh,
    iRigidBody* body,
		size_t bodyVertexIndex,
		size_t animeshVertexIndex = (size_t) ~0) = 0;

  /**
   * Get the vertex of the animesh which is anchored to the given vertex of the soft body.
   */
  virtual size_t GetAnimatedMeshAnchorVertex (size_t bodyVertexIndex) = 0;

  /**
   * Remove the given anchor.
   * \warning This won't actually work, due to a limitation inside the Bullet library...
   */
  virtual void RemoveAnimatedMeshAnchor (size_t bodyVertexIndex) = 0;
};
}
}
#endif