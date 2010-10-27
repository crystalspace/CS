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
#ifndef __CS_IKPHYSICAL_H__
#define __CS_IKPHYSICAL_H__

#include "csutil/scf_implementation.h"
#include "iutil/comp.h"
#include "csgeom/transfrm.h"
#include "csutil/leakguard.h"
#include "csutil/weakref.h"
#include "csutil/csstring.h"
#include "imesh/animnode/ik.h"
#include "ivaria/bullet.h"
#include "imesh/animnode/ragdoll.h"

CS_PLUGIN_NAMESPACE_BEGIN(IKPhysical)
{

  class IKPhysicalManager : public scfImplementation2<IKPhysicalManager,
    CS::Animation::iSkeletonIKNodeManager, iComponent>
  {
  public:
    CS_LEAKGUARD_DECLARE(IKPhysicalManager);

    IKPhysicalManager (iBase* parent);

    //-- CS::Animation::iSkeletonIKNodeManager
    virtual CS::Animation::iSkeletonIKNodeFactory* CreateAnimNodeFactory
      (const char *name, CS::Animation::iBodySkeleton* skeleton);

    virtual CS::Animation::iSkeletonIKNodeFactory* FindAnimNodeFactory (const char* name) const;
    virtual void ClearAnimNodeFactories ();

    //-- iComponent
    virtual bool Initialize (iObjectRegistry*);

    // Error reporting
    void Report (int severity, const char* msg, ...) const;

  private:
    iObjectRegistry* object_reg;
    csHash<csRef<CS::Animation::iSkeletonIKNodeFactory>, csString> factoryHash;

    friend class IKPhysicalAnimNodeFactory;
    friend class IKPhysicalAnimNode;
  };

  struct EffectorData
  {
    CS::Animation::iBodyChain* chain;
    CS::Animation::BoneID bone;
    csOrthoTransform transform;
  };

  class IKPhysicalAnimNodeFactory : public scfImplementation3
    <IKPhysicalAnimNodeFactory,
    scfFakeInterface<CS::Animation::iSkeletonAnimNodeFactory>,
    scfFakeInterface<CS::Animation::iSkeletonIKNodeFactory>,
    CS::Animation::iSkeletonIKPhysicalNodeFactory>
  {
    friend class IKPhysicalAnimNode;

  public:
    CS_LEAKGUARD_DECLARE(IKPhysicalAnimNodeFactory);

    IKPhysicalAnimNodeFactory (IKPhysicalManager* manager, const char *name,
			       CS::Animation::iBodySkeleton* skeleton);

    //-- CS::Animation::iSkeletonAnimNodeFactory
    virtual csPtr<CS::Animation::iSkeletonAnimNode> CreateInstance
      (CS::Animation::iSkeletonAnimPacket* packet, CS::Animation::iSkeleton* skeleton);

    virtual const char* GetNodeName () const;

    virtual CS::Animation::iSkeletonAnimNodeFactory* FindNode (const char* name);

    //-- CS::Animation::iSkeletonIKNodeFactory
    virtual CS::Animation::EffectorID AddEffector (CS::Animation::iBodyChain* chain,
						   CS::Animation::BoneID bone,
						   csOrthoTransform& transform);
    virtual void RemoveEffector (CS::Animation::EffectorID effector);

    //-- CS::Animation::iSkeletonIKPhysicalNodeFactory
    virtual void SetChildNode (CS::Animation::iSkeletonAnimNodeFactory* node);
    virtual CS::Animation::iSkeletonAnimNodeFactory* GetChildNode () const;
    virtual void ClearChildNode ();

    virtual void SetChainAutoReset (bool reset);
    virtual bool GetChainAutoReset () const;

  protected:
    IKPhysicalManager* manager;
    csString name;
    csRef<CS::Animation::iBodySkeleton> bodySkeleton;
    csRef<CS::Animation::iSkeletonAnimNodeFactory> childNode;
    csHash<EffectorData, CS::Animation::EffectorID> effectors;
    CS::Animation::EffectorID maxEffectorID;
    bool resetChain;
  };

  class IKPhysicalAnimNode : public scfImplementation3
    <IKPhysicalAnimNode,
    scfFakeInterface<CS::Animation::iSkeletonAnimNode>,
    scfFakeInterface<CS::Animation::iSkeletonIKNode>,
    CS::Animation::iSkeletonIKPhysicalNode>
  {
  public:
    CS_LEAKGUARD_DECLARE(IKPhysicalAnimNode);

    IKPhysicalAnimNode (IKPhysicalAnimNodeFactory* factory,
			CS::Animation::iSkeleton* skeleton,
			CS::Animation::iSkeletonAnimNode* childNode);
    ~IKPhysicalAnimNode ();

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

    //-- CS::Animation::iSkeletonAnimPacket
    virtual void Play ();

    virtual void Stop ();

    virtual void SetPlaybackPosition (float time);

    virtual float GetPlaybackPosition () const;

    virtual float GetDuration () const;

    virtual void SetPlaybackSpeed (float speed);

    virtual float GetPlaybackSpeed () const;

    virtual void BlendState (CS::Animation::csSkeletalState* state,
			     float baseWeight = 1.0f);

    virtual void TickAnimation (float dt);

    virtual bool IsActive () const;

    virtual CS::Animation::iSkeletonAnimNodeFactory* GetFactory () const;

    virtual CS::Animation::iSkeletonAnimNode* FindNode (const char* name);

    virtual void AddAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback);

    virtual void RemoveAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback);

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
    IKPhysicalAnimNodeFactory* factory;
    csWeakRef<iSceneNode> sceneNode;
    csWeakRef<CS::Animation::iSkeleton> skeleton;
    csRef<CS::Animation::iSkeletonAnimNode> childNode;
    csWeakRef<CS::Animation::iSkeletonRagdollNode> ragdollNode;
    bool isActive;

    csHash<ConstraintData, CS::Animation::EffectorID> constraints;

    struct ChainData
    {
      CS::Animation::RagdollState previousState;
      size_t constraintCount;
    };
    csHash<ChainData, CS::Animation::iBodyChain*> chains;

    csRef<CS::Physics::Bullet::iDynamicSystem> bulletDynamicSystem;
  };

}
CS_PLUGIN_NAMESPACE_END(IKPhysical)

#endif //__CS_IKPHYSICAL_H__
