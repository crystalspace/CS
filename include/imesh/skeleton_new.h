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

struct SkeletonAnimationParams
{
  SkeletonAnimationParams () : feather_in (0), feather_out (0), blend (1.0f),
    repeat (1), speed (1.0f), delay (0) { }

  // The blend is linearly increased from 0 to 1 (after feather_in), and then
  // starts decreasing back to 0 when feather_out ticks is left in the animation.
  csTicks feather_in, feather_out;
  // If in the blending layer then this is the mix blend factor.
  // Otherwise its unused. Clipped to 0 or above.
  float blend;
  // how many times this animation should repeat.
  // Is decremented each time a loop is finished.
  int repeat;
  // play speed
  float speed;
  // The name of the animation to sync with. Will just use the first
  // found animation if multiple matches are found.
  csString sync;
  // delay before the animation starts taking effect
  csTicks delay;
};

struct SkeletonAnimationKeyFrame
{
  struct BoneDescription
  {
    csString bone_name;
    csQuaternion rot;
    csVector3 pos;
  };
  csArray<BoneDescription> keyed_bones;
  csTicks time;
};

struct iNewSkeleton : public virtual iBase
{
  SCF_INTERFACE (iNewSkeleton, 0, 0, 0);

  enum MixLayerType
  {
    BLEND,
    OVERWRITE,
  };

  virtual bool PlayAnimation (const char* anim, SkeletonAnimationParams params,
      MixLayerType mixtype) = 0;

  virtual csReversibleTransform GetBoneTransformOffset (size_t boneid) = 0;

  virtual void CreateAnimation (const char* name,
      const csArray<SkeletonAnimationKeyFrame> &keyframes) = 0;
};

struct iNewSkeletonFactory : public virtual iBase
{
  SCF_INTERFACE (iNewSkeletonFactory, 0, 0, 0);

  virtual void CreateBone (const char *name) = 0;
};

struct iNewSkeletonGraveyard : public virtual iBase
{
  SCF_INTERFACE (iNewSkeletonGraveyard, 0, 0, 0);

  virtual iNewSkeletonFactory* CreateSkeletonFactory () = 0;
};

#endif //__CS_IASKELETON_H__
