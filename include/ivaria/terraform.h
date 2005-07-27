/*
    Copyright (C) 2004 by Anders Stenberg, Daniel Duhprey

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

#ifndef __CS_IVARIA_TERRAFORM_H__
#define __CS_IVARIA_TERRAFORM_H__

#include "csutil/scf.h"
#include "csutil/strset.h"

struct iMaterialWrapper;
struct iTerraSampler;

class csBox2;
class csVector2;
class csVector3;

SCF_VERSION (iTerraFormer, 0, 0, 1);

/**
 * TerraFormer objects are used to retrieve terrain data.
 * All data is retrieved in blocks, from sampler regions.
 */
struct iTerraFormer : public iBase
{
  /**
   * Get a sampler region from the terraformer. The sampler region will
   * be used for all actual data retrieval.
   */
  virtual csPtr<iTerraSampler> GetSampler (csBox2 region, 
    unsigned int resolution) = 0;

  /**
   * Sample float data at a given spot on the terrain.
   * Note that this should only be used when single heights are needed.
   * For multiple samples in a grid a sampler should be used.
   * Returns true if the requested type was returned, and false otherwise.
   */
  virtual bool SampleFloat (csStringID type, float x, float z, 
    float &value) = 0;

  /**
   * Sample csVector2 data at a given spot on the terrain.
   * Note that this should only be used when single samples are needed.
   * For multiple samples in a grid a sampler should be used.
   * Returns true if the requested type was returned, and false otherwise.
   */
  virtual bool SampleVector2 (csStringID type, float x, float z, 
    csVector2 &value) = 0;

  /**
   * Sample csVector2 data at a given spot on the terrain.
   * Note that this should only be used when single samples are needed.
   * For multiple samples in a grid a sampler should be used.
   * Returns true if the requested type was returned, and false otherwise.
   */
  virtual bool SampleVector3 (csStringID type, float x, float z, 
    csVector3 &value) = 0;

  /**
   * Sample integer data at a given spot on the terrain.
   * Note that this should only be used when single samples are needed.
   * For multiple samples in a grid a sampler should be used.
   * Returns true if the requested type was returned, and false otherwise.
   */
  virtual bool SampleInteger (csStringID type, float x, float z, 
    int &value) = 0;
};


SCF_VERSION (iTerraSampler, 0, 0, 1);

/**
 * TerraSampler objects are used for the actual queries of terrain data
 * Sampler regions are requested from the iTerraFormer plugin, and sampled
 * for data via the Sample methods.
 */
struct iTerraSampler : public iBase
{
  /**
   * Sample float data of the specified from the region. Data is sampled in 
   * a grid (regular or irregular) with the square resolution specified when 
   * the sampler was created. The returned array is guaranteed to be valid
   * until Cleanup is called.
   */
  virtual const float *SampleFloat (csStringID type) = 0;

  /**
   * Sample csVector2 data of the specified from the region. Data is sampled in 
   * a grid (regular or irregular) with the square resolution specified when 
   * the sampler was created. The returned array is guaranteed to be valid
   * until Cleanup is called.
   */
  virtual const csVector2 *SampleVector2 (csStringID type) = 0;

  /**
   * Sample csVector3 data of the specified from the region. Data is sampled in 
   * a grid (regular or irregular) with the square resolution specified when 
   * the sampler was created. The returned array is guaranteed to be valid
   * until Cleanup is called.
   */
  virtual const csVector3 *SampleVector3 (csStringID type) = 0;

  /**
   * Sample integer data of the specified from the region. Data is sampled in 
   * a grid (regular or irregular) with the square resolution specified when 
   * the sampler was created. The returned array is guaranteed to be valid
   * until Cleanup is called.
   */
  virtual const int *SampleInteger (csStringID type) = 0;

  /**
   * Retrieve the material palette used by this sampler region.
   * Null entries are allowed.
   */
  virtual const csArray<iMaterialWrapper*> &GetMaterialPalette () = 0;

  /// Retrieve the sample region specified when the sampler was created
  virtual const csBox2 &GetRegion () const = 0;

  /// Retrieve the sampling resolution specified when the sampler was created
  virtual unsigned int GetResolution () const = 0;
  
  /**
   * Retrieve the version number of this sampler. This will be increased
   * whenever any terrain data in this region changes, and should thereby
   * be used as a dirty indicator.
   */
  virtual unsigned int GetVersion () const = 0;
  
  /**
   * Hint to the sampler that no data will be retrieved from it for a while. 
   * This may give the sampler a chance to release data it's been caching for
   * faster retrieval, and thereby save memory. This is a hint only, and may
   * be ignored by the underlying implementation. Arrays returned by Sample
   * calls are not guaranteed to be valid after Cleanup has been called,
   * and must be considered invalid.
   */
  virtual void Cleanup () = 0; 
};

#endif // __CS_IVARIA_TERRAFORM_H__
