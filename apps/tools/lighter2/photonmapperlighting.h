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

    // Pointer to light that generated this photon
    Light* source;

    RayType type;

    // Refractive index of the material the photon is currently travelling in
    float refrIndex;

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

  class PhotonmapperLighting : public LightComponent
  {
  public:
    // Setup
    PhotonmapperLighting ();
    ~PhotonmapperLighting ();

    virtual csColor ComputeElementLightingComponent(Sector* sector, 
      ElementProxy element, SamplerSequence<2>& lightSampler,
      bool recordInfluence);

    virtual csColor ComputePointLightingComponent(Sector* sector, 
      Object* obj, const csVector3& point, const csVector3& normal, 
      SamplerSequence<2>& lightSampler);

    /**
     * SetPhotonStorage
     * Set weather direct and indirect photons should be stored
     * /param storeDirectPhotons - Directly emitted photons should be stored 
     * /param storeIndirectPhotons - Scattered photons should be stored
     */
    void SetPhotonStorage(bool storeDirectPhotons,
      bool storeIndirectPhotons);

    /**
    * EmitPhotons
    * Emits photons in the given sector from the lights
    * /param sect - the sector to emit photons into 
    * /param progress - the progress we are making in calculations
    */
    void EmitPhotons(Sector *sect, Statistics::Progress& progress);

    /**
    * BalancePhotons
    * Balance the photon map KD-Tree for the indicated sector
    * /param sect - the sector to balance 
    * /param progress - the progress we are making in calculations
    */
    void BalancePhotons(Sector *sect, Statistics::Progress& progress);

    /**
    * ParseSector
    * Parse the sector for meshes which can produce caustics add them to a list
    * returns whether to emit caustic photons or not
    * /param sect - the sector to be parsed
    */
    bool ParseSector(Sector *sect);

  private:
   /**
    * Emit Photon
    * Emits a photon in this sector and traces it through till its
    * termination using a monte-carlo based technique to determine
    * the termination.
    * /param sect - the sector the photon is emitting into
    * /param photon - the photon to emit
    * /param maxDepth - the maximum number of recursive bounces allowed
    * /param depth - the number of times this photon has scattered
    * /param ignoreDirect - weather or not to store direct photons (depth=0)
    */
    static void EmitPhoton(Sector* &sect, const PhotonRay &photon,
      const size_t &maxDepth, const size_t &depth, const bool &ignoreDirect, bool produceCaustic );

    /**
     * SpotlightDir
     *    Compute a random direction within the cone of the spotlight with
     * direction 'dir' and falloffOutter as 'cosTheta'.
     **/
    static csVector3 SpotlightDir(const csVector3 &dir, const float cosTheta);

    /**
     * CausticDir
     *    Compute a random direction within the cone of the caustic spotlight with
     * direction 'dir' and falloffOutter as 'cosTheta'.
     **/
    static csVector3 CausticDir(const csVector3 &dir, const float outerFalloffAngle);

    /**
     *DirectionalLightScatter
     *     Generate a point from a circular plane around the position of 
     * spotlight from where to emit the photon
     **/
    static csVector3 DirectionalLightScatter(const csVector3 &dir, const float spRadius,const csVector3 pos);
    /**
     * EqualScatter
     *    This function will compute a random direction in the hemisphere
     * around the vector n (which should be the surface normal at the hit
     * point).  The photon should be reflected in this direction to continue
     * photon tracing.  There is no weighting and all directions around
     * the hemisphere have an equal chance of occuring.
     * /param n - The surface normal at the photons hit point
     **/
    static csVector3 EqualScatter(const csVector3 &n);

    /**
     * DiffuseScatter
     *    This funciton will compute a random direction in the hemisphere
     * around the vector n (which should be the surface normal at the hit
     * point).  The photon should be reflected in this direction to continue
     * photon tracing.  The directions are weighted by the cos of the
     * angle between the normal and the new direction (lambert's law).
     * /param n - The surface normal at the phontons hit point
     **/
    static csVector3 DiffuseScatter(const csVector3 &n);

    /**
     * StratifiedSample
     *    This function divides the hemisphere around the vector n into a grid
     * of M by N samples and returns a vector at the grid point (i, j) that is
     * jittered off the grid slightly.  This is jittered, stratified sampling.
     * /param n - The surface normal to generate vectors around
     * /param i - The azimuth grid point to generate the vector from
     * /param j - The altitude grid point to generate the vector from
     * /param M - The number of altitude subdivisions
     * /param N - the number of azimuth subdivisions
     **/
    static csVector3 StratifiedSample(const csVector3 &n, const size_t i,
                        const size_t j, const size_t M, const size_t N);
    /**
     * RotateAroundN
     *    This function will rotate the vector n away from itself theta radians
     * then around the original n phi radians.  It is used for generating directions
     * to emit photons from light sources and to scatter those photons when bouncing.
     * /param n - The vector to rotate (usual a surface normal)
     * /param theta - Angle away from n to rotate (in radians)
     * /param phi - Angle around n to rotate (in radians)
     **/
    static csVector3 RotateAroundN(const csVector3 &n, const double theta, const double phi);

    csRandomVectorGen randVect;
    csRandomFloatGen randFloat;

    // These values all come from the lighter2 global settings
    int numPhotonsPerSector;  ///< Number of photons to emit in each sector of the world
    int numCausticPhotonsPerSector;  ///< Number of Caustic photons to emit in each sector of the world

    csArray <csSphere> causticList;
    bool directLightEnabled;   ///< Should direct photons be stored (first hit after emission)
    bool indirectLightEnabled; ///< Should photons be scattered to estimate indirect lighting

    float searchRadius;       ///< Maximum distance to search during density estimation
    int numSamplesForDensity; ///< Maximum number of photons to sample during density est.

    bool finalGather;               ///< Enable the final gather to eliminate noise
    size_t numFinalGatherMSubdivs;  ///< Number of rays used in the final gather
    size_t numFinalGatherNSubdivs;  ///< Number of rays used in the final gather
  };
}

#endif // __INDIRECTLIGHT_H__
