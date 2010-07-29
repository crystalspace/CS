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
#include "imesh/lookat.h"

namespace CS
{
namespace Animation
{

struct iBodyBoneJoint;

}
}

CS_PLUGIN_NAMESPACE_BEGIN(LookAt)
{

  class LookAtManager : public scfImplementation2<LookAtManager,
    CS::Animation::iSkeletonLookAtManager2, iComponent>
  {
  public:
    CS_LEAKGUARD_DECLARE(LookAtManager);

    LookAtManager (iBase* parent);

    //-- CS::Animation::iSkeletonLookAtManager2
    virtual CS::Animation::iSkeletonLookAtNodeFactory2* CreateAnimNodeFactory (const char *name,
							   CS::Animation::iBodySkeleton* skeleton);

    virtual CS::Animation::iSkeletonLookAtNodeFactory2* FindAnimNodeFactory (const char* name) const;
    virtual void ClearAnimNodeFactories ();

    //-- iComponent
    virtual bool Initialize (iObjectRegistry*);

    // error reporting
    void Report (int severity, const char* msg, ...) const;

  private:
    iObjectRegistry* object_reg;
    csHash<csRef<CS::Animation::iSkeletonLookAtNodeFactory2>, csString> factoryHash;
  };

  class LookAtAnimNodeFactory : public scfImplementation2<LookAtAnimNodeFactory, 
    scfFakeInterface<CS::Animation::iSkeletonAnimNodeFactory2>, CS::Animation::iSkeletonLookAtNodeFactory2>
  {
    friend class LookAtAnimNode;

  public:
    CS_LEAKGUARD_DECLARE(LookAtAnimNodeFactory);

    LookAtAnimNodeFactory (LookAtManager* manager, const char *name,
			   CS::Animation::iBodySkeleton* skeleton);

    //-- CS::Animation::iSkeletonLookAtNodeFactory2
    virtual void SetChildNode (CS::Animation::iSkeletonAnimNodeFactory2* node);
    virtual CS::Animation::iSkeletonAnimNodeFactory2* GetChildNode ();
    virtual void ClearChildNode ();

    //-- CS::Animation::iSkeletonAnimNodeFactory2
    virtual csPtr<CS::Animation::iSkeletonAnimNode2> CreateInstance (
	       CS::Animation::iSkeletonAnimPacket2* packet, CS::Animation::iSkeleton2* skeleton);

    virtual const char* GetNodeName () const;

    virtual CS::Animation::iSkeletonAnimNodeFactory2* FindNode (const char* name);

  protected:
    LookAtManager* manager;
    csString name;
    csRef<CS::Animation::iBodySkeleton> skeleton;
    csRef<CS::Animation::iSkeletonAnimNodeFactory2> childNode;
  };

  class LookAtAnimNode : public scfImplementation2<LookAtAnimNode, 
    scfFakeInterface<CS::Animation::iSkeletonAnimNode2>, CS::Animation::iSkeletonLookAtNode2>
  {
  public:
    CS_LEAKGUARD_DECLARE(LookAtAnimNode);

    LookAtAnimNode (LookAtAnimNodeFactory* factory, CS::Animation::iSkeleton2* skeleton,
		    CS::Animation::iSkeletonAnimNode2* childNode);

    //-- CS::Animation::iSkeletonLookAtNode2
    virtual void SetBone (CS::Animation::BoneID boneID);

    virtual void SetTarget (csVector3 target);
    virtual void SetTarget (iMovable* target, const csVector3& offset);
    virtual void SetTarget (iCamera* target, const csVector3& offset);
    virtual void RemoveTarget ();
  
    virtual void SetMaximumSpeed (float speed);
    virtual void SetAlwaysRotate (bool alwaysRotate);

    virtual void SetListenerDelay (float delay);
    virtual void AddListener (CS::Animation::iSkeletonLookAtListener2* listener);
    virtual void RemoveListener (CS::Animation::iSkeletonLookAtListener2* listener);

    //-- CS::Animation::iSkeletonAnimPacket2
    virtual void Play ();
    virtual void Stop ();

    virtual void SetPlaybackPosition (float time);
    virtual float GetPlaybackPosition () const;

    virtual float GetDuration () const;
    virtual void SetPlaybackSpeed (float speed);
    virtual float GetPlaybackSpeed () const;

    virtual void BlendState (CS::Animation::csSkeletalState2* state,
			     float baseWeight = 1.0f);
    virtual void TickAnimation (float dt);

    virtual bool IsActive () const;
    virtual CS::Animation::iSkeletonAnimNodeFactory2* GetFactory () const;
    virtual CS::Animation::iSkeletonAnimNode2* FindNode (const char* name);

    virtual void AddAnimationCallback (CS::Animation::iSkeletonAnimCallback2* callback);
    virtual void RemoveAnimationCallback (CS::Animation::iSkeletonAnimCallback2* callback);

  private:
    LookAtAnimNodeFactory* factory;
    csWeakRef<iSceneNode> sceneNode;
    csWeakRef<CS::Animation::iSkeleton2> skeleton;
    csRef<CS::Animation::iSkeletonAnimNode2> childNode;
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
    csRefArray<CS::Animation::iSkeletonLookAtListener2> listeners;
  };

}
CS_PLUGIN_NAMESPACE_END(LookAt)

#endif //__CS_LOOKAT_H__
