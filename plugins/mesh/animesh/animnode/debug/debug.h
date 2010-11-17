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
#ifndef __CS_DEBUGNODE_H__
#define __CS_DEBUGNODE_H__

#include "csutil/scf_implementation.h"
#include "iutil/comp.h"
#include "csutil/bitarray.h"
#include "csutil/leakguard.h"
#include "csutil/weakref.h"
#include "csutil/refarr.h"
#include "csutil/csstring.h"
#include "imesh/animnode/debug.h"
#include "imesh/bodymesh.h"
#include "iengine/sector.h"
#include "iengine/material.h"

CS_PLUGIN_NAMESPACE_BEGIN(DebugNode)
{

  class DebugNodeManager : public scfImplementation2<DebugNodeManager,
    CS::Animation::iSkeletonDebugNodeManager, iComponent>
  {
  public:
    CS_LEAKGUARD_DECLARE(DebugNodeManager);

    DebugNodeManager (iBase* parent);

    //-- CS::Animation::iSkeletonDebugNodeManager
    virtual CS::Animation::iSkeletonDebugNodeFactory* CreateAnimNodeFactory (const char* name);
    virtual CS::Animation::iSkeletonDebugNodeFactory* FindAnimNodeFactory (const char* name);
    virtual void ClearAnimNodeFactories ();

    //-- iComponent
    virtual bool Initialize (iObjectRegistry*);

    // Error reporting
    void Report (int severity, const char* msg, ...) const;

  private:
    iObjectRegistry* object_reg;
    csHash<csRef<CS::Animation::iSkeletonDebugNodeFactory>, csString> debugFactories;

    friend class DebugNodeFactory;
    friend class DebugNode;
  };

  class DebugNodeFactory : public scfImplementation2<DebugNodeFactory, 
    scfFakeInterface<CS::Animation::iSkeletonAnimNodeFactory>, CS::Animation::iSkeletonDebugNodeFactory>
  {
  public:
    CS_LEAKGUARD_DECLARE(DebugAnimNodeFactory);

    DebugNodeFactory (DebugNodeManager* manager, const char *name);
    ~DebugNodeFactory () {}

    //-- CS::Animation::iSkeletonDebugNodeFactory
    virtual void SetDebugModes (CS::Animation::SkeletonDebugMode modes);
    virtual void SetDebugImage (csPixmap* image);
    virtual void SetChildNode (CS::Animation::iSkeletonAnimNodeFactory* factory);
    virtual void AddChainMask (CS::Animation::iBodyChain* chain);
    virtual void RemoveChainMask (CS::Animation::iBodyChain* chain);
    virtual void SetLeafBonesDisplayed (bool displayed);

    //-- CS::Animation::iSkeletonAnimNodeFactory
    virtual csPtr<CS::Animation::iSkeletonAnimNode> CreateInstance (
	       CS::Animation::iSkeletonAnimPacket* packet, CS::Animation::iSkeleton* skeleton);
    virtual const char* GetNodeName () const;
    virtual CS::Animation::iSkeletonAnimNodeFactory* FindNode (const char* name);

  private:
    void ResetChainMask ();
    void ResetChainMaskNode (CS::Animation::iBodyChainNode* node);

    DebugNodeManager* manager;
    csString name;
    CS::Animation::SkeletonDebugMode modes;
    csPixmap* image;
    csRef<CS::Animation::iSkeletonAnimNodeFactory> subFactory;
    csRefArray<CS::Animation::iBodyChain> chains;
    csBitArray chainMask;
    bool leafBonesDisplayed;

    friend class DebugNode;
  };

  class DebugNode : public scfImplementation2<DebugNode, 
    scfFakeInterface<CS::Animation::iSkeletonAnimNode>, CS::Animation::iSkeletonDebugNode>
  {
  public:
    CS_LEAKGUARD_DECLARE(DebugNode);

    DebugNode (DebugNodeFactory* factory, CS::Animation::iSkeleton* skeleton);
    ~DebugNode () {}

    //-- CS::Animation::iSkeletonDebugNode
    virtual void Draw (iCamera* camera, csColor color = csColor (255, 0, 255));

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
    DebugNodeFactory* factory;
    csWeakRef<CS::Animation::iSkeleton> skeleton;
    csRef<CS::Animation::iSkeletonAnimNode> subNode;

    friend class DebugNodeFactory;
  };

}
CS_PLUGIN_NAMESPACE_END(DebugNode)

#endif // __CS_DEBUGNODE_H__
