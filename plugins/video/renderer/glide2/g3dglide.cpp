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

// G3D_GLIDE.CPP
// csGraphics3DGlide2x implementation file
// Written by xtrochu and Nathaniel

#if defined(OS_WIN32)
#include <windows.h>
#endif
#if defined(OS_MACOS)
#include <Dialogs.h>
#include <TextUtils.h>
#define GLIDE24_ONLY
#define SYSDEF_2DDRIVER_DEFS
#endif
#include <stdlib.h>
#include <stdarg.h>
#include <glide.h>

#include "sysdef.h"
#include "cs3d/software/scan.h" //@@@WHY?
#include "isystem.h"
#include "ipolygon.h"
#include "icamera.h"
#include "itexture.h"
#include "igraph3d.h"
#include "itxtmgr.h"
#include "ilghtmap.h"
#include "igraph2d.h"

#include "csutil/inifile.h"
#include "csutil/scanstr.h"
#include "qint.h"

#if defined (OS_WIN32)
#include "cs2d/winglide2/g2d.h"
#include "cs2d/winglide2/ig2d.h"
#elif defined (OS_MACOS)
#include "cs2d/macglide2/g2d.h"
#include "cs2d/macglide2/ig2d.h"
#else //Is there another platform Glide runs on?
//#include "cs2d/unxglide2/g2d.h"
//#include "cs2d/unxglide2/ig2d.h"
#endif

#include "cs3d/glide2/glidelib.h"
#include "cs3d/glide2/g3dglide.h"

csIniFile *configglide;

//
// Interface table definition
//

IMPLEMENT_UNKNOWN(csGraphics3DGlide2x)

BEGIN_INTERFACE_TABLE(csGraphics3DGlide2x)
  IMPLEMENTS_INTERFACE( IGraphics3D )
  IMPLEMENTS_INTERFACE (IHaloRasterizer)
END_INTERFACE_TABLE()

// Error Message handling
void sys_fatalerror(char *str, HRESULT hRes = S_OK)
{
#if defined(OS_WIN32)
  if (hRes!=S_OK)
  {
    LPVOID lpMsgBuf;
    char* szMsg;
    char szStdMessage[] = "\nLast Error: ";

    DWORD dwResult;
    dwResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
                             hRes,  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
                             (LPTSTR) &lpMsgBuf, 0, NULL );
        
    if (dwResult != 0)
    {
        szMsg = new char[strlen((const char*)lpMsgBuf) + strlen(str) + strlen(szStdMessage) + 1];
        strcpy( szMsg, str );
        strcat( szMsg, szStdMessage );
        strcat( szMsg, (const char*)lpMsgBuf );
                
        LocalFree( lpMsgBuf );

        MessageBox (NULL, szMsg, "Fatal Error in Glide2xRender.dll", MB_OK);
        delete szMsg;

        exit(1);
    }
  }

  MessageBox(NULL, str, "Fatal Error in Glide2xRender.dll", MB_OK);
#elif defined( OS_MACOS )
	Str255	theString;
	Str255	theString2;

	GlideLib_grSstControlMode( GR_CONTROL_DEACTIVATE );
	GetIndString( theString, kErrorStrings, kFatalErrorInGlide );
	strcpy( (char *)&theString2[1], str );
	theString2[0] = strlen( str );
	ParamText( theString, theString2,  "\p", "\p" );
	StopAlert( kGeneralErrorDialog, NULL );
	GlideLib_grSstControlMode( GR_CONTROL_ACTIVATE );
#else
  fprintf(stderr, "FATAL ERROR: %s", str);
#endif

  exit(1);
}

/**
* This Function gets information from the Current Board selected.
* Those informations are written to the Initialization Log
* It also use those informations to initialize the rendering process
* This initialisation includes:
*   - Standard init of the board
*   - Speficic init for rendering pass (One or two based on the number of TMUs)
**/
void csGraphics3DGlide2x::InitializeBoard(GrHwConfiguration & hw)
{
  int i;
        
  switch(hw.SSTs[board].type)
  {
#ifndef GLIDE24_ONLY
  case GR_SSTTYPE_Voodoo2:
    {
          SysPrintf (MSG_INITIALIZATION, " Board is a Voodoo2.\n");
          SysPrintf (MSG_INITIALIZATION, "  Pixelfx Revision %d.\n",hw.SSTs[board].sstBoard.Voodoo2Config.fbiRev);
          SysPrintf (MSG_INITIALIZATION, "  Pixelfx Memory : %d MB.\n",hw.SSTs[board].sstBoard.Voodoo2Config.fbRam);
          SysPrintf (MSG_INITIALIZATION, "  Number of Texelfx Chips %d.\n",hw.SSTs[board].sstBoard.Voodoo2Config.nTexelfx);
          for(i=0;i<hw.SSTs[board].sstBoard.Voodoo2Config.nTexelfx;i++)
          {
            SysPrintf (MSG_INITIALIZATION, "   Texelfx nb %d has revision %d and %d MB of RAM.\n",i+1,
                        hw.SSTs[board].sstBoard.Voodoo2Config.tmuConfig[i].tmuRev,
                        hw.SSTs[board].sstBoard.Voodoo2Config.tmuConfig[i].tmuRam);
          }
          if (hw.SSTs[board].sstBoard.Voodoo2Config.sliDetect)
            SysPrintf (MSG_INITIALIZATION, " SLI Detected.\n");

          if (hw.SSTs[board].sstBoard.Voodoo2Config.nTexelfx==1)
          {
            m_iMultiPass=true;
            iTMUTexture=1;
            iTMULightMap=0;
          }
          else
          {
            m_iMultiPass=false;
            iTMULightMap=/*hw.SSTs[board].sstBoard.Voodoo2Config.nTexelfx/2*/1;
            iTMUTexture=/*hw.SSTs[board].sstBoard.Voodoo2Config.nTexelfx-iTMULightMap*/1;
          }
    } break;
#endif

  case GR_SSTTYPE_VOODOO:
    {
      SysPrintf (MSG_INITIALIZATION, " Board is a Voodoo Graphics.\n");
      SysPrintf (MSG_INITIALIZATION, "  Pixelfx Revision %d.\n",hw.SSTs[board].sstBoard.VoodooConfig.fbiRev);
      SysPrintf (MSG_INITIALIZATION, "  Pixelfx Memory : %d MB.\n",hw.SSTs[board].sstBoard.VoodooConfig.fbRam);
      SysPrintf (MSG_INITIALIZATION, "  Number of Texelfx Chips %d.\n",hw.SSTs[board].sstBoard.VoodooConfig.nTexelfx);
      for(i=0;i<hw.SSTs[board].sstBoard.VoodooConfig.nTexelfx;i++)
        {
          SysPrintf (MSG_INITIALIZATION, "   Texelfx nb %d has revision %d and %d MB of RAM.\n",i+1,
                     hw.SSTs[board].sstBoard.VoodooConfig.tmuConfig[i].tmuRev,
                     hw.SSTs[board].sstBoard.VoodooConfig.tmuConfig[i].tmuRam);
        }
      if (hw.SSTs[board].sstBoard.VoodooConfig.sliDetect)
        SysPrintf (MSG_INITIALIZATION, " SLI Detected.\n");

      if (hw.SSTs[board].sstBoard.VoodooConfig.nTexelfx==1)
        {
          m_iMultiPass=true;
          iTMUTexture=1;
          iTMULightMap=0;
        }
      else
        {
          m_iMultiPass=false;
          iTMULightMap=/*hw.SSTs[board].sstBoard.VoodooConfig.nTexelfx/2*/1;
          iTMUTexture=/*hw.SSTs[board].sstBoard.VoodooConfig.nTexelfx-iTMULightMap*/1;
        }
    } break;

  case GR_SSTTYPE_SST96:
    {
      SysPrintf (MSG_INITIALIZATION, " Board is a Rush.\n");
      SysPrintf (MSG_INITIALIZATION, "  Pixelfx Memory : %d MB.\n",hw.SSTs[board].sstBoard.SST96Config.fbRam);
      SysPrintf (MSG_INITIALIZATION, "  Number of Texelfx Chips %d.\n",hw.SSTs[board].sstBoard.SST96Config.nTexelfx);
      SysPrintf (MSG_INITIALIZATION, "   Texelfx nb %d has revision %d and %d MB of RAM.\n",1,
                 hw.SSTs[board].sstBoard.SST96Config.tmuConfig.tmuRev,
                 hw.SSTs[board].sstBoard.SST96Config.tmuConfig.tmuRam);
      m_iMultiPass=true;
      iTMUTexture=1;
      iTMULightMap=0;
    } break;

  case GR_SSTTYPE_AT3D:
    {
      SysPrintf (MSG_INITIALIZATION, " Board is an AT3D.\n");
      SysPrintf (MSG_INITIALIZATION, "  Chipset Revision %d.\n",hw.SSTs[board].sstBoard.AT3DConfig.rev);
      m_iMultiPass=true;
      iTMUTexture=1;
      iTMULightMap=0;
    }break;

  default:
    {
      SysPrintf (MSG_INITIALIZATION, " Board is of an unknown type.(%x)\n",hw.SSTs[board].type);
      m_iMultiPass=true;
      iTMUTexture=1;
      iTMULightMap=0;
    } break;
  }
  if(configglide->GetYesNo("Glide2x","FORCEMULTIPASS",FALSE)&& m_iMultiPass==false)
    {
      SysPrintf (MSG_INITIALIZATION, " MultiPass Rendering enable by user.\n");
      m_iMultiPass=true;
    }

  if(m_iMultiPass)
    {
      SysPrintf (MSG_INITIALIZATION, " Will use MultiPass Rendering.\n");
    }
  else
    {
      SysPrintf (MSG_INITIALIZATION, " Will use SinglePass Rendering.\n");
      SysPrintf (MSG_INITIALIZATION, " Affected %d TMU for Texture and %d for LightMap.\n",iTMUTexture,iTMULightMap);
    }
  m_TMUs = new TMUInfo[2];

  if (m_iMultiPass)
    {
      m_TMUs[0].tmu_id=m_TMUs[1].tmu_id=0;
      m_TMUs[0].minAddress = GlideLib_grTexMinAddress(0);
      m_TMUs[1].maxAddress = GlideLib_grTexMaxAddress(0);
      m_TMUs[1].minAddress = m_TMUs[0].maxAddress = (m_TMUs[1].maxAddress -  m_TMUs[0].minAddress)/2;
      m_TMUs[1].minAddress = (m_TMUs[1].minAddress+7)&(~7);
    } 
  else {
    m_TMUs[0].tmu_id=0;
    m_TMUs[0].minAddress = GlideLib_grTexMinAddress(0);
    m_TMUs[0].maxAddress = GlideLib_grTexMaxAddress(0);
    m_TMUs[1].tmu_id=1;
    m_TMUs[1].minAddress = GlideLib_grTexMinAddress(1);
    m_TMUs[1].maxAddress = GlideLib_grTexMaxAddress(1);
  }
  m_TMUs[0].memory_size = (m_TMUs[0].maxAddress -  m_TMUs[0].minAddress);
  m_TMUs[1].memory_size = (m_TMUs[1].maxAddress -  m_TMUs[1].minAddress);
}


/**
* Automatic selection of the board.
* Algorithm used:
* If multiple boards, select which one in that order, (The Most RAM if equivalent):
* If it is a Voodoo2 with sli, it is selected
* If it is a Voodoo2, it is selected
* If it is a Voodoo with sli, it is selected
* If it is a Voodoo, it is selected
* Else, the first one
* At this moment, there is no documented way to detect Banshee !
**/
int csGraphics3DGlide2x::SelectBoard(GrHwConfiguration & hwconfig)
{
  int i;
  board=0;
  for(i=0; i<hwconfig.num_sst; i++)
    {
      switch(hwconfig.SSTs[i].type)
        {
#ifndef GLIDE24_ONLY
        case GR_SSTTYPE_Voodoo2:
          if(hwconfig.SSTs[board].type!=GR_SSTTYPE_Voodoo2)
            {
              board=i;
            }
          else
            {
              if(hwconfig.SSTs[board].sstBoard.Voodoo2Config.sliDetect==FXTRUE)
                {
                  if(hwconfig.SSTs[i].sstBoard.Voodoo2Config.sliDetect==FXTRUE)
                    {   // Equivalent => Most RAM
                      if(hwconfig.SSTs[i].sstBoard.Voodoo2Config.fbRam>hwconfig.SSTs[board].sstBoard.Voodoo2Config.fbRam)
                        {
                          board=i;
                        }
                    }
                }
              else
                {
                  if(hwconfig.SSTs[i].sstBoard.Voodoo2Config.sliDetect==FXTRUE)
                    {   
                      board=i;
                    }
                  else
                    {   // Equivalent => Most RAM
                      if(hwconfig.SSTs[i].sstBoard.Voodoo2Config.fbRam>hwconfig.SSTs[board].sstBoard.Voodoo2Config.fbRam)
                        {
                          board=i;
                        }
                    }
                }
            }
          break;
#endif

        case GR_SSTTYPE_VOODOO:
#ifndef GLIDE24_ONLY
          if(hwconfig.SSTs[board].type==GR_SSTTYPE_Voodoo2)
            break;
#endif

          if(hwconfig.SSTs[board].type!=GR_SSTTYPE_VOODOO)
            {
              board=i;
            }
          else
            {
              if(hwconfig.SSTs[board].sstBoard.VoodooConfig.sliDetect==FXTRUE)
                {
                  if(hwconfig.SSTs[i].sstBoard.VoodooConfig.sliDetect==FXTRUE)
                    {   // Equivalent => Most RAM
                      if(hwconfig.SSTs[i].sstBoard.VoodooConfig.fbRam>hwconfig.SSTs[board].sstBoard.VoodooConfig.fbRam)
                        {
                          board=i;
                        }
                    }
                }
              else
                {
                  if(hwconfig.SSTs[i].sstBoard.VoodooConfig.sliDetect==FXTRUE)
                    {   
                      board=i;
                    }
                  else
                    {   // Equivalent => Most RAM
                      if(hwconfig.SSTs[i].sstBoard.VoodooConfig.fbRam>hwconfig.SSTs[board].sstBoard.VoodooConfig.fbRam)
                        {
                          board=i;
                        }
                    }
                }
            }
          break;

        default:
          break;
        }
    }
  return board;
}

csGraphics3DGlide2x::csGraphics3DGlide2x(ISystem* piSystem) :
    m_pTextureCache(NULL),
    m_pLightmapCache(NULL),
    m_piSystem(piSystem)    
{
  HRESULT hRes;
// BM 21-02: Not existing definition replaced
#ifndef GLIDE_2D_DRIVER
  char *sz2DDriver = "crystalspace.graphics3d.glide.2x";
#else
  char *sz2DDriver = GLIDE_2D_DRIVER;
#endif

  IGraphics2DFactory* piFactory = NULL;
 

  m_piSystem->AddRef();
  ASSERT( m_piSystem );

#if defined (OS_WIN32) || defined (OS_MACOS)
  // we use a 2d driver include in dll (not separate) for win32 glide
  piFactory = new csGraphics2DGlide2xFactory();
#else
  CLSID clsid2dDriver;
  hRes = csCLSIDFromProgID( &sz2DDriver, &clsid2dDriver );

  if (FAILED(hRes))
  {       
      SysPrintf(MSG_FATAL_ERROR, "Error! 2D Graphics DLL with ProgID \"%s\" not found on this system.", sz2DDriver);
      exit(0);
  }

  hRes = csCoGetClassObject( clsid2dDriver, CLSCTX_INPROC_SERVER, NULL, (REFIID)
IID_IGraphics2DFactory, (void**)&piFactory );
  if (FAILED(hRes))
  {
      SysPrintf(MSG_FATAL_ERROR, "Error! Couldn't create 2D graphics driver instance.");
      exit(0);
  }
#endif

  hRes = piFactory->CreateInstance( IID_IGraphics2D, m_piSystem, (void**)&m_piG2D );
  if (FAILED(hRes))
  {
      SysPrintf(MSG_FATAL_ERROR, "Error! Couldn't create 2D graphics driver instance.");
      exit(0);
  }

  FINAL_RELEASE( piFactory );

  CHK (txtmgr = new csTextureManagerGlide (m_piSystem, m_piG2D));
}

STDMETHODIMP csGraphics3DGlide2x::Initialize ()
{ 
  m_piG2D->Initialize ();
  txtmgr->InitSystem ();

  SysPrintf (MSG_INITIALIZATION, "\nGlideRender Glide2x selected\n");

  configglide = new csIniFile("Glide2x.cfg");
  m_bVRetrace = configglide->GetYesNo("Glide2x","VRETRACE",FALSE);
  GrHwConfiguration grconfig;
  GlideLib_grSstQueryBoards(&grconfig);


  if(grconfig.num_sst==0) 
    sys_fatalerror("csGraphics3DGlide2x::Open : No 3dfx chip found");
        
  GlideLib_grGlideInit();
        
  if(!GlideLib_grSstQueryHardware(&grconfig))
    sys_fatalerror("csGraphics3DGlide2x::Open : Unable to find any 3dfx chip");
        
  char szVersion[80];
  GlideLib_grGlideGetVersion(szVersion);
  SysPrintf (MSG_INITIALIZATION, " Glide %s detected.\n",szVersion);
  SelectBoard(grconfig);
  SysPrintf (MSG_INITIALIZATION, " Board %d selected.\n",board);
        
  InitializeBoard(grconfig);

  GlideLib_grSstSelect(board);

  m_bHaloEffect=configglide->GetYesNo("Glide2x","DISABLE_HALO", false);
  if(m_bHaloEffect)
    SysPrintf (MSG_INITIALIZATION, " Disable Halo Effect support.\n");

  // gsteenss: glide now always uses 16-bit textures !

  /*use16BitTexture=configglide->GetYesNo("Glide2x","USE_16BIT_TEXTURE", false);
  if(use16BitTexture)
    SysPrintf (MSG_INITIALIZATION, " Use 16 bits for texture.\n");
    CHK (m_pTextureCache = new GlideTextureCache(&m_TMUs[0], (use16BitTexture)?16:8, new FixedTextureMemoryManager(m_TMUs[0].memory_size)));*/

  CHK (m_pTextureCache = new GlideTextureCache(&m_TMUs[0], 16, new FixedTextureMemoryManager(m_TMUs[0].memory_size)));

  CHK (m_pLightmapCache = new GlideLightmapCache(&m_TMUs[1],new FixedTextureMemoryManager(m_TMUs[1].memory_size)));

  // default
  m_Caps.ColorModel = G3DCOLORMODEL_RGB;
  m_Caps.CanClip = false;
  m_Caps.SupportsArbitraryMipMapping = false;
  m_Caps.BitDepth = 16;
  m_Caps.ZBufBitDepth = 32;
  m_Caps.minTexHeight = 2;
  m_Caps.minTexWidth = 2;
  m_Caps.maxTexHeight = 256;
  m_Caps.maxTexWidth = 256;
  m_Caps.PrimaryCaps.RasterCaps = G3DRASTERCAPS_SUBPIXEL;
  m_Caps.PrimaryCaps.canBlend = true;
  m_Caps.PrimaryCaps.ShadeCaps = G3DRASTERCAPS_LIGHTMAP;
  m_Caps.PrimaryCaps.PerspectiveCorrects = true;
  m_Caps.PrimaryCaps.FilterCaps = G3D_FILTERCAPS((int)G3DFILTERCAPS_NEAREST | (int)G3DFILTERCAPS_MIPNEAREST);

  rstate_dither = false;
  rstate_specular = false;
  rstate_bilinearmap = true;
  rstate_trilinearmap = false;
  rstate_gouraud = true;
  rstate_flat = true;
  rstate_alphablend = true;
  rstate_mipmap = true;
  rstate_edges = false;

  return S_OK;
}

static struct
{
        int width,height;
        GrScreenResolution_t res;
} StatGlideRes[]=
{
        {0,0,GR_RESOLUTION_NONE},
        {320,200,GR_RESOLUTION_320x200},
        {320,240,GR_RESOLUTION_320x240},
        {400,256,GR_RESOLUTION_400x256},
        {512,384,GR_RESOLUTION_512x384},
        {640,200,GR_RESOLUTION_640x200},
        {640,350,GR_RESOLUTION_640x350},
        {640,400,GR_RESOLUTION_640x400},
        {640,480,GR_RESOLUTION_640x480},
        {800,600,GR_RESOLUTION_800x600},
        {960,720,GR_RESOLUTION_960x720},
        {856,480,GR_RESOLUTION_856x480},
        {512,256,GR_RESOLUTION_512x256},
#ifndef GLIDE24_ONLY
  {1024,768,GR_RESOLUTION_1024x768},
        {1280,1024,GR_RESOLUTION_1280x1024},
        {1600,1200,GR_RESOLUTION_1600x1200},
        {400,300,GR_RESOLUTION_400x300},
#endif
};

#define SIZEOFRESSTRUCT (sizeof(StatGlideRes)/sizeof(StatGlideRes[0]))

static int getResolutionIndex(int width, int height)
{
  int i;
  for(i=1;i<SIZEOFRESSTRUCT;i++)
    {
      if((width==StatGlideRes[i].width)&&(height==StatGlideRes[i].height))
        return i;
    }
  return -1;
}

STDMETHODIMP csGraphics3DGlide2x::Open(char* Title)
{
  FxU32 hwnd=0;
  GrScreenResolution_t iRes;
  HRESULT hRes;
  IGraphicsInfo*      pGraphicsInfo = NULL;

#if defined(OS_WIN32)
  HWND w;
  IGlide2xGraphicsInfo* pSysGInfo = NULL;
#endif

  
  hRes=m_piG2D->QueryInterface(IID_IGraphicsInfo, (void**)&pGraphicsInfo);
  if (FAILED(hRes))
    goto OnError;

   
  // Open the 2D driver.
  hRes = m_piG2D->Open(Title);
  if ( FAILED(hRes) )
    goto OnError;

#if defined(OS_WIN32)
  hRes = m_piG2D->QueryInterface(IID_IGlide2xGraphicsInfo, (void**)&pSysGInfo);
  if (FAILED(hRes))
    goto OnError;
  
  pSysGInfo->GethWnd(&w);
  if(w)
    hwnd=(FxU32)w;
#endif

  if(use16BitTexture)
    SysPrintf(MSG_INITIALIZATION, "  Use 16 bit textures\n");

  pGraphicsInfo->GetWidth(m_nWidth);
  m_nHalfWidth = m_nWidth/2;
  
  pGraphicsInfo->GetHeight(m_nHeight);
  m_nHalfHeight = m_nHeight/2;
  
  if(/*configglide->GetYesNo("VideoDriver","FULL_SCREEN", true)*/true)
    {
      iRes=::getResolutionIndex(m_nWidth, m_nHeight);
      if(iRes==-1)
        sys_fatalerror("csGraphics3DGlide2x::Open() Invalid Resolution !");
    }
  else
    {
      iRes=0;
    }
        
  if(!GlideLib_grSstWinOpen(hwnd, StatGlideRes[iRes].res,GR_REFRESH_60Hz, // We should find a way to allow to change the refresh rate
                GR_COLORFORMAT_ARGB,GR_ORIGIN_LOWER_LEFT,2,1))
    {
      sys_fatalerror("csGraphics3DGlide2x::Open() : Could not open Window !");
    }

  GlideLib_grRenderBuffer(GR_BUFFER_BACKBUFFER);        // RENDER IN BACKBUFFER
  GlideLib_grColorMask(FXTRUE,FXFALSE);                 // DISABLE ALPHA BUFFER
  GlideLib_grDepthMask(FXTRUE);                                 // ENABLE ZBUFFER
  GlideLib_grDepthBufferMode(GR_DEPTHBUFFER_WBUFFER); // ENABLE WBUFFER
  GlideLib_grDepthBufferFunction(GR_CMP_LEQUAL);                // WBUFFER FUNCTION
  
  GlideLib_grBufferClear(0,0,GR_WDEPTHVALUE_FARTHEST); // CLEAR BACKBUFFER
  GlideLib_grBufferSwap(0);                                                     // PUT BACKBUFFER TO FRONT
  GlideLib_grCullMode(GR_CULL_DISABLE);                         // CULL POSITIVE 
  GlideLib_grChromakeyValue(0x0000);
        
  if(m_iMultiPass)
    { // This card has only one TMU, enable Multipass rendering   
      GlideLib_grTexCombine(m_TMUs[0].tmu_id,
      GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
      GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
      FXFALSE,FXFALSE); // TMU INIT
    
      GlideLib_grAlphaBlendFunction(
      GR_BLEND_ONE, GR_BLEND_ZERO,
      GR_BLEND_ONE, GR_BLEND_ZERO);

      GlideLib_grTexClampMode(m_TMUs[0].tmu_id,
                              GR_TEXTURECLAMP_WRAP,GR_TEXTURECLAMP_WRAP);        // TEXTURE WRAP
      GlideLib_grTexFilterMode(m_TMUs[0].tmu_id,
                               GR_TEXTUREFILTER_BILINEAR,GR_TEXTUREFILTER_BILINEAR);     // Bilinear TEXTURE

      //GlideLib_grTexLodBiasValue(m_TMUs[0].tmu_id,7.75);              // LOD BIAS
      GlideLib_grTexMipMapMode(m_TMUs[0].tmu_id,GR_MIPMAP_NEAREST,FXFALSE);     // MIPMAP Mode
      //GlideLib_grTexMipMapMode(m_TMUs[0].tmu_id,GR_MIPMAP_DISABLE,FXFALSE);
      RenderPolygon=RenderPolygonMultiPass;
    }
  else
    { // This card has several TMUs, enable Singlepass rendering
      GlideLib_grTexCombine(m_TMUs[0].tmu_id,
                            GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
                            GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_NONE,
                            FXFALSE,FXFALSE);   // TMU INIT (SCALED BY TMU1)
      GlideLib_grTexClampMode(m_TMUs[0].tmu_id,
                              GR_TEXTURECLAMP_WRAP,GR_TEXTURECLAMP_WRAP);        // TEXTURE WRAP
      GlideLib_grTexFilterMode(m_TMUs[0].tmu_id,
                               GR_TEXTUREFILTER_BILINEAR,GR_TEXTUREFILTER_BILINEAR);     // TEXTURE WRAP
      //GlideLib_grTexLodBiasValue(m_TMUs[0].tmu_id,7.75);              // LOD BIAS             
      GlideLib_grTexMipMapMode(m_TMUs[0].tmu_id,GR_MIPMAP_NEAREST,FXFALSE); // MIPMAP Mode
      
      GlideLib_grTexCombine(m_TMUs[1].tmu_id,
                            GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
                            GR_COMBINE_FUNCTION_ZERO,GR_COMBINE_FACTOR_NONE,
                            FXFALSE,FXFALSE);   // TMU INIT
      GlideLib_grTexClampMode(m_TMUs[1].tmu_id,
                              GR_TEXTURECLAMP_CLAMP,GR_TEXTURECLAMP_CLAMP);      // TEXTURE WRAP
      GlideLib_grTexFilterMode(m_TMUs[1].tmu_id,
                               GR_TEXTUREFILTER_BILINEAR,GR_TEXTUREFILTER_BILINEAR);     // TEXTURE WRAP
      GlideLib_grTexMipMapMode(m_TMUs[1].tmu_id,GR_MIPMAP_NEAREST,FXFALSE); // MIPMAP Mode
      GlideLib_grHints(0,GR_STWHINT_ST_DIFF_TMU1);
      GlideLib_grAlphaBlendFunction(
                                    GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA,
                                    GR_BLEND_ONE,GR_BLEND_ZERO);
      
      RenderPolygon=RenderPolygonSinglePass;
    }

  GlideLib_grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
                          GR_COMBINE_FACTOR_NONE,
                          GR_COMBINE_LOCAL_CONSTANT, 
                          GR_COMBINE_OTHER_NONE,FXFALSE);
  GlideLib_grColorCombine(GR_COMBINE_FUNCTION_BLEND,                    // COLOR COMBINE
                          GR_COMBINE_FACTOR_LOCAL,GR_COMBINE_LOCAL_ITERATED,
                          GR_COMBINE_OTHER_TEXTURE,FXFALSE);

  m_nDrawMode = 0;

  return S_OK;

OnError:
  
  if (FAILED(hRes))
    FINAL_RELEASE(m_piG2D);
  
  FINAL_RELEASE(pGraphicsInfo);
  
#if defined (OS_WIN32)
  FINAL_RELEASE(pSysGInfo);
#endif
     
  return hRes;  
}

STDMETHODIMP csGraphics3DGlide2x::Close()
{
  ClearCache();

  GlideLib_grSstWinClose();
  return S_OK;
}

STDMETHODIMP csGraphics3DGlide2x::GetColormapFormat( G3D_COLORMAPFORMAT& g3dFormat ) 
{
  if (use16BitTexture)
    g3dFormat = G3DCOLORFORMAT_PRIVATE;
  else
    g3dFormat = G3DCOLORFORMAT_GLOBAL;
  return S_OK;
}

STDMETHODIMP csGraphics3DGlide2x::SetDimensions (int width, int height)
{
  m_nWidth = width;
  m_nHeight = height;
  m_nHalfWidth = width/2;
  m_nHalfHeight = height/2;
  GlideLib_grClipWindow(0, 0, width, height);

  return S_OK;
}

csGraphics3DGlide2x::~csGraphics3DGlide2x()
{
  GlideLib_grGlideShutdown();

  FINAL_RELEASE( m_pCamera );
  FINAL_RELEASE( m_piG2D );
  FINAL_RELEASE( m_piSystem );
}

STDMETHODIMP csGraphics3DGlide2x::BeginDraw (int DrawFlags)
{
  if (DrawFlags & CSDRAW_2DGRAPHICS)
    {
      // if graphics is in 3D mode, turn it off
      if (m_nDrawMode & CSDRAW_3DGRAPHICS)
        FinishDraw ();
      // if 2D mode is not enabled, turn it on
      if (!(m_nDrawMode & CSDRAW_2DGRAPHICS))
        m_piG2D->BeginDraw ();
    }
  else if (DrawFlags & CSDRAW_3DGRAPHICS)
    {
      // if graphics is in 2D mode, turn it off
      if (m_nDrawMode & CSDRAW_2DGRAPHICS)
        FinishDraw ();
      
      // if 3D mode is not enabled, turn it on
      if (!(m_nDrawMode & CSDRAW_3DGRAPHICS))
        {      
          GlideLib_grBufferClear(0,0,GR_WDEPTHVALUE_FARTHEST);
          
        } /* endif */
    } /* endif */
  m_nDrawMode = DrawFlags;
  return S_OK;
}

/// End the frame
STDMETHODIMP csGraphics3DGlide2x::FinishDraw ()
{
  if (m_nDrawMode & CSDRAW_2DGRAPHICS)
                m_piG2D->FinishDraw ();

  m_nDrawMode = 0;
  return S_OK;
}

/// do the page swap.
STDMETHODIMP csGraphics3DGlide2x::Print(csRect* rect)
{
   // we need to tell the glide2d driver to update the screen...
  m_piG2D->Print (rect);



  if(m_bVRetrace)
    GlideLib_grBufferSwap(1);
  else
    GlideLib_grBufferSwap(0);
  return S_OK;
}

/// Set the mode for the Z buffer (functionality also exists in SetRenderState).
STDMETHODIMP csGraphics3DGlide2x::SetZBufMode (ZBufMode mode)
{
  if (mode==m_ZBufMode) 
    return S_OK;
  
  m_ZBufMode = mode;
  
  if (mode & ZBuf_Test)
    GlideLib_grDepthMask(FXFALSE);
  else
    GlideLib_grDepthMask(FXTRUE);    
  
  if (mode & ZBuf_Fill)      // write-only
    GlideLib_grDepthBufferFunction(GR_CMP_ALWAYS);    
  else 
    GlideLib_grDepthBufferFunction(GR_CMP_LEQUAL);    
  
  return S_OK;

}

#define SNAP (( float ) ( 3L << 18 ))

void csGraphics3DGlide2x::RenderPolygonSinglePass (GrVertex * verts, int num, bool haslight,TextureHandler*text,TextureHandler*light,bool is_transparent)
{
  int i;
  for(i=0;i<num;i++) 
    {
      verts[i].tmuvtx[0].sow *= text->width*verts[i].oow;
      verts[i].tmuvtx[0].tow *= text->height*verts[i].oow;
    }
  if(text)
    GlideLib_grTexSource(text->tmu->tmu_id, text->loadAddress,
                         GR_MIPMAPLEVELMASK_BOTH, &text->info);
  if(haslight && light)
    {
      for(i=0;i<num;i++)
        {
          verts[i].tmuvtx[1].sow *= light->width*verts[i].oow; 
          verts[i].tmuvtx[1].tow *= light->height*verts[i].oow;
        }
      GlideLib_grTexSource(light->tmu->tmu_id, light->loadAddress,
                           GR_MIPMAPLEVELMASK_BOTH, &light->info);
      
      GlideLib_grTexCombine(text->tmu->tmu_id,
                            GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
                            //GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, GR_COMBINE_FACTOR_ONE,
                            GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_NONE,
                            FXFALSE,FXFALSE);
       
    }   
  else
    {
      GlideLib_grTexCombine(text->tmu->tmu_id,
                            GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
                            GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_NONE,
                            FXFALSE,FXFALSE);
    }

       
 /*if(!haslight)
  {
    GlideLib_grTexCombine(0,
      GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_NONE,
      FXFALSE,FXFALSE);
  }
 */
   GlideLib_grDrawPlanarPolygonVertexList(num,verts);
}

void csGraphics3DGlide2x::RenderPolygonMultiPass (GrVertex * verts, int num, bool haslight,TextureHandler*text,TextureHandler*light,bool is_transparent)
{
  int i;
  for(i=0;i<num;i++)
    {
      verts[i].tmuvtx[0].sow *= text->width*verts[i].oow;
      verts[i].tmuvtx[0].tow *= text->height*verts[i].oow;
    }

  if(is_transparent)
    GlideLib_grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA,
                                  GR_BLEND_ONE, GR_BLEND_ZERO);

  if(text)
    GlideLib_grTexSource(text->tmu->tmu_id, text->loadAddress,
                         GR_MIPMAPLEVELMASK_BOTH, &text->info);

  GlideLib_grDrawPlanarPolygonVertexList(num,verts);

  if(haslight && light)
    {
      for(i=0;i<num;i++)
        {
          verts[i].tmuvtx[0].sow= verts[i].tmuvtx[1].sow * light->width*verts[i].oow; 
          verts[i].tmuvtx[0].tow= verts[i].tmuvtx[1].tow * light->height*verts[i].oow; 
        }

      GlideLib_grAlphaBlendFunction(GR_BLEND_DST_COLOR, GR_BLEND_ZERO,
                                    GR_BLEND_ZERO, GR_BLEND_ZERO);
      
      GlideLib_grTexClampMode(light->tmu->tmu_id,
                              GR_TEXTURECLAMP_CLAMP,GR_TEXTURECLAMP_CLAMP);      // TEXTURE CLAMP
      
      GlideLib_grTexSource(light->tmu->tmu_id, light->loadAddress,
                           GR_MIPMAPLEVELMASK_BOTH, &light->info);

      GlideLib_grDrawPlanarPolygonVertexList(num,verts);
      
      GlideLib_grTexClampMode(light->tmu->tmu_id,
                              GR_TEXTURECLAMP_WRAP,GR_TEXTURECLAMP_WRAP);        // TEXTURE WRAP

      if(!is_transparent)
        GlideLib_grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO,
                                      GR_BLEND_ONE, GR_BLEND_ZERO);
    }
  if(is_transparent)
    GlideLib_grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO,
                                  GR_BLEND_ONE, GR_BLEND_ZERO);
  
}

void csGraphics3DGlide2x::SetupPolygon( G3DPolygon& poly, float& J1, float& J2, float& J3, 
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
    ComcsVector3 vcam_0;
    
    poly.polygon->GetCameraVector(0, &vcam_0);
    
    M = 0;
    N = 0;
    O = 1/vcam_0.z;
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

STDMETHODIMP csGraphics3DGlide2x::DrawPolygon(G3DPolygon& poly)
{
        GrVertex * verts;

  if(!poly.polygon)
    return E_INVALIDARG;

  if (poly.num < 3) 
  {
    return E_INVALIDARG;
  }

        bool lm_exists=true;
        bool is_transparent = false;
        bool is_colorkeyed = false;
  int  poly_alpha;
  IPolygonTexture* pTex;
        float J1, J2, J3, K1, K2, K3;
        float M, N, O;
  
  // set up the geometry.
  SetupPolygon( poly, J1, J2, J3, K1, K2, K3, M, N, O );

  // retrieve the texture.
  poly.polygon->GetTexture(0, &pTex);

  
  csTextureMMGlide* txt_mm = (csTextureMMGlide*)GetcsTextureMMFromITextureHandle (poly.txt_handle);
  //csTexture* txt_unl = txt_mm->get_texture (mipmap);
  

  if (!pTex)
     return E_INVALIDARG;

  CacheTexture (pTex);

  poly.polygon->GetAlpha( poly_alpha );
  is_transparent = poly_alpha ? true : false;

  //HighColorCacheAndManage_Data* tcache;
  //HighColorCacheAndManage_Data* lcache;
  HighColorCache_Data* tcache;
  HighColorCache_Data* lcache;
  
  // retrieve the cached texture handle.
  tcache = txt_mm->get_hicolorcache();
  ASSERT( tcache );
        
  //FINAL_RELEASE( piTM );
  //FINAL_RELEASE( piMMC );
  
  // retrieve the lightmap from the cache.
  ILightMap* piLM = NULL;
  pTex->GetLightMap( &piLM );
  
  if ( piLM )
  {
    piLM->GetHighColorCache((HighColorCache_Data **) &lcache );
    if (lcache==NULL)
    {
      lm_exists = false;
    }
    
  }
  else
  {
    lm_exists=false;
  }

  if(is_transparent)
  {
    GrColor_t c = 0x00FFFFFF;
    c |= ((int)((float)(poly_alpha)/100.0f*255.0f) << 24);
    GlideLib_grConstantColorValue(c);
  }
  else
  {
      GlideLib_grConstantColorValue(0xFFFFFFFF);
  }
        
  verts=new GrVertex[poly.num];
  float q,x,y,ooz,z,u,v,lu,lv;
  int i;
  TextureHandler *thLm =NULL, 
                 *thTex = (TextureHandler *)tcache->pData;
                       
/*      if (poly.polygon->theDynLight) // Use a dynamic light if it exists
                q = (float)((poly.polygon->theDynLight->RawIntensity() * 256) >> 16);
        else*/
  q = 255;
  for(i=0;i<poly.num;i++)
    {
      x = poly.vertices[i].sx;
      y = poly.vertices[i].sy;
      verts[i].x = x + SNAP;
      verts[i].y =/* FRAME_HEIGHT -1 - */y + SNAP; 
      x-=m_nHalfWidth;
      y-=m_nHalfHeight;
      ooz = (M*(x) + N*(y) + O);
      verts[i].z = z = 1/ooz;
      u = (J1 * (x) + J2 * (y) + J3)*z;
      v = (K1 * (x) + K2 * (y) + K3)*z;
      verts[i].tmuvtx[0].sow= u; 
      verts[i].tmuvtx[0].tow= v; 
      verts[i].oow /*= verts[i].tmuvtx[0].oow = verts[i].tmuvtx[1].oow */= ooz;
      verts[i].r = q;
      verts[i].g = q;
      verts[i].b = q;
      //verts[i].a = poly.polygon->get_alpha(); // Not used
      //verts[i].x -= SNAP;  // You can forget it
      //verts[i].y -= SNAP;  // This one also
    }

  if(lm_exists)
    {
      thLm = (TextureHandler *)lcache->pData;
      
      float fMinU, fMinV, fMaxU, fMaxV;
      
      pTex->GetTextureBox( fMinU, fMinV, fMaxU, fMaxV );
      
      //thLm = (TextureHandler *)lcache->pData;
      float lu_dif, lv_dif,lu_scale, lv_scale;
      
      // gsteenss: following few lines were added to scale the lightmaps 
      // correctly (?)  
      int lmwidth, lmrealwidth, lmheight, lmrealheight;
      float scale_u,scale_v;
 
      piLM->GetWidth (lmwidth);
      piLM->GetRealWidth (lmrealwidth);
      piLM->GetHeight (lmheight);
      piLM->GetRealHeight (lmrealheight);
      
      //piLM->Release();
      FINAL_RELEASE(piLM);
      
      scale_u = (float)(lmrealwidth-1) / (float)lmwidth;
      scale_v = (float)(lmrealheight-1) / (float)lmheight;
      
      lu_dif = fMinU;
      lv_dif = fMinV;
      lu_scale = scale_u/(fMaxU - fMinU);
      lv_scale = scale_v/(fMaxV - fMinV);

      
      for(i=0;i<poly.num;i++)
        {
          lu = (verts[i].tmuvtx[0].sow- lu_dif) * lu_scale;
          lv = (verts[i].tmuvtx[0].tow- lv_dif) * lv_scale;
          verts[i].tmuvtx[1].sow= lu; 
          verts[i].tmuvtx[1].tow= lv; 
        }
    }
  
  if(is_colorkeyed)
    GlideLib_grChromakeyMode(GR_CHROMAKEY_ENABLE);
  
  RenderPolygon(verts,poly.num,lm_exists,thTex,thLm,is_transparent);
  
  if(is_colorkeyed)
    GlideLib_grChromakeyMode(GR_CHROMAKEY_DISABLE);
  
  if(is_transparent)
    GlideLib_grConstantColorValue(0xFFFFFFFF);
  
  delete[] verts;

  return S_OK;
}

/// Draw a projected (non-perspective correct) polygon.
STDMETHODIMP csGraphics3DGlide2x::DrawPolygonQuick (G3DPolygon& poly, bool gouroud)
{
  //  HighColorCacheAndManage_Data* tcache=NULL;
  HighColorCache_Data* tcache=NULL;

  if (poly.num < 3) return S_OK;

  csTextureMMGlide* txt_mm = (csTextureMMGlide*)GetcsTextureMMFromITextureHandle (poly.txt_handle);

  m_pTextureCache->Add(poly.txt_handle);
  tcache=txt_mm->get_hicolorcache();

  ASSERT( tcache );
        
  TextureHandler *thTex = (TextureHandler *)tcache->pData;

  if(thTex)
  {
    CHK(GrVertex *verts = new GrVertex[poly.num]);
    
    float x, y;
    for(int i=0; i<poly.num; i++)
    {
      x = poly.vertices[i].sx;
      y = poly.vertices[i].sy;
      verts[i].x = x + SNAP;
      verts[i].y = y + SNAP;
      x-=m_nHalfWidth;
      y-=m_nHalfHeight;
      if(gouroud)
      {
        verts[i].r = poly.pi_texcoords[i].r*255;
        verts[i].g = poly.pi_texcoords[i].g*255;
        verts[i].b = poly.pi_texcoords[i].b*255;
      }
      else
      {
        verts[i].r = 255;
        verts[i].g = 255;
        verts[i].b = 255;
      }
      verts[i].oow = poly.pi_texcoords[i].z;
      verts[i].x -= SNAP;
      verts[i].y -= SNAP;
      verts[i].tmuvtx[1].sow = verts[i].tmuvtx[0].sow = poly.pi_texcoords[i].u*thTex->width*verts[i].oow;
      verts[i].tmuvtx[1].tow = verts[i].tmuvtx[0].tow = poly.pi_texcoords[i].v*thTex->height*verts[i].oow;
    }
    
    GlideLib_grTexSource(thTex->tmu->tmu_id, thTex->loadAddress,
      GR_MIPMAPLEVELMASK_BOTH, &thTex->info);
    
    if(!m_iMultiPass)
    {
      GlideLib_grTexCombine(m_TMUs[0].tmu_id,
        GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
        GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_NONE,
        FXFALSE,FXFALSE);
    }

    GlideLib_grDrawPlanarPolygonVertexList(poly.num, verts);

    if(!m_iMultiPass)
    {
      GlideLib_grTexCombine(m_TMUs[0].tmu_id,
        GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
        GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_NONE,
        FXFALSE,FXFALSE);
    }
    
    delete[] verts;
  }

  return S_OK;
}

/// Give a texture to csGraphics3D to cache it.
STDMETHODIMP csGraphics3DGlide2x::CacheTexture (IPolygonTexture *texture)
{
  ITextureHandle* txt_handle = NULL;

  texture->GetTextureHandle (&txt_handle);
  m_pTextureCache->Add (txt_handle);
  m_pLightmapCache->Add (texture);
  return S_OK;  
}

/// Release a texture from the cache.
STDMETHODIMP csGraphics3DGlide2x::UncacheTexture (IPolygonTexture *piPT)
{
  (void)piPT;
  return E_NOTIMPL;
}

/// Dump the texture cache.
STDMETHODIMP csGraphics3DGlide2x::DumpCache(void)
{
  m_pTextureCache->Dump();
  m_pLightmapCache->Dump();
  return S_OK;
}

/// Clear the texture cache.
STDMETHODIMP csGraphics3DGlide2x::ClearCache(void)
{
  if(m_pTextureCache) m_pTextureCache->Clear();
  if(m_pLightmapCache) m_pLightmapCache->Clear();
  return S_OK;
}

STDMETHODIMP csGraphics3DGlide2x::DrawFltLight(G3DFltLight& light)
{
    return E_NOTIMPL;
}

STDMETHODIMP csGraphics3DGlide2x::GetCaps(G3D_CAPS *caps)
{
  if (!caps)
    return E_INVALIDARG;

  memcpy(caps, &m_Caps, sizeof(G3D_CAPS));

  return S_OK;
}

STDMETHODIMP csGraphics3DGlide2x::DrawLine (csVector3& v1, csVector3& v2, float fov, int color)
{
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

  float iz1 = fov/z1;
  int px1 = QInt (x1 * iz1 + (m_nWidth/2));
  int py1 = m_nHeight - 1 - QInt (y1 * iz1 + (m_nHeight/2));
  float iz2 = fov/z2;
  int px2 = QInt (x2 * iz2 + (m_nWidth/2));
  int py2 = m_nHeight - 1 - QInt (y2 * iz2 + (m_nHeight/2));

  m_piG2D->DrawLine (px1, py1, px2, py2, color);

  return S_OK;
}

STDMETHODIMP csGraphics3DGlide2x::SetRenderState(G3D_RENDERSTATEOPTION option, long value)
{
  switch (option)
    {
    case G3DRENDERSTATE_NOTHING:
      return S_OK;
      
    case G3DRENDERSTATE_ZBUFFERTESTENABLE:
      if (value)
        GlideLib_grDepthBufferFunction(GR_CMP_LEQUAL);
      else
        GlideLib_grDepthBufferFunction(GR_CMP_ALWAYS);
      break;
      
    case G3DRENDERSTATE_ZBUFFERFILLENABLE:
      if (value)
        GlideLib_grDepthMask(FXTRUE);
      else
        GlideLib_grDepthMask(FXFALSE);
      break;
    
    case G3DRENDERSTATE_DITHERENABLE:
/*
    if(value)
      m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE);
    else
      m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE);
*/
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
  case G3DRENDERSTATE_EDGESENABLE:
    rstate_edges = value;
    break;
  case G3DRENDERSTATE_DEBUGENABLE :
  case G3DRENDERSTATE_LIGHTFRUSTRUMENABLE :
  case G3DRENDERSTATE_FILTERINGENABLE :
  case G3DRENDERSTATE_PERFECTMAPPINGENABLE :
  case G3DRENDERSTATE_LIGHTINGENABLE :
  case G3DRENDERSTATE_INTERLACINGENABLE :
  case G3DRENDERSTATE_MMXENABLE :
    break;
  default:
    return E_INVALIDARG;
  }
  
  return S_OK;
}

STDMETHODIMP csGraphics3DGlide2x::GetRenderState(G3D_RENDERSTATEOPTION op, long& retval)
{
  switch(op)
  {
    case G3DRENDERSTATE_NOTHING:
      retval = 0;
      break;
    case G3DRENDERSTATE_ZBUFFERTESTENABLE:
      retval = (bool)(m_ZBufMode & ZBuf_Test);
      break;
    case G3DRENDERSTATE_ZBUFFERFILLENABLE:
      retval = (bool)(m_ZBufMode & ZBuf_Fill);
      break;
    case G3DRENDERSTATE_DITHERENABLE:
      retval = false;
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
    case G3DRENDERSTATE_EDGESENABLE:
      retval = rstate_edges;
      break;
    default:
      retval = 0;
      return E_INVALIDARG;
  }
  
  return S_OK;
}

STDMETHODIMP csGraphics3DGlide2x::GetZBufPoint(int, int, unsigned long** retval)
{
  *retval = NULL;
  return E_NOTIMPL;
}

STDMETHODIMP csGraphics3DGlide2x::OpenFogObject (CS_ID id, csFog* fog)
{
  return E_NOTIMPL;
}

STDMETHODIMP csGraphics3DGlide2x::AddFogPolygon (CS_ID id, G3DPolygon& poly, int fogtype)
{
  return E_NOTIMPL;
}

STDMETHODIMP csGraphics3DGlide2x::CloseFogObject (CS_ID id)
{
  return E_NOTIMPL;
}


void csGraphics3DGlide2x::SysPrintf(int mode, char* szMsg, ...)
{
  char buf[1024];
  va_list arg;

  va_start (arg, szMsg);
  vsprintf (buf, szMsg, arg);
  va_end (arg);

  m_piSystem->Print(mode, buf);
}

STDMETHODIMP csGraphics3DGlide2x::CreateHalo(float r, float g, float b, HALOINFO* pRetVal)
{
  if(m_bHaloEffect)
  {
    m_piG2D->AddRef();
    csHaloDrawer halo(m_piG2D, r, g, b);
    
    csG3DHardwareHaloInfo* retval = new csG3DHardwareHaloInfo();
    
    unsigned long *lpbuf = halo.GetBuffer();

    CHK(unsigned short *mem = new unsigned short [128*128]);

    // Warning : convertion maybe a bit bugged
    for (int j=0; j<128; j++)
    {
      unsigned short *lpL = &mem[j<<7]; // j * 128
      unsigned long *p = &lpbuf[j<<7]; // j * 128
      for(int i=0; i<128; i++)
      {
        int a, r, g, b;
        a=(*p>>24)>>4;
        r=((*p&0x00FF0000)>>16)>>4;
        g=((*p&0x0000FF00)>>8)>>4;
        b=(*p&0x000000FF)>>4;
        *lpL = (a << 12) | (r << 8) | (g << 4) | b;
        lpL++;
        p++;
      }
    }

    retval->halo = m_pTextureCache->LoadHalo((char *)mem);

    delete [] mem;
    delete [] lpbuf;

    *pRetVal = (HALOINFO)retval;
    return S_OK;
  }
  *pRetVal = NULL;
  return S_FALSE;
}

STDMETHODIMP csGraphics3DGlide2x::DestroyHalo(HALOINFO haloInfo)
{
  if(haloInfo != NULL)
  {
    m_pTextureCache->UnloadHalo(((csG3DHardwareHaloInfo*)haloInfo)->halo);
    delete (csG3DHardwareHaloInfo*)haloInfo;
  }
  return S_OK;
}

STDMETHODIMP csGraphics3DGlide2x::DrawHalo(csVector3* pCenter, float fIntensity, HALOINFO haloInfo)
{
  if(m_bHaloEffect)
  {
    if (haloInfo == NULL)
      return E_INVALIDARG;
    
    if (pCenter->x > m_nWidth || pCenter->x < 0 || pCenter->y > m_nHeight || pCenter->y < 0  ) 
      return S_FALSE;
/*
    int izz = QInt24 (1.0f / pCenter->z);
    HRESULT hRes = S_OK;

    unsigned long zb = z_buffer[(int)pCenter->x + (width * (int)pCenter->y)];

          // first, do a z-test to make sure the halo is visible
    if (izz < (int)zb)
      hRes = S_FALSE;
*/

    GrVertex vx[4];
    
                int ci = fIntensity*255.0f;
    float len = (m_nWidth/6);

    vx[0].a = ci; vx[0].r = ci; vx[0].g = ci; vx[0].b = ci;
    vx[0].x = pCenter->x - len;
    vx[0].y = pCenter->y - len;
    vx[0].z = pCenter->z;
    vx[0].oow = pCenter->z;
    vx[0].tmuvtx[0].sow = 0;
    vx[0].tmuvtx[0].tow = 0;

    vx[1].a = ci; vx[1].r = ci; vx[1].g = ci; vx[1].b = ci;
    vx[1].x = pCenter->x + len;
    vx[1].y = pCenter->y - len;
    vx[1].z = pCenter->z;
    vx[1].oow = pCenter->z;
    vx[1].tmuvtx[0].sow = 128;
    vx[1].tmuvtx[0].tow = 0;

    vx[2].a = ci; vx[2].r = ci; vx[2].g = ci; vx[2].b = ci;
    vx[2].x = pCenter->x + len;
    vx[2].y = pCenter->y + len;
    vx[2].z = pCenter->z;
    vx[2].oow = pCenter->z;
    vx[2].tmuvtx[0].sow = 128;
    vx[2].tmuvtx[0].tow = 128;

    vx[3].a = ci; vx[3].r = ci; vx[3].g = ci; vx[3].b = ci;
    vx[3].x = pCenter->x - len;
    vx[3].y = pCenter->y + len;
    vx[3].z = pCenter->z;
    vx[3].oow = pCenter->z;
    vx[3].tmuvtx[0].sow = 0;
    vx[3].tmuvtx[0].tow = 128;
    
    if(m_iMultiPass)
    {
      GlideLib_grAlphaBlendFunction( GR_BLEND_ONE_MINUS_SRC_COLOR, GR_BLEND_ZERO,
                                    GR_BLEND_DST_ALPHA, GR_BLEND_ZERO);
    }
    else // disable single pass blending
    {
      GlideLib_grTexCombine(m_TMUs[0].tmu_id,
                            GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
                            GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_NONE,
                            FXFALSE,FXFALSE);
    }

    GlideLib_grDepthBufferFunction(GR_CMP_ALWAYS);
    GlideLib_grDepthMask(FXFALSE);

    HighColorCacheAndManage_Data *halo=((csG3DHardwareHaloInfo*)haloInfo)->halo;
    TextureHandler *thTex = (TextureHandler *)halo->pData;
    GlideLib_grTexSource(thTex->tmu->tmu_id, thTex->loadAddress,
                         GR_MIPMAPLEVELMASK_BOTH,
                         &thTex->info);
    
    GlideLib_grDrawPlanarPolygonVertexList(4, vx);
    
    GlideLib_grDepthBufferFunction(GR_CMP_LEQUAL);
    GlideLib_grDepthMask(FXTRUE);

    if(m_iMultiPass)
    {
      GlideLib_grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO,
                                    GR_BLEND_ONE, GR_BLEND_ZERO);
    }
    else // enable single pass blending
    {
      GlideLib_grTexCombine(m_TMUs[0].tmu_id,
                            GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
                            GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_NONE,
                            FXFALSE,FXFALSE);
    }

    return S_OK;
  }
  return S_FALSE;
};

STDMETHODIMP csGraphics3DGlide2x::TestHalo(csVector3* pCenter)
{
  if(m_bHaloEffect)
  {
    if (pCenter->x > m_nWidth || pCenter->x < 0 || pCenter->y > m_nHeight || pCenter->y < 0  ) 
      return S_FALSE;
/*
    int izz = QInt24 (1.0f / pCenter->z);
    HRESULT hRes = S_OK;
      
    unsigned long zb = z_buffer[(int)pCenter->x + (width * (int)pCenter->y)];
        
    // first, do a z-test to make sure the halo is visible
    if (izz < (int)zb)
      hRes = S_FALSE;
*/
    return S_OK;
  }
  return S_FALSE;
};

/////////////

// csHaloDrawer implementation //

csGraphics3DGlide2x::csHaloDrawer::csHaloDrawer(IGraphics2D* piG2D, float r, float g, float b)
{
  mpBuffer = NULL;

  IGraphicsInfo* piGI;
  
  piG2D->QueryInterface(IID_IGraphicsInfo, (void**)&piGI);
  mpiG2D = piG2D;

  piGI->GetWidth(mWidth);
  piGI->GetHeight(mHeight);

  int dim = 128;

  // point variables
  int x=0;
  int y=dim/2;
  // decision variable
  int d = 1 - y;

  mpBuffer = new unsigned long[dim*dim];
  memset(mpBuffer, 0, dim*dim*sizeof(unsigned long));

  mBufferWidth = dim;
  mDim = dim;
  
  mRed = r; mGreen = b; mBlue = b;
  mx = my = dim / 2;

  ////// Draw the outer rim //////

  drawline_outerrim(-y, y, x);
  
  while (true)
  {
    if (d < 0)
      d += 2 * x + 3;
    else
    {
      d += 2 * (x - y) + 5;
      y--;
      if (y <= x)
        break;

      drawline_outerrim(-x, x, y);
      drawline_outerrim(-x, x, -y);
    }
    x++;

    drawline_outerrim(-y, y, x);
    drawline_outerrim(-y, y, -x);
  }

  ////// Draw the inner core /////

  x=0;
  y=dim/3;
  d = 1 - y;

  mDim = dim/1.5;

  mRatioRed = (r - (r/3.f)) / y;
  mRatioGreen = (g - (g/3.f)) / y;
  mRatioBlue = (b - (b/3.f)) / y;

  drawline_innerrim(-y, y, x);
  
  while (true)
  {
    if (d < 0)
      d += 2 * x + 3;
    else
    {
      d += 2 * (x - y) + 5;
      y--;
      if (y <= x)
        break;

      drawline_innerrim(-x, x, y);
      drawline_innerrim(-x, x, -y);
    }
    x++;
    drawline_innerrim(-y, y, x);
    drawline_innerrim(-y, y, -x);
  }

  ///// Draw the vertical lines /////
  
  // DAN: this doesn't look right yet.
#if 0
  int y1, y2;

  // the vertical line has a constant height, 
  // until the halo itself is of a constant height,
  // at which point the vertical line decreases.
  
  y1 = my - mWidth/10;
  y2 = my + mWidth/10;
  
  if (dim < mWidth / 6)
  {
    int q = mWidth/6 - dim;
    y1 += q;
    y2 -= q;
  }

  drawline_vertical(mx, y1, y2);
#endif

  FINAL_RELEASE(piGI);
}

csGraphics3DGlide2x::csHaloDrawer::~csHaloDrawer()
{
  FINAL_RELEASE(mpiG2D);
}

void csGraphics3DGlide2x::csHaloDrawer::drawline_vertical(int x, int y1, int y2)
{
  int i;
  unsigned long* buf;

  int r = (int)(mRed/2.5f * 256.0f);
  int g = (int)(mGreen/2.5f * 256.0f);
  int b = (int)(mBlue/2.5f * 256.0f);

  int c = (r << 16) | (g << 8) | b;

  while (y1 < y2)
  {
    buf = &mpBuffer[(mx-1) + (mBufferWidth * y1++)];
    
    for(i=0; i<3; i++)
    {
      buf[i] = c;    
    }
  }
}

void csGraphics3DGlide2x::csHaloDrawer::drawline_outerrim(int x1, int x2, int y)
{
  if (x1 == x2) return;

  int r = (int)(mRed / 3.5f * 256.0f);
  int g = (int)(mGreen / 3.5f * 256.0f);
  int b = (int)(mBlue / 3.5f * 256.0f);
 
  int a = QInt((r + g + b) / 3);

  // stopx makes sure we don't overrdraw when drawing the inner core. 
  // maybe there's something faster than a sqrt... - DAN
  // @@@ JORRIT: had to make some changes to prevent overflows!
  float sq = (mDim/3.0)*(mDim/3.0) - ((double)y*(double)y);
  int stopx = 0;
  if (sq > 0) stopx = (int)sqrt (sq);

  unsigned long* bufy;

  x1 += mx;
  x2 += mx;
  y += my;

  bufy = &mpBuffer[y * mBufferWidth];

//  unsigned short p;

  if (stopx)
  {    
    while (x1 <= (mx - stopx) + 2)
    { 
      bufy[x1++] = (a << 24) | (r << 16) | (g << 8) | b;
    }

    x1 = mx + stopx - 2;
    
    while (x1 <= x2)
    { 
      bufy[x1++] = (a << 24) | (r << 16) | (g << 8) | b;
    }
  }
  else
  {
    while (x1 <= x2)
    {
      bufy[x1++] = (a << 24) | (r << 16) | (g << 8) | b;
    }  
  }
}

void csGraphics3DGlide2x::csHaloDrawer::drawline_innerrim(int x1, int x2, int y)
{
  float w2 = x2 - x1;
  unsigned long* bufy;

  x1 += mx;
  x2 += mx;
  y += my;

  if (y >= mHeight || y <= 0) return;
  bufy = &mpBuffer[y * mBufferWidth];

  if (w2 == 0.0f) return;
  w2 /= 2.0f;

  int halfx = x1 + w2;

  float ir, ig, ib, ia;

  float rlow = mRed / 4.5f;
  float glow = mGreen / 4.5f;
  float blow = mBlue / 4.5f;
  
  if (y <= my)
  {
    int iy = y - (my - (mDim / 2));

    ir = (iy * mRatioRed + rlow) * 256;
    ig = (iy * mRatioGreen + glow) * 256;
    ib = (iy * mRatioBlue + blow) * 256;    
    ia = (ir + ig + ib) / 3.0f;     
  }
  else
  {
    int iy = (my + (mDim/2)) - y;

    ir = (iy * mRatioRed + rlow) * 256;
    ig = (iy * mRatioGreen + glow) * 256;
    ib = (iy * mRatioBlue + blow) * 256;
    ia = (ir + ig + ib) / 3.0f;
  }

  float r = rlow * 256;
  float g = glow * 256;
  float b = blow * 256;
  float a = (r + g + b) / 3.0f;

  if (a < 0) a = 0;
 
  if (ir > 245) ir = 245;
  if (ig > 245) ig = 245;
  if (ib > 245) ib = 245;
  if (ia > 250) ia = 250;

  float rdelta = (ir - r) / w2;
  float gdelta = (ig - g) / w2;
  float bdelta = (ib - b) / w2;
  float adelta = (ia - a) / w2;
 
  int br, bg, bb;

  unsigned short p;
  int inta;

  while (x1 <= halfx)
  {
    p = bufy[x1];
    
    inta = QInt(a);

    br = QInt(r);
    bg = QInt(g);
    bb = QInt(b);

    bufy[x1++] = (inta << 24) | (br << 16) | (bg << 8) | bb;

    r += rdelta; g+=gdelta; b+=bdelta; a+=adelta;
  }

  while(x1 <= x2)
  {
    p = bufy[x1];
    
    inta = QInt(a);

    br = QInt(r);
    bg = QInt(g);
    bb = QInt(b);

    bufy[x1++] = (inta << 24) | (br << 16) | (bg << 8) | bb;

    r -= rdelta; g -= gdelta; b -= bdelta; a -= adelta;
  }
}
