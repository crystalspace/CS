/*
    Copyright (C) 2004 Anders Stenberg, Daniel Duhprey

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


#ifndef __CS_SIMPLEFORMER_H__
#define __CS_SIMPLEFORMER_H__

#include "csgeom/vector3.h"

#include "iutil/comp.h"

#include "ivaria/simpleformer.h"
#include "ivaria/terraform.h"

struct iObjectRegistry;

class csSimpleSampler;

struct csIntMap
{
  csStringID type;
  int* data;
  csIntMap () : data (0) { }
  ~csIntMap () { delete[] data; }
};

struct csFloatMap
{
  csStringID type;
  float* data;
  csFloatMap () : data (0) { }
  ~csFloatMap () { delete[] data; }
};

/**
 * This is a simple implementation of a terraformer plugin.
 * It only handles a single heightmap
 */
class csSimpleFormer : public iTerraFormer
{
private:
  /// Object registry pointer (not csRef to avoid cyclic references)
  iObjectRegistry* objectRegistry;

  /// The heightmap to use
  float *heightData;

  /// Additional int maps.
  csArray<csIntMap> intmaps;
  /// Additional float maps.
  csArray<csFloatMap> floatmaps;

  /// Width of the heightmap data array
  unsigned int width;

  /// Height of the heightmap data array
  unsigned int height;

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
  friend class csSimpleSampler;

public:
  SCF_DECLARE_IBASE;

  /// csSimpleFormer constructor
  csSimpleFormer (iBase* parent);

  /// csSimpleFormer destructor
  virtual ~csSimpleFormer ();

  // --------- iSimpleFormerState implementation ---------

  /// Set what heightmap to use
  void SetHeightmap (iImage *heightmap);
  /// Set what heightmap to use directly
  void SetHeightmap (float* data, unsigned int width, unsigned int height);

  /// Set a scaling factor to be used in lookups
  void SetScale (csVector3 scale);

  /// Set an offset vector to be used in lookups
  void SetOffset (csVector3 offset);

  /// Set additional integer map.
  bool SetIntegerMap (csStringID type, iImage* map, int scale, int offset);
  /// Set additional float map.
  bool SetFloatMap (csStringID type, iImage* map, float scale, float offset);

  // Relays calls to the actual class
  struct SimpleFormerState : public iSimpleFormerState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSimpleFormer);
    virtual void SetHeightmap (iImage *heightmap)
    {
      scfParent->SetHeightmap (heightmap);
    }
    virtual void SetHeightmap (float* data, unsigned int width, unsigned int height)
    {
      return scfParent->SetHeightmap (data, width, height);
    }
    virtual void SetScale (csVector3 scale)
    {
      scfParent->SetScale (scale);
    }
    virtual void SetOffset (csVector3 offset)
    {
      scfParent->SetOffset (offset);
    }
    virtual bool SetIntegerMap (csStringID type, iImage* map, int scale = 1,
  	int offset = 0)
    {
      return scfParent->SetIntegerMap (type, map, scale, offset);
    }
    virtual bool SetFloatMap (csStringID type, iImage* map, float scale = 1.0,
  	float offset = 0.0)
    {
      return scfParent->SetFloatMap (type, map, scale, offset);
    }
  } scfiSimpleFormerState;


  // ------------ iTerraFormer implementation ------------

  /// Creates and returns a sampler. See interface for details
  virtual csPtr<iTerraSampler> GetSampler (csBox2 region, 
                                           unsigned int resolution);

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


  // ------------- iComponent implementation -------------

  /// Initializes this object
  bool Initialize (iObjectRegistry* objectRegistry);

  // Relays initialize call to the actual class
  struct Component : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSimpleFormer);
    virtual bool Initialize (iObjectRegistry* objectRegistry)
    {
      return scfParent->Initialize (objectRegistry);
    }
  } scfiComponent;
};

/**
 * This is the accompanying sampler implementation, that pretty much just
 * returns data straight from the heightmap.
 */
class csSimpleSampler : public iTerraSampler
{
private:
  /// Pointer to the terraformer object
  csSimpleFormer* terraFormer;

  /// The region assigned to the sampler
  csBox2 region;

  /// The resolution to use for sampling
  unsigned int resolution;

  /// Cached float data
  float *heights;

  /// Cached positions from the outer edge, used for normal calculations
  csVector3 *edgePositions;

  /// Cached vector2 data
  csVector2 *texCoords;

  /// Cached vector3 data
  csVector3 *positions, *normals;

  /// Distance between sample points in real and heightmap space
  csVector3 sampleDistanceReal, sampleDistanceHeight;

  /// Region corners in heightmap space
  csVector3 minCorner, maxCorner;

private:

  // Calculate and cache positions
  void CachePositions ();

  // Calculate and cache normals
  void CacheNormals ();

  // Calculate and cache heights
  void CacheHeights ();

  // Calculate and cache texture coordinates
  void CacheTexCoords ();

public:
  SCF_DECLARE_IBASE;

  // ------------ iTerraSampler implementation -----------

  /// csSimpleSampler constructor
  csSimpleSampler (csSimpleFormer* terraFormer,
                   csBox2 region, unsigned int resolution);

  /// csSimpleSampler destructor
  virtual ~csSimpleSampler ();

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
  virtual unsigned int GetResolution () const;

  /// Returns 0, since changes aren't allowed in this simple terraformer
  virtual unsigned int GetVersion () const;

  /// Deletes all cached data
  virtual void Cleanup (); 
};

#endif // __CS_SIMPLEFORMER_H__

