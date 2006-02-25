/*
    Copyright (C) 2004 Christoph "Fossi" Mewes

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
#include "iutil/vfs.h"
#include "iutil/stringarray.h"

#include "imap/loader.h"

#include <cstool/debugimagewriter.h>
#include <csgfx/memimage.h>

#include "pagingformer.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csPagingFormer)
  SCF_IMPLEMENTS_INTERFACE (iTerraFormer)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPagingFormerState)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csPagingFormer::PagingFormerState)
  SCF_IMPLEMENTS_INTERFACE (iPagingFormerState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csPagingFormer::Component)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csPagingFormer)


csPagingFormer::csPagingFormer (iBase* parent)
{
  // Construct iBases
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiPagingFormerState);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);

  // Initialize members
  objectRegistry = 0;

  scale = csVector3 (1);
  offset = csVector3 (0);
}


csPagingFormer::~csPagingFormer ()
{
  // Delete allocated data
  former = 0;
	
  // Destruct iBases
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiPagingFormerState);
  SCF_DESTRUCT_IBASE();
}


bool csPagingFormer::Initialize (iObjectRegistry* objectRegistry)
{
  // Initialize members
  csPagingFormer::objectRegistry = objectRegistry;

  // Get the shared string repository
  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
    objectRegistry, "crystalspace.shared.stringset", iStringSet);

  // Grab string IDs
  stringVertices = strings->Request ("vertices");
  stringNormals = strings->Request ("normals");
  stringTexture_Coordinates = strings->Request ("texture coordinates");
  stringHeights = strings->Request ("heights");
  stringMaterialIndices = strings->Request ("material indices");

  width = 0;
  height = 0;

  countx = 0;
  county = 0;

  return true;
}


  void csPagingFormer::SetHeightmapDir (const char* path)
  {
    hmdir = path;
    csRef<iVFS> VFS = csQueryRegistry<iVFS> (objectRegistry);
    csRef<iStringArray> heightmapnames = VFS->FindFiles(hmdir);
    
    heightmapnames->Sort();
    csString lastname = csString(heightmapnames->Get(heightmapnames->GetSize()-1));
    size_t ypos = lastname.FindLast('y');
    lastname = lastname.Slice(ypos+1,(lastname.FindLast('.')-ypos)-1);
  
    int numy;
    sscanf(lastname.GetData(),"%d",&numy);
    numy++;

    int numx = heightmapnames->GetSize() / numy;

    printf("popmap: %i/%i\n",numx,numy);

    csRef<iLoader> loader = csQueryRegistry<iLoader> (objectRegistry);
    csRef<iPluginManager> plugin_mgr = 
      csQueryRegistry<iPluginManager> (objectRegistry);

    int i,j;
    for (i = 0; i < numx; i++)
    {
      for (j = 0; j < numy; j++)
      {
        csRef<iTerraFormer> a = csLoadPlugin<iTerraFormer> (plugin_mgr, "crystalspace.terraformer.simple"); 

        csRef<iSimpleFormerState> state = 
          scfQueryInterface<iSimpleFormerState> (a);
        if (state)
        {
          csString fn = csString(path);
          fn += "x";
          fn += i;
          fn += "y";
          fn += j;
          fn += ".png";
          csRef<iImage> map = loader->LoadImage (fn, CS_IMGFMT_ANY);
          if (map)
          {
            state->SetHeightmap(map);
            width += map->GetWidth();
            height += map->GetHeight();
printf("loaded %s\n",fn.GetData());
          } else { exit(2000); }
        } else { exit(1000); }
      
        former.Push(a);
      }
    }

    countx = numx;
    county = numy;
printf("final width:%i height:%i\n",width,height);
  }


  /// Set a scaling factor to be used in lookups
  void csPagingFormer::SetScale (csVector3 scale)
  {
printf("SetScale\n");

    csRefArray<iTerraFormer>::Iterator it = former.GetIterator();
    while(it.HasNext())
    {
      csRef<iSimpleFormerState> state =       
        scfQueryInterface<iSimpleFormerState> (it.Next());
      if (state) state->SetScale(scale);
    }

    csPagingFormer::scale = scale;
  }


  /// Set an offset vector to be used in lookups
  void csPagingFormer::SetOffset (csVector3 offset)
  {
printf("SetOffset\n");

    csRefArray<iTerraFormer>::Iterator it = former.GetIterator();
    while(it.HasNext())
    {
      csRef<iSimpleFormerState> state =
        scfQueryInterface<iSimpleFormerState> (it.Next());
      if (state) state->SetOffset(offset);
    }

    csPagingFormer::offset = offset;
  }


  /// Set additional integer map.
  bool csPagingFormer::SetIntegerMap (csStringID type, iImage* map,
    int scale, int offset)
  {
    printf("SetIntMap\n");
    return false;
  }


  /// Set additional float map.
  bool csPagingFormer::SetFloatMap (csStringID type, iImage* map,
    float scale, float offset)
  {
    printf("SetFloatMap\n");
    return false;
  }


  // ------------ iTerraFormer implementation ------------



  /// Creates and returns a sampler. See interface for details
  csPtr<iTerraSampler> csPagingFormer::GetSampler (csBox2 region, 
    unsigned int resolution)
  {
printf("\n");
//printf("GetSampler\n");
    csRefArray<iTerraSampler> sampler;

    //determine the min and max corners in heightmap space
    float x1 = region.MinX();
    float z1 = region.MinY();
    float x2 = region.MaxX();
    float z2 = region.MaxY();

printf("Min: %f:%f Max: %f:%f\n", x1,z1,x2,z2);

    x1 = ((x1-offset.x)/scale.x+1)*(width/2);
    z1 = ((z1-offset.z)/scale.z+1)*(height/2);

    x2 = ((x2-offset.x)/scale.x+1)*(width/2);
    z2 = ((z2-offset.z)/scale.z+1)*(height/2);

printf("Min: %f:%f Max: %f:%f\n", x1,z1,x2,z2);

    //calculate the size of one formers terrain
    int sizex = (int)round(width/countx);
    int sizey = (int)round(height/county);

    int i,j;
    // go through the formers we need
    // and calculate the intersecting regions.
    for (i = (int)x1/(sizex); i < ceil(x2/sizex); i++)
    {
      for (j = (int)z1/(sizey); j < ceil(z2/sizey); j++)
      {
    
printf("x:%i,y:%i\n",i,j);
        csBox2 translated = csBox2 (x1,z1,x2,z2);
        csBox2 formerregion = csBox2 (i*sizex, j*sizey,
                                     (i+1)*sizex, (j+1)*sizey );

        uint samplerresolution;

        // how much of the whole region is covered by this former

        // @@@ sooooo wrong: because area says nothing about position
        // replace with intersection & cases like in Sampler

	if (fabs(formerregion.Area() - translated.Area()) > SMALL_EPSILON) //!=
        {
          // formers covers only a part, scale resolution accordingly
          float formerdist = formerregion.MaxX() - formerregion.MinX();
          float transdist = translated.MaxX() - translated.MinX();
          float perc = formerdist / transdist;
          float res = resolution * perc;
         
          samplerresolution = ceil(res);
/*
printf("%f\n", formerdist);
printf("%f\n", transdist);
printf("%f\n", perc);
printf("%f\n", res);
printf("%i\n", samplerresolution);
*/
        }
        else
        {
          // if requested region is covered by one whole former,
          // simply return it directly.
          samplerresolution = resolution;
        }

printf("res: %i->%i\n",resolution,samplerresolution);


        //translate the needed region back into worldspace
        csBox2 samplerregion = csBox2 (
          (formerregion.MinX()/(width/2)-1)*scale.x+offset.x,
          (formerregion.MinY()/(height/2)-1)*scale.z+offset.z,
          (formerregion.MaxX()/(width/2)-1)*scale.x+offset.x,
          (formerregion.MaxY()/(height/2)-1)*scale.z+offset.z
        );

        sampler.Push (csRef<iTerraSampler>
          (former[i * countx + j]->GetSampler(samplerregion, samplerresolution)));
      }
    }

    // devide length of requested region by formersize
    // to get number of formers in x direction
    uint formercountx = (uint)ceil((z2-z1)/sizey);
    return new csPagingSampler(this, sampler, formercountx, 
      region, resolution);
  }


  /**
   * Sample float data.
   * Allowed types:
   * heights
   */
  bool csPagingFormer::SampleFloat (csStringID type, float x, 
    float z, float &value)
  {
    printf("SampleFloat Former\n");
    return false;
  }


  /**
   * Sample csVector2 data.
   * No allowed types (will return false)
   */
  bool csPagingFormer::SampleVector2 (csStringID type, float x, float z,
    csVector2 &value)
  {
    printf("SampleVec2 Former\n");
    return false;
  }


  /**
   * Sample csVector3 data.
   * Allowed types:
   * vertices
   */
  bool csPagingFormer::SampleVector3 (csStringID type, float x, 
float z,
    csVector3 &value)
  {
    printf("SampleVec3 Former\n");
    return false;
  }


  /**
   * Sample integer data.
   * No allowed types (will return false)
   */
  bool csPagingFormer::SampleInteger (csStringID type, float x, float z,
    int &value)
  {
    printf("SampleInt Former\n");
    return false;
  }


//////////////////////////////////////////////////////////////////////////
//                            csPagingSampler
//////////////////////////////////////////////////////////////////////////


SCF_IMPLEMENT_IBASE (csPagingSampler)
  SCF_IMPLEMENTS_INTERFACE (iTerraSampler)
SCF_IMPLEMENT_IBASE_END

csPagingSampler::csPagingSampler (csPagingFormer* former,
 csRefArray<iTerraSampler> sampler, uint xcount, 
                                  csBox2 region, unsigned int resolution)
{
  SCF_CONSTRUCT_IBASE (former);
  // Initialize members
  csPagingSampler::terraFormer = former;
  csPagingSampler::sampler = sampler;
  csPagingSampler::region = region;
  csPagingSampler::resolution = resolution;
  csPagingSampler::xcount = xcount;

  positions = 0;
  normals = 0;
  texCoords = 0;
  heights = 0;
}


csPagingSampler::~csPagingSampler ()
{
  // Do a cleanup
  Cleanup ();
  SCF_DESTRUCT_IBASE ();
}


// Calculate and cache positions
void csPagingSampler::CachePositions ()
{
  if (positions != 0) return;

  // get memory for our cache
  positions = new csVector3[resolution*resolution];

bool smaller= false;

  // buffer for all the samplers maps
  uint num = sampler.GetSize();
  const csVector3 **maps = new const csVector3*[num];

  // first get the raw data from all the samplers
  // @@@ excluded from loop below for profiling
  uint k;
  for (k = 0; k < num; k++)
  {
      maps[k] = sampler.Get(k)->SampleVector3(terraFormer->stringVertices);
  }

  // then loop about all samplers again to copy needed data
  for (k = 0; k < num; k++)
  {
      csBox2 partialregion = sampler.Get(k)->GetRegion();
      uint partialresolution = sampler.Get(k)->GetResolution();

      uint fromx, fromy;  // array coordinates we copy from
      uint tox, toy;  // array coordinates we copy to
      uint copynum;  // number of elements we need to copy per step
      uint fromstep; // number of elements we need to skip per step
      uint tostep; // number of elements we need to skip per step
      
      if (fabs(partialregion.Area() - region.Area()) < SMALL_EPSILON) // ==
      {
printf("!equal\n");
 
        // this sampler has the whole data, just copy away
        // @@@ optimize copying, return

        fromx = 0;
        fromy = 0;
        tox = 0;
        toy = 0;
        copynum = partialresolution;
        fromstep = partialresolution;
        tostep = partialresolution;

assert(resolution == partialresolution);

      } else {
        // the request is either bigger or smaller then this sampler

        // calculate which part of the whole we deal with
        // and translate into heightmap coordinates
        csBox2 intersect = partialregion * region;

        float intersectminx = intersect.MinX();
        float intersectminy = intersect.MinY();
          intersectminx = ((intersectminx-terraFormer->offset.x)/terraFormer->scale.x +1)
            *(terraFormer->width/2);
          intersectminy = ((intersectminy-terraFormer->offset.z)/terraFormer->scale.z +1)
            *(terraFormer->height/2);


        if (region.Area() > intersect.Area())
        {
printf("!bigger\n");
          // the region is bigger, so this is only a part of it

          // translate the request's max coordinates because we need them later on
          float regionmaxx = region.MaxX();
          float regionmaxy = region.MaxY();
            regionmaxx = ((regionmaxx-terraFormer->offset.x)/terraFormer->scale.x +1)
              *(terraFormer->width/2);
            regionmaxy = ((regionmaxy-terraFormer->offset.z)/terraFormer->scale.z +1)
              *(terraFormer->height/2);

          // we need to copy this sampler's whole data
          fromx = 0;
          fromy = 0;
          copynum = partialresolution;
          fromstep = partialresolution;
          tostep = resolution;

printf("%f %f %i\n",intersectminx,regionmaxx,resolution);
printf("%f %f %i\n",intersectminy,regionmaxy,resolution);

          // to this former's relative position to the whole request
          tox = ceil((intersectminx/regionmaxx) *resolution);
          toy = ceil((intersectminy/regionmaxy) *resolution);
        }
        else
        {
//smaller = true;
printf("!smaller\n");
          // the region is smaller so it's only part of this sampler's data

        // translate the request's max coordinates because we need them later on
        float partialregionmaxx = partialregion.MaxX();
        float partialregionmaxy = partialregion.MaxY();
        float partialregionminx = partialregion.MinX();
        float partialregionminy = partialregion.MinY();
          partialregionmaxx = ((partialregionmaxx-terraFormer->offset.x)/terraFormer->scale.x +1)
            *(terraFormer->width/2);
          partialregionmaxy = ((partialregionmaxy-terraFormer->offset.z)/terraFormer->scale.z +1)
            *(terraFormer->height/2);
          partialregionminx = ((partialregionminx-terraFormer->offset.x)/terraFormer->scale.x +1)
            *(terraFormer->width/2);
          partialregionminy = ((partialregionminy-terraFormer->offset.z)/terraFormer->scale.z +1)
            *(terraFormer->height/2);

//printf("res: %i part: %i\n", resolution, partialresolution);
//printf("%f %f\n",intersectminx,partialregionmaxx);
//printf("%f %f\n",intersectminy,partialregionmaxy);

          // copy from the intersection's relative coordinates
          fromx = ceil(((intersectminx-partialregionminx)/(partialregionmaxx-partialregionminx)) *partialresolution);
          fromy = ceil(((intersectminy-partialregionminy)/(partialregionmaxy-partialregionminy)) *partialresolution);

          tox = 0;
          toy = 0;

          // we just need to copy a part of this sampler's data
          copynum = resolution;
          fromstep = partialresolution;
          tostep = resolution;
        }
      }

printf("fx: %i\n",fromx);
printf("fy: %i\n",fromy);
printf("tx: %i\n",tox);
printf("ty: %i\n",toy);
printf("cx: %i\n",copynum);
printf("fs: %i\n",fromstep);
printf("ts: %i\n",tostep);

      // loop over the part we need to copy
      uint l;
      for (l = 0; l < copynum ; l++ )
      {
        //in case of several formers the copied data needs to overlap
        //since we sample a little too much data for each
        //when the requested region's size is not a multiple of their count
        //we do this by overwriting the neighboring former's last line

        uint topos = (toy+l-(k%xcount))*tostep+tox-(k/xcount);
        uint frompos = (fromy+l)*fromstep+fromx;

//printf("%i: %i -> %i\n", l, frompos, topos);

        memcpy(positions + topos,
          maps[k] + frompos,
          copynum*sizeof(csVector3));
      }
    }
if (smaller) {
  csImageMemory pic = csImageMemory(resolution, resolution);
  csRGBpixel* dat = (csRGBpixel*)pic.GetImagePtr();
  for (int asdf=0; asdf < resolution*resolution; asdf++) {
    dat[asdf].red = positions[asdf].y;
    dat[asdf].green = positions[asdf].y;
    dat[asdf].blue = positions[asdf].y;
  }
  csDebugImageWriter a = csDebugImageWriter();
  csString fn = csString();
  fn += "x";
  fn += region.MinX();
  fn += "y";
  fn += region.MinY();
  fn += ".png";
  a.DebugImageWrite(&pic,fn);
printf(fn);
}
}


// Calculate and cache normals
void csPagingSampler::CacheNormals ()
{
}


// Calculate and cache heights
void csPagingSampler::CacheHeights ()
{
}


// Calculate and cache texture coordinates
void CacheTexCoords ()
{
}


const float *csPagingSampler::SampleFloat (csStringID type)
{
//printf("SampleFloat Sampler\n");

  // Check what we're supposed to return
  if (type == terraFormer->stringHeights)
  {

//printf("Sample Height Sampler\n");

    return sampler.Top()->SampleFloat(type);
  }
  else
  {
    // Something we can't return was requested
    return 0;
  }
}


const csVector2 *csPagingSampler::SampleVector2 (csStringID type)
{
//printf("SampleVec2 Sampler\n");
  // Check what we're supposed to return
  if (type == terraFormer->stringTexture_Coordinates)
  {
//printf("SampleTCs Sampler\n");
    return sampler.Top()->SampleVector2(type);
  }
  else
  {
    // Something we can't return was requested
    return 0;
  }
}


const csVector3 *csPagingSampler::SampleVector3 (csStringID type)
{
//printf("SampleVec3 Sampler\n");
  // Check what we're supposed to return
  if (type == terraFormer->stringVertices)
  {
//printf("SampleVert Sampler\n");

    CachePositions();    
    return positions;
  }
  else if (type == terraFormer->stringNormals)
  {
//printf("SampleNorm Sampler\n");
    return sampler.Top()->SampleVector3(type);
  }
  else
  {
    // Something we can't return was requested
    return 0;
  }
}


const int *csPagingSampler::SampleInteger (csStringID type)
{
//printf("SampleMaterial Sampler\n");
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


const csArray<iMaterialWrapper*> &csPagingSampler::GetMaterialPalette ()
{
//printf("GetMaterial Sampler\n");
  // Just return the palette
  return terraFormer->materialPalette;
}


const csBox2 &csPagingSampler::GetRegion () const
{
  // Just return the region
  return region;
}


unsigned int csPagingSampler::GetResolution () const
{
  // Just return the resolution
  return resolution;
}


unsigned int csPagingSampler::GetVersion () const
{
  // Just return 0, since we won't allow changes anyway
  return 0;
}


void csPagingSampler::Cleanup ()
{
  csRefArray<iTerraSampler>::Iterator it = sampler.GetIterator();
  while(it.HasNext())
  {
    it.Next()->Cleanup();
  }
  
  // Clean up allocated data
  delete[] positions;
  positions = 0;

  delete[] normals;
  normals = 0;

  delete[] heights;
  heights = 0;

  delete[] texCoords;
  texCoords = 0;
}
