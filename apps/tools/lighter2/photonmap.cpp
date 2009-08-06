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

#include "common.h"
#include "photonmap.h"
  
namespace lighter
{
  // How many new photons to allocate when expanding the array size
  #define ARRAY_EXPAND_AMOUNT   100000

  /**
   * This is the photon
   * The power is not compressed so the
   * size is 28 bytes
   */
  struct Photon {
    float pos[3];                  // The position of the photon in space
    short plane;                   // splitting plane for kd-tree
    unsigned char theta, phi;      // incoming direction
    float power[3];                // photon power (uncompressed)
  };

  /**
   * This structure is used only to locate the
   * nearest photons.  Initialize max, pos and
   * distSq[0] (squared maximum search distance)
   * before calling LocatePhotons.  LocatePhotons
   * will fill found, gotHeap and index.  See
   * IrradianceEstimate for more details.
   */
  struct NearestPhotons {

    // Initialize these members before using LocatePhotons()
    size_t max;             ///< Number of photons to search for
    float pos[3];           ///< Position to search around (for computing distances)

    bool bias;              ///< Should search be biased along the surface containing pos?
    float norm[3];          ///< Surface normal at pos (for biasing search along a surface)
    float biasScale;        ///< A scale between 0 & 1 to apply to distances along the normal

    // Initialize distSq[0] to the max search distance squared (distSq[1 to found] is filled by LocatePhotons())
    float *distSq;          ///< Array (or max heap) of squared distances from 'pos' to the photons in 'index'

    // These members are filled by the LocatePhotons() function
    size_t found;           ///< Number found so far
    bool gotHeap;           ///< Has the array been converted to a max heap yet?
    const Photon **index;   ///< Array (or max heap) of pointers to the photons

    // Functions to help with management of the max heap
    static void AddToHeap(NearestPhotons *const NP, const float &distSq, const Photon* &p)
    {
      // If array is not yet full, treat as a flat array
      if ( NP->found < NP->max )
      {
        NP->found++;
        NP->distSq[NP->found] = distSq;
        NP->index[NP->found] = p;
      }

      // Once array is full, must treat as a max heap
      else
      {
        // Build heap if we haven't already
        if (!NP->gotHeap) BuildMaxHeap(NP);

        // Find spot in heap for this photon
        size_t j = 2, parent = 1;
        while ( j <= NP->found )
        {
          if ( j < NP->found && NP->distSq[j] < NP->distSq[j+1] )
            j++;
          if ( distSq > NP->distSq[j] )
            break;
        
          NP->distSq[parent] = NP->distSq[j];
          NP->index[parent] = NP->index[j];
          parent = j;
          j += j;
        }

        // Replace photon at this spot in the heap with new photon
        if ( distSq < NP->distSq[parent] )
        {
          NP->index[parent] = p;
          NP->distSq[parent] = distSq;
        }

        // Any photon that is further out than the most distant
        // photon in the heap will be ignored so we update minimum
        // distance in NP to the distance to this furtherst node in
        // order to avoid extra work
        NP->distSq[0] = NP->distSq[1];
      }
    }

    static void BuildMaxHeap(NearestPhotons *const NP)
    {
      float dst2;
      const Photon *phot;

      size_t halfFound = NP->found>>1;
      for ( size_t k=halfFound; k>=1; k--)
      {
        size_t parent=k ;
        phot = NP->index[k];
        dst2 = NP->distSq[k];
        while ( parent <= halfFound )
        {
          size_t j = parent+parent;

          if (j<NP->found && NP->distSq[j]<NP->distSq[j+1])
            j++;

          if (dst2>=NP->distSq[j])
            break;

          NP->distSq[parent] = NP->distSq[j];
          NP->index[parent] = NP->index[j];

          parent=j;
        }

        NP->distSq[parent] = dst2;
        NP->index[parent] = phot;
      }

      // The max heap is now initialized
      NP->gotHeap = true;
    }
  };

  // Static member variables
  bool PhotonMap::directionTablesReady = false;
  float PhotonMap::cosTheta[256];
  float PhotonMap::sinTheta[256];
  float PhotonMap::cosPhi[256];
  float PhotonMap::sinPhi[256];

  /**
   * This is the constructor for the photon map.
   * To create the photon map it is necessary to specify
   * the maximum number of photons that will be stored
   * It is okay to underestimate this as it will expand
   * as needed.
   */
  PhotonMap::PhotonMap( const size_t maxPhot )
  {
    coneK = 1.0;
    storedPhotons = 0;
    prevScale = 1;
    initialSize = maxPhotons = maxPhot;

    photons = (Photon*)malloc( sizeof( Photon ) * ( maxPhotons+1 ) );

    bboxMin[0] = bboxMin[1] = bboxMin[2] = FLT_MAX;
    bboxMax[0] = bboxMax[1] = bboxMax[2] = -FLT_MAX;

    //----------------------------------------
    // initialize direction conversion tables
    //----------------------------------------
    if(!directionTablesReady)
    {
      for (size_t i=0; i<256; i++) {
        double angle = double(i)*(1.0/256.0)*PI;
        cosTheta[i] = cos( angle );
        sinTheta[i] = sin( angle );
        cosPhi[i]   = cos( 2.0*angle );
        sinPhi[i]   = sin( 2.0*angle );
      }
    }
  }

  PhotonMap::~PhotonMap()
  {
    free( photons );
  }

  /**
   * PhotonDir() - returns the direction of a photon
   */
  void PhotonMap :: PhotonDir( float *dir, const Photon *p ) const
  {
    dir[0] = sinTheta[p->theta]*cosPhi[p->phi];
    dir[1] = sinTheta[p->theta]*sinPhi[p->phi];
    dir[2] = cosTheta[p->theta];
  }

  size_t PhotonMap :: GetPhotonCount() { return storedPhotons; }

  inline float PhotonMap :: ConeWeight(float dist, float r, float k)
  {
    return (1.0 - dist/(k*r));
  }

  inline float PhotonMap :: GaussianWeight(float distSq, float rSq)
  {
    #define G_ALPHA   1.818f
    #define G_BETA    1.953f

    return G_ALPHA*(1.0 - ( 1.0 - expf(-G_BETA*(distSq/(2*rSq))) )/
                          ( 1.0 - expf(-G_BETA)) );
  }

  /**
   * IrradianceEstimate() - computes an irradiance estimate
   * at a given surface position
   */
  void PhotonMap :: IrradianceEstimate(
                                         float irrad[3],                // returned irradiance
                                         const float pos[3],            // surface position
                                         const float norm[3],         // surface normal at pos
                                         const float maxDist,           // max distance to look for photons
                                         const size_t nphotons,         // number of photons to use
                                         const FilterType filter) const // Filter to apply to photon power
  {
    // Zero irradiance for accumulation of photon power
    irrad[0] = irrad[1] = irrad[2] = 0.0;

    // Initialize the NearestPhotons structure
    NearestPhotons np;
    np.distSq = (float*)alloca( sizeof(float)*(nphotons+1) );
    np.index = (const Photon**)alloca( sizeof(Photon*)*(nphotons+1) );

    np.pos[0] = pos[0]; np.pos[1] = pos[1]; np.pos[2] = pos[2];

    float len = sqrtf(norm[0]*norm[0] + norm[1]*norm[1] + norm[2]*norm[2]);
    np.norm[0] = norm[0]/len; np.norm[1] = norm[1]/len; np.norm[2] = norm[2]/len;

    np.biasScale = 0.0001f;
    np.max = nphotons;
    np.found = 0;
    np.gotHeap = false;
    np.bias = true;
    np.distSq[0] = maxDist*maxDist;

    // locate the nearest photons
    LocatePhotons( &np, 1 );

    // if less than 8 photons assume black (Note: this may be dangerous)
    if (np.found < 8)
      return;

    // Choose weights and norm based on filter
    float pdir[3], normF;
    switch(filter)
    {
      case FILTER_NONE: default:
      {
        // sum irradiance from all photons
        for (size_t i=1; i<=np.found; i++)
        {
          const Photon *p = np.index[i];
          PhotonDir( pdir, p );
          if ( (pdir[0]*norm[0]+pdir[1]*norm[1]+pdir[2]*norm[2]) < 0.0f )
          {
            irrad[0] += p->power[0];
            irrad[1] += p->power[1];
            irrad[2] += p->power[2];
          }
        }

        // Norm factor is area of a circle
        normF = (PI*np.distSq[0]);
      }
      break;

      case FILTER_CONE:
      {
        // sum irradiance from all photons
        for (size_t i=1; i<=np.found; i++)
        {
          const Photon *p = np.index[i];
          PhotonDir( pdir, p );
          if ( (pdir[0]*norm[0]+pdir[1]*norm[1]+pdir[2]*norm[2]) < 0.0f )
          {
            float w = ConeWeight(sqrtf(np.distSq[i]), sqrtf(np.distSq[0]), coneK);
            irrad[0] += p->power[0]*w;
            irrad[1] += p->power[1]*w;
            irrad[2] += p->power[2]*w;
          }
        }

        // Norm factor is MODIFIED area of a circle
        normF = (PI*np.distSq[0])*(1.0f-2.0f/(3.0f*coneK));
      }
      break;

      case FILTER_GAUSS:
      {
        // sum irradiance from all photons
        for (size_t i=1; i<=np.found; i++)
        {
          const Photon *p = np.index[i];
          PhotonDir( pdir, p );
          if ( (pdir[0]*norm[0]+pdir[1]*norm[1]+pdir[2]*norm[2]) < 0.0f )
          {
            float w = GaussianWeight(np.distSq[i], np.distSq[0]);
            irrad[0] += p->power[0]*w;
            irrad[1] += p->power[1]*w;
            irrad[2] += p->power[2]*w;
          }
        }

        // Gaussian filter is already normalized
        normF = 1.0;
      }
      break;
    }

    // Normalize density
    irrad[0] /= normF;
    irrad[1] /= normF;
    irrad[2] /= normF;
  }

  /* LocatePhotons finds the nearest photons in the
   * photon map given the parameters in np
   */
  void PhotonMap :: LocatePhotons(
                                    NearestPhotons *const np,
                                    const size_t index ) const
  {
    // Get pointer to photon at this node
    const Photon *p = &(photons [index]);

    // If this is not a leaf then traverse it's children first
    if (index<halfStoredPhotons)
    {
      // Compute distance in splitting dimension only
      float splitDist = np->pos[ p->plane ] - p->pos[ p->plane ];

      // If splitDist is positive then there are likely to be more good
      // photons in the right sub-tree so we traverse it first
      if (splitDist > 0.0)
      {
        LocatePhotons( np, 2*index+1 );

        // If squared dist in the splitting dimension is greater than
        // or equal to the maximum squared dist then we can eliminate
        // the other sub-tree completely.
        if ( splitDist*splitDist < np->distSq[0] )
        {
          LocatePhotons( np, 2*index );
        }
      }
      
      // distSplit is negative so traverse the left sub-tree first
      else
      {
        LocatePhotons( np, 2*index );
        if ( splitDist*splitDist < np->distSq[0] )
        {
          LocatePhotons( np, 2*index+1 );
        }
      }
    }

    // Compute squared distance between current photon and np->pos
    float V[3], distSq, maxDist = np->distSq[0];
    V[0] = np->pos[0] - p->pos[0]; distSq = V[0]*V[0];
    V[1] = np->pos[1] - p->pos[1]; distSq += V[1]*V[1];
    V[2] = np->pos[2] - p->pos[2]; distSq += V[2]*V[2];

    // Bias against the normal direction (changes search area to a
    // flattened disk instead of a sphere as Jensen suggests)
    if(np->bias)
    {
      // Normalize V
      float len = sqrt(distSq);
      V[0] /= len; V[1] /= len; V[2] /= len;

      // Compute dot product with normal in absolute value
      float dotP = fabsf(V[0]*np->norm[0] + V[1]*np->norm[1] + V[2]*np->norm[2]);

      // Interpolate to get scale for distance.  This will produce
      // a scale of 1.0 (i.e. no scale) when perpendicular to the normal,
      // np->biasScale when parallel and interpolate between them elsewhere.
      float scale = 1.0*(1.0 - dotP) + np->biasScale*(dotP);

      // Scale maximum distance
      maxDist *= scale;
    }

    // Is this photon close enough?  If so, add it to the candidate list.
    if (distSq < maxDist )
    {
      NearestPhotons::AddToHeap(np, distSq, p);
    }
  }

  /* store puts a photon into the flat array that will form
   * the final kd-tree.
   *
   * Call this function to store a photon.
   */
  void PhotonMap :: Store(
                           const float power[3],
                           const float pos[3],
                           const float dir[3] )
  {
    // Check for storage and attempt to expand if needed
    if (storedPhotons>=maxPhotons && !Expand())
      return;

    storedPhotons++;
    Photon *const node = &(photons[storedPhotons]);

    for (size_t i=0; i<3; i++) {
      node->pos[i] = pos[i];

      if (node->pos[i] < bboxMin[i])
        bboxMin[i] = node->pos[i];
      if (node->pos[i] > bboxMax[i])
        bboxMax[i] = node->pos[i];

      node->power[i] = power[i];
    }

    int theta = int( acos(dir[2])*(256.0/PI) );
    if (theta>255)
      node->theta = 255;
    else
      node->theta = (unsigned char)theta;

    int phi = int( atan2(dir[1],dir[0])*(256.0/(2.0*PI)) );
    if (phi>255)
      node->phi = 255;
    else if (phi<0)
      node->phi = (unsigned char)(phi+256);
    else
      node->phi = (unsigned char)phi;
  }


  /* scale_photon_power is used to scale the power of all
   * photons once they have been emitted from the light
   * source, scale = l/(#emitted photons).
   * Call this function after each light source is processed.
   */
  void PhotonMap :: ScalePhotonPower( const float scale )
  {
    for (size_t i=prevScale; i<=storedPhotons; i++) {
      photons[i].power[0] *= scale;
      photons[i].power[1] *= scale;
      photons[i].power[2] *= scale;
    }
    prevScale = storedPhotons+1;
  }

  bool PhotonMap :: Expand()
  {
    // Increase size by initial amount
    size_t newMax = maxPhotons + initialSize;

    // Allocate a new array
    Photon *newPhotons = (Photon*)malloc( sizeof( Photon ) * ( newMax+1 ) );
    if(newPhotons == NULL) return false;

    // Copy over old data
    for(size_t i=0; i<maxPhotons; i++)
      newPhotons[i] = photons[i];

    // Free old array
    free(photons);

    // Replace old variables
    photons = newPhotons;
    maxPhotons = newMax;

    // return success
    return true;
  }

  /* balance creates a left-balanced kd-tree from the flat photon array.
   * This function should be called before the photon map
   * is used for rendering.
   */
  void PhotonMap :: Balance(Statistics::ProgressState &prog)
  {
    if (storedPhotons>1) {
      // allocate two temporary arrays for the balancing procedure
      Photon **pa1 = (Photon**)malloc(sizeof(Photon*)*(storedPhotons+1));
      Photon **pa2 = (Photon**)malloc(sizeof(Photon*)*(storedPhotons+1));

      for (size_t i=0; i<=storedPhotons; i++)
        pa2[i] = &(photons[i]);

      BalanceSegment( pa1, pa2, 1, 1, storedPhotons, prog );
      free(pa2);

      // reorganize balanced kd-tree (make a heap)
      size_t d, j=1, foo=1;
      Photon fooPhoton = photons[j];

      for (size_t i=1; i<=storedPhotons; i++) {
        d=pa1[j]-photons;
        pa1[j] = NULL;
        if (d != foo)
          photons[j] = photons [d];
        else {
          photons[j] = fooPhoton;

          if (i<storedPhotons) {
            for (;foo<=storedPhotons; foo++)
              if (pa1[foo] != NULL)
                break;
            fooPhoton = photons [foo];
            j = foo;
          }
          continue;
        }
        j = d;
      }
      free(pa1);
    }

    halfStoredPhotons = storedPhotons/2-1;
  }


  #define swap(ph,a,b) { Photon *ph2=ph[a]; ph[a]=ph[b]; ph[b]=ph2; }

  /* median_split splits the photon array into two separate
   * pieces around the median, with all photons below
   * the median in the lower half and all photons above
   * the median in the upper half. The comparison
   * criteria is the axis (indicated by the axis parameter)
   * (inspired by routine in "Algorithms in C++" by Sedgewick)
   */
  void PhotonMap :: MedianSplit(
                                  Photon **p,
                                  const size_t start,                // start of photon block in array
                                  const size_t end,                  // end of photon block in array
                                  const size_t median,               // desired median number
                                  const int axis )                // axis to split along
  {
    size_t left = start;
    size_t right = end;

    while ( right > left ) {
      const float v = p[right]->pos[axis] ;
      size_t i=left-1;
      size_t j=right;
      for (;;) {
        while ( p[++i]->pos[axis] < v )
          ;
        while ( p[--j]->pos[axis] > v && j>left )
          ;
        if ( i >= j )
          break;
        swap(p,i,j);
      }

      swap(p,i,right);
      if ( i >= median )
        right=i-1;
      if ( i <= median )
        left=i+1;
    }
  }


  /* See "Realistic Image Synthesis using Photon Mapping" Chapter 6
   * for an explanation of this function
   */
  void PhotonMap :: BalanceSegment(
                                     Photon **pbal,
                                     Photon **porg,
                                     const size_t index,
                                     const size_t start,
                                     const size_t end,
                                     Statistics::ProgressState &prog)
  {
    prog.Advance();

    //--------------------
    // compute new median
    //--------------------

    size_t median=1;
    while ((4*median) <= (end-start+1))
      median += median;


    if ((3*median) <= (end-start+1)) {
      median += median;
      median += start-1;
    } else
      median = end-median+1;

    //--------------------------
    // find axis to split along
    //--------------------------

    int axis=2;
    if ((bboxMax[0]-bboxMin[0])>(bboxMax[1]-bboxMin[1]) &&
      (bboxMax[0]-bboxMin[0])>(bboxMax[2]-bboxMin[2]))
      axis=0;
    else if ((bboxMax[1]-bboxMin[1])>(bboxMax[2]-bboxMin[2]))
      axis=1;

    //-------------------------------------------
    // partition photon block around the median
    // ------------------------------------------

    MedianSplit( porg, start, end, median, axis );

    pbal[ index ] = porg[ median ];
    pbal[ index ]->plane = axis;

    //----------------------------------------------
    // recursively balance the left and right block
    //----------------------------------------------

    if ( median > start ) {
      // balance left segment
      if ( start < median-1 ) {
        const float tmp=bboxMax[axis];
        bboxMax[axis] = pbal[index]->pos[axis];
        BalanceSegment( pbal, porg, 2*index, start, median-1, prog );
        bboxMax[axis] = tmp;
      } else {
        pbal[ 2*index ] = porg[start];
      }
    }

    if ( median < end ) {
      // balance right segment
      if ( median+1 < end ) {
        const float tmp = bboxMin[axis];
        bboxMin[axis] = pbal[index]->pos[axis];
        BalanceSegment( pbal, porg, 2*index+1, median+1, end, prog );
        bboxMin[axis] = tmp;
      } else {
        pbal[ 2*index+1 ] = porg[end];
      }
    }
  }

  void PhotonMap :: SaveToFile(const char* filename)
  {
    // Open file for writing in binary mode
    FILE* fout = fopen(filename, "wb");
    if(fout != NULL)
    {
      // Write the number of photons
      fwrite(&storedPhotons, sizeof(size_t), 1, fout);

      // Individual components are stored instead of
      // full photon structures to aid in compression

      // Write photon position
      for(size_t j=0; j<3; j++)
      {
        for(size_t i=0; i<storedPhotons; i++)
        {
          fwrite(&(photons[i].pos[j]), sizeof(float), 1, fout);
        }
      }

      // Write photon power
      for(size_t j=0; j<3; j++)
      {
        for(size_t i=0; i<storedPhotons; i++)
        {
          fwrite(&(photons[i].power[j]), sizeof(float), 1, fout);
        }
      }

      // Write photon direction
      for(size_t i=0; i<storedPhotons; i++)
      {
        fwrite(&(photons[i].theta), sizeof(unsigned char), 1, fout);
      }

      for(size_t i=0; i<storedPhotons; i++)
      {
        fwrite(&(photons[i].phi), sizeof(unsigned char), 1, fout);
      }

      // Write kd-tree splitting plane
      for(size_t i=0; i<storedPhotons; i++)
      {
        fwrite(&(photons[i].plane), sizeof(short), 1, fout);
      }

      // Close the file
      fclose(fout);
    }
  }

};
