/*
  Copyright (C) 2009 by Seth Berrier

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

#ifndef __LIGHTCOMPONENT_H__
#define __LIGHTCOMPONENT_H__

#include "csutil/noncopyable.h"

#include "sampler.h"
#include "primitive.h"

namespace lighter
{
  class Sector;
  class Object;
  class Light;

  // Class to calculate direct lighting
  class LightComponent : private CS::NonCopyable
  {
  public:
    // Setup
    LightComponent ();
    ~LightComponent ();

    bool SupportsPDLights ();

    void resizeAffectingLights( size_t newSize );
    void setAffectingLight(size_t idx, bool effects);

    virtual csColor ComputeElementLightingComponent(Sector* sector, 
      ElementProxy element, SamplerSequence<2>& lightSampler,
      bool recordInfluence) = 0;

    virtual csColor ComputePointLightingComponent(Sector* sector, 
      Object* obj, const csVector3& point, const csVector3& normal, 
      SamplerSequence<2>& lightSampler) = 0;

    virtual csColor ComputeElementLightingComponent (Sector* sector,
      ElementProxy element, SamplerSequence<2>& lightSampler,
      bool recordInfluence, Light* light);

    virtual csColor ComputePointLightingComponent(Sector* sector, 
      Object* obj, const csVector3& point, const csVector3& normal, 
      SamplerSequence<2>& lightSampler, Light* light);

  protected:
    bool PDLightsSupported;

    // Boolean array of which lights are important
    csBitArray affectingLights;
  };
}

#endif // __LIGHTCOMPONENT_H__
