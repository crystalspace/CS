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

#include "cssysdef.h"

#include "csgeom/math3d.h"
#include "csutil/event.h"
#include "csutil/eventnames.h"
#include "csutil/util.h"
#include "imap/services.h"
#include "iutil/object.h"
#include "iutil/document.h"
#include "iutil/evdefs.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"

#include "skeleton.h"

CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton)
{

SCF_IMPLEMENT_FACTORY (SkeletonGraveyard)

csReversibleTransform &Bone::GetFullTransform ()
{
  return full_transform;
}

BoneFactory::BoneFactory ()
{
}
void BoneFactory::SetName (const char* n)
{
  name = n;
}
const char* BoneFactory::GetName ()
{
  return name.GetData ();
}

void AnimationFactory::SetName (const char* n)
{
  name = n;
}
const char* AnimationFactory::GetName () const
{
  return name.GetData ();
}
KeyFrame &AnimationFactory::CreateKeyFrame ()
{
  // extend array by 1 element
  keyframes.SetSize (keyframes.GetSize ());
  return keyframes.Top ();
}

Animation::Animation (const AnimationFactory *anim,
    const SkeletonAnimationParams &p)
  : anim (anim), p (p), current_time (0)
{
}

const SkeletonAnimationParams &Animation::GetParameters ()
{
  return p;
}

void Animation::Advance (csTicks elapsed)
{
  current_time += elapsed;
}

AnimationMixer::AnimationMixer ()
  : last_update_time (-1)
{
}

void AnimationMixer::UpdateMixer (csTicks current)
{
  if (last_update_time == -1)
  {
    last_update_time = current;
    return;
  }
  csTicks elapsed = current - last_update_time;
  last_update_time = current;

  if (elapsed)
  {
  }
}

void AnimationMixer::AddAnimation (const Animation &anim, 
      iNewSkeleton::MixLayerType mixtype)
{
  if (mixtype == iNewSkeleton::BLEND)
    blend.Push (anim);
  else
    overwrite.Push (anim);

  // recalculate the blend weights
  sum_blends = 0.0f;
  for (csArray<Animation>::Iterator it = blend.GetIterator ();
      it.HasNext (); )
  {
    const SkeletonAnimationParams &p = it.Next ().GetParameters ();
    sum_blends += p.blend;
  }
}

Skeleton::Skeleton ()
  :  scfImplementationType(this), fact (0)
{
}

void Skeleton::Update (csTicks current)
{
  UpdateMixer (current);
}

bool Skeleton::PlayAnimation (const char* anim, SkeletonAnimationParams params,
    MixLayerType mixtype)
{
  if (!fact)
    return false;
  for (csArray<AnimationFactory>::Iterator it = fact->GetAnimFactIterator ();
      it.HasNext (); )
  {
    const AnimationFactory &a = it.Next ();
    if (!strcmp (a.GetName (), anim))
    {
      Animation animinst (&a, params);
      return true;
    }
  }
  return false;
}

csReversibleTransform Skeleton::GetBoneTransformOffset (size_t boneid)
{
  csReversibleTransform trans;
  return trans;
}

SkeletonFactory::SkeletonFactory () : scfImplementationType(this)
{
}

void SkeletonFactory::CreateBone (const char* name)
{
}

void SkeletonFactory::CreateAnimation (const char* name,
    const csArray<SkeletonAnimationKeyFrame> &keyframes)
{
  // extend array by 1 element
  anims.SetSize (anims.GetSize ());
  AnimationFactory &animfact = anims.Top ();
  for (csArray<SkeletonAnimationKeyFrame>::ConstIterator it =
    keyframes.GetIterator (); it.HasNext ();)
  {
    const SkeletonAnimationKeyFrame &kf = it.Next ();
    KeyFrame &nkf = animfact.CreateKeyFrame ();
    nkf.time = kf.time;
    // we need to do this here so we can lookup the skeleton factory pointer
    for (csArray<SkeletonAnimationKeyFrame::BoneDescription>::ConstIterator bit 
      = kf.keyed_bones.GetIterator (); bit.HasNext ();)
    {
      const SkeletonAnimationKeyFrame::BoneDescription &bonedes = bit.Next ();
      KeyFrame::BoneDescription nbonedes;
      nbonedes.rot = bonedes.rot;
      nbonedes.pos = bonedes.pos;
      nbonedes.bone = 0;
      nkf.keyed_bones.Push (nbonedes);
    }
  }
}

BoneFactory* SkeletonFactory::FindBone (const char* name)
{
  for (csArray<BoneFactory>::Iterator it = bones.GetIterator (); it.HasNext ();)
  {
    BoneFactory &bonef = it.Next ();
    if (!strcmp (bonef.GetName (), name))
      return &bonef;
  }
  return 0;
}

const csArray<AnimationFactory>::Iterator SkeletonFactory::GetAnimFactIterator ()
{
  return anims.GetIterator ();
}

void SkeletonFactory::SetName (const char* n)
{
  name = n;
}
const char* SkeletonFactory::GetName ()
{
  return name.GetData ();
}

SkeletonGraveyard::SkeletonGraveyard (iBase* parent) :
  scfImplementationType (this, parent), object_reg (0)
{
}

SkeletonGraveyard::~SkeletonGraveyard ()
{
  skeletons.DeleteAll();
  if (object_reg && evhandler)
  {
    csRef<iEventQueue> q = csQueryRegistry<iEventQueue> (object_reg);
    if (q)
      q->RemoveListener (evhandler);
    evhandler = 0;
  }
}

bool SkeletonGraveyard::Initialize (iObjectRegistry* obj_reg)
{
  object_reg = obj_reg;
  vc = csQueryRegistry<iVirtualClock> (object_reg);
  preprocess = csevPreProcess (object_reg);
  csRef<iEventQueue> eq (csQueryRegistry<iEventQueue> (object_reg));
  if (eq == 0)
    return false;
  evhandler.AttachNew (new SkelEventHandler (this));
  eq->RegisterListener (evhandler, preprocess);

  return true;
}

void SkeletonGraveyard::Update (csTicks time)
{
  for (size_t i = 0; i < skeletons.GetSize () ; i++)
  {
  }
}
bool SkeletonGraveyard::HandleEvent (iEvent& ev)
{
  if (ev.Name == preprocess)
  {
    Update (vc->GetCurrentTicks ());
    return true;
  }
  return false;
}

iNewSkeletonFactory* SkeletonGraveyard::CreateSkeletonFactory ()
{
  csRef<SkeletonFactory> fact;
  fact.AttachNew(new SkeletonFactory ());
  factories.Push(fact);
  return fact;
}

}
CS_PLUGIN_NAMESPACE_END(Skeleton)
