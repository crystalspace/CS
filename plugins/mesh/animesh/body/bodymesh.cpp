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

  CS::Animation::iBodySkeleton* BodyManager::CreateBodySkeleton (const char *name,
			            CS::Animation::iSkeletonFactory* skeletonFactory)
  {
    // Check name uniqueness
    csRef<CS::Animation::iBodySkeleton> newFact;
    newFact.AttachNew (new BodySkeleton (name, this, skeletonFactory));

    return factoryHash.PutUnique (name, newFact);
  }

  CS::Animation::iBodySkeleton* BodyManager::FindBodySkeleton (const char *name)
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
			      CS::Animation::iSkeletonFactory* skeletonFactory)
    : scfImplementationType (this), name (name), manager (manager),
    skeletonFactory (skeletonFactory)
  {
  }

  const char* BodySkeleton::GetName () const
  {
    return name;
  }

  CS::Animation::iSkeletonFactory* BodySkeleton::GetSkeletonFactory () const
  {
    return skeletonFactory;
  }

  void BodySkeleton::ClearAll ()
  {
    boneHash.DeleteAll ();
    chainHash.DeleteAll ();
  }

  CS::Animation::iBodyBone* BodySkeleton::CreateBodyBone (CS::Animation::BoneID boneID)
  {
    if (boneID == CS::Animation::InvalidBoneID)
    {
      manager->Report (CS_REPORTER_SEVERITY_ERROR,
		       "Invalid bone ID while creating body bone");
      return nullptr;
    }

    // check boneid exists in skeleton
    if (!skeletonFactory)
    {
      manager->Report (CS_REPORTER_SEVERITY_ERROR,
	       "No skeleton factory defined while creating body bone");
      return nullptr;
    }    

    if (!skeletonFactory->HasBone (boneID))
    {
      manager->Report (CS_REPORTER_SEVERITY_ERROR,
       "Bone %i doesn't exist in skeleton factory while creating body bone",
		       boneID);
      return nullptr;
    }    

    // check ID uniqueness
    csRef<BodyBone> newFact;
    newFact.AttachNew (new BodyBone (boneID));

    return boneHash.PutUnique (boneID, newFact);
  }

  CS::Animation::iBodyBone* BodySkeleton::FindBodyBone (const char *name) const
  {
    CS::Animation::BoneID boneID = skeletonFactory->FindBone (name);
    return boneHash.Get (boneID, 0);
  }

  CS::Animation::iBodyBone* BodySkeleton::FindBodyBone (CS::Animation::BoneID bone) const
  {
    return boneHash.Get (bone, 0);
  }

  void BodySkeleton::ClearBodyBones ()
  {
    boneHash.DeleteAll ();
  }

  CS::Animation::iBodyChain* BodySkeleton::CreateBodyChain (const char *name,
							    CS::Animation::BoneID rootBone)
  {
    // check that the chain name has not already be given
    if (chainHash.Contains (name))
    {
      manager->Report (CS_REPORTER_SEVERITY_ERROR,
		       "Chain %s has already been defined.", name);
      return nullptr;
    }

    // check the validity of the root bone
    if (!skeletonFactory->HasBone (rootBone))
    {
      manager->Report (CS_REPORTER_SEVERITY_ERROR,
       "The root bone %i is not in the skeleton factory of the animesh",
		       rootBone);
      return nullptr;
    }

    // chain is OK, create it
    csRef<BodyChain> chain;
    chain.AttachNew (new BodyChain (this, name, rootBone));

    return chainHash.PutUnique (name, chain);
  }

  CS::Animation::iBodyChain* BodySkeleton::FindBodyChain (const char *name) const
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

  BodyBone::BodyBone (CS::Animation::BoneID boneID)
    : scfImplementationType (this), animeshBone (boneID)
  {
  }

  CS::Animation::BoneID BodyBone::GetAnimeshBone () const
  {
    return animeshBone;
  }

  CS::Animation::iBodyBoneProperties* BodyBone::CreateBoneProperties ()
  {
    properties.AttachNew (new BodyBoneProperties ());
    return properties;
  }

  CS::Animation::iBodyBoneProperties* BodyBone::GetBoneProperties () const
  {
    return properties;
  }

  CS::Animation::iBodyBoneJoint* BodyBone::CreateBoneJoint ()
  {
    joint.AttachNew (new BodyBoneJoint ());
    return joint;
  }

  CS::Animation::iBodyBoneJoint* BodyBone::GetBoneJoint () const
  {
    return joint;
  }

  CS::Animation::iBodyBoneCollider* BodyBone::CreateBoneCollider ()
  {
    csRef<BodyBoneCollider> collider;
    collider.AttachNew (new BodyBoneCollider ());
    colliders.Push (collider);
    return collider;
  }

  size_t BodyBone::GetBoneColliderCount () const
  {
    return (size_t)colliders.GetSize ();
  }

  CS::Animation::iBodyBoneCollider* BodyBone::GetBoneCollider (size_t index) const
  {
    CS_ASSERT (index < colliders.GetSize ());
    return colliders[index];
  }


  /********************
   *  BodyChain
   ********************/

  CS_LEAKGUARD_IMPLEMENT(BodyChain);

  BodyChain::BodyChain (BodySkeleton* bodySkeleton, const char *name,
			CS::Animation::BoneID rootBone)
    : scfImplementationType (this), name (name), bodySkeleton (bodySkeleton)
  {
    rootNode.AttachNew (new BodyChainNode (rootBone));
  }

  const char* BodyChain::GetName () const
  {
    return name;
  }

  CS::Animation::iBodySkeleton* BodyChain::GetBodySkeleton () const
  {
    return bodySkeleton;
  }

  CS::Animation::iBodyChainNode* BodyChain::GetRootNode () const
  {
    return rootNode;
  }

  void CollectAllNodes (csHash<csRef<BodyChainNode>, CS::Animation::BoneID>& nodeHash,
			BodyChainNode* rootNode)
  {
    nodeHash.Put (rootNode->GetAnimeshBone (), rootNode);

    for (size_t i = 0; i < rootNode->GetChildCount (); i++)
      CollectAllNodes (nodeHash, (BodyChainNode*) rootNode->GetChild (i));
  }

  bool BodyChain::AddSubChain (CS::Animation::BoneID subBone)
  {
#ifdef CS_DEBUG
    // check if the bone is valid
    if (!bodySkeleton->skeletonFactory->HasBone (subBone))
    {
      bodySkeleton->manager->Report (CS_REPORTER_SEVERITY_ERROR,
				     "The child bone %i is not in the skeleton factory of the animesh",
				     subBone);
      return false;
    }
#endif

    // collect all nodes of this chain
    csHash<csRef<BodyChainNode>, CS::Animation::BoneID> currentHash;
    CollectAllNodes (currentHash, rootNode);

    // check that the sub bone is not already in the chain
    if (currentHash.Contains (subBone))
      return true;

    // store the new nodes so that they are deleted in case of error
    csHash<csRef<BodyChainNode>, CS::Animation::BoneID> nodeHash;

    // OK, create a chain node
    csRef<BodyChainNode> currentNode;
    currentNode.AttachNew (new BodyChainNode (subBone));
    nodeHash.Put (subBone, currentNode);

    // iterate on the parent nodes until we find a connection to the chain
    CS::Animation::BoneID parent = bodySkeleton->skeletonFactory->GetBoneParent (subBone);
    while (1)
    {
      // check that the root bone of the skeleton is not reached
      if (parent == CS::Animation::InvalidBoneID)
      {
	bodySkeleton->manager->Report (CS_REPORTER_SEVERITY_ERROR,
			 "The specified child bone %i (%s) is not really a child of root bone %i (%s)",
			 subBone, bodySkeleton->skeletonFactory->GetBoneName (subBone),
			 rootNode->boneID, bodySkeleton->skeletonFactory->GetBoneName (rootNode->boneID));
	return false;
      }

      // check if we already got this bone
      if (currentHash.Contains (parent))
      {
	// add the current node as a child of the node we already got
	csRef<BodyChainNode> node = currentHash.Get (parent, 0);
	node->AddChild (currentNode);
	break;
      }

      // OK, create a new node
      csRef<BodyChainNode> node;
      node.AttachNew (new BodyChainNode (parent));
      nodeHash.Put (parent, node);
      node->AddChild (currentNode);
      currentNode = node;

      parent = bodySkeleton->skeletonFactory->GetBoneParent (parent);
    }

    return true;
  }

  bool BodyChain::AddAllSubChains ()
  {
#ifdef CS_DEBUG
    // check if the chain is really empty
    if (rootNode->children.GetSize ())
    {
      bodySkeleton->manager->Report (CS_REPORTER_SEVERITY_ERROR,
				     "The chain %s is not empty while trying to add all sub chains",
				     name.GetData ());
      return false;
    }
#endif

    // create a structure holding all the bones of the skeleton.
    // When this method will return, all the bones that are not sub child of this chain will
    // be decref'd and therefore deleted.
    csHash<csRef<BodyChainNode>, CS::Animation::BoneID> nodeHash;
    nodeHash.Put (rootNode->boneID, rootNode);
    CS::Animation::BoneID maxBoneID = bodySkeleton->skeletonFactory->GetTopBoneID ();
    CS::Animation::BoneID rootBoneID = 0;

    for (CS::Animation::BoneID boneIt = 0; boneIt < maxBoneID; boneIt++)
    {
      // check if this is the root of this chain
      if (rootNode->boneID == boneIt)
	continue;

      // find or create an entry in the node hash
      csRef<BodyChainNode> node;
      if (nodeHash.Contains (boneIt))
	node = *nodeHash[boneIt];
      else
      {
	node.AttachNew (new BodyChainNode (boneIt));
	nodeHash.Put (boneIt, node);
      }

      // check if this is the root bone
      CS::Animation::BoneID parentBone = bodySkeleton->skeletonFactory->GetBoneParent (boneIt);
      if (parentBone == CS::Animation::InvalidBoneID)
      {
	rootBoneID = boneIt;
	continue;
      }

      // find or create an entry in the node hash for the parent of this node
      csRef<BodyChainNode> parentNode;
      if (nodeHash.Contains (parentBone))
	parentNode = *nodeHash[parentBone];
      else
      {
	parentNode.AttachNew (new BodyChainNode (parentBone));
	nodeHash.Put (parentBone, parentNode);
      }
      parentNode->AddChild (node);
    }

    return true;
  }

  void BodyChain::DebugPrint () const
  {
    printf ("Bone chain %s:\n", name.GetData ());
    Print (rootNode);
  }

  void BodyChain::Print (BodyChainNode* node, size_t level) const
  {
    for (size_t i = 0; i < level; i++)
      printf (" ");
    printf ("+ node %u: %s\n", node->boneID, bodySkeleton->skeletonFactory->GetBoneName (node->boneID));

    for (csRefArray<BodyChainNode>::Iterator it = node->children.GetIterator (); it.HasNext (); )
    {
      BodyChainNode*& node = it.Next ();
      Print (node, level + 1);
    }
  }

  void BodyChain::PopulateBoneMask (csBitArray& boneMask) const
  {
    boneMask.SetSize (bodySkeleton->skeletonFactory->GetTopBoneID () + 1);
    PopulateMask (rootNode, boneMask);
  }

  void BodyChain::PopulateMask (BodyChainNode* node, csBitArray& boneMask) const
  {
    boneMask.SetBit (node->GetAnimeshBone ());
    for (size_t i = 0; i < node->GetChildCount (); i++)
      PopulateMask (node->children[i], boneMask);
  }

  /********************
   *  BodyChainNode
   ********************/

  CS_LEAKGUARD_IMPLEMENT(BodyChainNode);

  BodyChainNode::BodyChainNode (CS::Animation::BoneID boneID)
    : scfImplementationType (this), boneID (boneID), parent (0)
  {
  }

  CS::Animation::BoneID BodyChainNode::GetAnimeshBone () const
  {
    return boneID;
  }

  size_t BodyChainNode::GetChildCount () const
  {
    return (size_t)children.GetSize ();
  }

  CS::Animation::iBodyChainNode* BodyChainNode::GetChild (size_t index) const
  {
    return children.Get (index);
  }

  CS::Animation::iBodyChainNode* BodyChainNode::GetParent () const
  {
    return parent;
  }

  void BodyChainNode::AddChild (BodyChainNode* node)
  {
    node->parent = this;
    children.Push (node);
  }

  CS::Animation::iBodyChainNode* BodyChainNode::FindSubChild (CS::Animation::BoneID child) const
  {
    if (child == boneID)
      return (CS::Animation::iBodyChainNode*) this;

    for (csRefArray<BodyChainNode>::ConstIterator it = children.GetIterator (); it.HasNext (); )
    {
      csRef<BodyChainNode> node = it.Next ();
      iBodyChainNode* result = node->FindSubChild (child);
      if (result)
	return result;
    }

    return nullptr;
  }

  void BodyChainNode::DebugPrint () const
  {
    Print ();
  }

  void BodyChainNode::Print (size_t level) const
  {
    for (size_t i = 0; i < level; i++)
      printf (" ");
    printf ("+ node %u\n", boneID);

    for (csRefArray<BodyChainNode>::ConstIterator it = children.GetIterator (); it.HasNext (); )
      it.Next ()->Print (level + 1);
  }

  /********************
   *  BodyBoneProperties
   ********************/

  CS_LEAKGUARD_IMPLEMENT(BodyBoneProperties);

  BodyBoneProperties::BodyBoneProperties ()
    : scfImplementationType (this), mass (1.0f), center (0.0f)
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
    : scfImplementationType (this), bounce (0.0f),
    maxAngle (-PI, -PI/2.0f, -PI/2.0f), maxDistance (-1.0f),
    minAngle (PI, PI/2.0f, PI/2.0f), minDistance (1.0f)
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
 
  void BodyBoneJoint::SetTransform (const csOrthoTransform &transform)
  {
    this->transform = transform;
  }

  csVector3 BodyBoneJoint::GetBounce () const
  {
    return bounce;
  }

  csVector3 BodyBoneJoint::GetMaximumAngle () const
  {
    return maxAngle;
  }

  csVector3 BodyBoneJoint::GetMaximumDistance () const
  {
    return maxDistance;
  }

  csVector3 BodyBoneJoint::GetMinimumAngle () const
  {
    return minAngle;
  }

  csVector3 BodyBoneJoint::GetMinimumDistance () const
  {
    return minDistance;
  }

  bool BodyBoneJoint::IsXRotConstrained () const
  {
    return rotConstraints[0];
  }

  bool BodyBoneJoint::IsXTransConstrained () const
  {
    return transConstraints[0];
  }

  bool BodyBoneJoint::IsYRotConstrained () const
  {
    return rotConstraints[1];
  }

  bool BodyBoneJoint::IsYTransConstrained () const
  {
    return transConstraints[1];
  }

  bool BodyBoneJoint::IsZRotConstrained () const
  {
    return rotConstraints[2];
  }

  bool BodyBoneJoint::IsZTransConstrained () const
  {
    return transConstraints[2];
  }

  csOrthoTransform BodyBoneJoint::GetTransform () const
  {
    return transform;
  }

  /********************
   *  BodyBoneCollider
   ********************/

  CS_LEAKGUARD_IMPLEMENT(BodyBoneCollider);

  BodyBoneCollider::BodyBoneCollider ()
    : scfImplementationType (this), friction (0.0f), softness (0.0f),
    elasticity (0.0f), density (0.0f), geometryType (NO_GEOMETRY),
    box_size (0.0f), length (0.0f), radius (0.0f)
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
 
  csColliderGeometryType BodyBoneCollider::GetGeometryType () const
  {
    return geometryType;
  }

  bool BodyBoneCollider::GetBoxGeometry (csVector3 &box_size) const
  {
    if (geometryType != BOX_COLLIDER_GEOMETRY)
      return false;

    box_size = this->box_size;
    return true;
  }

  bool BodyBoneCollider::GetCapsuleGeometry (float &length, float &radius) const
  {
    if (geometryType != CAPSULE_COLLIDER_GEOMETRY)
      return false;

    length = this->length;
    radius = this->radius;
    return true;
  }

  bool BodyBoneCollider::GetConvexMeshGeometry (iMeshWrapper *&mesh) const
  {
    if (geometryType != CONVEXMESH_COLLIDER_GEOMETRY)
      return false;

    mesh = this->mesh;
    return true;
  }

  bool BodyBoneCollider::GetCylinderGeometry (float &length, float &radius) const
  {
    if (geometryType != CYLINDER_COLLIDER_GEOMETRY)
      return false;

    length = this->length;
    radius = this->radius;
    return true;
  }

  bool BodyBoneCollider::GetMeshGeometry (iMeshWrapper *&mesh) const
  {
    if (geometryType != TRIMESH_COLLIDER_GEOMETRY)
      return false;

    mesh = this->mesh;
    return true;
  }

  bool BodyBoneCollider::GetPlaneGeometry (csPlane3 &plane) const
  {
    if (geometryType != PLANE_COLLIDER_GEOMETRY)
      return false;

    plane = this->plane;
    return true;
  }

  bool BodyBoneCollider::GetSphereGeometry (csSphere &sphere) const
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

  csOrthoTransform BodyBoneCollider::GetTransform () const
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
