/*
    Copyright (C) 2006 by Hristo Hristov

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

#ifndef __CS_SKELETON_H__
#define __CS_SKELETON_H__

#define __CS_SKELETAL_DEBUG

#include "csgeom/transfrm.h"
#include "csutil/array.h"
#include "csutil/csstring.h"
#include "csutil/refarr.h"
#include "csutil/scf_implementation.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/virtclk.h"
#include "imesh/skeleton_new.h"

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton)
{

class BoneFactory;
class SkeletonFactory;
class AnimationFactory;

class Bone
{
public:
  csReversibleTransform &GetFullTransform ();
private:
  BoneFactory* fact;
  Bone* parent;
  csArray<Bone*> children;

  // store relative (to parent bone) and local transform
  csReversibleTransform rel_transform, full_transform;
};
class BoneFactory
{
public:
  BoneFactory ();

  void SetName (const char* n);
  const char* GetName ();
private:
  csString name;
};

struct KeyFrame
{
  struct BoneDescription
  {
    BoneFactory* bone;
    csQuaternion rot;
    csVector3 pos;
  };
  csArray<BoneDescription> keyed_bones;
  csTicks time;
};
class Animation
{
public:
  Animation (const AnimationFactory *anim, const SkeletonAnimationParams &p);

  const SkeletonAnimationParams &GetParameters ();

  void Advance (csTicks elapsed);
private:
  const AnimationFactory* anim;
  SkeletonAnimationParams p;
  csTicks current_time;
};
class AnimationFactory
{
public:
  void SetName (const char* n);
  const char* GetName () const;

  KeyFrame &CreateKeyFrame ();
private:
  csString name;
  csArray<KeyFrame> keyframes;
};
class AnimationMixer
{
protected:
  AnimationMixer ();

  void UpdateMixer (csTicks current);

  void AddAnimation (const Animation &anim, 
      iNewSkeleton::MixLayerType mixtype);
private:
  // blend and overwrite mix layers
  csArray<Animation> blend, overwrite;
  // sum of blend weights from blend layer
  float sum_blends;
  // the time of the last update for this skeleton. is -1 on creation of this.
  long last_update_time;
};

class Skeleton : public scfImplementation1<Skeleton, iNewSkeleton>,
  public AnimationMixer
{
public:
  Skeleton ();

  void Update (csTicks current);

  virtual bool PlayAnimation (const char* anim, SkeletonAnimationParams params,
      MixLayerType mixtype);

  virtual csReversibleTransform GetBoneTransformOffset (size_t boneid);
private:
  csArray<Bone> bones;
  csArray<size_t> parent_bones;
  SkeletonFactory* fact;
};

class SkeletonFactory :
  public scfImplementation1<SkeletonFactory, iNewSkeletonFactory>
{
public:
  SkeletonFactory ();

  virtual void CreateBone (const char* name);

  virtual void CreateAnimation (const char* name,
      const csArray<SkeletonAnimationKeyFrame> &keyframes);

  BoneFactory* FindBone (const char* name);

  const csArray<AnimationFactory>::Iterator GetAnimFactIterator ();

  void SetName (const char* n);
  const char* GetName ();
private:
  csArray<BoneFactory> bones;
  csArray<size_t> parent_bones;
  csArray<AnimationFactory> anims;
  csString name;
};
class SkeletonGraveyard :
  public scfImplementation2<SkeletonGraveyard, iNewSkeletonGraveyard, iComponent>
{
public:
  SkeletonGraveyard (iBase *parent);
  virtual ~SkeletonGraveyard ();

  bool Initialize (iObjectRegistry* object_reg);
  bool HandleEvent (iEvent& ev);

  virtual iNewSkeletonFactory* CreateSkeletonFactory ();
private:

  class SkelEventHandler :
    public scfImplementation1<SkelEventHandler, iEventHandler>
  {
  private:
    SkeletonGraveyard* parent;

  public:
    SkelEventHandler (SkeletonGraveyard* parent)
      : scfImplementationType (this), parent (parent) { }
    virtual ~SkelEventHandler () { }
    virtual bool HandleEvent (iEvent& ev)
    {
      //return parent->HandleEvent (ev);
      return false;
    }

    CS_EVENTHANDLER_NAMES("crystalspace.skeleton.graveyard")
    CS_EVENTHANDLER_NIL_CONSTRAINTS
  };
  csRef<SkelEventHandler> evhandler;

  void Update (csTicks time);

  iObjectRegistry* object_reg;
  csRef<iVirtualClock> vc;
  csRefArray<Skeleton> skeletons;
  csRefArray<SkeletonFactory> factories;
  csEventID preprocess;
};

}
CS_PLUGIN_NAMESPACE_END(Skeleton)

#endif // __CS_SKELETON_H__
