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
  // we're going to find the closest times on both sides of i
  float close_lower = i, close_higher = i;
  // these are the keyframes for the times
  Keyframe low, up;
  // loop through available keyframes in this channel
  for (csHash<Keyframe, float>::ConstGlobalIterator it =
    keyframes.GetIterator (); it.HasNext (); )
  {
    float timekey;
    const Keyframe &key = it.Next (timekey);
    // we have perfect match. return this keyframe.
    if (ABS(timekey - i) < SMALL_EPSILON)
      return key;
    else if (timekey < i)
    {
      // if our lower bound value matches the time we seek (hasn't been set yet)
      //   OR we found a closer value to i
      if (ABS(close_lower - i) < SMALL_EPSILON || timekey > close_lower)
      {
        // set the lower time value and corresponding keyframe
        close_lower = timekey;
        low = key;
      }
    }
    else if (timekey > i)
    {
      // if our upper bound value matches the time we seek (hasn't been set yet)
      //   OR we found a closer value to i
      if (ABS(close_higher - i) < SMALL_EPSILON || timekey < close_higher)
      {
        // set the upper time value and corresponding keyframe
        close_higher = timekey;
        up = key;
      }
    }
  }

  // why does this sexy code stop the ghosts?
  if (ABS(close_lower - i) < SMALL_EPSILON)
    return up;
  else if (ABS(close_higher - i) < SMALL_EPSILON)
    return low;
  return  // intarpolate!
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
  // aggregate all the channels output into a frame and return taht
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
const char* Animation::GetName () const
{
  return fact->GetName ();
}
void Animation::Tick (float amount)
{
  // if animation is playing
  if (playcount != 0) // tick!
    timeline += amount * playspeed;
}
void Animation::ReadChannels (Frame &frame)
{
  // do nothing if animation isn't playing
  if (!playcount)
    return;
  // this is an empty shell, so request channels
  // from the factory :P
  fact->ReadChannels (timeline, frame);

  // reset timeline if it's gone past end of animation
  if (timeline > fact->GetAnimationLength ())
  {
    timeline = 0;
    if (playcount > 0)
      playcount--;
  }
  // for backwards playback
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
float Animation::GetPlaySpeed ()
{
  return playspeed;
}
void Animation::SetPlayCount (int count)
{
  playcount = count;
}
int Animation::GetPlayCount () const
{
  return playcount;
}
float Animation::GetAnimationLength () const
{
  return fact->GetAnimationLength ();
}
float Animation::GetTimeline () const
{
  return timeline;
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
  // tick all the child nodes. should end with the animations (leaf) ticking.
  for (csRefArray<iMixingNode>::Iterator it = nodes.GetIterator ();
    it.HasNext (); )
  {
    iMixingNode* node = it.Next ();
    if (!node->IsActive ())
    {
      //nodes.Delete (node);
      continue;
    }
    node->Tick (amount);
  }
}
void BlendNode::CalculateFramesAndChannels (csArray<Frame> &nodes_frames, csArray<size_t> &channel_ids)
{
  // loop through all child nodes and add their output in a list
  for (csRefArray<iMixingNode>::Iterator it = nodes.GetIterator ();
    it.HasNext (); )
  {
    iMixingNode* node = it.Next ();
    Frame frame;
    node->ReadChannels (frame);
    nodes_frames.Push (frame);
  }
  // now go through that list and build another list of all the existing channels
  for (csArray<Frame>::Iterator it = nodes_frames.GetIterator (); it.HasNext (); )
  {
    const Frame &frame = it.Next ();
    for (Frame::ConstGlobalIterator it = frame.GetIterator ();
      it.HasNext (); )
    {
      size_t id;
      it.Next (id);
      if (channel_ids.Find (id) == csArrayItemNotFound)
        channel_ids.Push (id);  // 404 - add to our list
    }
  }
}
void BlendNode::ReadChannels (Frame &result_frame)
{
  // read all the frames from all the child nodes
  csArray<Frame> nodes_frames;
  // comprehensive list of all channel ids used
  csArray<size_t> channel_ids;
  CalculateFramesAndChannels (nodes_frames, channel_ids);
  // go through each channel id
  for (csArray<size_t>::Iterator it = channel_ids.GetIterator ();
    it.HasNext (); )
  {
    const size_t id = it.Next ();
    // the intarpolatar likes to have weights with keyframes
    csArray<csTuple2<float, Keyframe> > keys;
    // find this channel in each frame and add it to keys
    for (size_t i = 0; i < nodes_frames.GetSize (); i++)
    {
      const Frame &frame = nodes_frames.Get (i);
      const Keyframe* keyf = frame.GetElementPointer (id);
      if (keyf)  // it exists! - intarpolatar can handle non normalised weights
        keys.Push (csTuple2<float, Keyframe> (blend_weights.Get (i), *keyf));
    }
    // now calculate the snapshot for this channel and add it to the output
    const Keyframe &in = csInterpolator<Keyframe>::Linear (keys);
    result_frame.PutUnique (id, in);
  }
}
size_t BlendNode::AddNode (float weight, csRef<iMixingNode> node)
{
  if (weight < 0.0f)
    weight = 0.0f;
  nodes.Push (node);
  blend_weights.Push (weight);
  return nodes.GetSize () - 1;    // return position of newly added child
}
void BlendNode::SetWeight (size_t i, float weight)
{
  if (weight < 0.0f)
    weight = 0.0f;
  blend_weights[i] = weight;
}
bool BlendNode::IsActive () const
{
  return true;    // always active
}

void AccumulateNode::ReadChannels (Frame &result_frame)
{
  // read all the frames from all the child nodes
  csArray<Frame> nodes_frames;
  // comprehensive list of all channel ids used
  csArray<size_t> channel_ids;
  CalculateFramesAndChannels (nodes_frames, channel_ids);
  // for each channel...
  for (csArray<size_t>::Iterator it = channel_ids.GetIterator ();
    it.HasNext (); )
  {
    const size_t id = it.Next ();
    // the intarpolatar likes to have weights with keyframes
    csArray<csTuple2<float, Keyframe> > keys;
    // this node works on cumulative weight so we need to keep track of that
    float cumul_weights = 0.0f;
    // find this channel in each frame and add it to keys
    // go from top as last added node is strongest
    for (int i = nodes_frames.GetSize () - 1; i >= 0; i--)
    {
      const Frame &frame = nodes_frames.Get (i);
      const Keyframe* keyf = frame.GetElementPointer (id);
      if (keyf)
      {
        // a bit of arithmetic.
        float w = blend_weights.Get (i),
          excess = 1.0f - cumul_weights,
          flat_w = w * excess;
        cumul_weights += flat_w;
        keys.Push (csTuple2<float, Keyframe> (flat_w, *keyf));
      }
    }
    const Keyframe &in = csInterpolator<Keyframe>::Linear (keys);
    result_frame.PutUnique (id, in);
  }
}

size_t AccumulateNode::AddNode (float weight, csRef<iMixingNode> node)
{
  // if we don't clamp the upper then life gets a bit difficult...
  if (weight > 1.0f)
    weight = 1.0f;
  return BlendNode::AddNode (weight, node);
}
void AccumulateNode::SetWeight (size_t i, float weight)
{
  if (weight > 1.0f)
    weight = 1.0f;
  BlendNode::SetWeight (i, weight);
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
  if (!mix_node)
    return;
  if (!s)
    return;
  mix_node->Tick (delta_time);
  Frame frame;
  // read output and apply it to the skeleton
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
csPtr<iBlendNode> AnimationLayer::CreateAccumulateNode ()
{
  return new AccumulateNode ();
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
  // loop through animation factories until we find what we want
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
