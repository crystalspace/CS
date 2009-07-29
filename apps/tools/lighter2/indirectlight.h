/*
  Copyright (C) 2008 by Greg Hoffman
  Portions Copyright (C) 2009 by Seth Berrier

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

#ifndef __INDIRECTLIGHT_H__
#define __INDIRECTLIGHT_H__

#include "lightcomponent.h"

namespace lighter
{
  struct PhotonRay
  {
    // Origin (O)
    csVector3 origin;

    // Direction (normalized)
    csVector3 direction;

    // Power from emitting light
    csColor color, power;

    RayType type;

    lighter::Ray getRay() const
    {
      lighter::Ray asRay;
      asRay.origin = origin;
      asRay.direction = direction;
      asRay.minLength = 0.01f;
      asRay.type = type;
      return asRay;
    }
  };

  class IndirectLight : public LightComponent
  {
  public:
    // Setup
    IndirectLight ();
    ~IndirectLight ();

    virtual csColor ComputeElementLightingComponent(Sector* sector, 
      ElementProxy element, SamplerSequence<2>& lightSampler,
      bool recordInfluence);

    virtual csColor ComputePointLightingComponent(Sector* sector, 
      Object* obj, const csVector3& point, const csVector3& normal, 
      SamplerSequence<2>& lightSampler);

    /**
    * EmitPhotons
    * Emits photons in the given sector from the lights
    * /param sect - the sector to emit photons into 
    * /param progress - the progress we are making in calculations
    */
    void EmitPhotons(Sector *sect, Statistics::Progress& progress);

  private:
   /**
    * Emit Photon
    * Emits a photon in this sector and traces it through till its
    * termination using a monte-carlo based technique to determine
    * the termination.
    * /param pos - the position we are starting the trace from
    * /param dir - the direction we are tracing
    * /param color - the color of the photon
    * /param power - the power of the current photon
    * /param depth - how deep this call currently is
    * /param depth - is this photon a reflected photon or an initial emmision (default)
    */
    static void EmitPhoton(Sector* &sect, const PhotonRay &photon, const size_t &depth);

    /**
     * DiffuseScatter
     *    This funciton will compute a random direction in the hemisphere
     * around the vector n (which should be the surface normal at the hit
     * point).  The photon should be reflected in this direction to continue
     * photon tracing.
     * /param n - The surface normal at the phontons hit point
     **/
    static csVector3 DiffuseScatter(const csVector3 &n);


    csRandomVectorGen randVect;
    csRandomFloatGen randFloat;

    // The global lighter2 settings do not change so we 
    // cache their values locally to avoid multiple lookups
    bool finalGather;
    int numFinalGatherRays;
    float searchRadius;
    int numPhotonsPerSector;
    int numSamplesPerPhoton;
  };
}

#endif // __INDIRECTLIGHT_H__
