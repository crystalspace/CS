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

#include "cssysdef.h"
#include <windows.h>
#include <stdlib.h>
#include "ddraw.h"
#include "d3d.h"
#include "d3dcaps.h"

#include "d3d_g3d.h"
#include "ivideo/itexture.h"
#include "iengine/itexture.h"
#include "iengine/ilghtmap.h"
#include "IGraph3d.h"
#include "csutil/util.h"

//----------------------------------------------------------------------------//

D3DCache::D3DCache (int max_size, csCacheType type, int bpp)
{
  D3DCache::type = type;
  D3DCache::bpp = bpp;
  cache_size = max_size;
  num = 0;
  total_size = 0;
  head = tail = NULL;
}

void D3DCache::cache_texture (iTextureHandle *texture)
{
  if (type != CS_TEXTURE)
    return;
    
  // commented out by Frank O'Connor (frank@oconnors.org)
  // to allow compile. piTM isn't referenced anyhow.
  //iTextureMap *piTM = NULL;
  int size = 0;
    
  for (int c =0; c < 4; c++)
  {
    int width, height;
        
    if (texture->GetMipMapDimensions (c, width, height))
      size += width * height;
  }
    
  size *= bpp/8;
    
  csTextureHandleDirect3D *txt_mm = (csTextureHandleDirect3D *)texture->GetPrivateObject ();
  csD3DCacheData *cached_texture = (csD3DCacheData *)txt_mm->GetCacheData ();
  if (cached_texture)
  {
    // move unit to front (MRU)
    if (cached_texture != head)
    {
      if (cached_texture->prev)
        cached_texture->prev->next = cached_texture->next;
      else
        head = cached_texture->next;
      if (cached_texture->next)
        cached_texture->next->prev = cached_texture->prev;
      else
        tail = cached_texture->prev;

      cached_texture->prev = NULL;
      cached_texture->next = head;
      if (head)
        head->prev = cached_texture;
      else
        tail = cached_texture;
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
      ASSERT( texh );
            
      tail = tail->prev;
      if (tail)
        tail->next = NULL;
      else
        head = NULL;
      cached_texture->prev = NULL;
            
      Unload(cached_texture);			// unload it.
                        
      num--;
      total_size -= cached_texture->lSize;
            
      delete cached_texture;
    }
        
    // now load the unit.
    num++;
    total_size += size;
        
    cached_texture = new csD3DCacheData;
    memset(cached_texture, 0, sizeof(cached_texture));
        
    cached_texture->next = head;
    cached_texture->prev = NULL;
    if (head) 
    {
      head->prev = cached_texture;
    }
    else 
    {
      tail = cached_texture;
    }
    head = cached_texture;
    cached_texture->pSource = texture;
    cached_texture->lSize   = size;
        
    txt_mm->SetCacheData (cached_texture);

    Load (cached_texture);				// load it.
  }
}

void D3DCache::cache_lightmap (iPolygonTexture *polytex)
{
  iLightMap *piLM = polytex->GetLightMap();
  if (!piLM)
    return;
    
  if (type != CS_LIGHTMAP)
    return;
    
  int width = piLM->GetWidth();
  int height = piLM->GetHeight();
  int size = width*height*(bpp/8);
    
  csD3DCacheData *cached_texture = (csD3DCacheData *)piLM->GetCacheData ();
  if (polytex->RecalculateDynamicLights () && cached_texture)
  {
    if (cached_texture->prev)
      cached_texture->prev->next = cached_texture->next;

    if (cached_texture->next) 
      cached_texture->next->prev = cached_texture->prev;
        
    // unload it.
    Unload (cached_texture);

    piLM->SetCacheData(NULL);
      
    num--;
    total_size -= cached_texture->lSize;
    delete cached_texture;
    cached_texture = NULL;
  }

  if (cached_texture)
  {
    // move unit to front (MRU)
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
      // out of memory. remove units from bottom of list.
      cached_texture = tail;
      iLightMap *ilm = (iLightMap *)cached_texture->pSource;
      tail = tail->prev;
      if (tail)
        tail->next = NULL;
      else
        head = NULL;
      cached_texture->prev = NULL;
      ilm->SetCacheData (NULL);
            
      Unload (cached_texture);			// unload it.

      num--;
      total_size -= cached_texture->lSize;

      delete cached_texture;
    }
        
    // now load the unit.
    num++;
    total_size += size;
        
    cached_texture = new csD3DCacheData;
    memset(cached_texture, 0, sizeof(cached_texture));

    cached_texture->next = head;
    cached_texture->prev = NULL;
        
    if (head) 
    {
      head->prev = cached_texture;
    }
    else 
    {
      tail = cached_texture;
    }
    head = cached_texture;

    cached_texture->pSource = piLM;
    cached_texture->lSize = size;
        
    piLM->SetCacheData(cached_texture);
        
    Load(cached_texture);				// load it.
  }
}

void D3DCache::Clear ()
{
  while (head)
  {
    csD3DCacheData *next = head->next;
    head->next = head->prev = NULL;

    Unload (head);

    if (type == CS_TEXTURE) 
    {
      iTextureHandle *texh = (iTextureHandle *)head->pSource;
      texh->SetCacheData (NULL);
      texh->DecRef ();
    }
    else if (type == CS_LIGHTMAP)
    {
      iLightMap *lm = (iLightMap *)head->pSource;
      lm->SetCacheData (NULL);
      lm->DecRef ();
    }

    delete head;
    head = next;
  }
    
  head = tail = NULL;
  total_size = 0;
  num = 0;
}

//--------------------------------------- D3DTextureCache Implementation -----//

D3DTextureCache::D3DTextureCache (int nMaxSize, bool bHardware,
  LPDIRECTDRAW4 pDDraw, LPDIRECT3DDEVICE3 pDevice, int nBpp, bool bMipmapping,
  csGraphics3DCaps *pRendercaps)
  : D3DCache (nMaxSize, CS_TEXTURE, nBpp)
{
  ASSERT(pRendercaps);
  ASSERT(pDevice);
  ASSERT(pDDraw);

  m_bHardware      = bHardware;
  m_lpDD           = pDDraw;
  m_lpD3dDevice    = pDevice;
  m_bMipMapping    = bMipmapping;
  m_pRendercaps    = pRendercaps;
}

void D3DTextureCache::Dump ()
{
}

void D3DTextureCache::Load (csD3DCacheData* cached_texture)
{
  iTextureHandle* txt_handle = (iTextureHandle *)cached_texture->pSource;
  csTextureHandle*    txt_mm     = (csTextureHandle *)   txt_handle->GetPrivateObject ();

  txt_handle->IncRef();

  bool transp = txt_handle->GetKeyColor ();
  //DDCOLORKEY key;
  if (transp)
  {
    //UByte r, g, b;
    //txt_handle->GetKeyColor (r, g, b);
    //key.dwColorSpaceLowValue = key.dwColorSpaceHighValue = D3DRGB (r, g, b);
  }

  ASSERT (m_lpDD);

  DDSCAPS2 ddsCaps;

  // get the texture
  csTextureDirect3D* txt_unl = (csTextureDirect3D*) txt_mm->get_texture (0);
 
  // create a texture surface in system memory and move it there.
  DDSURFACEDESC2 ddsd;
  memset (&ddsd, 0, sizeof (ddsd));
  ddsd.dwSize = sizeof (ddsd);
  ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
  ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;
  memcpy (&ddsd.ddpfPixelFormat,
    &csGraphics3DDirect3DDx6::m_ddsdTextureSurfDesc.ddpfPixelFormat, 
    sizeof (ddsd.ddpfPixelFormat));
  ddsd.dwHeight = txt_unl->get_width ();
  ddsd.dwWidth  = txt_unl->get_height ();

  // only process one level if we aren't mipmapping.
  int cLevels = (m_bMipMapping && txt_mm->get_texture (1)) ? 4 : 1;
  
  if (cLevels > 1)
  {
    ddsd.dwMipMapCount = 4;
    ddsd.dwFlags        |= DDSD_MIPMAPCOUNT;
    ddsd.ddsCaps.dwCaps |= DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
  }
  
  VERIFY_SUCCESS (m_lpDD->CreateSurface (&ddsd,
    &cached_texture->Texture.lpsurf, NULL));
  cached_texture->Texture.lpddpal = NULL;

  ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;

  // grab a reference to this level (we call Release later)
  IDirectDrawSurface4 *lpDDLevel = cached_texture->Texture.lpsurf; 
  lpDDLevel->AddRef ();
  
  // go through each mip map level and fill it in.
  for (int level = 0; level < cLevels; level++)
  {
    txt_unl  = (csTextureDirect3D*) txt_mm->get_texture (level);
    int size = txt_unl->get_size ();

    // lock the texture surface for writing
    memset (&ddsd, 0, sizeof (DDSURFACEDESC2));
    ddsd.dwSize = sizeof (ddsd);
    lpDDLevel->Lock (NULL, &ddsd, 0, NULL);

//
// OPTIMIZATION CONSIDERATIONS:
// Currently we are creating a texture in system memory, then memcpy()
// our data there. Maybe there is a way to create a texture in VRAM directly
// by providing a pointer to the texture data in the proper format? -- A.Z.
//

    memcpy (ddsd.lpSurface, txt_unl->get_image_data (), size * bpp / 8);

    lpDDLevel->Unlock (NULL);

    //if (transp)
    //  lpDDLevel->SetColorKey (DDCKEY_SRCBLT, &key);

    IDirectDrawSurface4 *lpDDNextLevel;
    lpDDLevel->GetAttachedSurface (&ddsCaps, &lpDDNextLevel);

    // Release our reference to this level.
    lpDDLevel->Release ();

    lpDDLevel = lpDDNextLevel; 
  }
   
  //assume, that all Mipmap levels have been set.
  ASSERT (lpDDLevel==NULL); 
  
  // get the texture interfaces and handles
  cached_texture->Texture.lpsurf->QueryInterface (IID_IDirect3DTexture2,
    (LPVOID *)&cached_texture->Texture.lptex);
  
  if (m_bHardware)
    LoadIntoVRAM (cached_texture);
}

void D3DTextureCache::LoadIntoVRAM (csD3DCacheData *cached_texture)
{
  DDSURFACEDESC2 ddsd;
  IDirectDrawSurface4 *lpddts;
  IDirect3DTexture2 *ddtex;

  memset (&ddsd, 0, sizeof (ddsd));
  ddsd.dwSize = sizeof(ddsd);
  cached_texture->Texture.lpsurf->GetSurfaceDesc(&ddsd);
  ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;

  ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_ALLOCONLOAD;
  if (m_bMipMapping)
  {
    ddsd.dwMipMapCount = 4; 
    ddsd.dwFlags        |= DDSD_MIPMAPCOUNT;
    ddsd.ddsCaps.dwCaps |= DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
  }

  VERIFY_SUCCESS( m_lpDD->CreateSurface(&ddsd, &lpddts, NULL) );
  
  VERIFY_SUCCESS (lpddts->QueryInterface (IID_IDirect3DTexture2,
    (LPVOID *)&ddtex));
  VERIFY_SUCCESS (ddtex->Load (cached_texture->Texture.lptex));

  cached_texture->Texture.lptex->Release ();
  cached_texture->Texture.lpsurf->Release ();
  
  cached_texture->Texture.lptex = ddtex;
  cached_texture->Texture.lpsurf = lpddts;
}

void D3DTextureCache::Unload (csD3DCacheData *cached_texture)
{
  if (cached_texture->Texture.lpsurf)
  {
    cached_texture->Texture.lpsurf->Release ();
    cached_texture->Texture.lpsurf = NULL;
  }
  if (cached_texture->Texture.lptex)
  {
    cached_texture->Texture.lptex->Release ();
    cached_texture->Texture.lptex = NULL;
  }
  if (cached_texture->Texture.lpddpal)
  {
    cached_texture->Texture.lpddpal->Release ();
    cached_texture->Texture.lpddpal = NULL;
  }
}

//-------------------------------------- D3DLightMapCache Implementation -----//

D3DLightMapCache::D3DLightMapCache(int nMaxSize, bool bHardware,
  LPDIRECTDRAW4 pDDraw, LPDIRECT3DDEVICE3 pDevice, int nBpp)
  : D3DCache (nMaxSize, CS_LIGHTMAP, nBpp)
{
  m_bHardware = bHardware;
  m_lpDD = pDDraw;
  m_lpD3dDevice = pDevice;
}

void D3DLightMapCache::Dump()
{
}

void D3DLightMapCache::Load (csD3DCacheData *cached_lightmap)
{
  iLightMap *piLM = (iLightMap *)cached_lightmap->pSource;
  ASSERT( piLM );

  piLM->IncRef();
  
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
  memcpy (&ddsd.ddpfPixelFormat, &csGraphics3DDirect3DDx6::m_ddsdLightmapSurfDesc.ddpfPixelFormat, sizeof(DDPIXELFORMAT));
  
  VERIFY_SUCCESS (m_lpDD->CreateSurface (&ddsd,
    &cached_lightmap->LightMap.lpsurf, NULL));
  
  unsigned long m;
  int s;
  
//
// OPTIMIZATION CONSIDERATIONS:
// texture manager already contains rsl, rsr, gsl, gsr, bsl, bsr variables
// that can be used to convert a image to the format used internally by the
// device (see csTextureDirect3D constructor how to do it). Thus we can
// skip the folllowing calculations, if we access texture manager somehow.
//

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
  memset (&ddsd, 0, sizeof(DDSURFACEDESC2));
  ddsd.dwSize = sizeof (ddsd);
  VERIFY_SUCCESS (cached_lightmap->LightMap.lpsurf->Lock(NULL, &ddsd, 0, NULL));
  
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
          int r = *(lpRed   + (i + (lwidth * j)));
          int g = *(lpGreen + (i + (lwidth * j)));
          int b = *(lpBlue  + (i + (lwidth * j)));
        
          int mean = (r+g+b)/3;
        
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
  cached_lightmap->LightMap.lpsurf->Unlock(NULL);
  
  cached_lightmap->LightMap.ratio_width = (rwidth*0.95)/(lwidth*1.1);
  cached_lightmap->LightMap.ratio_height = (rheight*0.95)/(lheight*1.1);
  
  // get the texture interfaces and handles
  VERIFY_SUCCESS( cached_lightmap->LightMap.lpsurf->QueryInterface (
    IID_IDirect3DTexture2, (LPVOID *)&cached_lightmap->LightMap.lptex) );
  
  if (m_bHardware)
    LoadIntoVRAM (cached_lightmap);
}       

void D3DLightMapCache::LoadIntoVRAM(csD3DCacheData *cached_lightmap)
{
  DDSURFACEDESC2 ddsd;
  IDirectDrawSurface4* lpddts;
  IDirect3DTexture2* ddtex;
  
  memset(&ddsd, 0, sizeof(ddsd));
  ddsd.dwSize = sizeof(ddsd);
  cached_lightmap->LightMap.lpsurf->GetSurfaceDesc(&ddsd);
  ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
  ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_ALLOCONLOAD;
  
  VERIFY_SUCCESS( m_lpDD->CreateSurface(&ddsd, &lpddts, NULL) );
  
  lpddts->QueryInterface(IID_IDirect3DTexture2, (LPVOID *)&ddtex);     
  
  VERIFY_SUCCESS (ddtex->Load (cached_lightmap->LightMap.lptex));
  
  cached_lightmap->LightMap.lptex->Release();
  cached_lightmap->LightMap.lpsurf->Release();
  
  cached_lightmap->LightMap.lptex = ddtex;
  cached_lightmap->LightMap.lpsurf = lpddts;
}

void D3DLightMapCache::Unload(csD3DCacheData *cached_lightmap)
{
  if (cached_lightmap->LightMap.lpsurf)
  {
    cached_lightmap->LightMap.lpsurf->Release();
    cached_lightmap->LightMap.lpsurf = NULL;
  }
  if (cached_lightmap->LightMap.lptex)
  {
    cached_lightmap->LightMap.lptex->Release();
    cached_lightmap->LightMap.lptex = NULL;
  }
}


