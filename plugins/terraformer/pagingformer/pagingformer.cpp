
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

//@@@ raw heightmaps
#include "csutil/csendian.h"
#include "csutil/dirtyaccessarray.h"

//@@@ debugging
#include "cstool/debugimagewriter.h"
#include "csgfx/imagememory.h"

#include "pagingformer.h"



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
  csRef<iStringSet> strings = csQueryRegistryTagInterface<iStringSet> (
    objectRegistry, "crystalspace.shared.stringset");

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


// @@@ raw heightmap file loaders. move to csImage and Loader ---------

template<typename Endianness>
struct GetterFloat
{
  static inline void Get (char*& buf, float&f)
  {
    f = csIEEEfloat::ToNative (Endianness::Convert 
      (csGetFromAddress::UInt32 (buf))); 
    buf += sizeof(uint32);
  }
  static inline size_t ItemSize()
  { return sizeof(uint32); }
};

template<typename Endianness>
struct GetterUint16
{
  static inline void Get (char*& buf, float&f)
  {
    uint16 v = Endianness::Convert (csGetFromAddress::UInt16 (buf));
    buf += sizeof (uint16);
    f = float(v) / 65535.0f;
  }
  static inline size_t ItemSize()
  { return sizeof(uint16); }
};

template<typename Endianness>
struct GetterUint32
{
  static inline void Get (char*& buf, float&f)
  {
    uint32 v = Endianness::Convert (csGetFromAddress::UInt32 (buf));
    buf += sizeof (uint32);
    f = float(v) / 4294967295.0f;
  }
  static inline size_t ItemSize()
  { return sizeof(uint32); }
};

template<typename Tgetter>
csDirtyAccessArray<float> ReadData (char* buf, uint w, uint h)
{
  const size_t num = w * h;
  csDirtyAccessArray<float> fdata;
  fdata.SetSize (num);
  for (size_t i = 0; i < num; i++)
  {
    Tgetter::Get (buf, fdata[i]);
  }
  return fdata;
}

// --------------------------------------------------------------------


void csPagingFormer::LoadFormer (uint x, uint y)
{
  csRef<iLoader> loader = csQueryRegistry<iLoader> (objectRegistry);
  csRef<iPluginManager> plugin_mgr = 
    csQueryRegistry<iPluginManager> (objectRegistry);

  csRef<iTerraFormer> simpleFormer = 
    csLoadPlugin<iTerraFormer> (plugin_mgr, "crystalspace.terraformer.simple"); 

  csRef<iSimpleFormerState> state = 
    scfQueryInterface<iSimpleFormerState> (simpleFormer);
  if (state)
  {
    csString ending = csString();

    ending += "x";
    ending += x;
    ending += "y";
    ending += y;

    if (heightmapformat == PAGINGHEIGHT_RAWFLOATLE)
    {
      csRef<iVFS> vfs = csQueryRegistry<iVFS> (objectRegistry);
      csRef<iDataBuffer> buf = 
        vfs->ReadFile(heightmapdir+ending+".raw", false);

//@@@ change to variable
#define hmwidth 256
#define hmheight 256

      csDirtyAccessArray<float> mapdata =
        ReadData<GetterFloat<csLittleEndian> >
        (buf->GetData(), hmwidth, hmheight);
      state->SetHeightmap(mapdata.GetArray(), hmwidth, hmheight);

      width = countx * hmwidth;
      height = county * hmheight;

#undef HMSIZE
    }
    else
    {
      csRef<iImage> map = 
        loader->LoadImage (heightmapdir+ending+".png", 
        CS_IMGFMT_ANY);
      if (map)
      {
        state->SetHeightmap(map);
        if (width==0)
        {
          width = countx * map->GetWidth();
          height = county * map->GetHeight();
        }

      } else { exit(2000); } // @@@
    } // heightmap format

    state->SetScale(csVector3(scale.x/countx, scale.y, 
      scale.z/county));
    state->SetOffset(offset + csVector3(
      (2*x*(scale.x/countx)-scale.x) +scale.x/countx,
      0,
      (2*y*(scale.z/county)-scale.z) +scale.z/county
      ));

    csHash<csString, csStringID>::GlobalIterator it = 
      intmapdir.GetIterator();
    while (it.HasNext())
    {
      csStringID thistype;
      const char* path = it.Next(thistype);
      csRef<iImage> map = 
        loader->LoadImage (path+ending+".png", 
        CS_IMGFMT_ANY);

      state->SetIntegerMap(thistype, map, 1, 0);
    }

    it = floatmapdir.GetIterator();
    while (it.HasNext())
    {
      csStringID thistype;
      const char* path = it.Next(thistype);
      csRef<iImage> map = 
        loader->LoadImage (path+ending+".png", 
        CS_IMGFMT_ANY);
      state->SetFloatMap(thistype, map, 1, 0);
    }

  } else { exit(1000); }

  former[y * countx + x] = simpleFormer;
}


void csPagingFormer::SetHeightmapDir (const char* path, const char *type)
{
  if (strcmp(type, "rawfloatle") == 0) 
    heightmapformat = PAGINGHEIGHT_RAWFLOATLE;
  else heightmapformat = PAGINGHEIGHT_IMAGE;

  heightmapdir = csStrNew(path);

  csRef<iVFS> VFS = csQueryRegistry<iVFS> (objectRegistry);
  csRef<iStringArray> heightmapnames = VFS->FindFiles(heightmapdir);

  //@@@ add check for 0 files

  heightmapnames->Sort();
  csString lastname = 
    csString(heightmapnames->Get(heightmapnames->GetSize()-1));
  size_t ypos = lastname.FindLast('y');
  lastname = lastname.Slice(ypos+1,(lastname.FindLast('.')-ypos)-1);

  uint numy;
  sscanf(lastname.GetData(),"%d",&numy);
  numy++;

  uint numx = (uint)heightmapnames->GetSize() / numy;
  countx = numx;
  county = numy;

  //printf("popmap: %i/%i\n",numx,numy);

  former = new csRef<iTerraFormer>[numx*numy];
  memset (former, 0, numx*numy);

  LoadFormer(0,0); // @@@ needed for width/height, set in mapfile?
  former[0] = 0;
}


void csPagingFormer::SetIntmapDir (const csStringID type, 
  const char* path)
{
  csRef<iVFS> VFS = csQueryRegistry<iVFS> (objectRegistry);
  csRef<iStringArray> heightmapnames = VFS->FindFiles(heightmapdir);

  heightmapnames->Sort();
  csString lastname = 
    csString(heightmapnames->Get(heightmapnames->GetSize()-1));
  size_t ypos = lastname.FindLast('y');
  lastname = lastname.Slice(ypos+1,(lastname.FindLast('.')-ypos)-1);

  uint numy;
  sscanf(lastname.GetData(),"%d",&numy);
  numy++;

  uint numx = (uint)heightmapnames->GetSize() / numy;

  if (numx == countx && numy == county)
  {
    intmapdir.Put(type, path);
  }
  else printf("heightmap and intmap numbers differ\n"); //@@@
}


void csPagingFormer::SetFloatmapDir (const csStringID type, 
  const char* path)
{
  csRef<iVFS> VFS = csQueryRegistry<iVFS> (objectRegistry);
  csRef<iStringArray> heightmapnames = VFS->FindFiles(heightmapdir);

  heightmapnames->Sort();
  csString lastname = 
    csString(heightmapnames->Get(heightmapnames->GetSize()-1));
  size_t ypos = lastname.FindLast('y');
  lastname = lastname.Slice(ypos+1,(lastname.FindLast('.')-ypos)-1);

  uint numy;
  sscanf(lastname.GetData(),"%d",&numy);
  numy++;

  uint numx = (uint)heightmapnames->GetSize() / numy;

  if (numx == countx && numy == county)
  {
    floatmapdir.Put(type, path);
  }
  else printf("heightmap and floatmap numbers differ\n"); //@@@
}


/// Set a scaling factor to be used in lookups
void csPagingFormer::SetScale (csVector3 newscale)
{
  //printf("SetScale\n");

  scale = newscale;

  for (uint i = 0; i < (countx*county); i++)
  {
    if (former[i] != 0)
    {
      csRef<iSimpleFormerState> state =       
        scfQueryInterface<iSimpleFormerState> (former[i]);
      if (state)
      {
        csVector3 applied = csVector3(newscale.x/countx, 
          newscale.y, 
          newscale.z/county);
        state->SetScale(applied);
        state->SetOffset(offset + csVector3(
          (2*(i%countx)*(scale.x/countx)-scale.x)+scale.x/countx,
          0,
          (2*(i/countx)*(scale.z/county)-scale.z)+scale.z/county
          ));
        //printf("&SetScale: %i\n", i);
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
bool csPagingFormer::SetIntegerMap (csStringID /*type*/, iImage* /*map*/,
  int /*scale*/, int /*offset*/)
{
  return false;
}

/// Get the integer map dimensions.
csVector2 csPagingFormer::GetIntegerMapSize (csStringID type)
{
  return csVector2(0,0);
}

/// Set additional float map.
bool csPagingFormer::SetFloatMap (csStringID /*type*/, iImage* /*map*/,
  float /*scale*/, float /*offset*/)
{
  return false;
}


// ------------ iTerraFormer implementation ------------



/// Creates and returns a sampler. See interface for details
csPtr<iTerraSampler> csPagingFormer::GetSampler (csBox2 region, 
  uint resx, uint resz)
{
  if (resz == 0) resz = resx;

bool breakagex = false;

  //printf("\n");
  //printf("GetSampler\n");
  csRefArray<iTerraSampler> sampler;

  //determine the min and max corners in heightmap space
  float x1 = region.MinX();
  float z1 = region.MinY();
  float x2 = region.MaxX();
  float z2 = region.MaxY();

  //printf("Min: %f:%f Max: %f:%f\n", x1,z1,x2,z2);

  x1 = ((x1-offset.x)/scale.x+1)*(width/2);
  z1 = ((z1-offset.z)/scale.z+1)*(height/2);

  x2 = ((x2-offset.x)/scale.x+1)*(width/2);
  z2 = ((z2-offset.z)/scale.z+1)*(height/2);

  //printf("Min: %f:%f Max: %f:%f\n", x1,z1,x2,z2);

  //calculate the size of one formers terrain
  int sizex = (int)(width/countx + 0.5f);
  int sizey = (int)(height/county + 0.5f);

  //printf("size: %i %i\n", sizex, sizey);

  uint i,j;
  // go through the formers we need
  // and calculate the intersecting regions.
  uint maxformerx = (uint)(ceilf(x2/sizex));
  uint maxformerz = (uint)(ceilf(z2/sizey));

  //printf("min x:%f y:%f\n",x1/(sizex),z1/(sizey));
  //printf("min x:%i y:%i\n",(uint)x1/(sizex),(uint)z1/(sizey));
  //printf("max x:%f y:%f\n",x2/sizex,z2/sizey);
  //printf("max x:%f y:%f\n",ceilf(x2/sizex),ceilf(z2/sizey));
  //printf("max x:%i y:%i\n",maxformerx,maxformerz);
  for (i = (uint)x1/(sizex); i < maxformerx; i++)
  {
    for (j = (uint)z1/(sizey); j < maxformerz; j++)
    {
      //printf("-------------------------\n");
      if (former[j * countx + i] == 0)
      {
        printf("*loading former: %i %i\n ",i,j);
        LoadFormer(i,j);
      }

      //printf("x:%i,y:%i\n",i,j);
      csBox2 translated = csBox2 (x1,z1,x2,z2);
      csBox2 formerregion = csBox2 (i*sizex, j*sizey,
        (i+1)*sizex, (j+1)*sizey );

      //printf("Min: %f:%f Max: %f:%f\n", translated.MinX(),translated.MinY(),translated.MaxX(),translated.MaxY());
      //printf("Min: %f:%f Max: %f:%f\n", formerregion.MinX(),formerregion.MinY(),formerregion.MaxX(),formerregion.MaxY());

      /*
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
      */

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
      uint samplerresolutionx = (uint)(ceilf(samplerresx)+0.5);
      uint samplerresolutiony = (uint)(ceilf(samplerresz)+0.5);

      //printf("%f %f\n", formerdistx, formerdistz);
      //printf("%f %f\n", transdistx, transdistz);
      //printf("%f %f\n", percx,percz);
      //printf("res: %i->%i\n",resx,samplerresolutionx);
      //printf("res: %i->%i\n",resz,samplerresolutiony);

      if (samplerresolutionx == 0) breakagex = true;

      float samplerregionminx = formerregion.MinX();
      float samplerregionminy = formerregion.MinY();
      float samplerregionmaxx = formerregion.MaxX();
      float samplerregionmaxy = formerregion.MaxY();
      samplerregionminx = ((samplerregionminx/(width/2)-1)*scale.x+offset.x);
      samplerregionminy = ((samplerregionminy/(height/2)-1)*scale.z+offset.z);
      samplerregionmaxx = ((samplerregionmaxx/(width/2)-1)*scale.x+offset.x);
      samplerregionmaxy = ((samplerregionmaxy/(height/2)-1)*scale.z+offset.z);

      //printf("Min: %f:%f Max: %f:%f\n", samplerregionminx,samplerregionminy,samplerregionmaxx,samplerregionmaxy);

      csBox2 asdf = csBox2(
        samplerregionminx,
        samplerregionminy,
        samplerregionmaxx,
        samplerregionmaxy
        );

      //printf("x: %f:%i y: %f:%i\n", scale.x, countx, scale.z,county);
      //printf("Min: %f:%f Max: %f:%f\n", 
      //  asdf.MinX(),asdf.MinY(),asdf.MaxX(),asdf.MaxY());

      if (!((samplerresolutionx == 0) || (samplerresolutiony == 0)) ) {

        sampler.Push (csRef<iTerraSampler>
          (former[j * countx + i]->GetSampler
            (asdf, samplerresolutionx, samplerresolutiony)));

        /*
        const csVector3 *tests = new csVector3[samplerresolutionx*samplerresolutiony];
        tests = sampler.Get(sampler.GetSize()-1)->SampleVector3(stringVertices);
        csBox2 testregion = asdf;

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
        fn += testregion.MinX();
        fn += "y";
        fn += testregion.MinY();
        fn += "-";
        fn += rand();
        fn += ".png";
        a.DebugImageWrite(&pic,fn);
        printf(fn);
        printf("\n\n");
        */
      }
    }
  }

  // devide length of requested region by formersize
  // to get number of formers in x direction
  uint formercountx = maxformerx-(uint)(x1/sizex);
  if (formercountx == 0) formercountx = 1;
  if (breakagex) formercountx -= 1;
  return new csPagingSampler(this, sampler, formercountx, 
    region, resx, resz);

}


/**
* Sample float data.
* Allowed types:
* heights
* Will not return any actual value, just if the map is present.
*/
bool csPagingFormer::SampleFloat (csStringID type, float /*x*/, 
  float /*z*/, float &/*value*/)
{
  return floatmapdir.Contains(type);
}


/**
* Sample csVector2 data.
* Will return false.
*/
bool csPagingFormer::SampleVector2 (csStringID /*type*/, float /*x*/,
  float /*z*/, csVector2 &/*value*/)
{
  return false;
}


/**
* Sample csVector3 data.
* Allowed types:
* vertices
*/
bool csPagingFormer::SampleVector3 (csStringID /*type*/, float /*x*/, 
  float /*z*/, csVector3 &/*value*/)
{
  return false;
}


/**
* Sample integer data.
* Will not return any actual value, just if the map is present.
*/
bool csPagingFormer::SampleInteger (csStringID type, float /*x*/, float /*z*/,
  int &/*value*/)
{
  return intmapdir.Contains(type);
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
  size_t num = sampler.GetSize();
  const csVector3 **maps = new const csVector3*[num];

  // first get the raw data from all the samplers
  // @@@ excluded from loop below for easier profiling
  size_t k;
  //uint sumresx = 0;
  //uint sumresz = 0;
  //uint formerresx;
  //uint formerresz;
  for (k = 0; k < num; k++)
  {
    maps[k] = sampler.Get(k)->SampleVector3(terraFormer->stringVertices);
    //sampler.Get(k)->GetResolution(formerresx,formerresz);

    //printf ("sampler: %i %i: ", k, xcount);
    /*
    if (k < num/xcount)
    {
      sumresz += formerresz;
      //printf("z %i ", formerresz);
    }
    if (k % (num/xcount) == 0)
    {
      sumresx += formerresx;
      //printf("x %i", formerresx);
    }
    //printf("\n");
    */
  }

  //printf("%i > %i?\n",sumresx,resx);
  //printf("%i > %i?\n",sumresz,resz);
  //assert(sumresx >= resx);
  //assert(sumresz >= resz);

  //if (sumresx > resx) printf ("&overlapx\n");
  //if (sumresz > resz) printf ("&overlapz\n");

  //bool overlapx = (sumresx > resx);
  //bool overlapz = (sumresz > resz);

  // then loop about all samplers again to copy needed data
  for (k = 0; k < num; k++)
  {
    csBox2 partialregion = sampler.Get(k)->GetRegion();
    partialregion = csBox2(
      partialregion.MinX(),
      partialregion.MinY(),
      partialregion.MaxX(),
      partialregion.MaxY()
      );
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

    //printf("Min: %f:%f Max: %f:%f\n", partialregion.MinX(),partialregion.MinY(),partialregion.MaxX(),partialregion.MaxY());
    //printf("Min: %f:%f Max: %f:%f\n", region.MinX(),region.MinY(),region.MaxX(),region.MaxY());
    //printf("Min: %f:%f Max: %f:%f\n", intersect.MinX(),intersect.MinY(),intersect.MaxX(),intersect.MaxY());

    float intersectminx = intersect.MinX();
    float intersectminy = intersect.MinY();

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
    tox = (uint)(((intersectminx-region.MinX())/
      (region.MaxX()-region.MinX())) *resx); 
    toy = (uint)(((intersectminy-region.MinY())/
      (region.MaxY()-region.MinY())) *resz); 
    if (tox > resx) tox = resx;
    if (toy > resz) toy = resz;

    /*
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
    uint ycount = (uint)(num/xcount+0.5);
    printf("%i, %i\n", k%ycount, k/ycount);
    */

    //uint xoverwrite =0;// (overlapx) ? k/xcount : 0;
    //uint yoverwrite =0;// (overlapz) ? k%xcount : 0;
    //printf("%i, %i\n", xoverwrite,yoverwrite);

    // loop over the part we need to copy
    uint l;
    for (l = 0; l < copyy ; l++ )
    {
      //in case of several formers the copied data needs to overlap
      //since we sample a little too much data for each
      //when the requested region's size is not a multiple of their count
      //we do this by overwriting the neighboring former's last line

      uint topos = (toy+l)*tostep+tox;
      uint frompos = (fromy+l)*fromstep+fromx;

      //printf("%i: %i -> %i\n", l, frompos, topos);

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
    //printf("SampleVert Sampler\n");

    CachePositions();    
    return positions;
  }
  else if (type == terraFormer->stringNormals)
  {
    //printf("SampleNorm Sampler\n");

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
  //printf("SampleMaterial Sampler\n");
  // Check what we're supposed to return
  if (type == terraFormer->stringMaterialIndices)
  {
    // This isn't implemented yet. We just return 0
    return 0;
  }
  else if (terraFormer->intmapdir.Contains(type))
  {
    // get memory for our cache
    int *map = new int[resx*resz];

    // buffer for all the samplers maps
    size_t num = sampler.GetSize();
    const int **maps = new const int*[num];

  // first get the raw data from all the samplers
  // @@@ excluded from loop below for easier profiling
  size_t k;
  //uint sumresx = 0;
  //uint sumresz = 0;
  //uint formerresx;
  //uint formerresz;
  for (k = 0; k < num; k++)
  {
    maps[k] = sampler.Get(k)->SampleInteger(type);
    //sampler.Get(k)->GetResolution(formerresx,formerresz);

    //printf ("sampler: %i %i: ", k, xcount);
    /*
    if (k < num/xcount)
    {
      sumresz += formerresz;
      //printf("z %i ", formerresz);
    }
    if (k % (num/xcount) == 0)
    {
      sumresx += formerresx;
      //printf("x %i", formerresx);
    }
    //printf("\n");
    */
  }

  //printf("%i > %i?\n",sumresx,resx);
  //printf("%i > %i?\n",sumresz,resz);
  //assert(sumresx >= resx);
  //assert(sumresz >= resz);

  //if (sumresx > resx) printf ("&overlapx\n");
  //if (sumresz > resz) printf ("&overlapz\n");

  //bool overlapx = (sumresx > resx);
  //bool overlapz = (sumresz > resz);

  // then loop about all samplers again to copy needed data
  for (k = 0; k < num; k++)
  {
    csBox2 partialregion = sampler.Get(k)->GetRegion();
    partialregion = csBox2(
      partialregion.MinX(),
      partialregion.MinY(),
      partialregion.MaxX(),
      partialregion.MaxY()
      );
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

    //printf("Min: %f:%f Max: %f:%f\n", partialregion.MinX(),partialregion.MinY(),partialregion.MaxX(),partialregion.MaxY());
    //printf("Min: %f:%f Max: %f:%f\n", region.MinX(),region.MinY(),region.MaxX(),region.MaxY());
    //printf("Min: %f:%f Max: %f:%f\n", intersect.MinX(),intersect.MinY(),intersect.MaxX(),intersect.MaxY());

    float intersectminx = intersect.MinX();
    float intersectminy = intersect.MinY();

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
    tox = (uint)(((intersectminx-region.MinX())/
      (region.MaxX()-region.MinX())) *resx); 
    toy = (uint)(((intersectminy-region.MinY())/
      (region.MaxY()-region.MinY())) *resz); 
    if (tox > resx) tox = resx;
    if (toy > resz) toy = resz;

    /*
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
    uint ycount = (uint)(num/xcount+0.5);
    printf("%i, %i\n", k%ycount, k/ycount);
    */

    //uint xoverwrite =0;// (overlapx) ? k/xcount : 0;
    //uint yoverwrite =0;// (overlapz) ? k%xcount : 0;
    //printf("%i, %i\n", xoverwrite,yoverwrite);

    // loop over the part we need to copy
    uint l;
    for (l = 0; l < copyy ; l++ )
    {
      //in case of several formers the copied data needs to overlap
      //since we sample a little too much data for each
      //when the requested region's size is not a multiple of their count
      //we do this by overwriting the neighboring former's last line

      uint topos = (toy+l)*tostep+tox;
      uint frompos = (fromy+l)*fromstep+fromx;

      //printf("%i: %i -> %i\n", l, frompos, topos);

        memcpy(map + topos,
          maps[k] + frompos,
          copyx*sizeof(int));
      }
    }
    return map;
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


}
CS_PLUGIN_NAMESPACE_END(PagingFormer)
