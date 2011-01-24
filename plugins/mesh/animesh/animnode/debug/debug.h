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
#include "cstool/animnodetmpl.h"
#include "csutil/bitarray.h"
#include "csutil/leakguard.h"
#include "csutil/weakref.h"
#include "csutil/refarr.h"
#include "csutil/csstring.h"
#include "iutil/comp.h"
#include "imesh/animnode/debug.h"
#include "imesh/bodymesh.h"
#include "iengine/sector.h"
#include "iengine/material.h"

CS_PLUGIN_NAMESPACE_BEGIN(DebugNode)
{

  CS_DECLARE_ANIMNODE_MANAGER(DebugNode, DebugNode, CS::Animation::iSkeletonDebugNodeFactory);

  class DebugNodeFactory
    : public scfImplementation2<DebugNodeFactory, 
    scfFakeInterface<CS::Animation::iSkeletonAnimNodeFactory>,
    CS::Animation::iSkeletonDebugNodeFactory>,
    CS::Animation::csSkeletonAnimNodeFactorySingle
  {
  public:
    CS_LEAKGUARD_DECLARE(DebugAnimNodeFactory);

    DebugNodeFactory (DebugNodeManager* manager, const char *name);
    ~DebugNodeFactory () {}

    //-- CS::Animation::iSkeletonDebugNodeFactory
    virtual void SetDebugModes (CS::Animation::SkeletonDebugMode modes);
    virtual CS::Animation::SkeletonDebugMode GetDebugModes ();
    virtual void SetDebugImage (csPixmap* image);
    virtual void SetBoneMask (csBitArray& boneMask);
    virtual void UnsetBoneMask ();
    virtual void SetLeafBonesDisplayed (bool displayed);

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

  private:
    DebugNodeManager* manager;
    CS::Animation::SkeletonDebugMode modes;
    csPixmap* image;
    bool boneMaskUsed;
    csBitArray boneMask;
    bool leafBonesDisplayed;

    friend class DebugNode;
  };

  class DebugNode
    : public scfImplementation2<DebugNode, 
    scfFakeInterface<CS::Animation::iSkeletonAnimNode>,
    CS::Animation::iSkeletonDebugNode>,
    CS::Animation::csSkeletonAnimNodeSingle
  {
  public:
    CS_LEAKGUARD_DECLARE(DebugNode);

    DebugNode (DebugNodeFactory* factory, CS::Animation::iSkeleton* skeleton);
    ~DebugNode () {}

    //-- CS::Animation::iSkeletonDebugNode
    virtual void Draw (iCamera* camera, csColor color = csColor (255, 0, 255));

    //-- CS::Animation::iSkeletonAnimNode
    inline virtual void Play ()
    { csSkeletonAnimNodeSingle::Play (); }
    inline virtual void Stop ()
    { csSkeletonAnimNodeSingle::Stop (); }

    inline virtual void SetPlaybackPosition (float time)
    { csSkeletonAnimNodeSingle::SetPlaybackPosition (time); }
    inline virtual float GetPlaybackPosition () const
    { return csSkeletonAnimNodeSingle::GetPlaybackPosition (); }

    inline virtual float GetDuration () const
    { return csSkeletonAnimNodeSingle::GetDuration (); }
    inline virtual void SetPlaybackSpeed (float speed)
    { csSkeletonAnimNodeSingle::SetPlaybackSpeed (speed); }
    inline virtual float GetPlaybackSpeed () const
    { return csSkeletonAnimNodeSingle::GetPlaybackSpeed (); }

    inline virtual void BlendState (CS::Animation::csSkeletalState* state,
				    float baseWeight = 1.0f)
    { csSkeletonAnimNodeSingle::BlendState (state, baseWeight); }
    inline virtual void TickAnimation (float dt)
    { csSkeletonAnimNodeSingle::TickAnimation (dt); }

    inline virtual bool IsActive () const
    { return csSkeletonAnimNodeSingle::IsActive (); }

    virtual CS::Animation::iSkeletonAnimNodeFactory* GetFactory () const;
    virtual CS::Animation::iSkeletonAnimNode* FindNode (const char* name);

    inline virtual void AddAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback)
    { csSkeletonAnimNodeSingle::AddAnimationCallback (callback); }
    inline virtual void RemoveAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback)
    { csSkeletonAnimNodeSingle::RemoveAnimationCallback (callback); }

  private:
    csRef<DebugNodeFactory> factory;

    friend class DebugNodeFactory;
  };

}
CS_PLUGIN_NAMESPACE_END(DebugNode)

#endif // __CS_DEBUGNODE_H__
