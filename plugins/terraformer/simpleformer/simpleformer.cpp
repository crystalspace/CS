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
//                           Filtering helpers
//////////////////////////////////////////////////////////////////////////

// Looks up height with bilinear interpolation
float BiLinearData (float* data, int width, int height, float x, float z)
{
  // Calculate surrounding integer indices
  int lowX = (int) floor (x), lowZ = (int) floor (z);
  int highX = (int) ceil (x), highZ = (int) ceil (z);

  // Clamp coordinates to heightmap size
  lowX = MAX (MIN (lowX, width-1), 0);
  highX = MAX (MIN (highX, width-1), 0);
  lowZ = MAX (MIN (lowZ, height-1), 0);
  highZ = MAX (MIN (highZ, height-1), 0);

  // Grab height data at the four points
  float height1 = 0, height2 = 0, height3 = 0, height4 = 0;
  height1 = data[lowX+lowZ*width];
  height2 = data[highX+lowZ*width];
  height3 = data[lowX+highZ*width];
  height4 = data[highX+highZ*width];

  // Blend between the heights (standard bilinear)
  return height1*(1-(x-lowX))*(1-(z-lowZ))+
         height2*(x-lowX)*(1-(z-lowZ))+
         height3*(1-(x-lowX))*(z-lowZ)+
         height4*(x-lowX)*(z-lowZ);
}

// Bicubic weight function
static float WeightFunction (float x)
{
  float a = (x+2)<0?0:(x+2);
  float b = (x+1)<0?0:(x+1);
  float c = (x+0)<0?0:(x+0);
  float d = (x-1)<0?0:(x-1);
  return (a*a*a - 4*b*b*b + 6*c*c*c - 4*d*d*d)/6.0;
}

// Looks up height with bicubic interpolation
float BiCubicData (float* data, int width, int height, float x, float z)
{
  float result = 0;

  float deltaX = x-floor (x);
  float deltaZ = z-floor (z);

  // If deltaX/Z is close to 0, we're pretty much 
  // right on a heightmap sample point, which means it's useless to blend
// Jorrit: Disabled this because it gives small irregularities.
#if 0
  if (fabs (deltaX) <= SMALL_EPSILON && 
      fabs (deltaZ) <= SMALL_EPSILON)
  {
    int intX = (int) floor (x + 0.5);
    int intZ = (int) floor (z + 0.5);
    intX = MAX (MIN (intX, width-1), 0);
    intZ = MAX (MIN (intZ, height-1), 0);
    return data[(int)intX+(int)intZ*width];
  }
#endif

  int ix = (int) floor (x) - 1;
  int iy = (int) floor (z) - 1;

  // Grab 16 surrounding heights and blend them
  for (int j=0; j<4; ++j)
  {
    // Calculate integer position
    int intZ = iy + j;
    // Clamp coordinates to heightmap size
    if (intZ < 0) intZ = 0;
    else if (intZ > height-1) intZ = height-1;

    // Multiply with width.
    intZ *= width;

    // Precalc weight function for 'j'.
    float jweight = WeightFunction (j-1.0f-deltaZ);

    for (int i=0; i<4; ++i)
    {
      // Calculate integer position
      int intX = ix + i;
      // Clamp coordinates to heightmap size
      if (intX < 0) intX = 0;
      else if (intX > width-1) intX = width-1;

      // Grab the height
      float height = data[intX+intZ];

      // Weight the height using a cubic weight function
      height *= WeightFunction (i-1.0f-deltaX) * jweight;

      // Add the weighted height to the result
      result += height;
    }
  }

  // Return the resulting height
  return result;
}

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

bool csSimpleFormer::SetIntegerMap (csStringID type, iImage* map,
	int scale, int offset)
{
  size_t w = map->GetWidth ();
  size_t h = map->GetHeight ();
  if (width != w || height != h) return false;

  // First check if we already have an intmap of this type.
  size_t intmap_idx = (size_t)~0;
  size_t i = 0;
  while (i < intmaps.Length ())
    if (intmaps[i].type == type)
    {
      intmap_idx = i;
      break;
    }
    else i++;
  if (intmap_idx == (size_t)~0)
    intmap_idx = intmaps.Push (csIntMap ());

  csIntMap& intmap = intmaps[intmap_idx];
  intmap.type = type;

  // Allocate data
  delete[] intmap.data;
  intmap.data = new int[width*height];

  // Check what type of image we got
  if (map->GetFormat () & CS_IMGFMT_TRUECOLOR)
  {
    // We don't support this!
    intmaps.DeleteIndex (intmap_idx);
    return false;
  }
  else if (map->GetFormat () & CS_IMGFMT_PALETTED8)
  {
    // It's a paletted image, so we grab data & palette
    unsigned char *data = (unsigned char*)map->GetImageData ();
    const csRGBpixel *palette = map->GetPalette ();

    // Keep an index to avoid uneccesary x+y*w calculations
    // Start at last line, since we're gonna want it reversed in Y later
    // (negative Y in heightmap image goes to positive Z in terrain)
    //int idx = (height-1)*width;

    unsigned int x, y;
    // Loop through the data
    for (y = 0; y < height; ++y)
    {
      for (x = 0; x < width; ++x)
      {
        // Grab the intensity as height
        // We're reversing Y to later get negative Y in heightmap image 
        // to positive Z in terrain
        intmap.data[x+(height-y-1)*width] = 
          int (palette[data[x+y*width]].Intensity ()) * scale + offset;
      }
    }
  }

  return true;
}

bool csSimpleFormer::SetFloatMap (csStringID type, iImage* map,
	float scale, float offset)
{
  size_t w = map->GetWidth ();
  size_t h = map->GetHeight ();
  if (width != w || height != h) return false;

  // First check if we already have an floatmap of this type.
  size_t floatmap_idx = (size_t)~0;
  size_t i = 0;
  while (i < floatmaps.Length ())
    if (floatmaps[i].type == type)
    {
      floatmap_idx = i;
      break;
    }
    else i++;
  if (floatmap_idx == (size_t)~0)
    floatmap_idx = floatmaps.Push (csFloatMap ());

  csFloatMap& floatmap = floatmaps[floatmap_idx];
  floatmap.type = type;

  // Allocate data
  delete[] floatmap.data;
  floatmap.data = new float[width*height];

  // Check what type of image we got
  if (map->GetFormat () & CS_IMGFMT_TRUECOLOR)
  {
    // It's a RGBA image
    csRGBpixel *data = (csRGBpixel *)map->GetImageData ();

    unsigned int x, y;
    // Loop through the data
    for (y = 0; y < height; ++y)
    {
      for (x = 0; x < width; ++x)
      {
        // Grab the intensity as height
        // We're reversing Y to later get negative Y in heightmap image 
        // to positive Z in terrain
        floatmap.data[x+(height-y-1)*width] = 
          data[x+y*width].Intensity () * scale /255.0 + offset;
      }
    }
  }
  else if (map->GetFormat () & CS_IMGFMT_PALETTED8)
  {
    // It's a paletted image, so we grab data & palette
    unsigned char *data = (unsigned char*)map->GetImageData ();
    const csRGBpixel *palette = map->GetPalette ();

    // Keep an index to avoid uneccesary x+y*w calculations
    // Start at last line, since we're gonna want it reversed in Y later
    // (negative Y in heightmap image goes to positive Z in terrain)
    //int idx = (height-1)*width;

    unsigned int x, y;
    // Loop through the data
    for (y = 0; y < height; ++y)
    {
      for (x = 0; x < width; ++x)
      {
        // Grab the intensity as height
        // We're reversing Y to later get negative Y in heightmap image 
        // to positive Z in terrain
        floatmap.data[x+(height-y-1)*width] = 
          palette[data[x+y*width]].Intensity () * scale / 255.0 + offset;
      }
    }
  }

  return true;
}

void csSimpleFormer::SetHeightmap (float* data, unsigned int width,
	unsigned int height)
{
  csSimpleFormer::width = width;
  csSimpleFormer::height = height;

  delete [] heightData;
  heightData = new float[width*height];
  memcpy (heightData, data, width*height*sizeof(float));
}

void csSimpleFormer::SetHeightmap (iImage *heightmap)
{
  // Grab dimensions
  width = heightmap->GetWidth ();
  height = heightmap->GetHeight ();

  // Allocate height data
  heightData = new float[width*height];

  // Check what type of image we got
  if ((heightmap->GetFormat () & CS_IMGFMT_MASK) == CS_IMGFMT_TRUECOLOR)
  {
    // It's a RGBA image
    csRGBpixel *data = (csRGBpixel *)heightmap->GetImageData ();

    unsigned int x, y;
    // Loop through the data
    for (y = 0; y < height; ++y)
    {
      // Keep an index to avoid uneccesary x+y*w calculations
      uint index1 = (height-y-1)*width;
      uint index2 = y*width;
      for (x = 0; x < width; ++x)
      {
        // Grab height from R,G,B (triplet is effectively a 24-bit number)
        // We're reversing Y to later get negative Y in heightmap image 
        // to positive Z in terrain - sampler space has an inverted Y
	// axis in comparison to image space.
	const csRGBpixel& heixel = data[index2++];
	int h = heixel.red * 65536 + heixel.green * 256 + heixel.blue;
        heightData[index1++] = (float)h/16777215.0;
      }
    }
  }
  else if ((heightmap->GetFormat () & CS_IMGFMT_MASK) == CS_IMGFMT_PALETTED8)
  {
    // It's a paletted image, so we grab data & palette
    unsigned char *data = (unsigned char*)heightmap->GetImageData ();

    unsigned int x, y;
    // Loop through the data
    for (y = 0; y < height; ++y)
    {
      // Keep an index to avoid uneccesary x+y*w calculations
      uint index1 = (height-y-1)*width;
      uint index2 = y*width;
      for (x = 0; x < width; ++x)
      {
        // Grab the index value as height
        // We're reversing Y to later get negative Y in heightmap image 
        // to positive Z in terrain - sampler space has an inverted Y
	// axis in comparison to image space.
        heightData[index1++] = (float)data[index2++] / 255.0;
      }
    }
  }
  else
  {
    // Well...
    memset (heightData, 0, sizeof(float) * width * height);
    csReport (objectRegistry, CS_REPORTER_SEVERITY_WARNING,
      "crystalspace.terraformer.simple", "Odd image format");
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

bool csSimpleFormer::SampleFloat (csStringID type, float x, float z, 
                                  float &value)
{
  if (type == stringHeights)
  {
    // Transform input coordinates to heightmap space.
    // See CachePositions for details
    x = ((x-offset.x)/scale.x+1)*(width/2);
    z = ((z-offset.z)/scale.z+1)*(height/2);

    // Calculate height and return it
    value = BiLinearData (heightData, width, height, x, z) * 
      scale.y + offset.y;
    return true;
  }
  else
  {
    // Check if it is one of the float maps.
    size_t i;
    for (i = 0 ; i < floatmaps.Length () ; i++)
      if (floatmaps[i].type == type)
      {
        // Transform input coordinates to heightmap space.
        // See CachePositions for details
        x = ((x-offset.x)/scale.x+1)*(width/2);
        z = ((z-offset.z)/scale.z+1)*(height/2);

        // Calculate height and return it
        value = BiLinearData (floatmaps[i].data, width, height, x, z);
        return true;
      }
  }
  return false;
}

bool csSimpleFormer::SampleVector2 (csStringID type, float x, float z, 
                                    csVector2 &value)
{
  return false;
}

bool csSimpleFormer::SampleVector3 (csStringID type, float x, float z, 
                                    csVector3 &value)
{
  if (type == stringVertices)
  {
    value = csVector3 (x, 0, z);
    SampleFloat (stringHeights, x, z, value.y);
    return true;
  }
  return false;
}

bool csSimpleFormer::SampleInteger (csStringID type, float x, float z, 
                                    int &value)
{
  {
    // Check if it is one of the int maps.
    size_t i;
    for (i = 0 ; i < intmaps.Length () ; i++)
      if (intmaps[i].type == type)
      {
        // Transform input coordinates to heightmap space.
        // See CachePositions for details
        x = ((x-offset.x)/scale.x+1)*(width/2);
        z = ((z-offset.z)/scale.z+1)*(height/2);

        // Calculate height and return it
        value = intmaps[i].data[int (z) * width + int (x)];
        return true;
      }
  }
  return false;
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
  SCF_CONSTRUCT_IBASE (terraFormer);
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
  SCF_DESTRUCT_IBASE ();
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
      // If we're at the corners, we'll just move along
      if ((i>0 || j>0) && (i>0 || j<resolution+1) &&
          (i<resolution+1 || j>0) && (i<resolution+1 || j<resolution+1))
      {
        // If we're not on the extra edge, store the position in
        // our output buffer, and if we are, store it in the edge buffer
        if (i>0 && i<resolution+1 && j>0 && j<resolution+1)
        {
          positions[posIdx++] = 
            csVector3 (xr,
                        BiCubicData (terraFormer->heightData,
                                     terraFormer->width,
                                     terraFormer->height, xh, zh)*
                        terraFormer->scale.y + terraFormer->offset.y,
                        zr);
        }
	else
	{
          edgePositions[edgeIdx++] = 
            csVector3 (xr,
                        BiCubicData (terraFormer->heightData,
                                     terraFormer->width,
                                     terraFormer->height, xh, zh)*
                          terraFormer->scale.y + terraFormer->offset.y,
                        zr);
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
  const float startx = minCorner.x / (float)(terraFormer->width);
  csVector2 texCoord;
  texCoord.y = minCorner.z / (float)(terraFormer->height);
  const csVector2 tcStep (
    sampleDistanceHeight.x / (float)(terraFormer->width), 
    sampleDistanceHeight.z / (float)(terraFormer->height));
  for (unsigned int i = 0; i<resolution; ++i)
  {
    texCoord.x = startx; 
    for (unsigned int j = 0; j<resolution; ++j)
    {
      // Just assign the texture coordinate
      texCoords[idx++].Set (texCoord.x, 1.0f-texCoord.y); //= texCoord;
      texCoord.x += tcStep.x;
    }
    texCoord.y += tcStep.y;
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
  }
  else
  {
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
  }
  else
  {
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
  }
  else if (type == terraFormer->stringNormals)
  {
    // Make sure we've got data
    CacheNormals ();

    return normals;
  }
  else
  {
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
  }
  else
  {
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
