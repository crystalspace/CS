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

// HICACHE.CPP
// Hi-color texture/lightmap cache
// An abstract class: derive the specific class from this class
// Written by Dan Ogles
#include "cssysdef.h"
#include "cssys/system.h"
#include "gltex.h"
#include "glcache.h"
#include "gllib.h"
#include "ivideo/itexture.h"
#include "iengine/itexture.h"
#include "iengine/ipolygon.h"
#include "iengine/ilghtmap.h"

IMPLEMENT_IBASE (csGlideTextureCache)
IMPLEMENT_IBASE_END

csGlideTextureCache::csGlideTextureCache (TMUInfo *tmu, int bpp, TextureMemoryManager * tm)
{
    CONSTRUCT_IBASE (NULL);
    m_tmu = tmu;
    cache_size= m_tmu->memory_size;
    num=0;
    total_size=0;
    head=tail=NULL;
    this->bpp = bpp;
    manager = tm;
}

csGlideTextureCache::~csGlideTextureCache()
{
    Clear();
}

void csGlideTextureCache::Remove (iTextureHandle *texture)
{
  if (texture->GetCacheData ())
  {
    Unload ((csGlideCacheData*)texture->GetCacheData ());
    texture->SetCacheData (NULL);
  }
    
}

void csGlideTextureCache::Add(iTextureHandle *texture, bool alpha)
{
  int size = 0;
  int nMM=0;
  for (int c =0; c<4; c++)
  {
    int width, height;
    if (texture->GetMipMapDimensions( c, width, height ))
    {
      size += width * height;
      nMM++;
    };  
  }

  size *= alpha ? 1 : bpp/8;

  csGlideCacheData *cached_texture = (csGlideCacheData *)texture->GetCacheData ();
  if (cached_texture)
  {
    // move unit to front (MRU)
    if(cached_texture != head)
    {
      if(cached_texture->prev) cached_texture->prev->next = cached_texture->next;
      else head = cached_texture->next;
      if(cached_texture->next) cached_texture->next->prev = cached_texture->prev;
      else tail = cached_texture->prev;

      cached_texture->prev = NULL;
      cached_texture->next = head;
      if(head) head->prev = cached_texture;
      else tail = cached_texture;
      head = cached_texture;
    }
  }
  else
  {
    if ( size > cache_size )
    {
      printf( "Knock knock, you try to load a texture of size %d (incl. mipmaps) but your TMU can only handle %ld\n", size, cache_size );
      exit(1);
    }
    // unit is not in memory. load it into the cache
    //      printf( "load a texture of size %d,  TMU can handle %ld\n", size, cache_size );
    while ((total_size + size >= cache_size) || !manager->hasFreeSpace(size))
    {
      // out of memory. remove units from bottom of list.
      cached_texture = tail;

      iTextureHandle *texh = (iTextureHandle *)cached_texture->pSource;
      texh->SetCacheData (NULL);
      Unload(cached_texture);          // unload it.
    }
    
    cached_texture = new csGlideCacheData;

    cached_texture->pSource = texture;
    cached_texture->lSize = size;
    cached_texture->texhnd.type = alpha ? CS_GLIDE_ALPHACACHE : CS_GLIDE_TEXCACHE;
    Load (cached_texture,nMM);       // load it.
    if ( cached_texture->texhnd.loaded )
    {
      // now load the unit.
      num++;
      total_size += size;
      cached_texture->next = head;
      cached_texture->prev = NULL;
      if (head) 
        head->prev = cached_texture;
      else 
        tail = cached_texture;
      head = cached_texture;
      texture->SetCacheData (cached_texture);
    }
    else
    {
      delete cached_texture;
      texture->SetCacheData (NULL);
    }
  }
}

void csGlideTextureCache::Add(iPolygonTexture *polytex)
{
  iLightMap *piLM = polytex->GetLightMap ();
  if (!piLM) return;

  int width = piLM->GetWidth();
  int height = piLM->GetHeight();
  int size = width*height*(bpp/8);
  csGlideCacheData *cached_texture = (csGlideCacheData *)piLM->GetCacheData ();
    
  if (polytex->RecalculateDynamicLights() && cached_texture)
  {
    piLM->SetCacheData (NULL);
    
    Unload(cached_texture);          // unload it.
    cached_texture = NULL;
  }

  if (cached_texture)
  {
    // move unit to front (MRU)
    if(cached_texture != head)
    {
      if(cached_texture->prev) cached_texture->prev->next = cached_texture->next;
      else head = cached_texture->next;
      if(cached_texture->next) cached_texture->next->prev = cached_texture->prev;
      else tail = cached_texture->prev;

      cached_texture->prev = NULL;
      cached_texture->next = head;
      if(head) head->prev = cached_texture;
      else tail = cached_texture;
      head = cached_texture;
    }
  }
  else
  {
    // unit is not in memory. load it into the cache
    while((total_size + size >= cache_size ) || !manager->hasFreeSpace(size))
    {
      // out of memory. remove units from bottom of list.
      cached_texture = tail;
      iLightMap *ilm = (iLightMap *)cached_texture->pSource;
      assert( ilm );

      ilm->SetCacheData(NULL);

      Unload(cached_texture);          // unload it.
            
    }
    
    cached_texture = new csGlideCacheData;

    cached_texture->pSource = piLM;
    cached_texture->lSize = size;
    cached_texture->texhnd.type = CS_GLIDE_LITCACHE;
    
    Load(cached_texture);       // load it.
    if ( cached_texture->texhnd.loaded )
    {
      num++;
      total_size += size;
      cached_texture->next = head;
      cached_texture->prev = NULL;
      if (head) 
        head->prev = cached_texture;
      else 
        tail = cached_texture;
      head = cached_texture;
      piLM->SetCacheData(cached_texture);
    }
    else
    {
      delete cached_texture;
      piLM->SetCacheData (NULL);
    }
  }
}

void csGlideTextureCache::Clear()
{
  while(head)
  {
    csGlideCacheData *n = head;
    head = head->next;
    if (n->texhnd.type & ( CS_GLIDE_TEXCACHE|CS_GLIDE_ALPHACACHE ) ){
      iTextureHandle *texh = (iTextureHandle *)n->pSource;
      texh->SetCacheData (NULL);
    }
    else
    {
      iLightMap *texh = (iLightMap *)n->pSource;
      texh->SetCacheData (NULL);
    }
    Unload(n);
  }    

  head = tail = NULL;
  total_size = 0;
  num = 0;
}

bool csGlideTextureCache::CalculateTexData ( int width, int height, float wfak, float hfak, 
                                        GrLOD_t *lod, int nLod, csGlideCacheData *d)
{
  bool succ=true;
  int texwidth = 0, texheight = 0;
  int lodoff = 0;
  GrAspectRatio_t aspectRatio = GR_ASPECT_LOG2_1x1;
  GrLOD_t lodlevels[]= { GR_LOD_LOG2_256, 
                         GR_LOD_LOG2_128, 
			 GR_LOD_LOG2_64, 
			 GR_LOD_LOG2_32, 
			 GR_LOD_LOG2_16, 
			 GR_LOD_LOG2_8, 
			 GR_LOD_LOG2_4,
			 GR_LOD_LOG2_2,
			 GR_LOD_LOG2_1,
			 -1,-1,-1,-1 };
  
  if(width>=height)
  {
    switch(width/height)
    {
    case 1:
      aspectRatio = GR_ASPECT_LOG2_1x1;
      texwidth    = int(256. * wfak);
      texheight   = int(256. * hfak);
      break;
    case 2:
      aspectRatio = GR_ASPECT_LOG2_2x1;
      texwidth    = int(256. * wfak);
      texheight   = int(128. * hfak);
      break;
    case 4:
      aspectRatio = GR_ASPECT_LOG2_4x1;
      texwidth    = int(256. * wfak);
      texheight   = int(64. * hfak);
      break;
    case 8:
      aspectRatio = GR_ASPECT_LOG2_8x1;
      texwidth    = int(256. * wfak);
      texheight   = int(32. * hfak);
      break;
    default:
      succ=false;
      break;
    }
  }
  else if(height>width)
  {
    switch(height/width)
    {
    case 2:
      aspectRatio = GR_ASPECT_LOG2_1x2;
      texwidth    = int(128. * wfak);
      texheight   = int(256. * hfak);
      break;
    case 4:
      aspectRatio = GR_ASPECT_LOG2_1x4;
      texwidth    = int(64. * wfak);
      texheight   = int(256. * hfak);
      break;
    case 8:
      aspectRatio = GR_ASPECT_LOG2_1x8;
      texwidth    = int(32. * wfak);
      texheight   = int(256. * hfak);
      break;
    default:
      succ=false;
      break;
    }
  }
  
  if (succ)
  {
    d->texhnd.info.aspectRatioLog2 = aspectRatio;
    d->texhnd.width            = texwidth;
    d->texhnd.height           = texheight;
  }
  else
  {
    //Printf(MSG_CONSOLE,"GlideError : Texture Ratio: (%dx%d)\n",width,height);
    return false;
  }

  switch(MAX(width,height))
  {
  case 256:
    lodoff=0;
    break;
  case 128:
    lodoff=1;
    break;
  case 64:
    lodoff=2;
    break;
  case 32:
    lodoff=3;
    break;
  case 16:
    lodoff=4;
    break;
  case 8:
    lodoff=5;
    break;
  case 4:
    lodoff=6;
    break;
  case 2:
    lodoff=7;
    break;
  case 1:
    lodoff=8;
    break;
  default:
    succ=false;
    break;
  }

  if (succ)
  {
    memcpy( lod, (lodlevels+lodoff), nLod * sizeof(GrLOD_t));
    d->texhnd.info.largeLodLog2 = lod[0];
    d->texhnd.info.smallLodLog2 = lod[ MIN( nLod-1, 8 - lodoff ) ];
  }
  else
  {
    //CsPrintf(MSG_CONSOLE,"GlideError : Can't compute lod-level because tex-size is not pow of 2 (actual size is: (%dx%d))\n",width,height);
    return false;
  }

  return succ;
}

void csGlideTextureCache::Unload ( csGlideCacheData *d )
{
  if ( d == head ) head = d->next;
  if ( d == tail ) tail = d->prev;
  if ( d->prev ) d->prev->next = d->next;
  if ( d->next ) d->next->prev = d->prev;
  manager->freeSpaceMem ( d->mempos );
  total_size -= d->lSize;
  num--;
  delete d->mempos;
  delete d;
}
 
void csGlideTextureCache::LoadTex(csGlideCacheData *d, int nMM)
{
  iTextureHandle* txt_handle = (iTextureHandle*)d->pSource;
  csTextureHandle* txt_mm = (csTextureHandle*)txt_handle->GetPrivateObject ();
  csTextureGlide* txt_unl = (csTextureGlide *)txt_mm->get_texture (0);
  int i;
  int width = txt_unl->get_width ();
  int height = txt_unl->get_height ();

  GrLOD_t lod[nMM];
  d->texhnd.loaded = false;
 
  if (CalculateTexData ( width, height, 1, 1, lod, nMM, d ))
  {
    d->texhnd.info.format=GR_TEXFMT_RGB_565;
    d->texhnd.tmu = m_tmu;
    d->texhnd.info.data = (unsigned char*)txt_unl->get_bitmap();
    d->texhnd.size = GlideLib_grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH,&d->texhnd.info);

    /* if(d->texhnd.size!=d->size)
    {
      CsPrintf(MSG_CONSOLE,"Ooups size! (%d != %d)\n",texhnd->size,d->size);
      }*/
  
    d->mempos=manager->allocSpaceMem(d->texhnd.size);
    d->texhnd.loadAddress = d->mempos->offset+m_tmu->minAddress;

    unsigned char* src=NULL;
    // Download all mipmaps
    for( i=0; i<nMM; i++)
    {
      if (lod[i]!=-1)
      {
      
        csTextureGlide *txt_mip = (csTextureGlide *)txt_mm->get_texture (i);
        src = (unsigned char*)txt_mip->get_bitmap();

        GlideLib_grTexDownloadMipMapLevel(d->texhnd.tmu->tmu_id,
                                          d->texhnd.loadAddress,
                                          lod[i],
                                          d->texhnd.info.largeLodLog2,
                                          d->texhnd.info.aspectRatioLog2,
                                          d->texhnd.info.format,
                                          GR_MIPMAPLEVELMASK_BOTH,
                                          src);
      }
      d->texhnd.loaded = true;
    }
  }
}

void csGlideTextureCache::LoadLight (csGlideCacheData *d)
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

void csGlideTextureCache::LoadAlpha (csGlideCacheData *d)
{

  iTextureHandle* txt_handle = (iTextureHandle*)d->pSource;
  csGlideAlphaMap* txt_mm = (csGlideAlphaMap*)txt_handle->GetPrivateObject ();
  
  int width, height;  
  txt_mm->GetMipMapDimensions (0, width, height );

  GrLOD_t lod;
  d->texhnd.loaded = false;
 
  if (CalculateTexData ( width, height, 1, 1, &lod, 1, d ))
  {
    d->texhnd.info.format=GR_TEXFMT_ALPHA_8;
    d->texhnd.tmu = m_tmu;
    d->texhnd.info.data=(unsigned char*)txt_mm->GetAlphaMapData ();
    d->texhnd.size = GlideLib_grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH,&d->texhnd.info);

    d->mempos=manager->allocSpaceMem(d->texhnd.size);
    d->texhnd.loadAddress = d->mempos->offset+m_tmu->minAddress;

    GlideLib_grTexDownloadMipMapLevel(d->texhnd.tmu->tmu_id,
                                      d->texhnd.loadAddress,
                                      lod,
                                      d->texhnd.info.largeLodLog2,
                                      d->texhnd.info.aspectRatioLog2,
                                      d->texhnd.info.format,
                                      GR_MIPMAPLEVELMASK_BOTH,
                                      d->texhnd.info.data);
    d->texhnd.loaded = true;
    
  }
}

void csGlideTextureCache::Load (csGlideCacheData *d, int nMM)
{
  switch ( d->texhnd.type )
  {
  case CS_GLIDE_TEXCACHE:
    LoadTex ( d, nMM );
    break;
  case CS_GLIDE_LITCACHE:
    LoadLight ( d );
    break;
  case CS_GLIDE_ALPHACACHE:
    LoadAlpha ( d );
    break;
  default:
    // wtf ?
    break;
  }
}

void csGlideTextureCache::Dump ()
{
  //CsPrintf (MSG_CONSOLE, "Textures in the cache: %d\n", num);
  //CsPrintf (MSG_CONSOLE, "Total size: %ld bytes\n", total_size);
  int mean;
  if (num == 0) mean = 0;
  else mean = total_size/num;
  //CsPrintf (MSG_CONSOLE, "Bytes per texture: %d\n", mean);
  //CsPrintf (MSG_CONSOLE, "Fragmentation of Memory: %d\n",manager->getFragmentationState());
}
