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

// test of CVS change 

// G3D_D3D.CPP
// csGraphics3DDirect3DDx6 implementation file
// Written by Dan Ogles
// Some modifications by Nathaniel

// Ported to COM by Dan Ogles on 8.26.98
#define INITGUID

#include <windows.h>
#include <stdlib.h>
#include "ddraw.h"
#include "d3d.h"
#include "d3dcaps.h"

#include "sysdef.h"
#include "StdAssrt.h"
#include "cs2d/ddraw6/IG2D.h"
#include "cssys/win32/iDDetect.h"
#include "isystem.h"
#include "ipolygon.h"
#include "icamera.h"
#include "itexture.h"
#include "IGraph3d.h"
#include "ilghtmap.h"

#include "csutil/inifile.h"

#include "cs3d/direct3d6/g3d_d3d.h"

csIniFile *configd3d6;

void out(char *str,...)
{
  FILE *f;
  if((f=fopen("glide_out.txt", "a+"))!=NULL)
  {
    char buf[1024];
    va_list arg;
    
    va_start (arg, str);
    vsprintf (buf, str, arg);
    va_end (arg);

    fprintf(f, "%s", buf);
    fclose(f);
  }
}

/***** File-scope variables *****/

static bool bGotTexDesc=0, bGotLitDesc=0;
static const float SCALE_FACTOR = 1.0f/2500.0f;

/* ************************************************************** 
csGraphics3DDirect3DDx6 Class Definition
************************************************************** */

//
// Static member variables
//

DDPIXELFORMAT csGraphics3DDirect3DDx6::m_ddpfTexturePixelFormat = { 0 };
DDPIXELFORMAT csGraphics3DDirect3DDx6::m_ddpfLightmapPixelFormat = { 0 };

//
// Interface table definition
//

BEGIN_INTERFACE_TABLE(csGraphics3DDirect3DDx6)
IMPLEMENTS_INTERFACE( IGraphics3D )
END_INTERFACE_TABLE()

IMPLEMENT_UNKNOWN_NODELETE(csGraphics3DDirect3DDx6)


//
// Implementation
//

HRESULT CALLBACK csGraphics3DDirect3DDx6::EnumTextFormatsCallback(LPDDPIXELFORMAT lpddpf, LPVOID lpUserArg)
{
  memset(lpUserArg, TRUE, sizeof(BOOL));
  
  // we are looking for an 8 bit palettized surface for textures,
  // and a hi/true color surface for lightmaps.
  
  if(lpddpf->dwFlags & DDPF_ALPHAPIXELS)
  {
    return D3DENUMRET_OK;   // keep looking.
  }
  else if(lpddpf->dwFlags & DDPF_PALETTEINDEXED8 && !bGotTexDesc)
  {
    memcpy(&csGraphics3DDirect3DDx6::m_ddpfTexturePixelFormat, lpddpf, sizeof(DDPIXELFORMAT));
    bGotTexDesc=1;
    if(bGotLitDesc) return D3DENUMRET_CANCEL;           // got the descriptors we need    
  }
  else if(lpddpf->dwRGBBitCount >=16 && !bGotLitDesc)
  {
    memcpy(&csGraphics3DDirect3DDx6::m_ddpfLightmapPixelFormat, lpddpf, sizeof(DDPIXELFORMAT));
    bGotLitDesc=1;
    if(bGotTexDesc) return D3DENUMRET_CANCEL;
  }
  return D3DENUMRET_OK; // keep on looking
}

HRESULT CALLBACK csGraphics3DDirect3DDx6::EnumZBufferCallback( DDPIXELFORMAT* pddpf, VOID* pddpfDesired )
{
  if( pddpf->dwFlags == DDPF_ZBUFFER /*&& pddpf->dwZBufferBitDepth == 32*/)
  {
    memcpy( pddpfDesired, pddpf, sizeof(DDPIXELFORMAT) ); 
    return D3DENUMRET_CANCEL;
  } 

  return D3DENUMRET_OK;
}

csGraphics3DDirect3DDx6::csGraphics3DDirect3DDx6(ISystem* piSystem) : 
m_bIsHardware(false),
m_dwDeviceBitDepth(-1),
m_hd3dBackMat(NULL),
m_bIsLocked(false),
m_iTypeLightmap(-1),
m_pLightmapCache(NULL),
m_lpD3D(NULL),
m_lpd3dBackMat(NULL),
m_lpd3dDevice(NULL),
m_lpd3dViewport(NULL),
m_lpDD(NULL),
m_lpddDevice(NULL),
m_lpddPrimary(NULL),
m_lpddZBuffer(NULL),
m_pTextureCache(NULL),
m_piSystem(piSystem)
{
  HRESULT hRes;
  CLSID clsid2dDriver;
  char *sz2DDriver = "crystalspace.graphics2d.direct3d.dx6";
  IGraphics2DFactory* piFactory = NULL;
  
  cd3d6onfig = new csIniFile("Direct3DDX6.cfg");

  piSystem->AddRef();
  ASSERT( m_piSystem );

  SysPrintf (MSG_INITIALIZATION, "\nDirect3DRender DX6 selected\n");
  
  hRes = csCLSIDFromProgID( &sz2DDriver, &clsid2dDriver );
  
  if (FAILED(hRes))
  {	  
    SysPrintf(MSG_FATAL_ERROR, "FATAL: Cannot open \"%s\" 2D Graphics driver", sz2DDriver);
    exit(0);
  }
  
  hRes = csCoGetClassObject( clsid2dDriver, CLSCTX_INPROC_SERVER, NULL, IID_IGraphics2DFactory, (void**)&piFactory );
  if (FAILED(hRes))
  {
    SysPrintf(MSG_FATAL_ERROR, "Error! Couldn't create 2D graphics driver instance.");
    exit(0);
  }
  
  hRes = piFactory->CreateInstance( IID_IGraphics2D, m_piSystem, (void**)&m_piG2D );
  if (FAILED(hRes))
  {
    SysPrintf(MSG_FATAL_ERROR, "Error! Couldn't create 2D graphics driver instance.");
    exit(0);
  }
  
  FINAL_RELEASE( piFactory );
  
  // default
  m_Caps.ColorModel = G3DCOLORMODEL_RGB;
  m_Caps.CanClip = false;
  m_Caps.SupportsArbitraryMipMapping = false;
  m_Caps.BitDepth = 16;
  m_Caps.ZBufBitDepth = 32;
  m_Caps.minTexHeight = 2;
  m_Caps.minTexWidth = 2;
  m_Caps.maxTexHeight = 1024;
  m_Caps.maxTexWidth = 1024;
  m_Caps.PrimaryCaps.RasterCaps = G3DRASTERCAPS_SUBPIXEL;
  m_Caps.PrimaryCaps.canBlend = false;
  m_Caps.PrimaryCaps.ShadeCaps = G3DRASTERCAPS_LIGHTMAP;
  m_Caps.PrimaryCaps.PerspectiveCorrects = false;
  m_Caps.PrimaryCaps.FilterCaps = G3D_FILTERCAPS((int)G3DFILTERCAPS_NEAREST | (int)G3DFILTERCAPS_MIPNEAREST);

  use16BitTexture=cd3d6onfig->GetYesNo("Direct3DDX6","USE_16BIT_TEXTURE",false);

  rstate_dither = false;
  rstate_specular = false;
  rstate_bilinearmap = false;
  rstate_trilinearmap = false;
  rstate_flat = false;
  rstate_alphablend = false;
  rstate_mipmap = false;
  rstate_gouraud = true;
}

STDMETHODIMP csGraphics3DDirect3DDx6::Open(char* Title)
{
  LPD3DDEVICEDESC lpD3dDeviceDesc;
  DWORD dwDeviceMemType;
  DWORD dwZBufferMemType;
  DWORD dwZBufferBitDepth;
  DDSURFACEDESC2 ddsd;
  HRESULT hRes;
  BOOL ddsdFound = FALSE;
  D3DMATERIAL d3dMaterial;
  bool bMipmapping;
  
  IDirectDraw4* lpDD4 = NULL;
  DDSCAPS2 ddsCaps;
  DWORD dwTotal, dwFree;
  
  IDDraw6GraphicsInfo* pSysGInfo = NULL;
  IGraphicsInfo*      pGraphicsInfo = NULL;
  D3DRECT rect;
  
  hRes = m_piG2D->QueryInterface(IID_IDDraw6GraphicsInfo, (void**)&pSysGInfo);
  if (FAILED(hRes))
    goto OnError;
  
  m_piG2D->QueryInterface(IID_IGraphicsInfo, (void**)&pGraphicsInfo);
  if (FAILED(hRes))
    goto OnError;
  
  // Open the 2D driver.
  
  hRes = m_piG2D->Open(Title);
  if ( FAILED(hRes) )
    goto OnError;

  // Get the direct detection device.
  
  pSysGInfo->GetDirectDetection(&m_pDirectDevice);
  ASSERT( m_pDirectDevice );
  
  pSysGInfo->GetDirectDrawDriver(&m_lpDD);
  pSysGInfo->GetDirectDrawPrimary(&m_lpddPrimary);
  pSysGInfo->GetDirectDrawBackBuffer(&m_lpddDevice);
  
  pGraphicsInfo->GetWidth(m_nWidth);
  m_nHalfWidth = m_nWidth/2;
  
  pGraphicsInfo->GetHeight(m_nHeight);
  m_nHalfHeight = m_nHeight/2;
  
  ZeroMemory(&ddsCaps, sizeof(ddsCaps)); 
  ddsCaps.dwCaps = DDSCAPS_TEXTURE;
  hRes = m_lpDD->GetAvailableVidMem(&ddsCaps, &dwTotal, &dwFree);
  if ( FAILED(hRes) )
    goto OnError;
  
  // get direct3d interface
  
  hRes = m_lpDD->QueryInterface(IID_IDirect3D3, (LPVOID *)&m_lpD3D);
  if(FAILED(hRes))
    goto OnError;
  
  memset(&ddsd, 0, sizeof(ddsd));
  ddsd.dwSize = sizeof(ddsd);
  hRes = m_lpddPrimary->GetSurfaceDesc(&ddsd);
  if( FAILED(hRes) )
    goto OnError;
  
  switch (ddsd.ddpfPixelFormat.dwRGBBitCount)
  {
  case  1UL: m_dwDeviceBitDepth = DDBD_1; m_Caps.ZBufBitDepth = 1; break;
  case  2UL: m_dwDeviceBitDepth = DDBD_2; m_Caps.ZBufBitDepth = 2; break;
  case  4UL: m_dwDeviceBitDepth = DDBD_4; m_Caps.ZBufBitDepth = 4; break;
  case  8UL: m_dwDeviceBitDepth = DDBD_8; m_Caps.ZBufBitDepth = 8; break;
  case 16UL: m_dwDeviceBitDepth = DDBD_16; m_Caps.ZBufBitDepth = 16; break;
  case 24UL: m_dwDeviceBitDepth = DDBD_24; m_Caps.ZBufBitDepth = 24; break;
  case 32UL: m_dwDeviceBitDepth = DDBD_32; m_Caps.ZBufBitDepth = 32; break;
  default:
    ASSERT( FALSE );
  }
  
  // assign globals for software/hardware
  lpD3dDeviceDesc = m_pDirectDevice->GetDesc3D();
  memcpy(&m_Guid, m_pDirectDevice->GetGuid3D(), sizeof(GUID));
  
  ASSERT(m_pDirectDevice->GetHardware());
  
  dwDeviceMemType = DDSCAPS_VIDEOMEMORY;
  dwZBufferMemType = DDSCAPS_VIDEOMEMORY;
  m_bIsHardware = true;
  

  if (m_pDirectDevice->GetMipmap())
    bMipmapping = true;
  else
    bMipmapping = false;
  
  // select type of lightmapping
  if (m_pDirectDevice->GetAlphaBlend())
  {
    if (m_pDirectDevice->GetAlphaBlendType() == 1)        
      m_iTypeLightmap = 1;        
    else
      m_iTypeLightmap = 2;
  }
  else
  {
    m_iTypeLightmap = 0;
  }
  
  // Create Z-buffer
  
  if (!lpD3dDeviceDesc->dwDeviceZBufferBitDepth && m_bIsHardware)
  {
    hRes = CSD3DERR_NOZBUFFER;
    goto OnError;
  }

  DDPIXELFORMAT ddpfZBuffer;

  m_lpD3D->EnumZBufferFormats(m_Guid, EnumZBufferCallback, (VOID*)&ddpfZBuffer );

  memset(&ddsd, 0, sizeof(ddsd));
  ddsd.dwSize = sizeof(ddsd);
  ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
  ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | dwZBufferMemType;
  ddsd.dwWidth = m_nWidth;
  ddsd.dwHeight = m_nHeight;
  memcpy( &ddsd.ddpfPixelFormat, &ddpfZBuffer, sizeof(DDPIXELFORMAT) );
  
  hRes = m_lpDD->CreateSurface(&ddsd, &m_lpddZBuffer, NULL);
  if (FAILED(hRes))
    goto OnError;   
  
  hRes = m_lpddDevice->AddAttachedSurface(m_lpddZBuffer);
  if (FAILED(hRes))
    goto OnError; 
  
  // get the device interface
  hRes = m_lpD3D->CreateDevice(m_Guid, m_lpddDevice, &m_lpd3dDevice, NULL);
  if (FAILED(hRes))
    goto OnError;        
  
  // get the texture format we want
  m_lpd3dDevice->EnumTextureFormats(EnumTextFormatsCallback, (LPVOID)&ddsdFound);
  
  if (bGotLitDesc && (!bGotTexDesc || use16BitTexture))
  {
    if(use16BitTexture)
      SysPrintf (MSG_INITIALIZATION, "\nForce 16-bit format used for texture memory.\n");
    else
      SysPrintf (MSG_INITIALIZATION, "\nWARNING: Slower 16-bit format used for texture memory.\n");
    memcpy(&m_ddpfTexturePixelFormat, &m_ddpfLightmapPixelFormat, sizeof(DDPIXELFORMAT));
    bGotTexDesc = true;
  }
  
  if (bGotTexDesc && !bGotLitDesc)
  {
    SysPrintf (MSG_INITIALIZATION, "\nWARNING: No 16-bit texture format supported in hardware. Grayscale lighting used.\n");
    memcpy(&m_ddpfLightmapPixelFormat, &m_ddpfTexturePixelFormat, sizeof(DDPIXELFORMAT));
  }
  
  if (!bGotTexDesc && !bGotLitDesc)
  {
    hRes = E_FAIL;
    goto OnError;
  }    
  
  // create a black background
  
  hRes = m_lpD3D->CreateMaterial(&m_lpd3dBackMat, NULL);
  if (FAILED(hRes))
    goto OnError;
  
  memset(&d3dMaterial, 0, sizeof(d3dMaterial));
  d3dMaterial.dwSize = sizeof(d3dMaterial);
  d3dMaterial.dwRampSize = 1;
  
  hRes = m_lpd3dBackMat->SetMaterial(&d3dMaterial);
  if (FAILED(hRes))
    goto OnError;     
  
  hRes = m_lpd3dBackMat->GetHandle(m_lpd3dDevice, &m_hd3dBackMat);
  if (FAILED(hRes))
    goto OnError;
  
  // create the viewport
  
  hRes = m_lpD3D->CreateViewport(&m_lpd3dViewport, NULL);
  if (FAILED(hRes))
    goto OnError;
  
  // assign the viewport
  
  hRes = m_lpd3dDevice->AddViewport(m_lpd3dViewport);
  if (FAILED(hRes))
    goto OnError;
  
  // assign the background to the viewport
  
  hRes = m_lpd3dViewport->SetBackground(m_hd3dBackMat);
  if (FAILED(hRes))
    goto OnError;
  
  // set default render-states.
  m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
  m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE);
	m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE);
/*  
  hRes = m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_LINEAR);
  if (FAILED(hRes))
    m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_NEAREST);
  
  m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEARMIPLINEAR);
  if (FAILED(hRes))
  {
    hRes = m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_MIPLINEAR);
    if (FAILED(hRes))
      m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEAR);
  }
*/
  m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE);

  m_lpd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_LINEAR );
	m_lpd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_LINEAR );
  m_lpd3dDevice->SetTextureStageState( 1, D3DTSS_MIPFILTER, D3DTFP_NONE );  
  switch(bMipmapping)
  {
  case true:    
    m_lpd3dDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTFP_LINEAR );
    break;

  case false:
    m_lpd3dDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTFP_NONE );
    SysPrintf (MSG_INITIALIZATION, "\nWARNING: Hardware don't support mipmapping !\n");
    break;
  }

  switch( m_iTypeLightmap )
  {
  case 0:
    SysPrintf (MSG_INITIALIZATION, "\nWARNING: Hardware don't support lightmaps !\n");
    break;
    
  case 1:    
    m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCCOLOR);
    m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_DESTCOLOR);
    break;
    
  case 2:
    SysPrintf (MSG_INITIALIZATION, "\nWARNING: Hardware support only bad lightmapping !\n");
    m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
    m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCCOLOR);
    break;
    
  default:
    ASSERT( FALSE );
  }
  
  // Set default Z-buffer mode.
  m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
  m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);

  m_ZBufMode = CS_ZBUF_FILL;
  
  // save half of the memory for textures,
  // half for lightmaps
  if (m_iTypeLightmap != 0)
  {
    CHK (m_pTextureCache = new D3DTextureCache(dwFree/2, m_bIsHardware, m_lpDD, m_lpd3dDevice, m_ddpfTexturePixelFormat.dwRGBBitCount, bMipmapping));
    CHK (m_pLightmapCache = new D3DLightMapCache(dwFree/2, m_bIsHardware, m_lpDD, m_lpd3dDevice, m_ddpfLightmapPixelFormat.dwRGBBitCount));
  }
  else
  {
    CHK (m_pTextureCache = new D3DTextureCache(dwFree, m_bIsHardware, m_lpDD, m_lpd3dDevice, m_ddpfTexturePixelFormat.dwRGBBitCount, bMipmapping));
    m_pLightmapCache = NULL;
  }
  
  // init the viewport.
  SetDimensions(m_nWidth, m_nHeight);
  
  // clear the Z-buffer    
  rect.x1 = 0; rect.y1 = 0; 
  rect.x2 = m_nWidth; rect.y2 = m_nHeight;
  
  hRes = m_lpd3dViewport->Clear(1, &rect, D3DCLEAR_ZBUFFER);
  if (FAILED(hRes))
    goto OnError;
  
OnError:
  
  if (FAILED(hRes))
    FINAL_RELEASE(m_piG2D);
  
  FINAL_RELEASE(pGraphicsInfo);
  FINAL_RELEASE(pSysGInfo);
  
  return hRes;
}

csGraphics3DDirect3DDx6::~csGraphics3DDirect3DDx6()
{   
  FINAL_RELEASE( m_pCamera );
  FINAL_RELEASE( m_piG2D );
  FINAL_RELEASE( m_piSystem );
}

STDMETHODIMP csGraphics3DDirect3DDx6::Close()
{
  ClearCache();
  
  FINAL_RELEASE(m_lpd3dBackMat);
  FINAL_RELEASE(m_lpd3dViewport);
  FINAL_RELEASE(m_lpd3dDevice);
  FINAL_RELEASE(m_lpd3dBackMat);
  FINAL_RELEASE(m_lpd3dViewport);
  FINAL_RELEASE(m_lpD3D);
  
  return S_OK;
}

STDMETHODIMP csGraphics3DDirect3DDx6::SetDimensions(int nWidth, int nHeight)
{
  D3DVIEWPORT2 d3dViewport;
  
  ASSERT( m_lpd3dViewport );
  ASSERT( nWidth && nHeight );
  
  m_nWidth = nWidth;
  m_nHeight = nHeight;
  m_nHalfWidth = nWidth/2;
  m_nHalfHeight = nHeight/2;
  
  memset(&d3dViewport, 0, sizeof(d3dViewport));
  d3dViewport.dwSize = sizeof(D3DVIEWPORT2);
  d3dViewport.dwX = 0;
  d3dViewport.dwY = 0;
  d3dViewport.dwWidth = nWidth;
  d3dViewport.dwHeight = nHeight;
  d3dViewport.dvClipX = 0;
  d3dViewport.dvClipY = 0;
  d3dViewport.dvClipWidth = (float)nWidth;
  d3dViewport.dvClipHeight = (float)nHeight;
  d3dViewport.dvMinZ = -1.0f;
  d3dViewport.dvMaxZ = 1.0f;
  
  if (FAILED(m_lpd3dViewport->SetViewport2(&d3dViewport)))
    return E_FAIL;
  
  if (FAILED(m_lpd3dDevice->SetCurrentViewport(m_lpd3dViewport)))
    return E_FAIL;
  
  return S_OK;
}

STDMETHODIMP csGraphics3DDirect3DDx6::BeginDraw (int nDrawFlags)
{
  if (nDrawFlags & CSDRAW_2DGRAPHICS)
  {
    // if graphics is in 3D mode, turn it off
    if (m_nDrawMode & CSDRAW_3DGRAPHICS)
      FinishDraw ();
    
    // if 2D mode is not enabled, turn it on
    if (!(m_nDrawMode & CSDRAW_2DGRAPHICS))
      m_piG2D->BeginDraw ();
  }
  else if (nDrawFlags & CSDRAW_3DGRAPHICS)
  {
    // if graphics is in 2D mode, turn it off
    if (m_nDrawMode & CSDRAW_2DGRAPHICS)
      FinishDraw ();
    
    // if 3D mode is not enabled, turn it on
    if (!(m_nDrawMode & CSDRAW_3DGRAPHICS))
    {
      HRESULT hRes;
      DWORD dwClearFlag = 0;
      D3DRECT rect;
      
      rect.x1 = 0; rect.x2 = m_nWidth;
      rect.y1 = 0; rect.y2 = m_nHeight;
      
      if (nDrawFlags & CSDRAW_CLEARZBUFFER)
        dwClearFlag |= D3DCLEAR_ZBUFFER;
      
      if (nDrawFlags & CSDRAW_CLEARSCREEN)
        dwClearFlag |= D3DCLEAR_TARGET;
      
      if (dwClearFlag)
      {
        hRes = m_lpd3dViewport->Clear(1UL, &rect, dwClearFlag);
        if (FAILED(hRes))
          return E_FAIL;
      }
      
      hRes = m_lpd3dDevice->BeginScene();
      if (FAILED(hRes))
        return E_FAIL;
      
    } 
  } 
  
  m_nDrawMode = nDrawFlags;
  
  return S_OK;
}

STDMETHODIMP csGraphics3DDirect3DDx6::FinishDraw ()
{
  if (m_nDrawMode & CSDRAW_2DGRAPHICS)
  {
    // delegate to the 2D driver to finish up it's operations
    m_piG2D->FinishDraw ();
  }
  else
  {
    // end our scene (we're in 3D state)
    HRESULT hRes;
    
    hRes = m_lpd3dDevice->EndScene();
    if (FAILED(hRes))
      return E_FAIL;
  }
  m_nDrawMode = 0;
  
  return S_OK;
}

STDMETHODIMP csGraphics3DDirect3DDx6::SetZBufMode(G3DZBufMode mode)
{
  if (mode==m_ZBufMode) 
    return S_OK;
  
  m_ZBufMode = mode;
  
  if (mode == CS_ZBUF_TEST)
    VERIFY( m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE) == DD_OK );
  else
    VERIFY( m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE) == DD_OK );    
  
  if (mode == CS_ZBUF_FILL)      // write-only
    VERIFY( m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS) == DD_OK );
  else 
    VERIFY( m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL) == DD_OK );
  
  return S_OK;
}


STDMETHODIMP csGraphics3DDirect3DDx6::DumpCache()
{
  m_pTextureCache->Dump();
  m_pLightmapCache->Dump();
  
  return S_OK;
}

STDMETHODIMP csGraphics3DDirect3DDx6::ClearCache()
{
  if ( m_pTextureCache ) 
  {
    m_pTextureCache->Clear();
  }
  if ( m_pLightmapCache )
  {
    m_pLightmapCache->Clear();
  }
  
  return S_OK;
}

STDMETHODIMP csGraphics3DDirect3DDx6::CacheTexture(IPolygonTexture *piPT)
{
  ITextureMap* piTM = NULL;
  IMipMapContainer* piMMC = NULL;
  
  piPT->GetTexture(&piTM);
  ASSERT( piTM );
  
  piTM->GetParent(&piMMC);
  ASSERT( piMMC );
  
  ASSERT( m_pTextureCache != NULL );
  
  m_pTextureCache->Add(piMMC);
  
  if (m_iTypeLightmap && m_pLightmapCache) 
  {
    m_pLightmapCache->Add(piPT);
  } 
  
  FINAL_RELEASE( piTM );
  FINAL_RELEASE( piMMC );
  
  return S_OK;
}

STDMETHODIMP csGraphics3DDirect3DDx6::UncacheTexture(IPolygonTexture *texture)
{
  (void)texture;
  return E_NOTIMPL;
}

void csGraphics3DDirect3DDx6::SetupPolygon( G3DPolygonDP& poly, float& J1, float& J2, float& J3, 
                                        float& K1, float& K2, float& K3,
                                        float& M,  float& N,  float& O  )
{
  float P1, P2, P3, P4, Q1, Q2, Q3, Q4;
  
  float Ac = poly.normal.A, 
    Bc = poly.normal.B, 
    Cc = poly.normal.C, 
    Dc = poly.normal.D;
  
  float inv_aspect;
  
  ASSERT( m_pCamera );
  m_pCamera->GetInvAspect(inv_aspect);
  
  // Get the plane normal of the polygon. Using this we can calculate
  // '1/z' at every screen space point.
  
  if (ABS (Dc) < 0.06)
  {
    M = 0;
    N = 0;
    O = 1/poly.z_value;
  }
  else
  {
    float inv_Dc = 1/Dc;
    
    M = -Ac*inv_Dc*inv_aspect;
    N = -Bc*inv_Dc*inv_aspect;
    O = -Cc*inv_Dc;
  }
  
  P1 = poly.plane.m_cam2tex->m11;
  P2 = poly.plane.m_cam2tex->m12;
  P3 = poly.plane.m_cam2tex->m13;
  P4 = -(P1*poly.plane.v_cam2tex->x + P2*poly.plane.v_cam2tex->y + P3*poly.plane.v_cam2tex->z);
  Q1 = poly.plane.m_cam2tex->m21;
  Q2 = poly.plane.m_cam2tex->m22;
  Q3 = poly.plane.m_cam2tex->m23;
  Q4 = -(Q1*poly.plane.v_cam2tex->x + Q2*poly.plane.v_cam2tex->y + Q3*poly.plane.v_cam2tex->z);
  
  
  if (ABS (Dc) < 0.06)
  {
    // The Dc component of the plane of the polygon is too small.
    J1= 0;
    J2= 0;
    J3= 0;
    K1= 0;
    K2= 0;
    K3= 0;
  }
  else
  {
    J1 = P1 * inv_aspect + P4*M;
    J2 = P2 * inv_aspect + P4*N;
    J3 = P3              + P4*O;
    K1 = Q1 * inv_aspect + Q4*M;
    K2 = Q2 * inv_aspect + Q4*N;
    K3 = Q3              + Q4*O;
  }
}


STDMETHODIMP csGraphics3DDirect3DDx6::DrawPolygon (G3DPolygonDP& poly)
{
  ASSERT( m_lpd3dDevice );
  
  bool bLightmapExists = true,
    bColorKeyed     = false,
    bTransparent;
  
  int  poly_alpha;
  
  float J1, J2, J3, K1, K2, K3;
  float M, N, O;
  int i;
  
  HighColorCache_Data* pTexCache;
  HighColorCache_Data* pLightCache;
  D3DTLVERTEX vx;
  
  IPolygonTexture* pTex;
  
  float z;
  
  if (poly.num < 3) 
  {
    return E_INVALIDARG;
  }
  
  // retrieve the texture.
  pTex = poly.poly_texture[0];
  
  if (!pTex)
    return E_INVALIDARG;
  
  // set up the geometry.
  SetupPolygon( poly, J1, J2, J3, K1, K2, K3, M, N, O );

  // cache the texture and initialize rasterization.
  CacheTexture (pTex);
  
  ITextureMap* piTM;
  IMipMapContainer* piMMC;
  
  poly_alpha = poly.alpha;
  bTransparent = poly_alpha>0 ? true : false;

  pTex->GetTexture( &piTM );
  ASSERT( piTM );
  
  piTM->GetParent( &piMMC );
  ASSERT( piMMC );
  
  piMMC->GetTransparent (bColorKeyed);
 
  // retrieve the cached texture handle.
  piMMC->GetHighColorCache( &pTexCache ); 
  ASSERT( pTexCache );
  
  FINAL_RELEASE( piTM );
  FINAL_RELEASE( piMMC );
  
  // retrieve the lightmap from the cache.
  ILightMap* piLM = NULL;
  pTex->GetLightMap( &piLM );
  
  if ( piLM )
  {
    piLM->GetHighColorCache( &pLightCache );
    if (!pLightCache)
    {
      bLightmapExists = false;
    }
    
    piLM->Release();
  }
  else
  {
    bLightmapExists=false;
  }
  
  // 
  // Setup Direct3D render states for texture mapping
  //

/*
// Set the diffuse light map.lpD3DDev->SetTexture(1,lptexDiffuseLightMap ); 
// Set the blend stage.
lpD3DDev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_ADD );
lpD3DDev->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
lpD3DDev->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT );
*/
/*
  if ( bTransparent )
  {
    m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
    m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCALPHA);
  }
  
  if ( bColorKeyed )
  {
    m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, TRUE);
  }
*/  
//  m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, ((D3DTextureCache_Data *)pTexCache->pData)->htex);
  m_lpd3dDevice->SetTexture(0, ((D3DTextureCache_Data *)pTexCache->pData)->lptex);
  // Set the base texture operation and args

  m_lpd3dDevice->SetTextureStageState(0,D3DTSS_COLOROP,
    D3DTOP_MODULATE );
  m_lpd3dDevice->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE );
  m_lpd3dDevice->SetTextureStageState(0,D3DTSS_COLORARG2, D3DTA_DIFFUSE ); 
  
  m_lpd3dDevice->Begin(D3DPT_TRIANGLEFAN, D3DVT_TLVERTEX, D3DDP_DONOTUPDATEEXTENTS);
  
  // render texture-mapped poly
  
  for (i=0; i < poly.num; i++)
  {
    z = 1.0f  / (M*(poly.vertices[i].sx-m_nHalfWidth) + N*(poly.vertices[i].sy-m_nHalfHeight) + O);
    
    vx.sx = poly.vertices[i].sx;
    vx.sy = m_nHeight-poly.vertices[i].sy;
    vx.sz = z*(float)SCALE_FACTOR;
    vx.rhw = 1/z;


    if (m_iTypeLightmap == 2)
    {
      vx.color = D3DRGBA(1.0f, 1.0f, 1.0f, (float)poly_alpha/100.0f);
    }
    else
    {
      vx.color = D3DRGBA(0.5f, 0.5f, 0.5f, (float)poly_alpha/100.0f);
    }
    
    vx.specular = 0;
    vx.tu =  (J1 * (poly.vertices[i].sx-m_nHalfWidth) + J2 * (poly.vertices[i].sy-m_nHalfHeight) + J3) * z;
    vx.tv =  (K1 * (poly.vertices[i].sx-m_nHalfWidth) + K2 * (poly.vertices[i].sy-m_nHalfHeight) + K3) * z;
    
    m_lpd3dDevice->Vertex( &vx );  
  }
  
  m_lpd3dDevice->End(0);

/*  
  float lu_dif, lv_dif, lu_scale, lv_scale;
  float fMinU, fMinV, fMaxU, fMaxV;
  
  pTex->GetTextureBox( fMinU, fMinV, fMaxU, fMaxV );
  
  if ( !bTransparent  &&  bLightmapExists  &&  m_iTypeLightmap )
  {
    // calculate lightmapping variables.
    
    lu_dif = fMinU;
    lv_dif = fMinV;
    
    lu_scale = 1.0f/(fMaxU - fMinU);
    lv_scale = 1.0f/(fMaxV - fMinV);
    
    // 
    //  Setup DirectX render states for light mapping.
    //
    m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
    m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCCOLOR);
    m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_EQUAL);
    m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_CLAMP);
    
  ASSERT( ((D3DLightCache_Data*)pLightCache->pData)->htex != 0 );
    m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, ((D3DLightCache_Data *)pLightCache->pData)->htex);
    // Set the blend stage.
    lpD3DDev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_ADD );
    lpD3DDev->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    lpD3DDev->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT );
    
    // render light-mapped poly
    
    m_lpd3dDevice->Begin(D3DPT_TRIANGLEFAN, D3DVT_TLVERTEX, D3DDP_DONOTUPDATEEXTENTS);
    for (i=0; i<poly.num; i++)
    {
      z = 1.0f  / (M*(poly.vertices[i].sx-m_nHalfWidth) + N*(poly.vertices[i].sy-m_nHalfHeight) + O);
      
      vx.sx = poly.vertices[i].sx;
      vx.sy = m_nHeight-poly.vertices[i].sy;
      vx.sz = z*(float)SCALE_FACTOR;
      vx.rhw = 1/z;
      
      if(m_iTypeLightmap == 2)            
        vx.color = D3DRGBA(0.5, 0.5, 0.5, 0.5);            
      else
        vx.color = D3DRGB(0.5, 0.5, 0.5);
      
      vx.specular = 0;
      vx.tu =  (((J1 * (poly.vertices[i].sx-m_nHalfWidth) + J2 * (poly.vertices[i].sy-m_nHalfHeight) + J3) * z) - lu_dif) * lu_scale;
      vx.tv =  (((K1 * (poly.vertices[i].sx-m_nHalfWidth) + K2 * (poly.vertices[i].sy-m_nHalfHeight) + K3) * z) - lv_dif) * lv_scale;
      
      m_lpd3dDevice->Vertex( &vx );
    }
    m_lpd3dDevice->End(0);
    
    // reset render states.
    
    m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_WRAP);
    m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);
    m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
    
    m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);        
  }

  // reset render states.
  
  if (bColorKeyed)
  {
    m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);
  }
  
  if (bTransparent)
  {
    m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);
    m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
  }
*/  
  return S_OK;
}

STDMETHODIMP csGraphics3DDirect3DDx6::DrawPolygonQuick (G3DPolygonDPQ& poly)
{    
  int i;
  HighColorCache_Data *pTexData;
  D3DTLVERTEX vx;
  
  ASSERT( m_pTextureCache );
  
  m_pTextureCache->Add(poly.pi_texture);
  poly.pi_texture->GetHighColorCache(&pTexData);
  
  VERIFY( m_lpd3dDevice->SetTexture(0, ((D3DTextureCache_Data *)pTexData->pData)->lptex) == DD_OK);
  VERIFY( m_lpd3dDevice->Begin(D3DPT_TRIANGLEFAN, D3DVT_TLVERTEX, D3DDP_DONOTUPDATEEXTENTS) == DD_OK );
  
  for(i=0; i<poly.num; i++)
  {
    vx.sx = poly.vertices[i].sx;
    vx.sy = m_nHeight-poly.vertices[i].sy;
    vx.sz = SCALE_FACTOR / poly.vertices[i].z;
    vx.rhw = poly.vertices[i].z;
    vx.color = D3DRGB(0.8, 0.8, 0.8);
    vx.specular = D3DRGB(0.0, 0.0, 0.0);
    vx.tu = poly.vertices[i].u;
    vx.tv = poly.vertices[i].v;
    
    m_lpd3dDevice->Vertex( &vx );
  }
  
  VERIFY( m_lpd3dDevice->End(0) == DD_OK );
  
  return S_OK;
}

STDMETHODIMP csGraphics3DDirect3DDx6::DrawFltLight(G3DFltLight& light)
{
  return E_NOTIMPL;
}

STDMETHODIMP csGraphics3DDirect3DDx6::GetCaps(G3D_CAPS *caps)
{
  if (!caps)
    return E_INVALIDARG;
  
  memcpy(caps, &m_Caps, sizeof(G3D_CAPS));
  
  return S_OK;
}

STDMETHODIMP csGraphics3DDirect3DDx6::GetStringError( HRESULT hRes, char* szErrorString )
{
  return E_NOTIMPL;
}

STDMETHODIMP csGraphics3DDirect3DDx6::DrawLine (csVector3& v1, csVector3& v2, int color)
{
  ASSERT( m_pCamera );
  if (v1.z < SMALL_Z && v2.z < SMALL_Z) return S_FALSE;
  
  float x1 = v1.x, y1 = v1.y, z1 = v1.z;
  float x2 = v2.x, y2 = v2.y, z2 = v2.z;
  
  if (z1 < SMALL_Z)
  {
    // x = t*(x2-x1)+x1;
    // y = t*(y2-y1)+y1;
    // z = t*(z2-z1)+z1;
    float t = ((float)SMALL_Z-z1) / (z2-z1);
    x1 = t*(x2-x1)+x1;
    y1 = t*(y2-y1)+y1;
    z1 = (float)SMALL_Z;
  }
  else if (z2 < SMALL_Z)
  {
    // x = t*(x2-x1)+x1;
    // y = t*(y2-y1)+y1;
    // z = t*(z2-z1)+z1;
    float t = ((float)SMALL_Z-z1) / (z2-z1);
    x2 = t*(x2-x1)+x1;
    y2 = t*(y2-y1)+y1;
    z2 = (float)SMALL_Z;
  }
  
  float aspect;
  m_pCamera->GetAspect( aspect );
  
  float iz1 = aspect/z1;
  int px1 = QInt (x1 * iz1 + (m_nWidth/2));
  int py1 = m_nHeight - 1 - QInt (y1 * iz1 + (m_nHeight/2));
  float iz2 = aspect/z2;
  int px2 = QInt (x2 * iz2 + (m_nWidth/2));
  int py2 = m_nHeight - 1 - QInt (y2 * iz2 + (m_nHeight/2));
  
  m_piG2D->DrawLine (px1, py1, px2, py2, color);
  
  return S_OK;
}

STDMETHODIMP csGraphics3DDirect3DDx6::SetRenderState(G3D_RENDERSTATEOPTION option, long value)
{
  switch (option)
  {
  case G3DRENDERSTATE_NOTHING:
    return S_OK;
    
  case G3DRENDERSTATE_ZBUFFERTESTENABLE:
    if (value)
      m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);
    else
      m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS);
    break;
    
  case G3DRENDERSTATE_ZBUFFERFILLENABLE:
    if (value)
      m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
    else
      m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
    break;
    
  case G3DRENDERSTATE_DITHERENABLE:
    rstate_dither = value;
    break;
  case G3DRENDERSTATE_SPECULARENABLE:
    rstate_specular = value;
    break;
  case G3DRENDERSTATE_BILINEARMAPPINGENABLE:
    rstate_bilinearmap = value;
    break;
  case G3DRENDERSTATE_TRILINEARMAPPINGENABLE:
    rstate_trilinearmap = value;
    break;
  case G3DRENDERSTATE_TRANSPARENCYENABLE:
    rstate_alphablend = value;
    break;
  case G3DRENDERSTATE_MIPMAPENABLE:
    rstate_mipmap = value;
    break;
  case G3DRENDERSTATE_TEXTUREMAPPINGENABLE:
    rstate_flat = !value;
    break;
  case G3DRENDERSTATE_DEBUGENABLE :
  case G3DRENDERSTATE_FILTERINGENABLE :
  case G3DRENDERSTATE_PERFECTMAPPINGENABLE :
  case G3DRENDERSTATE_LIGHTINGENABLE :
  case G3DRENDERSTATE_INTERLACINGENABLE :
  case G3DRENDERSTATE_MMXENABLE :
    break;
  case G3DRENDERSTATE_GOURAUDENABLE:
    rstate_gouraud = value;
    break;
  default:
    return E_INVALIDARG;
  }
  
  return S_OK;
}

STDMETHODIMP csGraphics3DDirect3DDx6::GetRenderState(G3D_RENDERSTATEOPTION op, long& retval)
{
  DWORD d3d_value;
  switch(op)
  {
    case G3DRENDERSTATE_NOTHING:
      retval = 0;
      break;
    case G3DRENDERSTATE_ZBUFFERTESTENABLE:
      m_lpd3dDevice->GetRenderState(D3DRENDERSTATE_ZFUNC, &d3d_value);
      retval = (bool)(D3DCMP_LESSEQUAL & d3d_value);
      break;
    case G3DRENDERSTATE_ZBUFFERFILLENABLE:
      m_lpd3dDevice->GetRenderState(D3DRENDERSTATE_ZWRITEENABLE, &d3d_value);
      retval = (bool)(TRUE & d3d_value);
      break;
    case G3DRENDERSTATE_DITHERENABLE:
      retval = rstate_dither;
      break;
    case G3DRENDERSTATE_SPECULARENABLE:
      retval = rstate_specular;
      break;
    case G3DRENDERSTATE_BILINEARMAPPINGENABLE:
      retval = rstate_bilinearmap;
      break;
    case G3DRENDERSTATE_TRILINEARMAPPINGENABLE:
      retval = rstate_trilinearmap;
      break;
    case G3DRENDERSTATE_TRANSPARENCYENABLE:
      retval = rstate_alphablend;
      break;
    case G3DRENDERSTATE_MIPMAPENABLE:
      retval = rstate_mipmap;
      break;
    case G3DRENDERSTATE_TEXTUREMAPPINGENABLE:
      retval = !rstate_flat;
      break;
    case G3DRENDERSTATE_GOURAUDENABLE:
      retval = rstate_gouraud;
      break;
    default:
      retval = 0;
      return E_INVALIDARG;
  }
  
  return S_OK;
}

STDMETHODIMP csGraphics3DDirect3DDx6::OpenFogObject (CS_ID id, csFog* fog)
{
  return E_NOTIMPL;
}

STDMETHODIMP csGraphics3DDirect3DDx6::AddFogPolygon (CS_ID id, G3DPolygonAFP& poly, int fogtype)
{
  return E_NOTIMPL;
}

STDMETHODIMP csGraphics3DDirect3DDx6::CloseFogObject (CS_ID id)
{
  return E_NOTIMPL;
}

void csGraphics3DDirect3DDx6::SysPrintf(int mode, char* szMsg, ...)
{
  char buf[1024];
  va_list arg;
  
  va_start (arg, szMsg);
  vsprintf (buf, szMsg, arg);
  va_end (arg);
  
  m_piSystem->Print(mode, buf);
}


