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
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef __CS_BASICNODES_H__
#define __CS_BASICNODES_H__

#include "csutil/scf_implementation.h"
#include "cstool/animnodetmpl.h"
#include "csutil/leakguard.h"
#include "csutil/weakref.h"
#include "csutil/refarr.h"
#include "csutil/csstring.h"
#include "imesh/animnode/speed.h"
#include "iutil/comp.h"

CS_PLUGIN_NAMESPACE_BEGIN(SpeedNode)
{

  class SpeedNodeManager;

  class SpeedNodeFactory : public scfImplementation2<SpeedNodeFactory, 
    scfFakeInterface<CS::Animation::iSkeletonAnimNodeFactory>, CS::Animation::iSkeletonSpeedNodeFactory>
  {
  public:
    CS_LEAKGUARD_DECLARE(SpeedAnimNodeFactory);

    SpeedNodeFactory (SpeedNodeManager* manager, const char *name);

    //-- CS::Animation::iSkeletonSpeedNodeFactory
    virtual void AddNode (CS::Animation::iSkeletonAnimNodeFactory* factory, float speed);

    //-- CS::Animation::iSkeletonAnimNodeFactory
    virtual csPtr<CS::Animation::iSkeletonAnimNode> CreateInstance (
	       CS::Animation::iSkeletonAnimPacket* packet, CS::Animation::iSkeleton* skeleton);
    virtual const char* GetNodeName () const;
    virtual CS::Animation::iSkeletonAnimNodeFactory* FindNode (const char* name);

  private:
    SpeedNodeManager* manager;
    csString name;
    csRefArray<CS::Animation::iSkeletonAnimNodeFactory> subFactories;
    csArray<float> speedList;

    friend class SpeedNode;
  };

  class SpeedNode : public scfImplementation2<SpeedNode, 
    scfFakeInterface<CS::Animation::iSkeletonAnimNode>, CS::Animation::iSkeletonSpeedNode>
  {
  public:
    CS_LEAKGUARD_DECLARE(SpeedNode);

    SpeedNode (SpeedNodeFactory* factory, CS::Animation::iSkeleton* skeleton);

    //-- CS::Animation::iSkeletonSpeedNode
    virtual void SetSpeed (float speed);

    //-- CS::Animation::iSkeletonAnimNode
    virtual void Play ();
    virtual void Stop ();

    virtual void SetPlaybackPosition (float time);
    virtual float GetPlaybackPosition () const;

    virtual float GetDuration () const;
    virtual void SetPlaybackSpeed (float speed);
    virtual float GetPlaybackSpeed () const;

    virtual void BlendState (CS::Animation::AnimatedMeshState* state,
			     float baseWeight = 1.0f);
    virtual void TickAnimation (float dt);

    virtual bool IsActive () const;
    virtual CS::Animation::iSkeletonAnimNodeFactory* GetFactory () const;
    virtual CS::Animation::iSkeletonAnimNode* FindNode (const char* name);

    virtual void AddAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback);
    virtual void RemoveAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback);

  private:
    void UpdateNewSpeed (float speed);

    SpeedNodeFactory* factory;
    csWeakRef<CS::Animation::iSkeleton> skeleton;
    csRefArray<CS::Animation::iSkeletonAnimNode> subNodes;
    float playbackSpeed;
    float speed;
    size_t slowNode, fastNode;
    float currentPosition;
    float cycleDuration;
    float speedRatio;
    bool isPlaying;

    friend class SpeedNodeFactory;
  };

  class SpeedNodeManager
    : public CS::Animation::AnimNodeManagerCommon<SpeedNodeManager,
						  CS::Animation::iSkeletonSpeedNodeManager,
						  SpeedNodeFactory>
  {
  public:
    SpeedNodeManager (iBase* parent)
     : AnimNodeManagerCommonType (parent) {}
  };

}
CS_PLUGIN_NAMESPACE_END(SpeedNode)

#endif //__CS_BASICNODES_H__
