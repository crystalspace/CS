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

#include "common.h"
#include "irradiancecache.h"

namespace lighter
{
  /**
   * This is the sample structure
   * The power is not compressed so the
   * size is 28 bytes
   */
  struct IrradianceSample {
    float pos[3];   // sample position
    int plane;      // splitting plane for kd-tree
    float power[3]; // irradiance (uncompressed)
  };

  /**
   * This is the constructor for the irradiance cache.
   * To create the irradiance cache it is necessary to
   * specify the maximum number of samples that will be stored.
   * It is okay to underestimate this as it will expand as
   * needed.
   */
  IrradianceCache::IrradianceCache( const size_t maxSamps )
  {
    storedSamples = 0;
    initialSize = maxSamples = maxSamps;

    samples = (IrradianceSample*)malloc( sizeof( IrradianceSample ) * ( maxSamples+1 ) );

    bboxMin[0] = bboxMin[1] = bboxMin[2] = FLT_MAX;
    bboxMax[0] = bboxMax[1] = bboxMax[2] = -FLT_MAX;
  }

  IrradianceCache::~IrradianceCache()
  {
    free( samples );
  }

  size_t IrradianceCache :: GetSampleCount() { return storedSamples; }

  /* store puts a sample into the flat array that will form
   * the final kd-tree.
   *
   * Call this function to store a photon.
   */
  void IrradianceCache :: Store(
                           const float power[3],
                           const float pos[3] )
  {
    // Check for storage and attempt to expand if needed
    if (storedSamples>=maxSamples && !Expand())
      return;

    storedSamples++;
    IrradianceSample *const node = &(samples[storedSamples]);

    for (size_t i=0; i<3; i++) {
      node->pos[i] = pos[i];

      if (node->pos[i] < bboxMin[i])
        bboxMin[i] = node->pos[i];
      if (node->pos[i] > bboxMax[i])
        bboxMax[i] = node->pos[i];

      node->power[i] = power[i];
    }
  }

  bool IrradianceCache :: Expand()
  {
    // Increase size by initial amount
    size_t newMax = maxSamples + initialSize;

    // Allocate a new array
    IrradianceSample *newSamples =
      (IrradianceSample*)malloc( sizeof( IrradianceSample ) * ( newMax+1 ) );
    if(newSamples == NULL) return false;

    // Copy over old data
    for(size_t i=0; i<maxSamples; i++)
      newSamples[i] = samples[i];

    // Free old array
    free(samples);

    // Replace old variables
    samples = newSamples;
    maxSamples = newMax;

    // return success
    return true;
  }

  /* balance creates a left-balanced kd-tree from the flat photon array.
   * This function should be called before the photon map
   * is used for rendering.
   */
  void IrradianceCache :: Balance(Statistics::ProgressState &prog)
  {
    if (storedSamples>1) {
      // allocate two temporary arrays for the balancing procedure
      IrradianceSample **pa1 =
        (IrradianceSample**)malloc(sizeof(IrradianceSample*)*(storedSamples+1));
      IrradianceSample **pa2 =
        (IrradianceSample**)malloc(sizeof(IrradianceSample*)*(storedSamples+1));

      for (size_t i=0; i<=storedSamples; i++)
        pa2[i] = &(samples[i]);

      BalanceSegment( pa1, pa2, 1, 1, storedSamples, prog );
      free(pa2);

      // reorganize balanced kd-tree (make a heap)
      size_t d, j=1, foo=1;
      IrradianceSample fooSample = samples[j];

      for (size_t i=1; i<=storedSamples; i++) {
        d=pa1[j]-samples;
        pa1[j] = NULL;
        if (d != foo)
          samples[j] = samples [d];
        else {
          samples[j] = fooSample;

          if (i<storedSamples) {
            for (;foo<=storedSamples; foo++)
              if (pa1[foo] != NULL)
                break;
            fooSample = samples [foo];
            j = foo;
          }
          continue;
        }
        j = d;
      }
      free(pa1);
    }

    halfStoredSamples = storedSamples/2-1;
  }


  #define swap(ph,a,b) { IrradianceSample *ph2=ph[a]; ph[a]=ph[b]; ph[b]=ph2; }

  /* median_split splits the photon array into two separate
   * pieces around the median, with all photons below
   * the median in the lower half and all photons above
   * the median in the upper half. The comparison
   * criteria is the axis (indicated by the axis parameter)
   * (inspired by routine in "Algorithms in C++" by Sedgewick)
   */
  void IrradianceCache :: MedianSplit(
                                  IrradianceSample **p,
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
  void IrradianceCache :: BalanceSegment(
                                     IrradianceSample **pbal,
                                     IrradianceSample **porg,
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

  void IrradianceCache :: SaveToFile(const char* filename)
  {
    // Open file for writing in binary mode
    FILE* fout = fopen(filename, "wb");
    if(fout != NULL)
    {
      // Write the number of photons
      fwrite(&storedSamples, sizeof(size_t), 1, fout);

      // Individual components are stored instead of
      // full photon structures to aid in compression

      // Write photon position
      for(size_t j=0; j<3; j++)
      {
        for(size_t i=0; i<storedSamples; i++)
        {
          fwrite(&(samples[i].pos[j]), sizeof(float), 1, fout);
        }
      }

      // Write photon power
      for(size_t j=0; j<3; j++)
      {
        for(size_t i=0; i<storedSamples; i++)
        {
          fwrite(&(samples[i].power[j]), sizeof(float), 1, fout);
        }
      }

      // Write kd-tree splitting plane
      for(size_t i=0; i<storedSamples; i++)
      {
        fwrite(&(samples[i].plane), sizeof(short), 1, fout);
      }

      // Close the file
      fclose(fout);
    }
  }

};
