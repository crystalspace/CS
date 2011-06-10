#ifndef __IVARIA_PHYSICS_BULLET_H__
#define __IVARIA_PHYSICS_BULLET_H__

namespace CS
{
namespace Physics
{
namespace Bullet
{

enum DebugMode
{
  DEBUG_NOTHING = 0,     /*!< Nothing will be displayed. */
  DEBUG_COLLIDERS = 1,   /*!< Display the colliders of the bodies. */
  DEBUG_AABB = 2,        /*!< Display the axis aligned bounding boxes of the bodies. */
  DEBUG_JOINTS = 4       /*!< Display the joint positions and limits. */
};

struct iSoftBody : public virtual iBase
{
  SCF_INTERFACE (CS::Physics::Bullet::iSoftBody, 1, 0, 0);

  /**
  * Draw the debug informations of this soft body. This has to be called
  * at each frame, and will add 2D lines on top of the rendered scene.
  */
  virtual void DebugDraw (iView* rView) = 0;

  /// Set linear stiffness coefficient [0,1].
  virtual void SetLinearStiff (float stiff) = 0;

  /// Set area/angular stiffness coefficient [0,1].
  virtual void SetAngularStiff (float stiff) = 0;

  /// Set volume stiffness coefficient [0,1].
  virtual void SetVolumeStiff (float stiff) = 0;

  /// Reset the collision flag to 0.
  virtual void ResetCollisionFlag () = 0;

  /// Set true if use cluster vs convex handling for rigid vs soft collision detection.
  virtual void SetClusterCollisionRS (bool cluster) = 0;

  /// Set true if use cluster vs cluster handling for soft vs soft collision detection.
  virtual void SetClusterCollisionSS (bool cluster) = 0;

  /// Set soft vs rigid hardness [0,1] (cluster only).
  virtual void SetSRHardness (float hardness) = 0;

  /// Set soft vs kinetic hardness [0,1] (cluster only).
  virtual void SetSKHardness (float hardness) = 0;

  /// Set soft vs soft hardness [0,1] (cluster only).
  virtual void SetSSHardness (float hardness) = 0;

  /// Set soft vs rigid impulse split [0,1] (cluster only).
  virtual void SetSRImpulse (float impulse) = 0;

  /// Set soft vs rigid impulse split [0,1] (cluster only).
  virtual void SetSKImpulse (float impulse) = 0;

  /// Set soft vs rigid impulse split [0,1] (cluster only).
  virtual void SetSSImpulse (float impulse) = 0;

  /// Set velocities correction factor (Baumgarte).
  virtual void SetVeloCorrectionFactor (float factor) = 0;

  /// Set damping coefficient [0,1].
  virtual void SetDamping (float damping) = 0;

  /// Set drag coefficient [0,+inf].
  virtual void SetDrag (float drag) = 0;

  /// Set lift coefficient [0,+inf].
  virtual void SetLift (float lift) = 0;

  /// Set pressure coefficient [-inf,+inf].
  virtual void SetPressure (float pressure) = 0;

  /// Set volume conversation coefficient [0,+inf].
  virtual void SetVolumeConversationCoefficient (float conversation) = 0;

  /// Set pose matching coefficient [0,1].	
  virtual void SetShapeMatchThreshold (float matching) = 0;

  /// Set rigid contacts hardness [0,1].
  virtual void SetRContactsHardness (float hardness) = 0;

  /// Set kinetic contacts hardness [0,1].
  virtual void SetKContactsHardness (float hardness) = 0;

  /// Set soft contacts hardness [0,1].
  virtual void SetSContactsHardness (float hardness) = 0;

  /// Set anchors hardness [0,1].
  virtual void SetAnchorsHardness (float hardness) = 0;

  /// Set velocities solver iterations.
  virtual void SetVeloSolverIterations (int iter) = 0;

  /// Set positions solver iterations.
  virtual void SetPositionIterations (int iter) = 0;

  /// Set drift solver iterations.
  virtual void SetDriftIterations (int iter) = 0;

  /// Set cluster solver iterations.
  virtual void SetClusterIterations (int iter) = 0;

  /// Set true if use pose matching.
  virtual void SetShapeMatching (bool match) = 0;

  virtual void SetBendingConstraint (bool bending) = 0;

  virtual void GenerateCluster (int iter) = 0;

  /**
  * Configure the soft body with parameters set above.
  * If bending constraint is used set it with true.
  */
  //Lulu: use RebuildObject () instead?
  //virtual void ConfigureSoftBody(bool bending) = 0;
};

struct iPhysicalSector : public virtual iBase
{
  SCF_INTERFACE (CS::Physics::Bullet::iPhysicalSector, 1, 0, 0);

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

  virtual void SetGimpactEnabled (bool enabled) = 0; 

  virtual bool GetGimpactEnabled () = 0; 

  /**
  * Save the current state of the dynamic world in a file.
  * \return True if the operation succeeds, false otherwise.
  */
  virtual void SaveWorld (const char* filename) = 0;

  /**
  * Draw the debug informations of the dynamic system. This has to be called
  * at each frame, and will add 2D lines on top of the rendered scene. The
  * objects to be displayed are defined by SetDebugMode().
  */
  virtual void DebugDraw (iView* rview) = 0;

  /**
  * Set the mode to be used when displaying debug informations. The default value
  * is 'CS::Physics::Bullet::DEBUG_COLLIDERS | CS::Physics::Bullet::DEBUG_JOINTS'.
  * \remark Don't forget to call DebugDraw() at each frame to effectively display
  * the debug informations.
  */
  virtual void SetDebugMode (DebugMode mode) = 0;

  /// Get the current mode used when displaying debug informations.
  virtual DebugMode GetDebugMode () = 0;  

  virtual void StartProfile () = 0;

  virtual void StopProfile () = 0;

  virtual void DumpProfile (bool resetProfile = true) = 0;
};
}
}
}
#endif