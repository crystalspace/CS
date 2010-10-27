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
#ifndef __CS_IKCCD_H__
#define __CS_IKCCD_H__

#include "csutil/scf_implementation.h"
#include "iutil/comp.h"
#include "csgeom/transfrm.h"
#include "csutil/leakguard.h"
#include "csutil/weakref.h"
#include "csutil/csstring.h"
#include "imesh/animnode/ik.h"
#include "imesh/bodymesh.h"

#include "iutil/visualdebug.h"

CS_PLUGIN_NAMESPACE_BEGIN(IKCCD)
{

  class IKCCDNodeManager : public scfImplementation2<IKCCDNodeManager,
    CS::Animation::iSkeletonIKNodeManager, iComponent>
  {
  public:
    CS_LEAKGUARD_DECLARE(IKCCDNodeManager);

    IKCCDNodeManager (iBase* parent);

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

    friend class IKCCDNodeFactory;
    friend class IKCCDNode;
  };

  struct EffectorData
  {
    CS::Animation::iBodyChain* chain;
    CS::Animation::BoneID bone;
    csOrthoTransform transform;
  };

  class IKCCDNodeFactory : public scfImplementation3
    <IKCCDNodeFactory,
    scfFakeInterface<CS::Animation::iSkeletonAnimNodeFactory>,
    scfFakeInterface<CS::Animation::iSkeletonIKNodeFactory>,
    CS::Animation::iSkeletonIKCCDNodeFactory>
  {
    friend class IKCCDNode;

  public:
    CS_LEAKGUARD_DECLARE(IKCCDNodeFactory);

    IKCCDNodeFactory (IKCCDNodeManager* manager, const char *name,
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
    virtual void SetChildNode (CS::Animation::iSkeletonAnimNodeFactory* node);
    virtual CS::Animation::iSkeletonAnimNodeFactory* GetChildNode () const;
    virtual void ClearChildNode ();

    //-- CS::Animation::iSkeletonIKCCDNodeFactory
    virtual void SetMaximumIterations (size_t max);
    virtual size_t GetMaximumIterations ();
    virtual void SetTargetDistance (float distance);
    virtual float GetTargetDistance ();
    virtual void SetMotionRatio (float ratio);
    virtual float GetMotionRatio ();
    virtual void SetJointInitialization (bool initialized);
    virtual bool GetJointInitialization ();
    virtual void SetUpwardIterations (bool upward);
    virtual bool GetUpwardIterations ();

  protected:
    IKCCDNodeManager* manager;
    csString name;
    csRef<CS::Animation::iBodySkeleton> bodySkeleton;
    csRef<CS::Animation::iSkeletonAnimNodeFactory> childNode;
    csHash<EffectorData, CS::Animation::EffectorID> effectors;
    CS::Animation::EffectorID maxEffectorID;
    size_t maximumIterations;
    float targetDistance;
    float motionRatio;
    bool jointInitialized;
    bool upwardIterations;
  };

  class IKCCDNode : public scfImplementation3
    <IKCCDNode,
    scfFakeInterface<CS::Animation::iSkeletonAnimNode>,
    scfFakeInterface<CS::Animation::iSkeletonIKNode>,
    CS::Animation::iSkeletonIKCCDNode>
  {
  public:
    CS_LEAKGUARD_DECLARE(IKCCDNode);

    IKCCDNode (IKCCDNodeFactory* factory,
		   CS::Animation::iSkeleton* skeleton,
		   CS::Animation::iSkeletonAnimNode* childNode);
    ~IKCCDNode ();

    //-- CS::Animation::iSkeletonIKCCDNode

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
    };

  private:
    IKCCDNodeFactory* factory;
    csWeakRef<iSceneNode> sceneNode;
    csWeakRef<CS::Animation::iSkeleton> skeleton;
    csRef<CS::Animation::iSkeletonAnimNode> childNode;
    bool isActive;

    csHash<ConstraintData, CS::Animation::EffectorID> constraints;

    struct BoneData
    {
      CS::Animation::iBodyBone* bodyBone;
      csQuaternion rotation;
      csVector3 boneOffset;
      csQuaternion boneRotation;
    };
  };

}
CS_PLUGIN_NAMESPACE_END(IKCCD)

#endif //__CS_IKCCD_H__
