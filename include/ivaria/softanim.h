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

struct iRigidBody;

namespace CS {
namespace Physics {
namespace Bullet {

struct iSoftBody;

} // namespace Bullet
} // namespace Physics
} // namespace CS

namespace CS {
namespace Mesh {

struct iAnimatedMesh;

} // namespace Mesh
} // namespace CS

namespace CS {
namespace Animation {

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
  SCF_INTERFACE (CS::Animation::iSoftBodyAnimationControlType, 1, 0, 0);
};

/**
 * Animation control factory for a genmesh animated by a CS::Physics::Bullet::iSoftBody.
 *
 * Main creators of instances implementing this interface:
 * - CS::Animation::iSoftBodyAnimationControlType::CreateAnimationControlFactory()
 *
 * Main ways to get pointers to this interface:
 * - iGeneralFactoryState::GetAnimationControlFactory()
 *
 * Main users of this interface:
 * - Genmesh plugin (crystalspace.mesh.object.genmesh) 
 */
struct iSoftBodyAnimationControlFactory : public iGenMeshAnimationControlFactory
{
  SCF_INTERFACE (CS::Animation::iSoftBodyAnimationControlFactory, 1, 0, 0);
};

/**
 * Animation control for a genmesh animated by a CS::Physics::Bullet::iSoftBody. This class will
 * animate the vertices of the genmesh depending on the physical simulation of the
 * soft body. It will also update automatically the position of the genmesh.
 *
 * The soft body controlling the animation of the genmesh can also be attached precisely to a
 * given vertex of an animesh. This allows to have the soft body following precisely the vertices
 * of the animesh, even when it is deformed by the skinning and morphing processes.
 *
 * Main creators of instances implementing this interface:
 * - CS::Animation::iSoftBodyAnimationControlFactory::CreateAnimationControl()
 *
 * Main ways to get pointers to this interface:
 * - iGeneralMeshState::GetAnimationControl()
 *
 * Main users of this interface:
 * - Genmesh plugin (crystalspace.mesh.object.genmesh) 
 */
struct iSoftBodyAnimationControl : public iGenMeshAnimationControl
{
  SCF_INTERFACE (CS::Animation::iSoftBodyAnimationControl, 2, 0, 0);

  /**
   * Set the soft body to be used to animate the genmesh. You can switch this soft body
   * at any time, the animation of the genmesh will just be adapted to the new soft body.
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

  /**
   * Create an anchor between the soft body and an animesh. The position of the anchor
   * will be updated accordingly when the vertex is moved by the skinning and morphing
   * processes of the animesh.
   *
   * This anchor is only effective if the vertex of the animesh is influenced by more
   * than one bone or by some morph targets. If it is not the case then it is more
   * efficient to simply use CS::Physics::Bullet::iSoftBody::AnchorVertex(size_t,iRigidBody*).
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

} // namespace Animation
} // namespace CS

#endif // __CS_IVARIA_SOFTBODYANIM_H__
