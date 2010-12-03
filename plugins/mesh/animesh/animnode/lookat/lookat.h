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
#include "iutil/comp.h"
#include "csutil/leakguard.h"
#include "csutil/weakref.h"
#include "csutil/refarr.h"
#include "csutil/csstring.h"
#include "imesh/animnode/lookat.h"

namespace CS {
namespace Animation {

struct iBodyBoneJoint;

}
}

CS_PLUGIN_NAMESPACE_BEGIN(LookAt)
{

  class LookAtNodeManager : public scfImplementation2<LookAtNodeManager,
    CS::Animation::iSkeletonLookAtNodeManager, iComponent>
  {
  public:
    CS_LEAKGUARD_DECLARE(LookAtNodeManager);

    LookAtNodeManager (iBase* parent);

    //-- CS::Animation::iSkeletonLookAtNodeManager
    virtual CS::Animation::iSkeletonLookAtNodeFactory* CreateAnimNodeFactory (const char *name,
							   CS::Animation::iBodySkeleton* skeleton);

    virtual CS::Animation::iSkeletonLookAtNodeFactory* FindAnimNodeFactory (const char* name) const;
    virtual void ClearAnimNodeFactories ();

    //-- iComponent
    virtual bool Initialize (iObjectRegistry*);

    // error reporting
    void Report (int severity, const char* msg, ...) const;

  private:
    iObjectRegistry* object_reg;
    csHash<csRef<CS::Animation::iSkeletonLookAtNodeFactory>, csString> factoryHash;
  };

  class LookAtAnimNodeFactory : public scfImplementation2<LookAtAnimNodeFactory, 
    scfFakeInterface<CS::Animation::iSkeletonAnimNodeFactory>, CS::Animation::iSkeletonLookAtNodeFactory>
  {
    friend class LookAtAnimNode;

  public:
    CS_LEAKGUARD_DECLARE(LookAtAnimNodeFactory);

    LookAtAnimNodeFactory (LookAtNodeManager* manager, const char *name,
			   CS::Animation::iBodySkeleton* skeleton);

    //-- CS::Animation::iSkeletonLookAtNodeFactory
    virtual void SetChildNode (CS::Animation::iSkeletonAnimNodeFactory* node);
    virtual CS::Animation::iSkeletonAnimNodeFactory* GetChildNode ();
    virtual void ClearChildNode ();

    //-- CS::Animation::iSkeletonAnimNodeFactory
    virtual csPtr<CS::Animation::iSkeletonAnimNode> CreateInstance (
	       CS::Animation::iSkeletonAnimPacket* packet, CS::Animation::iSkeleton* skeleton);

    virtual const char* GetNodeName () const;

    virtual CS::Animation::iSkeletonAnimNodeFactory* FindNode (const char* name);

  protected:
    LookAtNodeManager* manager;
    csString name;
    csRef<CS::Animation::iBodySkeleton> skeleton;
    csRef<CS::Animation::iSkeletonAnimNodeFactory> childNode;
  };

  class LookAtAnimNode : public scfImplementation2<LookAtAnimNode, 
    scfFakeInterface<CS::Animation::iSkeletonAnimNode>, CS::Animation::iSkeletonLookAtNode>
  {
  public:
    CS_LEAKGUARD_DECLARE(LookAtAnimNode);

    LookAtAnimNode (LookAtAnimNodeFactory* factory, CS::Animation::iSkeleton* skeleton,
		    CS::Animation::iSkeletonAnimNode* childNode);

    //-- CS::Animation::iSkeletonLookAtNode
    virtual void SetBone (CS::Animation::BoneID boneID);

    virtual void SetTarget (csVector3 target);
    virtual void SetTarget (iMovable* target, const csVector3& offset);
    virtual void SetTarget (iCamera* target, const csVector3& offset);
    virtual void RemoveTarget ();
  
    virtual void SetMaximumSpeed (float speed);
    virtual void SetAlwaysRotate (bool alwaysRotate);

    virtual void SetListenerDelay (float delay);
    virtual void AddListener (CS::Animation::iSkeletonLookAtListener* listener);
    virtual void RemoveListener (CS::Animation::iSkeletonLookAtListener* listener);

    //-- CS::Animation::iSkeletonAnimNode
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
    LookAtAnimNodeFactory* factory;
    csWeakRef<iSceneNode> sceneNode;
    csWeakRef<CS::Animation::iSkeleton> skeleton;
    csRef<CS::Animation::iSkeletonAnimNode> childNode;
    CS::Animation::BoneID boneID;
    csRef<CS::Animation::iBodyBoneJoint> bodyJoint;
    char targetMode;
    char trackingStatus;
    csVector3 targetPosition;
    csRef<iMovable> targetMovable;
    csRef<iCamera> targetCamera;
    csVector3 targetOffset;
    bool isPlaying;
    float maximumSpeed;
    bool alwaysRotate;
    float frameDuration;
    float previousPitch, previousYaw, previousRoll;
    bool trackingInitialized;
    char listenerStatus;
    float listenerMinimumDelay;
    float listenerDelay;
    csRefArray<CS::Animation::iSkeletonLookAtListener> listeners;
  };

}
CS_PLUGIN_NAMESPACE_END(LookAt)

#endif //__CS_LOOKAT_H__
