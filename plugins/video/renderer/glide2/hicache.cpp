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
#include "sysdef.h"
#include "cssys/system.h"
#include "cs3d/glide2/gl_txtmgr.h"
#include "cs3d/glide2/hicache.h"
#include "itexture.h"
#include "ipolygon.h"
#include "ilghtmap.h"

HighColorCache::HighColorCache(int max_size, HIGHCOLOR_TYPE type, int bpp, TextureMemoryManager * tm)
{
    this->type = type;
    cache_size=max_size;
    num=0;
    total_size=0;
    head=tail=NULL;
    this->bpp = bpp;
    manager = tm;
}

HighColorCache::~HighColorCache()
{
    Clear();
}

void HighColorCache::Add(iTextureHandle *texture)
{
  // gsteenss: original dataptr...
  //HighColorCacheAndManage_Data *cached_texture;
  int size = 0;
  
  if(type!=HIGHCOLOR_TEXCACHE) return;

  for (int c =0; c<4; c++)
  {
    int width, height;
    if (texture->GetMipMapDimensions( c, width, height ))
      size += width * height;
  }

  size *= bpp/8;

  bool bIsInVideoMemory;
  
  csTextureMMGlide* txt_mm = (csTextureMMGlide*)texture->GetPrivateObject ();
  csGlideCacheData *cached_texture = (csGlideCacheData *)txt_mm->GetCacheData ();
  
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
    while((total_size + size >= cache_size)&&(manager->hasFreeSpace(size)))
    {
      // out of memory. remove units from bottom of list.
      cached_texture = tail;

      iTextureHandle *texh = (iTextureHandle *)cached_texture->pSource;

      tail = tail->prev;
      if(tail) tail->next = NULL;
      else head = NULL;
      cached_texture->prev = NULL;
      total_size -= cached_texture->lSize;
      num--;

      csTextureMMGlide* txt_mm2 = (csTextureMMGlide*)texh->GetPrivateObject ();
      txt_mm2->SetCacheData (NULL);
      Unload(cached_texture);          // unload it.
    }
    
    // now load the unit.
    num++;
    total_size += size;

    CHK (cached_texture = new csGlideCacheData);

    cached_texture->next = head;
    cached_texture->prev = NULL;
    if(head) head->prev = cached_texture;
    else tail = cached_texture;
    head = cached_texture;
    cached_texture->pSource = texture;
    cached_texture->lSize = size;

    txt_mm->SetCacheData (cached_texture);

    Load(cached_texture);       // load it.
  }
}

void HighColorCache::Add(iPolygonTexture *polytex)
{
  iLightMap *piLM = polytex->GetLightMap ();
  if (!piLM) return;

  if (type != HIGHCOLOR_LITCACHE) return;

  int width = piLM->GetWidth();
  int height = piLM->GetHeight();
  int size = width*height*(bpp/8);

  csGlideCacheData *cached_texture = (csGlideCacheData *)piLM->GetCacheData ();
    
  if (polytex->RecalculateDynamicLights() && cached_texture)
  {
    if(cached_texture->next)
      cached_texture->next->prev = cached_texture->prev;
    if(cached_texture->prev)
      cached_texture->prev->next = cached_texture->next;
    
    piLM->SetCacheData (NULL);
    num--;
    total_size -= cached_texture->lSize;
    
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
    while(total_size + size >= cache_size)
    {
      // out of memory. remove units from bottom of list.
      cached_texture = tail;
      iLightMap *ilm = (iLightMap *)cached_texture->pSource;
      assert( ilm );

      tail = tail->prev;
      if (tail)
        tail->next = NULL;
      else
        head = NULL;
      cached_texture->prev = NULL;

      ilm->SetCacheData(NULL);

      num--;
      total_size -= cached_texture->lSize;
      
      Unload(cached_texture);          // unload it.
            
    }
    
    // now load the unit.
    num++;
    total_size += size;

    CHK (cached_texture = new csGlideCacheData);

    cached_texture->next = head;
    cached_texture->prev = NULL;
    if(head) head->prev = cached_texture;
    else tail = cached_texture;
    head = cached_texture;
    cached_texture->pSource = piLM;
    cached_texture->lSize = size;
    
    piLM->SetCacheData(cached_texture);
    
    cached_texture->pData = NULL;
    Load(cached_texture);       // load it.
  }
}

void HighColorCache::Clear()
{
/*
  while(head)
  {
    HighColorCacheAndManage_Data *n = (HighColorCacheAndManage_Data*)head->next;
    head->next = head->prev = NULL;
        
    Unload(head);
    manager->freeSpaceMem(((HighColorCacheAndManage_Data *)head)->mempos);
    delete ((HighColorCacheAndManage_Data *)head)->mempos;
    ((HighColorCacheAndManage_Data *)head)->mempos=0;           

    if(type==HIGHCOLOR_TEXCACHE) 
    {
      IMipMapContainer* texh = NULL;
      
      head->pSource->QueryInterface( IID_IMipMapContainer, (void**)texh );
      assert( texh );
      
      texh->SetCacheData(NULL);
    }
    else if(type==HIGHCOLOR_LITCACHE)
    {
      iLightMap* piLM;
    
      head->pSource->QueryInterface( IID_iLightMap, (void**)piLM );
      assert( piLM );
      
      piLM->SetCacheData(NULL);
    }
    head = n;
  }
*/  
  while(head)
  {
    csGlideCacheData *n = head;
    head = head->next;
        
    Unload(n);
  }    

  head = tail = NULL;
  total_size = 0;
  num = 0;
}
