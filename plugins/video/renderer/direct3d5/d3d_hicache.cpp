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

#include "video/renderer/direct3d5/d3d_hicache.h"
#include "video/renderer/direct3d5/d3d_txtmgr.h"
#include "itexture.h"
#include "ipolygon.h"
#include "ilghtmap.h"

HighColorCache::HighColorCache(int max_size, HIGHCOLOR_TYPE type, int bpp)
{
    HighColorCache::type = type;
    cache_size=max_size;
    num=0;
    total_size=0;
    head=tail=NULL;
    HighColorCache::bpp = bpp;
}

HighColorCache::~HighColorCache()
{
    Clear();
}

void HighColorCache::Add (iTextureHandle *texture)
{
    iTextureMap* piTM = NULL;
    int size = 0;
    
    if(type!=HIGHCOLOR_TEXCACHE) return;
    
    for (int c =0; c<4; c++)
    {
        int width, height;
        
        if (texture->GetMipMapDimensions (c, width, height))
          size += width * height;
    }
    
    size *= bpp/8;
    
    csTextureMMDirect3D *txt_mm = (csTextureMMDirect3D*)texture->GetPrivateObject ();
    csD3DCacheData *cached_texture = (csD3DCacheData *)txt_mm->GetCacheData ();
    
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
        while (total_size + size >= cache_size)
        {
            // out of memory. remove units from bottom of list.
            cached_texture = tail;
            iTextureHandle *texh = (iTextureHandle *)cached_texture->pSource;
            texh->SetCacheData (NULL);

            tail = tail->prev;
            if (tail)
              tail->next = NULL;
            else
              head = NULL;
            cached_texture->prev = NULL;

            Unload (cached_texture);			// unload it.
                        
            num--;
            total_size -= cached_texture->lSize;

            delete cached_texture;
        }
        
        // now load the unit.
        num++;
        total_size += size;
        
        CHK (cached_texture = new csD3DCacheData);
        
        cached_texture->next = head;
        cached_texture->prev = NULL;
        if (head)
          head->prev = cached_texture;
        else
          tail = cached_texture;
        head = cached_texture;
        cached_texture->pSource = texture;
        cached_texture->lSize = size;

        txt_mm->SetCacheData (cached_texture);

        cached_texture->pData = NULL;
        Load(cached_texture);				// load it.
    }
}

void HighColorCache::Add(iPolygonTexture *polytex)
{
    csD3DCacheData *cached_texture;
    csD3DCacheData* l = NULL;

    iLightMap *piLM = polytex->GetLightMap ();
    if (!piLM)
      return;
    
    if (type != HIGHCOLOR_LITCACHE)
      return;
    
    int width = piLM->GetWidth();
    int height = piLM->GetHeight();
    int size = width*height*(bpp/8);
    
    if (polytex->RecalculateDynamicLights () && piLM->IsCached ())
    {
        l = (csD3DCacheData *)piLM->GetCacheData ();
        
        if (l->prev)
                l->prev->next = l->next;

        if (l->next) 
                l->next->prev = l->prev;
        
        
        Unload(l);					// unload it.

        piLM->SetCacheData(NULL);
      
        num--;
        total_size -= l->lSize;
        delete l;
    }

    if (piLM->IsCached())
    {
        // move unit to front (MRU)
        
        cached_texture = (csD3DCacheData *)piLM->GetCacheData ();
        //ASSERT(cached_texture);
        
        if (cached_texture != head)
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
        while (total_size + size >= cache_size)
        {
            iLightMap* ilm = QUERY_INTERFACE(l->pSource, iLightMap);
            ASSERT( ilm );
            
            // out of memory. remove units from bottom of list.
            l = tail;
            tail = tail->prev;
            if(tail) tail->next = NULL;
            else head = NULL;
            l->prev = NULL;
            ilm->SetCacheData(NULL);
            
            Unload(l);					// unload it.
            
            num--;
            total_size -= l->lSize;

            delete l;
        }
        
        // now load the unit.
        num++;
        total_size += size;
        
        CHK (cached_texture = new csD3DCacheData);
        
        cached_texture->next = head;
        cached_texture->prev = NULL;
        
        if (head) head->prev = cached_texture;
        else tail = cached_texture;
        head = cached_texture;

        cached_texture->pSource = piLM;
        cached_texture->lSize = size;
        
        piLM->SetCacheData(cached_texture);
        
        cached_texture->pData = NULL;
        Load(cached_texture);				// load it.
    }
}

void HighColorCache::Clear()
{
/*
    while(head)
    {
        csD3DCacheData *n = head->next;
        head->next = head->prev = NULL;
        
        Unload(head);
        
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
        //delete head;

        head = n;
    }
    
    head = tail = NULL;
    total_size = 0;
    num = 0;
*/
}
