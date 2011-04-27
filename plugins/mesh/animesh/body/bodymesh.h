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
#ifndef __CS_BODYMESH_H__
#define __CS_BODYMESH_H__

#include "csutil/scf_implementation.h"
#include "iutil/comp.h"
#include "csutil/weakref.h"
#include "csutil/leakguard.h"
#include "csutil/hash.h"
#include "csutil/csstring.h"
#include "csutil/refarr.h"
#include "imesh/animesh.h"
#include "imesh/bodymesh.h"
#include "ivaria/dynamics.h"
#include <csgeom/matrix3.h>
#include <csgeom/plane3.h>
#include <csgeom/sphere.h>

CS_PLUGIN_NAMESPACE_BEGIN(Bodymesh)
{
  class BodyBone;
  class BodyChainNode;

  class BodyBoneProperties : public scfImplementation1<BodyBoneProperties,
    CS::Animation::iBodyBoneProperties>
  {
  public:
    CS_LEAKGUARD_DECLARE(BodyBoneProperties);

    BodyBoneProperties ();

    virtual void SetMass (float mass);
    virtual float GetMass () const;

    virtual void SetCenter (const csVector3 &center);
    virtual csVector3 GetCenter () const;

    virtual void SetInertia (const csMatrix3 &inertia);
    virtual csMatrix3 GetInertia () const;

  private:
    float mass;
    csVector3 center;
    csMatrix3 inertia;
  };

  class BodyBoneJoint : public scfImplementation1<BodyBoneJoint,
    CS::Animation::iBodyBoneJoint>
  {
  public:
    CS_LEAKGUARD_DECLARE(BodyBoneJoint);

    BodyBoneJoint ();

    virtual void SetBounce (const csVector3 &bounce);
    virtual void SetMaximumAngle (const csVector3 &max);
    virtual void SetMaximumDistance (const csVector3 &max);
    virtual void SetMinimumAngle (const csVector3 &min);
    virtual void SetMinimumDistance (const csVector3 &min);
    virtual void SetRotConstraints (bool X, bool Y, bool Z);
    virtual void SetTransConstraints (bool X, bool Y, bool Z);
    virtual void SetTransform (const csOrthoTransform &transform);
 
    virtual csVector3 GetBounce () const;
    virtual csVector3 GetMaximumAngle () const;
    virtual csVector3 GetMaximumDistance () const;
    virtual csVector3 GetMinimumAngle () const;
    virtual csVector3 GetMinimumDistance () const;
    virtual bool IsXRotConstrained () const;
    virtual bool IsXTransConstrained () const;
    virtual bool IsYRotConstrained () const;
    virtual bool IsYTransConstrained () const;
    virtual bool IsZRotConstrained () const;
    virtual bool IsZTransConstrained () const;
    virtual csOrthoTransform GetTransform () const;

  private:
    csVector3 bounce;
    csVector3 maxAngle;
    csVector3 maxDistance;
    csVector3 minAngle;
    csVector3 minDistance;
    bool rotConstraints[3];
    bool transConstraints[3];
    csOrthoTransform transform;
  };

  class BodyBoneCollider : public scfImplementation1<BodyBoneCollider,
    CS::Animation::iBodyBoneCollider>
  {
  public:
    CS_LEAKGUARD_DECLARE(BodyBoneCollider);

    BodyBoneCollider ();

    virtual bool SetBoxGeometry (const csVector3 &box_size);
    virtual bool SetCapsuleGeometry (float length, float radius);
    virtual bool SetConvexMeshGeometry (iMeshWrapper *mesh);
    virtual bool SetCylinderGeometry (float length, float radius);
    virtual bool SetMeshGeometry (iMeshWrapper *mesh);
    virtual bool SetPlaneGeometry (const csPlane3 &plane);
    virtual bool SetSphereGeometry (const csSphere &sphere);
 
    virtual csColliderGeometryType GetGeometryType () const;

    virtual bool GetBoxGeometry (csVector3 &box_size) const;
    virtual bool GetCapsuleGeometry (float &length, float &radius) const;
    virtual bool GetConvexMeshGeometry (iMeshWrapper *&mesh) const;
    virtual bool GetCylinderGeometry (float &length, float &radius) const;
    virtual bool GetMeshGeometry (iMeshWrapper *&mesh) const;
    virtual bool GetPlaneGeometry (csPlane3 &plane) const;
    virtual bool GetSphereGeometry (csSphere &sphere) const;

    virtual void SetTransform (const csOrthoTransform &transform);
    virtual csOrthoTransform GetTransform () const;

    virtual void SetFriction (float friction);
    virtual float GetFriction () const;

    virtual void SetSoftness (float softness);
    virtual float GetSoftness () const;

    virtual void SetElasticity (float elasticity);
    virtual float GetElasticity () const;

    virtual void SetDensity (float density);
    virtual float GetDensity () const;

  private:
    csOrthoTransform transform;
    float friction;
    float softness;
    float elasticity;
    float density;
    csColliderGeometryType geometryType;
    csVector3 box_size;
    float length;
    float radius;
    csRef<iMeshWrapper> mesh;
    csPlane3 plane;
    csSphere sphere;
  };

  class BodyManager : public scfImplementation2<BodyManager, 
                                           CS::Animation::iBodyManager, iComponent>
  {
  public:
    CS_LEAKGUARD_DECLARE(BodyManager);

    BodyManager (iBase* parent);

    //-- CS::Animation::iBodyManager
    virtual CS::Animation::iBodySkeleton* CreateBodySkeleton (const char *name,
				CS::Animation::iSkeletonFactory* skeletonFactory);
    virtual CS::Animation::iBodySkeleton* FindBodySkeleton (const char *name);
    virtual void ClearBodySkeletons ();

    //-- iComponent
    virtual bool Initialize (iObjectRegistry* object_reg);

    void Report (int severity, const char* msg, ...);
  
  private:
    iObjectRegistry* object_reg;
    csHash<csRef<CS::Animation::iBodySkeleton>, csString> factoryHash;
  };

  class BodySkeleton : public scfImplementation1<BodySkeleton,
                                                        CS::Animation::iBodySkeleton>
  {
  public:
    CS_LEAKGUARD_DECLARE(BodySkeleton);

    BodySkeleton (const char* name, BodyManager* manager,
		  CS::Animation::iSkeletonFactory* skeletonFactory);

    virtual const char* GetName () const;

    virtual CS::Animation::iSkeletonFactory* GetSkeletonFactory () const;

    virtual void ClearAll ();

    virtual CS::Animation::iBodyBone* CreateBodyBone (CS::Animation::BoneID boneID);
    virtual CS::Animation::iBodyBone* FindBodyBone (const char *name) const;
    virtual CS::Animation::iBodyBone* FindBodyBone (CS::Animation::BoneID bone) const;
    virtual void ClearBodyBones ();

    virtual CS::Animation::iBodyChain* CreateBodyChain (const char *name,
							CS::Animation::BoneID rootBone);
    virtual CS::Animation::iBodyChain* FindBodyChain (const char *name) const;
    virtual void ClearBodyChains ();

  private:
    csString name;
    BodyManager* manager;
    // This is a weakref to avoid circular references between animesh,
    // body skeleton and animation nodes
    csWeakRef<CS::Animation::iSkeletonFactory> skeletonFactory;
    csHash<csRef<BodyBone>, CS::Animation::BoneID> boneHash;
    csHash<csRef<CS::Animation::iBodyChain>, csString> chainHash;

    friend class BodyChain;
  };

  class BodyBone : public scfImplementation1<BodyBone, CS::Animation::iBodyBone>
  {
  public:
    CS_LEAKGUARD_DECLARE(BodyBone);

    BodyBone (CS::Animation::BoneID boneID);

    virtual CS::Animation::BoneID GetAnimeshBone () const;

    virtual CS::Animation::iBodyBoneProperties* CreateBoneProperties ();
    virtual CS::Animation::iBodyBoneProperties* GetBoneProperties () const;

    virtual CS::Animation::iBodyBoneJoint* CreateBoneJoint ();
    virtual CS::Animation::iBodyBoneJoint* GetBoneJoint () const;

    virtual CS::Animation::iBodyBoneCollider* CreateBoneCollider ();
    virtual size_t GetBoneColliderCount () const;
    virtual CS::Animation::iBodyBoneCollider* GetBoneCollider (size_t index) const;

  private:
    CS::Animation::BoneID animeshBone;
    csRef<BodyBoneProperties> properties;
    csRef<BodyBoneJoint> joint;
    csRefArray<BodyBoneCollider> colliders;

    friend class BodyChainNode;
  };

  class BodyChain : public scfImplementation1<BodyChain, CS::Animation::iBodyChain>
  {
  public:
    CS_LEAKGUARD_DECLARE(BodyChain);

    BodyChain (BodySkeleton* bodySkeleton, const char *name,
	       CS::Animation::BoneID rootBone);

    virtual const char* GetName () const;
    virtual CS::Animation::iBodySkeleton* GetBodySkeleton () const;
    virtual CS::Animation::iBodyChainNode* GetRootNode () const;

    virtual bool AddSubChain (CS::Animation::BoneID subBone);
    virtual bool AddAllSubChains ();

    virtual void DebugPrint () const;

    virtual void PopulateBoneMask (csBitArray& boneMask) const;

  private:
    void Print (BodyChainNode* node, size_t level = 0) const;
    void PopulateMask (BodyChainNode* node, csBitArray& boneMask) const;

    csString name;
    csRef<BodySkeleton> bodySkeleton;
    csRef<BodyChainNode> rootNode;
  };

  class BodyChainNode : public scfImplementation1<BodyChainNode,
    CS::Animation::iBodyChainNode>
  {
  public:
    CS_LEAKGUARD_DECLARE(BodyChainNode);

    BodyChainNode (CS::Animation::BoneID boneID);

    virtual CS::Animation::BoneID GetAnimeshBone () const;

    virtual size_t GetChildCount () const;
    virtual CS::Animation::iBodyChainNode* GetChild (size_t index) const;
    virtual CS::Animation::iBodyChainNode* GetParent () const;
    virtual iBodyChainNode* FindSubChild (CS::Animation::BoneID child) const;

    void AddChild (BodyChainNode* node);

    virtual void DebugPrint () const;

  private:
    void Print (size_t level = 0) const;

    CS::Animation::BoneID boneID;
    csWeakRef<BodyChainNode> parent;
    csRefArray<BodyChainNode> children;

    friend class BodyChain;
  };

}
CS_PLUGIN_NAMESPACE_END(Bodymesh)

#endif //__CS_BODYMESH_H__
