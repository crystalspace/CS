/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein and Dan Ogles.

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

// G3D_D3D.CPP
// csGraphics3DDirect3DDx5 implementation file
// Written by Dan Ogles
// Some modifications by Nathaniel Saint Martin

// Ported to COM by Dan Ogles on 8.26.98
#define INITGUID

#include "cssysdef.h"
#include <stdlib.h>
#include <ddraw.h>
#include <d3d.h>
#include <d3dcaps.h>

#include "csutil/scf.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/plane3.h"
#include "video/renderer/direct3d5/d3d_g3d.h"
#include "video/renderer/direct3d5/d3d_txtcache.h"
#include "video/canvas/ddraw/IG2D.h"
#include "cssys/win32/iDDetect.h"
#include "csutil/inifile.h"
#include "csgeom/polyclip.h"
#include "qint.h"
#include "isystem.h"
#include "igraph3d.h"
#include "itxtmgr.h"
#include "itexture.h"
#include "ipolygon.h"
#include "icamera.h"
#include "ilghtmap.h"
#include "igraph2d.h"

/***** File-scope variables *****/

static bool bGotTexDesc=false, bGotLitDesc=false, bGotHaloDesc=false;
static bool use32BitTexture=false, use16BitTexture=false;
static const float SCALE_FACTOR = 1.0f/2500.0f;

/* ************************************************************** 
csGraphics3DDirect3DDx5 Class Definition
************************************************************** */

//
// Static member variables
//

DDSURFACEDESC csGraphics3DDirect3DDx5::m_ddsdTextureSurfDesc = { 0 };
DDSURFACEDESC csGraphics3DDirect3DDx5::m_ddsdLightmapSurfDesc = { 0 };
DDSURFACEDESC csGraphics3DDirect3DDx5::m_ddsdHaloSurfDesc = { 0 };

//
// Interface table definition
//

IMPLEMENT_FACTORY (csGraphics3DDirect3DDx5)

EXPORT_CLASS_TABLE (d3ddx5)
  EXPORT_CLASS (csGraphics3DDirect3DDx5, "crystalspace.graphics3d.direct3d.dx5",
    "Direct3D DX5 3D graphics driver for Crystal Space")
EXPORT_CLASS_TABLE_END

IMPLEMENT_IBASE (csGraphics3DDirect3DDx5)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iGraphics3D)
IMPLEMENT_IBASE_END

//
// Implementation
//

HRESULT CALLBACK csGraphics3DDirect3DDx5::EnumTextFormatsCallback(LPDDSURFACEDESC lpddsd, LPVOID lpUserArg)
{
  memset(lpUserArg, TRUE, sizeof(BOOL));
  
  // Search a halo texture format with alpha channel
  if (lpddsd->ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS)
  {
    if (lpddsd->ddpfPixelFormat.dwRGBBitCount == 16
      && lpddsd->ddpfPixelFormat.dwRGBAlphaBitMask == 0xF000
      && !bGotHaloDesc)
    {
      memcpy(&csGraphics3DDirect3DDx5::m_ddsdHaloSurfDesc, lpddsd, sizeof(DDSURFACEDESC));
      bGotHaloDesc = true;
    }

    if (lpddsd->ddpfPixelFormat.dwRGBBitCount == 32
      && lpddsd->ddpfPixelFormat.dwRGBAlphaBitMask == 0xFF000000
      && use32BitTexture)
    {
      memcpy(&csGraphics3DDirect3DDx5::m_ddsdHaloSurfDesc, lpddsd, sizeof(DDSURFACEDESC));
      bGotHaloDesc = true;
    }
  }
  // Search a 16/32 bits format
  else if(lpddsd->ddpfPixelFormat.dwRGBBitCount >=16)
  {
    if (lpddsd->ddpfPixelFormat.dwRGBBitCount == 16)
    {
      //Only use that format, if there has not been found annother format. (That might be the
      //32 Bit texture formtat if use32BitTexture is true)
      if(!bGotLitDesc)
      {
        memcpy(&csGraphics3DDirect3DDx5::m_ddsdLightmapSurfDesc, lpddsd, sizeof(DDSURFACEDESC));
        bGotLitDesc = true;
      }

      if(!bGotTexDesc)
      {
        memcpy(&csGraphics3DDirect3DDx5::m_ddsdTextureSurfDesc, lpddsd, sizeof(DDSURFACEDESC));
        bGotTexDesc = true;
      }
    }
    else if (lpddsd->ddpfPixelFormat.dwRGBBitCount == 32)
    {
      if (use32BitTexture)
      {
        //This will override a potentially found 16 Bit texture mode:
        memcpy(&csGraphics3DDirect3DDx5::m_ddsdLightmapSurfDesc, lpddsd, sizeof(DDSURFACEDESC));
        bGotLitDesc = true;
        memcpy(&csGraphics3DDirect3DDx5::m_ddsdTextureSurfDesc, lpddsd, sizeof(DDSURFACEDESC));
        bGotTexDesc = true;
      }
    }
  }

  return D3DENUMRET_OK; // keep on looking
}

csGraphics3DDirect3DDx5::csGraphics3DDirect3DDx5(iBase* iParent) : 
  m_bIsHardware(false),
  m_bHaloEffect(false),
  m_dwDeviceBitDepth(0),
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
  m_piSystem(NULL),
  m_bVerbose(true),
  config(NULL),
  m_pClipper(NULL)
{
  CONSTRUCT_IBASE (iParent);

  // default
  m_Caps.CanClip = false;
  m_Caps.minTexHeight = 2;
  m_Caps.minTexWidth = 2;
  m_Caps.maxTexHeight = 1024;
  m_Caps.maxTexWidth = 1024;
  m_Caps.fog = G3DFOGMETHOD_VERTEX;
  m_Caps.NeedsPO2Maps = true;
  m_Caps.MaxAspectRatio = 1; 

  zdist_mipmap1 = 12;
  zdist_mipmap2 = 24;
  zdist_mipmap3 = 40;

  rstate_dither = false;
  rstate_bilinearmap = false;
  rstate_trilinearmap = true;
  rstate_gouraud = true;
  rstate_flat = true;
  rstate_alphablend = true;
  rstate_mipmap = true;

  m_gouraud = true;
}

csGraphics3DDirect3DDx5::~csGraphics3DDirect3DDx5 ()
{
  if (config)
    delete config;
  if (m_piG2D)
    m_piG2D->DecRef ();
  if (m_piSystem)
    m_piSystem->DecRef ();
}

bool csGraphics3DDirect3DDx5::Initialize (iSystem *iSys)
{
  m_piSystem = iSys;
  m_piSystem->IncRef ();

  m_piSystem->Printf (MSG_INITIALIZATION, "\nDirect3DRender selected\n");

  iVFS* v = m_piSystem->GetVFS();
  config = new csIniFile (v, "/config/direct3ddx5.cfg");
  v->DecRef(); v = NULL;

  m_piG2D = LOAD_PLUGIN (m_piSystem, SOFTWARE_2D_DRIVER, NULL, iGraphics2D);
  if (!m_piG2D)
    return false;

  txtmgr = new csTextureManagerDirect3D (m_piSystem, m_piG2D, config);

  m_bVerbose = config->GetYesNo ("Direct3DDX5", "VERBOSE", false);

  use16BitTexture = config->GetYesNo ("Direct3DDX5", "USE_16BIT_TEXTURE", false);
  if (use16BitTexture)
    use32BitTexture = config->GetYesNo ("Direct3DDX5", "EXTEND_32BIT_TEXTURE", false);

  return true;
}

bool csGraphics3DDirect3DDx5::Open(const char* Title)
{
  LPD3DDEVICEDESC lpD3dDeviceDesc;
  DWORD dwDeviceMemType;
  DWORD dwZBufferMemType;
  DWORD dwZBufferBitDepth = 0;
  DDSURFACEDESC ddsd;
  HRESULT hRes;
  BOOL ddsdFound = FALSE;
  D3DMATERIAL d3dMaterial;
  bool bMipmapping;
  
  IDirectDraw2* lpDD2 = NULL;
  DDSCAPS ddsCaps;
  DWORD dwTotal, dwFree;
  D3DRECT rect;

  // Get the direct detection device.
  iGraphics2DDDraw3* pSysGInfo = QUERY_INTERFACE(m_piG2D, iGraphics2DDDraw3);
  ASSERT(pSysGInfo);

  pSysGInfo->SetFor3D(true);

  // Open the 2D driver.
  
  if (!m_piG2D->Open(Title))
    return false;
  
  pSysGInfo->GetDirectDetection(&m_pDirectDevice);
  ASSERT( m_pDirectDevice );
  
  pSysGInfo->GetDirectDrawDriver(&m_lpDD);
  ASSERT(m_lpDD);

  pSysGInfo->GetDirectDrawPrimary(&m_lpddPrimary);
  ASSERT(m_lpddPrimary);

  pSysGInfo->GetDirectDrawBackBuffer(&m_lpddDevice);
  ASSERT(m_lpddDevice);
  
  m_nWidth = m_piG2D->GetWidth();
  m_nHalfWidth = m_nWidth/2;
  
  m_nHeight = m_piG2D->GetHeight();
  m_nHalfHeight = m_nHeight/2;

  // get the amount of texture memory on the video card.
  hRes = m_lpDD->QueryInterface(IID_IDirectDraw2, (LPVOID*)&lpDD2);
  if ( FAILED(hRes) )
    return false;
  
  ddsCaps.dwCaps = DDSCAPS_TEXTURE;
  hRes = lpDD2->GetAvailableVidMem(&ddsCaps, &dwTotal, &dwFree);
  FINAL_RELEASE(lpDD2);
  
  if ( FAILED(hRes) )
    return false;
  
  // get direct3d interface
  
  hRes = m_lpDD->QueryInterface(IID_IDirect3D2, (LPVOID *)&m_lpD3D);
  if(FAILED(hRes))
    return false;
  
  memset(&ddsd, 0, sizeof(ddsd));
  ddsd.dwSize = sizeof(ddsd);
  hRes = m_lpddPrimary->GetSurfaceDesc(&ddsd);
  if( FAILED(hRes) )
    return false;
  
  switch (ddsd.ddpfPixelFormat.dwRGBBitCount)
  {
  case  1UL: m_dwDeviceBitDepth = DDBD_1; break;
  case  2UL: m_dwDeviceBitDepth = DDBD_2; break;
  case  4UL: m_dwDeviceBitDepth = DDBD_4; break;
  case  8UL: m_dwDeviceBitDepth = DDBD_8; break;
  case 16UL: m_dwDeviceBitDepth = DDBD_16; break;
  case 24UL: m_dwDeviceBitDepth = DDBD_24; break;
  case 32UL: m_dwDeviceBitDepth = DDBD_32; break;
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
  
  // Create Z-buffer
  if (!lpD3dDeviceDesc->dwDeviceZBufferBitDepth && m_bIsHardware)
  {
    //hRes = CSD3DERR_NOZBUFFER;
    return false;
  }
  
  if (lpD3dDeviceDesc->dwDeviceZBufferBitDepth & DDBD_32) 
    dwZBufferBitDepth = 32;
  else if (lpD3dDeviceDesc->dwDeviceZBufferBitDepth & DDBD_24) 
    dwZBufferBitDepth = 24;
  else if (lpD3dDeviceDesc->dwDeviceZBufferBitDepth & DDBD_16) 
    dwZBufferBitDepth = 16;
  else if (lpD3dDeviceDesc->dwDeviceZBufferBitDepth & DDBD_8) 
    dwZBufferBitDepth = 8;
  else if (lpD3dDeviceDesc->dwDeviceZBufferBitDepth & DDBD_4) 
    dwZBufferBitDepth = 4;
  else if (lpD3dDeviceDesc->dwDeviceZBufferBitDepth & DDBD_2) 
    dwZBufferBitDepth = 2;
  else if (lpD3dDeviceDesc->dwDeviceZBufferBitDepth & DDBD_1) 
    dwZBufferBitDepth = 1;
  else ASSERT( FALSE );

  m_Caps.minTexHeight = lpD3dDeviceDesc->dwMinTextureHeight;
  m_Caps.minTexWidth  = lpD3dDeviceDesc->dwMinTextureWidth;
  m_Caps.maxTexHeight = lpD3dDeviceDesc->dwMaxTextureHeight;
  m_Caps.maxTexWidth  = lpD3dDeviceDesc->dwMaxTextureWidth;

  if (lpD3dDeviceDesc->dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY)
  {
    m_piSystem->Printf (MSG_INITIALIZATION, " Warning: Your Direct3D Device supports only square textures!\n");
    m_piSystem->Printf (MSG_INITIALIZATION, "          This is a potential performance hit!\n");
    m_Caps.MaxAspectRatio = 1;
  }
  else
  {
    m_piSystem->Printf (MSG_INITIALIZATION, "Your Direct3D Device also supports non square textures - that's good.\n");
    m_Caps.MaxAspectRatio = 32768;
  }

  memset(&ddsd, 0, sizeof(ddsd));
  ddsd.dwSize = sizeof(ddsd);
  ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_ZBUFFERBITDEPTH;
  ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | dwZBufferMemType;
  ddsd.dwWidth = m_nWidth;
  ddsd.dwHeight = m_nHeight;
  ddsd.dwZBufferBitDepth = dwZBufferBitDepth;
  
  hRes = m_lpDD->CreateSurface(&ddsd, &m_lpddZBuffer, NULL);
  if (FAILED(hRes))
    return false;   
  
  hRes = m_lpddDevice->AddAttachedSurface(m_lpddZBuffer);
  if (FAILED(hRes))
    return false; 
  
  if(m_bVerbose)
    m_piSystem->Printf (MSG_INITIALIZATION, " Use %d depth for ZBuffer.\n", dwZBufferBitDepth);

  // get the device interface
  hRes = m_lpD3D->CreateDevice(m_Guid, m_lpddDevice, &m_lpd3dDevice);
  if (FAILED(hRes))
    return false;        
  
  // get the texture format we want
  m_lpd3dDevice->EnumTextureFormats(EnumTextFormatsCallback, (LPVOID)&ddsdFound);
  
  if (bGotLitDesc && !bGotTexDesc)
  {
    memcpy(&m_ddsdTextureSurfDesc, &m_ddsdLightmapSurfDesc, sizeof(DDSURFACEDESC));
    bGotTexDesc = true;
  }
  
  if (bGotTexDesc && !bGotLitDesc)
  {
    memcpy(&m_ddsdLightmapSurfDesc, &m_ddsdTextureSurfDesc, sizeof(DDSURFACEDESC));
    bGotLitDesc = true;
  }

  if (!bGotTexDesc && !bGotLitDesc)
  {
    m_piSystem->Printf (MSG_INITIALIZATION, " ERROR : No 16 or 32 bits texture format supported in hardware.\n");
    hRes = E_FAIL;
    return false;
  }    

  if(m_bVerbose)
  {
    if(m_ddsdTextureSurfDesc.ddpfPixelFormat.dwRGBBitCount==16)
      m_piSystem->Printf (MSG_INITIALIZATION, " Use 16 bits texture format.\n");
    else
      m_piSystem->Printf (MSG_INITIALIZATION, " Use 32 bits texture format.\n");
  }

  // select type of lightmapping
  if (m_pDirectDevice->GetAlphaBlend() && !config->GetYesNo("Direct3DDX5","DISABLE_LIGHTMAP", false))
  {
    if (m_pDirectDevice->GetAlphaBlendType() == 1)
    {
      m_iTypeLightmap = 1;
    }
    else
    {
      m_piSystem->Printf (MSG_INITIALIZATION, " WARNING : Bad lightmapping supported.\n");
      m_iTypeLightmap = 2;
    }
  }
  else
  {
    if(config->GetYesNo("Direct3DDX5","DISABLE_LIGHTMAP", false))
      m_piSystem->Printf (MSG_INITIALIZATION, " WARNING : Lightmapping disable by user.\n");
    else
      m_piSystem->Printf (MSG_INITIALIZATION, " WARNING : Lightmapping not supported by hadware.\n");
    m_iTypeLightmap = 0;
  }

  if(!m_iTypeLightmap)
  {
    m_piSystem->Printf (MSG_INITIALIZATION, " WARNING : Lightmapping disable.\n");
  }
  else
  {
    if(m_bVerbose)
    {
      if(m_ddsdLightmapSurfDesc.ddpfPixelFormat.dwRGBBitCount==8)
        m_piSystem->Printf (MSG_INITIALIZATION, " Use 8 bits palettized format for lightmap memory.\n");
      else if(m_ddsdLightmapSurfDesc.ddpfPixelFormat.dwRGBBitCount==16)
        m_piSystem->Printf (MSG_INITIALIZATION, " Use 16 bits lightmap format.\n");
      else
        m_piSystem->Printf (MSG_INITIALIZATION, " Use 32 bits lightmap format.\n");
    }
  }
 
  // set Halo effect configuration
  if (m_pDirectDevice->GetAlphaBlendHalo() && !config->GetYesNo("Direct3DDX5","DISABLE_HALO", false))
    m_bHaloEffect = true;
  else
  {
    if(m_pDirectDevice->GetAlphaBlendHalo() && config->GetYesNo("Direct3DDX5","DISABLE_HALO", false))
      m_piSystem->Printf (MSG_INITIALIZATION, " WARNING : Halo effect disable by user.\n");
    else
      m_piSystem->Printf (MSG_INITIALIZATION, " WARNING : Halo effect not support by hardware.\n");
    m_bHaloEffect = false;
  }

  if (!bGotHaloDesc && m_bHaloEffect)
  {
    m_piSystem->Printf (MSG_INITIALIZATION, " WARNING : No halo texture format supported by hardware.\n");
    m_bHaloEffect = false;
  }

  if(!m_bHaloEffect)
  {
    m_piSystem->Printf (MSG_INITIALIZATION, " WARNING : Halo effect disable.\n");
  }
  else
  {
    if(m_bVerbose)
      m_piSystem->Printf (MSG_INITIALIZATION, " Use %d bits format for halo effect\n",
        m_ddsdHaloSurfDesc.ddpfPixelFormat.dwRGBBitCount);
  }

  if(m_bVerbose)
  {
    m_piSystem->Printf (MSG_INITIALIZATION, " Use 24 bits internal format for texture\n");
  }

  // set mipmapping configuration
  if (m_pDirectDevice->GetMipmap() && !config->GetYesNo("Direct3DDX5","DISABLE_MIPMAP", false))
    bMipmapping = true;
  else
  {
    if(config->GetYesNo("Direct3DDX5","DISABLE_MIPMAP", false)
      && m_pDirectDevice->GetMipmap())
      m_piSystem->Printf (MSG_INITIALIZATION, " WARNING : Mipmapping disable by user.\n");
    else
      m_piSystem->Printf (MSG_INITIALIZATION, " WARNING : Mipmapping not supported in hardware.\n");
    bMipmapping = false;
  }
  if(!bMipmapping)
  {
    m_piSystem->Printf (MSG_INITIALIZATION, " WARNING : Mipmapping disable.\n");
  }

  // create a black background
  hRes = m_lpD3D->CreateMaterial(&m_lpd3dBackMat, NULL);
  if (FAILED(hRes))
    return false;
  
  memset(&d3dMaterial, 0, sizeof(d3dMaterial));
  d3dMaterial.dwSize = sizeof(d3dMaterial);
  d3dMaterial.dwRampSize = 1;
  
  hRes = m_lpd3dBackMat->SetMaterial(&d3dMaterial);
  if (FAILED(hRes))
    return false;     
  
  hRes = m_lpd3dBackMat->GetHandle(m_lpd3dDevice, &m_hd3dBackMat);
  if (FAILED(hRes))
    return false;
  
  // create the viewport
  
  hRes = m_lpD3D->CreateViewport(&m_lpd3dViewport, NULL);
  if (FAILED(hRes))
    return false;
  
  // assign the viewport
  
  hRes = m_lpd3dDevice->AddViewport(m_lpd3dViewport);
  if (FAILED(hRes))
    return false;
  
  // assign the background to the viewport
  
  hRes = m_lpd3dViewport->SetBackground(m_hd3dBackMat);
  if (FAILED(hRes))
    return false;
  
  // set default render-states.
  m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
  m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE);
        m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE);
  
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
  
  if(config->GetYesNo("Direct3DDX5","ENABLE_DITHER", true))
    m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE);
  else
    m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE);
  
  m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
  m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);

  // Set default Z-buffer mode.
  m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
  m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);
  m_ZBufMode = CS_ZBUF_FILL;
  
  // save half of the memory for textures,
  // half for lightmaps
  if (m_iTypeLightmap != 0)
  {
    m_pTextureCache = new D3DTextureCache(dwFree/2, m_bIsHardware, m_lpDD, m_lpd3dDevice, m_ddsdTextureSurfDesc.ddpfPixelFormat.dwRGBBitCount, bMipmapping, &m_Caps);
    m_pLightmapCache = new D3DLightMapCache(dwFree/2, m_bIsHardware, m_lpDD, m_lpd3dDevice, m_ddsdLightmapSurfDesc.ddpfPixelFormat.dwRGBBitCount);
  }
  else
  {
    m_pTextureCache = new D3DTextureCache(dwFree, m_bIsHardware, m_lpDD, m_lpd3dDevice, m_ddsdTextureSurfDesc.ddpfPixelFormat.dwRGBBitCount, bMipmapping, &m_Caps);
    m_pLightmapCache = NULL;
  }
  
  // init the viewport.
  SetDimensions(m_nWidth, m_nHeight);
  
  // clear the Z-buffer    
  rect.x1 = 0; rect.y1 = 0; 
  rect.x2 = m_nWidth; rect.y2 = m_nHeight;
  
  hRes = m_lpd3dViewport->Clear(1, &rect, D3DCLEAR_ZBUFFER);
  if (FAILED(hRes))
    return false;
  return true;
}

void csGraphics3DDirect3DDx5::Close()
{
  ClearCache();
  
  FINAL_RELEASE(m_lpd3dBackMat);
  FINAL_RELEASE(m_lpd3dViewport);
  FINAL_RELEASE(m_lpd3dDevice);
  FINAL_RELEASE(m_lpd3dBackMat);
  FINAL_RELEASE(m_lpd3dViewport);
  FINAL_RELEASE(m_lpD3D);
}

void csGraphics3DDirect3DDx5::SetDimensions(int nWidth, int nHeight)
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
    return;
  
  if (FAILED(m_lpd3dDevice->SetCurrentViewport(m_lpd3dViewport)))
    return;
}

void csGraphics3DDirect3DDx5::SetPerspectiveCenter (int x, int y)
{
  m_nHalfWidth = x;
  m_nHalfHeight = y;
}

bool csGraphics3DDirect3DDx5::BeginDraw (int nDrawFlags)
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
        return false;
    } 
  } 
  
  m_nDrawMode = nDrawFlags;
  return true;
}

void csGraphics3DDirect3DDx5::FinishDraw ()
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
      return;
  }
  m_nDrawMode = 0;
}

void csGraphics3DDirect3DDx5::DumpCache()
{
  m_pTextureCache->Dump();
  m_pLightmapCache->Dump();
}

void csGraphics3DDirect3DDx5::ClearCache()
{
  if ( m_pTextureCache ) 
    m_pTextureCache->Clear();
  if ( m_pLightmapCache )
    m_pLightmapCache->Clear();
}

void csGraphics3DDirect3DDx5::CacheTexture(iPolygonTexture *texture)
{
  iTextureHandle* txt_handle = texture->GetTextureHandle ();
  m_pTextureCache->Add (txt_handle);
  m_pLightmapCache->Add (texture);
}

void csGraphics3DDirect3DDx5::SetupPolygon( G3DPolygonDP& poly, float& J1, float& J2, float& J3, 
                                        float& K1, float& K2, float& K3,
                                        float& M,  float& N,  float& O  )
{
  float P1, P2, P3, P4, Q1, Q2, Q3, Q4;
  
  float Ac = poly.normal.A (), 
    Bc = poly.normal.B (), 
    Cc = poly.normal.C (), 
    Dc = poly.normal.D ();
  
  float inv_aspect = poly.inv_aspect;
  
  // Get the plane normal of the polygon. Using this we can calculate
  // '1/z' at every screen space point.
  
  if (ABS (Dc) < 0.06)
  {
    M = 0;
    N = 0;
    O = 1/(poly.z_value);
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

void csGraphics3DDirect3DDx5::DrawPolygon (G3DPolygonDP& poly)
{
  ASSERT( m_lpd3dDevice );
  
  bool bLightmapExists = true,
    bColorKeyed     = false,
    bTransparent;
  
  int  poly_alpha;
  
  float J1, J2, J3, K1, K2, K3;
  float M, N, O;
  int i;
  
  csD3DCacheData* pTexCache   = NULL;
  csD3DCacheData* pLightCache = NULL;
  D3DTLVERTEX vx;
  
  float z;
  
  if (poly.num < 3) 
  {
    return;
  }
  
  // set up the geometry.
  SetupPolygon( poly, J1, J2, J3, K1, K2, K3, M, N, O );

  poly_alpha = poly.alpha;
  bTransparent = poly_alpha>0 ? true : false;

  //Mipmapping is done for us by DirectX, so we will always select the texture with
  //the highest resolution.
  iPolygonTexture *pTex = poly.poly_texture;

  if (!pTex)
    return;

  // cache the texture and initialize rasterization.
  CacheTexture (pTex);

  // retrieve the texture from the cache by handle.
  csTextureMMDirect3D* txt_mm = (csTextureMMDirect3D*)poly.txt_handle->GetPrivateObject ();
  pTexCache = (csD3DCacheData *)txt_mm->GetCacheData ();
  
  bColorKeyed = txt_mm->GetTransparent ();
 
  // retrieve the lightmap from the cache.
  iLightMap* piLM = pTex->GetLightMap ();
  
  if ( piLM )
  {
    pLightCache = (csD3DCacheData *)piLM->GetCacheData ();
    if (!pLightCache)
    {
      bLightmapExists = false;
    }
  }
  else
  {
    bLightmapExists=false;
  }
  
  if(m_iTypeLightmap == 0)
    bLightmapExists = false;

  if( bLightmapExists )
  {
    switch( m_iTypeLightmap )
    {
    case 1:    
      VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_DESTCOLOR), DD_OK );
      VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCCOLOR), DD_OK );
      break;
      
    case 2:
      VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_SRCALPHA), DD_OK );
      VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCCOLOR), DD_OK );
      break;
    }
  }
  
  if ( bTransparent )
  {
    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE), DD_OK );
    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCALPHA), DD_OK );
    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_INVSRCALPHA), DD_OK );
  }

  if ( bColorKeyed )
  {
    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, TRUE), DD_OK );
  }

  D3DTextureCache_Data *pD3D_texcache = (D3DTextureCache_Data *)pTexCache->pData;

  VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, pD3D_texcache->htex), DD_OK);
  
  VERIFY_RESULT(m_lpd3dDevice->Begin(D3DPT_TRIANGLEFAN, D3DVT_TLVERTEX, D3DDP_DONOTUPDATEEXTENTS), DD_OK);
  
  // render texture-mapped poly
  
  for (i=0; i < poly.num; i++)
  {
    z = 1.0f  / (M*(poly.vertices[i].sx-m_nHalfWidth) + N*(poly.vertices[i].sy-m_nHalfHeight) + O);
    
    vx.sx = poly.vertices[i].sx;
    vx.sy = m_nHeight-poly.vertices[i].sy;

    vx.sz = z*(float)SCALE_FACTOR;

    if(vx.sz>0.9999)
      vx.sz=0.9999;

    vx.rhw = 1/z;

    vx.color = D3DRGBA(1.0f, 1.0f, 1.0f, (float)poly_alpha/100.0f);
        
    vx.specular = 0;
    vx.tu =  (J1 * (poly.vertices[i].sx-m_nHalfWidth) + J2 * (poly.vertices[i].sy-m_nHalfHeight) + J3) * z;
    vx.tv =  (K1 * (poly.vertices[i].sx-m_nHalfWidth) + K2 * (poly.vertices[i].sy-m_nHalfHeight) + K3) * z;
    
    m_lpd3dDevice->Vertex( &vx );  
  }
  
  m_lpd3dDevice->End(0);
  
  if ( bLightmapExists )
  {
    iLightMap *thelightmap = pTex->GetLightMap ();

    int lmwidth = thelightmap->GetWidth ();
    int lmrealwidth = thelightmap->GetRealWidth ();
    int lmheight = thelightmap->GetHeight ();
    int lmrealheight = thelightmap->GetRealHeight ();
    float scale_u = (float)(lmrealwidth-1) / (float)lmwidth;
    float scale_v = (float)(lmrealheight-1) / (float)lmheight;

    float lightmap_low_u, lightmap_low_v, lightmap_high_u, lightmap_high_v;
    pTex->GetTextureBox(lightmap_low_u,lightmap_low_v,
                       lightmap_high_u,lightmap_high_v);

    float lightmap_scale_u = scale_u / (lightmap_high_u - lightmap_low_u), 
          lightmap_scale_v = scale_v / (lightmap_high_v - lightmap_low_v);

    if(!bTransparent)
    {
      VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE), DD_OK);
    }
    else
    {
      VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCALPHA), DD_OK);     
      VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_DESTCOLOR), DD_OK);
    }

    //Attention: Now we need to be in a D3DRENDERSTATE_ZFUNC that allows the same z-value to be
    //written once again. We are operating at D3DCMP_LESSEQUAL so we are safe here.
    //setting to D3DCMP_EQUAL should theoretically work too, but will result in some
    //visual problems in 32Bit color. (driver problems I guess)
    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_CLAMP), DD_OK);
    
    ASSERT( ((D3DLightCache_Data*)pLightCache->pData)->htex != 0 );
    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, 
                                  ((D3DLightCache_Data *)pLightCache->pData)->htex), DD_OK);
    
    // render light-mapped poly
    VERIFY_RESULT(m_lpd3dDevice->Begin(D3DPT_TRIANGLEFAN, D3DVT_TLVERTEX, D3DDP_DONOTUPDATEEXTENTS), DD_OK);
    for (i=0; i<poly.num; i++)
    {
      float sx = poly.vertices[i].sx - m_nHalfWidth;
      float sy = poly.vertices[i].sy - m_nHalfHeight;

      float u_over_sz = (J1 * sx + J2 * sy + J3);
      float v_over_sz = (K1 * sx + K2 * sy + K3);

      z = 1.0f  / (M*sx + N*sy + O);
      
      float light_u = (u_over_sz*z - lightmap_low_u) * lightmap_scale_u;
      float light_v = (v_over_sz*z - lightmap_low_v) * lightmap_scale_v;

      vx.sx = poly.vertices[i].sx;
      vx.sy = m_nHeight-poly.vertices[i].sy;
      vx.sz = z*(float)SCALE_FACTOR;

      if(vx.sz>0.9999)
        vx.sz=0.9999;

      vx.rhw = 1/z;
      
      if(!bTransparent)
      {
        if(m_iTypeLightmap == 2)
          vx.color = (D3DCOLOR) D3DRGBA(1.0f, 1.0f, 1.0f, 1.0f);
        else
          vx.color = D3DRGB(1.0f, 1.0f, 1.0f);
      }
      else 
      {
        vx.color = D3DRGBA(1.0f, 1.0f, 1.0f, (float)poly_alpha/100.0f);
      }
      
      vx.specular = 0;

      vx.tu = light_u;
      vx.tv = light_v;

      VERIFY_RESULT(m_lpd3dDevice->Vertex( &vx ), DD_OK);
    }
    VERIFY_RESULT(m_lpd3dDevice->End(0), DD_OK);
    
    // reset render states.
    
    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_WRAP), DD_OK);
    if(!bTransparent)
      VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE), DD_OK);
    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL), DD_OK);
  }
  
  // reset render states.
  // If there is vertex fog then we apply that last.
  if (poly.use_fog)
  {
    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE),  DD_OK);
    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL), DD_OK);
    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA), DD_OK); 
    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_SRCALPHA), DD_OK);
    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, 0), DD_OK);

    VERIFY_RESULT(m_lpd3dDevice->Begin(D3DPT_TRIANGLEFAN, D3DVT_TLVERTEX, D3DDP_DONOTUPDATEEXTENTS), DD_OK);

    for (i=0; i<poly.num; i++)
    {
      float sx = poly.vertices[i].sx - m_nHalfWidth;
      float sy = poly.vertices[i].sy - m_nHalfHeight;

      z = 1.0f  / (M*sx + N*sy + O);
      
      vx.sx = poly.vertices[i].sx;
      vx.sy = m_nHeight-poly.vertices[i].sy;
      vx.sz = z*(float)SCALE_FACTOR;

      if(vx.sz>0.9999)
        vx.sz=0.9999;

      vx.rhw = 1/z;
      
      float I = poly.fog_info[i].intensity; 
      float r = poly.fog_info[i].r; 
      float g = poly.fog_info[i].g; 
      float b = poly.fog_info[i].b; 

      if (I > 1.0) 
      {
        I = 1.0;
      }

      if (r>1.0f || g>1.0f || b>1.0f)
      {
        float      max = r;
        if (g>max) max = g;
        if (b>max) max = b;

        float f = 1/max;
        r *= f;
        g *= f;
        b *= f;
      }

      vx.color    = D3DRGBA(r, g, b, I);
      vx.specular = 0;

      VERIFY_RESULT(m_lpd3dDevice->Vertex( &vx ), DD_OK);
    }
    VERIFY_RESULT(m_lpd3dDevice->End(0), DD_OK);
    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE),  DD_OK);
  }
 
  if (bColorKeyed)
  {
    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE), DD_OK);
  }
  
  if (bTransparent )
  {
    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE), DD_OK);
    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE), DD_OK);
  }

  if (bLightmapExists)
  {
    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO), DD_OK);
    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE), DD_OK);
  }
} 

void csGraphics3DDirect3DDx5::StartPolygonFX (iTextureHandle* handle,
  UInt mode)
{
  ASSERT(handle);
  
  //In the Crystal Space DirectX renderer, we handle Alpha differently: 0.0 is transparent, and
  //1.0 opaque. It looks like this is more close to the way DirectX will internall handle Alpha 
  //transparency.
  float alpha = float (255 - (mode & CS_FX_MASK_ALPHA)) / 255.;
  m_gouraud = rstate_gouraud && ((mode & CS_FX_GOURAUD) != 0);
  m_mixmode = mode;
  m_alpha   = alpha;

  csTextureMMDirect3D* txt_mm = (csTextureMMDirect3D*)handle->GetPrivateObject ();

  m_pTextureCache->Add (handle);

  csD3DCacheData* pTexData = (csD3DCacheData *)txt_mm->GetCacheData ();

  if (txt_mm->GetTransparent ())
    VERIFY_RESULT (m_lpd3dDevice->SetRenderState (D3DRENDERSTATE_COLORKEYENABLE, TRUE), DD_OK);

  if ((mode & CS_FX_MASK_MIXMODE) != CS_FX_COPY)
    VERIFY_RESULT (m_lpd3dDevice->SetRenderState (D3DRENDERSTATE_ALPHABLENDENABLE, TRUE), DD_OK);

  //Note: In all explanations of Mixing:
  //Color: resulting color
  //SRC:   Color of the texel (content of the texture to be drawn)
  //DEST:  Color of the pixel on screen
  //Alpha: Alpha value of the polygon
  switch (mode & CS_FX_MASK_MIXMODE)
  {
    case CS_FX_MULTIPLY:
      //Color = SRC * DEST +   0 * SRC = DEST * SRC
      m_alpha = 1.0f;
      VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCCOLOR), DD_OK);
      VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO), DD_OK);
      break;
    case CS_FX_MULTIPLY2:
      //Color = SRC * DEST + DEST * SRC = 2 * DEST * SRC
      m_alpha = 1.0f;
      VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCCOLOR), DD_OK); 
      VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_DESTCOLOR), DD_OK);
      break;
    case CS_FX_ADD:
      //Color = 1 * DEST + 1 * SRC = DEST + SRC
      m_alpha = 1.0f;
      VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE), DD_OK); 
      VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE), DD_OK);
      break;
    case CS_FX_ALPHA:
      //Color = Alpha * DEST + (1-Alpha) * SRC 
      VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA), DD_OK); 
      VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_SRCALPHA),    DD_OK);
      break;
    case CS_FX_TRANSPARENT:
      //Color = 1 * DEST + 0 * SRC = DEST
      VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE), DD_OK); 
      VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO), DD_OK);
      break;
    case CS_FX_COPY:
    default:
      //Color = 0 * DEST + 1 * SRC = SRC
      m_alpha = 1.0f;
      VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO), DD_OK); 
      VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE), DD_OK);
      break;
  }

  VERIFY_RESULT (m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, ((D3DTextureCache_Data *)pTexData->pData)->htex), DD_OK);
}

void csGraphics3DDirect3DDx5::FinishPolygonFX()
{
  VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE), DD_OK);

  VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE), DD_OK);

  VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO), DD_OK); 
  VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE), DD_OK);
}

void csGraphics3DDirect3DDx5::DrawPolygonFX(G3DPolygonDPFX& poly)
{
  int i;
  D3DTLVERTEX vx;

  VERIFY_RESULT( m_lpd3dDevice->Begin(D3DPT_TRIANGLEFAN, D3DVT_TLVERTEX, D3DDP_DONOTUPDATEEXTENTS), DD_OK );
  
  for(i=0; i<poly.num; i++)
  {
    vx.sx = poly.vertices[i].sx;
    vx.sy = m_nHeight-poly.vertices[i].sy;
    vx.sz = SCALE_FACTOR / poly.vertices[i].z;
    vx.rhw = poly.vertices[i].z;
    if (m_gouraud)
      vx.color = D3DRGBA(poly.vertices[i].r, poly.vertices[i].g, poly.vertices[i].b, m_alpha);
    else
      vx.color = D3DRGBA(1.0f, 1.0f, 1.0f, m_alpha);
    vx.specular = D3DRGB(0.0, 0.0, 0.0);
    vx.tu = poly.vertices[i].u;
    vx.tv = poly.vertices[i].v;
    
    VERIFY_RESULT(m_lpd3dDevice->Vertex( &vx ), DD_OK);
  }

  VERIFY_RESULT( m_lpd3dDevice->End(0), DD_OK );

  if (poly.use_fog)
  {
    // Remember old values.
    // @@@ This can't be efficient! We are setting and resetting
    // Renderstate values for every triangle of a sprite. Maybe we
    // should first do all non-fogged triangles and then in one
    // go all fogged triangles?

    DWORD OldAlpha;
    DWORD OldZFunc;
    DWORD OldDestBlend;
    DWORD OldSrcBlend;
    DWORD OldTexture;

    VERIFY_RESULT(m_lpd3dDevice->GetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, &OldAlpha),     DD_OK);
    VERIFY_RESULT(m_lpd3dDevice->GetRenderState(D3DRENDERSTATE_ZFUNC,            &OldZFunc),     DD_OK);
    VERIFY_RESULT(m_lpd3dDevice->GetRenderState(D3DRENDERSTATE_DESTBLEND,        &OldDestBlend), DD_OK); 
    VERIFY_RESULT(m_lpd3dDevice->GetRenderState(D3DRENDERSTATE_SRCBLEND,         &OldSrcBlend),  DD_OK);
    VERIFY_RESULT(m_lpd3dDevice->GetRenderState(D3DRENDERSTATE_TEXTUREHANDLE,    &OldTexture),   DD_OK);
   
    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE),  DD_OK);
    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC,            D3DCMP_LESSEQUAL), DD_OK);
    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,        D3DBLEND_INVSRCALPHA), DD_OK); 
    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,         D3DBLEND_SRCALPHA), DD_OK);
    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE,    0), DD_OK);

    VERIFY_RESULT( m_lpd3dDevice->Begin(D3DPT_TRIANGLEFAN, D3DVT_TLVERTEX, D3DDP_DONOTUPDATEEXTENTS), DD_OK );
  
    for(i=0; i<poly.num; i++)
    {
      vx.sx = poly.vertices[i].sx;
      vx.sy = m_nHeight-poly.vertices[i].sy;
      vx.sz = SCALE_FACTOR / poly.vertices[i].z;
      vx.rhw = poly.vertices[i].z;

      float I = poly.fog_info[i].intensity; 
      float r = poly.fog_info[i].r; 
      float g = poly.fog_info[i].g; 
      float b = poly.fog_info[i].b; 

      if (I > 1.0) 
      {
        I = 1.0;
      }

      if (r>1.0f || g>1.0f || b>1.0f)
      {
        float      max = r;
        if (g>max) max = g;
        if (b>max) max = b;

        float f = 1/max;
        r *= f;
        g *= f;
        b *= f;
      }

      vx.color    = D3DRGBA(r, g, b, I);
      vx.specular = D3DRGB(0.0, 0.0, 0.0);
      vx.tu = poly.vertices[i].u;
      vx.tv = poly.vertices[i].v;
    
      VERIFY_RESULT(m_lpd3dDevice->Vertex( &vx ), DD_OK);
    }

    VERIFY_RESULT( m_lpd3dDevice->End(0), DD_OK );

    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, OldAlpha),     DD_OK);
    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC,            OldZFunc),     DD_OK);
    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,        OldDestBlend), DD_OK); 
    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,         OldSrcBlend),  DD_OK);
    VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE,    OldTexture),   DD_OK);
  }
}

void csGraphics3DDirect3DDx5::DrawLine (const csVector3& v1, const csVector3& v2, float aspect, int color)
{
  if (v1.z < SMALL_Z && v2.z < SMALL_Z) return;
  
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
  
  float iz1 = aspect/z1;
  int px1 = QInt (x1 * iz1 + (m_nWidth/2));
  int py1 = m_nHeight - 1 - QInt (y1 * iz1 + (m_nHeight/2));
  float iz2 = aspect/z2;
  int px2 = QInt (x2 * iz2 + (m_nWidth/2));
  int py2 = m_nHeight - 1 - QInt (y2 * iz2 + (m_nHeight/2));
  
  m_piG2D->DrawLine (px1, py1, px2, py2, color);
}

bool csGraphics3DDirect3DDx5::SetRenderState(G3D_RENDERSTATEOPTION option, long value)
{
  switch (option)
  {
    case G3DRENDERSTATE_ZBUFFERMODE:
      m_States.SetZFunc (value & CS_ZBUF_TEST ? D3DCMP_LESSEQUAL : D3DCMP_ALWAYS);
      VERIFY_RESULT (m_lpd3dDevice2->SetRenderState (D3DRENDERSTATE_ZWRITEENABLE,
        (value & CS_ZBUF_FILL) ? TRUE : FALSE), DD_OK);
      break;
    case G3DRENDERSTATE_DITHERENABLE:
      if(value)
        VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE), DD_OK);
      else
        VERIFY_RESULT(m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE), DD_OK);
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
    case G3DRENDERSTATE_LIGHTINGENABLE :
    case G3DRENDERSTATE_INTERLACINGENABLE :
    case G3DRENDERSTATE_MMXENABLE :
      break;
    case G3DRENDERSTATE_GOURAUDENABLE:
      rstate_gouraud = value;
      break;
    default:
      return false;
  }

  return true;
}

long csGraphics3DDirect3DDx5::GetRenderState(G3D_RENDERSTATEOPTION op)
{
  DWORD d3d_value, mode;
  switch(op)
  {
    case G3DRENDERSTATE_ZBUFFERMODE:
      mode = 0;
      VERIFY_RESULT(m_lpd3dDevice2->GetRenderState(D3DRENDERSTATE_ZFUNC, &d3d_value), DD_OK);
      if (d3d_value & D3DCMP_LESSEQUAL)
        mode |= CS_ZBUF_TEST;
      VERIFY_RESULT(m_lpd3dDevice2->GetRenderState(D3DRENDERSTATE_ZWRITEENABLE, &d3d_value), DD_OK);
      if (d3d_value & TRUE)
        mode |= CS_ZBUF_FILL;
      return mode;
    case G3DRENDERSTATE_DITHERENABLE:
      VERIFY_RESULT(m_lpd3dDevice->GetRenderState(D3DRENDERSTATE_DITHERENABLE, &d3d_value), DD_OK);
      return (d3d_value == TRUE)?true:false;
    case G3DRENDERSTATE_BILINEARMAPPINGENABLE:
      return rstate_bilinearmap;
    case G3DRENDERSTATE_TRILINEARMAPPINGENABLE:
      return rstate_trilinearmap;
    case G3DRENDERSTATE_TRANSPARENCYENABLE:
      return rstate_alphablend;
    case G3DRENDERSTATE_MIPMAPENABLE:
      return rstate_mipmap;
    case G3DRENDERSTATE_TEXTUREMAPPINGENABLE:
      return !rstate_flat;
    case G3DRENDERSTATE_GOURAUDENABLE:
      return rstate_gouraud;
    default:
      return 0;
  }
}

void csGraphics3DDirect3DDx5::SetClipper (csVector2* vertices, int num_vertices)
{
  delete m_pClipper;
  m_pClipper = NULL;
  if (!vertices) return;
  // @@@ This could be better! We are using a general polygon clipper
  // even in cases where a box clipper would be better. We should
  // have a special SetBoxClipper call in iGraphics3D.
  m_pClipper = new csPolygonClipper (vertices, num_vertices, false, true);
}

void csGraphics3DDirect3DDx5::OpenFogObject (CS_ID /*id*/, csFog* /*fog*/)
{
}

void csGraphics3DDirect3DDx5::DrawFogPolygon (CS_ID /*id*/,
  G3DPolygonDFP& /*poly*/, int /*fogtype*/)
{
}

void csGraphics3DDirect3DDx5::CloseFogObject (CS_ID /*id*/)
{
}
