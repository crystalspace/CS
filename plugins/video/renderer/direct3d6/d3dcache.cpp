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

#include <windows.h>
#include <stdlib.h>
#include "ddraw.h"
#include "d3d.h"
#include "d3dcaps.h"

#include "sysdef.h"
#include "cs3d/direct3d6/d3dcache.h"

#include "cs3d/direct3d6/g3d_d3d.h"
#include "itexture.h"
#include "ilghtmap.h"
#include "IGraph3d.h"

///////////////////////////////////
// D3DTextureCache Implementation//
///////////////////////////////////
D3DTextureCache::D3DTextureCache(int nMaxSize, bool bHardware, LPDIRECTDRAW4 pDDraw, LPDIRECT3DDEVICE3 pDevice, int nBpp, bool bMipmapping)
: HighColorCache(nMaxSize, HIGHCOLOR_TEXCACHE, nBpp)
{
  m_bHardware=bHardware;
  m_lpDD = pDDraw;
  m_lpD3dDevice = pDevice;
  m_bMipMapping = bMipmapping;
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
  IMipMapContainer *piMMC = NULL;
  ITextureMap *piTM = NULL;
  D3DTextureCache_Data* cached_texture;
  DDSCAPS2 ddsCaps;
  IDirectDrawSurface4* lpDDLevel;
  IDirectDrawSurface4* lpDDNextLevel;
  
  DDCOLORKEY key = { 0, 0 };
  RGBpaletteEntry* Palette;
  CHK (cached_texture = new D3DTextureCache_Data);

  d->pSource->QueryInterface( IID_IMipMapContainer, (void**)&piMMC );
  assert( piMMC != NULL );

  piMMC->GetPrivateColorMap((unsigned char**)&Palette);
  
  unsigned char *lpC;
  unsigned short *lpS;
  unsigned long *lpL;
  unsigned char *lpSrc, *lpySrc;
  
  int red_shift, red_scale;
  int green_shift, green_scale;
  int blue_shift, blue_scale;
  
  ASSERT( m_lpDD );
  
  d->pData = cached_texture;
  
  DDSURFACEDESC2 ddsd;
  int nWidth, nHeight;

  // get the texture map
  piMMC->GetTexture(0, &piTM);
  ASSERT( piTM );

  // create a texture surface in system memory and move it there.
  ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
  ddsd.dwSize = sizeof(DDSURFACEDESC2);
  ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
  ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;
  ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
  memcpy(&ddsd.ddpfPixelFormat, &csGraphics3DDirect3DDx6::m_ddpfTexturePixelFormat, sizeof(DDPIXELFORMAT));
  
  nHeight = piTM->GetHeight(); 
  ddsd.dwHeight = (unsigned long)nHeight;
  
  nWidth = piTM->GetWidth(); 
  ddsd.dwWidth = (unsigned long)nWidth;
  
  if(m_bMipMapping)
  {
    ddsd.dwFlags |= DDSD_MIPMAPCOUNT;
    ddsd.dwMipMapCount = 4; 
    ddsd.ddsCaps.dwCaps |= DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
  }
  
  VERIFY_SUCCESS( m_lpDD->CreateSurface(&ddsd, &cached_texture->lpsurf, NULL) );
  
  if(bpp==8)
  {
    ASSERT(Palette);
    // attach the palette to the texture.
    VERIFY_SUCCESS( m_lpDD->CreatePalette(DDPCAPS_INITIALIZE | DDPCAPS_8BIT | DDPCAPS_ALLOW256, (PALETTEENTRY*)Palette, &cached_texture->lpddpal, NULL) );
    VERIFY_SUCCESS( cached_texture->lpsurf->SetPalette(cached_texture->lpddpal) );
  } 
  else
  {
    cached_texture->lpddpal=NULL;
    unsigned long m;
    int s;
    
    // figure out the scale/shift of the rgb components for the surface
    for(s=0, m = csGraphics3DDirect3DDx6::m_ddpfTexturePixelFormat.dwRBitMask; !(m & 1); s++, m >>= 1);
    red_shift = s;
    red_scale = 255 / (csGraphics3DDirect3DDx6::m_ddpfTexturePixelFormat.dwRBitMask >> s);
    
    for(s=0, m = csGraphics3DDirect3DDx6::m_ddpfTexturePixelFormat.dwGBitMask; !(m & 1); s++, m >>= 1);
    green_shift = s;
    green_scale = 255 / (csGraphics3DDirect3DDx6::m_ddpfTexturePixelFormat.dwGBitMask >> s);
    
    for(s=0, m = csGraphics3DDirect3DDx6::m_ddpfTexturePixelFormat.dwBBitMask; !(m & 1); s++, m >>= 1);
    blue_shift = s;
    blue_scale = 255 / (csGraphics3DDirect3DDx6::m_ddpfTexturePixelFormat.dwBBitMask >> s);
  }
  
  lpDDLevel = cached_texture->lpsurf; 
  ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;

  // grab a reference to this level (we call Release later)
  lpDDLevel->IncRef();
      
  // only process one level if we aren't mipmapping.
  int cLevels = m_bMipMapping ? 4 : 1;
  
  // go through each mip map level and fill it in.
  for (int z = 0; z < cLevels; z++)
  {
      int width, height;
      
      piMMC->GetTexture(z, &piTM);
      ASSERT( piTM != NULL );
      
      width = piTM->GetWidth();
      height = piTM->GetHeight();
      lpSrc = piTM->GetBitmap();
      ASSERT( lpSrc != NULL );
      
      // lock the texture surface for writing
      memset(&ddsd, 0, sizeof(DDSURFACEDESC));
      ddsd.dwSize = sizeof(ddsd);
      lpDDLevel->Lock(NULL, &ddsd, 0, NULL);
      
      int i, j;
      
      // fill in the surface, depending on the bpp
      switch(bpp)
      {
      case 32:
          for (j = 0; j < height; j++)
          {
              lpL = (unsigned long *)(((char *)ddsd.lpSurface) + ddsd.lPitch * j);
              lpySrc = lpSrc + (j * width);
              
              for(i=0; i< width; i++)
              {
                  int r,g,b;
                  
                  r = Palette[*lpySrc].red;
                  g = Palette[*lpySrc].green;
                  b = Palette[*lpySrc].blue;
                  
                  *lpL =  ((r / red_scale) << red_shift) |
                      ((g / green_scale) << green_shift) |
                      ((b / blue_scale) << blue_shift);
                  
                  lpySrc++;
                  lpL++;
              }
          }
          break;
          
      case 16:
          for (j = 0; j < height; j++)
          {
              lpS = (unsigned short *)(((char *)ddsd.lpSurface) + ddsd.lPitch * j);
              lpySrc = lpSrc + (j * width);
              
              for(i=0; i< width; i++)
              {
                  int r,g,b;
                  
                  r = Palette[*lpySrc].red;
                  g = Palette[*lpySrc].green;
                  b = Palette[*lpySrc].blue;
                  
                  *lpS =  ((r / red_scale) << red_shift) |
                      ((g / green_scale) << green_shift) |
                      ((b / blue_scale) << blue_shift);
                  
                  lpySrc++;
                  lpS++;
              }
          }
          break;
          
      case 8:
          for (j = 0; j < height; j++)
          {
              lpC = (unsigned char *)(((char *)ddsd.lpSurface) + ddsd.lPitch * j);
              lpySrc = lpSrc + (j * width);
              
              for(i=0; i< width; i++)
              {
                  *lpC = *lpySrc;
                  
                  lpySrc++;
                  lpC++;
              }
          }
          break;
          
      default:
          ASSERT( FALSE );
          break;
      }
      
      lpDDLevel->Unlock(NULL);
      
      lpDDLevel->SetColorKey(DDCKEY_SRCBLT, &key);
      
      lpDDLevel->GetAttachedSurface(&ddsCaps, &lpDDNextLevel);
      
      // Release our reference to this level.
      lpDDLevel->Release();
      
      lpDDLevel = lpDDNextLevel; 
  }
  
  // get the texture interfaces and handles
  cached_texture->lpsurf->QueryInterface(IID_IDirect3DTexture2, (LPVOID *)&cached_texture->lptex);
  
  if(m_bHardware)
  {
    LoadIntoVRAM(cached_texture);    
  }

  piMMC->DecRef ();
}

void D3DTextureCache::LoadIntoVRAM(D3DTextureCache_Data *tex)
{
  DDSURFACEDESC2 ddsd;
  IDirectDrawSurface4* lpddts;
  IDirect3DTexture2*  ddtex;
    
  memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
  ddsd.dwSize = sizeof(DDSURFACEDESC2);
  tex->lpsurf->GetSurfaceDesc(&ddsd);
  ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
  ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_ALLOCONLOAD;
  if(m_bMipMapping)
    ddsd.ddsCaps.dwCaps |= DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
  
  VERIFY_SUCCESS( m_lpDD->CreateSurface(&ddsd, &lpddts, NULL) );
  
  if(bpp==8) 
  {
    lpddts->SetPalette(tex->lpddpal);
    
    tex->lpddpal->Release();
    tex->lpddpal=NULL;
  }
  
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
  iLightMap *piLM;
  CHK (D3DLightCache_Data *cached_texture = new D3DLightCache_Data);
  d->pData = cached_texture;
  
  VERIFY_SUCCESS( d->pSource->QueryInterface( IID_iLightMap, (void**)&piLM ) );
  ASSERT( piLM );

  int lwidth = piLM->GetWidth();
  int lheight = piLM->GetHeight();
  
  // create the lightmap surface. this is a hi/true color surface.
  DDSURFACEDESC2 ddsd;
  memset(&ddsd, 0, sizeof(DDSURFACEDESC));
  ddsd.dwSize = sizeof(ddsd);
  ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
  ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;
  memcpy(&ddsd.ddpfPixelFormat, &csGraphics3DDirect3DDx6::m_ddpfLightmapPixelFormat, sizeof(DDPIXELFORMAT));
  ddsd.dwHeight = lheight;
  ddsd.dwWidth = lwidth;
  
  ASSERT(!(lheight%2));
  ASSERT(!(lwidth%2));
  
  VERIFY_SUCCESS( m_lpDD->CreateSurface(&ddsd, &cached_texture->lpsurf, NULL) );
  
  unsigned long m;
  int s;
  
  int red_shift, red_scale;
  int green_shift, green_scale;
  int blue_shift, blue_scale;
  
  if (bpp!=8)
  {
    // figure out the scale/shift of the rgb components for the surface
    // (i took this from a d3d example by MS)
    for(s=0, m = csGraphics3DDirect3DDx6::m_ddpfLightmapPixelFormat.dwRBitMask; !(m & 1); s++, m >>= 1);
    red_shift = s;
    red_scale = 255 / (csGraphics3DDirect3DDx6::m_ddpfLightmapPixelFormat.dwRBitMask >> s);
    
    for(s=0, m = csGraphics3DDirect3DDx6::m_ddpfLightmapPixelFormat.dwGBitMask; !(m & 1); s++, m >>= 1);
    green_shift = s;
    green_scale = 255 / (csGraphics3DDirect3DDx6::m_ddpfLightmapPixelFormat.dwGBitMask >> s);
    
    for(s=0, m = csGraphics3DDirect3DDx6::m_ddpfLightmapPixelFormat.dwBBitMask; !(m & 1); s++, m >>= 1);
    blue_shift = s;
    blue_scale = 255 / (csGraphics3DDirect3DDx6::m_ddpfLightmapPixelFormat.dwBBitMask >> s);
  }
  
  // lock the lightmap surface
  memset(&ddsd, 0, sizeof(DDSURFACEDESC));
  ddsd.dwSize = sizeof(ddsd);
  VERIFY_SUCCESS( cached_texture->lpsurf->Lock(NULL, &ddsd, 0, NULL) );
  
  // get the red, green, and blue lightmaps.
  unsigned char *lpRed; 
  unsigned char *lpGreen;
  unsigned char *lpBlue; 

  piLM->GetMap(0, &lpRed);
  piLM->GetMap(1, &lpGreen);
  piLM->GetMap(2, &lpBlue);
  
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
        
        r = *(lpRed + (i + (lwidth * j)));
        g = *(lpGreen + (i + (lwidth * j)));
        b = *(lpBlue + (i + (lwidth * j)));

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

        r = *(lpRed + (i + (lwidth * j)));
        g = *(lpGreen + (i + (lwidth * j)));
        b = *(lpBlue + (i + (lwidth * j)));

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
        
        mean = (r+g+b)/3;
        
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
  
  if(bpp==8) 
  {
    PALETTEENTRY pe[256];
    LPDIRECTDRAWPALETTE lpddpal;
    
    for(int i=0; i<256; i++)
    {
      pe[i].peRed = i;
      pe[i].peBlue = i;
      pe[i].peGreen = i;
      pe[i].peFlags = PC_NOCOLLAPSE;
    }
    
    // attach the palette to the texture.
    VERIFY_SUCCESS( m_lpDD->CreatePalette(DDPCAPS_INITIALIZE | DDPCAPS_8BIT | DDPCAPS_ALLOW256, pe, &lpddpal, NULL) );
    
    lpddts->SetPalette(lpddpal);
    lpddpal->Release();
  }
  
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


