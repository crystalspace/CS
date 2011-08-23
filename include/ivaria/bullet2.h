/*
    Copyright (C) 2011 by Liu Lu

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

#ifndef __IVARIA_PHYSICS_BULLET_H__
#define __IVARIA_PHYSICS_BULLET_H__

/**\file
 * Bullet physics interfaces
 */

#include "csutil/scf_interface.h"
#include "iutil/objreg.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "imesh/genmesh.h"
#include "csgeom/tri.h"
#include "cstool/primitives.h"

namespace CS
{
namespace Physics2
{
namespace Bullet2
{

/**
 * The type of debug mode.
 */
enum DebugMode
{
  DEBUG_NOTHING = 0,     /*!< Nothing will be displayed. */
  DEBUG_COLLIDERS = 1,   /*!< Display the colliders of the bodies. */
  DEBUG_AABB = 2,        /*!< Display the axis aligned bounding boxes of the bodies. */
  DEBUG_JOINTS = 4,      /*!< Display the joint positions and limits. */
};

/**
 * The Bullet implementation of iSoftBody also implements this
 * interface.
 * \sa CS::Physics2::iRigidBody CS::Physics2::iSoftBody
 */
struct iSoftBody : public virtual iBase
{
  SCF_INTERFACE (CS::Physics2::Bullet2::iSoftBody, 1, 0, 0);

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

  /// Get true if use cluster vs convex handling for rigid vs soft collision detection.
  virtual bool GetClusterCollisionRS () = 0;

  /// Set true if use cluster vs cluster handling for soft vs soft collision detection.
  virtual void SetClusterCollisionSS (bool cluster) = 0;

  /// Get true if use cluster vs cluster handling for soft vs soft collision detection.
  virtual bool GetClusterCollisionSS () = 0;

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

  /// Set true if use bending constraint.
  virtual void SetBendingConstraint (bool bending) = 0;

  /// Generate cluster for the soft body.
  virtual void GenerateCluster (int iter) = 0;
};

/**
 * The Bullet implementation of iPhysicalSector also implements this
 * interface.
 * \sa CS::Physics2::iPhysicalSector
 */
struct iPhysicalSector : public virtual iBase
{
  SCF_INTERFACE (CS::Physics2::Bullet2::iPhysicalSector, 1, 0, 0);

  /**
   * Save the current state of the dynamic world in a file.
   * \return True if the operation succeeds, false otherwise.
   */
  virtual bool SaveWorld (const char* filename) = 0;

  /**
   * Draw the debug informations of the dynamic system. This has to be called
   * at each frame, and will add 2D lines on top of the rendered scene. The
   * objects to be displayed are defined by SetDebugMode().
   */
  virtual void DebugDraw (iView* rview) = 0;

  /**
   * Set the mode to be used when displaying debug informations. The default value
   * is 'CS::Physics2::Bullet2::DEBUG_COLLIDERS | CS::Physics2::Bullet2::DEBUG_JOINTS'.
   * \remark Don't forget to call DebugDraw() at each frame to effectively display
   * the debug informations.
   */
  virtual void SetDebugMode (DebugMode mode) = 0;

  /// Get the current mode used when displaying debug informations.
  virtual DebugMode GetDebugMode () = 0;  

  /**
   * Start the profiling of the simulation. This would add an overhead to the
   * computations, but allows to display meaningful information on the behavior
   * of the simulation.
   */ 
  virtual void StartProfile () = 0;

  /**
   * Stop the profiling of the simulation. This would add an overhead to the
   */
  virtual void StopProfile () = 0;

  /**
   * Dump the profile information on the standard output. StartProfile() must
   * have been called before.
   * \param resetProfile Whether or not the profile data must be reset after
   * the dumping.
   */
  virtual void DumpProfile (bool resetProfile = true) = 0;
};
}
}
}
#endif