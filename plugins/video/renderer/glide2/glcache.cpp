/*
    Copyright (C) 1998 by Jorrit Tyberghein and Dan Ogles.

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

// GLIDECACHE.CPP
// GlideTextureCache and GlideLightmapCache implementation file
// Written by xtrochu and Nathaniel

#include "sysdef.h"
#include "cssys/system.h"
#include "cs3d/glide2/g3dglide.h"
#include "cs3d/glide2/glcache.h"
#include "cs3d/glide2/glidelib.h"
#include "ilghtmap.h"

#include "glide.h"

GlideTextureCache::GlideTextureCache(TMUInfo *t, int bpp, TextureMemoryManager *man)
: HighColorCache(t->memory_size, HIGHCOLOR_TEXCACHE, bpp,man)
{
  m_tmu = t;
}

void GlideTextureCache::Dump()
{
  //CsPrintf (MSG_CONSOLE, "Textures in the cache: %d\n", num);
  //CsPrintf (MSG_CONSOLE, "Total size: %ld bytes\n", total_size);
  int mean;
  if (num == 0) mean = 0;
  else mean = total_size/num;
  //CsPrintf (MSG_CONSOLE, "Bytes per texture: %d\n", mean);
  //CsPrintf (MSG_CONSOLE, "Fragmentation of Memory: %d\n",manager->getFragmentationState());
}

void GlideTextureCache::Load(csGlideCacheData *d)
{

  iTextureHandle* txt_handle = (iTextureHandle*)d->pSource;
  csTextureMM* txt_mm = (csTextureMM*)txt_handle->GetPrivateObject ();
  csTexture* txt_unl = txt_mm->get_texture (0);
  
  int i;
  int width = txt_unl->get_width ();
  int height = txt_unl->get_height ();

  GrLOD_t lod[4];
  d->texhnd.loaded = false;
 
  if (CalculateTexData ( width, height, 1.0, 1.0, lod, 4, d ))
  {
    d->texhnd.info.format=GR_TEXFMT_RGB_565;
    d->texhnd.tmu = m_tmu;
    d->texhnd.info.data=(unsigned char*)txt_mm->get_texture (0)->get_bitmap();
    d->texhnd.size = GlideLib_grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH,&d->texhnd.info);

    /* if(d->texhnd.size!=d->size)
    {
      CsPrintf(MSG_CONSOLE,"Ooups size! (%d != %d)\n",texhnd->size,d->size);
      }*/
  
    d->mempos=manager->allocSpaceMem(d->texhnd.size);
    d->texhnd.loadAddress = d->mempos->offset+m_tmu->minAddress;

    unsigned char* src=NULL;
    // Download all mipmaps
    for( i=0; i<4; i++)
    {
      if (lod[i]!=-1)
      {
        csTexture* txt_mip = txt_mm->get_texture (i);
        src = (unsigned char*)txt_mip->get_bitmap();

        GlideLib_grTexDownloadMipMapLevel(d->texhnd.tmu->tmu_id,
                                          d->texhnd.loadAddress,
                                          lod[i],
                                          d->texhnd.info.largeLod,
                                          d->texhnd.info.aspectRatio,
                                          d->texhnd.info.format,
                                          GR_MIPMAPLEVELMASK_BOTH,
                                          src);
      }
      d->texhnd.loaded = true;
    }
  }
}

void GlideTextureCache::Unload(csGlideCacheData *d)
{
  manager->freeSpaceMem(d->mempos);
  delete d->mempos;
  delete d;
}

csGlideCacheData * GlideTextureCache::LoadHalo(char *data)
{
  CHK (csGlideCacheData *d = new csGlideCacheData);

  GrLOD_t lod;
  d->texhnd.loaded = false;
  if (CalculateTexData ( 128, 128, 1.0, 1.0, &lod, 1, d ))
  {
    d->texhnd.tmu = m_tmu;
    d->texhnd.info.format=GR_TEXFMT_ARGB_4444;
    d->texhnd.info.data=data;
    d->texhnd.size = GlideLib_grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH,&d->texhnd.info);
    d->mempos=manager->allocSpaceMem(d->texhnd.size);
    d->texhnd.loadAddress = d->mempos->offset+m_tmu->minAddress;
  
    GlideLib_grTexDownloadMipMap(d->texhnd.tmu->tmu_id, d->texhnd.loadAddress, GR_MIPMAPLEVELMASK_BOTH, &d->texhnd.info);

    d->texhnd.loaded = true;
  }
  return d;
}

void GlideTextureCache::UnloadHalo(csGlideCacheData *d)
{
      manager->freeSpaceMem(d->mempos);
      delete d->mempos;
      delete d;
}

GlideLightmapCache::GlideLightmapCache(TMUInfo *t,TextureMemoryManager*man)
: HighColorCache(t->memory_size, HIGHCOLOR_LITCACHE, 16,man)
{
  m_tmu = t;
}

void GlideLightmapCache::Dump()
{
  //CsPrintf (MSG_CONSOLE, "Lightmaps in the cache: %d\n", num);
  //CsPrintf (MSG_CONSOLE, "Total size: %ld bytes\n", total_size);
  int mean;
  if (num == 0) mean = 0;
  else mean = total_size/num;
  //CsPrintf (MSG_CONSOLE, "Bytes per lightmap: %d\n", mean);
  //CsPrintf (MSG_CONSOLE, "Fragmentation of Memory: %d\n",manager->getFragmentationState());
}

void GlideLightmapCache::Load(csGlideCacheData *d)
{

  iLightMap *piLM = QUERY_INTERFACE (d->pSource, iLightMap);

  int width = piLM->GetWidth();
  int height = piLM->GetHeight();

  float rheight = piLM->GetRealHeight();
  float rwidth = piLM->GetRealWidth();

  GrLOD_t lod;
  d->texhnd.loaded=false;

  if (CalculateTexData ( width, height, rwidth/width, rheight/height, &lod, 1, d ))
  {
  
    // get the red, green, and blue lightmaps.
    unsigned char *lpRed;
    unsigned char *lpGreen;
    unsigned char *lpBlue;
    
    lpRed = piLM->GetMap(0);
    lpGreen = piLM->GetMap(1);
    lpBlue = piLM->GetMap(2);
    
    unsigned short *mem = new unsigned short[width*height];
    int i,j;
    unsigned short *lpS=mem;
    unsigned short max=127;

    for(j=0; j<height; j++)
    {
      for(i=0; i<width; i++)
      {
        unsigned short r,g,b;
        
        r = *lpRed++;
        g = *lpGreen++;
        b = *lpBlue++;

        // gsteenss: changed to test 2*light*text
        // we must ensure r,g,b are below 128
        if (r>max) r=max;  
        if (g>max) g=max;  
        if (b>max) b=max;  
        
        *lpS = 2*(((r >> 3) << 11) |
                 ((g >> 2) << 5) |
                 ((b >> 3)));
        lpS++;
      }
    }
    
    d->texhnd.tmu = m_tmu;
    d->texhnd.info.format=GR_TEXFMT_RGB_565;
    d->texhnd.info.data=mem;
    d->texhnd.size = GlideLib_grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH,&d->texhnd.info);
    d->mempos=manager->allocSpaceMem(d->texhnd.size);
    d->texhnd.loadAddress = d->mempos->offset+m_tmu->minAddress;
    
    GlideLib_grTexDownloadMipMap(d->texhnd.tmu->tmu_id, d->texhnd.loadAddress, GR_MIPMAPLEVELMASK_BOTH, &d->texhnd.info);
    d->texhnd.loaded=true;
    delete [] mem;
  }  
  piLM->DecRef ();
}


void GlideLightmapCache::Unload(csGlideCacheData *d)
{
  manager->freeSpaceMem(d->mempos);
  delete d->mempos;
  delete d;
}

