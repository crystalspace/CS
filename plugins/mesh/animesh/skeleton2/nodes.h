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
  
    BaseNodeSingle (iSkeletonAnimNode2* owner)
      : owner (owner)
    {}

    void FireAnimationFinishedCb ()
    {
      for (size_t i = 0; i < callbacks.GetSize (); ++i)
      {
        callbacks[i]->AnimationFinished (owner);
      }
    }

    void FireAnimationCycleCb ()
    {
      for (size_t i = 0; i < callbacks.GetSize (); ++i)
      {
        callbacks[i]->AnimationCycled (owner);
      }
    }

    void FireStateChangeCb (bool playing)
    {
      for (size_t i = 0; i < callbacks.GetSize (); ++i)
      {
        callbacks[i]->PlayStateChanged (owner, playing);
      }
    }

    void FireDurationChangeCb ()
    {
      for (size_t i = 0; i < callbacks.GetSize (); ++i)
      {
        callbacks[i]->DurationChanged (owner);
      }
    }

    void AddAnimationCallback (iSkeletonAnimCallback2* callback)
    {
      callbacks.PushSmart (callback);
    }

    void RemoveAnimationCallback (iSkeletonAnimCallback2* callback)
    {
      callbacks.Delete (callback);
    }    
    
    csRefArray<iSkeletonAnimCallback2> callbacks;
    iSkeletonAnimNode2* owner;
  };


  /**
   * Helper class for nodes having one or more children
   */
  class BaseNodeChildren : public BaseNodeSingle
  {
  protected:
    CS_LEAKGUARD_DECLARE(BaseNodeChildren);
  
    BaseNodeChildren (iSkeletonAnimNode2* owner)
      : BaseNodeSingle (owner), manualCbInstall (false)
    {}

    void AddAnimationCallback (iSkeletonAnimCallback2* callback);
    void RemoveAnimationCallback (iSkeletonAnimCallback2* callback);

    void InstallInnerCb (bool manual);
    void RemoveInnerCb (bool manual);

    virtual void AnimationFinished (iSkeletonAnimNode2* node);
    virtual void AnimationCycled (iSkeletonAnimNode2* node);
    virtual void PlayStateChanged (iSkeletonAnimNode2* node, bool isPlaying);
    virtual void DurationChanged (iSkeletonAnimNode2* node);

    class InnerCallback : public scfImplementation1<InnerCallback,
                                                    iSkeletonAnimCallback2>
    {
    public:
      InnerCallback (BaseNodeChildren* parent);

      virtual ~InnerCallback () {}

      //-- iSkeletonAnimCallback2
      virtual void AnimationFinished (iSkeletonAnimNode2* node);
      virtual void AnimationCycled (iSkeletonAnimNode2* node);
      virtual void PlayStateChanged (iSkeletonAnimNode2* node, bool isPlaying);
      virtual void DurationChanged (iSkeletonAnimNode2* node);

    private:
      BaseNodeChildren* parent;
    };

    csRef<InnerCallback> cb;
    csRefArray<iSkeletonAnimNode2> subNodes;
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
  
    void SetupInstance (BaseNodeChildren* child, iSkeletonAnimPacket2* packet, 
      iSkeleton2* skeleton)
    {
      for (size_t i = 0; i < subFactories.GetSize (); ++i)
      {
        csRef<iSkeletonAnimNode2> node = 
          subFactories[i]->CreateInstance (packet, skeleton);
        child->subNodes.Push (node);
      }
    }

    csRefArray<iSkeletonAnimNodeFactory2> subFactories;
  };



  //----------------------------------------

  class AnimationNode;

  class AnimationNodeFactory :
    public scfImplementation2<AnimationNodeFactory,
                              iSkeletonAnimationNodeFactory2,
                              scfFakeInterface<iSkeletonAnimNodeFactory2> >
  {
  public:
    CS_LEAKGUARD_DECLARE(AnimationNodeFactory);
  
    AnimationNodeFactory (const char* name);

    //-- iSkeletonAnimationNodeFactory2
    virtual void SetAnimation (iSkeletonAnimation2* animation);
    virtual iSkeletonAnimation2* GetAnimation () const;
    virtual void SetCyclic (bool cyclic);
    virtual bool IsCyclic () const;
    virtual void SetPlaybackSpeed (float speed);
    virtual float GetPlaybackSpeed () const;
    virtual void SetAutomaticReset (bool reset);
    virtual bool GetAutomaticReset () const;
    virtual void SetAutomaticStop (bool enabled);
    virtual bool GetAutomaticStop () const;

    //-- iSkeletonAnimNodeFactory2
    virtual csPtr<iSkeletonAnimNode2> CreateInstance (
      iSkeletonAnimPacket2* packet, iSkeleton2* skeleton);
    virtual const char* GetNodeName () const;
    virtual iSkeletonAnimNodeFactory2* FindNode (const char* name);

  private:
    csString name;

    csRef<iSkeletonAnimation2> animation;
    bool cyclic, automaticReset, automaticStop;
    float playbackSpeed, animationDuration;

    friend class AnimationNode;
  };

  class AnimationNode :
    public scfImplementation2<AnimationNode,
                              iSkeletonAnimationNode2,
                              scfFakeInterface<iSkeletonAnimNode2> >,
    public BaseNodeSingle
  {
  public:
    CS_LEAKGUARD_DECLARE(AnimationNode);
  
    AnimationNode (AnimationNodeFactory* factory);

    //-- iSkeletonAnimationNode2
    

    //-- iSkeletonAnimNode2
    virtual void Play ();
    virtual void Stop ();
    virtual void SetPlaybackPosition (float time);
    virtual float GetPlaybackPosition () const;
    virtual float GetDuration () const;
    virtual void SetPlaybackSpeed (float speed);
    virtual float GetPlaybackSpeed () const;
    virtual void BlendState (csSkeletalState2* state, float baseWeight = 1.0f);
    virtual void TickAnimation (float dt);
    virtual bool IsActive () const;
    virtual iSkeletonAnimNodeFactory2* GetFactory () const;
    virtual iSkeletonAnimNode2* FindNode (const char* name);
    virtual void AddAnimationCallback (iSkeletonAnimCallback2* callback);
    virtual void RemoveAnimationCallback (iSkeletonAnimCallback2* callback);

  private:
    csRef<AnimationNodeFactory> factory;

    bool isPlaying;
    float playbackPosition;
    float playbackSpeed;
  };
  

}
CS_PLUGIN_NAMESPACE_END(Skeleton2)


#endif
