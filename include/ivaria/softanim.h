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
#ifndef __CS_IVARIA_SOFTBODYANIM_H__
#define __CS_IVARIA_SOFTBODYANIM_H__

#include "csutil/scf_interface.h"
#include "imesh/genmesh.h"

namespace CS
{
  namespace Physics
  {
    namespace Bullet
    {
      struct iSoftBody;
    } // namespace Bullet
  } // namespace Physics
} // namespace CS

/**
 * Animation control type for a genmesh animated by a CS::Physics::Bullet::iSoftBody.
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
  SCF_INTERFACE (iSoftBodyAnimationControlType, 1, 0, 0);
};

/**
 * Animation control factory for a genmesh animated by a CS::Physics::Bullet::iSoftBody.
 *
 * Main creators of instances implementing this interface:
 * - iSoftBodyAnimationControlType::CreateAnimationControlFactory()
 *
 * Main ways to get pointers to this interface:
 * - iGeneralFactoryState::GetAnimationControlFactory()
 *
 * Main users of this interface:
 * - Genmesh plugin (crystalspace.mesh.object.genmesh) 
 */
struct iSoftBodyAnimationControlFactory : public iGenMeshAnimationControlFactory
{
  SCF_INTERFACE (iSoftBodyAnimationControlFactory, 1, 0, 0);
};

/**
 * Animation control for a genmesh animated by a CS::Physics::Bullet::iSoftBody. This class will
 * animate the vertices of the genmesh depending on the physical simulation of the
 * soft body. It will also update automatically the position of the genmesh.
 *
 * Main creators of instances implementing this interface:
 * - iSoftBodyAnimationControlFactory::CreateAnimationControl()
 *
 * Main ways to get pointers to this interface:
 * - iGeneralMeshState::GetAnimationControl()
 *
 * Main users of this interface:
 * - Genmesh plugin (crystalspace.mesh.object.genmesh) 
 */
struct iSoftBodyAnimationControl : public iGenMeshAnimationControl
{
  SCF_INTERFACE (iSoftBodyAnimationControl, 2, 0, 0);

  /**
   * Set the soft body to be used to animate the genmesh.
   * \param body The soft body that will be used to animate this genmesh.
   * \param doubleSided True if the genmesh is double-sided (ie this is a cloth
   * soft body), false otherwise. If the genmesh is double-sided, then the duplicated
   * vertices must be added at the end of the vertex array, so that a vertex of index
   * 'i' is duplicated at index 'i + body->GetVertexCount ()'.
   */
  virtual void SetSoftBody (CS::Physics::Bullet::iSoftBody* body, bool doubleSided = false) = 0;

  /**
   * Get the soft body used to animate the genmesh.
   */
  virtual CS::Physics::Bullet::iSoftBody* GetSoftBody () = 0;
};

#endif // __CS_IVARIA_SOFTBODYANIM_H__
