/*
  Copyright (C) 2006 by Marten Svanfeldt

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

#ifndef __DIRECTLIGHT_H__
#define __DIRECTLIGHT_H__

#include "csutil/noncopyable.h"

#include "primitive.h"
#include "sampler.h"

namespace lighter
{
  class Sector;
  class Raytracer;
  class Primitive;
  class Light_old;
  class Light;
  
  // Class to calculate direct lighting
  class DirectLighting : private CS::NonCopyable
  {
  public:
    // Setup
    static void Initialize ();

    // Shade by using all primitives within range
    static void ShadeDirectLighting (Sector* sector, float progressStep);

    //-- Shade a point
    typedef csColor (*PVLPointShader)(Sector* sector, 
      const csVector3& point, const csVector3& normal, 
      SamplerSequence<2>& lightSampler, Raytracer& rt);

    // Shade a single point in space with direct lighting
    static csColor UniformShadeAllLightsNonPD (Sector* sector, const csVector3& point,
      const csVector3& normal, SamplerSequence<2>& lightSampler, Raytracer& rt);

    // Shade a single point in space with direct lighting using a single light
    static csColor UniformShadeRndLightNonPD (Sector* sector, const csVector3& point,
      const csVector3& normal, SamplerSequence<2>& lightSampler, Raytracer& rt);

    //-- Shade a lightmap element
    typedef csColor (*LMElementShader)(Sector* sector, ElementProxy element,
      SamplerSequence<2>& lightSampler, Raytracer& rt);

    // Shade a primitive element with direct lighting
    static csColor UniformShadeAllLightsNonPD (Sector* sector, ElementProxy element,
      SamplerSequence<2>& lightSampler, Raytracer& rt);

    // Shade a primitive element with direct lighting using a single light
    static csColor UniformShadeRndLightNonPD (Sector* sector, ElementProxy element,
      SamplerSequence<2>& lightSampler, Raytracer& rt);


    //-- Shade using one light
    // Shade a primitive element with direct lighting
    static csColor UniformShadeOneLight (Sector* sector, const csVector3& point,
      const csVector3& normal, Light* light, SamplerSequence<2>& lightSampler, 
      Raytracer& rt);

    // Shade a primitive element with direct lighting
    static csColor UniformShadeOneLight (Sector* sector, ElementProxy element,
      Light* light, SamplerSequence<2>& lightSampler, Raytracer& rt);

  private:
    // Static methods...
    inline static csColor ShadeLight (Light* light, const csVector3& point,
      const csVector3& normal, Raytracer& rt, SamplerSequence<2>& lightSampler,
      const Primitive* shadowIgnorePrimitive = 0);

    static void ShadeLightmap (Sector* sector, Object* obj, Raytracer& rt, 
      SamplerSequence<2>& masterSampler);

    static void ShadePerVertex (Sector* sector, Object* obj, Raytracer& rt,
      SamplerSequence<2>& masterSampler);

    // Static data
    static PVLPointShader pvlPointShader;
    static LMElementShader lmElementShader;
  };
}

#endif
