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
#include "cs3d/glide2/hicache2.h"
#include "cs3d/glide2/gl_txtmgr.h"
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

void HighColorCacheAndManage::Add(ITextureHandle *texture)
{
  // gsteenss: original dataptr...
  //HighColorCacheAndManage_Data *cached_texture;
  HighColorCache_Data *cached_texture;
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
  
  csTextureMMGlide* txt_mm = (csTextureMMGlide*)GetcsTextureMMFromITextureHandle (texture);
  bIsInVideoMemory = txt_mm->is_in_videomemory ();
  
  if (bIsInVideoMemory)
	{
		// move unit to front (MRU)

    
    cached_texture = txt_mm->get_hicolorcache ();
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
                        ITextureHandle* piMMC;
                        HighColorCacheAndManage_Data* l = (HighColorCacheAndManage_Data*)tail;

                        l->pSource->QueryInterface( IID_ITextureHandle, (void**)&piMMC );

			tail = tail->prev;
			if(tail) tail->next = NULL;
			else head = NULL;
			l->prev = NULL;

                        csTextureMMGlide* txt_mm2 = (csTextureMMGlide*)GetcsTextureMMFromITextureHandle (piMMC);
                        txt_mm2->set_in_videomemory (false);
                        txt_mm2->set_hicolorcache (NULL);
                        Unload(l);					// unload it.
//			manager->freeSpaceMem(l->mempos);
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

                txt_mm->set_in_videomemory (true);
                txt_mm->set_hicolorcache (cached_texture);

//		cached_texture->mempos=manager->allocSpaceMem(size);
		Load(cached_texture);				// load it.
	}
}

void HighColorCacheAndManage::Add(IPolygonTexture *polytex)
{
	HighColorCacheAndManage_Data *cached_texture;
  ILightMap *piLM; 
	HighColorCacheAndManage_Data* l;

  polytex->GetLightMap( &piLM );
  if (!piLM) return;

	if(type!=HIGHCOLOR_LITCACHE) return;

  int size, width, height;
    
  piLM->GetWidth(width);
  piLM->GetHeight(height);
  size = width*height*(bpp/8);
    
  bool dl;
  polytex->RecalculateDynamicLights(dl);

  bool bInVideoMemory;
  piLM->GetInVideoMemory( bInVideoMemory );
  
  if (dl && bInVideoMemory)
	{
    piLM->GetHighColorCache((HighColorCache_Data**)&l);
    
    if(l->next)
      l->next->prev = l->prev;
    if(l->prev)
      l->prev->next = l->next;
    
    piLM->SetInVideoMemory(false);
    piLM->SetHighColorCache(NULL);
    
    Unload(l);					// unload it.
    //			manager->freeSpaceMem(l->mempos);
    delete l->mempos;
    l->mempos=0;						
    num--;
    total_size -= l->lSize;
    
    piLM->Release();
	}

  piLM->GetInVideoMemory( bInVideoMemory );
  if (bInVideoMemory)
	{
		// move unit to front (MRU)

    piLM->GetHighColorCache((HighColorCache_Data**)&cached_texture);

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
      ILightMap* ilm;
      
      l->pSource->QueryInterface( IID_ILightMap, (void**)&ilm );
      ASSERT( ilm );

			tail = tail->prev;
			if(tail) tail->next = NULL;
			else head = NULL;
			l->prev = NULL;

      ilm->SetInVideoMemory(false);
      ilm->SetHighColorCache(NULL);

			Unload(l);					// unload it.
//			manager->freeSpaceMem(l->mempos);
			delete l->mempos;
			l->mempos=0;						
						
			num--;
			total_size -= l->lSize;

      ilm->Release();
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
    
    piLM->SetInVideoMemory(true);
    piLM->SetHighColorCache(cached_texture);
    
    cached_texture->pData = NULL;
//		cached_texture->mempos=manager->allocSpaceMem(size);
		Load(cached_texture);				// load it.
	}

  FINAL_RELEASE( piLM );
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
      piMMC->SetInVideoMemory(false);
        
      piMMC->Release();
    }
    else if(type==HIGHCOLOR_LITCACHE)
    {
      ILightMap* piLM;
    
      head->pSource->QueryInterface( IID_ILightMap, (void**)piLM );
      assert( piLM );
      
      piLM->SetHighColorCache(NULL);
      piLM->SetInVideoMemory(false);
        
      piLM->Release();
    }
 		head = n;
	}
*/	
	head = tail = NULL;
	total_size = 0;
	num = 0;
}
