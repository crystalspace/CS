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
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef __CS_BASICNODES_H__
#define __CS_BASICNODES_H__

#include "csutil/scf_implementation.h"
#include "iutil/comp.h"
#include "csutil/leakguard.h"
#include "csutil/weakref.h"
#include "csutil/refarr.h"
#include "csutil/csstring.h"
#include "imesh/basicskelanim.h"

CS_PLUGIN_NAMESPACE_BEGIN(BasicNodes)
{

  class BasicNodesManager : public scfImplementation2<BasicNodesManager,
    CS::Animation::iSkeletonBasicNodesManager2, iComponent>
  {
  public:
    CS_LEAKGUARD_DECLARE(BasicManager);

    BasicNodesManager (iBase* parent);

    //-- CS::Animation::iSkeletonBasicNodesManager2
    virtual CS::Animation::iSkeletonSpeedNodeFactory2* CreateSpeedNodeFactory (const char* name);
    virtual CS::Animation::iSkeletonSpeedNodeFactory2* FindSpeedNodeFactory (const char* name);
    virtual void ClearSpeedNodeFactories ();

    //-- iComponent
    virtual bool Initialize (iObjectRegistry*);

    // error reporting
    void Report (int severity, const char* msg, ...) const;

  private:
    iObjectRegistry* object_reg;
    csHash<csRef<CS::Animation::iSkeletonSpeedNodeFactory2>, csString> speedFactories;
  };

  class SpeedNodeFactory : public scfImplementation2<SpeedNodeFactory, 
    scfFakeInterface<CS::Animation::iSkeletonAnimNodeFactory2>, CS::Animation::iSkeletonSpeedNodeFactory2>
  {
  public:
    CS_LEAKGUARD_DECLARE(SpeedAnimNodeFactory);

    SpeedNodeFactory (BasicNodesManager* manager, const char *name);

    //-- CS::Animation::iSkeletonSpeedNodeFactory2
    virtual void AddNode (CS::Animation::iSkeletonAnimNodeFactory2* factory, float speed);

    //-- CS::Animation::iSkeletonAnimNodeFactory2
    virtual csPtr<CS::Animation::iSkeletonAnimNode2> CreateInstance (
	       CS::Animation::iSkeletonAnimPacket2* packet, CS::Animation::iSkeleton2* skeleton);
    virtual const char* GetNodeName () const;
    virtual CS::Animation::iSkeletonAnimNodeFactory2* FindNode (const char* name);

  private:
    BasicNodesManager* manager;
    csString name;
    csRefArray<CS::Animation::iSkeletonAnimNodeFactory2> subFactories;
    csArray<float> speedList;

    friend class SpeedNode;
  };

  class SpeedNode : public scfImplementation2<SpeedNode, 
    scfFakeInterface<CS::Animation::iSkeletonAnimNode2>, CS::Animation::iSkeletonSpeedNode2>
  {
  public:
    CS_LEAKGUARD_DECLARE(SpeedNode);

    SpeedNode (SpeedNodeFactory* factory, CS::Animation::iSkeleton2* skeleton);

    //-- CS::Animation::iSkeletonSpeedNode2
    virtual void SetSpeed (float speed);

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
    void UpdateNewSpeed (float speed);

    SpeedNodeFactory* factory;
    csWeakRef<CS::Animation::iSkeleton2> skeleton;
    csRefArray<CS::Animation::iSkeletonAnimNode2> subNodes;
    float speed;
    size_t slowNode, fastNode;
    float currentPosition;
    float cycleDuration;
    float speedRatio;
    bool isPlaying;

    friend class SpeedNodeFactory;
  };

}
CS_PLUGIN_NAMESPACE_END(BasicNodes)

#endif //__CS_BASICNODES_H__
