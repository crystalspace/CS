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
#include "imesh/skeleton2anim.h"
#include "csutil/leakguard.h"
#include "csutil/refarr.h"
#include "csutil/csstring.h"

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2)
{
  /**
   * Helper class for nodes having no children
   */
  class BaseNodeSingle
  {
  protected:
    CS_LEAKGUARD_DECLARE(BaseNodeSingle);
  
    BaseNodeSingle (CS::Animation::iSkeletonAnimNode2* owner)
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

    inline void AddAnimationCallback (CS::Animation::iSkeletonAnimCallback2* callback)
    {
      callbacks.PushSmart (callback);
    }

    inline void RemoveAnimationCallback (CS::Animation::iSkeletonAnimCallback2* callback)
    {
      callbacks.Delete (callback);
    }    
    
    csRefArray<CS::Animation::iSkeletonAnimCallback2> callbacks;
    CS::Animation::iSkeletonAnimNode2* owner;
  };


  /**
   * Helper class for nodes having one or more children
   */
  class BaseNodeChildren : public BaseNodeSingle
  {
  protected:
    CS_LEAKGUARD_DECLARE(BaseNodeChildren);
  
    BaseNodeChildren (CS::Animation::iSkeletonAnimNode2* owner)
      : BaseNodeSingle (owner), manualCbInstall (false)
    {}
    virtual ~BaseNodeChildren () {}

    void AddAnimationCallback (CS::Animation::iSkeletonAnimCallback2* callback);
    void RemoveAnimationCallback (CS::Animation::iSkeletonAnimCallback2* callback);

    void InstallInnerCb (bool manual);
    void RemoveInnerCb (bool manual);

    virtual void AnimationFinished (CS::Animation::iSkeletonAnimNode2* node);
    virtual void AnimationCycled (CS::Animation::iSkeletonAnimNode2* node);
    virtual void PlayStateChanged (CS::Animation::iSkeletonAnimNode2* node, bool isPlaying);
    virtual void DurationChanged (CS::Animation::iSkeletonAnimNode2* node);

    class InnerCallback : public scfImplementation1<InnerCallback,
                                                    CS::Animation::iSkeletonAnimCallback2>
    {
    public:
      InnerCallback (BaseNodeChildren* parent);

      virtual ~InnerCallback () {}

      //-- CS::Animation::iSkeletonAnimCallback2
      virtual void AnimationFinished (CS::Animation::iSkeletonAnimNode2* node);
      virtual void AnimationCycled (CS::Animation::iSkeletonAnimNode2* node);
      virtual void PlayStateChanged (CS::Animation::iSkeletonAnimNode2* node, bool isPlaying);
      virtual void DurationChanged (CS::Animation::iSkeletonAnimNode2* node);

    private:
      BaseNodeChildren* parent;
    };

    csRef<InnerCallback> cb;
    csRefArray<CS::Animation::iSkeletonAnimNode2> subNodes;
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
  
    void SetupInstance (BaseNodeChildren* child, CS::Animation::iSkeletonAnimPacket2* packet, 
      CS::Animation::iSkeleton2* skeleton)
    {
      for (size_t i = 0; i < subFactories.GetSize (); ++i)
      {
        csRef<CS::Animation::iSkeletonAnimNode2> node = 
          subFactories[i]->CreateInstance (packet, skeleton);
        child->subNodes.Push (node);
      }
    }

    csRefArray<CS::Animation::iSkeletonAnimNodeFactory2> subFactories;
  };



  //----------------------------------------

  class AnimationNode;

  class AnimationNodeFactory :
    public scfImplementation2<AnimationNodeFactory,
                              CS::Animation::iSkeletonAnimationNodeFactory2,
                              scfFakeInterface<CS::Animation::iSkeletonAnimNodeFactory2> >
  {
  public:
    CS_LEAKGUARD_DECLARE(AnimationNodeFactory);
  
    AnimationNodeFactory (const char* name);

    //-- CS::Animation::iSkeletonAnimationNodeFactory2
    virtual void SetAnimation (CS::Animation::iSkeletonAnimation2* animation);
    virtual CS::Animation::iSkeletonAnimation2* GetAnimation () const;
    virtual void SetCyclic (bool cyclic);
    virtual bool IsCyclic () const;
    virtual void SetPlaybackSpeed (float speed);
    virtual float GetPlaybackSpeed () const;
    virtual void SetAutomaticReset (bool reset);
    virtual bool GetAutomaticReset () const;
    virtual void SetAutomaticStop (bool enabled);
    virtual bool GetAutomaticStop () const;

    //-- CS::Animation::iSkeletonAnimNodeFactory2
    virtual csPtr<CS::Animation::iSkeletonAnimNode2> CreateInstance (
      CS::Animation::iSkeletonAnimPacket2* packet, CS::Animation::iSkeleton2* skeleton);
    virtual const char* GetNodeName () const;
    virtual CS::Animation::iSkeletonAnimNodeFactory2* FindNode (const char* name);

  private:
    csString name;

    csRef<CS::Animation::iSkeletonAnimation2> animation;
    bool cyclic, automaticReset, automaticStop;
    float playbackSpeed, animationDuration;

    friend class AnimationNode;
  };

  class AnimationNode :
    public scfImplementation2<AnimationNode,
                              CS::Animation::iSkeletonAnimationNode2,
                              scfFakeInterface<CS::Animation::iSkeletonAnimNode2> >,
    public BaseNodeSingle
  {
  public:
    CS_LEAKGUARD_DECLARE(AnimationNode);
  
    AnimationNode (AnimationNodeFactory* factory);

    //-- CS::Animation::iSkeletonAnimationNode2
    

    //-- CS::Animation::iSkeletonAnimNode2
    virtual void Play ();
    virtual void Stop ();
    virtual void SetPlaybackPosition (float time);
    virtual float GetPlaybackPosition () const;
    virtual float GetDuration () const;
    virtual void SetPlaybackSpeed (float speed);
    virtual float GetPlaybackSpeed () const;
    virtual void BlendState (CS::Animation::csSkeletalState2* state, float baseWeight = 1.0f);
    virtual void TickAnimation (float dt);
    virtual bool IsActive () const;
    virtual CS::Animation::iSkeletonAnimNodeFactory2* GetFactory () const;
    virtual CS::Animation::iSkeletonAnimNode2* FindNode (const char* name);
    virtual void AddAnimationCallback (CS::Animation::iSkeletonAnimCallback2* callback);
    virtual void RemoveAnimationCallback (CS::Animation::iSkeletonAnimCallback2* callback);

  private:
    csRef<AnimationNodeFactory> factory;

    bool isPlaying;
    float playbackPosition;
    float playbackSpeed;
  };
  

}
CS_PLUGIN_NAMESPACE_END(Skeleton2)


#endif
