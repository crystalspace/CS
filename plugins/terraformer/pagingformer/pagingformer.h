/*
    Copyright (C) 2006 Christoph "Fossi" Mewes
    
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


#ifndef __CS_PAGINGFORMER_H__
#define __CS_PAGINGFORMER_H__

#include "csgeom/box.h"
#include "csgeom/vector3.h"
#include "csutil/csobject.h"
#include "csutil/refarr.h"
#include "csutil/scf_implementation.h"

#include "iutil/comp.h"
#include "ivaria/terraform.h"
#include "ivaria/pagingformer.h"
#include "ivaria/simpleformer.h"

struct iObjectRegistry;

CS_PLUGIN_NAMESPACE_BEGIN(PagingFormer)
{

enum HeightmapFormat
{
  PAGINGHEIGHT_IMAGE = 1,
  PAGINGHEIGHT_RAWFLOATLE
};

class csPagingSampler;
class csSimpleFormer;

/**
 * This is a simple implementation of a terraformer plugin.
 * It only handles a single heightmap
 */
class csPagingFormer :
  public scfImplementationExt3<csPagingFormer,
                            csObject,
                            iTerraFormer,
                            iPagingFormerState,
                            iComponent>
{
private:
  /// Object registry pointer (not csRef to avoid cyclic references)
  iObjectRegistry* objectRegistry;

  /// Array to save a grid of SimpleFormers
  csRef<iTerraFormer>* former;

  /// Path to directory containing the heightmaps
  char* heightmapdir;
  HeightmapFormat heightmapformat;  

  /// Array of paths to directories containing the intmaps
  csHash<csString, csStringID> intmapdir;

  /// Array of paths to directories containing the floatmaps
  csHash<csString, csStringID> floatmapdir;

  /// Complete width of the heightmap data array
  unsigned int width;

  /// Complete height of the heightmap data array
  unsigned int height;

  /// Number of Formers in x direction
  unsigned int countx;

  /// Number of Formers in y direction
  unsigned int county;

  /// Scaling factor
  csVector3 scale;

  /// Offset vector
  csVector3 offset;

  /// Material palette containing all used materials
  csArray<iMaterialWrapper*> materialPalette;

  /// Cached string id for "vertices"
  csStringID stringVertices;  
  /// Cached string id for "normals"
  csStringID stringNormals;
  /// Cached string id for "texture coordinates"
  csStringID stringTexture_Coordinates;
  /// Cached string id for "heights"
  csStringID stringHeights;
  /// Cached string id for "material indices"
  csStringID stringMaterialIndices;

  // Allow csSimpleSampler to access the data of this class
  friend class csPagingSampler;

  void LoadFormer(uint x, uint y);

public:
  /// csSimpleFormer constructor
  csPagingFormer (iBase* parent);

  /// csSimpleFormer destructor
  virtual ~csPagingFormer ();

  // --------- iPagingFormerState implementation ---------

  /// Set the directory containing the heightmaps
  void SetHeightmapDir (const char *path,
                        const char *type = "image");

  /// Set a scaling factor to be used in lookups
  void SetScale (csVector3 scale);

  /// Set an offset vector to be used in lookups
  void SetOffset (csVector3 offset);

  /// Set additional integer map.
  bool SetIntegerMap (csStringID type, iImage* map, int scale, int offset);
  /// Set additional float map.
  bool SetFloatMap (csStringID type, iImage* map, float scale, float offset);

  /// Get the integer map dimensions.
  virtual csVector2 GetIntegerMapSize (csStringID type);

  /// Set the directory containing the heightmaps
  void SetIntmapDir (csStringID type, const char* path);

  /// Set the directory containing the heightmaps
  void SetFloatmapDir (csStringID type, const char* path);

  // ------------ iTerraFormer implementation ------------

  /// Creates and returns a sampler. See interface for details
  virtual csPtr<iTerraSampler> GetSampler (csBox2 region, 
                                           uint resx, uint resz = 0);

  /**
   * Sample float data.
   * Allowed types:
   * heights
   */ 
  virtual bool SampleFloat (csStringID type, float x, float z, 
    float &value);

  /**
   * Sample csVector2 data.
   * No allowed types (will return false)
   */ 
  virtual bool SampleVector2 (csStringID type, float x, float z, 
    csVector2 &value);

  /**
   * Sample csVector3 data.
   * Allowed types:
   * vertices
   */ 
  virtual bool SampleVector3 (csStringID type, float x, float z, 
    csVector3 &value);

  /**
   * Sample integer data.
   * No allowed types (will return false)
   */ 
  virtual bool SampleInteger (csStringID type, float x, float z, 
    int &value);
  
  virtual iObject *QueryObject () { return (csObject*)this; }
  

  // ------------- iComponent implementation -------------

  /// Initializes this object
  bool Initialize (iObjectRegistry* objectRegistry);

};

/**
 * This is the accompanying sampler implementation, that pretty much just
 * returns data straight from the heightmap.
 */
class csPagingSampler : public scfImplementation1<csPagingSampler, 
                                                  iTerraSampler>
{
private:
  csPagingFormer* terraFormer;

  /// Array of all the Samplers making up this part of the terrain
  csRefArray<iTerraSampler> sampler;

  /// count of samplers in x direction
  uint xcount;

  /// The region assigned to the sampler
  csBox2 region;

  /// The resolution to use for sampling
  unsigned int resx;
  unsigned int resz;

  /// Cached float data
  float *heights;

  /// Cached vector2 data
  csVector2 *texCoords;

  /// Cached vector3 data
  csVector3 *positions, *normals;

  // Calculate and cache positions
  void CachePositions ();

  // Calculate and cache normals
  void CacheNormals ();

  // Calculate and cache heights
  void CacheHeights ();

  // Calculate and cache texture coordinates
  void CacheTexCoords ();

public:
  // ------------ iTerraSampler implementation -----------

  csPagingSampler (csPagingFormer*, csRefArray<iTerraSampler> sampler,
                   uint xcount, csBox2 region, uint resx, uint resz = 0);

  /// csSimpleSampler destructor
  virtual ~csPagingSampler ();

  /**
   * Sample float data.
   * Allowed types:
   * height
   */ 
  virtual const float *SampleFloat (csStringID type);

  /**
   * Sample 2d vector data.
   * Allowed types:
   * texture coordinates
   */ 
  virtual const csVector2 *SampleVector2 (csStringID type);

  /**
   * Sample 3d vector data.
   * Allowed types:
   * vertices
   * normals
   */ 
  virtual const csVector3 *SampleVector3 (csStringID type);

  /**
   * Sample integer data.
   * Allowed types:
   * material indices
   */ 
  virtual const int *SampleInteger (csStringID type);

  /// Returns the material palette
  virtual const csArray<iMaterialWrapper*> &GetMaterialPalette ();

  /// Returns the sample region
  virtual const csBox2 &GetRegion () const;

  /// Returns the sampling resolution
  virtual void GetResolution (uint &resx, uint &resz) const;

  /// Returns 0, since changes aren't allowed in this simple terraformer
  virtual unsigned int GetVersion () const;

  /// Deletes all cached data
  virtual void Cleanup (); 
};

}
CS_PLUGIN_NAMESPACE_END(PagingFormer)

#endif // __CS_PAGINGFORMER_H__
