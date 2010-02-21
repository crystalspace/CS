/*
  Copyright (C) 2010 Christian Van Brussel, Communications and Remote
      Sensing Laboratory of the School of Engineering at the 
      Universite catholique de Louvain, Belgium
      http://www.tele.ucl.ac.be

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
 * Debugging of dynamics systems
 */

#include "csutil/scf.h"

struct iDynamicSystemDebugger;
struct iDynamicSystem;
struct iSector;

/**
 * Creation of dynamic system debuggers.
 */
struct iDynamicsDebuggerManager : public virtual iBase
{
  SCF_INTERFACE(iDynamicsDebuggerManager, 1, 0, 0);

  /**
   * Create a new debugger.
   */
  virtual iDynamicSystemDebugger* CreateDebugger () = 0;
};

/**
 * A class to help visualization and debugging of physical
 * simulations made through the iDynamicSystem plugin.
 */
struct iDynamicSystemDebugger : public virtual iBase
{
  SCF_INTERFACE(iDynamicSystemDebugger, 1, 0, 0);

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
};

#endif // __CS_IVARIA_DYNAMICSDEBUG_H__
