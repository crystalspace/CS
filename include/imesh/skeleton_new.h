/*
    Copyright (C) 2005 by Hristo Hristov

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

#ifndef __CS_IASKELETON_H__
#define __CS_IASKELETON_H__

#include "csgeom/quaternion.h"
#include "csutil/scf.h"

struct iAmirSkeletonFactory;

struct iAmirSkeletonAnimationNode : public iBase
{
  SCF_INTERFACE (iAmirSkeletonAnimationNode, 0, 0, 0);

  // The blend is linearly increased from 0 to 1 (after feather_in), and then
  // starts decreasing back to 0 when feather_out ticks is left in the animation.
  virtual csTicks GetFeatherIn () = 0;
  virtual csTicks SetFeatherIn (csTicks featherin) = 0;
  virtual csTicks GetFeatherOut () = 0;
  virtual csTicks SetFeatherOut (csTicks featherout) = 0;
  // If in the blending layer then this is the mix blend factor.
  // Otherwise its unused. Clipped to 0 or above.
  virtual float GetBlend () = 0;
  virtual float SetBlend () = 0;
};

// an instance of an animation
struct iAmirSkeletonAnimation : public iAmirSkeletonAnimationNode
{
  SCF_INTERFACE (iAmirSkeletonAnimation, 0, 0, 0);

  // gotten from hidden factory interface
  virtual const char* GetName () = 0;

  // how many times this animation should repeat.
  // Is decremented each time a loop is finished.
  virtual int GetRepeat () = 0;
  virtual void SetRepeat (float repeat) = 0;
  // play speed
  virtual float GetSpeed () = 0;
  virtual void SetSpeed (float speed) = 0;
  // The name of the animation to sync with. Will just use the first
  // found animation if multiple matches are found. (stored as size_t id)
  virtual const char* GetSyncAnimation () = 0;
  virtual void SetSyncAnimation (const char* animname) = 0;

  // tsk tsk, i mean headstart
  // delay before the animation starts taking effect
  virtual csTicks GetDelay () = 0;
  virtual void SetDelay (csTicks delay) = 0;
};

// an instance of an animation
struct iAmirSkeletonAnimationFactory : public iBase
{
  SCF_INTERFACE (iAmirSkeletonAnimationFactory, 0, 0, 0);

  // a name and an array of these make up an animation factory
  struct KeyFrame
  {
  public:
    void SetTimePosition (csTicks t) { time = t; }
    void AddKeyFrameToBone (size_t boneid, const csQuaternion &rot,
      const csVector3 &pos)
    {
      BoneDescription d;
      d.rot = rot;
      d.pos = pos;
      keyed_bones.Push (d);
    }
  private:
    struct BoneDescription
    {
      size_t boneid;
      csQuaternion rot;
      csVector3 pos;
    };
    csArray<BoneDescription> keyed_bones;
    csTicks time;
  };
  // add keyframe
  virtual KeyFrame* CreateKeyFrame () = 0;

  virtual const char* GetName () = 0;
  virtual void SetName () = 0;
};

struct iAmirSkeletonAnimationMixerNode : public iAmirSkeletonAnimationNode
{
  SCF_INTERFACE (iAmirSkeletonAnimationMixerNode, 0, 0, 0);

  enum MixType
  {
    BLEND = 0,
    OVERWRITE
    //ADD eventually...
  };

  virtual MixType GetMixType () = 0;
  virtual MixType SetMixType (MixType mixtype) = 0;

  virtual csArray<iBase*> &GetAnimations ();
};

struct iAmirSkeleton : public virtual iBase
{
  SCF_INTERFACE (iAmirSkeleton, 0, 0, 0);

  // get root animation node. if its just a normal animation then it plays
  virtual iBase* GetAnimation ();

  virtual iAmirSkeletonFactory* GetFactory() = 0;
};

struct iAmirSkeletonFactory : public virtual iBase
{
  SCF_INTERFACE (iAmirSkeletonFactory, 0, 0, 0);

  // name is discarded in non-debug build
  virtual void CreateBone (const char *name) = 0;

  virtual iAmirSkeletonAnimation* FindAnimationByName (const char *name) = 0;

  virtual iAmirSkeleton* CreateSkeleton () = 0;
};

struct iAmirSkeletonGraveyard : public virtual iBase
{
  SCF_INTERFACE (iAmirSkeletonGraveyard, 0, 0, 0);

  virtual iAmirSkeletonFactory* CreateFactory () = 0;
  virtual iSkeletonFactory *FindFactory(const char *name) = 0;
};

#endif //__CS_IASKELETON_H__
