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

#include "cstool/debugimagewriter.h"
#include "csgfx/memimage.h"

#include "pagingformer.h"

CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(PagingFormer)
{

SCF_IMPLEMENT_FACTORY (csPagingFormer)

#define PAGING_MAX_FORMERS 16

csPagingFormer::csPagingFormer (iBase* parent)
  : scfImplementationType (this, parent)
{
  // Initialize members
  objectRegistry = 0;

  scale = csVector3 (1);
  offset = csVector3 (0);
}


csPagingFormer::~csPagingFormer ()
{
  //@@@ delete allocated data
  former = 0;
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


  void csPagingFormer::LoadFormer (uint x, uint y)
  {
    csRef<iLoader> loader = csQueryRegistry<iLoader> (objectRegistry);
    csRef<iPluginManager> plugin_mgr = 
      csQueryRegistry<iPluginManager> (objectRegistry);


    csRef<iTerraFormer> a = csLoadPlugin<iTerraFormer> (plugin_mgr, "crystalspace.terraformer.simple"); 

    csRef<iSimpleFormerState> state = 
      scfQueryInterface<iSimpleFormerState> (a);
        if (state)
        {
          csString fn = csString(hmdir);
          fn += "x";
          fn += x;
          fn += "y";
          fn += y;
          fn += ".png";
          csRef<iImage> map = loader->LoadImage (fn, CS_IMGFMT_ANY);
          if (map)
          {
            state->SetHeightmap(map);
            if (width==0)
            {
              width = countx * map->GetWidth();
              height = county * map->GetHeight();
            }
printf("loaded %s\n",fn.GetData());

            state->SetScale(csVector3(scale.x/countx, scale.y, scale.z/county));
            state->SetOffset(offset + csVector3(
              (2*x*(scale.x/countx)-scale.x)+scale.x/countx,
              0,
              (2*y*(scale.z/county)-scale.z)+scale.z/county
            ));
printf("%f %f\n",2*x*(scale.x/countx)-scale.x,2*y*(scale.z/county)-scale.z);
          } else { exit(2000); } // @@@
        } else { exit(1000); }

        former[x * county + y] = a;
  }


  void csPagingFormer::SetHeightmapDir (const char* path)
  {
    hmdir = csStrNew(path);

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
    countx = numx;
    county = numy;

    printf("popmap: %i/%i\n",numx,numy);

    former = new csRef<iTerraFormer>[numx*numy];
    memset (former, 0, numx*numy);

    LoadFormer(0,0); // @@@ needed for width/height, set in mapfile?
  }


  /// Set a scaling factor to be used in lookups
  void csPagingFormer::SetScale (csVector3 newscale)
  {
printf("SetScale\n");

    scale = newscale;

    for (uint i = 0; i < (countx*county); i++)
    {
      if (former[i] != 0)
      {
        csRef<iSimpleFormerState> state =       
          scfQueryInterface<iSimpleFormerState> (former[i]);
        if (state)
        {
          csVector3 applied = csVector3(newscale.x/countx, newscale.y, newscale.z/county);
          state->SetScale(applied);
          state->SetOffset(offset + csVector3(
            (2*(i%countx)*(scale.x/countx)-scale.x)+scale.x/countx,
            0,
            (2*(i/countx)*(scale.z/county)-scale.z)+scale.z/county
          ));
printf("&SetScale: %i\n", i);

        }
      }
    }
  }


  /// Set an offset vector to be used in lookups
  void csPagingFormer::SetOffset (csVector3 offset)
  {
printf("!WARNING: SetOffset is still broken!\n");

    for (uint i = 0; i < (countx*county)-1; i++)
    {
      csRef<iSimpleFormerState> state =
        scfQueryInterface<iSimpleFormerState> (former[i]);
      if (state)
          state->SetOffset(offset + csVector3(
            2*(i%countx)*(scale.x/countx)-scale.x-scale.x,
            0,
            2*(i/countx)*(scale.z/county)-scale.z-scale.z
          ));
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
                                       uint resx, uint resz)
  {
    if (resz == 0) resz = resx;

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
    int sizex = (int)(width/countx + 0.5f);
    int sizey = (int)(height/county + 0.5f);

    int i,j;
    // go through the formers we need
    // and calculate the intersecting regions.
    for (i = (int)x1/(sizex); i < ceil(x2/sizex); i++)
    {
      for (j = (int)z1/(sizey); j < ceil(z2/sizey); j++)
      {
printf("-------------------------\n");
        if (former[i * countx + j] == 0)
        {
printf("*loading former: %i %i: ",i,j);
          LoadFormer(i,j);
        }
    
printf("x:%i,y:%i\n",i,j);
        csBox2 translated = csBox2 (x1,z1,x2,z2);
        csBox2 formerregion = csBox2 (i*sizex, j*sizey,
                                     (i+1)*sizex, (j+1)*sizey );

printf("Min: %f:%f Max: %f:%f\n", translated.MinX(),translated.MinY(),translated.MaxX(),translated.MaxY());
printf("Min: %f:%f Max: %f:%f\n", formerregion.MinX(),formerregion.MinY(),formerregion.MaxX(),formerregion.MaxY());

      // optimization for simplest cases
        if (formerregion.Contains(translated))
        {
printf("*contained\n");

          // just relay the request to the former
          csBox2 samplerregion = csBox2(
            ceil(region.MinX()),
            ceil(region.MinY()),
            ceil(region.MaxX()),
            ceil(region.MaxY())
          );

printf("Min: %f:%f Max: %f:%f\n", samplerregion.MinX(),samplerregion.MinY(),samplerregion.MaxX(),samplerregion.MaxY());

          return former[i * countx + j]->GetSampler(samplerregion, 
                                                    resx, resz);
        }

          formerregion *= translated;

      // the requested region is represented by multiple formers
      // so we have to scale the resolution accordingly

        float formerdistx = formerregion.MaxX() - formerregion.MinX();
        float transdistx = translated.MaxX() - translated.MinX();

        float percx = formerdistx / transdistx;
        float samplerresx = resx * percx;

        float formerdistz = formerregion.MaxY() - formerregion.MinY();
        float transdistz = translated.MaxY() - translated.MinY();

        float percz = formerdistz / transdistz;
        float samplerresz = resz * percz;

        // in case the requested resolution isn't a multiple of our formercount
        // we need to sample a little too much data from each and overlap later
        uint samplerresolutionx = (uint)(ceil(samplerresx)+0.5);
        uint samplerresolutiony = (uint)(ceil(samplerresz)+0.5);

printf("%f %f\n", formerdistx, formerdistz);
printf("%f %f\n", transdistx, transdistz);
printf("%f %f\n", percx,percz);
printf("res: %i->%i\n",resx,samplerresolutionx);
printf("res: %i->%i\n",resz,samplerresolutiony);

          float samplerregionminx = formerregion.MinX();
          float samplerregionminy = formerregion.MinY();
          float samplerregionmaxx = formerregion.MaxX();
          float samplerregionmaxy = formerregion.MaxY();
            samplerregionminx = ((samplerregionminx/(width/2)-1)*scale.x+offset.x);
            samplerregionminy = ((samplerregionminy/(height/2)-1)*scale.z+offset.z);
            samplerregionmaxx = ((samplerregionmaxx/(width/2)-1)*scale.x+offset.x);
            samplerregionmaxy = ((samplerregionmaxy/(height/2)-1)*scale.z+offset.z);

printf("Min: %f:%f Max: %f:%f\n", samplerregionminx,samplerregionminy,samplerregionmaxx,samplerregionmaxy);

  csBox2 asdf = csBox2(
            ceil(samplerregionminx),
            ceil(samplerregionminy),
            ceil(samplerregionmaxx),
            ceil(samplerregionmaxy)
          );

printf("x: %f:%i y: %f:%i\n", scale.x, countx, scale.z,county);
printf("Min: %f:%f Max: %f:%f\n", asdf.MinX(),asdf.MinY(),asdf.MaxX(),asdf.MaxY());

sampler.Push (csRef<iTerraSampler>
          (former[i * countx + j]->GetSampler(asdf, samplerresolutionx, samplerresolutiony)));

      const csVector3 *tests = new csVector3[samplerresolutionx*samplerresolutiony];
      tests = sampler.Get(sampler.GetSize()-1)->SampleVector3(stringVertices);
      csBox2 testregion = asdf;

//if (smaller) {
  csImageMemory pic = csImageMemory(samplerresolutionx, samplerresolutiony);
  csRGBpixel* dat = (csRGBpixel*)pic.GetImagePtr();
  for (int asdf=0; asdf < samplerresolutionx*samplerresolutiony; asdf++) 
  {
    dat[asdf].red = tests[asdf].y;
    dat[asdf].green = tests[asdf].y;
    dat[asdf].blue = tests[asdf].y;
  }
  csDebugImageWriter a = csDebugImageWriter();
  csString fn = csString();
  fn += "i";
  fn += i;
  fn += "j";
  fn += j; 
  fn += "x";
  fn += testregion.MinX();
  fn += "y";
  fn += testregion.MinY();
  fn += "-";
  fn += rand();
  fn += ".png";
  a.DebugImageWrite(&pic,fn);
printf(fn);
printf("\n\n");
//}
      }
    }

    // devide length of requested region by formersize
    // to get number of formers in x direction
    uint formercountx = ceilf(z2/sizey)-floorf(z1/sizey);
    return new csPagingSampler(this, sampler, formercountx, 
      region, resx, resz);

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
    float z, csVector3 &value)
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


csPagingSampler::csPagingSampler (csPagingFormer* former,
                       csRefArray<iTerraSampler> sampler, uint xcount, 
                       csBox2 region, unsigned int resx, uint resz)
 : scfImplementationType (this)
{
  // Initialize members
  csPagingSampler::terraFormer = former;
  csPagingSampler::sampler = sampler;
  csPagingSampler::region = region;
  csPagingSampler::resx = resx;
  csPagingSampler::resz = (resz==0)?resx:resz;
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
}


// Calculate and cache positions
void csPagingSampler::CachePositions ()
{
  if (positions != 0) return;

  // get memory for our cache
  positions = new csVector3[resx*resz];

  // buffer for all the samplers maps
  uint num = sampler.GetSize();
  const csVector3 **maps = new const csVector3*[num];

  // first get the raw data from all the samplers
  // @@@ excluded from loop below for easier profiling
  uint k;
  for (k = 0; k < num; k++)
  {
      maps[k] = sampler.Get(k)->SampleVector3(terraFormer->stringVertices);
  }

  // then loop about all samplers again to copy needed data
  for (k = 0; k < num; k++)
  {
      csBox2 partialregion = sampler.Get(k)->GetRegion();
      uint partialresolutionx, partialresolutionz;
      sampler.Get(k)->GetResolution(partialresolutionx,partialresolutionz);

      uint fromx, fromy;  // array coordinates we copy from
      uint tox, toy;      // array coordinates we copy to
      uint copyx;       // number of elements we need to copy per step
      uint copyy;       // number of elements we need to copy per step
      uint fromstep;      // number of elements we need to skip per step
      uint tostep;        // number of elements we need to skip per step


        // calculate which part of the whole we deal with
        // and translate into heightmap coordinates
        csBox2 intersect = partialregion * region;

printf("Min: %f:%f Max: %f:%f\n", partialregion.MinX(),partialregion.MinY(),partialregion.MaxX(),partialregion.MaxY());
printf("Min: %f:%f Max: %f:%f\n", region.MinX(),region.MinY(),region.MaxX(),region.MaxY());
printf("Min: %f:%f Max: %f:%f\n", intersect.MinX(),intersect.MinY(),intersect.MaxX(),intersect.MaxY());

        float intersectminx = intersect.MinX();
        float intersectminy = intersect.MinY();
//          intersectminx = ((intersectminx-terraFormer->offset.x)/terraFormer->scale.x +1)
//            *(terraFormer->width/2);
//          intersectminy = ((intersectminy-terraFormer->offset.z)/terraFormer->scale.z +1)
//            *(terraFormer->height/2);

/*
        if (!region.Contains(partialregion))
        {
printf("!smaller\n");
          // the region is smaller so it's only part of this sampler's data

          // translate the request's max coordinates because we need them later on

//@@@ local translations!

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

printf("res: %i part: %i\n", resolution, partialresolution);
printf("%f %f\n",intersectminx,partialregionminx);
printf("%f %f\n",intersectminy,partialregionminy);

          // copy from the intersection's relative coordinates
          fromx = (uint)ceil(((intersectminx-partialregionminx)/(partialregionmaxx-partialregionminx)) *partialresolution);
          fromy = (uint)ceil(((intersectminy-partialregionminy)/(partialregionmaxy-partialregionminy)) *partialresolution);

          tox = (k/xcount) * partialresolution;
          toy = (k%xcount) * partialresolution;

          // we just need to copy a part of this sampler's data
          copynum = partialresolution;
          fromstep = partialresolution;
          tostep = resolution;

        } else {

printf("!bigger\n");
*/

          // this whole sampler is completely in the region, so
          // we need to copy this sampler's whole data
          fromx = 0;
          fromy = 0;
          copyx = partialresolutionx;
          copyy = partialresolutionz;
          fromstep = partialresolutionx;
          tostep = resx;
//@@@
          // to this former's relative position to the whole request
          tox = (uint)ceil(((intersectminx-region.MinX())/(region.MaxX()-region.MinX())) *resx);
          toy = (uint)ceil(((intersectminy-region.MinY())/(region.MaxY()-region.MinY())) *resz); 

printf("fx: %i\n",fromx);
printf("fy: %i\n",fromy);
printf("tx: %i\n",tox);
printf("ty: %i\n",toy);
printf("cx: %i\n",copyx);
printf("cy: %i\n",copyy);
printf("fs: %i\n",fromstep);
printf("ts: %i\n",tostep);
printf("%i, %i\n", k, xcount);
printf("%i, %i\n", k%xcount, k/xcount);
      uint ycount = sampler.GetSize() / xcount;
printf("%i, %i\n", k%(sampler.GetSize()/xcount), 
k/(sampler.GetSize()/xcount));

      // loop over the part we need to copy
      uint l;
      for (l = 0; l < copyy ; l++ )
      {
        //in case of several formers the copied data needs to overlap
        //since we sample a little too much data for each
        //when the requested region's size is not a multiple of their count
        //we do this by overwriting the neighboring former's last line

        uint topos = (toy+l-(k%xcount))*tostep+tox-(k/xcount);
        uint frompos = (fromy+l)*fromstep+fromx;

printf("%i: %i -> %i\n", l, frompos, topos);

        memcpy(positions + topos,
          maps[k] + frompos,
          copyx*sizeof(csVector3));
      }
    }
}



void csPagingSampler::CacheNormals ()
{
  // Break if we've already cached the data
  if (normals != 0)
    return;

  // Allocate new data
  normals = new csVector3[resx*resz];

  // Make sure we've got some position data to base the normals on
  CachePositions ();

  // Keep index counters to avoid uneccessary x+y*w calculations
  int normIdx = 0;
  int posIdx = 0;

  // Intermediate vectors
  csVector3 v1, v2;

  // Iterate through the samplepoints in the region
  for (unsigned int i = 0; i<resz; ++i)
  {
    for (unsigned int j = 0; j<resx; ++j)
    {
      // Calculate two gradient vectors
      // Conditionals check wheter to fetch from edge data
      v1 = j==resx-1?
        positions[posIdx]:positions[posIdx+1];
      v1 -= j==0?
        positions[posIdx]:positions[posIdx-1];

      v2 = i==resz-1?
        positions[posIdx]:positions[posIdx+resx];
      v2 -= i==0?
        positions[posIdx]:positions[posIdx-resx];
/*
printf("%i %i %i\n",j,i,posIdx);
printf("%i %i\n",resx,resz);
printf("%f %f %f\n",v1.x,v1.y,v1.z);
printf("%f %f %f\n",v2.x,v2.y,v2.z);
*/
      // Cross them and normalize to get a normal
      normals[normIdx++] = (v2%v1).Unit ();
      posIdx++;
    }
  }
}


//@@@ fixme with some copymagic too
void csPagingSampler::CacheHeights ()
{
}


void csPagingSampler::CacheTexCoords ()
{
  // Break if we've already cached the data
  if (texCoords != 0)
    return;

  // Allocate new data
  texCoords = new csVector2[resx*resz];

  // Keep index counter to avoid uneccessary x+y*w calculations
  int idx = 0;

//@@@ heightmap space?
  csVector3 minCorner = csVector3 (region.MinX (), 0, region.MinY ());
  csVector3 maxCorner = csVector3 (region.MaxX (), 0, region.MaxY ());

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
  csVector3 sampleDistanceHeight;
   sampleDistanceHeight.x = (maxCorner.x-minCorner.x)/(float)(resx-1);
   sampleDistanceHeight.z = (maxCorner.z-minCorner.z)/(float)(resz-1);

  // Iterate through the samplepoints in the region
  // Sample texture coordinates as x/z positions in heightmap space
  const float startx = minCorner.x / (float)(terraFormer->width);
  csVector2 texCoord;
  texCoord.y = minCorner.y / (float)(terraFormer->height);

  const csVector2 tcStep (
    sampleDistanceHeight.x / (float)(terraFormer->width), 
    sampleDistanceHeight.z / (float)(terraFormer->height));

//printf("#res: %i\n\n",resolution);
  for (unsigned int i = 0; i<resz; ++i)
  {
    texCoord.x = startx; 
    for (unsigned int j = 0; j<resx; ++j)
    {
      // Just assign the texture coordinate
//printf("%f,%f ",texCoord.x,1.0f-texCoord.y);
      texCoords[idx++].Set (texCoord.x, 1.0f-texCoord.y); //= texCoord;
      texCoord.x += tcStep.x;
    }
    texCoord.y += tcStep.y;
//printf("\n");
  }
}


const float *csPagingSampler::SampleFloat (csStringID type)
{
//printf("SampleFloat Sampler\n");

  // Check what we're supposed to return
  if (type == terraFormer->stringHeights)
  {

printf("Sample Height Sampler\n");

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
printf("SampleTCs Sampler\n");

    CacheTexCoords();
    return texCoords;
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
printf("SampleVert Sampler\n");

    CachePositions();    
    return positions;
  }
  else if (type == terraFormer->stringNormals)
  {
printf("SampleNorm Sampler\n");

    CacheNormals();
    return normals;
  }
  else
  {
    // Something we can't return was requested
    return 0;
  }
}


const int *csPagingSampler::SampleInteger (csStringID type)
{
printf("SampleMaterial Sampler\n");
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


void csPagingSampler::GetResolution (uint &resx, uint &resz) const
{
  resx = csPagingSampler::resx;
  resz = csPagingSampler::resz;
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


/*
heightmap debug printer

      const csVector3 *tests = new csVector3[samplerresolution*samplerresolution];
      tests = sampler.Get(sampler.GetSize()-1)->SampleVector3(stringVertices);

//if (smaller) {
  csImageMemory pic = csImageMemory(resolution, resolution);
  csRGBpixel* dat = (csRGBpixel*)pic.GetImagePtr();
  for (int asdf=0; asdf < resolution*resolution; asdf++) {
    dat[asdf].red = tests[asdf].y;
    dat[asdf].green = tests[asdf].y;
    dat[asdf].blue = tests[asdf].y;
  }
  csDebugImageWriter a = csDebugImageWriter();
  csString fn = csString();
  fn += "x";
  fn += samplerregion.MinX();
  fn += "y";
  fn += samplerregion.MinY();
  fn += "-";
  fn += rand();
  fn += ".png";
  a.DebugImageWrite(&pic,fn);
printf(fn);
//}
*/

}
CS_PLUGIN_NAMESPACE_END(PagingFormer)
