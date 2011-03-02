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
#ifndef __CS_LOOKAT_H__
#define __CS_LOOKAT_H__

#include "csutil/scf_implementation.h"
#include "cstool/animnodetmpl.h"
#include "csutil/leakguard.h"
#include "csutil/weakref.h"
#include "csutil/refarr.h"
#include "csutil/csstring.h"
#include "imesh/animnode/lookat.h"
#include "iutil/comp.h"

namespace CS {
namespace Animation {

struct iBodyBoneJoint;

}
}

CS_PLUGIN_NAMESPACE_BEGIN(LookAt)
{
  class LookAtNodeManager;
  
  class LookAtNodeFactory :
  public scfImplementation2<LookAtNodeFactory,
    scfFakeInterface<CS::Animation::iSkeletonAnimNodeFactory>,
    CS::Animation::iSkeletonLookAtNodeFactory>,
    CS::Animation::csSkeletonAnimNodeFactorySingle
  {
  public:
    CS_LEAKGUARD_DECLARE(LookAtNodeFactory);

    LookAtNodeFactory (LookAtNodeManager* manager, const char *name);

    //-- CS::Animation::iSkeletonLookAtNodeFactory
    virtual void SetBodySkeleton (CS::Animation::iBodySkeleton* skeleton);
    virtual void SetBone (CS::Animation::BoneID boneID);
    virtual void SetMaximumSpeed (float speed);
    virtual void SetAlwaysRotate (bool alwaysRotate);
    virtual void SetListenerDelay (float delay);

    inline virtual void SetChildNode (CS::Animation::iSkeletonAnimNodeFactory* factory)
    { CS::Animation::csSkeletonAnimNodeFactorySingle::SetChildNode (factory); }
    inline virtual iSkeletonAnimNodeFactory* GetChildNode () const
    { return CS::Animation::csSkeletonAnimNodeFactorySingle::GetChildNode (); }

    //-- CS::Animation::iSkeletonAnimNodeFactory
    virtual csPtr<CS::Animation::iSkeletonAnimNode> CreateInstance (
	       CS::Animation::iSkeletonAnimPacket* packet, CS::Animation::iSkeleton* skeleton);
    inline virtual const char* GetNodeName () const
      { return csSkeletonAnimNodeFactorySingle::GetNodeName (); }
    virtual CS::Animation::iSkeletonAnimNodeFactory* FindNode (const char* name);

  protected:
    LookAtNodeManager* manager;
    csRef<CS::Animation::iBodySkeleton> skeleton;
    CS::Animation::BoneID boneID;
    csRef<CS::Animation::iBodyBoneJoint> bodyJoint;
    float maximumSpeed;
    bool alwaysRotate;
    float listenerMinimumDelay;

    friend class LookAtNode;
  };

  class LookAtNode
    : public scfImplementation2<LookAtNode,
				scfFakeInterface<CS::Animation::iSkeletonAnimNode>,
				CS::Animation::iSkeletonLookAtNode>,
      CS::Animation::SkeletonAnimNodeSingle
  {
  public:
    CS_LEAKGUARD_DECLARE(LookAtNode);

    LookAtNode (LookAtNodeFactory* factory, CS::Animation::iSkeleton* skeleton);

    //-- CS::Animation::iSkeletonLookAtNode
    virtual bool HasTarget ();
    virtual void SetTarget (csVector3 target);
    virtual void SetTarget (iMovable* target, const csVector3& offset);
    virtual void SetTarget (iCamera* target, const csVector3& offset);
    virtual void RemoveTarget ();
  
    virtual void AddListener (CS::Animation::iSkeletonLookAtListener* listener);
    virtual void RemoveListener (CS::Animation::iSkeletonLookAtListener* listener);

    //-- CS::Animation::iSkeletonAnimNode
    virtual void Play ();

    virtual void BlendState (CS::Animation::csSkeletalState* state,
				    float baseWeight = 1.0f);
    virtual void TickAnimation (float dt);

    virtual CS::Animation::iSkeletonAnimNodeFactory* GetFactory () const;
    virtual CS::Animation::iSkeletonAnimNode* FindNode (const char* name);

  private:
    LookAtNodeFactory* factory;
    char targetMode;
    char trackingStatus;
    csVector3 targetPosition;
    csRef<iMovable> targetMovable;
    csRef<iCamera> targetCamera;
    csVector3 targetOffset;
    float frameDuration;
    float previousPitch, previousYaw, previousRoll;
    bool trackingInitialized;
    char listenerStatus;
    float listenerDelay;
    csRefArray<CS::Animation::iSkeletonLookAtListener> listeners;

    friend class LookAtNodeFactory;
  };

  class LookAtNodeManager
    : public CS::Animation::AnimNodeManagerCommon<LookAtNodeManager,
						  CS::Animation::iSkeletonLookAtNodeManager,
						  LookAtNodeFactory>
  {
  public:
    LookAtNodeManager (iBase* parent)
     : AnimNodeManagerCommonType (parent) {}
  };

}
CS_PLUGIN_NAMESPACE_END(LookAt)

#endif //__CS_LOOKAT_H__
