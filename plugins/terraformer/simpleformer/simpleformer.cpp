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
  // Allocate height data
  heightData = new float[heightmap->GetWidth ()*heightmap->GetHeight ()];

  // Check what type of image we got
  if (heightmap->GetFormat () & CS_IMGFMT_TRUECOLOR)
  {
    // It's a RGBA image
    csRGBpixel *data = (csRGBpixel *)heightmap->GetImageData ();

    // Keep an index to avoid uneccesary x+y*w calculations
    int idx = 0;

    // Loop through the data
    for (int y=0; y<heightmap->GetHeight (); ++y)
    {
      for (int x=0; x<heightmap->GetWidth (); ++x)
      {
        // Grab the intensity as height
        heightData[idx] = data[idx].Intensity ()/255.0;
        idx++;
      }
    }
  } else if (heightmap->GetFormat () & CS_IMGFMT_PALETTED8)
  {
    // It's a paletted image, so we grab data & palette
    unsigned char *data = (unsigned char*)heightmap->GetImageData ();
    csRGBpixel *palette = heightmap->GetPalette ();

    // Keep an index to avoid uneccesary x+y*w calculations
    int idx = 0;

    // Loop through the data
    for (int y=0; y<heightmap->GetHeight (); ++y)
    {
      for (int x=0; x<heightmap->GetWidth (); ++x)
      {
        // Grab the intensity as height
        heightData[idx] = palette[data[idx]].Intensity ()/255.0;
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

  // Not much can go wrong :)
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

  sampleDistance = 0;
}

csSimpleSampler::~csSimpleSampler ()
{
  // Do a cleanup
  Cleanup ();
}

inline int iround(double x)
{
  return (int)floor(x + 0.5);
}

void csSimpleSampler::CachePositions ()
{
  // Break if we've already cached the data
  if (positions != 0)
    return;

  // Allocate new data
  // We will sample 1 too much in both x and z, for correct normal
  // calculations at region edges
  positions = new csVector3[(resolution+2)*(resolution+2)];
  
  // Compute region corners in heightmap space
  minCorner = csVector3 (region.MinX (), 0, region.MinY ());
  maxCorner = csVector3 (region.MinX (), 0, region.MinY ());
  
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

  // Compute distance between sample points
  sampleDistance = (maxCorner-minCorner)/(float)(resolution-1);

  // Keep an index counter to avoid uneccessary x+y*w calculations
  int idx = 0;

  // Iterate through the samplepoints in the region
  float z = minCorner.z-sampleDistance.z;
  for (unsigned int i=0; i<resolution+2; ++i)
  {
    float x = minCorner.x-sampleDistance.x;
    for (unsigned int j=0; j<resolution+2; ++j)
    {
      unsigned int ix = iround (x);
      unsigned int iz = iround (z);
      
      // Check if we're inside the heightmap (if <0 it will wrap to >size)
      if (ix<terraFormer->width && iz<terraFormer->height)
      {
        // We are, so grab height*scale + offset
        positions[idx++] = 
          csVector3 (x,
                     terraFormer->heightData[ix+iz*terraFormer->width]*
                       terraFormer->scale.y + terraFormer->offset.y,
                     z);
      } else {
        // We're not, so just grab 0 + offset
        positions[idx++] = csVector3 (x, terraFormer->offset.y, z);
      }

      // Step x forward
      x += sampleDistance.x;
    }

    // Step z forward
    z += sampleDistance.z;
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
  int posIdx = resolution+1;

  // Iterate through the samplepoints in the region
  for (unsigned int i = 0; i<resolution; ++i)
  {
    for (unsigned int j = 0; i<resolution; ++j)
    {
      // Calculate two gradient vectors
      csVector3 v1 (sampleDistance.x, 
                    positions[posIdx+1].y-positions[posIdx-1].y,
                    0);
      csVector3 v2 (0, 
                    positions[posIdx+resolution+2].y-
                      positions[posIdx-resolution-2].y, 
                    sampleDistance.z);
      // Cross them and normalize to get a normal
      normals[normIdx++] = (v1%v2).Unit ();
      posIdx++;
    }
    // Skip past outer edges
    posIdx += 2;
  }
}

bool csSimpleSampler::Sample (csStringID type, float* out)
{
  // Check what we're supposed to return
  if (type == terraFormer->stringHeights)
  {
    // Make sure we've got data
    CachePositions ();

    // Keep index counters to avoid uneccessary x+y*w calculations
    int outIdx = 0;
    int posIdx = resolution+1;

    // Iterate through the samplepoints in the region
    for (unsigned int i = 0; i<resolution; ++i)
    {
      for (unsigned int j = 0; i<resolution; ++j)
      {
        out[outIdx++] = positions[posIdx++].y;
      }
      // Skip past outer edges
      posIdx += 2;
    }

    return true;
  } else {
    // Something we can't return was requested
    return false;
  }
}

bool csSimpleSampler::Sample (csStringID type, csVector2* out)
{
  // Check what we're supposed to return
  if (type == terraFormer->stringTexture_Coordinates)
  {
    // Make sure we've got data
    // The positions aren't really needed, but CachePositions
    // calculates some other useful stuff too, so we just assume
    // positions will be needed eventually too
    CachePositions ();

    // Keep index counter to avoid uneccessary x+y*w calculations
    int outIdx = 0;

    // Iterate through the samplepoints in the region
    // Sample texture coordinates as x/z positions in heightmap space
    csVector2 texCoord (minCorner.x, minCorner.z);
    for (unsigned int i = 0; i<resolution; ++i)
    {
      texCoord.x = minCorner.x;
      for (unsigned int j = 0; i<resolution; ++j)
      {
        out[outIdx++] = texCoord;
        texCoord.x += sampleDistance.x;
      }
      texCoord.y += sampleDistance.z;
    }

    return true;
  } else {
    // Something we can't return was requested
    return false;
  }
}

bool csSimpleSampler::Sample (csStringID type, csVector3* out)
{
  // Check what we're supposed to return
  if (type == terraFormer->stringPositions)
  {
    // Make sure we've got data
    CachePositions ();

    // Keep index counters to avoid uneccessary x+y*w calculations
    int outIdx = 0;
    int posIdx = resolution+1;

    // Copy our cached data straight to the output
    for (unsigned int i=0; i<resolution; ++i)
    {
      // Copy a line at once
      memcpy (out+outIdx, positions+posIdx, resolution*sizeof(csVector3));

      // Skip to the next line
      outIdx += resolution;
      posIdx += resolution+2;
    }

    return true;
  } else if (type == terraFormer->stringNormals)
  {
    // Make sure we've got data
    CacheNormals ();

    // This is the most convenient types of them all
    // Our cached data is prepared for direct copying
    memcpy (out, normals, resolution*resolution*sizeof(csVector3));
    return true;
  } else {
    // Something we can't return was requested
    return false;
  }
}

bool csSimpleSampler::Sample (csStringID type, int* out)
{
  // Check what we're supposed to return
  if (type == terraFormer->stringMaterialIndices)
  {
    // This isn't implemented yet. We just return 0
    memset (out, 0, resolution*resolution*sizeof(int));
    return true;
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

void Cleanup ()
{
}


void csSimpleSampler::Cleanup ()
{
  // Clean up allocated data
  delete[] positions;
  positions = 0;

  delete[] normals;
  normals = 0;
}

