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

#include "cssysdef.h"

#include "csgfx/rgbpixel.h"

#include "igraphic/image.h"

#include "iutil/objreg.h"
#include "iutil/plugin.h"

#include "ivaria/reporter.h"

#include "simpleformer.h"

CS_IMPLEMENT_PLUGIN

//////////////////////////////////////////////////////////////////////////
//                             csSimpleFormer
//////////////////////////////////////////////////////////////////////////

SCF_IMPLEMENT_IBASE (csSimpleFormer)
SCF_IMPLEMENTS_INTERFACE (iTerraFormer)
SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iSimpleFormerState)
SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSimpleFormer::SimpleFormerState)
SCF_IMPLEMENTS_INTERFACE (iSimpleFormerState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSimpleFormer::Component)
SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csSimpleFormer)

csSimpleFormer::csSimpleFormer (iBase* parent)
{
  // Construct iBases
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiSimpleFormerState);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);

  // Initialize members
  objectRegistry = 0;
  heightData = 0;

  scale = csVector3 (1);
  offset = csVector3 (0);
}

csSimpleFormer::~csSimpleFormer ()
{
  // Delete allocated data
  delete [] heightData;

  // Destruct iBases
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiSimpleFormerState);
  SCF_DESTRUCT_IBASE();
}

void csSimpleFormer::SetHeightmap (iImage *heightmap)
{
  // Grab dimensions
  width = heightmap->GetWidth ();
  height = heightmap->GetHeight ();

  // Allocate height data
  heightData = new float[width*height];

  // Check what type of image we got
  if (heightmap->GetFormat () & CS_IMGFMT_TRUECOLOR)
  {
    // It's a RGBA image
    csRGBpixel *data = (csRGBpixel *)heightmap->GetImageData ();

    // Keep an index to avoid uneccesary x+y*w calculations
    int idx = 0;

    unsigned int x, y;
    // Loop through the data
    for (y = 0; y < height; ++y)
    {
      for (x = 0; x < width; ++x)
      {
        // Grab the intensity as height
        heightData[idx] = data[idx].Intensity ()/255.0;
        idx++;
      }
    }
  }
  else if (heightmap->GetFormat () & CS_IMGFMT_PALETTED8)
  {
    // It's a paletted image, so we grab data & palette
    unsigned char *data = (unsigned char*)heightmap->GetImageData ();
    csRGBpixel *palette = heightmap->GetPalette ();

    // Keep an index to avoid uneccesary x+y*w calculations
    int idx = 0;

    unsigned int x, y;
    // Loop through the data
    for (y = 0; y < height; ++y)
    {
      for (x = 0; x < width; ++x)
      {
        // Grab the intensity as height
        heightData[idx] = palette[data[idx]].Intensity () / 255.0;
        idx++;
      }
    }
  }
}

void csSimpleFormer::SetScale (csVector3 scale)
{
  csSimpleFormer::scale = scale;
}

void csSimpleFormer::SetOffset (csVector3 offset)
{
  csSimpleFormer::offset = offset;
}

bool csSimpleFormer::Initialize (iObjectRegistry* objectRegistry)
{
  // Initialize members
  csSimpleFormer::objectRegistry = objectRegistry;

  // Get the shared string repository
  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
    objectRegistry, "crystalspace.shared.stringset", iStringSet);

  // Grab string IDs
  stringVertices = strings->Request ("vertices");
  stringNormals = strings->Request ("normals");
  stringTexture_Coordinates = strings->Request ("texture coordinates");
  stringHeights = strings->Request ("heights");
  stringMaterialIndices = strings->Request ("material indices");

  return true;
}

csPtr<iTerraSampler> csSimpleFormer::GetSampler (csBox2 region, 
                                                 unsigned int resolution)
{
  // Create a new sampler and return it
  return new csSimpleSampler (this, region, resolution);
}


//////////////////////////////////////////////////////////////////////////
//                            csSimpleSampler
//////////////////////////////////////////////////////////////////////////

SCF_IMPLEMENT_IBASE (csSimpleSampler)
SCF_IMPLEMENTS_INTERFACE (iTerraSampler)
SCF_IMPLEMENT_IBASE_END

csSimpleSampler::csSimpleSampler (csSimpleFormer *terraFormer,
                                  csBox2 region, unsigned int resolution)
{
  // Initialize members
  csSimpleSampler::terraFormer = terraFormer;
  csSimpleSampler::region = region;
  csSimpleSampler::resolution = resolution;
  
  positions = 0;
  normals = 0;
  texCoords = 0;
  heights = 0;
  edgePositions = 0;

  sampleDistanceReal = 0;
  sampleDistanceHeight = 0;
}

csSimpleSampler::~csSimpleSampler ()
{
  // Do a cleanup
  Cleanup ();
}

inline int iround(double x)
{
  return (int)floor(x);
}

void csSimpleSampler::CachePositions ()
{
  // Break if we've already cached the data
  if (positions != 0)
    return;

  // Allocate new data
  // We will sample 1 too much in both x and z, for correct normal
  // calculations at region edges
  positions = new csVector3[resolution*resolution];
  edgePositions = new csVector3 [resolution*4];

  // Compute region corners
  minCorner = csVector3 (region.MinX (), 0, region.MinY ());
  maxCorner = csVector3 (region.MaxX (), 0, region.MaxY ());

  // Compute distance between sample points
  sampleDistanceReal = (maxCorner-minCorner)/(float)(resolution-1);

  // We wanna compute our region in heightmap space
  // Heightmap -> real space is computed by:
  // 1. Divide by size/2 to get to 0..2
  // 2. Subtract by 1 to get to -1..1
  // 3. Multiply by scale
  // 4. Add offset
  // We're now doing it all backwards to get to heightmap space.

  // 4. Add offset
  minCorner -= terraFormer->offset;
  maxCorner -= terraFormer->offset;

  // 3. Multiply by scale
  minCorner.x /= terraFormer->scale.x;
  maxCorner.x /= terraFormer->scale.x;
  minCorner.y /= terraFormer->scale.y;
  maxCorner.y /= terraFormer->scale.y;
  minCorner.z /= terraFormer->scale.z;
  maxCorner.z /= terraFormer->scale.z;

  // 2. Subtract by 1 to get to -1..1
  minCorner += csVector3 (1, 0, 1);
  maxCorner += csVector3 (1, 0, 1);

  // 1. Divide by size/2 to get to 0..2
  minCorner.x *= (float)terraFormer->width/2;
  maxCorner.x *= (float)terraFormer->width/2;
  minCorner.z *= (float)terraFormer->height/2;
  maxCorner.z *= (float)terraFormer->height/2;

  // Compute distance between sample points in heightmap space
  sampleDistanceHeight = (maxCorner-minCorner)/(float)(resolution-1);

  // Keep index counters to avoid uneccessary x+y*w calculations
  int posIdx = 0, edgeIdx = 0;

  // Iterate through the samplepoints in the region
  float zr = region.MinY ()-sampleDistanceReal.z;
  float zh = minCorner.z-sampleDistanceHeight.z;
  for (unsigned int i=0; i<resolution+2; ++i)
  {
    float xr = region.MinX ()-sampleDistanceReal.x;
    float xh = minCorner.x-sampleDistanceHeight.x;
    for (unsigned int j=0; j<resolution+2; ++j)
    {
      unsigned int ix = iround (xh);
      unsigned int iz = iround (zh);
      
      // If we're at the corners, we'll just move along
      if ((i>0 || j>0) && (i>0 || j<resolution+1) &&
          (i<resolution+1 || j>0) && (i<resolution+1 || j<resolution+1))
      {
        // Check if we're inside the heightmap (if <0 it will wrap to >size)
        if (ix<terraFormer->width && iz<terraFormer->height)
        {
          // If we're not on the extra edge, store the position in
          // our output buffer, and if we are, store it in the edge buffer
          if (i>0 && i<resolution+1 && j>0 && j<resolution+1)
          {
            positions[posIdx++] = 
              csVector3 (xr,
                        terraFormer->heightData[ix+iz*terraFormer->width]*
                          terraFormer->scale.y + terraFormer->offset.y,
                         zr);
          } else {
            edgePositions[edgeIdx++] = 
              csVector3 (xr,
                        terraFormer->heightData[ix+iz*terraFormer->width]*
                          terraFormer->scale.y + terraFormer->offset.y,
                         zr);
          }
        } else {
          // If we're not on the extra edge, store the position (with height
          // 0 since we're outside the heightmap) in our output buffer, and 
          // if we are, store it in the edge buffer
          if (i>0 && i<resolution+1 && j>0 && j<resolution+1)
          {
            positions[posIdx++] = 
              csVector3 (xr, terraFormer->offset.y, zr);
          } else {
            edgePositions[edgeIdx++] = 
              csVector3 (xr, terraFormer->offset.y, zr);
          }
        }
      }

      // Step x forward
      xr += sampleDistanceReal.x;
      xh += sampleDistanceHeight.x;
    }

    // Step z forward
    zr += sampleDistanceReal.z;
    zh += sampleDistanceHeight.z;
  }
}

void csSimpleSampler::CacheNormals ()
{
  // Break if we've already cached the data
  if (normals != 0)
    return;

  // Allocate new data
  normals = new csVector3[resolution*resolution];

  // Make sure we've got some position data to base the normals on
  CachePositions ();

  // Keep index counters to avoid uneccessary x+y*w calculations
  int normIdx = 0;
  int posIdx = 0;

  // Intermediate vectors
  csVector3 v1, v2;

  // Iterate through the samplepoints in the region
  for (unsigned int i = 0; i<resolution; ++i)
  {
    for (unsigned int j = 0; j<resolution; ++j)
    {
      // Calculate two gradient vectors
      // Conditionals check wheter to fetch from edge data
      v1 = j==resolution-1?
              edgePositions[1+resolution+i*2]:positions[posIdx+1];
      v1 -= j==0?
              edgePositions[resolution+i*2]:positions[posIdx-1];

      v2 = i==resolution-1?
        edgePositions[resolution*3+j]:positions[posIdx+resolution];
      v2 -= i==0?
        edgePositions[j]:positions[posIdx-resolution];

      // Cross them and normalize to get a normal
      normals[normIdx++] = (v2%v1).Unit ();
      posIdx++;
    }
  }

  // We've got our normals, so we can get rid of the edges
  delete[] edgePositions;
  edgePositions = 0;
}

void csSimpleSampler::CacheHeights ()
{
  // Break if we've already cached the data
  if (heights != 0)
    return;

  // Allocate new data
  heights = new float[resolution*resolution];

  // Make sure we've got some position data to base the normals on
  CachePositions ();

  // Keep index counters to avoid uneccessary x+y*w calculations
  int idx = 0;

  // Iterate through the samplepoints in the region
  for (unsigned int i = 0; i<resolution; ++i)
  {
    for (unsigned int j = 0; j<resolution; ++j)
    {
      heights[idx] = positions[idx].y;
      idx++;
    }
  }
}

void csSimpleSampler::CacheTexCoords ()
{
  // Break if we've already cached the data
  if (texCoords != 0)
    return;

  // Allocate new data
  texCoords = new csVector2[resolution*resolution];

  // The positions aren't really needed, but CachePositions
  // calculates some other useful stuff too, so we just assume
  // positions will be needed eventually too
  CachePositions ();

  // Keep index counter to avoid uneccessary x+y*w calculations
  int idx = 0;

  // Iterate through the samplepoints in the region
  // Sample texture coordinates as x/z positions in heightmap space
  csVector2 texCoord (minCorner.x, minCorner.z);
  for (unsigned int i = 0; i<resolution; ++i)
  {
    texCoord.x = minCorner.x;
    for (unsigned int j = 0; j<resolution; ++j)
    {
      // Just assign the texture coordinate
      texCoords[idx++] = texCoord;
      texCoord.x += sampleDistanceHeight.x;
    }
    texCoord.y += sampleDistanceHeight.z;
  }
}

const float *csSimpleSampler::SampleFloat (csStringID type)
{
  // Check what we're supposed to return
  if (type == terraFormer->stringHeights)
  {
    // Make sure we've got data
    CacheHeights ();

    return heights;
  } else {
    // Something we can't return was requested
    return 0;
  }
}

const csVector2 *csSimpleSampler::SampleVector2 (csStringID type)
{
  // Check what we're supposed to return
  if (type == terraFormer->stringTexture_Coordinates)
  {
    // Make sure we've got data
    CacheTexCoords ();

    return texCoords;
  } else {
    // Something we can't return was requested
    return 0;
  }
}

const csVector3 *csSimpleSampler::SampleVector3 (csStringID type)
{
  // Check what we're supposed to return
  if (type == terraFormer->stringVertices)
  {
    // Make sure we've got data
    CachePositions ();

    return positions;
  } else if (type == terraFormer->stringNormals)
  {
    // Make sure we've got data
    CacheNormals ();

    return normals;
  } else {
    // Something we can't return was requested
    return 0;
  }
}

const int *csSimpleSampler::SampleInteger (csStringID type)
{
  // Check what we're supposed to return
  if (type == terraFormer->stringMaterialIndices)
  {
    // This isn't implemented yet. We just return 0
    return 0;
  } else {
    // Something we can't return was requested
    return false;
  }
}

const csArray<iMaterialWrapper*> &csSimpleSampler::GetMaterialPalette ()
{
  // Just return the palette
  return terraFormer->materialPalette;
}

const csBox2 &csSimpleSampler::GetRegion () const
{
  // Just return the region
  return region;
}

unsigned int csSimpleSampler::GetResolution () const
{
  // Just return the resolution
  return resolution;
}

unsigned int csSimpleSampler::GetVersion () const
{
  // Just return 0, since we won't allow changes anyway
  return 0;
}

void csSimpleSampler::Cleanup ()
{
  // Clean up allocated data
  delete[] positions;
  positions = 0;

  delete[] normals;
  normals = 0;

  delete[] heights;
  heights = 0;

  delete[] texCoords;
  texCoords = 0;

  delete[] edgePositions;
  edgePositions = 0;
}
