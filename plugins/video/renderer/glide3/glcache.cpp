/*
    Copyright (C) 1998 by Jorrit Tyberghein.

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
#include "cssys/common/system.h"
//#include "engine/polygon/polygon.h"	BAD NOT ALLOWED @@@ USE IPOLYGON.H INSTEAD!
//#include "engine/polygon/polytext.h"
//#include "engine/camera.h"
//#include "engine/light/dynlight.h"
//#include "engine/light/lghtmap.h"
#include "cs3d/software/graph3d.h"
#include "cs3d/glide3/g3dglide.h"
#include "cs3d/glide3/glcache.h"

#include "glide.h"

GlideTextureCache::GlideTextureCache(TMUInfo *t, int bpp, TextureMemoryManager *man)
: HighColorCacheAndManage(t->memory_size, HIGHCOLOR_TEXCACHE, bpp,man)
{
	m_tmu = t;
//	m_tmu->currentAddress = m_tmu->minAddress;
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

void GlideTextureCache::Load(HighColorCacheAndManage_Data *d)
{
  IMipMapContainer *piMMC = NULL;
  ITextureMap *piTM = NULL;

  CHK (TextureHandler *texhnd = new TextureHandler);

	bool bdef=true;
  int i,j;
      
  d->pSource->QueryInterface( IID_IMipMapContainer, (void**)&piMMC );
  assert( piMMC != NULL );

  // get the texture map
  piMMC->GetTexture(0, &piTM);
  ASSERT( piTM );

	int height;
	int width;

  piTM->GetWidth(width); 
  piTM->GetHeight(height); 

  GrLOD_t lod[4];
	GrAspectRatio_t aspectRatio=GR_ASPECT_LOG2_1x1;
	
	switch(max(width,height))
	{
	case 256:
		lod[0]=GR_LOD_LOG2_256;
		lod[1]=GR_LOD_LOG2_128;
		lod[2]=GR_LOD_LOG2_64;
		lod[3]=GR_LOD_LOG2_32;
		break;
	case 128:
		lod[0]=GR_LOD_LOG2_128;
		lod[1]=GR_LOD_LOG2_64;
		lod[2]=GR_LOD_LOG2_32;
		lod[3]=GR_LOD_LOG2_16;
		break;
	case 64:
		lod[0]=GR_LOD_LOG2_64;
		lod[1]=GR_LOD_LOG2_32;
		lod[2]=GR_LOD_LOG2_16;
		lod[3]=GR_LOD_LOG2_8;
		break;
	case 32:
		lod[0]=GR_LOD_LOG2_32;
		lod[1]=GR_LOD_LOG2_16;
		lod[2]=GR_LOD_LOG2_8;
		lod[3]=GR_LOD_LOG2_4;
		break;
	case 16:
		lod[0]=GR_LOD_LOG2_16;
		lod[1]=GR_LOD_LOG2_8;
		lod[2]=GR_LOD_LOG2_4;
		lod[3]=GR_LOD_LOG2_2;
		break;
	case 8:
		lod[0]=GR_LOD_LOG2_8;
		lod[1]=GR_LOD_LOG2_4;
		lod[2]=GR_LOD_LOG2_2;
		lod[3]=GR_LOD_LOG2_1;
		break;
	case 4:
		lod[0]=GR_LOD_LOG2_4;
		lod[1]=GR_LOD_LOG2_2;
		lod[2]=GR_LOD_LOG2_1;
		lod[3]=-1;
		break;
	case 2:
		lod[0]=GR_LOD_LOG2_2;
		lod[1]=GR_LOD_LOG2_1;
		lod[2]=-1;
		lod[3]=-1;
		break;
	case 1:
		lod[0]=1;
		lod[1]=-1;
		lod[2]=-1;
		lod[3]=-1;
		break;
	default:
		bdef=false;
		break;
	}
	if(width>height)
	{
		switch(width/height)
		{
		case 2:
			aspectRatio=GR_ASPECT_LOG2_2x1;
			break;
		case 4:
			aspectRatio=GR_ASPECT_LOG2_4x1;
			break;
		case 8:
			aspectRatio=GR_ASPECT_LOG2_8x1;
			break;
		default:
			bdef=false;
			break;
		}
	}
	else if(height>width)
	{
		switch(height/width)
		{
		case 2:
			aspectRatio=GR_ASPECT_LOG2_1x2;
			break;
		case 4:
			aspectRatio=GR_ASPECT_LOG2_1x4;
			break;
		case 8:
			aspectRatio=GR_ASPECT_LOG2_1x8;
			break;
		default:
			bdef=false;
			break;
		}
	}

  switch(aspectRatio)
	{
	case GR_ASPECT_LOG2_1x1:
		width=height=256;
		break;
	case GR_ASPECT_LOG2_2x1:
		width=256;
		height=128;
		break;
	case GR_ASPECT_LOG2_4x1:
		width=256;
		height=64;
		break;
	case GR_ASPECT_LOG2_8x1:
		width=256;
		height=32;
		break;
	case GR_ASPECT_LOG2_1x2:
		width=128;
		height=256;
		break;
	case GR_ASPECT_LOG2_1x4:
		width=64;
		height=256;
		break;
	case GR_ASPECT_LOG2_1x8:
		width=32;
		height=256;
		break;
	}

	if(bdef)
	{
    unsigned char *lpSrc = NULL;
    unsigned short *mem = NULL;
    RGBpaletteEntry* GPalette;

    piTM->GetBitmap(&lpSrc);
    ASSERT( lpSrc != NULL );

    if(bpp==16)
    {
      texhnd->info.format=GR_TEXFMT_RGB_565;

      piMMC->GetPrivateColorMap((unsigned char**)&GPalette);
      ASSERT(GPalette);

      mem = new unsigned short[width*height];
      if(mem == NULL)
        return;
      
      unsigned short *lpS=mem;
      unsigned char *lpySrc;
      for(j=0; j<height; j++)
      {
        //     = (unsigned short *)(((char *)mem) + width * j);
        lpySrc = lpSrc + (j * width);

        for(i=0; i<width; i++)
        {
          unsigned short r,g,b;
          
          r = GPalette[*lpySrc].red;
          g = GPalette[*lpySrc].green;
          b = GPalette[*lpySrc].blue;

          *lpS =  ((r >> 3) << 11) |
            ((g >> 2) << 5) |
            ((b >> 3));

          lpS++;
          lpySrc++;
        }
      }
      lpSrc = (unsigned char *)mem;
    }
    else
      texhnd->info.format=GR_TEXFMT_P_8;
    FINAL_RELEASE( piTM );

    texhnd->tmu = m_tmu;
		for(i=3;lod[i]==-1;i--);
		texhnd->info.smallLodLog2=lod[i];
		texhnd->info.largeLodLog2=lod[0];
		texhnd->info.aspectRatioLog2=aspectRatio;
		texhnd->info.data=lpSrc;
    texhnd->size = grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH,&texhnd->info);

/*		if(texhnd->size!=d->size)
		{
			CsPrintf(MSG_CONSOLE,"Ooups size! (%d != %d)\n",texhnd->size,d->size);
		}
*/	
    d->mempos=manager->allocSpaceMem(texhnd->size);
		texhnd->loadAddress = d->mempos->offset+m_tmu->minAddress;
		texhnd->width=width;
		texhnd->height=height;
		grTexDownloadMipMapLevel(texhnd->tmu->tmu_id,
			texhnd->loadAddress,lod[0],
			texhnd->info.largeLodLog2,
			texhnd->info.aspectRatioLog2,
			texhnd->info.format,
			GR_MIPMAPLEVELMASK_BOTH,
			texhnd->info.data);

		// Download all mipmaps
		for(i=1; i<4; i++)
		{
      if(lod[i]!=-1)
			{
        piMMC->GetTexture(i, &piTM);
        ASSERT( piTM != NULL );
        
        piTM->GetWidth(width);
        piTM->GetHeight(height);
        piTM->GetBitmap(&lpSrc);
        ASSERT( lpSrc != NULL );
        
        if(bpp==16)
        {
          int i,j;
          unsigned short *lpS=mem;
          unsigned char *lpSrcPtr=lpSrc;
          for(j=0; j<height; j++)
          {
            //     = (unsigned short *)(((char *)mem) + width * j);
            
            for(i=0; i<width; i++)
            {
              unsigned short r,g,b;
              
              r = GPalette[*lpSrcPtr].red;
              g = GPalette[*lpSrcPtr].green;
              b = GPalette[*lpSrcPtr].blue;
              
              lpSrcPtr++;
              
              *lpS =  ((r >> 3) << 11) |
                ((g >> 2) << 5) |
                ((b >> 3));
              lpS++;
            }
          }
          lpSrc = (unsigned char *)mem;
        }

        FINAL_RELEASE( piTM );

				grTexDownloadMipMapLevel(texhnd->tmu->tmu_id,
					texhnd->loadAddress,
					lod[i],
					texhnd->info.largeLodLog2,
					texhnd->info.aspectRatioLog2,
					texhnd->info.format,
					GR_MIPMAPLEVELMASK_BOTH,
					lpSrc
					);
      }
		}
    if(mem)
      delete mem;

    //	m_tmu->currentAddress += texhnd->size;
	}
  else
  {
    delete texhnd;
    texhnd = NULL;
  }
	
  FINAL_RELEASE( piMMC );
  d->pData = texhnd;
}

void GlideTextureCache::Unload(HighColorCacheAndManage_Data *d)
{
//	TextureHandler * th = (TextureHandler *)d->data;
	manager->freeSpaceMem(d->mempos);

}

GlideLightmapCache::GlideLightmapCache(TMUInfo *t,TextureMemoryManager*man)
: HighColorCacheAndManage(t->memory_size, HIGHCOLOR_LITCACHE, 16,man)
{
	m_tmu = t;
	
//	m_tmu->currentAddress = m_tmu->minAddress;
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

void GlideLightmapCache::Load(HighColorCacheAndManage_Data *d)
{
	CHK (TextureHandler *texhnd = new TextureHandler);
  ILightMap *piLM;

  VERIFY_SUCCESS( d->pSource->QueryInterface( IID_ILightMap, (void**)&piLM ) );
  ASSERT( piLM );

  int height; 
  int width;

  piLM->GetHeight(height);
  piLM->GetWidth(width);

  ASSERT(!(height%2));
  ASSERT(!(width%2));

  bool bdef=true;
	GrLOD_t lod=GR_LOD_LOG2_1;
	GrAspectRatio_t aspectRatio=GR_ASPECT_LOG2_1x1;
	
	switch(max(width,height))
	{
	case 256:
		lod=GR_LOD_LOG2_256;
		break;
	case 128:
		lod=GR_LOD_LOG2_128;
		break;
	case 64:
		lod=GR_LOD_LOG2_64;
		break;
	case 32:
		lod=GR_LOD_LOG2_32;
		break;
	case 16:
		lod=GR_LOD_LOG2_16;
		break;
	case 8:
		lod=GR_LOD_LOG2_8;
		break;
	case 4:
		lod=GR_LOD_LOG2_4;
		break;
	case 2:
		lod=GR_LOD_LOG2_2;
		break;
	case 1:
		lod=GR_LOD_LOG2_1;
		break;
	default:
		bdef=false;
		//CsPrintf(MSG_CONSOLE,"GlideError : Texture Size: (%dx%d)\n",width,height);
		break;
	}
	if(width>height)
	{
		switch(width/height)
		{
		case 2:
			aspectRatio=GR_ASPECT_LOG2_2x1;
			break;
		case 4:
			aspectRatio=GR_ASPECT_LOG2_4x1;
			break;
		case 8:
			aspectRatio=GR_ASPECT_LOG2_8x1;
			break;
		default:
			bdef=false;
			//CsPrintf(MSG_CONSOLE,"GlideError : Texture Ratio1: (%dx%d)\n",width,height);
			break;
		}
	}
	else if(height>width)
	{
		switch(height/width)
		{
		case 2:
			aspectRatio=GR_ASPECT_LOG2_1x2;
			break;
		case 4:
			aspectRatio=GR_ASPECT_LOG2_1x4;
			break;
		case 8:
			aspectRatio=GR_ASPECT_LOG2_1x8;
			break;
		default:
			bdef=false;
			//CsPrintf(MSG_CONSOLE,"GlideError : Texture Ratio2: (%dx%d)\n",width,height);
			break;
		}
	}
	
	if(bdef)
	{
    
    // get the red, green, and blue lightmaps.
    unsigned char *lpRed;
    unsigned char *lpGreen;
    unsigned char *lpBlue;
    
    piLM->GetMap(0, &lpRed);
    piLM->GetMap(1, &lpGreen);
    piLM->GetMap(2, &lpBlue);
    
    unsigned short *mem = new unsigned short[width*height];
    
    int i,j;
    unsigned short *lpS=mem;
    for(j=0; j<height; j++)
    {
      //     = (unsigned short *)(((char *)mem) + width * j);
      
      for(i=0; i<width; i++)
      {
        unsigned short r,g,b;
        
        r = *lpRed++;
        g = *lpGreen++;
        b = *lpBlue++;
        
        *lpS =  ((r >> 3) << 11) |
          ((g >> 2) << 5) |
          ((b >> 3));
        lpS++;
      }
    }
    
    switch(aspectRatio)
    {
    case GR_ASPECT_LOG2_1x1:
      width=256;
      height=256;
      break;
    case GR_ASPECT_LOG2_2x1:
      width=256;
      height=128;
      break;
    case GR_ASPECT_LOG2_4x1:
      width=256;
      height=64;
      break;
    case GR_ASPECT_LOG2_8x1:
      width=256;
      height=32;
      break;
    case GR_ASPECT_LOG2_1x2:
      width=128;
      height=256;
      break;
    case GR_ASPECT_LOG2_1x4:
      width=64;
      height=256;
      break;
    case GR_ASPECT_LOG2_1x8:
      width=32;
      height=256;
      break;
    }
    
    texhnd->tmu = m_tmu;
    texhnd->info.smallLodLog2=lod;
    texhnd->info.largeLodLog2=lod;
    texhnd->info.aspectRatioLog2=aspectRatio;
    texhnd->info.format=GR_TEXFMT_RGB_565;
    texhnd->info.data=mem;
    texhnd->size = grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH,&texhnd->info);
    d->mempos=manager->allocSpaceMem(texhnd->size);
    texhnd->loadAddress = d->mempos->offset+m_tmu->minAddress;
    texhnd->width=width;
    texhnd->height=height;

    grTexDownloadMipMap(texhnd->tmu->tmu_id, texhnd->loadAddress, GR_MIPMAPLEVELMASK_BOTH, &texhnd->info);
	
    delete mem;
	}
  else
  {
    delete texhnd;
    texhnd = NULL;
  }

	d->pData = texhnd;
  FINAL_RELEASE( piLM );
}

void GlideLightmapCache::Unload(HighColorCacheAndManage_Data *d)
{
			manager->freeSpaceMem(d->mempos);
}
