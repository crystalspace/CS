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

class iBodyBoneJoint;

CS_PLUGIN_NAMESPACE_BEGIN(LookAt)
{

  class LookAtManager : public scfImplementation2<LookAtManager,
    iLookAtManager, iComponent>
  {
  public:
    CS_LEAKGUARD_DECLARE(LookAtManager);

    LookAtManager (iBase* parent);

    //-- iLookAtManager
    virtual iLookAtAnimNodeFactory* CreateAnimNodeFactory (const char *name,
							   iBodySkeleton* skeleton);

    virtual iLookAtAnimNodeFactory* FindAnimNodeFactory (const char* name) const;
    virtual void ClearAnimNodeFactories ();

    //-- iComponent
    virtual bool Initialize (iObjectRegistry*);

    // error reporting
    void Report (int severity, const char* msg, ...) const;

  private:
    iObjectRegistry* object_reg;
    csHash<csRef<iLookAtAnimNodeFactory>, csString> factoryHash;
  };

  class LookAtAnimNodeFactory : public scfImplementation2<LookAtAnimNodeFactory, 
    scfFakeInterface<iSkeletonAnimNodeFactory2>, iLookAtAnimNodeFactory>
  {
    friend class LookAtAnimNode;

  public:
    CS_LEAKGUARD_DECLARE(LookAtAnimNodeFactory);

    LookAtAnimNodeFactory (LookAtManager* manager, const char *name,
			   iBodySkeleton* skeleton);

    //-- iLookAtAnimNodeFactory
    virtual void SetChildNode (iSkeletonAnimNodeFactory2* node);
    virtual iSkeletonAnimNodeFactory2* GetChildNode ();
    virtual void ClearChildNode ();

    //-- iSkeletonAnimNodeFactory2
    virtual csPtr<iSkeletonAnimNode2> CreateInstance (
	       iSkeletonAnimPacket2* packet, iSkeleton2* skeleton);

    virtual const char* GetNodeName () const;

    virtual iSkeletonAnimNodeFactory2* FindNode (const char* name);

  protected:
    LookAtManager* manager;
    csString name;
    csRef<iBodySkeleton> skeleton;
    csRef<iSkeletonAnimNodeFactory2> childNode;
  };

  class LookAtAnimNode : public scfImplementation2<LookAtAnimNode, 
    scfFakeInterface<iSkeletonAnimNode2>, iLookAtAnimNode>
  {
  public:
    LookAtAnimNode (LookAtAnimNodeFactory* factory, iSkeleton2* skeleton,
		    iSkeletonAnimNode2* childNode);

    //-- iLookAtAnimNode
    virtual void SetAnimatedMesh (iAnimatedMesh* mesh);
    virtual void SetBone (BoneID boneID);

    virtual void SetTarget (csVector3 target);
    virtual void SetTarget (iMovable* target, const csVector3& offset);
    virtual void SetTarget (iCamera* target, const csVector3& offset);
    virtual void RemoveTarget ();
  
    virtual void SetMaximumSpeed (float speed);
    virtual void SetAlwaysRotate (bool alwaysRotate);

    virtual void SetListenerDelay (float delay);
    virtual void AddListener (iLookAtListener* listener);
    virtual void RemoveListener (iLookAtListener* listener);

    //-- iSkeletonAnimPacket2
    virtual void Play ();
    virtual void Stop ();

    virtual void SetPlaybackPosition (float time);
    virtual float GetPlaybackPosition () const;

    virtual float GetDuration () const;
    virtual void SetPlaybackSpeed (float speed);
    virtual float GetPlaybackSpeed () const;

    virtual void BlendState (csSkeletalState2* state,
			     float baseWeight = 1.0f);
    virtual void TickAnimation (float dt);

    virtual bool IsActive () const;
    virtual iSkeletonAnimNodeFactory2* GetFactory () const;
    virtual iSkeletonAnimNode2* FindNode (const char* name);

    virtual void AddAnimationCallback (iSkeletonAnimCallback2* callback);
    virtual void RemoveAnimationCallback (iSkeletonAnimCallback2* callback);

  private:
    void InitializeTracking ();

    LookAtAnimNodeFactory* factory;
    csWeakRef<iSceneNode> sceneNode;
    csWeakRef<iSkeleton2> skeleton;
    csRef<iSkeletonAnimNode2> childNode;
    BoneID boneID;
    csRef<iBodyBoneJoint> bodyJoint;
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
    float previousPitch, previousYaw;
    char listenerStatus;
    float listenerMinimumDelay;
    float listenerDelay;
    csRefArray<iLookAtListener> listeners;
  };

}
CS_PLUGIN_NAMESPACE_END(LookAt)

#endif //__CS_LOOKAT_H__
