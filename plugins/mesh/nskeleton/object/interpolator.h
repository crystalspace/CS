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

#ifndef __CS_INTERPOLATOR_H__
#define __CS_INTERPOLATOR_H__

#include "imesh/skelanim.h"

using Skeleton::Animation::Keyframe;

template <typename Value>
class csInterpolator
{
public:
  static Value Linear (float i, float ta, const Value &a, float tb,
    const Value &b)
  {
    return (i - ta) / (tb - ta) * (b - a) + a;
  }
};

template <>
class csInterpolator<Keyframe>
{
public:
  static Keyframe Linear (float i, float ta, const Keyframe &a, float tb,
    const Keyframe &b)
  {
    return Keyframe (
      a.first.SLerp (b.first, (i - ta) / (tb - ta)),
      csInterpolator <csVector3>::Linear (i, ta, a.second, tb, b.second));
  }
  static Keyframe Linear (const csArray<csTuple2 <float, Keyframe> > &keyframes)
  {
    Keyframe result;
    float accum_weight = 0.0f;
    result.second.Set (0.0f);
    for (csArray<csTuple2 <float, Keyframe> >::ConstIterator it =
      keyframes.GetIterator (); it.HasNext (); )
    {
      const csTuple2<float, Keyframe> &keyf = it.Next ();
      float keyf_weight = keyf.first,
        keyfw = keyf_weight / (keyf_weight + accum_weight);
      accum_weight += keyf_weight;
      result = Keyframe (result.first.SLerp (keyf.second.first, keyfw),
        keyfw * (keyf.second.second - result.second) + result.second);
    }
    return result;
  }
};

#endif // __CS_NSKELETONANIMATION_H__
