/*
  Copyright (C) 2008 by Marten Svanfeldt

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

#ifndef __CS_NODES_H__
#define __CS_NODES_H__

#include "csutil/scf_implementation.h"
#include "imesh/skeleton2.h"
#include "imesh/animnode/skeleton2anim.h"
#include "csutil/leakguard.h"
#include "csutil/refarr.h"
#include "csutil/csstring.h"
#include "csutil/weakrefarr.h"

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2)
{
  /**
   * Helper class for nodes having no children
   */
  class BaseNodeSingle
  {
  protected:
    CS_LEAKGUARD_DECLARE(BaseNodeSingle);
  
    BaseNodeSingle (CS::Animation::iSkeletonAnimNode* owner)
      : owner (owner)
    {}

    inline void FireAnimationFinishedCb ()
    {
      for (size_t i = 0; i < callbacks.GetSize (); ++i)
      {
        callbacks[i]->AnimationFinished (owner);
      }
    }

    inline void FireAnimationCycleCb ()
    {
      for (size_t i = 0; i < callbacks.GetSize (); ++i)
      {
        callbacks[i]->AnimationCycled (owner);
      }
    }

    inline void FireStateChangeCb (bool playing)
    {
      for (size_t i = 0; i < callbacks.GetSize (); ++i)
      {
        callbacks[i]->PlayStateChanged (owner, playing);
      }
    }

    inline void FireDurationChangeCb ()
    {
      for (size_t i = 0; i < callbacks.GetSize (); ++i)
      {
        callbacks[i]->DurationChanged (owner);
      }
    }

    inline void AddAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback)
    {
      callbacks.PushSmart (callback);
    }

    inline void RemoveAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback)
    {
      callbacks.Delete (callback);
    }    
    
    csWeakRefArray<CS::Animation::iSkeletonAnimCallback> callbacks;
    CS::Animation::iSkeletonAnimNode* owner;
  };


  /**
   * Helper class for nodes having one or more children
   */
  class BaseNodeChildren : public BaseNodeSingle
  {
  protected:
    CS_LEAKGUARD_DECLARE(BaseNodeChildren);
  
    BaseNodeChildren (CS::Animation::iSkeletonAnimNode* owner)
      : BaseNodeSingle (owner), manualCbInstall (false)
    {}
    virtual ~BaseNodeChildren () {}

    void AddAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback);
    void RemoveAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback);

    void InstallInnerCb (bool manual);
    void RemoveInnerCb (bool manual);

    virtual void AnimationFinished (CS::Animation::iSkeletonAnimNode* node);
    virtual void AnimationCycled (CS::Animation::iSkeletonAnimNode* node);
    virtual void PlayStateChanged (CS::Animation::iSkeletonAnimNode* node, bool isPlaying);
    virtual void DurationChanged (CS::Animation::iSkeletonAnimNode* node);

    class InnerCallback : public scfImplementation1<InnerCallback,
                                                    CS::Animation::iSkeletonAnimCallback>
    {
    public:
      InnerCallback (BaseNodeChildren* parent);

      virtual ~InnerCallback () {}

      //-- CS::Animation::iSkeletonAnimCallback
      virtual void AnimationFinished (CS::Animation::iSkeletonAnimNode* node);
      virtual void AnimationCycled (CS::Animation::iSkeletonAnimNode* node);
      virtual void PlayStateChanged (CS::Animation::iSkeletonAnimNode* node, bool isPlaying);
      virtual void DurationChanged (CS::Animation::iSkeletonAnimNode* node);

    private:
      BaseNodeChildren* parent;
    };

    csRef<InnerCallback> cb;
    csRefArray<CS::Animation::iSkeletonAnimNode> subNodes;
    bool manualCbInstall;
    friend class BaseFactoryChildren; 
  };

  /**
   * Helper class for factories having multiple children
   */
  class BaseFactoryChildren
  {
  protected:
    CS_LEAKGUARD_DECLARE(BaseFactoryChildren);
  
    void SetupInstance (BaseNodeChildren* child, CS::Animation::iSkeletonAnimPacket* packet, 
      CS::Animation::iSkeleton* skeleton)
    {
      for (size_t i = 0; i < subFactories.GetSize (); ++i)
      {
        csRef<CS::Animation::iSkeletonAnimNode> node = 
          subFactories[i]->CreateInstance (packet, skeleton);
        child->subNodes.Push (node);
      }
    }

    csRefArray<CS::Animation::iSkeletonAnimNodeFactory> subFactories;
  };



  //----------------------------------------

  class AnimationNode;

  class AnimationNodeFactory :
    public scfImplementation2<AnimationNodeFactory,
                              CS::Animation::iSkeletonAnimationNodeFactory,
                              scfFakeInterface<CS::Animation::iSkeletonAnimNodeFactory> >
  {
  public:
    CS_LEAKGUARD_DECLARE(AnimationNodeFactory);
  
    AnimationNodeFactory (const char* name);

    //-- CS::Animation::iSkeletonAnimationNodeFactory
    virtual void SetAnimation (CS::Animation::iSkeletonAnimation* animation);
    virtual CS::Animation::iSkeletonAnimation* GetAnimation () const;
    virtual void SetCyclic (bool cyclic);
    virtual bool IsCyclic () const;
    virtual void SetPlaybackSpeed (float speed);
    virtual float GetPlaybackSpeed () const;
    virtual void SetAutomaticReset (bool reset);
    virtual bool GetAutomaticReset () const;
    virtual void SetAutomaticStop (bool enabled);
    virtual bool GetAutomaticStop () const;

    //-- CS::Animation::iSkeletonAnimNodeFactory
    virtual csPtr<CS::Animation::iSkeletonAnimNode> CreateInstance (
      CS::Animation::iSkeletonAnimPacket* packet, CS::Animation::iSkeleton* skeleton);
    virtual const char* GetNodeName () const;
    virtual CS::Animation::iSkeletonAnimNodeFactory* FindNode (const char* name);

  private:
    csString name;

    csRef<CS::Animation::iSkeletonAnimation> animation;
    bool cyclic, automaticReset, automaticStop;
    float playbackSpeed;

    friend class AnimationNode;
  };

  class AnimationNode :
    public scfImplementation2<AnimationNode,
                              CS::Animation::iSkeletonAnimationNode,
                              scfFakeInterface<CS::Animation::iSkeletonAnimNode> >,
    public BaseNodeSingle
  {
  public:
    CS_LEAKGUARD_DECLARE(AnimationNode);
  
    AnimationNode (AnimationNodeFactory* factory);

    //-- CS::Animation::iSkeletonAnimationNode
    

    //-- CS::Animation::iSkeletonAnimNode
    virtual void Play ();
    virtual void Stop ();
    virtual void SetPlaybackPosition (float time);
    virtual float GetPlaybackPosition () const;
    virtual float GetDuration () const;
    virtual void SetPlaybackSpeed (float speed);
    virtual float GetPlaybackSpeed () const;
    virtual void BlendState (CS::Animation::AnimatedMeshState* state, float baseWeight = 1.0f);
    virtual void TickAnimation (float dt);
    virtual bool IsActive () const;
    virtual CS::Animation::iSkeletonAnimNodeFactory* GetFactory () const;
    virtual CS::Animation::iSkeletonAnimNode* FindNode (const char* name);
    virtual void AddAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback);
    virtual void RemoveAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback);

  private:
    csRef<AnimationNodeFactory> factory;

    bool isPlaying;
    float playbackPosition;
    float playbackSpeed;
  };
  

}
CS_PLUGIN_NAMESPACE_END(Skeleton2)


#endif
