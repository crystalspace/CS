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

#include "imesh/nskeleton.h"

#include "animation.h"

CS_PLUGIN_NAMESPACE_BEGIN(SkeletonAnimation)
{

using namespace Skeleton::Animation;

Channel::Channel () : scfImplementationType (this)
{
}
void Channel::AddKeyframe (float t, const Keyframe &k)
{
  keyframes.PutUnique (t, k);
}
Keyframe Channel::ReadValue (float i) const
{
  float close_lower = i, close_higher = i;
  Keyframe low, up;
  for (csHash<Keyframe, float>::ConstGlobalIterator it =
    keyframes.GetIterator (); it.HasNext (); )
  {
    float timekey;
    const Keyframe &key = it.Next (timekey);
    if (timekey == i)
      return key;
    else if (timekey < i)
    {
      if (close_lower == i || timekey > close_lower)
      {
        close_lower = timekey;
        low = key;
      }
    }
    else if (timekey > i)
    {
      if (close_higher == i || timekey < close_higher)
      {
        close_higher = timekey;
        up = key;
      }
    }
  }

  if (close_lower == i)
    return up;
  else if (close_higher == i)
    return low;
  return
    csInterpolator<Keyframe>::Linear (i, close_lower, low, close_higher, up);
}
void Channel::SetID (size_t id)
{
  ident = id;
}
size_t Channel::GetID () const
{
  return ident;
}

AnimationFactory::AnimationFactory ()
  : scfImplementationType (this), animlength (0)
{
}
csPtr<iAnimation> AnimationFactory::CreateAnimation ()
{
  return new Animation (csRef<AnimationFactory> (this));
}
void AnimationFactory::SetName (const char* n)
{
  name = n;
}
const char* AnimationFactory::GetName () const
{
  return name;
}
void AnimationFactory::AddChannel (csRef<iChannel> channel)
{
  channels.Push (channel);
}
void AnimationFactory::ReadChannels (float time, Frame &frame) const
{
  for (csRefArray<iChannel>::ConstIterator it =
    channels.GetIterator (); it.HasNext (); )
  {
    const iChannel* channel = it.Next ();
    frame.PutUnique (channel->GetID (), channel->ReadValue (time));
  }
}
void AnimationFactory::SetAnimationLength (float length)
{
  animlength = length;
}
float AnimationFactory::GetAnimationLength () const
{
  return animlength;
}

Animation::Animation (csRef<iAnimationFactory> fact)
  : scfImplementationType (this), fact (fact), timeline (0), playspeed (1.0f),
  playcount (1)
{
}
void Animation::Tick (float amount)
{
  timeline += amount * playspeed;
  printf ("%f\n", timeline);
}
void Animation::ReadChannels (Frame &frame)
{
  if (!playcount)
    return;
  fact->ReadChannels (timeline, frame);
  if (timeline > fact->GetAnimationLength ())
  {
    timeline = 0;
    if (playcount > 0)
      playcount--;
  }
  else if (timeline < 0)
  {
    timeline = fact->GetAnimationLength ();
    if (playcount > 0)
      playcount--;
  }
}
void Animation::SetPlaySpeed (float speed)
{
  playspeed = speed;
}
void Animation::SetPlayCount (int count)
{
  playcount = count;
}
int Animation::GetPlayCount () const
{
  return playcount;
}
bool Animation::IsActive () const
{
  return playcount != 0;
}

BlendNode::BlendNode () : scfImplementationType (this)
{
}
void BlendNode::Tick (float amount)
{
  for (csRefArray<iMixingNode>::Iterator it = nodes.GetIterator ();
    it.HasNext (); )
  {
    iMixingNode* node = it.Next ();
    if (!node->IsActive ())
    {
      nodes.Delete (node);
      continue;
    }
    node->Tick (amount);
  }
}
void BlendNode::ReadChannels (Frame &result_frame)
{
  csArray<Frame> nodes_frames;
  for (csRefArray<iMixingNode>::Iterator it = nodes.GetIterator ();
    it.HasNext (); )
  {
    iMixingNode* node = it.Next ();
    Frame frame;
    node->ReadChannels (frame);
    nodes_frames.Push (frame);
  }
  csArray<size_t> channel_ids;
  for (csArray<Frame>::Iterator it = nodes_frames.GetIterator (); it.HasNext (); )
  {
    const Frame &frame = it.Next ();
    for (Frame::ConstGlobalIterator it = frame.GetIterator ();
      it.HasNext (); )
    {
      size_t id;
      it.Next (id);
      if (channel_ids.Find (id) == csArrayItemNotFound)
        channel_ids.Push (id);
    }
  }
  for (csArray<size_t>::Iterator it = channel_ids.GetIterator ();
    it.HasNext (); )
  {
    const size_t id = it.Next ();
    csArray<csTuple2<float, Keyframe> > keys;
    // find this channel in each frame and add it to the list
    for (size_t i = 0; i < nodes_frames.GetSize (); i++)
    {
      const Frame &frame = nodes_frames.Get (i);
      const Keyframe* keyf = frame.GetElementPointer (id);
      if (keyf)
        keys.Push (csTuple2<float, Keyframe> (blend_weights.Get (i), *keyf));
    }
    const Keyframe &in = csInterpolator<Keyframe>::Linear (keys);
    result_frame.PutUnique (id, in);
  }
}
void BlendNode::AddNode (float weight, csRef<iMixingNode> node)
{
  nodes.Push (node);
  blend_weights.Push (weight);
}
bool BlendNode::IsActive () const
{
  return true;
}

OverwriteNode::OverwriteNode () : scfImplementationType (this)
{
}
void OverwriteNode::Tick (float amount)
{
  for (csRefArray<iMixingNode>::Iterator it = nodes.GetIterator ();
    it.HasNext (); )
  {
    iMixingNode* node = it.Next ();
    if (!node->IsActive ())
    {
      nodes.Delete (node);
      continue;
    }
    node->Tick (amount);
  }
}
void OverwriteNode::ReadChannels (Frame &frame)
{
  for (csRefArray<iMixingNode>::Iterator it = nodes.GetIterator ();
    it.HasNext (); )
  {
    iMixingNode* node = it.Next ();
    node->ReadChannels (frame);
  }
}
void OverwriteNode::AddNode (csRef<iMixingNode> node)
{
  nodes.Push (node);
}
bool OverwriteNode::IsActive () const
{
  return true;
}

AnimationLayer::AnimationLayer () : scfImplementationType (this)
{
}
csRef<iMixingNode> AnimationLayer::GetRootMixingNode ()
{
  return mix_node;
}
void AnimationLayer::SetRootMixingNode (csRef<iMixingNode> root)
{
  mix_node = root;
}
void AnimationLayer::UpdateSkeleton (Skeleton::iSkeleton *s, float delta_time)
{
  // blaa!
  if (mix_node)
    mix_node->Tick (delta_time);
  if (!s)
    return;
  Frame frame;
  mix_node->ReadChannels (frame);
  for (size_t i = 0; i < s->GetChildrenCount (); i++)
  {
    const Keyframe* key = frame.GetElementPointer (i);
    if (key)
    {
      Skeleton::iSkeleton::iBone* b = s->GetChild (i);
      b->SetRotation (key->first);
      b->SetPosition (key->second);
      //csQuaternion q (key->first);
      //csVector3 v (key->second);
      //printf ("'%s' (%f, %f, %f, %f)   (%s)\n", b->GetFactory ()->GetName (), q.v.x, q.v.y, q.v.z, q.w, v.Description ().GetData ());
    }
  }
}

csPtr<iBlendNode> AnimationLayer::CreateBlendNode ()
{
  return new BlendNode ();
}
csPtr<iOverwriteNode> AnimationLayer::CreateOverwriteNode ()
{
  return new OverwriteNode ();
}

AnimationFactoryLayer::AnimationFactoryLayer () : scfImplementationType (this)
{
}
iAnimationFactory* AnimationFactoryLayer::CreateAnimationFactory ()
{
  csRef<AnimationFactory> fact;
  fact.AttachNew (new AnimationFactory ());
  animfacts.Push (fact);
  return fact;
}
csPtr<iChannel> AnimationFactoryLayer::CreateAnimationFactoryChannel ()
{
  return new Channel ();
}
iAnimationFactory* AnimationFactoryLayer::FindAnimationFactoryByName (
  const char* name)
{
  for (csRefArray<AnimationFactory>::Iterator it = animfacts.GetIterator ();
    it.HasNext (); )
  {
    AnimationFactory* animfact = it.Next ();
    if (!strcmp (animfact->GetName (), name))
      return animfact;
  }
  return 0;
}

void AnimationFactoryLayer::Debug ()
{
  puts ("AnimationFactoryLayer");
  for (csRefArray<AnimationFactory>::Iterator it = animfacts.GetIterator ();
    it.HasNext (); )
  {
    AnimationFactory* animfact = it.Next ();
    printf ("  Animation: %s\n", animfact->GetName ());
  }
}

}
CS_PLUGIN_NAMESPACE_END(SkeletonAnimation)
