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

// G3D_D3D.CPP
// csGraphics3DDirect3DDx6 implementation file
// Written by Dan Ogles
// Some modifications by Nathaniel Saint Martin

// Ported to COM by Dan Ogles on 8.26.98
// modified by Tristan McLure  09/08/1999
#define INITGUID

#include <windows.h>
#include <stdlib.h>
#include "ddraw.h"
#include "d3d.h"
#include "d3dcaps.h"

#include "sysdef.h"
#include "csutil/scf.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "cs3d/direct3d61/d3d_g3d.h"
#include "cs3d/direct3d61/d3d_txtcache.h"
#include "cs2d/ddraw61/IG2D.h"
#include "cssys/win32/iDDetect.h"
#include "csutil/inifile.h"
#include "qint.h"
#include "isystem.h"
#include "igraph3d.h"
#include "itxtmgr.h"
#include "itexture.h"
#include "ipolygon.h"
#include "icamera.h"
#include "ilghtmap.h"
#include "igraph2d.h"

#ifdef _MSC_VER
// get rid of evil annoying "variable may be used without having been init'ed"
#pragma warning(disable : 4701)
#endif

/***** File-scope variables *****/

static bool bGotTexDesc=false, bGotLitDesc=false, bGotHaloDesc=false;
static bool use32BitTexture=false, use16BitTexture=false;
static const float SCALE_FACTOR = 1.0f/2500.0f;

static const DWORD D3DFVF_TLVERTEX2 = (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX2);

struct D3DTLVERTEX2
{
    float sx, sy, sz;
    float rhw;
    D3DCOLOR color;
    D3DCOLOR specular;
    union
    {
        float tu;
        float tu1;
    };
    union
    {
        float tv;
        float tv1;
    };
    float tu2;
    float tv2;
};

//
// lame Utils
//

#ifdef PROC_INTEL
// This will only work for positive values.
// float cmp greater
inline bool FLOATCMPG(float a, float b)
{
  unsigned int A = *((unsigned int *) &a);
  unsigned int B = *((unsigned int *) &b);

  if (A > B)
    return true;

  return false;
}

// float cmp less
inline bool FLOATCMPL(float a, float b)
{
  unsigned int A = *((unsigned int *) &a);
  unsigned int B = *((unsigned int *) &b);

  if (A < B)
    return true;

  return false;
}

#else
#define FLOATCMPG(a, b) (a > b)
#define FLOATCMPL(a, b) (a < b)
#endif

#define CLAMP(a, b) (a = (FLOATCMPG(a, b) ? b : a))
#define CLAMPG(a, b) CLAMP(a, b)
#define CLAMPL(a, b) (a = (FLOATCMPL(a, b) ? b : a))

#define D3DRGB_(r, g, b) \
    (0xff000000L | ( (QInt(((r) * 255))) << 16) | ((QInt(((g) * 255))) << 8) | QInt(((b) * 255)))
#define D3DRGBA_(r, g, b, a) \
    (   ((QInt(((a) * 255))) << 24) | ((QInt(((r) * 255))) << 16) \
    |   ((QInt(((g) * 255))) << 8) | QInt(((b) * 255)) \
    )

/* ************************************************************** 
csGraphics3DDirect3DDx61 Class Definition
************************************************************** */

//
// Static member variables
//

DDSURFACEDESC2 csGraphics3DDirect3DDx6::m_ddsdTextureSurfDesc = { 0 };
DDSURFACEDESC2 csGraphics3DDirect3DDx6::m_ddsdLightmapSurfDesc = { 0 };
// have to work on the HALO-support ... will not be easy ... shit MS
DDSURFACEDESC2 csGraphics3DDirect3DDx6::m_ddsdHaloSurfDesc = { 0 };

//
// Interface table definition
//

IMPLEMENT_FACTORY (csGraphics3DDirect3DDx6)

EXPORT_CLASS_TABLE (d3ddx61)
  EXPORT_CLASS (csGraphics3DDirect3DDx6, "crystalspace.graphics3d.direct3d.dx61",
    "Direct3D DX6.1 3D graphics driver for Crystal Space")
EXPORT_CLASS_TABLE_END

IMPLEMENT_IBASE (csGraphics3DDirect3DDx6)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iGraphics3D)
IMPLEMENT_IBASE_END

//
// Implementation
//

// Z-Buffer format enumeration
// This will try to find a z-buffer with the bit-depth specified in "p_desired". If this
// bit-depth is zero, then the function uses the first available z-buffer format.
// @@@ Clearly, this can be enhanced
HRESULT CALLBACK csGraphics3DDirect3DDx6::EnumZBufferFormatsCallback(LPDDPIXELFORMAT lpddpf, LPVOID lpUserArg)
{
    DDPIXELFORMAT * p_desired = (DDPIXELFORMAT *)lpUserArg;

    if (p_desired->dwZBufferBitDepth == 0 || p_desired->dwZBufferBitDepth == lpddpf->dwZBufferBitDepth)
    {
        *p_desired = *lpddpf;
        return D3DENUMRET_CANCEL;
    }
    return D3DENUMRET_OK;
}

// Texture format enumeration
// due the fact that MS changed their DX6.x API I have to write a new Texture-format-enumerator
HRESULT CALLBACK csGraphics3DDirect3DDx6::EnumPixelFormatsCallback(LPDDPIXELFORMAT lpddpf, LPVOID lpUserArg)
{
  memset(lpUserArg, TRUE, sizeof(BOOL));

  // Search a halo texture format with alpha channel
  if (lpddpf->dwFlags & DDPF_ALPHAPIXELS)
  {
    if (lpddpf->dwRGBBitCount == 16
      && lpddpf->dwRGBAlphaBitMask == 0xF000
      && !bGotHaloDesc)
    {
      memcpy(&csGraphics3DDirect3DDx6::m_ddsdHaloSurfDesc.ddpfPixelFormat, lpddpf, sizeof(DDPIXELFORMAT));
      bGotHaloDesc = true;
    }

    if (lpddpf->dwRGBBitCount == 32
      && lpddpf->dwRGBAlphaBitMask == 0xFF000000
      && use32BitTexture)
    {
      memcpy(&csGraphics3DDirect3DDx6::m_ddsdHaloSurfDesc.ddpfPixelFormat, lpddpf, sizeof(DDPIXELFORMAT));
      bGotHaloDesc = true;
    }
  }
  // Search a 16/32 bits format
  else if(lpddpf->dwRGBBitCount >=16)
  {
    if (lpddpf->dwRGBBitCount == 16)
    {
      //Only use that format, if there has not been found annother format. (That might be the
      //32 Bit texture formtat if use32BitTexture is true)
      if(!bGotLitDesc)
      {
        memcpy(&csGraphics3DDirect3DDx6::m_ddsdLightmapSurfDesc.ddpfPixelFormat, lpddpf, sizeof(DDPIXELFORMAT));
        bGotLitDesc = true;
      }

      if(!bGotTexDesc)
      {
        memcpy(&csGraphics3DDirect3DDx6::m_ddsdTextureSurfDesc.ddpfPixelFormat, lpddpf, sizeof(DDPIXELFORMAT));
        bGotTexDesc = true;
      }
    }
    else if (lpddpf->dwRGBBitCount == 32)
    {
      if (use32BitTexture)
      {
        //This will override a potentially found 16 Bit texture mode:
        memcpy(&csGraphics3DDirect3DDx6::m_ddsdLightmapSurfDesc.ddpfPixelFormat, lpddpf, sizeof(DDPIXELFORMAT));
        bGotLitDesc = true;
        memcpy(&csGraphics3DDirect3DDx6::m_ddsdTextureSurfDesc.ddpfPixelFormat, lpddpf, sizeof(DDPIXELFORMAT));
        bGotTexDesc = true;
      }
    }
  }

  return D3DENUMRET_OK; // keep on looking
}

csGraphics3DDirect3DDx6::csGraphics3DDirect3DDx6(iBase *iParent) : 
  m_bIsHardware(false),
  m_bHaloEffect(false),
  m_dwDeviceBitDepth(0),
  m_hd3dBackMat(NULL),
  m_bIsLocked(false),
  m_iTypeLightmap(-1),
  m_pLightmapCache(NULL),
  m_lpD3D(NULL),
  m_lpd3dBackMat(NULL),
  m_pDirectDevice(NULL),
  m_lpd3dDevice(NULL),
  m_lpd3dDevice2(NULL),
  m_lpd3dViewport(NULL),
  m_lpDD4(NULL),
  m_lpddDevice(NULL),
  m_lpddPrimary(NULL),
  m_lpddZBuffer(NULL),
  m_pTextureCache(NULL),
  m_piSystem(NULL),
  m_bVerbose(true),
  m_nHeight(0),
  m_nHalfHeight(0),
  m_nWidth(0),
  m_nHalfWidth(0),
  m_mixmode(0),
  m_ZBufMode(CS_ZBUF_NONE),
  m_nDrawMode(0)
{
  CONSTRUCT_IBASE (iParent);

  // default
  m_Caps.ColorModel = G3DCOLORMODEL_RGB;
  m_Caps.CanClip = false;
  m_Caps.SupportsArbitraryMipMapping = false;
  m_Caps.BitDepth = 16;
  m_Caps.ZBufBitDepth = 32;
  m_Caps.minTexHeight = 2;
  m_Caps.minTexWidth = 2;
  m_Caps.maxTexHeight = 2048;
  m_Caps.maxTexWidth = 2048;
  m_Caps.PrimaryCaps.RasterCaps = G3DRASTERCAPS_SUBPIXEL;
  m_Caps.PrimaryCaps.canBlend = true;
  m_Caps.PrimaryCaps.ShadeCaps = G3DRASTERCAPS_LIGHTMAP;
  m_Caps.PrimaryCaps.PerspectiveCorrects = true;
  m_Caps.PrimaryCaps.FilterCaps = G3D_FILTERCAPS((int)G3DFILTERCAPS_NEAREST | (int)G3DFILTERCAPS_MIPNEAREST);
  m_Caps.fog = G3D_FOGMETHOD(0);

  m_MaxAspectRatio = 1; 

  zdist_mipmap1 = 12;
  zdist_mipmap2 = 24;
  zdist_mipmap3 = 40;

  rstate_dither = false;
  rstate_specular = false;
  rstate_bilinearmap = false;
  rstate_trilinearmap = true;
  rstate_gouraud = true;
  rstate_flat = true;
  rstate_alphablend = true;
  rstate_mipmap = true;

  m_gouraud = true;

  config = new csIniFile("Direct3DDX6.cfg");
}

csGraphics3DDirect3DDx6::~csGraphics3DDirect3DDx6()
{
  if (config)
    delete config;
  if (m_piG2D)
    m_piG2D->DecRef ();
  //if (m_piSystem)
  //  m_piSystem->DecRef ();
}

bool csGraphics3DDirect3DDx6::Initialize (iSystem *iSys)
{
  m_piSystem = iSys;
  m_piSystem->IncRef ();

  if (!m_piSystem->RegisterDriver ("iGraphics3D", this))
    return false;

  SysPrintf (MSG_INITIALIZATION, "\nDirect3DRender DX6.1 selected\n");

  m_piG2D = LOAD_PLUGIN (m_piSystem, "crystalspace.graphics2d.direct3d.dx61", iGraphics2D);
  if (!m_piG2D)
    return false;

  CHK (txtmgr = new csTextureManagerDirect3D (m_piSystem, m_piG2D));
  txtmgr->Initialize ();

  m_bVerbose = config->GetYesNo("Direct3DDX6", "VERBOSE", false);

  use16BitTexture = config->GetYesNo("Direct3DDX6","USE_16BIT_TEXTURE", false);
  if (use16BitTexture)
    use32BitTexture = config->GetYesNo("Direct3DDX6","EXTEND_32BIT_TEXTURE", false);  

  return true;
}

bool csGraphics3DDirect3DDx6::Open(const char* Title)
{
  LPD3DDEVICEDESC lpD3dDeviceDesc;
  DWORD dwDeviceMemType;
  DWORD dwZBufferMemType;
  DWORD dwZBufferBitDepth = 0;
  DDSURFACEDESC2 ddsd;
  HRESULT hRes;
  BOOL ddsdFound = FALSE;
  D3DMATERIAL d3dMaterial;
  bool bMipmapping;
  
  IDirectDraw4* lpDD4 = NULL;
  DDSCAPS2 ddsCaps;
//  DDCAPS_DX6 dd6caps;
  DWORD dwTotal, dwFree;
  D3DRECT rect;
  
  // Get the direct detection device.
  iGraphics2DDDraw6* pSysGInfo = QUERY_INTERFACE(m_piG2D, iGraphics2DDDraw6);
  ASSERT(pSysGInfo);

  pSysGInfo->SetFor3D(true);

  // Open the 2D driver.
  if (!m_piG2D->Open(Title))
    return false;
  
  pSysGInfo->GetDirectDetection(&m_pDirectDevice);
  ASSERT( m_pDirectDevice );
  
  pSysGInfo->GetDirectDrawDriver(&m_lpDD4);
  ASSERT(m_lpDD4);

  pSysGInfo->GetDirectDrawPrimary(&m_lpddPrimary);
  ASSERT(m_lpddPrimary);

  pSysGInfo->GetDirectDrawBackBuffer(&m_lpddDevice);
  ASSERT(m_lpddDevice);
  
  m_nWidth = m_piG2D->GetWidth();
  m_nHalfWidth = m_nWidth/2;
  
  m_nHeight = m_piG2D->GetHeight();
  m_nHalfHeight = m_nHeight/2;
  
  // get the amount of texture memory on the video card.
  hRes = m_lpDD4->QueryInterface(IID_IDirectDraw4, (LPVOID*)&lpDD4);
  if ( FAILED(hRes) )
    return false;
    
  memset(&ddsCaps, 0, sizeof(ddsCaps));

  ddsCaps.dwCaps = DDSCAPS_TEXTURE;
  hRes = lpDD4->GetAvailableVidMem(&ddsCaps, &dwTotal, &dwFree);
  // dwTotal = dd6caps.dwVidMemTotal;
  // dwFree = dd6caps.dwVidMemFree;
  
  SysPrintf (MSG_INITIALIZATION, " %d bytes VideoMem Total \n", dwTotal);
  SysPrintf (MSG_INITIALIZATION, " %d bytes VideoMem Free \n", dwFree);

  FINAL_RELEASE (lpDD4);
  
  if ( FAILED(hRes) )
  { 
    SysPrintf (MSG_FATAL_ERROR, "Error in: 'GetAvailableVidMem' ! \n");
    return false;
  }
  // get direct3d interface
  
  hRes = m_lpDD4->QueryInterface(IID_IDirect3D3, (LPVOID *)&m_lpD3D);
  if(FAILED(hRes))
  {
    SysPrintf (MSG_FATAL_ERROR, "Query for 'IID_IDirect3D3' failed! \n");
    return false;
  }

  memset(&ddsd, 0, sizeof(ddsd));
  ddsd.dwSize = sizeof(ddsd);
  hRes = m_lpddPrimary->GetSurfaceDesc(&ddsd);
  if( FAILED(hRes) )
  {
    SysPrintf (MSG_FATAL_ERROR, "Error in 'GetSurfaceDesc' ! \n");
    return false;
  }

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
  
  SysPrintf (MSG_INITIALIZATION, " %d-bit Colordepth selected\n", ddsd.ddpfPixelFormat.dwRGBBitCount);
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
    SysPrintf (MSG_FATAL_ERROR, "No Z-Buffer ! \n");
    return false;
  }

  // Get the pixel-format of the rendering target, because the z-buffer should
  // have the same bit-depth, if possible.
  DDPIXELFORMAT ddpfTest;
  memset(&ddpfTest, 0, sizeof(ddpfTest));
  ddpfTest.dwSize = sizeof(ddpfTest);

  hRes = m_lpddDevice->GetPixelFormat(&ddpfTest);
  if (FAILED(hRes))
  {
    SysPrintf (MSG_FATAL_ERROR, " Cannot get pixel-format of rendering target. ! \n");
    return false;   
  }

  dwZBufferBitDepth = ddpfTest.dwRGBBitCount;
  SysPrintf (MSG_INITIALIZATION, " %d-bit Z-Buffer depth selected\n", dwZBufferBitDepth);
 
  m_Caps.ZBufBitDepth = dwZBufferBitDepth;
  m_Caps.minTexHeight = lpD3dDeviceDesc->dwMinTextureHeight;
  m_Caps.minTexWidth  = lpD3dDeviceDesc->dwMinTextureWidth;
  m_Caps.maxTexHeight = lpD3dDeviceDesc->dwMaxTextureHeight;
  m_Caps.maxTexWidth  = lpD3dDeviceDesc->dwMaxTextureWidth;

  if (lpD3dDeviceDesc->dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY)
  {
    SysPrintf (MSG_INITIALIZATION, " Warning: Your Direct3D Device supports only square textures!\n");
    SysPrintf (MSG_INITIALIZATION, "          This is a potential performance hit!\n");
    m_MaxAspectRatio = 1;
  }
  else
  {
    SysPrintf (MSG_INITIALIZATION, "Your Direct3D Device also supports non square textures - that's good.\n");
    m_MaxAspectRatio = 32768;
  }

  memset(&ddsd, 0, sizeof(ddsd));
  ddsd.dwSize = sizeof(ddsd);
  ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT; // | DDSD_ZBUFFERBITDEPTH;
  ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | dwZBufferMemType;
  ddsd.dwWidth = m_nWidth;
  ddsd.dwHeight = m_nHeight;

  // Find a good z-buffer format
  ddsd.ddpfPixelFormat.dwSize = 0;

  // Try to find a z-buffer format with the same bit-depth as the rendering target
  ddsd.ddpfPixelFormat.dwZBufferBitDepth = dwZBufferBitDepth;
  hRes = m_lpD3D->EnumZBufferFormats(m_Guid, EnumZBufferFormatsCallback, &ddsd.ddpfPixelFormat);
  if (SUCCEEDED(hRes) && ddsd.ddpfPixelFormat.dwSize == 0)
  {
      // Try to find any other z-buffer format
      ddsd.ddpfPixelFormat.dwZBufferBitDepth = 0;
      hRes = m_lpD3D->EnumZBufferFormats(m_Guid, EnumZBufferFormatsCallback, &ddsd.ddpfPixelFormat);
  }

  if (FAILED(hRes) || ddsd.ddpfPixelFormat.dwSize == 0)
  {
    SysPrintf (MSG_FATAL_ERROR, " Z-Buffer enumeration failed. ! \n");
    return false;   
  }
 
  SysPrintf (MSG_INITIALIZATION, " Resolution: %d x %d selected\n", ddsd.dwWidth, ddsd.dwHeight);
 
 // ddsd.dwZBufferBitDepth = dwZBufferBitDepth;
 // dwZBufferBitDepth is not longer part of ddsd under DX6.x
 // use ddsd.ddpfPixelFormat.dwZBufferBitDepth = dwZBufferBitDepth instead

  hRes = m_lpDD4->CreateSurface(&ddsd, &m_lpddZBuffer, NULL);
  if (FAILED(hRes))
  {
    SysPrintf (MSG_FATAL_ERROR, " Error creating Z-Buffer, 'CreateSurface' failed ! \n");
    return false;   
  }

  hRes = m_lpddDevice->AddAttachedSurface(m_lpddZBuffer);
  if (FAILED(hRes))
    return false; 

  if(m_bVerbose)
    SysPrintf (MSG_INITIALIZATION, " Use %d depth for ZBuffer.\n", dwZBufferBitDepth);

  // get the device interface
  hRes = m_lpD3D->CreateDevice(m_Guid, m_lpddDevice, &m_lpd3dDevice2, NULL);
  if (FAILED(hRes))
    return false;        
  
  // get the texture format we want
  // here is a massive bug ... need help
  m_lpd3dDevice2->EnumTextureFormats(EnumPixelFormatsCallback, (LPVOID)&ddsdFound);
  
  if (bGotLitDesc && !bGotTexDesc)
  {
    memcpy(&m_ddsdTextureSurfDesc, &m_ddsdLightmapSurfDesc, sizeof(DDSURFACEDESC2));
    bGotTexDesc = true;
  }
  
  if (bGotTexDesc && !bGotLitDesc)
  {
    memcpy(&m_ddsdLightmapSurfDesc, &m_ddsdTextureSurfDesc, sizeof(DDSURFACEDESC2));
    bGotLitDesc = true;
  }

  if (!bGotTexDesc && !bGotLitDesc)
  {
    SysPrintf (MSG_INITIALIZATION, " ERROR : No 16 or 32 bits texture format supported in hardware.\n");
    hRes = E_FAIL;
    return false;
  }    

  if(m_bVerbose)
  {
    if(m_ddsdTextureSurfDesc.ddpfPixelFormat.dwRGBBitCount==16)
      SysPrintf (MSG_INITIALIZATION, " Using 16-bit texture format.\n");
    else
      SysPrintf (MSG_INITIALIZATION, " Using 32-bit texture format.\n");
  }

  // select type of lightmapping
  if (m_pDirectDevice->GetAlphaBlend() && !config->GetYesNo("Direct3DDX6","DISABLE_LIGHTMAP", false))
  {
    if (m_pDirectDevice->GetAlphaBlendType() == 1)
    {
      m_iTypeLightmap = 1;
    }
    else
    {
      SysPrintf (MSG_INITIALIZATION, " WARNING : Bad lightmapping supported.\n");
      m_iTypeLightmap = 2;
    }
  }
  else
  {
    if(config->GetYesNo("Direct3DDX6","DISABLE_LIGHTMAP", false))
      SysPrintf (MSG_INITIALIZATION, " WARNING : Lightmapping disabled by user.\n");
    else
      SysPrintf (MSG_INITIALIZATION, " WARNING : Lightmapping not supported by hardware.\n");
    m_iTypeLightmap = 0;
  }

  if(!m_iTypeLightmap)
  {
    SysPrintf (MSG_INITIALIZATION, " WARNING : Lightmapping disabled.\n");
  }
  else
  {
    if(m_bVerbose)
    {
      if(m_ddsdLightmapSurfDesc.ddpfPixelFormat.dwRGBBitCount==8)
        SysPrintf (MSG_INITIALIZATION, " Using 8-bit palettized format for lightmap memory.\n");
      else if(m_ddsdLightmapSurfDesc.ddpfPixelFormat.dwRGBBitCount==16)
        SysPrintf (MSG_INITIALIZATION, " Using 16-bit lightmap format.\n");
      else
        SysPrintf (MSG_INITIALIZATION, " Using 32-bit lightmap format.\n");
    }
  }

  // set Halo effect configuration
  if (m_pDirectDevice->GetAlphaBlendHalo() && !config->GetYesNo("Direct3DDX6","DISABLE_HALO", false))
    m_bHaloEffect = true;
  else
  {
    if(m_pDirectDevice->GetAlphaBlendHalo() && config->GetYesNo("Direct3DDX6","DISABLE_HALO", false))
      SysPrintf (MSG_INITIALIZATION, " WARNING : Halo effect disabled by user.\n");
    else
      SysPrintf (MSG_INITIALIZATION, " WARNING : Halo effect not supported by hardware.\n");
    m_bHaloEffect = false;
  }

  if (!bGotHaloDesc && m_bHaloEffect)
  {
    SysPrintf (MSG_INITIALIZATION, " WARNING : No halo texture format supported by hardware.\n");
    m_bHaloEffect = false;
  }

  if(!m_bHaloEffect)
  {
    SysPrintf (MSG_INITIALIZATION, " WARNING : Halo effect disabled.\n");
  }
  else
  {
    if(m_bVerbose)
      SysPrintf (MSG_INITIALIZATION, " Using %d-bit format for halo effect\n",
        m_ddsdHaloSurfDesc.ddpfPixelFormat.dwRGBBitCount);
  }

  if(m_bVerbose)
  {
    SysPrintf (MSG_INITIALIZATION, " Using 24-bit internal format for texture\n");
  }

  // set mipmapping configuration
  if (m_pDirectDevice->GetMipmap() && !config->GetYesNo("Direct3DDX6","DISABLE_MIPMAP", false))
  {  bMipmapping = true;
     SysPrintf (MSG_INITIALIZATION, " Mipmapping enabled and supported\n");
  }
  else
  {
    if(config->GetYesNo("Direct3DDX6","DISABLE_MIPMAP", false)
      && m_pDirectDevice->GetMipmap())
      SysPrintf (MSG_INITIALIZATION, " WARNING : Mipmapping disabled by user.\n");
    else
      SysPrintf (MSG_INITIALIZATION, " WARNING : Mipmapping not supported in hardware.\n");
    bMipmapping = false;
  }
  if(!bMipmapping)
  {
    SysPrintf (MSG_INITIALIZATION, " WARNING : Mipmapping disabled.\n");
  }

  // create a black background
  hRes = m_lpD3D->CreateMaterial(&m_lpd3dBackMat, NULL);
  if (FAILED(hRes))
  {
    SysPrintf (MSG_INITIALIZATION, " Error creating Backgroundmaterial!\n");
    return false;
  }
  memset(&d3dMaterial, 0, sizeof(d3dMaterial));
  d3dMaterial.dwSize = sizeof(d3dMaterial);
  d3dMaterial.dwRampSize = 1;
  
  hRes = m_lpd3dBackMat->SetMaterial(&d3dMaterial);
  if (FAILED(hRes))
  {
    SysPrintf (MSG_INITIALIZATION, "Error setting Backgroundmaterial!\n");
    return false;     
  }

  hRes = m_lpd3dBackMat->GetHandle(m_lpd3dDevice2, &m_hd3dBackMat);
  if (FAILED(hRes))
  {
    SysPrintf (MSG_INITIALIZATION, "Error getting handle for Backgroundmaterial!\n"); 
    return false;
  }
  // create the viewport
  
  hRes = m_lpD3D->CreateViewport(&m_lpd3dViewport, NULL);
  if (FAILED(hRes))
  {
    SysPrintf (MSG_INITIALIZATION, "Error creating viewport!\n");
    return false;
  }
  
  // assign the viewport
  
  hRes = m_lpd3dDevice2->AddViewport(m_lpd3dViewport);
  if (FAILED(hRes))
  {
    SysPrintf (MSG_INITIALIZATION, "Error assigning viewport!\n");
    return false;
  }
  // assign the background to the viewport
  
  hRes = m_lpd3dViewport->SetBackground(m_hd3dBackMat);
  if (FAILED(hRes))
  {
    SysPrintf (MSG_INITIALIZATION, "Error assigning background to viewport!\n");
    return false;
  }
  // set default render-states.
  m_lpd3dDevice2->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
  m_lpd3dDevice2->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE);
  m_lpd3dDevice2->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE);
  
  hRes = m_lpd3dDevice2->SetRenderState(D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_LINEAR);
  if (FAILED(hRes))
    m_lpd3dDevice2->SetRenderState(D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_NEAREST);
  
  m_lpd3dDevice2->SetRenderState(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEARMIPLINEAR);
  if (FAILED(hRes))
  {
    hRes = m_lpd3dDevice2->SetRenderState(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_MIPLINEAR);
    if (FAILED(hRes))
      m_lpd3dDevice2->SetRenderState(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEAR);
  }
  
  if(config->GetYesNo("Direct3DDX6","ENABLE_DITHER", true))
    m_lpd3dDevice2->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE);
  else
    m_lpd3dDevice2->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE);
  
  m_lpd3dDevice2->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
  m_lpd3dDevice2->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);

  // Set default Z-buffer mode.
  m_lpd3dDevice2->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
  m_lpd3dDevice2->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);
  m_ZBufMode = CS_ZBUF_FILL;
  
  // save half of the memory for textures,
  // half for lightmaps
  SysPrintf (MSG_INITIALIZATION, "Initializing lightmap- and texturecache...");
  // Here is the "Assertion failed!"-bug I get on my machine
  if (m_iTypeLightmap != 0)
  {
    CHK (m_pTextureCache = new D3DTextureCache(dwFree/2, m_bIsHardware, m_lpDD4, m_lpd3dDevice2, m_ddsdTextureSurfDesc.ddpfPixelFormat.dwRGBBitCount, bMipmapping, &m_Caps, m_MaxAspectRatio));
    CHK (m_pLightmapCache = new D3DLightMapCache(dwFree/2, m_bIsHardware, m_lpDD4, m_lpd3dDevice2, m_ddsdLightmapSurfDesc.ddpfPixelFormat.dwRGBBitCount));
 
  }
  else
  {
    CHK (m_pTextureCache = new D3DTextureCache(dwFree, m_bIsHardware, m_lpDD4, m_lpd3dDevice2, m_ddsdTextureSurfDesc.ddpfPixelFormat.dwRGBBitCount, bMipmapping, &m_Caps, m_MaxAspectRatio));
    m_pLightmapCache = NULL;
  }
  
  SysPrintf (MSG_INITIALIZATION, "completed!\n");

  // init the viewport.
 
  SetDimensions(m_nWidth, m_nHeight);
  SysPrintf (MSG_INITIALIZATION, "SetDimensions succesfull!\n");

  // clear the Z-buffer    
  rect.x1 = 0; rect.y1 = 0; 
  rect.x2 = m_nWidth; rect.y2 = m_nHeight;
  
  hRes = m_lpd3dViewport->Clear(1, &rect, D3DCLEAR_ZBUFFER);
  if (FAILED(hRes))
    return false;

  ConfigureRendering();
  // init render state cache
  m_States.Initialize(m_lpd3dDevice2, m_piSystem);
  // the buffer size could be potentially tweaked for better performance
  m_VertexCache.Initialize(m_lpd3dDevice2, 32);
  // set to false if we want to revert on "older" PolygonFX (supports fogging)
  m_bBatchPolygonFX = true;

  return true;
}

void csGraphics3DDirect3DDx6::Close()
{
  m_VertexCache.ShutDown();

  ClearCache();
  
  FINAL_RELEASE(m_lpd3dBackMat);
  FINAL_RELEASE(m_lpd3dViewport);
  FINAL_RELEASE(m_lpd3dDevice2);
  FINAL_RELEASE(m_lpd3dBackMat);
  FINAL_RELEASE(m_lpd3dViewport);
  FINAL_RELEASE(m_lpD3D);
}

// TODO/FIXME : - implement some of this into DirectDetection
//        - add user settings control
//        - maybe keep render state tables as static and only
//          only store settings as index into table
// This makes obsolete alot of initialization done in ::Open()
void csGraphics3DDirect3DDx6::ConfigureRendering()
{  
  LPD3DDEVICEDESC lpD3dDeviceDesc = m_pDirectDevice->GetDesc3D();

  // single pass multitexturing detection
  if (lpD3dDeviceDesc->wMaxSimultaneousTextures >= 2)
    m_bMultiTexture = true;
  else
    m_bMultiTexture = false;

  // start up with minimum settings
  m_bRenderTransparent = false;
  m_bRenderLightmap = false;

  // set filters  
  m_lpd3dDevice2->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
  m_lpd3dDevice2->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFG_POINT);
  m_lpd3dDevice2->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFG_POINT);

  if (lpD3dDeviceDesc->dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR)
    m_lpd3dDevice2->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);

  if (lpD3dDeviceDesc->dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR)
    m_lpd3dDevice2->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFG_LINEAR);

  if (lpD3dDeviceDesc->dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR)
    m_lpd3dDevice2->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFG_LINEAR);

  // FIXME : DX documentation states that some hardware only support
  //       D3DTOP_SELECTARG1 for the first stage, strange
  m_lpd3dDevice2->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
  m_lpd3dDevice2->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
  m_lpd3dDevice2->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
  m_lpd3dDevice2->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
  m_lpd3dDevice2->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

  m_lpd3dDevice2->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
  m_lpd3dDevice2->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);

  m_lpd3dDevice2->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);
  m_lpd3dDevice2->SetTextureStageState(1, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);

  //  Dest*Alpha + (srcTex*srcLightmap)*InvAlpha
  //  (Dest*Alpha + srcTex*InvAlpha)*srcLightmap

    //.dpcTriCaps
  if (m_bMultiTexture)
  {
    bool FoundGoodTextureOp = false;

    // set default states for multitexturing

    // more filters
    m_lpd3dDevice2->SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTFG_POINT);
    m_lpd3dDevice2->SetTextureStageState(1, D3DTSS_MINFILTER, D3DTFG_POINT);
    m_lpd3dDevice2->SetTextureStageState(1, D3DTSS_MIPFILTER, D3DTFG_POINT);

    if (lpD3dDeviceDesc->dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR)
      m_lpd3dDevice2->SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTFG_LINEAR);

    if (lpD3dDeviceDesc->dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR)
      m_lpd3dDevice2->SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTFG_LINEAR);

    if (lpD3dDeviceDesc->dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR) 
      m_lpd3dDevice2->SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTFG_LINEAR);

    m_lpd3dDevice2->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    m_lpd3dDevice2->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
    m_lpd3dDevice2->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
    m_lpd3dDevice2->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

    struct _TexOpList {
    DWORD Flag;
    D3DTEXTUREOP Op;
    } TexOpList [] = {D3DTEXOPCAPS_MODULATE2X,  D3DTOP_MODULATE2X,
              D3DTEXOPCAPS_MODULATE,  D3DTOP_MODULATE,
              D3DTEXOPCAPS_ADDSIGNED, D3DTOP_ADDSIGNED2X,
              D3DTEXOPCAPS_ADD,     D3DTOP_ADD,
    };

    m_LightmapTextureOp = D3DTOP_DISABLE;

    // NOTE : Im not too sure what's the proper way of validating
    //        this stuff, maybe Im doing something wrong.

    for (int i = 0; i < sizeof(TexOpList)/sizeof(_TexOpList); i++)
    {
      if (lpD3dDeviceDesc->dwTextureOpCaps & TexOpList[i].Flag)
      {
        m_lpd3dDevice2->SetTextureStageState(1, D3DTSS_COLOROP, TexOpList[i].Op);

        DWORD NumPass;
        HRESULT hResult = m_lpd3dDevice2->ValidateDevice(&NumPass);

        switch (hResult)
        {
          case D3D_OK :
            FoundGoodTextureOp = true;
          break;
          case DDERR_INVALIDOBJECT :
          case DDERR_INVALIDPARAMS :
          case D3DERR_TOOMANYOPERATIONS :
          case D3DERR_UNSUPPORTEDALPHAARG :
          case D3DERR_UNSUPPORTEDALPHAOPERATION :
          case D3DERR_UNSUPPORTEDCOLORARG :
          case D3DERR_UNSUPPORTEDCOLOROPERATION :
          break;

          // none of this should be happening ...
          // except maybe conflicting texture filter
          case D3DERR_CONFLICTINGTEXTUREFILTER :
          case D3DERR_CONFLICTINGTEXTUREPALETTE :
          case D3DERR_UNSUPPORTEDFACTORVALUE :
          case D3DERR_UNSUPPORTEDTEXTUREFILTER :
          case D3DERR_WRONGTEXTUREFORMAT :
          default :
          break;
        }

        if (FoundGoodTextureOp)
        {
          m_bRenderLightmap = true;
          m_LightmapTextureOp = TexOpList[i].Op;
          break;
        }

      } // end of if(Flag)
    } // end of for()

    if (!FoundGoodTextureOp)
    {
      SysPrintf(MSG_INITIALIZATION, "WARNING : Cant find any decent lightmap TextureOp for stage1\n");
      // fall back upon lame multipass
      m_bMultiTexture = false;
    }

    m_lpd3dDevice2->SetTextureStageState(1, D3DTSS_COLOROP, m_LightmapTextureOp);
    } // end of if (m_bMultiTexture)

  // Multipass'ing
  if (!m_bMultiTexture)
  {
    m_lpd3dDevice2->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

    struct _BlendList {
      DWORD SrcFlag, DstFlag;
      D3DBLEND Src, Dst;
    } BlendList[] = {{D3DPBLENDCAPS_DESTCOLOR, D3DPBLENDCAPS_SRCCOLOR,  D3DBLEND_DESTCOLOR, D3DBLEND_SRCCOLOR},
             {D3DPBLENDCAPS_DESTCOLOR, D3DPBLENDCAPS_ZERO,    D3DBLEND_DESTCOLOR, D3DBLEND_ZERO},
             {D3DPBLENDCAPS_ONE, D3DPBLENDCAPS_ONE,       D3DBLEND_ONE, D3DBLEND_ONE},
             {D3DPBLENDCAPS_ONE, D3DPBLENDCAPS_SRCCOLOR,    D3DBLEND_ONE, D3DBLEND_SRCCOLOR},
             };

    for (int i = 0; i < sizeof(BlendList)/sizeof(_BlendList); i++)
    {
      if ((lpD3dDeviceDesc->dpcTriCaps.dwSrcBlendCaps & BlendList[i].SrcFlag) &&
        (lpD3dDeviceDesc->dpcTriCaps.dwDestBlendCaps & BlendList[i].DstFlag))
      {
        m_bRenderLightmap = true;
        m_LightmapSrcBlend = BlendList[i].Src;
        m_LightmapDstBlend = BlendList[i].Dst;
        break;
      }
    }

  } // end of if (!m_bMultiTexture)

//--- transparency settings ---

  // hum, this code looks familiar eh
  struct _TransList {
    DWORD SrcFlag, DstFlag;
    D3DBLEND Src, Dst;
  } TransList[] = {{D3DPBLENDCAPS_INVSRCALPHA, D3DPBLENDCAPS_SRCALPHA, D3DBLEND_INVSRCALPHA, D3DBLEND_SRCALPHA},
                   {D3DPBLENDCAPS_SRCALPHA, D3DPBLENDCAPS_INVSRCALPHA, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA},
                   };
  // TODO : store translucency type
  for (int i = 0; i < sizeof(TransList)/sizeof(_TransList); i++)
  {
    if ((lpD3dDeviceDesc->dpcTriCaps.dwSrcBlendCaps & TransList[i].SrcFlag) &&
        (lpD3dDeviceDesc->dpcTriCaps.dwDestBlendCaps & TransList[i].DstFlag))
    {
      m_bRenderTransparent = true;

      m_TransSrcBlend = TransList[i].Src;
      m_TransDstBlend = TransList[i].Dst;
      break;
    }
  }

} // end of void csGraphics3DDirect3DDx6::ConfigureRendering()

void csGraphics3DDirect3DDx6::SetDimensions(int nWidth, int nHeight)
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
  
  if (FAILED(m_lpd3dDevice2->SetCurrentViewport(m_lpd3dViewport)))
    return;
}

bool csGraphics3DDirect3DDx6::BeginDraw (int nDrawFlags)
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
      
      hRes = m_lpd3dDevice2->BeginScene();
      if (FAILED(hRes))
        return E_FAIL;
      
    } 
  } 
  
  m_nDrawMode = nDrawFlags;
  
  return true;
}

void csGraphics3DDirect3DDx6::FinishDraw ()
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
    
    hRes = m_lpd3dDevice2->EndScene();
    if (FAILED(hRes))
      return;
  }
  m_nDrawMode = 0;
}

void csGraphics3DDirect3DDx6::SetZBufMode(G3DZBufMode mode)
{
  if (mode==m_ZBufMode) 
    return;
  
  m_ZBufMode = mode;
  
  if (mode == CS_ZBUF_TEST)
    VERIFY_RESULT( m_lpd3dDevice2->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE), DD_OK );
  else
    VERIFY_RESULT( m_lpd3dDevice2->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE), DD_OK );
  
  if (mode == CS_ZBUF_FILL)      // write-only
  m_States.SetZFunc(D3DCMP_ALWAYS);
  else 
    m_States.SetZFunc(D3DCMP_LESSEQUAL);
}


G3D_COLORMAPFORMAT csGraphics3DDirect3DDx6::GetColormapFormat() 
{
  return G3DCOLORFORMAT_24BIT;
}

void csGraphics3DDirect3DDx6::DumpCache()
{
  m_pTextureCache->Dump();
  m_pLightmapCache->Dump();
}

void csGraphics3DDirect3DDx6::ClearCache()
{
  if ( m_pTextureCache ) 
  {
    m_pTextureCache->Clear();
  }
  if ( m_pLightmapCache )
  {
    m_pLightmapCache->Clear();
  }
}

void csGraphics3DDirect3DDx6::CacheTexture(iPolygonTexture *texture)
{
  iTextureHandle* txt_handle = texture->GetTextureHandle ();
  m_pTextureCache->Add (txt_handle);
  m_pLightmapCache->Add (texture);
}

void csGraphics3DDirect3DDx6::UncacheTexture(iPolygonTexture *texture)
{
  (void)texture;
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

void csGraphics3DDirect3DDx6::MultitextureDrawPolygon(G3DPolygonDP & poly)
{
  ASSERT( m_lpd3dDevice2 );

  bool bLightmapExists = true,
  bColorKeyed = false,
  bTransparent;
  
  int  poly_alpha;
  
  float J1, J2, J3, K1, K2, K3;
  float M, N, O;
  int i;

  csHighColorCacheData* pTexCache   = NULL;
  csHighColorCacheData* pLightCache = NULL;
  D3DTLVERTEX2 vx;

  float z;

  if (poly.num < 3) 
    return;

  // set up the geometry.
  SetupPolygon( poly, J1, J2, J3, K1, K2, K3, M, N, O );

  poly_alpha = poly.alpha;
  if ((poly_alpha > 0) && m_bRenderTransparent)
    bTransparent = true;
  else
    bTransparent = false;

  //Mipmapping is done for us by DirectX, so we will always select the texture with
  //the highest resolution.
  iPolygonTexture *pTex = poly.poly_texture[0];

  if (!pTex)
    return;

  // cache the texture and initialize rasterization.
  CacheTexture (pTex);

  // retrieve the texture from the cache by handle.
  csTextureMMDirect3D* txt_mm = (csTextureMMDirect3D*)poly.txt_handle->GetPrivateObject ();
  pTexCache = txt_mm->GetHighColorCacheData ();

  bColorKeyed = txt_mm->GetTransparent ();

  // retrieve the lightmap from the cache.
  iLightMap* piLM = pTex->GetLightMap ();

  if ( piLM  && m_bRenderLightmap)
  {
    pLightCache = piLM->GetHighColorCache ();
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

  if ( bTransparent )
  {
    m_States.SetAlphaBlendEnable(true);
    m_States.SetDstBlend(m_TransDstBlend);
    m_States.SetSrcBlend(m_TransSrcBlend);
  }
  else
  {
    m_States.SetAlphaBlendEnable(false);
  }

  if ( bColorKeyed )
  {
    m_States.SetColorKeyEnable(true);
  }
  else
  {
    m_States.SetColorKeyEnable(false);
  }

  D3DTextureCache_Data *pD3D_texcache = (D3DTextureCache_Data *)pTexCache->pData;

  m_States.SetTexture(0, pD3D_texcache->lptex);

  float lightmap_scale_u, lightmap_scale_v;
  float lightmap_low_u, lightmap_low_v;
  if (bLightmapExists )
  {
    // set lightmap stuff
    iLightMap *thelightmap = pTex->GetLightMap ();

    int lmwidth = thelightmap->GetWidth ();
    int lmrealwidth = thelightmap->GetRealWidth ();
    int lmheight = thelightmap->GetHeight ();
    int lmrealheight = thelightmap->GetRealHeight ();
    float scale_u = (float)(lmrealwidth-1) / (float)lmwidth;
    float scale_v = (float)(lmrealheight-1) / (float)lmheight;

    float lightmap_high_u, lightmap_high_v;
    pTex->GetTextureBox(lightmap_low_u,lightmap_low_v,
                       lightmap_high_u,lightmap_high_v);

    lightmap_scale_u = scale_u / (lightmap_high_u - lightmap_low_u), 
    lightmap_scale_v = scale_v / (lightmap_high_v - lightmap_low_v);

    m_States.SetTexture(1, ((D3DLightCache_Data *)pLightCache->pData)->lptex);
    m_States.SetStageState(1, D3DTSS_COLOROP, m_LightmapTextureOp);
  }
  else
  {
    m_States.SetStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
  }

  D3DCOLOR vertex_color;

  if (bTransparent)
  {
    vertex_color = RGBA_MAKE(0xFF, 0xFF, 0xFF, QInt(poly_alpha * (255.0f / 100.0f)));
  }
  else
  {
  // TODO : turn off color component interpolation if no transparency
    vertex_color = RGBA_MAKE(0xFF, 0xFF, 0xFF, 0xFF);
  }

  VERIFY_RESULT(m_lpd3dDevice2->Begin(D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX2,
                D3DDP_DONOTUPDATEEXTENTS), DD_OK);  

  // render texture-mapped poly
  for (i=0; i < poly.num; i++)
  {
    float sx = poly.vertices[i].sx - m_nHalfWidth;
    float sy = poly.vertices[i].sy - m_nHalfHeight;
    float u_over_sz = (J1 * sx + J2 * sy + J3);
    float v_over_sz = (K1 * sx + K2 * sy + K3);
    float one_over_sz = (M * sx + N * sy + O);

    z = 1.0f  / one_over_sz;

    vx.sx = poly.vertices[i].sx;
    vx.sy = m_nHeight-poly.vertices[i].sy;

    vx.sz = z*(float)SCALE_FACTOR;

      if(vx.sz>0.9999)
        vx.sz=0.9999;

    vx.color = vertex_color;
    vx.rhw = one_over_sz;

    vx.specular = 0;
    vx.tu = u_over_sz * z;
    vx.tv = v_over_sz * z;

    if (bLightmapExists)
    {
      vx.tu2 = (vx.tu - lightmap_low_u) * lightmap_scale_u;
      vx.tv2 = (vx.tv - lightmap_low_v) * lightmap_scale_v;
    }

    VERIFY_RESULT(m_lpd3dDevice2->Vertex( &vx ), DD_OK);
  }

  m_lpd3dDevice2->End(0);

  // If there is vertex fog then we apply that last.
  if (poly.use_fog)
  {
    m_States.SetAlphaBlendEnable(true);
    m_States.SetZFunc(D3DCMP_LESSEQUAL);
    m_States.SetDstBlend(D3DBLEND_SRCALPHA);
    m_States.SetSrcBlend(D3DBLEND_INVSRCALPHA);
    m_States.SetTexture(0, NULL);
    m_States.SetStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

    VERIFY_RESULT(m_lpd3dDevice2->Begin(D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX, D3DDP_DONOTUPDATEEXTENTS), DD_OK);

    for (i=0; i<poly.num; i++)
    {
      D3DTLVERTEX vx;
      float sx = poly.vertices[i].sx - m_nHalfWidth;
      float sy = poly.vertices[i].sy - m_nHalfHeight;
      float one_over_sz = (M * sx + N * sy + O);

      z = 1.0f  / one_over_sz;

      vx.sx = poly.vertices[i].sx;
      vx.sy = m_nHeight-poly.vertices[i].sy;
      vx.sz = z*(float)SCALE_FACTOR;

//      if(vx.sz>0.9999)
//        vx.sz=0.9999;

      vx.rhw = one_over_sz;

    float I = poly.fog_info[i].intensity;
    CLAMPG(I, 1.0f);
    I = 1.0f - I;

      float r = poly.fog_info[i].r > 1.0f ? 1.0f : poly.fog_info[i].r;
      float g = poly.fog_info[i].g > 1.0f ? 1.0f : poly.fog_info[i].g;
      float b = poly.fog_info[i].b > 1.0f ? 1.0f : poly.fog_info[i].b;

      vx.color    = D3DRGBA_(r, g, b, I);

      vx.specular = 0;

      VERIFY_RESULT(m_lpd3dDevice2->Vertex( &vx ), DD_OK);
    }
    VERIFY_RESULT(m_lpd3dDevice2->End(0), DD_OK);
  }
} // end of ::MultitextureDrawPolygon


// TODO : Vertex fogging should be implemented through the specular component,
//      no need for an additional pass.
void csGraphics3DDirect3DDx6::DrawPolygon (G3DPolygonDP& poly)
{
  ASSERT( m_lpd3dDevice2 );

  if (m_bMultiTexture)
  {
  MultitextureDrawPolygon(poly);
  return;
  }

  bool bLightmapExists = true,
  bColorKeyed = false,
  bTransparent;
  
  int  poly_alpha;
  
  float J1, J2, J3, K1, K2, K3;
  float M, N, O;
  int i;

  csHighColorCacheData* pTexCache   = NULL;
  csHighColorCacheData* pLightCache = NULL;
  // NOTE : is it better to have this as a class member?
  //    or use d3d vertex buffers
  D3DTLVERTEX vx[100];

  float z;

  if (poly.num < 3) 
    return;

  // set up the geometry.
  SetupPolygon( poly, J1, J2, J3, K1, K2, K3, M, N, O );

  poly_alpha = poly.alpha;
  if ((poly_alpha > 0) && m_bRenderTransparent)
    bTransparent = true;
  else
    bTransparent = false;

  //Mipmapping is done for us by DirectX, so we will always select the texture with
  //the highest resolution.
  iPolygonTexture *pTex = poly.poly_texture[0];

  if (!pTex)
    return;

  // cache the texture and initialize rasterization.
  CacheTexture (pTex);

  // retrieve the texture from the cache by handle.
  csTextureMMDirect3D* txt_mm = (csTextureMMDirect3D*)poly.txt_handle->GetPrivateObject ();
  pTexCache = txt_mm->GetHighColorCacheData ();

  bColorKeyed = txt_mm->GetTransparent ();

  // retrieve the lightmap from the cache.
  iLightMap* piLM = pTex->GetLightMap ();

  if ( piLM  && m_bRenderLightmap)
  {
    pLightCache = piLM->GetHighColorCache ();
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

  if ( bTransparent )
  {
    m_States.SetAlphaBlendEnable(true);
    m_States.SetDstBlend(m_TransDstBlend);
    m_States.SetSrcBlend(m_TransSrcBlend);
  }
  else
  {
    m_States.SetAlphaBlendEnable(false);
  }

  if ( bColorKeyed )
  {
    m_States.SetColorKeyEnable(true);
  }
  else
  {
    m_States.SetColorKeyEnable(false);
  }

  D3DTextureCache_Data *pD3D_texcache = (D3DTextureCache_Data *)pTexCache->pData;

  m_States.SetTexture(0, pD3D_texcache->lptex);

  D3DCOLOR vertex_color;

//  if (bTransparent && bLightmapExists)
  if (bTransparent)
  {
    vertex_color = D3DRGBA_(1.0f, 1.0f, 1.0f, (float)poly_alpha * (1.0f / 100.0f));
  }
  else
  {
    vertex_color = D3DRGBA_(1.0f, 1.0f, 1.0f, 1.0f);
  }

  m_States.SetTextureAddress(D3DTADDRESS_WRAP);

  // render texture-mapped poly
  for (i=0; i < poly.num; i++)
  {
    float sx = poly.vertices[i].sx - m_nHalfWidth;
    float sy = poly.vertices[i].sy - m_nHalfHeight;
    float u_over_sz = (J1 * sx + J2 * sy + J3);
    float v_over_sz = (K1 * sx + K2 * sy + K3);
    float one_over_sz = (M * sx + N * sy + O);

    z = 1.0f  / one_over_sz;

    vx[i].sx = poly.vertices[i].sx;
    vx[i].sy = m_nHeight-poly.vertices[i].sy;

    vx[i].sz = z*(float)SCALE_FACTOR;

    vx[i].color = vertex_color;
    vx[i].rhw = one_over_sz;

    vx[i].specular = 0;
    vx[i].tu = u_over_sz * z;
    vx[i].tv = v_over_sz * z;
  }

  VERIFY_RESULT(m_lpd3dDevice2->DrawPrimitive(D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX,
                        vx, i, D3DDP_DONOTUPDATEEXTENTS), DD_OK);

//----

  if (bLightmapExists)
  {
    // set blending modes
    m_States.SetAlphaBlendEnable(true);
    m_States.SetSrcBlend(m_LightmapSrcBlend);
    m_States.SetDstBlend(m_LightmapDstBlend);
    m_States.SetTextureAddress(D3DTADDRESS_CLAMP);

    float lightmap_scale_u, lightmap_scale_v;
    float lightmap_low_u, lightmap_low_v;

    // set lightmap stuff
    iLightMap *thelightmap = pTex->GetLightMap ();

    int lmwidth = thelightmap->GetWidth ();
    int lmrealwidth = thelightmap->GetRealWidth ();
    int lmheight = thelightmap->GetHeight ();
    int lmrealheight = thelightmap->GetRealHeight ();
    float scale_u = (float)(lmrealwidth-1) / (float)lmwidth;
    float scale_v = (float)(lmrealheight-1) / (float)lmheight;

    float lightmap_high_u, lightmap_high_v;
    pTex->GetTextureBox(lightmap_low_u,lightmap_low_v,
                       lightmap_high_u,lightmap_high_v);

    lightmap_scale_u = scale_u / (lightmap_high_u - lightmap_low_u), 
    lightmap_scale_v = scale_v / (lightmap_high_v - lightmap_low_v);

    m_States.SetTexture(0, ((D3DLightCache_Data *)pLightCache->pData)->lptex);

    for (i=0; i < poly.num; i++)
    {
      vx[i].tu = (vx[i].tu - lightmap_low_u) * lightmap_scale_u;
      vx[i].tv = (vx[i].tv - lightmap_low_v) * lightmap_scale_v;
    }

    VERIFY_RESULT(m_lpd3dDevice2->DrawPrimitive(D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX,
                          vx, i, D3DDP_DONOTUPDATEEXTENTS), DD_OK);
  } // end of if (bLightmapExists )

//----

  // reset render states.
  // If there is vertex fog then we apply that last.
  if (poly.use_fog)
  {
    m_States.SetAlphaBlendEnable(true);
    m_States.SetZFunc(D3DCMP_LESSEQUAL);
    m_States.SetDstBlend(D3DBLEND_SRCALPHA);
    m_States.SetSrcBlend(D3DBLEND_INVSRCALPHA);
    m_States.SetTexture(0, NULL);

    for (i=0; i<poly.num; i++)
    {
    float I = poly.fog_info[i].intensity;
    CLAMPG(I, 1.0f);
    I = 1.0f - I;      

      float r = poly.fog_info[i].r > 1.0f ? 1.0f : poly.fog_info[i].r;
      float g = poly.fog_info[i].g > 1.0f ? 1.0f : poly.fog_info[i].g;
      float b = poly.fog_info[i].b > 1.0f ? 1.0f : poly.fog_info[i].b;

      vx[i].color = D3DRGBA_(r, g, b, I);
    }

  VERIFY_RESULT(m_lpd3dDevice2->DrawPrimitive(D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX,
                        vx, i, D3DDP_DONOTUPDATEEXTENTS), DD_OK);
  }

}  // end of DrawPolygon()

void csGraphics3DDirect3DDx6::StartPolygonFX (iTextureHandle* handle, UInt mode)
{
  m_gouraud = ((mode & CS_FX_GOURAUD) != 0);
  m_mixmode = mode;
  m_alpha = float (mode & CS_FX_MASK_ALPHA) * (1.0f / 255);

//  m_lpd3dDevice2->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, true);

  csHighColorCacheData* pTexData;
  if (handle)
  {
    csTextureMMDirect3D* txt_mm = (csTextureMMDirect3D*)handle->GetPrivateObject ();
    m_pTextureCache->Add (handle);

  pTexData = txt_mm->GetHighColorCacheData ();
  m_textured = true;
  }
  else
  {
  m_textured = false;
  }

  // PolygonFXs dont use multitexturing
  m_States.SetStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

  // Note: In all explanations of Mixing:
  // Color: resulting color
  // SRC:   Color of the texel (content of the texture to be drawn)
  // DEST:  Color of the pixel on screen
  // Alpha: Alpha value of the polygon
  bool enable_blending = true;
  switch (mode & CS_FX_MASK_MIXMODE)
  {
    case CS_FX_MULTIPLY:
      // Color = SRC * DEST +   0 * SRC = DEST * SRC
      m_alpha = 0.0f;
      m_States.SetDstBlend(D3DBLEND_SRCCOLOR);
      m_States.SetSrcBlend(D3DBLEND_ZERO);
      break;
    case CS_FX_MULTIPLY2:
      //Color = SRC * DEST + DEST * SRC = 2 * DEST * SRC
      m_alpha = 0.0f;
      m_States.SetDstBlend(D3DBLEND_SRCCOLOR);
      m_States.SetSrcBlend(D3DBLEND_DESTCOLOR);
      break;
    case CS_FX_ADD:
      //Color = 1 * DEST + 1 * SRC = DEST + SRC
      m_alpha = 0.0f;
      m_States.SetDstBlend(D3DBLEND_ONE);
      m_States.SetSrcBlend(D3DBLEND_ONE);
      break;
    case CS_FX_ALPHA:
      //Color = Alpha * DEST + (1-Alpha) * SRC 
      m_States.SetDstBlend(D3DBLEND_SRCALPHA);
      m_States.SetSrcBlend(D3DBLEND_INVSRCALPHA);
      break;
    case CS_FX_TRANSPARENT:
      //Color = 1 * DEST + 0 * SRC = DEST
      m_States.SetDstBlend(D3DBLEND_ONE);
      m_States.SetSrcBlend(D3DBLEND_ZERO);
      break;
    case CS_FX_COPY:
    default:
      //Color = 0 * DEST + 1 * SRC = SRC
      m_alpha = 0.0f;
      m_States.SetDstBlend(D3DBLEND_ZERO);
      m_States.SetSrcBlend(D3DBLEND_ONE);
      break;
  }

  if (m_textured)
    m_States.SetTexture(0, ((D3DTextureCache_Data *)pTexData->pData)->lptex);
  else
    m_States.SetTexture(0, NULL);

  if (m_alpha == 0.0f && (m_mixmode & CS_FX_MASK_MIXMODE) == CS_FX_ALPHA)
  {
    // workaround for a bug in alpha transparency. It looks like on some cards
    // you may not select alpha == 0, (opaque) because that will make
    // the result invisible. :-(
    m_alpha = 0.01f; 
  }

  m_ialpha = QInt(m_alpha * 255.0f);
} // end of StartPolygonFX()

void csGraphics3DDirect3DDx6::FinishPolygonFX()
{
  if (m_bBatchPolygonFX)
    BatchFinishPolygonFX();
}

//
// vertex coloring : range [0..2]
// 0 = no intensity, 1 = normal intensity, 2 = double intensity,
// but d3d cant "OverModulate", clamp anything outside of [0..1],
// we could try to counter balance this with a specular component
//
void csGraphics3DDirect3DDx6::DrawPolygonFX(G3DPolygonDPFX& poly)
{
  if (m_bBatchPolygonFX)
  {
    BatchDrawPolygonFX(poly);
    return;
  }

  int i;
  D3DTLVERTEX vx;
  D3DCOLOR FlatColor;

  float flat_r = 1., flat_g = 1., flat_b = 1.;
  if (!m_textured)
  {
    flat_r = poly.flat_color_r; 
    flat_g = poly.flat_color_g;
    flat_b = poly.flat_color_b;
  CLAMP(flat_r, 1.0f);
  CLAMP(flat_g, 1.0f);
  CLAMP(flat_b, 1.0f);

    if (!m_gouraud)
    FlatColor = D3DRGBA_(flat_r, flat_g, flat_b, m_alpha);
  }

  VERIFY_RESULT( m_lpd3dDevice2->Begin(D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX, D3DDP_DONOTUPDATEEXTENTS), DD_OK );

  for(i=0; i<poly.num; i++)
  {
    vx.sx = poly.vertices[i].sx;
    vx.sy = m_nHeight-poly.vertices[i].sy;
    vx.sz = SCALE_FACTOR / poly.vertices[i].z;
    vx.rhw = poly.vertices[i].z;
    if (m_gouraud)
  {
    float r, g, b;
    r = poly.vertices[i].r;
    CLAMP(r, 1.0f);
    g = poly.vertices[i].g;
    CLAMP(g, 1.0f);
    b = poly.vertices[i].b;
    CLAMP(b, 1.0f);
      vx.color = D3DRGBA_(r, g, b, m_alpha);
  }
    else
  {
    if (!m_textured)
      vx.color = FlatColor;
      else
    {
      float r, g, b;
      r = flat_r * poly.vertices[i].r;
      CLAMP(r, 1.0f);
      g = flat_r * poly.vertices[i].g;
      CLAMP(g, 1.0f);
      b = flat_r * poly.vertices[i].b;
      CLAMP(b, 1.0f);
        vx.color = D3DRGBA_(r, g, b, m_alpha);
    }
  }

  vx.specular = RGBA_MAKE(0, 0, 0, 0);
    vx.tu = poly.vertices[i].u;
    vx.tv = poly.vertices[i].v;

    VERIFY_RESULT(m_lpd3dDevice2->Vertex( &vx ), DD_OK);
  }

  VERIFY_RESULT( m_lpd3dDevice2->End(0), DD_OK );

  if (poly.use_fog)
  {
    // Remember old values.
    // @@@ This can't be efficient! We are setting and resetting
    // Renderstate values for every triangle of a sprite. Maybe we
    // should first do all non-fogged triangles and then in one
    // go all fogged triangles?

    m_States.PushAlphaBlendEnable();
    m_States.PushZFunc();
    m_States.PushDstBlend();
    m_States.PushSrcBlend();
    m_States.PushTexture(0);

    m_States.SetAlphaBlendEnable(true);
    m_States.SetZFunc(D3DCMP_LESSEQUAL);
    m_States.SetDstBlend(D3DBLEND_SRCALPHA);
    m_States.SetSrcBlend(D3DBLEND_INVSRCALPHA);
    m_States.SetTexture(0, NULL);

    VERIFY_RESULT( m_lpd3dDevice2->Begin(D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX, D3DDP_DONOTUPDATEEXTENTS), DD_OK );
  
    for(i=0; i<poly.num; i++)
    {
      vx.sx = poly.vertices[i].sx;
      vx.sy = m_nHeight-poly.vertices[i].sy;
      vx.sz = SCALE_FACTOR / poly.vertices[i].z;
      vx.rhw = poly.vertices[i].z;

    float I = poly.fog_info[i].intensity;
    CLAMPG(I, 1.0f);
    I = 1.0f - I;      

      float r = poly.fog_info[i].r > 1.0f ? 1.0f : poly.fog_info[i].r;
      float g = poly.fog_info[i].g > 1.0f ? 1.0f : poly.fog_info[i].g;
      float b = poly.fog_info[i].b > 1.0f ? 1.0f : poly.fog_info[i].b;

      vx.color    = D3DRGBA_(r, g, b, I);

      vx.specular = RGBA_MAKE(0, 0, 0, 0);
      vx.tu = poly.vertices[i].u;
      vx.tv = poly.vertices[i].v;
    
      VERIFY_RESULT(m_lpd3dDevice2->Vertex( &vx ), DD_OK);
    }

    VERIFY_RESULT( m_lpd3dDevice2->End(0), DD_OK );

  m_States.PopAlphaBlendEnable();
  m_States.PopZFunc();
  m_States.PopDstBlend();
  m_States.PopSrcBlend();
  m_States.PopTexture(0);
  }
}

void csGraphics3DDirect3DDx6::BatchStartPolygonFX(iTextureHandle*, UInt)
{
}

void csGraphics3DDirect3DDx6::BatchFinishPolygonFX()
{
  m_VertexCache.EmptyBuffer();
}

void csGraphics3DDirect3DDx6::BatchDrawPolygonFX(G3DPolygonDPFX& poly)
{
  int i;
  D3DTLVERTEX vx[2];
  D3DCOLOR FlatColor;

  float flat_r = 1., flat_g = 1., flat_b = 1.;
  if (!m_textured)
  {
    flat_r = poly.flat_color_r; 
    flat_g = poly.flat_color_g;
    flat_b = poly.flat_color_b;
    CLAMP(flat_r, 1.0f);
    CLAMP(flat_g, 1.0f);
    CLAMP(flat_b, 1.0f);

    if (!m_gouraud)
    FlatColor = D3DRGBA_(flat_r, flat_g, flat_b, m_alpha);
  }

//-- make base vertex
  vx[0].sx = poly.vertices[0].sx;
  vx[0].sy = m_nHeight-poly.vertices[0].sy;
  vx[0].sz = SCALE_FACTOR / poly.vertices[0].z;
  vx[0].rhw = poly.vertices[0].z;
  if (m_gouraud)
  {
    float r, g, b;
    r = poly.vertices[0].r;
    CLAMP(r, 1.0f);
    g = poly.vertices[0].g;
    CLAMP(g, 1.0f);
    b = poly.vertices[0].b;
    CLAMP(b, 1.0f);
    vx[0].color = D3DRGBA_(r, g, b, m_alpha);
  }
  else
  {
    if (!m_textured)
      vx[0].color = FlatColor;
    else
    {
      float r, g, b;
      r = flat_r * poly.vertices[0].r;
      CLAMP(r, 1.0f);
      g = flat_r * poly.vertices[0].g;
      CLAMP(g, 1.0f);
      b = flat_r * poly.vertices[0].b;
      CLAMP(b, 1.0f);
      vx[0].color = D3DRGBA_(r, g, b, m_alpha);
    }
  }
  vx[0].specular = RGBA_MAKE(0, 0, 0, 0);
  vx[0].tu = poly.vertices[0].u;
  vx[0].tv = poly.vertices[0].v;
//-- make "last" vertex
  vx[1].sx = poly.vertices[1].sx;
  vx[1].sy = m_nHeight-poly.vertices[1].sy;
  vx[1].sz = SCALE_FACTOR / poly.vertices[1].z;
  vx[1].rhw = poly.vertices[1].z;
  if (m_gouraud)
  {
    float r, g, b;
    r = poly.vertices[1].r;
    CLAMP(r, 1.0f);
    g = poly.vertices[1].g;
    CLAMP(g, 1.0f);
    b = poly.vertices[1].b;
    CLAMP(b, 1.0f);
    vx[1].color = D3DRGBA_(r, g, b, m_alpha);
  }
  else
  {
    if (!m_textured)
      vx[1].color = FlatColor;
    else
    {
      float r, g, b;
      r = flat_r * poly.vertices[1].r;
      CLAMP(r, 1.0f);
      g = flat_r * poly.vertices[1].g;
      CLAMP(g, 1.0f);
      b = flat_r * poly.vertices[1].b;
      CLAMP(b, 1.0f);
      vx[1].color = D3DRGBA_(r, g, b, m_alpha);
    }
  }
  vx[1].specular = RGBA_MAKE(0, 0, 0, 0);
  vx[1].tu = poly.vertices[1].u;
  vx[1].tv = poly.vertices[1].v;
//-- process polygons
  for(i=2; i<poly.num; i++)
  {
    D3DTLVERTEX * V = m_VertexCache.AddPolygon();
    V[0] = vx[0];
    V[1] = vx[1];

    vx[1].sx = poly.vertices[i].sx;
    vx[1].sy = m_nHeight-poly.vertices[i].sy;
    vx[1].sz = SCALE_FACTOR / poly.vertices[i].z;
    vx[1].rhw = poly.vertices[i].z;
    if (m_gouraud)
    {
      float r, g, b;
      r = poly.vertices[i].r;
      CLAMP(r, 1.0f);
      g = poly.vertices[i].g;
      CLAMP(g, 1.0f);
      b = poly.vertices[i].b;
      CLAMP(b, 1.0f);
      vx[1].color = D3DRGBA_(r, g, b, m_alpha);
    }
    else
    {
      if (!m_textured)
        vx[1].color = FlatColor;
      else
      {
        float r, g, b;
        r = flat_r * poly.vertices[i].r;
        CLAMP(r, 1.0f);
        g = flat_r * poly.vertices[i].g;
        CLAMP(g, 1.0f);
        b = flat_r * poly.vertices[i].b;
        CLAMP(b, 1.0f);
        vx[1].color = D3DRGBA_(r, g, b, m_alpha);
      }
    }

    vx[1].specular = RGBA_MAKE(50, 50, 50, 0);
    vx[1].tu = poly.vertices[i].u;
    vx[1].tv = poly.vertices[i].v;

    V[2] = vx[1];
  }

  // TODO/FIXME : implemente fog
}

void csGraphics3DDirect3DDx6::GetCaps(G3D_CAPS *caps)
{
  if (!caps)
    return;
  
  memcpy(caps, &m_Caps, sizeof(G3D_CAPS));
}

void csGraphics3DDirect3DDx6::DrawLine (csVector3& v1, csVector3& v2, float aspect, int color)
{
  if (v1.z < SMALL_Z && v2.z < SMALL_Z)
    return;
  
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

bool csGraphics3DDirect3DDx6::SetRenderState(G3D_RENDERSTATEOPTION option, long value)
{
  switch (option)
  {
    case G3DRENDERSTATE_NOTHING:
      break;
    
    case G3DRENDERSTATE_ZBUFFERTESTENABLE:
      if (value)
        m_States.SetZFunc(D3DCMP_LESSEQUAL);
      else
        m_States.SetZFunc(D3DCMP_ALWAYS);
      break;
    
    case G3DRENDERSTATE_ZBUFFERFILLENABLE:
      if (value)
        VERIFY_RESULT(m_lpd3dDevice2->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE), DD_OK);
      else
        VERIFY_RESULT(m_lpd3dDevice2->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE), DD_OK);
      break;
    
    case G3DRENDERSTATE_DITHERENABLE:
      if(value)
        VERIFY_RESULT(m_lpd3dDevice2->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE), DD_OK);
      else
        VERIFY_RESULT(m_lpd3dDevice2->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE), DD_OK);
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
      return false;
  }
  
  return true;
}

long csGraphics3DDirect3DDx6::GetRenderState(G3D_RENDERSTATEOPTION op)
{
  DWORD d3d_value;
  switch(op)
  {
    case G3DRENDERSTATE_NOTHING:
      return 0;
    case G3DRENDERSTATE_ZBUFFERTESTENABLE:
      VERIFY_RESULT(m_lpd3dDevice2->GetRenderState(D3DRENDERSTATE_ZFUNC, &d3d_value), DD_OK);
      return (bool)(D3DCMP_LESSEQUAL & d3d_value);
    case G3DRENDERSTATE_ZBUFFERFILLENABLE:
      VERIFY_RESULT(m_lpd3dDevice2->GetRenderState(D3DRENDERSTATE_ZWRITEENABLE, &d3d_value), DD_OK);
      return (bool)(TRUE & d3d_value);
    case G3DRENDERSTATE_DITHERENABLE:
      VERIFY_RESULT(m_lpd3dDevice2->GetRenderState(D3DRENDERSTATE_DITHERENABLE, &d3d_value), DD_OK);
      return (d3d_value == TRUE)?true:false;
    case G3DRENDERSTATE_SPECULARENABLE:
      return rstate_specular;
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

void csGraphics3DDirect3DDx6::OpenFogObject (CS_ID /*id*/, csFog* /*fog*/)
{
}

void csGraphics3DDirect3DDx6::AddFogPolygon (CS_ID /*id*/,
  G3DPolygonAFP& /*poly*/, int /*fogtype*/)
{
}

void csGraphics3DDirect3DDx6::CloseFogObject (CS_ID /*id*/)
{
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
