/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#include "sysdef.h"
#include "cssys/system.h"
#include "cs3d/glide2/gl_txtmgr.h"
#include "cs3d/glide2/hicache2.h"
#include "itexture.h"
#include "ipolygon.h"
#include "ilghtmap.h"

HighColorCacheAndManage::HighColorCacheAndManage(int max_size, HIGHCOLOR_TYPE type, int bpp,TextureMemoryManager * tm) : HighColorCache(max_size,type,bpp)
{
  // New For management.
  manager=tm;
}

HighColorCacheAndManage::~HighColorCacheAndManage()
{
  Clear();
}

void HighColorCacheAndManage::Add(iTextureHandle *texture)
{
  // gsteenss: original dataptr...
  //HighColorCacheAndManage_Data *cached_texture;
  csHighColorCacheData *cached_texture;
  int size = 0;
  
  if(type!=HIGHCOLOR_TEXCACHE) return;

  for (int c =0; c<4; c++)
  {
    int width, height;
    texture->GetMipMapDimensions( c, width, height );    
    size += width * height;
  }

  size *= bpp/8;

  bool bIsInVideoMemory;
  
  csTextureMMGlide* txt_mm = (csTextureMMGlide*)texture->GetPrivateObject ();
  bIsInVideoMemory = txt_mm->IsCached ();
  
  if (bIsInVideoMemory)
  {
    // move unit to front (MRU)

    
    cached_texture = txt_mm->GetHighColorCacheData ();
    if(!cached_texture) return;

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
                        iTextureHandle* piMMC;
                        HighColorCacheAndManage_Data* l = (HighColorCacheAndManage_Data*)tail;

                        piMMC = QUERY_INTERFACE( l->pSource, iTextureHandle );

      tail = tail->prev;
      if(tail) tail->next = NULL;
      else head = NULL;
      l->prev = NULL;

                        csTextureMMGlide* txt_mm2 = (csTextureMMGlide*)piMMC->GetPrivateObject ();
                        txt_mm2->SetInCache(false);
                        txt_mm2->SetHighColorCacheData (NULL);
                        Unload(l);          // unload it.
//      manager->freeSpaceMem(l->mempos);
      delete l->mempos;
      l->mempos=0;
      num--;
      total_size -= l->lSize;
    }
    
    // now load the unit.
    num++;
    total_size += size;

    CHK (cached_texture = new HighColorCacheAndManage_Data);

    cached_texture->next = head;
    cached_texture->prev = NULL;
    if(head) head->prev = cached_texture;
    else tail = cached_texture;
    head = cached_texture;
    cached_texture->pSource = texture;
    cached_texture->lSize = size;

                txt_mm->SetInCache(true);
                txt_mm->SetHighColorCacheData (cached_texture);

//    cached_texture->mempos=manager->allocSpaceMem(size);
    Load(cached_texture);       // load it.
  }
}

void HighColorCacheAndManage::Add(iPolygonTexture *polytex)
{
  HighColorCacheAndManage_Data *cached_texture=NULL;
  HighColorCacheAndManage_Data* l=NULL;

  iLightMap *piLM = polytex->GetLightMap ();
  if (!piLM) return;

  if(type!=HIGHCOLOR_LITCACHE) return;

  int width = piLM->GetWidth();
  int height = piLM->GetHeight();
  int size = width*height*(bpp/8);
    
  if (polytex->RecalculateDynamicLights() && piLM->IsCached())
  {
    l = (HighColorCacheAndManage_Data*)piLM->GetHighColorCache ();
    
    if(l->next)
      l->next->prev = l->prev;
    if(l->prev)
      l->prev->next = l->next;
    
    piLM->SetInCache(false);
    piLM->SetHighColorCache(NULL);
    
    Unload(l);          // unload it.
    //      manager->freeSpaceMem(l->mempos);
    delete l->mempos;
    l->mempos=0;            
    num--;
    total_size -= l->lSize;
  }

  if (piLM->IsCached())
  {
    // move unit to front (MRU)

    cached_texture = (HighColorCacheAndManage_Data*)piLM->GetHighColorCache ();

    if(!cached_texture) return;

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
      iLightMap* ilm;
      
      ilm = QUERY_INTERFACE( l->pSource, iLightMap );
      assert( ilm );

      tail = tail->prev;
      if(tail) tail->next = NULL;
      else head = NULL;
      l->prev = NULL;

      ilm->SetInCache(false);
      ilm->SetHighColorCache(NULL);

      Unload(l);          // unload it.
//      manager->freeSpaceMem(l->mempos);
      delete l->mempos;
      l->mempos=0;            
            
      num--;
      total_size -= l->lSize;

      ilm->DecRef();
    }
    
    // now load the unit.
    num++;
    total_size += size;

    CHK (cached_texture = new HighColorCacheAndManage_Data);

    cached_texture->next = head;
    cached_texture->prev = NULL;
    if(head) head->prev = cached_texture;
    else tail = cached_texture;
    head = cached_texture;
    cached_texture->pSource = piLM;
    cached_texture->lSize = size;
    
    piLM->SetInCache(true);
    piLM->SetHighColorCache(cached_texture);
    
    cached_texture->pData = NULL;
//    cached_texture->mempos=manager->allocSpaceMem(size);
    Load(cached_texture);       // load it.
  }
}

void HighColorCacheAndManage::Clear()
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
      IMipMapContainer* piMMC = NULL;
      
      head->pSource->QueryInterface( IID_IMipMapContainer, (void**)piMMC );
      assert( piMMC );
      
      piMMC->SetHighColorCache(NULL);
      piMMC->SetInCache(false);
        
      piMMC->DecRef();
    }
    else if(type==HIGHCOLOR_LITCACHE)
    {
      iLightMap* piLM;
    
      head->pSource->QueryInterface( IID_iLightMap, (void**)piLM );
      assert( piLM );
      
      piLM->SetHighColorCache(NULL);
      piLM->SetInCache(false);
        
      piLM->DecRef();
    }
    head = n;
  }
*/  
  head = tail = NULL;
  total_size = 0;
  num = 0;
}
