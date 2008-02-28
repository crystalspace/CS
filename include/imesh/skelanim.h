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

#ifndef __CS_INSKELETONANIMATION_H__
#define __CS_INSKELETONANIMATION_H__

#include "csgeom/quaternion.h"
#include "csgeom/vector3.h"
#include "csutil/hash.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "csutil/strset.h"
#include "csutil/tuple.h"

namespace Skeleton
{
struct iSkeleton;

namespace Animation
{

struct iAnimation;

typedef csTuple2<csQuaternion, csVector3> Keyframe;
typedef csHash<Keyframe, size_t> Frame;

struct iChannel : public virtual iBase
{
  SCF_INTERFACE (iChannel, 0, 0, 1);
  virtual void AddKeyframe (float t, const Keyframe &k) = 0;
  virtual Keyframe ReadValue (float i) const = 0;
  virtual void SetID (size_t id) = 0;
  virtual size_t GetID () const = 0;
};
struct iAnimationFactory : public virtual iBase
{
  SCF_INTERFACE (iAnimationFactory, 0, 0, 1);
  virtual csPtr<iAnimation> CreateAnimation () = 0;
  virtual void SetName (const char* n) = 0;
  virtual const char* GetName () const = 0;
  virtual void AddChannel (csRef<iChannel> channel) = 0;
  virtual void ReadChannels (float time, Frame &frame) const = 0;
  virtual void SetAnimationLength (float length) = 0;
  virtual float GetAnimationLength () const = 0;
};
struct iAnimation : public virtual iBase
{
  SCF_INTERFACE (iAnimation, 0, 0, 1);
  virtual void Tick (float amount) = 0;
  virtual void ReadChannels (Frame &frame) = 0;
  virtual void SetPlaySpeed (float speed) = 0;
  virtual void SetPlayCount (int count) = 0;
  virtual int GetPlayCount () const = 0;
};

struct iMixingNode : public virtual iBase
{
  SCF_INTERFACE (iMixingNode, 0, 0, 1);
  virtual void Tick (float amount) = 0;
  virtual void ReadChannels (Frame &frame) = 0;
  virtual bool IsActive () const = 0;
};

struct iBlendNode : public virtual iBase
{
  SCF_INTERFACE (iBlendNode, 0, 0, 1);
  virtual void Tick (float amount) = 0;
  virtual void ReadChannels (Frame &frame) = 0;
  virtual void AddNode (float weight, csRef<iMixingNode> node) = 0;
};

struct iOverwriteNode : public virtual iBase
{
  SCF_INTERFACE (iOverwriteNode, 0, 0, 1);
  virtual void Tick (float amount) = 0;
  virtual void ReadChannels (Frame &frame) = 0;
  virtual void AddNode (csRef<iMixingNode> node) = 0;
};

struct iAnimationLayer : public virtual iBase
{
  SCF_INTERFACE (iAnimationLayer, 0, 0, 1);
  virtual csRef<iMixingNode> GetRootMixingNode () = 0;
  virtual void SetRootMixingNode (csRef<iMixingNode> root) = 0;
  virtual void UpdateSkeleton (Skeleton::iSkeleton *s, float delta_time) = 0;

  virtual csPtr<iBlendNode> CreateBlendNode () = 0;
  virtual csPtr<iOverwriteNode> CreateOverwriteNode () = 0;
};

struct iAnimationFactoryLayer : public virtual iBase
{
  SCF_INTERFACE (iAnimationFactoryLayer, 0, 0, 1);
  virtual iAnimationFactory* CreateAnimationFactory () = 0;
  virtual csPtr<iChannel> CreateAnimationFactoryChannel () = 0;
  virtual iAnimationFactory* FindAnimationFactoryByName (const char* name) = 0;

  /// @@ GENJIX @@
  virtual void Debug () = 0;
};

}
}

#endif //__CS_INSKELETONANIMATION_H__
