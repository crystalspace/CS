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

#ifndef __LIGHTCALCULATOR_H__
#define __LIGHTCALCULATOR_H__

#include "csutil/noncopyable.h"

#include "sampler.h"
#include "statistics.h"

#include <vector>
using namespace std;

namespace lighter
{
  // Forward declarations to avoid unnecessary includes
  class LightComponent;
  class Sector;
  class Object;
  
  // Class to calculate direct lighting
  class LightCalculator : private CS::NonCopyable
  {
  public:
    // Constructor & Destructor
    LightCalculator (const csVector3& tangentSpaceNorm, size_t subLightmapNum);
    ~LightCalculator ();

    void addComponent(LightComponent* newComponent, float scaler = 1.0, float offset = 0.0);

    // Loop through all objects in a sector and calculate the
    // static light using all attached LightComponents
    void ComputeSectorStaticLighting (Sector* sector, 
            Statistics::Progress& progress);

  private:
    void ComputeObjectStaticLightingForLightmap (Sector* sector,
        Object* obj, SamplerSequence<2>& masterSampler,
        Statistics::ProgressState& progress);

    void ComputeObjectStaticLightingForVertex (Sector* sector,
        Object* obj, SamplerSequence<2>& masterSampler,
        Statistics::ProgressState& progress);

    void ComputeAffectingLights (Object* obj);

    csVector3 ComputeVertexNormal (Object* obj, size_t index) const;

    // The list of lighting component objects and their coefficients & offsets
    vector<float> componentCoefficient, componentOffset;
    vector<LightComponent*> component;

    csVector3 tangentSpaceNorm;
    bool fancyTangentSpaceNorm;

    size_t subLightmapNum;
//    csBitArray affectingLights;
  };
}

#endif // __LIGHTCALCULATOR_H__
