/*
  Copyright (C) 2001 Henrik Wann Jensen / A.K. Peters Ltd.
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

//-----------------------------------------------------------------------------
// Based on: photonmap.cc
// An example implementation of the photon map data structure
//
// Henrik Wann Jensen - February 2001
//
// from appendix B of 'Realistic Image Synthesis Using Photon Mapping'
//-----------------------------------------------------------------------------

#ifndef __PHOTONMAP_H__
#define __PHOTONMAP_H__

namespace lighter
{
  struct Photon;
  struct NearestPhotons;

  class PhotonMap
  {
  public:
    PhotonMap( size_t max_phot );
    ~PhotonMap();

    void store(
      const float power[3],         // photon power
      const float pos[3],           // photon position
      const float dir[3] );         // photon direction

    void scale_photon_power(
      const float scale );          // 1/(number of emitted photons)

    void balance(void);             // balance the kd-tree (before use!)

    void irradiance_estimate(
      float irrad[3],                // returned irradiance
      const float pos[3],            // surface position
      const float normal[3],         // surface normal at pos
      const float max_dist,          // max distance to look for photons
      const size_t nphotons ) const; // number of photons to use

    void locate_photons(
      NearestPhotons *const np,     // np is used to locate the photons
      const size_t index ) const;      // call with index = 1

    void photon_dir(
      float *dir,                    // direction of photon (returned)
      const Photon *p ) const;       // the photon

  private:

    bool expand();

    void balance_segment(
      Photon **pbal,
      Photon **porg,
      const size_t index,

      const size_t start,
      const size_t end );

    void median_split(
      Photon **p,
      const size_t start,
      const size_t end,
      const size_t median,
      const int axis );

    Photon *photons;

    size_t stored_photons;
    size_t half_stored_photons;
    size_t max_photons;
    int prev_scale;

    float bbox_min[3];     // use bbox_min;
    float bbox_max[3];     // use bbox_max;

    // We save some memory by making these static
    static bool directionTablesReady;
    static float costheta[256];
    static float sintheta[256];
    static float cosphi[256];
    static float sinphi[256];
  };
};

#endif // __PHOTONMAP_H__