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

#include "statistics.h"

namespace lighter
{
  struct Photon;
  struct NearestPhotons;

  enum FilterType
  {
    FILTER_NONE,
    FILTER_CONE,
    FILTER_GAUSS
  };

  class PhotonMap
  {
  public:
    PhotonMap( size_t maxPhot );
    ~PhotonMap();

    /**
     * Store
     *    Add a photon to the photonMap structure.
     * /param power - The radiant power of this photon
     * /param pos - The world position of this photon
     * /param dir - The INCOMING direction for this photon
     **/
    void Store(
      const float power[3],
      const float pos[3],
      const float dir[3] );

    size_t GetPhotonCount();
    
    /**
     * ScalePhotonPower
     *    Scale the power member of every photon in the map that
     * was added since the last scale.  This is useful for scaling
     * by the number of photons emitted for a single light.
     * /param scale - Fraction to scale each photon's power by
     **/
    void ScalePhotonPower(
      const float scale );

    /**
     * Balance
     *    Restructure the photon map heap into a balanced kd-tree.
     * You must call this function before using the map with the
     * functions IrradianceEstimate, RadianceEstimate and LocatePhotons.
     **/
    void Balance(Statistics::ProgressState &prog);

    /**
     * IrradianceEstimate
     *    Estimate the irradiance landing on a specific point by computing
     * the photon density in that area.  NOTE: This is NOT the visible light
     * emitted from a point.
     * /param irrad - Returned irradiance estimate
     * /param pos - Position in scene to compute estimate (should be on surface)
     * /param normal - The surface normal at 'pos'
     * /param maxDist - The maximum distance to search for neighbors (world units)
     * /param nphotons - The number of neighbor photons to use for estimate.
     **/
    void IrradianceEstimate(
      float irrad[3],
      const float pos[3],
      const float normal[3],
      const float maxDist,
      const size_t nphotons,
      const FilterType filter = FILTER_CONE) const;

    /**
     * LocatePhotons
     *     Search the kd-tree heap to find photons.  the NearestPhotons structure
     * must be initialized with the number to search for, the maximum distance
     * squared and the world position to search around.  Upon return, 'np' will
     * contain a count of the number of photons found and pointers to the actual
     * photon structures in a max heap.
     * /param np - Structure to define the search and return the located photons.
     * /param index - Index to start search (call with index = 1).
     **/
    void LocatePhotons(
      NearestPhotons *const np,
      const size_t index ) const;

    /**
     * PhotonDir
     *    Convert the spherical coordinates stored in a photon into a
     * world vector direction using the pre-compute lookup table.
     * /param dir - returned vector direction (must be an array of 3 floats)
     * /param p - photon structure to compute direction for
     **/
    void PhotonDir(
      float *dir,
      const Photon *p ) const;

    /**
     * SaveToFile
     *    Write the contents of the photon map to a file for external use.
     * The file will have the following format:
     *
     *    <size_t:photonCount>
     *
     *    <float:PositionXCoordinate>*photonCount
     *    <float:PositionYCoordinate>*photonCount
     *    <float:PositionZCoordinate>*photonCount
     *
     *    <float:PowerRedComponent>*photonCount
     *    <float:PowerGreenComponent>*photonCount
     *    <float:PowerBlueComponent>*photonCount
     *
     *    <unsigned char:DirectionThetaAngle>*photonCount
     *    <unsigned char:DirectionPhiAngle>*photonCount
     *
     *    <short int:KDTreeSplittingPlane>*photonCount
     *
     * Note: whitespace and newlines above are not part of the file format.
     * Endiness is the machine native format.
     *
     * /param filename - name of file to store photon map data
     **/
    void SaveToFile(
      const char* filename);

  private:

    static inline float ConeWeight(float dist, float r, float k);
    static inline float GaussianWeight(float distSq, float rSq);

    bool Expand();

    void BalanceSegment(
      Photon **pbal,
      Photon **porg,
      const size_t index,

      const size_t start,
      const size_t end,
      Statistics::ProgressState &prog);

    void MedianSplit(
      Photon **p,
      const size_t start,
      const size_t end,
      const size_t median,
      const int axis );

    Photon *photons;          ///< Internal array of photons (kd-tree heap after call to Balance())

    size_t initialSize;       ///< The initial requested array size (used for array expansion)
    size_t storedPhotons;     ///< Number of photons stored in the internal array
    size_t halfStoredPhotons; ///< Half the photons stored in the internal array
    size_t maxPhotons;        ///< Allocated size of internal array
    int prevScale;            ///< Index of the last photon scaled with ScalePhotonPower
    float coneK;              ///< K factor used to control cone filter

    float bboxMin[3];         ///< Minimum value in each dimension for bounding box computation
    float bboxMax[3];         ///< Maximum value in each dimension for bounding box computation

    // Precomputed cosine and sine lookup tables for spherical coordinate conversion
    static bool directionTablesReady;
    static float cosTheta[256];
    static float sinTheta[256];
    static float cosPhi[256];
    static float sinPhi[256];
  };
};

#endif // __PHOTONMAP_H__