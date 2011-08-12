/*
  Copyright (C) 2009-10 Christian Van Brussel, Communications and Remote
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
#ifndef __CS_IMESH_BODYMESH_H__
#define __CS_IMESH_BODYMESH_H__

/**\file
 * Physical description of an animated mesh.
 */

#include "csutil/scf_interface.h"

#include "imesh/animnode/skeleton2anim.h"
#include "ivaria/collision2.h"
#include "ivaria/physical2.h"

/**\addtogroup meshplugins
 * @{ */

namespace CS {
namespace Animation {

struct iSkeletonFactory2;
struct iBodySkeleton2;
struct iBodyBone2;
struct iBodyChain2;
struct iBodyChainNode2;
struct iBodyBoneProperties2;
struct iBodyBoneJoint2;
struct iBodyBoneCollider2;

/**
 * A class to manage the creation and deletion of bodies' skeletons.
 */
struct iBodyManager2 : public virtual iBase
{
  SCF_INTERFACE(CS::Animation::iBodyManager2, 1, 0, 0);

  /**
   * Create a new body skeleton with the given name.
   */
  virtual iBodySkeleton2* CreateBodySkeleton
    (const char *name, iSkeletonFactory* skeletonFactory) = 0;

  /**
   * Find a body skeleton from its name.
   */
  virtual iBodySkeleton2* FindBodySkeleton (const char *name) = 0;

  /**
   * Delete all body skeletons.
   */
  virtual void ClearBodySkeletons () = 0;
};

/**
 * This class holds the physical description of the skeleton of an animated mesh.
 * For each relevant bone of the skeleton, one has to define an CS::Animation::iBodyBone that 
 * will hold the colliders, joint and properties of the bone.
 * Subtrees of the skeleton are defined through the CS::Animation::iBodyChain object.
 */
struct iBodySkeleton2 : public virtual iBase
{
  SCF_INTERFACE(CS::Animation::iBodySkeleton2, 1, 0, 0);

  /**
   * Return the name of the body skeleton.
   */
  virtual const char* GetName () const = 0;

  /**
   * Get the skeleton factory associated with this body skeleton.
   */
  virtual iSkeletonFactory* GetSkeletonFactory () const = 0;

  /**
   * Delete all body bones and all body chains.
   */
  virtual void ClearAll () = 0;

  /**
   * Create a new body bone.
   * \param boneID The ID of the animesh bone associated to the body bone.
   */
  virtual iBodyBone2* CreateBodyBone (BoneID boneID) = 0;

  /**
   * Find a body bone from the name of the associated animesh bone.
   */
  virtual iBodyBone2* FindBodyBone (const char *name) const = 0;

  /**
   * Delete all body bones.
   */
  virtual void ClearBodyBones () = 0;

  /**
   * Create a new body chain, ie a subtree of the animesh skeleton.
   * \param name The name of the body chain.
   * \param rootBone The root of the body chain.
   * \return The body chain upon success, nullptr if there was a problem.
   */
  virtual iBodyChain2* CreateBodyChain (const char *name, BoneID rootBone) = 0;

  /**
   * Find a body chain from its name.
   */
  virtual iBodyChain2* FindBodyChain (const char *name) const = 0;

  /**
   * Delete all body chains.
   */
  virtual void ClearBodyChains () = 0;

  /**
   * Find a body bone from the ID of the associated animesh bone.
   */
  virtual iBodyBone2* FindBodyBone (BoneID bone) const = 0;
};

/**
 * A body bone holds the physical description of the bone of an animated mesh.
 */
struct iBodyBone2 : public virtual iBase
{
  SCF_INTERFACE(CS::Animation::iBodyBone2, 1, 0, 0);

  /**
   * Return the ID of the bone of the animated mesh associated with this body bone.
   */
  virtual BoneID GetAnimeshBone () const = 0;

  /**
   * Create physical properties for this body bone. Only one CS::Animation::iBodyBoneProperties
   * can be created per bone. If no properties are defined then they will be
   * computed automatically by the dynamic system.
   */
  virtual iBodyBoneProperties2* CreateBoneProperties () = 0;

  /**
   * Get the physical properties of this body bone.
   * \return 0 if no properties were defined. 
   */
  virtual iBodyBoneProperties2* GetBoneProperties () const = 0;

  ///**
  // * Create a joint for this body bone. Only one CS::Animation::iBodyBoneJoint
  // * can be created per bone.
  // */
  virtual iBodyBoneJoint2* CreateBoneJoint () = 0;

  ///**
  // * Get the description of the joint of this body bone.
  // * \return 0 if no joint was defined. 
  // */
  virtual iBodyBoneJoint2* GetBoneJoint () const = 0;

  ///**
  // * Add a new collider for this body bone.
  // */
  virtual iBodyBoneCollider2* CreateBoneCollider () = 0;

  ///**
  // * Get the count of colliders for this body bone.
  // */
  virtual size_t GetBoneColliderCount () const = 0;

  /**
   * Get the collider of this body bone with the given index.
   */
  virtual iBodyBoneCollider2* GetBoneCollider (size_t index) const = 0;
};

/**
 * A body chain is a subtree of the skeleton of an animated mesh. It is used
 * to apply varying animation controllers on different parts of the skeleton.
 */
struct iBodyChain2 : public virtual iBase
{
  SCF_INTERFACE(CS::Animation::iBodyChain2, 1, 0, 0);

  /**
   * Get the name of this body chain.
   */
  virtual const char* GetName () const = 0;

  /**
   * Get the associated physical description of the skeleton
   */
  virtual iBodySkeleton2* GetBodySkeleton () const = 0;

  /**
   * Get the root node of this chain.
   */
  virtual iBodyChainNode2* GetRootNode () const = 0;

  /**
   * Add a sub-chain to this chain, ie all nodes from the root of the chain to the given subBone.
   *
   * For example, if you want to create a chain for the two legs of a human 
   * body, you should set the pelvis as the root node, and call this method once for each foot.
   * \return True upon success, false otherwise (ie the subBone has not been found as a sub-child
   * of the root of this chain)
   */
  virtual bool AddSubChain (CS::Animation::BoneID subBone) = 0;

  /**
   * Add all sub-child of the root node of this chain.
   * \return True upon success, false otherwise (this should never happen although)
   */
  virtual bool AddAllSubChains () = 0;

  /**
   * Print the hierarchical structure of this bone chain to the standard output.
   */
  virtual void DebugPrint () const = 0;

  /**
   * Populate the given bone mask with the bones of this chain. The bit mask will
   * not be reset before populating it, but its size will be grown if needed to the
   * count of bones in the skeleton.
   */
  virtual void PopulateBoneMask (csBitArray& boneMask) const = 0;
};

/**
 * A node in a body chain tree. A node is directly associated to a 
 * body bone.
 */
struct iBodyChainNode2 : public virtual iBase
{
  SCF_INTERFACE(CS::Animation::iBodyChainNode2, 1, 0, 0);

  /**
   * Return the ID of the bone of the animated mesh associated with this node.
   */
  virtual BoneID GetAnimeshBone () const = 0;

  /**
   * Get the count of children of this node.
   */
  virtual size_t GetChildCount () const = 0;

  /**
   * Get the given child of this node.
   */
  virtual iBodyChainNode2* GetChild (size_t index) const = 0;

  /**
   * Get the parent node of this node. Returns 0 if the node 
   * is the root of the body chain.
   */
  virtual iBodyChainNode2* GetParent () const = 0;

  /**
   * Find the node of the bone with the given BoneID in the descendent children of this node.
   * Return 0 if the given bone was not found.
   */
  virtual iBodyChainNode2* FindSubChild (CS::Animation::BoneID child) const = 0;

  /**
   * Print the hierarchical structure of this node and its children to the standard output.
   */
  virtual void DebugPrint () const = 0;
};

/**
 * These are the main properties of a bone's rigid body.
 */
struct iBodyBoneProperties2 : public virtual iBase
{
  SCF_INTERFACE(CS::Animation::iBodyBoneProperties2, 1, 0, 0);

  /**
   * Set the total mass of the rigid body of this bone.
   */
  virtual void SetMass (float mass) = 0;

  /**
   * Get the total mass of the rigid body of this bone.
   */
  virtual float GetMass () const = 0;
 
  /**
   * Set the local transform of this collider.
   */
  virtual void SetTransform (const csOrthoTransform &transform) = 0;

  /**
   * Get the local transform of this collider.
   */
  virtual csOrthoTransform GetTransform () const = 0;

  /**
   * Set the friction of the collider surface.
   */
  virtual void SetFriction (float friction) = 0;

  /**
   * Get the friction of the collider surface.
   */
  virtual float GetFriction () const = 0;

  /**
   * Set the softness of the collider surface.
   */
  virtual void SetSoftness (float softness) = 0;

  /**
   * Get the softness of the collider surface.
   */
  virtual float GetSoftness () const = 0;

  /**
   * Set the elasticity of the collider surface.
   */
  virtual void SetElasticity (float elasticity) = 0;

  /**
   * Get the elasticity of the collider surface.
   */
  virtual float GetElasticity () const = 0;

  /**
   * Set the density of the body.
   */
  virtual void SetDensity (float density) = 0;

  /**
   * Get the density of the body.
   */
  virtual float GetDensity () const = 0;
};


/**
 * These are the properties of the joint associated to a bone. Refer 
 * to the CS::Physical2::iJoint documentation for further information on these properties.
 */
struct iBodyBoneJoint2 : public virtual iBase
{
  SCF_INTERFACE(CS::Animation::iBodyBoneJoint2, 1, 0, 0);

  /**
   * Set the restitution of the joint's stop point.
   */
  virtual void SetBounce (const csVector3 &bounce) = 0;

  /**
   * Set the maximum constrained angle between bodies (in radian).
   */
  virtual void SetMaximumAngle (const csVector3 &max) = 0;

  /**
   * Set the maximum constrained distance between bodies.
   */
  virtual void SetMaximumDistance (const csVector3 &max) = 0;

  /**
   * Set the minimum constrained angle between bodies (in radian).
   */
  virtual void SetMinimumAngle (const csVector3 &min) = 0;

  /**
   * Set the minimum constrained distance between bodies.
   */
  virtual void SetMinimumDistance (const csVector3 &min) = 0;

  /**
   * Set the rotational constraints on the 3 axes.
   */
  virtual void SetRotConstraints (bool X, bool Y, bool Z) = 0;

  /**
   * Set the translation constraints on the 3 axes.
   */
  virtual void SetTransConstraints (bool X, bool Y, bool Z) = 0;
 
  /**
   * Set the transform of the joint relatively to this bone.
   */
  virtual void SetTransform (const csOrthoTransform &transform) = 0;
 
  /**
   * Get the restitution of the joint's stop point.
   */
  virtual csVector3 GetBounce () const = 0;

  /**
   * Get the maximum constrained angle between bodies (in radian).
   */
  virtual csVector3 GetMaximumAngle () const = 0;

  /**
   * Get the maximum constrained distance between bodies.
   */
  virtual csVector3 GetMaximumDistance () const = 0;

  /**
   * Get the minimum constrained angle between bodies (in radian).
   */
  virtual csVector3 GetMinimumAngle () const = 0;

  /**
   * Get the minimum constrained distance between bodies.
   */
  virtual csVector3 GetMinimumDistance () const = 0;

  /**
   * True if this axis' rotation is constrained.
   */
  virtual bool IsXRotConstrained () const = 0;

  /**
   * True if this axis' translation is constrained.
   */
  virtual bool IsXTransConstrained () const = 0;

  /**
   * True if this axis' rotation is constrained.
   */
  virtual bool IsYRotConstrained () const = 0;

  /**
   * True if this axis' translation is constrained.
   */
  virtual bool IsYTransConstrained () const = 0;

  /**
   * True if this axis' rotation is constrained.
   */
  virtual bool IsZRotConstrained () const = 0;

  /**
   * True if this axis' translation is constrained.
   */
  virtual bool IsZTransConstrained () const = 0;

  /**
   * Get the transform of the joint relatively to this bone.
   */
  virtual csOrthoTransform GetTransform () const = 0;
};

/**
 * These are the properties of the collider associated to a bone. Refer 
 * to the CS::Collision2::iCollider documentation for further information 
 * on these properties.
 */
struct iBodyBoneCollider2 : public virtual iBase
{
  SCF_INTERFACE(CS::Animation::iBodyBoneCollider2, 1, 0, 0);

  /**
   * Set the collider as a box (defined by the given size).
   */
  virtual bool SetBoxGeometry (const csVector3 &box_size) = 0;

  /**
   * Set the collider as a capsule.
   */
  virtual bool SetCapsuleGeometry (float length, float radius) = 0;

  /**
   * Set the collider as the geometry of the given convex mesh factory.
   */
  virtual bool SetConvexMeshGeometry (iMeshWrapper *mesh) = 0;

  /**
   * Set the collider as a cylinder.
   */
  virtual bool SetCylinderGeometry (float length, float radius) = 0;

    /**
   * Set the collider as a cone.
   */
  virtual bool SetConeGeometry (float length, float radius) = 0;

  /**
   * Set the collider as the geometry of the given concave mesh factory.
   */
  virtual bool SetConcaveMeshGeometry (iMeshWrapper *mesh) = 0;

  /**
   * Set the collider as a plane. 
   */
  virtual bool SetPlaneGeometry (const csPlane3 &plane) = 0;

  /**
   * Set the collider as a sphere. 
   */
  virtual bool SetSphereGeometry (const float radius) = 0;
 
  /**
   * Get the type of the collider geometry.
   */
  virtual CS::Collision2::ColliderType GetGeometryType () const = 0;

  /**
   * If this collider has a box geometry then the method will return true and the 
   * size of the box, otherwise it will return false.
   */
  virtual bool GetBoxGeometry (csVector3 &box_size) const = 0;

  /**
   * If this collider has a capsule geometry then the method will return true and
   * the capsule's length and radius, otherwise it will return false.
   */
  virtual bool GetCapsuleGeometry (float &length, float &radius) const = 0;

  /**
   * If this collider has a cone geometry then the method will return true and
   * the capsule's length and radius, otherwise it will return false.
   */
  virtual bool GetConeGeometry (float &length, float &radius) const = 0;

  /**
   * If this collider has a convex mesh geometry then the method will return true and
   * the mesh, otherwise it will return false.
   */
  virtual bool GetConvexMeshGeometry (iMeshWrapper *&mesh) const = 0;

  /**
   * If this collider has a cylinder geometry then the method will return true and
   * the cylinder's length and radius, otherwise it will return false.
   */
  virtual bool GetCylinderGeometry (float &length, float &radius) const = 0;

  /**
   * If this collider has a concave mesh geometry then the method will return true and
   * the mesh, otherwise it will return false.
   */
  virtual bool GetConcaveMeshGeometry (iMeshWrapper *&mesh) const = 0;

  /**
   * If this collider has a plane geometry then the method will return true and 
   * the plane, otherwise it will return false.
   */
  virtual bool GetPlaneGeometry (csPlane3 &plane) const = 0;

  /**
   * If this collider has a sphere geometry then the method will return true and
   * the sphere, otherwise it will return false.
   */
  virtual bool GetSphereGeometry (float &radius) const = 0;

};

} // namespace Animation
} // namespace CS

/** @} */

#endif // __CS_IMESH_BODYMESH_H__
