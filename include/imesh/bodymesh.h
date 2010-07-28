/*
  Copyright (C) 2009 Christian Van Brussel, Communications and Remote
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

#include "imesh/skeleton2anim.h"
#include "ivaria/dynamics.h"

/**\addtogroup meshplugins
 * @{ */

struct iSkeletonFactory2;
struct iBodySkeleton;
struct iBodyBone;
struct iBodyChain;
struct iBodyChainNode;
struct iBodyBoneProperties;
struct iBodyBoneJoint;
struct iBodyBoneCollider;

/**
 * A class to manage the creation and deletion of bodies' skeletons.
 */
struct iBodyManager : public virtual iBase
{
  SCF_INTERFACE(iBodyManager, 2, 0, 0);

  /**
   * Create a new body skeleton with the specified name.
   */
  virtual iBodySkeleton* CreateBodySkeleton (const char *name,
			      iSkeletonFactory2* skeletonFactory) = 0;

  /**
   * Find a body skeleton from its name.
   */
  virtual iBodySkeleton* FindBodySkeleton (const char *name) = 0;

  /**
   * Delete all body skeletons.
   */
  virtual void ClearBodySkeletons () = 0;
};

/**
 * This class holds the physical description of the skeleton of an animated mesh.
 * For each relevant bone of the skeleton, one has to define an iBodyBone that 
 * will hold the colliders, joint and properties of the bone.
 * Subtrees of the skeleton are defined through the iBodyChain object.
 */
struct iBodySkeleton : public virtual iBase
{
  SCF_INTERFACE(iBodySkeleton, 2, 0, 1);

  /**
   * Return the name of the body skeleton.
   */
  virtual const char* GetName () const = 0;

  /**
   * Get the skeleton factory associated with this body skeleton.
   */
  virtual iSkeletonFactory2* GetSkeletonFactory () const = 0;

  /**
   * Delete all body bones and all body chains.
   */
  virtual void ClearAll () = 0;

  /**
   * Create a new body bone.
   * \param boneID The ID of the animesh bone associated to the body bone.
   */
  virtual iBodyBone* CreateBodyBone (BoneID boneID) = 0;

  /**
   * Find a body bone from the name of the associated animesh bone.
   */
  virtual iBodyBone* FindBodyBone (const char *name) const = 0;

  /**
   * Delete all body bones.
   */
  virtual void ClearBodyBones () = 0;

  /**
   * Create a new body chain, ie a subtree of the animesh skeleton.
   * For example, if you want to create a chain for the two legs of a human 
   * body, you should set the pelvis as the root bone, and set two child bones
   * with the feet.
   * \param name The name of the body chain.
   * \param rootBone The root of the body chain. It must be followed 
   * by a list of child bones. All bones from the root bone to the child 
   * bones will be added. All bones that are added must have a iBodyBone 
   * defined.
   * \return The body chain upon success, 0 if there was a problem.
   */
  virtual iBodyChain* CreateBodyChain (
    const char *name, BoneID rootBone, ...) = 0;

  /**
   * Find a body chain from its name.
   */
  virtual iBodyChain* FindBodyChain (const char *name) const = 0;

  /**
   * Delete all body chains.
   */
  virtual void ClearBodyChains () = 0;

  /**
   * Find a body bone from the ID of the associated animesh bone.
   */
  virtual iBodyBone* FindBodyBone (BoneID bone) const = 0;
};

/**
 * A body bone holds the physical description of the bone of an animated mesh.
 */
struct iBodyBone : public virtual iBase
{
  SCF_INTERFACE(iBodyBone, 1, 0, 0);

  /**
   * Return the ID of the bone of the animated mesh associated to this body bone.
   */
  virtual BoneID GetAnimeshBone () const = 0;

  /**
   * Create physical properties for this body bone. Only one iBodyBoneProperties
   * can be created per bone. If no properties are defined then they will be
   * computed automatically by the dynamic system.
   */
  virtual iBodyBoneProperties* CreateBoneProperties () = 0;

  /**
   * Get the physical properties of this body bone.
   * \return 0 if no properties were defined. 
   */
  virtual iBodyBoneProperties* GetBoneProperties () const = 0;

  /**
   * Create a joint for this body bone. Only one iBodyBoneJoint
   * can be created per bone.
   */
  virtual iBodyBoneJoint* CreateBoneJoint () = 0;

  /**
   * Get the description of the joint of this body bone.
   * \return 0 if no joint was defined. 
   */
  virtual iBodyBoneJoint* GetBoneJoint () const = 0;

  /**
   * Add a new collider for this body bone.
   */
  virtual iBodyBoneCollider* CreateBoneCollider () = 0;

  /**
   * Get the count of colliders for this body bone.
   */
  virtual uint GetBoneColliderCount () const = 0;

  /**
   * Get the specified collider of this body bone.
   */
  virtual iBodyBoneCollider* GetBoneCollider (uint index) const = 0;
};

/**
 * A body chain is a subtree of the skeleton of an animated mesh. It is used
 * to apply varying animation controllers on different parts of the skeleton.
 */
struct iBodyChain : public virtual iBase
{
  SCF_INTERFACE(iBodyChain, 1, 0, 0);

  /**
   * Get the name of this body chain.
   */
  virtual const char* GetName () const = 0;

  /**
   * Get the root node of this chain.
   */
  virtual iBodyChainNode* GetRootNode () const = 0;
};

/**
 * A node in a body chain tree. A node is directly associated to a 
 * body bone.
 */
struct iBodyChainNode : public virtual iBase
{
  SCF_INTERFACE(iBodyChainNode, 1, 0, 0);

  /**
   * Get the body bone associated with this node.
   */
  virtual iBodyBone* GetBodyBone () const = 0;

  /**
   * Get the count of children of this node.
   */
  virtual uint GetChildCount () const = 0;

  /**
   * Get the specified child of this node.
   */
  virtual iBodyChainNode* GetChild (uint index) const = 0;

  /**
   * Get the parent node of this node. Returns 0 if the node 
   * is the root of the body chain.
   */
  virtual iBodyChainNode* GetParent () const = 0;
};

/**
 * These are the main properties of a bone's rigid body.
 */
struct iBodyBoneProperties : public virtual iBase
{
  SCF_INTERFACE(iBodyBoneProperties, 1, 0, 0);

  /**
   * Set the total mass of the rigid body of this bone.
   */
  virtual void SetMass (float mass) = 0;

  /**
   * Get the total mass of the rigid body of this bone.
   */
  virtual float GetMass () const = 0;

  /**
   * Set the center of mass of the rigid body of this bone.
   */
  virtual void SetCenter (const csVector3 &center) = 0;

  /**
   * Get the center of mass of the rigid body of this bone.
   */
  virtual csVector3 GetCenter () const = 0;

  /**
   * Set the matrix of inertia of the rigid body of this bone.
   */
  virtual void SetInertia (const csMatrix3 &inertia) = 0;

  /**
   * Get the matrix of inertia of the rigid body of this bone.
   */
  virtual csMatrix3 GetInertia () const = 0;
};

/**
 * These are the properties of the joint associated to a bone. Refer 
 * to the iJoint documentation for further information on these properties.
 */
struct iBodyBoneJoint : public virtual iBase
{
  SCF_INTERFACE(iBodyBoneJoint, 1, 0, 0);

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
   * Get the restitution of the joint's stop point.
   */
  virtual csVector3 GetBounce () = 0;

  /**
   * Get the maximum constrained angle between bodies (in radian).
   */
  virtual csVector3 GetMaximumAngle () = 0;

  /**
   * Get the maximum constrained distance between bodies.
   */
  virtual csVector3 GetMaximumDistance () = 0;

  /**
   * Get the minimum constrained angle between bodies (in radian).
   */
  virtual csVector3 GetMinimumAngle () = 0;

  /**
   * Get the minimum constrained distance between bodies.
   */
  virtual csVector3 GetMinimumDistance () = 0;

  /**
   * True if this axis' rotation is constrained.
   */
  virtual bool IsXRotConstrained () = 0;

  /**
   * True if this axis' translation is constrained.
   */
  virtual bool IsXTransConstrained () = 0;

  /**
   * True if this axis' rotation is constrained.
   */
  virtual bool IsYRotConstrained () = 0;

  /**
   * True if this axis' translation is constrained.
   */
  virtual bool IsYTransConstrained () = 0;

  /**
   * True if this axis' rotation is constrained.
   */
  virtual bool IsZRotConstrained () = 0;

  /**
   * True if this axis' translation is constrained.
   */
  virtual bool IsZTransConstrained () = 0;
};

/**
 * These are the properties of the collider associated to a bone. Refer 
 * to the iDynamicsSystemCollider documentation for further information 
 * on these properties.
 */
struct iBodyBoneCollider : public virtual iBase
{
  SCF_INTERFACE(iBodyBoneCollider, 2, 0, 0);

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
   * Set the collider as the geometry of the given concave mesh factory.
   */
  virtual bool SetMeshGeometry (iMeshWrapper *mesh) = 0;

  /**
   * Set the collider as a plane. 
   */
  virtual bool SetPlaneGeometry (const csPlane3 &plane) = 0;

  /**
   * Set the collider as a sphere. 
   */
  virtual bool SetSphereGeometry (const csSphere &sphere) = 0;
 
  /**
   * Get the type of the collider geometry.
   */
  virtual csColliderGeometryType GetGeometryType () = 0;

  /**
   * If this collider has a box geometry then the method will return true and the 
   * size of the box, otherwise it will return false.
   */
  virtual bool GetBoxGeometry (csVector3 &box_size) = 0;

  /**
   * If this collider has a capsule geometry then the method will return true and
   * the capsule's length and radius, otherwise it will return false.
   */
  virtual bool GetCapsuleGeometry (float &length, float &radius) = 0;

  /**
   * If this collider has a convex mesh geometry then the method will return true and
   * the mesh, otherwise it will return false.
   */
  virtual bool GetConvexMeshGeometry (iMeshWrapper *&mesh) = 0;

  /**
   * If this collider has a cylinder geometry then the method will return true and
   * the cylinder's length and radius, otherwise it will return false.
   */
  virtual bool GetCylinderGeometry (float &length, float &radius) = 0;

  /**
   * If this collider has a mesh geometry then the method will return true and
   * the mesh, otherwise it will return false.
   */
  virtual bool GetMeshGeometry (iMeshWrapper *&mesh) = 0;

  /**
   * If this collider has a plane geometry then the method will return true and 
   * the plane, otherwise it will return false.
   */
  virtual bool GetPlaneGeometry (csPlane3 &plane) = 0;

  /**
   * If this collider has a sphere geometry then the method will return true and
   * the sphere, otherwise it will return false.
   */
  virtual bool GetSphereGeometry (csSphere &sphere) = 0;
 
  /**
   * Set the local transform of this collider.
   */
  virtual void SetTransform (const csOrthoTransform &transform) = 0;

  /**
   * Get the local transform of this collider.
   */
  virtual csOrthoTransform GetTransform () = 0;

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

/** @} */

#endif // __CS_IMESH_BODYMESH_H__
