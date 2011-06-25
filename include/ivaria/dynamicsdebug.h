/*
  Copyright (C) 2010 Christian Van Brussel, Institute of Information
      and Communication Technologies, Electronics and Applied Mathematics
      at Universite catholique de Louvain, Belgium
      http://www.uclouvain.be/en-icteam.html

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
#ifndef __CS_IVARIA_DYNAMICSDEBUG_H__
#define __CS_IVARIA_DYNAMICSDEBUG_H__

/**\file
 * Debugging of dynamic systems
 */

#include "csutil/scf.h"
#include "ivaria/bullet.h"

struct iDynamicSystem;
struct iSector;
struct iMaterialWrapper;

namespace CS {
namespace Debug {

struct iDynamicSystemDebugger;

/**
 * Creation of dynamic system debuggers.
 */
struct iDynamicsDebuggerManager : public virtual iBase
{
  SCF_INTERFACE(CS::Debug::iDynamicsDebuggerManager, 1, 0, 0);

  /**
   * Create a new debugger.
   */
  virtual CS::Debug::iDynamicSystemDebugger* CreateDebugger () = 0;
};

/**
 * A class to help visualization and debugging of physical
 * simulations made through the iDynamicSystem plugin.
 */
struct iDynamicSystemDebugger : public virtual iBase
{
  SCF_INTERFACE(CS::Debug::iDynamicSystemDebugger, 1, 0, 1);

  /**
   * Set the dynamic system that has to be debugged.
   */
  virtual void SetDynamicSystem (iDynamicSystem* system) = 0;

  /**
   * Set the iSector where reside the meshes animated by the
   * dynamic simulation.
   */
  virtual void SetDebugSector (iSector* sector) = 0;

  /**
   * Set whether the debug mode is active or not. If active, then all
   * the meshes attached to a iRigidBody will be replaced by a new
   * mesh with the size and transform of the bodies' colliders. It
   * allows to see what is really happening at the physical simulation
   * level.
   * \param debugMode True to activate the debug mode, false to set
   * back the initial meshes.
   */
  virtual void SetDebugDisplayMode (bool debugMode) = 0;

  /**
   * Update the list of colliders that are displayed. Call this when you have
   * added or removed some dynamic bodies to/from the dynamic system.
   */
  virtual void UpdateDisplay () = 0;

  /**
   * Set the material to be used for the colliders of the rigid bodies that are
   * in 'static' state. If 0 is passed then the rigid bodies in 'static' state
   * won't be displayed. If this method is not used, then a default red colored
   * material will be used.
   */
  virtual void SetStaticBodyMaterial (iMaterialWrapper* material) = 0;

  /**
   * Set the material to be used for the colliders of the rigid bodies that are
   * in 'dynamic' state. If 0 is passed then the rigid bodies in 'dynamic' state
   * won't be displayed. If this method is not used, then a default green colored
   * material will be used.
   */
  virtual void SetDynamicBodyMaterial (iMaterialWrapper* material) = 0;

  /**
   * Set the material to be used for the colliders of the rigid bodies that are
   * in the given state. If 0 is passed then the rigid bodies in the given state
   * won't be displayed. If this method is not used, then a default blue colored
   * material will be used.
   */
  virtual void SetBodyStateMaterial (CS::Physics::Bullet::BodyState state,
				     iMaterialWrapper* material) = 0;
};

} //namespace Debug
} //namespace CS

#endif // __CS_IVARIA_DYNAMICSDEBUG_H__
