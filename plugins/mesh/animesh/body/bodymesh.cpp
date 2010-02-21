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

#include "cssysdef.h"

#include "csutil/scf.h"
#include "bodymesh.h"
#include <iengine/mesh.h>
#include "ivaria/reporter.h"

CS_PLUGIN_NAMESPACE_BEGIN(Bodymesh)
{

  /********************
   *  BodyManager
   ********************/

  SCF_IMPLEMENT_FACTORY(BodyManager);

  CS_LEAKGUARD_IMPLEMENT(BodyManager);

  BodyManager::BodyManager (iBase* parent)
    : scfImplementationType (this, parent)
  {
  }

  iBodySkeleton* BodyManager::CreateBodySkeleton (const char *name,
			            iSkeletonFactory2* skeletonFactory)
  {
    // Check name uniqueness
    csRef<iBodySkeleton> newFact;
    newFact.AttachNew (new BodySkeleton (name, this, skeletonFactory));

    return factoryHash.PutUnique (name, newFact);
  }

  iBodySkeleton* BodyManager::FindBodySkeleton (const char *name)
  {
    return factoryHash.Get (name, 0);
  }

  void BodyManager::ClearBodySkeletons ()
  {
    factoryHash.DeleteAll ();
  }

  bool BodyManager::Initialize (iObjectRegistry* object_reg)
  {
    this->object_reg = object_reg;
    return true;
  }

  void BodyManager::Report (int severity, const char* msg, ...)
  {
    va_list arg;
    va_start (arg, msg);
    csRef<iReporter> rep (csQueryRegistry<iReporter> (object_reg));
    if (rep)
      rep->ReportV (severity, "crystalspace.mesh.animesh.body",
		    msg, arg);
    else
      {
	csPrintfV (msg, arg);
	csPrintf ("\n");
      }
    va_end (arg);
  }


  /********************
   *  BodySkeleton
   ********************/

  CS_LEAKGUARD_IMPLEMENT(BodySkeleton);

  BodySkeleton::BodySkeleton (const char* name, BodyManager* manager,
			      iSkeletonFactory2* skeletonFactory)
    : scfImplementationType (this), name (name), manager (manager),
    skeletonFactory (skeletonFactory)
  {
  }

  const char* BodySkeleton::GetName () const
  {
    return name;
  }

  iSkeletonFactory2* BodySkeleton::GetSkeletonFactory () const
  {
    return skeletonFactory;
  }

  void BodySkeleton::ClearAll ()
  {
    boneHash.DeleteAll ();
    chainHash.DeleteAll ();
  }

  iBodyBone* BodySkeleton::CreateBodyBone (BoneID boneID)
  {
    if (boneID == InvalidBoneID)
    {
      manager->Report (CS_REPORTER_SEVERITY_ERROR,
		       "Invalid bone ID while creating body bone");
      return 0;
    }

    // check boneid exists in skeleton
    if (!skeletonFactory)
    {
      manager->Report (CS_REPORTER_SEVERITY_ERROR,
	       "No skeleton factory defined while creating body bone");
      return 0;
    }    

    if (!skeletonFactory->HasBone (boneID))
    {
      manager->Report (CS_REPORTER_SEVERITY_ERROR,
       "Bone %i doesn't exist in skeleton factory while creating body bone",
		       boneID);
      return 0;
    }    

    // check ID uniqueness
    csRef<iBodyBone> newFact;
    newFact.AttachNew (new BodyBone (boneID));

    return boneHash.PutUnique (boneID, newFact);
  }

  iBodyBone* BodySkeleton::FindBodyBone (const char *name) const
  {
    BoneID boneID = skeletonFactory->FindBone (name);
    return boneHash.Get (boneID, 0);
  }

  iBodyBone* BodySkeleton::FindBodyBone (BoneID bone) const
  {
    return boneHash.Get (bone, 0);
  }

  void BodySkeleton::ClearBodyBones ()
  {
    boneHash.DeleteAll ();
  }

  iBodyChain* BodySkeleton::CreateBodyChain (
		const char *name, BoneID rootBone, ...)
  {
    // TODO:
    //   - adding only one node: one root + one child same as root
    //   - adding all child nodes: only one root + no child

    // check chain name not already given
    if (chainHash.Contains (name))
    {
      manager->Report (CS_REPORTER_SEVERITY_ERROR,
		       "Chain %s has already been defined.", name);
      return 0;
    }

    // check validity of root bone
    if (rootBone == InvalidBoneID)
    {
      manager->Report (CS_REPORTER_SEVERITY_ERROR,
		       "Invalid root bone while creating body chain.");
      return 0;
    }

    // check if the bone exists in the skeleton
    if (!skeletonFactory->HasBone (rootBone))
    {
      manager->Report (CS_REPORTER_SEVERITY_ERROR,
       "The root bone %i is not in the skeleton factory of the animesh",
		       rootBone);
      return 0;
    }

    // check if we have data on this bone
    if (!boneHash.Contains (rootBone))
    {
      manager->Report (CS_REPORTER_SEVERITY_ERROR,
		       "No iBodyBone defined for root bone: %i",
		       rootBone);
      return 0;
    }

    // create list of nodes, put in it the root node
    csHash<csRef<BodyChainNode>, BoneID> nodeHash;
    csRef<BodyChainNode> rootNode;
    rootNode.AttachNew (new BodyChainNode (boneHash.Get (rootBone, 0)));			
    nodeHash.Put (rootBone, rootNode);

    // for each final child nodes, add all nodes on the path to it
    va_list vl;
    va_start (vl, rootBone);

    BoneID bone, parent;
    while (1)
    {
      bone = va_arg (vl, BoneID);
      if (!bone) break;

      // TODO: check no repetition of child bones
      if (nodeHash.Contains (bone))
	continue;

      // check bone is valid
      if (bone == InvalidBoneID)
      {
	manager->Report (CS_REPORTER_SEVERITY_ERROR,
			 "Invalid child bone while creating chain");
	va_end (vl);
	return 0;
      }

      // check if the bone exists in the skeleton
      if (!skeletonFactory->HasBone (bone))
      {
	manager->Report (CS_REPORTER_SEVERITY_ERROR,
	 "The child bone %i is not in the skeleton factory of the animesh",
			 bone);
	va_end (vl);
	return 0;
      }

      // check if we have data on this bone
      if (!boneHash.Contains (bone))
      {
	manager->Report (CS_REPORTER_SEVERITY_ERROR,
			 "No iBodyBone defined for child bone: %i (%s)",
			 bone, skeletonFactory->GetBoneName (bone));
	va_end (vl);
	return 0;
      }

      // OK, create a chain node
      csRef<BodyChainNode> currentNode;
      currentNode.AttachNew (new BodyChainNode (boneHash.Get (bone, 0)));
      nodeHash.Put (bone, currentNode);

      // go up the skeleton tree until the root bone is found
      parent = skeletonFactory->GetBoneParent (bone);
      while (1)
      {
	// check that the root bone of the skeleton is not reached
	if (parent == InvalidBoneID)
	{
	  manager->Report (CS_REPORTER_SEVERITY_ERROR,
   "The specified child bone %i (%s) is not really a child of root bone %i (%s)",
			   bone, skeletonFactory->GetBoneName (bone),
			   rootBone, skeletonFactory->GetBoneName (rootBone));
	  va_end (vl);
	  return 0;
	}

	// check if we already got this bone
	if (nodeHash.Contains (parent))
	{
	  // add current node as a child of the node we already got
	  if (currentNode)
	  {
	    csRef<BodyChainNode> node = nodeHash.Get (parent, 0);
	    node->AddChild (currentNode);
	  }
	  break;
	}

	// check if we have data on this bone
	if (!boneHash.Contains (parent))
	{
	  manager->Report (CS_REPORTER_SEVERITY_ERROR,
	   "BodySkeleton::CreateChain: No iBodyBone defined for bone: %i (%s)",
			   parent, skeletonFactory->GetBoneName (parent));
	  va_end (vl);
	  return 0;
	}

	// OK, create a new node
	csRef<BodyChainNode> node;
	node.AttachNew (new BodyChainNode (boneHash.Get (parent, 0)));
	nodeHash.Put (parent, node);
	if (currentNode)
	  node->AddChild (currentNode);
	currentNode = node;

	parent = skeletonFactory->GetBoneParent (parent);
      }
    }

    va_end (vl);

    // chain is OK, create it
    csRef<BodyChain> chain;
    chain.AttachNew (new BodyChain (name, rootNode));

    return chainHash.PutUnique (name, chain);
  }

  iBodyChain* BodySkeleton::FindBodyChain (const char *name) const
  {
    return chainHash.Get (name, 0);
  }

  void BodySkeleton::ClearBodyChains ()
  {
    chainHash.DeleteAll ();
  }


  /********************
   *  BodyBone
   ********************/

  CS_LEAKGUARD_IMPLEMENT(BodyBone);

  BodyBone::BodyBone (BoneID boneID)
    : scfImplementationType (this), animeshBone (boneID)
  {
  }

  BoneID BodyBone::GetAnimeshBone () const
  {
    return animeshBone;
  }

  iBodyBoneProperties* BodyBone::CreateBoneProperties ()
  {
    properties.AttachNew (new BodyBoneProperties ());
    return properties;
  }

  iBodyBoneProperties* BodyBone::GetBoneProperties () const
  {
    return properties;
  }

  iBodyBoneJoint* BodyBone::CreateBoneJoint ()
  {
    joint.AttachNew (new BodyBoneJoint ());
    return joint;
  }

  iBodyBoneJoint* BodyBone::GetBoneJoint () const
  {
    return joint;
  }

  iBodyBoneCollider* BodyBone::CreateBoneCollider ()
  {
    csRef<BodyBoneCollider> collider;
    collider.AttachNew (new BodyBoneCollider ());
    colliders.Push (collider);
    return collider;
  }

  uint BodyBone::GetBoneColliderCount () const
  {
    return colliders.GetSize ();
  }

  iBodyBoneCollider* BodyBone::GetBoneCollider (uint index) const
  {
    CS_ASSERT (index < colliders.GetSize ());
    return colliders[index];
  }


  /********************
   *  BodyChain
   ********************/

  CS_LEAKGUARD_IMPLEMENT(BodyChain);

  BodyChain::BodyChain (const char *name, iBodyChainNode* rootNode)
    : scfImplementationType (this), name (name), rootNode (rootNode)
  {
  }

  const char* BodyChain::GetName () const
  {
    return name;
  }

  iBodyChainNode* BodyChain::GetRootNode () const
  {
    return rootNode;
  }


  /********************
   *  BodyChainNode
   ********************/

  CS_LEAKGUARD_IMPLEMENT(BodyChainNode);

  BodyChainNode::BodyChainNode (iBodyBone* bone)
    : scfImplementationType (this), bone (bone), parent (0)
  {
  }

  iBodyBone* BodyChainNode::GetBodyBone () const
  {
    return bone;
  }

  uint BodyChainNode::GetChildCount () const
  {
    return children.GetSize ();
  }

  iBodyChainNode* BodyChainNode::GetChild (uint index) const
  {
    return children.Get (index);
  }

  iBodyChainNode* BodyChainNode::GetParent () const
  {
    return parent;
  }

  void BodyChainNode::AddChild (BodyChainNode* node)
  {
    node->parent = this;
    children.Push (node);
  }


  /********************
   *  BodyBoneProperties
   ********************/

  CS_LEAKGUARD_IMPLEMENT(BodyBoneProperties);

  BodyBoneProperties::BodyBoneProperties ()
    : scfImplementationType (this), mass (0.0),
    center (0.0)
  {
  }

  void BodyBoneProperties::SetMass (float mass)
  {
    this->mass = mass;
  }

  float BodyBoneProperties::GetMass () const
  {
    return mass;
  }

  void BodyBoneProperties::SetCenter (const csVector3 &center)
  {
    this->center = center;
  }

  csVector3 BodyBoneProperties::GetCenter () const
  {
    return center;
  }

  void BodyBoneProperties::SetInertia (const csMatrix3 &inertia)
  {
    this->inertia = inertia;
  }

  csMatrix3 BodyBoneProperties::GetInertia () const
  {
    return inertia;
  }


  /********************
   *  BodyBoneJoint
   ********************/

  CS_LEAKGUARD_IMPLEMENT(BodyBoneJoint);

  BodyBoneJoint::BodyBoneJoint ()
    : scfImplementationType (this), bounce (0.0),
    maxAngle (-PI, -PI/2.0, -PI/2.0), maxDistance (-1.0),
    minAngle (PI, PI/2.0, PI/2.0), minDistance (1.0)
  {
    rotConstraints[0] = false;
    rotConstraints[1] = false;
    rotConstraints[2] = false;
    transConstraints[0] = false;
    transConstraints[1] = false;
    transConstraints[2] = false;
  }

  void BodyBoneJoint::SetBounce (const csVector3 &bounce)
  {
    this->bounce = bounce;
  }

  void BodyBoneJoint::SetMaximumAngle (const csVector3 &max)
  {
    maxAngle = max;
  }

  void BodyBoneJoint::SetMaximumDistance (const csVector3 &max)
  {
    maxDistance = max;
  }

  void BodyBoneJoint::SetMinimumAngle (const csVector3 &min)
  {
    minAngle = min;
  }

  void BodyBoneJoint::SetMinimumDistance (const csVector3 &min)
  {
    minDistance = min;
  }

  void BodyBoneJoint::SetRotConstraints (bool X, bool Y, bool Z)
  {
    rotConstraints[0] = X;
    rotConstraints[1] = Y;
    rotConstraints[2] = Z;
  }

  void BodyBoneJoint::SetTransConstraints (bool X, bool Y, bool Z)
  {
    transConstraints[0] = X;
    transConstraints[1] = Y;
    transConstraints[2] = Z;
  }
 
  csVector3 BodyBoneJoint::GetBounce ()
  {
    return bounce;
  }

  csVector3 BodyBoneJoint::GetMaximumAngle ()
  {
    return maxAngle;
  }

  csVector3 BodyBoneJoint::GetMaximumDistance ()
  {
    return maxDistance;
  }

  csVector3 BodyBoneJoint::GetMinimumAngle ()
  {
    return minAngle;
  }

  csVector3 BodyBoneJoint::GetMinimumDistance ()
  {
    return minDistance;
  }

  bool BodyBoneJoint::IsXRotConstrained ()
  {
    return rotConstraints[0];
  }

  bool BodyBoneJoint::IsXTransConstrained ()
  {
    return transConstraints[0];
  }

  bool BodyBoneJoint::IsYRotConstrained ()
  {
    return rotConstraints[1];
  }

  bool BodyBoneJoint::IsYTransConstrained ()
  {
    return transConstraints[1];
  }

  bool BodyBoneJoint::IsZRotConstrained ()
  {
    return rotConstraints[2];
  }

  bool BodyBoneJoint::IsZTransConstrained ()
  {
    return transConstraints[2];
  }


  /********************
   *  BodyBoneCollider
   ********************/

  CS_LEAKGUARD_IMPLEMENT(BodyBoneCollider);

  BodyBoneCollider::BodyBoneCollider ()
    : scfImplementationType (this), friction (0.0), softness (0.0),
    elasticity (0.0), density (0.0), geometryType (NO_GEOMETRY),
    box_size (0.0), length (0.0), radius (0.0)
  {
  }
  
  bool BodyBoneCollider::SetBoxGeometry (const csVector3 &box_size)
  {
    geometryType = BOX_COLLIDER_GEOMETRY;
    this->box_size = box_size;
    return true;
  }

  bool BodyBoneCollider::SetCapsuleGeometry (float length, float radius)
  {
    geometryType = CAPSULE_COLLIDER_GEOMETRY;
    this->length = length;
    this->radius = radius;
    return true;
  }

  bool BodyBoneCollider::SetConvexMeshGeometry (iMeshWrapper *mesh)
  {
    geometryType = CONVEXMESH_COLLIDER_GEOMETRY;
    this->mesh = mesh;
    return false;
  }

  bool BodyBoneCollider::SetCylinderGeometry (float length, float radius)
  {
    geometryType = CYLINDER_COLLIDER_GEOMETRY;
    this->length = length;
    this->radius = radius;
    return true;
  }

  bool BodyBoneCollider::SetMeshGeometry (iMeshWrapper *mesh)
  {
    geometryType = TRIMESH_COLLIDER_GEOMETRY;
    this->mesh = mesh;
    return true;
  }

  bool BodyBoneCollider::SetPlaneGeometry (const csPlane3 &plane)
  {
    geometryType = PLANE_COLLIDER_GEOMETRY;
    this->plane = plane;
    return true;
  }

  bool BodyBoneCollider::SetSphereGeometry (const csSphere &sphere)
  {
    geometryType = SPHERE_COLLIDER_GEOMETRY;
    this->sphere = sphere;
    return true;
  }
 
  csColliderGeometryType BodyBoneCollider::GetGeometryType ()
  {
    return geometryType;
  }

  bool BodyBoneCollider::GetBoxGeometry (csVector3 &box_size)
  {
    if (geometryType != BOX_COLLIDER_GEOMETRY)
      return false;

    box_size = this->box_size;
    return true;
  }

  bool BodyBoneCollider::GetCapsuleGeometry (float &length, float &radius)
  {
    if (geometryType != CAPSULE_COLLIDER_GEOMETRY)
      return false;

    length = this->length;
    radius = this->radius;
    return true;
  }

  bool BodyBoneCollider::GetConvexMeshGeometry (iMeshWrapper *&mesh)
  {
    if (geometryType != CONVEXMESH_COLLIDER_GEOMETRY)
      return false;

    mesh = this->mesh;
    return true;
  }

  bool BodyBoneCollider::GetCylinderGeometry (float &length, float &radius)
  {
    if (geometryType != CYLINDER_COLLIDER_GEOMETRY)
      return false;

    length = this->length;
    radius = this->radius;
    return true;
  }

  bool BodyBoneCollider::GetMeshGeometry (iMeshWrapper *&mesh)
  {
    if (geometryType != TRIMESH_COLLIDER_GEOMETRY)
      return false;

    mesh = this->mesh;
    return true;
  }

  bool BodyBoneCollider::GetPlaneGeometry (csPlane3 &plane)
  {
    if (geometryType != PLANE_COLLIDER_GEOMETRY)
      return false;

    plane = this->plane;
    return true;
  }

  bool BodyBoneCollider::GetSphereGeometry (csSphere &sphere)
  {
    if (geometryType != SPHERE_COLLIDER_GEOMETRY)
      return false;

    sphere = this->sphere;
    return true;
  }

  void BodyBoneCollider::SetTransform (const csOrthoTransform &transform)
  {
    this->transform = transform;
  }

  csOrthoTransform BodyBoneCollider::GetTransform ()
  {
    return transform;
  }

  void BodyBoneCollider::SetFriction (float friction)
  {
    this->friction = friction;
  }

  float BodyBoneCollider::GetFriction () const
  {
    return friction;
  }

  void BodyBoneCollider::SetSoftness (float softness)
  {
    this->softness = softness;
  }

  float BodyBoneCollider::GetSoftness () const
  {
    return softness;
  }

  void BodyBoneCollider::SetElasticity (float elasticity)
  {
    this->elasticity = elasticity;
  }

  float BodyBoneCollider::GetElasticity () const
  {
    return elasticity;
  }

  void BodyBoneCollider::SetDensity (float density)
  {
    this->density = density;
  }

  float BodyBoneCollider::GetDensity () const
  {
    return density;
  }

}
CS_PLUGIN_NAMESPACE_END(Bodymesh)
