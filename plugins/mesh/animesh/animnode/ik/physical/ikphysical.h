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
#ifndef __CS_IKPHYSICAL_H__
#define __CS_IKPHYSICAL_H__

#include "csutil/scf_implementation.h"
#include "csgeom/transfrm.h"
#include "cstool/animnodetmpl.h"
#include "csutil/leakguard.h"
#include "csutil/weakref.h"
#include "csutil/csstring.h"
#include "imesh/animnode/ik.h"
#include "ivaria/bullet.h"
#include "imesh/animnode/ragdoll.h"
#include "iutil/comp.h"

CS_PLUGIN_NAMESPACE_BEGIN(IKPhysical)
{

  class IKPhysicalNodeManager;

  struct EffectorData
  {
    CS::Animation::iBodyChain* chain;
    CS::Animation::BoneID bone;
    csOrthoTransform transform;
  };

  class IKPhysicalNodeFactory
    : public scfImplementation3<IKPhysicalNodeFactory,
    scfFakeInterface<CS::Animation::iSkeletonAnimNodeFactory>,
    scfFakeInterface<CS::Animation::iSkeletonIKNodeFactory>,
    CS::Animation::iSkeletonIKPhysicalNodeFactory>,
    public CS::Animation::SkeletonAnimNodeFactorySingle
  {
  public:
    CS_LEAKGUARD_DECLARE(IKPhysicalNodeFactory);

    IKPhysicalNodeFactory (IKPhysicalNodeManager* manager, const char *name);

    //-- CS::Animation::iSkeletonIKNodeFactory
    virtual void SetBodySkeleton (CS::Animation::iBodySkeleton* skeleton);
    virtual CS::Animation::iBodySkeleton* GetBodySkeleton () const;
    virtual CS::Animation::EffectorID AddEffector (CS::Animation::iBodyChain* chain,
						   CS::Animation::BoneID bone,
						   csOrthoTransform& transform);
    virtual void RemoveEffector (CS::Animation::EffectorID effector);

    inline virtual void SetChildNode (CS::Animation::iSkeletonAnimNodeFactory* factory)
    { CS::Animation::SkeletonAnimNodeFactorySingle::SetChildNode (factory); }
    inline virtual iSkeletonAnimNodeFactory* GetChildNode () const
    { return CS::Animation::SkeletonAnimNodeFactorySingle::GetChildNode (); }

    //-- CS::Animation::iSkeletonIKPhysicalNodeFactory
    virtual void SetChainAutoReset (bool reset);
    virtual bool GetChainAutoReset () const;

    //-- CS::Animation::iSkeletonAnimNodeFactory
    csPtr<CS::Animation::SkeletonAnimNodeSingleBase> ActualCreateInstance (
      CS::Animation::iSkeletonAnimPacket* packet, CS::Animation::iSkeleton* skeleton);
  protected:
    IKPhysicalNodeManager* manager;
    csRef<CS::Animation::iBodySkeleton> bodySkeleton;
    csHash<EffectorData, CS::Animation::EffectorID> effectors;
    CS::Animation::EffectorID maxEffectorID;
    bool resetChain;

    friend class IKPhysicalNode;
  };

  class IKPhysicalNode
    : public scfImplementation3<IKPhysicalNode,
				scfFakeInterface<CS::Animation::iSkeletonAnimNode>,
				scfFakeInterface<CS::Animation::iSkeletonIKNode>,
				CS::Animation::iSkeletonIKPhysicalNode>,
      CS::Animation::SkeletonAnimNodeSingle<IKPhysicalNodeFactory>
  {
  public:
    CS_LEAKGUARD_DECLARE(IKPhysicalNode);

    IKPhysicalNode (IKPhysicalNodeFactory* factory,
		    CS::Animation::iSkeleton* skeleton);
    ~IKPhysicalNode ();

    //-- CS::Animation::iSkeletonIKPhysicalNode
    virtual void SetRagdollNode (CS::Animation::iSkeletonRagdollNode* ragdollNode);
    virtual CS::Animation::iSkeletonRagdollNode* GetRagdollNode () const;

    //-- CS::Animation::iSkeletonIKNode
    virtual void AddConstraint (CS::Animation::EffectorID effector,
				csOrthoTransform& transform);
    virtual void AddConstraint (CS::Animation::EffectorID effector,
				iMovable* target, const csOrthoTransform& offset);
    virtual void AddConstraint (CS::Animation::EffectorID effector,
				iCamera* target, const csOrthoTransform& offset);

    virtual void RemoveConstraint (CS::Animation::EffectorID effector);

    //-- CS::Animation::iSkeletonAnimNode
    virtual void Play ();
    virtual void Stop ();

    virtual void BlendState (CS::Animation::AnimatedMeshState* state,
			     float baseWeight = 1.0f);
    virtual void TickAnimation (float dt);
  private:
    enum ConstraintType
    {
      CONSTRAINT_FIXED = 0,
      CONSTRAINT_MOVABLE,
      CONSTRAINT_CAMERA
    };

    struct ConstraintData
    {
      ConstraintData ()
      : movable (0), camera (0) {}

      ConstraintType type;
      iMovable* movable;
      iCamera* camera;
      csOrthoTransform offset;
      csRef<CS::Physics::Bullet::iPivotJoint> dragJoint;
    };
    void AddConstraint (CS::Animation::EffectorID effectorID, ConstraintData& constraint);

  private:
    csWeakRef<iSceneNode> sceneNode;
    csWeakRef<CS::Animation::iSkeletonRagdollNode> ragdollNode;

    csHash<ConstraintData, CS::Animation::EffectorID> constraints;

    struct ChainData
    {
      CS::Animation::RagdollState previousState;
      size_t constraintCount;
    };
    csHash<ChainData, CS::Animation::iBodyChain*> chains;

    csRef<CS::Physics::Bullet::iDynamicSystem> bulletDynamicSystem;

    friend class IKPhysicalNodeFactory;
  };


  class IKPhysicalNodeManager
    : public CS::Animation::AnimNodeManagerCommon<IKPhysicalNodeManager,
						  CS::Animation::iSkeletonIKNodeManager,
						  IKPhysicalNodeFactory>
  {
  public:
    IKPhysicalNodeManager (iBase* parent)
     : AnimNodeManagerCommonType (parent) {}
     
    void Report (int severity, const char* msg, ...) const;
  };
}
CS_PLUGIN_NAMESPACE_END(IKPhysical)

#endif //__CS_IKPHYSICAL_H__
