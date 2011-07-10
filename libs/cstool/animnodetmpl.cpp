/*
  Copyright (C) 2011 Christian Van Brussel, Institute of Information
      and Communication Technologies, Electronics and Applied Mathematics
      at Universite catholique de Louvain, Belgium
      http://www.uclouvain.be/en-icteam.html

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

#include "cssysdef.h"
#include "cstool/animnodetmpl.h"

using namespace CS::Animation;

// ------------------------------   csSkeletonAnimNodeFactory   ------------------------------

SkeletonAnimNodeFactory::SkeletonAnimNodeFactory (const char* name)
  : name (name)
{}

const char* SkeletonAnimNodeFactory::GetNodeName () const
{
  return name;
}

// ------------------------------   csSkeletonAnimNodeFactorySingle   ------------------------------

SkeletonAnimNodeFactorySingle::SkeletonAnimNodeFactorySingle (const char* name)
  : SkeletonAnimNodeFactory (name)
{}

void SkeletonAnimNodeFactorySingle::SetChildNode (iSkeletonAnimNodeFactory* factory)
{
  childNodeFactory = factory;
}

iSkeletonAnimNodeFactory* SkeletonAnimNodeFactorySingle::GetChildNode () const
{
  return childNodeFactory;
}

csPtr<iSkeletonAnimNode> SkeletonAnimNodeFactorySingle::CreateInstance (iSkeletonAnimPacket* packet,
									iSkeleton* skeleton)
{
  csRef<SkeletonAnimNodeSingleBase> newP (ActualCreateInstance (packet, skeleton));

  if (childNodeFactory)
  {
    csRef<CS::Animation::iSkeletonAnimNode> node =
      childNodeFactory->CreateInstance (packet, skeleton);
    newP->childNode = node;
  }

  return csPtr<CS::Animation::iSkeletonAnimNode> (newP);
}

iSkeletonAnimNodeFactory* SkeletonAnimNodeFactorySingle::FindNode (const char* name)
{
  if (this->name == name)
    return this;

  if (childNodeFactory)
    return childNodeFactory->FindNode (name);

  return nullptr;
}

// ------------------------------   csSkeletonAnimNodeSingle   ------------------------------

SkeletonAnimNodeSingleBase::SkeletonAnimNodeSingleBase (iSkeleton* skeleton)
  : skeleton (skeleton), isPlaying (false), playbackSpeed (1.0f)
{}

iSkeletonAnimNode* SkeletonAnimNodeSingleBase::GetChildNode () const
{
  return childNode;
}

void SkeletonAnimNodeSingleBase::Play ()
{
  if (isPlaying)
    return;

  if (childNode)
    childNode->Play ();

  isPlaying = true;
}

void SkeletonAnimNodeSingleBase::Stop ()
{
  if (!isPlaying)
    return;

  if (childNode)
    childNode->Stop ();

  isPlaying = false;
}

void SkeletonAnimNodeSingleBase::SetPlaybackPosition (float time)
{
  if (childNode)
    childNode->SetPlaybackPosition (time);
}

float SkeletonAnimNodeSingleBase::GetPlaybackPosition () const
{
  if (childNode)
    return childNode->GetPlaybackPosition ();

  return 0.0f;
}

float SkeletonAnimNodeSingleBase::GetDuration () const
{
  if (childNode)
    return childNode->GetDuration ();

  return 0.0f;
}

void SkeletonAnimNodeSingleBase::SetPlaybackSpeed (float speed)
{
  playbackSpeed = speed;

  if (childNode)
    childNode->SetPlaybackSpeed (speed);
}

float SkeletonAnimNodeSingleBase::GetPlaybackSpeed () const
{
  return playbackSpeed;
}

void SkeletonAnimNodeSingleBase::BlendState (AnimatedMeshState* state,
					   float baseWeight)
{
  if (childNode)
    childNode->BlendState (state, baseWeight);
}

void SkeletonAnimNodeSingleBase::TickAnimation (float dt)
{
  if (childNode)
    childNode->TickAnimation (dt);
}

bool SkeletonAnimNodeSingleBase::IsActive () const
{
  return isPlaying;
}

void SkeletonAnimNodeSingleBase::AddAnimationCallback (iSkeletonAnimCallback* callback)
{
  if (childNode)
    childNode->AddAnimationCallback (callback);
}

void SkeletonAnimNodeSingleBase::RemoveAnimationCallback (iSkeletonAnimCallback* callback)
{
  if (childNode)
    childNode->RemoveAnimationCallback (callback);
}

// ------------------------------   SkeletonAnimNodeFactoryMulti   ------------------------------

SkeletonAnimNodeFactoryMulti::SkeletonAnimNodeFactoryMulti (const char* name)
  : SkeletonAnimNodeFactory (name)
{}

void SkeletonAnimNodeFactoryMulti::AddChildNode (iSkeletonAnimNodeFactory* factory)
{
  childNodeFactories.Push (factory);
}

void SkeletonAnimNodeFactoryMulti::RemoveChildNode (iSkeletonAnimNodeFactory* factory)
{
  childNodeFactories.Delete (factory);
}

void SkeletonAnimNodeFactoryMulti::ClearChildNodes ()
{
  childNodeFactories.DeleteAll ();
}

iSkeletonAnimNodeFactory* SkeletonAnimNodeFactoryMulti::GetChildNode (size_t index) const
{
  CS_ASSERT (index < childNodeFactories.GetSize ());

  return childNodeFactories[index];
}

