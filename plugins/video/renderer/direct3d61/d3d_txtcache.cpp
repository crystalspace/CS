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

// D3DCache.CPP
// The Direct3D texture/lightmap cache class
// Written by Dan Ogles
// some modifications by Nathaniel
// modified for DX6.1 by Tristan McLure (09/08/1999)
#include <windows.h>
#include <stdlib.h>
#include "ddraw.h"
#include "d3d.h"
#include "d3dcaps.h"

#include "sysdef.h"
#include "cs3d/direct3d61/d3d_txtcache.h"

#include "cs3d/direct3d61/d3d_g3d.h"
#include "itexture.h"
#include "ilghtmap.h"
#include "IGraph3d.h"
#include "csutil/util.h"

const int    LightmapBrightness = 0;   //can be 0 (darkest) to 254 (everything becomes white)
const double LightmapGamma      = 1.0; //can be from 0.01   to 1.0 (1.0= no effect)
  
const int    TextureBrightness = 0;   //can be 0 (darkest) to 254 (everything becomes white)
const double TextureGamma      = 1.0; //can be from 0.01   to 1.0 (1.0= no effect)

///////////////////////////////////
// D3DTextureCache Implementation//
///////////////////////////////////
D3DTextureCache::D3DTextureCache(int nMaxSize, bool bHardware, LPDIRECTDRAW4 pDDraw, 
                                 LPDIRECT3DDEVICE3 pDevice, int nBpp, bool bMipmapping,
				 G3D_CAPS* pRendercaps, int MaxAspectRatio)
: HighColorCache(nMaxSize, HIGHCOLOR_TEXCACHE, nBpp)
{
  ASSERT(MaxAspectRatio>0);
  ASSERT(pRendercaps);
  ASSERT(pDevice);
  ASSERT(pDDraw);

  m_bHardware      = bHardware;
  m_lpDD           = pDDraw;
  m_lpD3dDevice    = pDevice;
  m_bMipMapping    = bMipmapping;
  m_pRendercaps    = pRendercaps;
  m_MaxAspectRatio = MaxAspectRatio;
  
  for (int i=0; i<256; i++)
  {
    m_GammaCorrect[i] = TextureBrightness+(255-TextureBrightness)*pow(i/255.0, TextureGamma);
  }
}

void D3DTextureCache::Dump()
{
/*CsPrintf (MSG_CONSOLE, "Textures in the cache: %d\n", num);
CsPrintf (MSG_CONSOLE, "Total size: %ld bytes\n", total_size);
int mean;
if (num == 0) mean = 0;
else mean = total_size/num;
  CsPrintf (MSG_CONSOLE, "Bytes per texture: %d\n", mean);*/
}

void D3DTextureCache::Load(csHighColorCacheData *d)
{
  D3DTextureCache_Data* cached_texture;
  DDSCAPS2 ddsCaps;
  IDirectDrawSurface4* lpDDLevel;
  IDirectDrawSurface4* lpDDNextLevel;
  
  DDCOLORKEY key = { 0, 0 };
  CHK (cached_texture = new D3DTextureCache_Data);
  
  iTextureHandle* txt_handle = (iTextureHandle*)d->pSource;
  csTextureMM* txt_mm = (csTextureMM*)txt_handle->GetPrivateObject ();
  
  int red_shift   = 0, red_scale   = 0;
  int green_shift = 0, green_scale = 0;
  int blue_shift  = 0, blue_scale  = 0;
  
  ASSERT( m_lpDD );
  
  d->pData = cached_texture;
  
  DDSURFACEDESC2 ddsd;
  
  // get the texture map
  csTexture* txt_unl = txt_mm->get_texture (0);
  
  // create a texture surface in system memory and move it there.
  memset(&ddsd, 0, sizeof(ddsd));
  ddsd.dwSize = sizeof(ddsd);
  ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
  ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;

  memcpy(&ddsd.ddpfPixelFormat, 
         &csGraphics3DDirect3DDx6::m_ddsdTextureSurfDesc.ddpfPixelFormat, 
         sizeof(ddsd.ddpfPixelFormat));

  int OriginalWidth  = txt_unl->get_width ();
  int OriginalHeight = txt_unl->get_height ();

  ASSERT(IsPowerOf2(OriginalWidth));
  ASSERT(IsPowerOf2(OriginalHeight));

  //Some Direct3D cards do not accept all texture formats. For example, 
  //the 3DFX Voodoo cards will not accept textures that are larger than
  //256 pixels. Or the Riva128 card will only accept square textures.
  //To help these cards we will now scale the texture to a format that
  //is accepted by these cards. (Of course that is a slight performace
  //hit, but this happens only when loading a level, so it is still
  //ok.)
  int NewWidth  = OriginalWidth;
  int NewHeight = OriginalHeight;

  while (NewWidth/NewHeight > m_MaxAspectRatio) NewHeight += NewHeight;
  while (NewHeight/NewWidth > m_MaxAspectRatio) NewWidth  += NewWidth;

  if (NewWidth< m_pRendercaps->minTexWidth)  NewWidth  = m_pRendercaps->minTexWidth;
  if (NewHeight<m_pRendercaps->minTexHeight) NewHeight = m_pRendercaps->minTexHeight;
  if (NewWidth> m_pRendercaps->maxTexWidth)  NewWidth  = m_pRendercaps->maxTexWidth;
  if (NewHeight>m_pRendercaps->maxTexHeight) NewHeight = m_pRendercaps->maxTexHeight;

  ddsd.dwHeight = (unsigned long)NewHeight;
  ddsd.dwWidth  = (unsigned long)NewWidth;
  
  if(m_bMipMapping)
  {
    ddsd.dwMipMapCount = 4; 
    ddsd.dwFlags        |= DDSD_MIPMAPCOUNT;
    ddsd.ddsCaps.dwCaps |= DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
  }
  
  VERIFY_SUCCESS(m_lpDD->CreateSurface(&ddsd, &cached_texture->lpsurf, NULL) );

  cached_texture->lpddpal=NULL;
  unsigned long m;
  int s;
  
  // figure out the scale/shift of the rgb components for the surface
  for(s=0, m = csGraphics3DDirect3DDx6::m_ddsdTextureSurfDesc.ddpfPixelFormat.dwRBitMask; !(m & 1); s++, m >>= 1);
  red_shift = s;
  red_scale = 255 / (csGraphics3DDirect3DDx6::m_ddsdTextureSurfDesc.ddpfPixelFormat.dwRBitMask >> s);
  
  for(s=0, m = csGraphics3DDirect3DDx6::m_ddsdTextureSurfDesc.ddpfPixelFormat.dwGBitMask; !(m & 1); s++, m >>= 1);
  green_shift = s;
  green_scale = 255 / (csGraphics3DDirect3DDx6::m_ddsdTextureSurfDesc.ddpfPixelFormat.dwGBitMask >> s);
  
  for(s=0, m = csGraphics3DDirect3DDx6::m_ddsdTextureSurfDesc.ddpfPixelFormat.dwBBitMask; !(m & 1); s++, m >>= 1);
  blue_shift = s;
  blue_scale = 255 / (csGraphics3DDirect3DDx6::m_ddsdTextureSurfDesc.ddpfPixelFormat.dwBBitMask >> s);
  
  lpDDLevel = cached_texture->lpsurf; 

  ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
  
  // grab a reference to this level (we call Release later)
  lpDDLevel->AddRef();
  
  // only process one level if we aren't mipmapping.
  int cLevels = m_bMipMapping ? 4 : 1;
  
  // go through each mip map level and fill it in.
  for (int z = 0; z < cLevels; z++)
  {
    txt_unl              = txt_mm->get_texture (z);
    int OriginalWidth    = txt_unl->get_width ();
    int OriginalHeight   = txt_unl->get_height ();
    unsigned long* lpSrc = (ULong *)txt_unl->get_bitmap();

    //When using Mipmaps, every Mipmap level has half the size of 
    //the previous level. So we now derive all actual sizes from
    //NewWidth/ NewHeight
    int CurrentWidth  = NewWidth  / (1<<z);
    int CurrentHeight = NewHeight / (1<<z);

    // lock the texture surface for writing
    memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(ddsd);
    lpDDLevel->Lock(NULL, &ddsd, 0, NULL);
    
    // fill in the surface, depending on the bpp and texture format
    switch(bpp)
    {
      case 32:
      {
        if ((CurrentWidth == OriginalWidth) && (CurrentHeight == OriginalHeight))
	{
	  //No zooming needed, so we can use a rather fast method to add to the cache
	  for (int j = 0; j < CurrentHeight; j++)
	  {
	    unsigned long* lpL = (unsigned long *)(((char *)ddsd.lpSurface) + ddsd.lPitch * j);
	    unsigned long* llpySrc = lpSrc + (j * CurrentWidth);
        
	    for (int i=0; i < CurrentWidth; i++)
	    {
	      int r = m_GammaCorrect[R24(*llpySrc)];
	      int g = m_GammaCorrect[G24(*llpySrc)];
	      int b = m_GammaCorrect[B24(*llpySrc)];

	      *lpL =  ((r / red_scale)   << red_shift)   |
		      ((g / green_scale) << green_shift) |
		      ((b / blue_scale)  << blue_shift);
          
	      lpL++;
	      llpySrc++;
	    }
	  }
	}
	else
	{
	  //Original size and Current size do differ, that makes things a bit
	  //more difficult
	  for (int j = 0; j < CurrentHeight; j++)
	  {
	    unsigned long* lpL = (unsigned long *)(((char *)ddsd.lpSurface) + ddsd.lPitch * j);
	    
	    //select the proper row.
	    unsigned long* llpySrc = lpSrc + ((j*OriginalHeight/CurrentHeight) * OriginalWidth);
        
	    for (int i=0; i < CurrentWidth; i++)
	    {
	      //select the proper texel. (we could do this much nicer by using filtering, but
	      //this would even cause a greater slowdown, and would be more work to code)
	      unsigned long * llpxSrc = llpySrc + (i*OriginalWidth/CurrentWidth);

	      int r = m_GammaCorrect[R24(*llpxSrc)];
	      int g = m_GammaCorrect[G24(*llpxSrc)];
	      int b = m_GammaCorrect[B24(*llpxSrc)];

	      *lpL =  ((r / red_scale)   << red_shift)   |
		      ((g / green_scale) << green_shift) |
		      ((b / blue_scale)  << blue_shift);
          
	      lpL++;
	    }
	  }
	}
        break;
      }  
      case 16:
      {
        if ((CurrentWidth == OriginalWidth) && (CurrentHeight == OriginalHeight))
	{
	  //No zooming needed, so we can use a rather fast method to add to the cache
	  for (int j = 0; j < CurrentHeight; j++)
	  {
	    unsigned short* lpS = (unsigned short *)(((char *)ddsd.lpSurface) + ddsd.lPitch * j);
	    unsigned long*  llpySrc = lpSrc + (j * CurrentWidth);
        
	    for(int i=0; i< CurrentWidth; i++)
	    {
	      int r = m_GammaCorrect[R24(*llpySrc)];
	      int g = m_GammaCorrect[G24(*llpySrc)];
	      int b = m_GammaCorrect[B24(*llpySrc)];
          
	      *lpS =  ((r / red_scale)   << red_shift)   |
		      ((g / green_scale) << green_shift) |
		      ((b / blue_scale)  << blue_shift);
          
	      llpySrc++;
	      lpS++;
	    }
	  }
	}
	else
	{
	  //Original size and Current size do differ, that makes things a bit
	  //more difficult
	  for (int j = 0; j < CurrentHeight; j++)
	  {
	    unsigned short* lpS = (unsigned short *)(((char *)ddsd.lpSurface) + ddsd.lPitch * j);
	    
	    //select the proper row.
	    unsigned long * llpySrc = lpSrc + ((j*OriginalHeight/CurrentHeight) * OriginalWidth);
        
	    for (int i=0; i < CurrentWidth; i++)
	    {
	      //select the proper texel. (we could do this much nicer by using filtering, but
	      //this would even cause a greater slowdown, and would be more work to code)
	      unsigned long * llpxSrc = llpySrc + (i*OriginalWidth/CurrentWidth);

	      int r = m_GammaCorrect[R24(*llpxSrc)];
	      int g = m_GammaCorrect[G24(*llpxSrc)];
	      int b = m_GammaCorrect[B24(*llpxSrc)];

	      *lpS =  ((r / red_scale)   << red_shift)   |
		      ((g / green_scale) << green_shift) |
		      ((b / blue_scale)  << blue_shift);
          
	      lpS++;
	    }
	  }
	}
        break;
      }
      default:
      {
        ASSERT( FALSE );
        break;
      }
    } //switch(bpp)
      
    lpDDLevel->Unlock(NULL);
    
    lpDDLevel->SetColorKey(DDCKEY_SRCBLT, &key);
    
    lpDDLevel->GetAttachedSurface(&ddsCaps, &lpDDNextLevel);
    
    // Release our reference to this level.
    lpDDLevel->Release();
    
    lpDDLevel = lpDDNextLevel; 
  }
   
  //assume, that all Mipmap levels have been set.
  ASSERT(lpDDLevel==NULL); 
  
  // get the texture interfaces and handles
  cached_texture->lpsurf->QueryInterface(IID_IDirect3DTexture2, (LPVOID *)&cached_texture->lptex);
  
  if(m_bHardware)
  {
    LoadIntoVRAM(cached_texture);    
  }
}

void D3DTextureCache::LoadIntoVRAM(D3DTextureCache_Data *tex)
{
  DDSURFACEDESC2 ddsd;
  IDirectDrawSurface4* lpddts;
  IDirect3DTexture2*  ddtex;
  
  memset(&ddsd, 0, sizeof(ddsd));
  ddsd.dwSize = sizeof(ddsd);
  tex->lpsurf->GetSurfaceDesc(&ddsd);
  ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;

  ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_ALLOCONLOAD;
  if(m_bMipMapping)
  {
    ddsd.dwMipMapCount = 4; 
    ddsd.dwFlags        |= DDSD_MIPMAPCOUNT;
    ddsd.ddsCaps.dwCaps |= DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
  }

  VERIFY_SUCCESS( m_lpDD->CreateSurface(&ddsd, &lpddts, NULL) );
  
  VERIFY_SUCCESS( lpddts->QueryInterface(IID_IDirect3DTexture2, (LPVOID *)&ddtex) );
  VERIFY_SUCCESS( ddtex->Load(tex->lptex) );

  tex->lptex->Release();
  tex->lpsurf->Release();
  
  tex->lptex = ddtex;
  tex->lpsurf = lpddts;
}

void D3DTextureCache::Unload(csHighColorCacheData *d)
{
  D3DTextureCache_Data *t = (D3DTextureCache_Data *)d->pData;
  if (!t) return;
  
  if (t->lpsurf)
  {
    t->lpsurf->Release();
    t->lpsurf = NULL;
  }
  if (t->lptex)
  {
    t->lptex->Release();
    t->lptex = NULL;
  }
  if (t->lpddpal)
  {
    t->lpddpal->Release();
    t->lpddpal=NULL;
  }
  
  d->pData = NULL;
}

///////////////////////////////////
// D3DLightMapCache Implementation//
///////////////////////////////////
D3DLightMapCache::D3DLightMapCache(int nMaxSize, bool bHardware, LPDIRECTDRAW4 pDDraw, LPDIRECT3DDEVICE3 pDevice, int nBpp)
: HighColorCache(nMaxSize, HIGHCOLOR_LITCACHE, nBpp)
{
  m_bHardware=bHardware;
  m_lpDD = pDDraw;
  m_lpD3dDevice= pDevice;

  for (int i=0; i<256; i++)
  {
    m_GammaCorrect[i] = LightmapBrightness+(255-LightmapBrightness)*pow(i/255.0, LightmapGamma);
  }
}

void D3DLightMapCache::Dump()
{
/*CsPrintf (MSG_CONSOLE, "Lightmaps in the cache: %d\n", num);
CsPrintf (MSG_CONSOLE, "Total size: %ld bytes\n", total_size);
int mean;
if (num == 0) mean = 0;
else mean = total_size/num;
  CsPrintf (MSG_CONSOLE, "Bytes per lightmap: %d\n", mean);*/
}

void D3DLightMapCache::Load(csHighColorCacheData *d)
{
  CHK (D3DLightCache_Data *cached_texture = new D3DLightCache_Data);
  d->pData = cached_texture;
  
  iLightMap *piLM = QUERY_INTERFACE(d->pSource, iLightMap);
  ASSERT( piLM );
  
  int lwidth = piLM->GetWidth();
  int lheight = piLM->GetHeight();
  
  int rheight = piLM->GetRealHeight();
  int rwidth = piLM->GetRealWidth();
  
  // create the lightmap surface. this is a hi/true color surface.
  DDSURFACEDESC2 ddsd;
  memset(&ddsd, 0, sizeof(ddsd));
  ddsd.dwSize = sizeof(ddsd);
  ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
  ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;
  ddsd.dwHeight = lheight;
  ddsd.dwWidth = lwidth;
  memcpy(&ddsd.ddpfPixelFormat, &csGraphics3DDirect3DDx6::m_ddsdLightmapSurfDesc.ddpfPixelFormat, sizeof(DDPIXELFORMAT));
  
  ASSERT(!(lheight%2));
  ASSERT(!(lwidth%2));
  
  VERIFY_SUCCESS( m_lpDD->CreateSurface(&ddsd, &cached_texture->lpsurf, NULL) );
  
  unsigned long m;
  int s;
  
  int red_shift   = 0, red_scale   = 0;
  int green_shift = 0, green_scale = 0;
  int blue_shift  = 0, blue_scale  = 0;
  
  // figure out the scale/shift of the rgb components for the surface
  // (i took this from a d3d example by MS)
  for(s=0, m = csGraphics3DDirect3DDx6::m_ddsdLightmapSurfDesc.ddpfPixelFormat.dwRBitMask; !(m & 1); s++, m >>= 1);
  red_shift = s;
  red_scale = 255 / (csGraphics3DDirect3DDx6::m_ddsdLightmapSurfDesc.ddpfPixelFormat.dwRBitMask >> s);
  
  for(s=0, m = csGraphics3DDirect3DDx6::m_ddsdLightmapSurfDesc.ddpfPixelFormat.dwGBitMask; !(m & 1); s++, m >>= 1);
  green_shift = s;
  green_scale = 255 / (csGraphics3DDirect3DDx6::m_ddsdLightmapSurfDesc.ddpfPixelFormat.dwGBitMask >> s);
  
  for(s=0, m = csGraphics3DDirect3DDx6::m_ddsdLightmapSurfDesc.ddpfPixelFormat.dwBBitMask; !(m & 1); s++, m >>= 1);
  blue_shift = s;
  blue_scale = 255 / (csGraphics3DDirect3DDx6::m_ddsdLightmapSurfDesc.ddpfPixelFormat.dwBBitMask >> s);
  
  // lock the lightmap surface
  memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
  ddsd.dwSize = sizeof(ddsd);
  VERIFY_SUCCESS( cached_texture->lpsurf->Lock(NULL, &ddsd, 0, NULL) );
  
  // get the red, green, and blue lightmaps.
  unsigned char *lpRed; 
  unsigned char *lpGreen;
  unsigned char *lpBlue; 
  
  lpRed   = piLM->GetMap(0);
  lpGreen = piLM->GetMap(1);
  lpBlue  = piLM->GetMap(2);
  
  unsigned long *lpL;
  unsigned short *lpS;
  unsigned char *lpC;
  
  int i,j;
  
  switch(bpp)
  {
  case 32:
    for (j=0; j<lheight; j++)
    {
      lpL = (unsigned long *)(((char *)ddsd.lpSurface) + ddsd.lPitch * j);
      
      for(i=0; i<lwidth; i++)
      {
        int r,g,b;
        
        r = m_GammaCorrect[*(lpRed + (i + (lwidth * j)))];
        g = m_GammaCorrect[*(lpGreen + (i + (lwidth * j)))];
        b = m_GammaCorrect[*(lpBlue + (i + (lwidth * j)))];
        
        *lpL =  ((r / red_scale) << red_shift) |
          ((g / green_scale) << green_shift) |
          ((b / blue_scale) << blue_shift);
        lpL++;
      }
    }
    break;
    
  case 16:
    for (j=0; j<lheight; j++)
    {
      lpS = (unsigned short *)(((char *)ddsd.lpSurface) + ddsd.lPitch * j);
      
      for(i=0; i<lwidth; i++)
      {
        int r,g,b;
        
        r = m_GammaCorrect[*(lpRed + (i + (lwidth * j)))];
        g = m_GammaCorrect[*(lpGreen + (i + (lwidth * j)))];
        b = m_GammaCorrect[*(lpBlue + (i + (lwidth * j)))];
        
        *lpS =  ((r / red_scale) << red_shift) |
          ((g / green_scale) << green_shift) |
          ((b / blue_scale) << blue_shift);
        lpS++;
      }
    }
    break;
    
  case 8:
    // if the m_bHardware only supports 8 bit maps, 
    // then we have to use gray-scale lightmaps.
    for(j=0; j<lheight; j++)
    {
      lpC = (unsigned char *)(((char *)ddsd.lpSurface) + ddsd.lPitch * j);
      
      for(i=0; i<lwidth; i++)
      {
        int r,g,b,mean;
        
        r = *(lpRed + (i + (lwidth * j)));
        g = *(lpGreen + (i + (lwidth * j)));
        b = *(lpBlue + (i + (lwidth * j)));
        
        mean = m_GammaCorrect[(r+g+b)/3];
        
        *lpC = mean;
        lpC++;
      }
    }
    break;
    
  default:
    ASSERT(FALSE);
    break;
  }
  
  // unlock the lightmap
  cached_texture->lpsurf->Unlock(NULL);
  
  cached_texture->ratio_width = (rwidth*0.95)/(lwidth*1.1);
  cached_texture->ratio_height = (rheight*0.95)/(lheight*1.1);
  
  // get the texture interfaces and handles
  VERIFY_SUCCESS( cached_texture->lpsurf->QueryInterface(IID_IDirect3DTexture2, (LPVOID *)&cached_texture->lptex) );
  
  if (m_bHardware)
  {
    LoadIntoVRAM(cached_texture);
  }
  
  piLM->DecRef ();
}       

void D3DLightMapCache::LoadIntoVRAM(D3DLightCache_Data *tex)
{
  DDSURFACEDESC2 ddsd;
  IDirectDrawSurface4* lpddts;
  IDirect3DTexture2*  ddtex;
  
  memset(&ddsd, 0, sizeof(ddsd));
  ddsd.dwSize = sizeof(ddsd);
  tex->lpsurf->GetSurfaceDesc(&ddsd);
  ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
  ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_ALLOCONLOAD;
  
  VERIFY_SUCCESS( m_lpDD->CreateSurface(&ddsd, &lpddts, NULL) );
  
  lpddts->QueryInterface(IID_IDirect3DTexture2, (LPVOID *)&ddtex);     
  
  VERIFY_SUCCESS( ddtex->Load(tex->lptex) );
  
  tex->lptex->Release();
  tex->lpsurf->Release();
  
  tex->lptex = ddtex;
  tex->lpsurf = lpddts;
}

void D3DLightMapCache::Unload(csHighColorCacheData *d)
{
  D3DTextureCache_Data *t = (D3DTextureCache_Data *)d->pData;
  if (!d->pData)
    return;
  
  d->pData = NULL;
  
  if( t->lpsurf)
  {
    t->lpsurf->Release();
    t->lpsurf = NULL;
  }
  if (t->lptex)
  {
    t->lptex->Release();
    t->lptex = NULL;
  }
}


