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

#include "sysdef.h"

#if defined(OS_WIN32)
#include <windows.h>
#endif
#if defined(OS_MACOS)
#include <Dialogs.h>
#include <TextUtils.h>
#define GLIDE24_ONLY
#endif
#include <stdlib.h>
#include <stdarg.h>

#include "isystem.h"
#include "itexture.h"
#include "itxtmgr.h"
#include "ilghtmap.h"
#include "igraph2d.h"

#include "csgeom/plane3.h"
#include "csutil/inifile.h"
#include "csutil/scanstr.h"
#include "qint.h"

#if defined (OS_WIN32)
#include "cs2d/winglide2/g2d.h"
#include "cs2d/winglide2/ig2d.h"
#elif defined (OS_MACOS)
#include "cs2d/macglide2/g2d.h"
#include "cs2d/macglide2/ig2d.h"
#include "cssys/mac/MacRSRCS.h"
#else //Is there another platform Glide runs on?
#endif

#include "gllib2.h"
#include "g3dgl2.h"
#include "glalpha2.h"

//
// Interface table definition
//

#define SysPrintf m_piSystem->Printf

IMPLEMENT_FACTORY (csGraphics3DGlide2x)

EXPORT_CLASS_TABLE (glide23d)
  EXPORT_CLASS (csGraphics3DGlide2x, "crystalspace.graphics3d.glide.2",
    "Glide v2 3D graphics driver for Crystal Space")
EXPORT_CLASS_TABLE_END

IMPLEMENT_IBASE (csGraphics3DGlide2x)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iGraphics3D)
IMPLEMENT_IBASE_END


#define GLIDE_FX_VERTSIZE 10

// Error Message handling
void sys_fatalerror( char* thestr, int hRes=0 )
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
  (void)hRes;
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
  (void)hRes;
  fprintf(stderr, "FATAL ERROR: %s", thestr);
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
  if(config->GetYesNo("Glide2x","FORCEMULTIPASS",FALSE)&& m_iMultiPass==false)
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
      m_TMUs[0].maxAddress = GlideLib_grTexMaxAddress(0);
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

csGraphics3DGlide2x::csGraphics3DGlide2x(iBase* iParent) :
    m_pTextureCache(NULL),
    m_pLightmapCache(NULL),
    m_pAlphamapCache(NULL)
{
  CONSTRUCT_IBASE (iParent);

  m_piSystem = NULL;
  config = new csIniFile ("glide2x.cfg");
  m_pCamera = NULL;
  m_thTex = NULL;
  m_verts = NULL;
  m_vertsize = 0;
  m_dpverts = NULL;
  m_dpvertsize = 0;
  poly_alpha = -1;
  poly_fog = false;
  clipper = NULL;
  m_piGlide2D = NULL;
  
  // default
  m_Caps.CanClip = false;
  m_Caps.minTexHeight = 2;
  m_Caps.minTexWidth = 2;
  m_Caps.maxTexHeight = 256;
  m_Caps.maxTexWidth = 256;
  m_Caps.fog = G3DFOGMETHOD_VERTEX;
  m_Caps.NeedsPO2Maps = true;
  m_Caps.MaxAspectRatio = 8;
  
  m_renderstate.dither = false;
  m_renderstate.bilinearmap = true;
  m_renderstate.trilinearmap = false;
  m_renderstate.gouraud = true;
  m_renderstate.textured = true;
  m_renderstate.alphablend = true;
  m_renderstate.mipmap = true;
  m_renderstate.lighting = true;
}

csGraphics3DGlide2x::~csGraphics3DGlide2x()
{
  GlideLib_grGlideShutdown();

  if (clipper)
    delete clipper;
  if (m_verts)
    delete [] m_verts;
  if (m_pTextureCache)
    m_pTextureCache->DecRef ();
  if (m_pLightmapCache)
    m_pLightmapCache->DecRef ();
  if (m_pAlphamapCache)
    m_pAlphamapCache->DecRef ();
  if (config)
    CHKB (delete config);
  if (txtmgr)
    CHKB (delete txtmgr);
  if (m_pCamera)
    m_pCamera->DecRef ();
  if (m_piGlide2D)
    m_piGlide2D->DecRef ();
  if (m_piG2D)
    m_piG2D->DecRef ();
  if (m_piSystem)
    m_piSystem->DecRef ();
}

bool csGraphics3DGlide2x::Initialize (iSystem *iSys)
{
  (m_piSystem = iSys)->IncRef ();

  SysPrintf (MSG_INITIALIZATION, "\nGlideRender Glide2x selected\n");

  m_piG2D = LOAD_PLUGIN (m_piSystem, GLIDE_2D_DRIVER, NULL, iGraphics2D);
  if (!m_piG2D)
    return false;

  CHK (txtmgr = new csTextureManagerGlide (m_piSystem, m_piG2D, config));

  m_bVRetrace = config->GetYesNo("Glide2x","VRETRACE",FALSE);
  // tell the 2D driver whether to wait for VRETRACE
  m_piGlide2D  = QUERY_INTERFACE ( m_piG2D, iGraphics2DGlide );
  if (!m_piGlide2D){
      SysPrintf ( MSG_INITIALIZATION, "\nCould not set VRETRACE\n");
  }
  else{
      SysPrintf ( MSG_INITIALIZATION, "\nVRETRACE is %s\n", m_bVRetrace ? "on" : "off" );
      m_piGlide2D->SetVRetrace( m_bVRetrace );
  }
  
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

  m_bHaloEffect=config->GetYesNo("Glide2x","DISABLE_HALO", false);
  if (m_bHaloEffect)
    SysPrintf (MSG_INITIALIZATION, " Disable Halo Effect support.\n");

  // generate fogtable
  guFogGenerateExp( fogtable, 0.02f );

  // if a board sports only one TMU all kind of textures go into this one
  // otherwise plain textures and alphamaps are in the first TMU and lightmaps in the 2nd
  FixedTextureMemoryManager *pTMM = new FixedTextureMemoryManager (m_TMUs[0].memory_size);
  CHK (m_pTextureCache = new csGlideTextureCache ( &m_TMUs[0], 16, pTMM ));
  if ( m_iMultiPass )
  {
    m_pLightmapCache = m_pTextureCache;
    m_pTextureCache->IncRef ();
  }
  else
  {
    pTMM = new FixedTextureMemoryManager (m_TMUs[1].memory_size);
    CHK (m_pLightmapCache = new csGlideTextureCache ( &m_TMUs[1], 16, pTMM ));
  }
  m_pAlphamapCache = m_pTextureCache;
  m_pTextureCache->IncRef ();

  return true;
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
  for(i=1;(unsigned)i<SIZEOFRESSTRUCT;i++)
    if ( (width==StatGlideRes[i].width) && (height==StatGlideRes[i].height) )
      return i;
  return -1;
}

bool csGraphics3DGlide2x::Open(const char* Title)
{
  FxU32 hwnd=0;
  GrScreenResolution_t iRes;

  // we have to force 640x480 resoltuions on boards with TMU < 4MB to enable Depthbuffering
  if ( m_piG2D->GetWidth () > 640 && m_iMultiPass && m_TMUs[0].memory_size < (2<<21) )
  {
    SysPrintf (MSG_INITIALIZATION, "  Forcing 640x480 resolution to enable Depthbuffer on this system\n");
    if ( !m_piGlide2D )
    {
       SysPrintf (MSG_INITIALIZATION, "  Yikes, cannot force, because 2D driver handle is NULL... exiting\n");
       return false;
    }
    m_piGlide2D->ForceResolution( 640, 480 );
  }

  m_nWidth = m_piG2D->GetWidth ();
  m_nHalfWidth = m_nWidth/2;
  
  // Open the 2D driver.
  if (!m_piG2D->Open (Title))
    return false;

#if defined(OS_WIN32)
  pSysGInfo->GethWnd (&w);
  if(w)
    hwnd=(FxU32)w;
#endif

  if (use16BitTexture)
    SysPrintf (MSG_INITIALIZATION, "  Use 16 bit textures\n");

  m_nWidth = m_piG2D->GetWidth ();
  m_nHalfWidth = m_nWidth/2;
  
  m_nHeight = m_piG2D->GetHeight ();
  m_nHalfHeight = m_nHeight/2;

  iRes=::getResolutionIndex (m_nWidth, m_nHeight);
  if (iRes==-1)
    sys_fatalerror ("csGraphics3DGlide2x::Open() Invalid Resolution !");
  
  // We should find a way to allow to change the refresh rate        
  if (!GlideLib_grSstWinOpen (hwnd, StatGlideRes[iRes].res,GR_REFRESH_60Hz, 
                GR_COLORFORMAT_ARGB,GR_ORIGIN_LOWER_LEFT,2,1))
    sys_fatalerror ("csGraphics3DGlide2x::Open() : Could not open Window !");
  

  m_piG2D->DoubleBuffer (true);        // RENDER IN BACKBUFFER
  GlideLib_grColorMask (FXTRUE,FXFALSE);                 // DISABLE ALPHA BUFFER
  GlideLib_grDepthMask (FXTRUE);                                 // ENABLE ZBUFFER
  GlideLib_grDepthBufferMode (GR_DEPTHBUFFER_WBUFFER); // ENABLE WBUFFER
  GlideLib_grDepthBufferFunction (GR_CMP_LEQUAL);                // WBUFFER FUNCTION
  
  GlideLib_grBufferClear (0,0,GR_WDEPTHVALUE_FARTHEST); // CLEAR BACKBUFFER
  GlideLib_grBufferSwap (0);                                                     // PUT BACKBUFFER TO FRONT
  GlideLib_grCullMode (GR_CULL_DISABLE);                         // CULL POSITIVE 
  GlideLib_grChromakeyValue (0x0000);
        
  if (m_iMultiPass)
  { // This card has only one TMU, enable Multipass rendering   
    GlideLib_grTexCombine (m_TMUs[0].tmu_id,
                           GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
                           GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
                           FXFALSE,FXFALSE);
    
    GlideLib_grAlphaBlendFunction (GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ONE, GR_BLEND_ZERO);
    GlideLib_grTexClampMode (m_TMUs[0].tmu_id, GR_TEXTURECLAMP_WRAP,GR_TEXTURECLAMP_WRAP);
    GlideLib_grTexFilterMode (m_TMUs[0].tmu_id, GR_TEXTUREFILTER_BILINEAR,GR_TEXTUREFILTER_BILINEAR);

    GlideLib_grTexMipMapMode (m_TMUs[0].tmu_id,GR_MIPMAP_NEAREST,FXFALSE);
    RenderPolygon=RenderPolygonMultiPass;
  }
  else
  { // This card has several TMUs, enable Singlepass rendering

    // Setup 1st TMU
    GlideLib_grTexCombine (m_TMUs[0].tmu_id,
                           GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
                           GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_NONE,
                           FXFALSE,FXFALSE);   // TMU INIT (SCALED BY TMU1)
    GlideLib_grTexClampMode (m_TMUs[0].tmu_id,
                             GR_TEXTURECLAMP_WRAP,GR_TEXTURECLAMP_WRAP);
    GlideLib_grTexFilterMode (m_TMUs[0].tmu_id,
                              GR_TEXTUREFILTER_BILINEAR,GR_TEXTUREFILTER_BILINEAR);
    GlideLib_grTexMipMapMode (m_TMUs[0].tmu_id,GR_MIPMAP_NEAREST,FXFALSE); 

    // Setup 2nd TMU      
    GlideLib_grTexCombine (m_TMUs[1].tmu_id,
                           GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
                           GR_COMBINE_FUNCTION_ZERO,GR_COMBINE_FACTOR_NONE,
                           FXFALSE,FXFALSE);
    GlideLib_grTexClampMode (m_TMUs[1].tmu_id, GR_TEXTURECLAMP_CLAMP,GR_TEXTURECLAMP_CLAMP);
    GlideLib_grTexFilterMode (m_TMUs[1].tmu_id, GR_TEXTUREFILTER_BILINEAR,GR_TEXTUREFILTER_BILINEAR);
    GlideLib_grTexMipMapMode (m_TMUs[1].tmu_id,GR_MIPMAP_NEAREST,FXFALSE);
    GlideLib_grHints (0,GR_STWHINT_ST_DIFF_TMU1);
    GlideLib_grAlphaBlendFunction (GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA,
                                   GR_BLEND_ONE,GR_BLEND_ZERO);
      
    RenderPolygon=RenderPolygonSinglePass;
  }

  GlideLib_grAlphaCombine (GR_COMBINE_FUNCTION_LOCAL,
                           GR_COMBINE_FACTOR_NONE,
                           GR_COMBINE_LOCAL_CONSTANT, 
                           GR_COMBINE_OTHER_NONE,FXFALSE);
  GlideLib_grColorCombine (GR_COMBINE_FUNCTION_BLEND,
                           GR_COMBINE_FACTOR_LOCAL,GR_COMBINE_LOCAL_ITERATED,
                           GR_COMBINE_OTHER_TEXTURE,FXFALSE);

  m_nDrawMode = 0;
 
  return true;
}

void csGraphics3DGlide2x::Close()
{
  ClearCache();
  GlideLib_grSstWinClose();
}

void csGraphics3DGlide2x::SetDimensions (int width, int height)
{
  m_nWidth = width;
  m_nHeight = height;
  m_nHalfWidth = width/2;
  m_nHalfHeight = height/2;
  GlideLib_grClipWindow (0, 0, width, height);
}

void csGraphics3DGlide2x::SetPerspectiveCenter (int x, int y)
{
  m_nHalfWidth = x;
  m_nHalfHeight = y;
}

void csGraphics3DGlide2x::SetClipper (csVector2* vertices, int num_vertices)
{
  CHK (delete clipper);
  clipper = NULL;
  if (!vertices) return;
  
  CHK (clipper = new csPolygonClipper (vertices, num_vertices, false, true));
}

bool csGraphics3DGlide2x::BeginDraw (int DrawFlags)
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
//      ClearBufferUnderTop ();
      GlideLib_grBufferClear (0,0,GR_WDEPTHVALUE_FARTHEST);
          
    } /* endif */
  } /* endif */
  
  m_nDrawMode = DrawFlags;
  return true;
}

void csGraphics3DGlide2x::ClearBufferUnderTop ()
{
    /// clear the screen by rendering a black polygon without disturbing overlays
   GrVertex v[4];
   v[0].x = 0; v[0].y = 0; 
   v[1].x = 0; v[1].y = m_nHeight; 
   v[2].x = m_nWidth; v[2].y = m_nHeight; 
   v[3].x = m_nWidth; v[3].y = 0;
   
   for(int i=0; i < 4; i++ ){
     v[i].oow = 1./GR_WDEPTHVALUE_FARTHEST;
//     v[i].r = v[i].g = v[i].b = 0.0; v[i].a = 0;
   }
   
  GrState state;
  grGlideGetState ( &state );
  GlideLib_grColorCombine (GR_COMBINE_FUNCTION_LOCAL,                    // COLOR COMBINE
                           GR_COMBINE_FACTOR_NONE,GR_COMBINE_LOCAL_CONSTANT,
                           GR_COMBINE_OTHER_NONE,FXFALSE);
   
   grConstantColorValue ( 0xffff0000 ); 

   grDepthMask ( FXTRUE );
   grDepthBufferFunction ( GR_CMP_NOTEQUAL );
   grDepthBufferMode ( GR_DEPTHBUFFER_WBUFFER_COMPARE_TO_BIAS );
   grDepthBiasLevel ( 0 );
   GlideLib_grDrawPlanarPolygonVertexList (4,v);
   // Restore Z-buffer mode
   SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, m_ZBufMode);
   grGlideSetState ( &state );
}

/// End the frame
void csGraphics3DGlide2x::FinishDraw ()
{
  if (m_nDrawMode & CSDRAW_2DGRAPHICS)
    m_piG2D->FinishDraw ();

  m_nDrawMode = 0;
}

void csGraphics3DGlide2x::Print(csRect* rect)
{
  m_piG2D->Print (rect);
}

/// Set the mode for the Z buffer (functionality also exists in SetRenderState).
#define SNAP (( float ) ( 3L << 18 ))

void csGraphics3DGlide2x::RenderPolygonSinglePass (GrVertex * verts, int num, TextureHandler* text, 
                                                   TextureHandler* light, bool is_transparent)
{
  int i;

  if (text)
  {
    for (i=0;i<num;i++) 
    {
      verts[i].tmuvtx[0].sow *= text->width*verts[i].oow;
      verts[i].tmuvtx[0].tow *= text->height*verts[i].oow;
    }

    GlideLib_grTexSource (text->tmu->tmu_id, text->loadAddress,
                          GR_MIPMAPLEVELMASK_BOTH, &text->info);
  }

  if(light)
  {
    for(i=0;i<num;i++)
    {
      verts[i].tmuvtx[1].sow *= light->width*verts[i].oow; 
      verts[i].tmuvtx[1].tow *= light->height*verts[i].oow;
    }

    GlideLib_grTexSource (light->tmu->tmu_id, light->loadAddress,
                          GR_MIPMAPLEVELMASK_BOTH, &light->info);
  }
  
  if ( text || light )
  {
    GlideLib_grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE, 
                             GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
    GlideLib_grAlphaCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE, 
                             GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_CONSTANT, FXFALSE );

    if ( !light )
      GlideLib_grTexCombine (text->tmu->tmu_id,
                             GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
                             GR_COMBINE_FUNCTION_ZERO,  GR_COMBINE_FACTOR_NONE,
                             FXFALSE,FXFALSE);
    else if ( !text )
      GlideLib_grTexCombine (light->tmu->tmu_id == 1 ? 0 : 1 ,
                             GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE,
                             GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE,
                             FXFALSE,FXFALSE);
    else
      GlideLib_grTexCombine (text->tmu->tmu_id,
                             GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
                             GR_COMBINE_FUNCTION_ZERO,  GR_COMBINE_FACTOR_NONE,
                             FXFALSE,FXFALSE);
  }
  else
  {
    GlideLib_grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE, 
                             GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_CONSTANT, FXFALSE );
    GlideLib_grAlphaCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE, 
                             GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_CONSTANT, FXFALSE );
  }

  if (is_transparent)       
    GlideLib_grAlphaBlendFunction ( GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA, GR_BLEND_ONE, GR_BLEND_ZERO );
  else
    GlideLib_grAlphaBlendFunction ( GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ONE, GR_BLEND_ZERO );
 /*if(!haslight)
  {
    GlideLib_grTexCombine(0,
      GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_NONE,
      FXFALSE,FXFALSE);
  }
 */

  GlideLib_grDrawPlanarPolygonVertexList(num,verts);
/*
  GlideLib_grColorCombine( GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, 
                             GR_COMBINE_LOCAL_CONSTANT, GR_COMBINE_OTHER_NONE, FXFALSE );
  GlideLib_grConstantColorValue ( 0xffffffff );
  for ( i=0; i < num; i++ )
    grDrawLine( &verts[i], &verts[ (i+1)%num ] );
  */
}

void csGraphics3DGlide2x::RenderPolygonMultiPass (GrVertex* verts, int num, TextureHandler* text, 
						  TextureHandler* light, bool is_transparent)
{
  int i;


  if (text)
  {
    for (i=0;i<num;i++)
    {
      verts[i].tmuvtx[0].sow *= text->width*verts[i].oow;
      verts[i].tmuvtx[0].tow *= text->height*verts[i].oow;
    }
    GlideLib_grColorCombine ( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE, 
                              GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
    GlideLib_grAlphaCombine ( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE, 
                              GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_CONSTANT, FXFALSE );
    GlideLib_grTexClampMode (text->tmu->tmu_id, GR_TEXTURECLAMP_WRAP,GR_TEXTURECLAMP_WRAP);
    GlideLib_grTexSource (text->tmu->tmu_id, text->loadAddress,GR_MIPMAPLEVELMASK_BOTH, &text->info);
  }
  else
  {
    GlideLib_grColorCombine ( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE, 
                              GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_CONSTANT, FXFALSE );
    GlideLib_grAlphaCombine ( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE, 
                              GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_CONSTANT, FXFALSE );
  }

  if (is_transparent)
    GlideLib_grAlphaBlendFunction (GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA,
                                   GR_BLEND_ONE, GR_BLEND_ZERO);
  else
    GlideLib_grAlphaBlendFunction (GR_BLEND_ONE, GR_BLEND_ZERO,
                                   GR_BLEND_ONE, GR_BLEND_ZERO);
				   

  GlideLib_grDrawPlanarPolygonVertexList (num,verts);

  if (light)
  {
    for (i=0;i<num;i++)
    {
      verts[i].tmuvtx[0].sow= verts[i].tmuvtx[1].sow * light->width*verts[i].oow; 
      verts[i].tmuvtx[0].tow= verts[i].tmuvtx[1].tow * light->height*verts[i].oow; 
    }

    GlideLib_grColorCombine ( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE, 
                              GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
    GlideLib_grAlphaCombine ( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE, 
                              GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_CONSTANT, FXFALSE );

    GlideLib_grAlphaBlendFunction (GR_BLEND_ZERO, GR_BLEND_SRC_COLOR,
                                   GR_BLEND_ZERO, GR_BLEND_ZERO);
    GlideLib_grTexClampMode (light->tmu->tmu_id, GR_TEXTURECLAMP_CLAMP,GR_TEXTURECLAMP_CLAMP);
    GlideLib_grTexSource (light->tmu->tmu_id, light->loadAddress,
                          GR_MIPMAPLEVELMASK_BOTH, &light->info);

    GlideLib_grDrawPlanarPolygonVertexList (num,verts);
  }
  
}

void csGraphics3DGlide2x::SetupPolygon ( G3DPolygonDP& poly, float& J1, float& J2, float& J3, 
                                                             float& K1, float& K2, float& K3,
                                                             float& M,  float& N,  float& O  )
{
  // Get the plane normal of the polygon. Using this we can calculate
  // '1/z' at every screen space point.
  
  if (ABS (poly.normal.D ()) < 0.06)
  {
    J1= 0;
    J2= 0;
    J3= 0;
    K1= 0;
    K2= 0;
    K3= 0;
  }
  else
  {
    float inv_aspect = poly.inv_aspect;
    float P1, P2, P3, P4, Q1, Q2, Q3, Q4;
      

    P1 = poly.plane.m_cam2tex->m11;
    P2 = poly.plane.m_cam2tex->m12;
    P3 = poly.plane.m_cam2tex->m13;
    P4 = -(P1*poly.plane.v_cam2tex->x + P2*poly.plane.v_cam2tex->y + P3*poly.plane.v_cam2tex->z);

    Q1 = poly.plane.m_cam2tex->m21;
    Q2 = poly.plane.m_cam2tex->m22;
    Q3 = poly.plane.m_cam2tex->m23;
    Q4 = -(Q1*poly.plane.v_cam2tex->x + Q2*poly.plane.v_cam2tex->y + Q3*poly.plane.v_cam2tex->z);

    J1 = P1 * inv_aspect + P4*M;
    J2 = P2 * inv_aspect + P4*N;
    J3 = P3              + P4*O;

    K1 = Q1 * inv_aspect + Q4*M;
    K2 = Q2 * inv_aspect + Q4*N;
    K3 = Q3              + Q4*O;

  }  
  
}

void csGraphics3DGlide2x::DrawPolygon (G3DPolygonDP& poly)
{
  if (poly.num < 3 || poly.normal.D () == 0.0) return;
  iPolygonTexture* pTex;
  iLightMap* piLM = NULL;
  csGlideCacheData* tcache = NULL;
  csGlideCacheData* lcache = NULL;
  TextureHandler *thLm  = NULL; 
  TextureHandler *thTex = NULL;
  bool lm_exists=false;
  bool is_transparent = false;
  bool is_colorkeyed = false;
  // retrieve the texture.
  pTex = poly.poly_texture;
  // cache the tex if neccessary
  if ( pTex && ( m_renderstate.textured || m_renderstate.lighting ) )
    CacheTexture (pTex);
  // get texture and lightmap cache data if avail.
  if ( m_renderstate.textured )
  {
    // get the cache data
    if ( pTex )
      tcache = (csGlideCacheData *)poly.txt_handle->GetCacheData ();
    if ( !tcache || !tcache->texhnd.loaded )  
      return;
    else
      thTex = &tcache->texhnd;
  } 

  // retrieve the lightmap from the cache.
  piLM = ( m_renderstate.lighting && pTex ? pTex->GetLightMap () : NULL );
  if (piLM)
  {
    lcache = (csGlideCacheData *)piLM->GetCacheData ();
    if (lcache!=NULL && lcache->texhnd.loaded)
    {
      lm_exists = true;
      thLm = &lcache->texhnd;
    }
  }

  // set color and transparency
  UByte a, r, g, b;
  a=r=g=b=255;

  if ( poly.alpha )
  {
    is_transparent = true;
    a = 255 - (int)(( poly.alpha / 100.f ) * 255.f);
    GlideLib_grDepthMask( FXFALSE );
  }
 
  // if we draw a flat shaded polygon we gonna use the mean color
  if ( !m_renderstate.textured )
    poly.txt_handle->GetMeanColor ( r, g, b );

  GlideLib_grConstantColorValue ( (a << 24) | (r << 16) | (g << 8) | b );
  
  
  float J1, J2, J3, K1, K2, K3;
  float M, N, O;
  float x,y,ooz,z,u,v,lu,lv;
  int i;

  // set up the geometry.
#ifdef DO_HW_UVZ
  if ( !poly.uvz )
#endif
  {
    float inv_aspect = poly.inv_aspect;
    float inv_Dc = 1/poly.normal.D ();
  
    if (ABS (poly.normal.D ()) < 0.06)
    {
      M = N = 0;
      O = 1/poly.z_value;
    }
    else
    {
      M = -poly.normal.A ()*inv_Dc*inv_aspect;
      N = -poly.normal.B ()*inv_Dc*inv_aspect;
      O = -poly.normal.C ()*inv_Dc;
    }
    SetupPolygon ( poly, J1, J2, J3, K1, K2, K3, M, N, O );
  }
  
  if ( m_dpvertsize < poly.num )
  {
    m_dpverts=new GrVertex[poly.num];
    m_dpvertsize = poly.num;
    for( i=0; i < poly.num; i++ )
    {
      m_dpverts[i].r = 255;
      m_dpverts[i].g = 255;
      m_dpverts[i].b = 255;
    }
  }

#ifdef DO_HW_UVZ
  int j;
#endif

  for(i=0;i < poly.num;i++)
  {
#ifdef DO_HW_UVZ
    if ( poly.uvz )
    {
      m_dpverts[i].x = poly.vertices[i].sx + SNAP;
      m_dpverts[i].y = poly.vertices[i].sy + SNAP; 
      j = poly.mirror ? poly.num-i-1 : i;
      u = poly.uvz[ j ].x;
      v = poly.uvz[ j ].y;
      m_dpverts[i].z = z = poly.uvz[ j ].z;
      ooz = 1/z;
    }
    else
#endif
    {
      x = poly.vertices[i].sx;
      y = poly.vertices[i].sy;
      m_dpverts[i].x = x + SNAP;
      m_dpverts[i].y = y + SNAP; 
      x-=m_nHalfWidth;
      y-=m_nHalfHeight;
      ooz = (M*(x) + N*(y) + O);
      m_dpverts[i].z = z = 1/ooz;
      u = (J1 * (x) + J2 * (y) + J3)*z;
      v = (K1 * (x) + K2 * (y) + K3)*z;
    }
    m_dpverts[i].tmuvtx[0].sow= u; 
    m_dpverts[i].tmuvtx[0].tow= v; 
    m_dpverts[i].oow /*= verts[i].tmuvtx[0].oow = verts[i].tmuvtx[1].oow */= ooz;
  }
  if (lm_exists)
  {
    float fMinU, fMinV, fMaxU, fMaxV;
    pTex->GetTextureBox ( fMinU, fMinV, fMaxU, fMaxV );
      
    float lu_dif, lv_dif,lu_scale, lv_scale;
  
    // gsteenss: following few lines were added to scale the lightmaps 
    // correctly (?)  
    float scale_u,scale_v;
    int lmwidth = piLM->GetWidth ();
    int lmrealwidth = piLM->GetRealWidth ();
    int lmheight = piLM->GetHeight ();
    int lmrealheight = piLM->GetRealHeight ();
      
    scale_u = (float)(lmrealwidth-1) / (float)lmwidth;
    scale_v = (float)(lmrealheight-1) / (float)lmheight;
      
    lu_dif = fMinU;
    lv_dif = fMinV;
    lu_scale = scale_u/(fMaxU - fMinU);
    lv_scale = scale_v/(fMaxV - fMinV);

    for (i=0;i<poly.num;i++)
    {
      lu = (m_dpverts[i].tmuvtx[0].sow- lu_dif) * lu_scale;
      lv = (m_dpverts[i].tmuvtx[0].tow- lv_dif) * lv_scale;
      m_dpverts[i].tmuvtx[1].sow= lu; 
      m_dpverts[i].tmuvtx[1].tow= lv; 
    }
  }

  if (is_colorkeyed)
    GlideLib_grChromakeyMode (GR_CHROMAKEY_ENABLE);
  
  if (poly_fog != poly.use_fog)
  {
    poly_fog = poly.use_fog;
    if ( poly_fog )
    {
      GlideLib_grFogMode ( GR_FOG_WITH_TABLE );
      GlideLib_grFogTable ( fogtable );
      GlideLib_grFogColorValue ( 0xFFC0C0C0 );
    }
    else
      GlideLib_grFogMode ( GR_FOG_DISABLE );
  }

  RenderPolygon (m_dpverts,poly.num,thTex,thLm,is_transparent);

  if (is_colorkeyed)
    GlideLib_grChromakeyMode (GR_CHROMAKEY_DISABLE);
  
  if ( poly.alpha )
    GlideLib_grDepthMask( FXTRUE );

}

void csGraphics3DGlide2x::StartPolygonFX (iTextureHandle *handle,  UInt mode)
{
  if ( m_renderstate.textured && handle )
  {
    csTextureMMGlide* txt_mm = (csTextureMMGlide*)handle->GetPrivateObject ();
    csGlideCacheData* tcache;
    m_pTextureCache->Add (handle, false);
    tcache = (csGlideCacheData *)txt_mm->GetCacheData ();

    m_thTex = ( tcache ? &tcache->texhnd : NULL );
    if ( m_thTex )
    {
      GlideLib_grTexSource (m_thTex->tmu->tmu_id, m_thTex->loadAddress, 
                            GR_MIPMAPLEVELMASK_BOTH, &m_thTex->info);

      if(!m_iMultiPass)
      {
        GlideLib_grTexCombine (m_TMUs[0].tmu_id,
                               GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
                               GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_NONE,
                               FXFALSE,FXFALSE);
      }
      GlideLib_grAlphaCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE, 
                               GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_CONSTANT, FXFALSE );
    }
  }
  if ( !m_verts ) {
    m_verts = new GrVertex[ GLIDE_FX_VERTSIZE ];
    m_vertsize = GLIDE_FX_VERTSIZE;
  }
  
  m_dpfx.mixmode = mode;
  m_dpfx.gouraud = m_renderstate.gouraud && ((mode & CS_FX_GOURAUD) != 0);

  if ( m_dpfx.gouraud )
  {
    if ( !m_thTex )
    GlideLib_grColorCombine( GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, 
                             GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_NONE, FXFALSE );
    else
    GlideLib_grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL, 
                             GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
  }
  else if ( m_thTex )
    GlideLib_grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE, 
                             GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_TEXTURE, FXFALSE );

  switch (mode & CS_FX_MASK_MIXMODE)
  {
  case CS_FX_MULTIPLY:
    // Color = SRC * DEST + 0 * SRC = SRC * DEST
    m_dpfx.alpha = 255;
    GlideLib_grAlphaBlendFunction ( GR_BLEND_DST_COLOR, GR_BLEND_ZERO, GR_BLEND_ONE, GR_BLEND_ZERO );
    break;
  case CS_FX_MULTIPLY2:
    // Color = SRC * DEST + DEST * SRC = 2 * SRC * DEST
    m_dpfx.alpha = 255;
    GlideLib_grAlphaBlendFunction ( GR_BLEND_DST_COLOR, GR_BLEND_SRC_COLOR, GR_BLEND_ONE, GR_BLEND_ZERO );
    break;
  case CS_FX_ADD:
    // Color = 1 * DEST + 1 * SRC = SRC + DEST
    m_dpfx.alpha = 255;
    GlideLib_grAlphaBlendFunction ( GR_BLEND_ONE, GR_BLEND_ONE, GR_BLEND_ONE, GR_BLEND_ZERO );
    break;
  case CS_FX_ALPHA:
    // Color = (1-Alpha) * DEST + Alpha * SRC 
    m_dpfx.alpha   = 255 - (mode & CS_FX_MASK_ALPHA);
    GlideLib_grAlphaBlendFunction ( GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA, 
                                    GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA );
    GlideLib_grDepthMask( FXFALSE );
    break;
  case CS_FX_TRANSPARENT:
    // Color = 1 * DEST + 0 * SRC = DEST
    m_dpfx.alpha = 255;
    GlideLib_grAlphaBlendFunction ( GR_BLEND_ZERO, GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ONE );
    break;
  case CS_FX_COPY:
  default:
    // Color = 0 * DEST + 1 * SRC = SRC
    m_dpfx.alpha = 255;
    GlideLib_grAlphaBlendFunction ( GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ONE, GR_BLEND_ZERO );
    break;
  }
  
  UByte r, g, b;
  r=g=b=255;

  if ( mode & CS_FX_KEYCOLOR )
  {
    GlideLib_grChromakeyMode (GR_CHROMAKEY_ENABLE);
    handle->GetTransparent ( r, g, b );
    GlideLib_grChromakeyValue (0xff << 24 | r << 16 | g << 8 | b );
  }
  
  // if we draw a flat shaded polygon we gonna use the mean color
  if ( !m_renderstate.textured && handle)
  {
    handle->GetMeanColor ( r, g, b );
    GlideLib_grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE, 
                             GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_CONSTANT, FXFALSE );
    GlideLib_grAlphaCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE, 
                             GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_CONSTANT, FXFALSE );
    GlideLib_grAlphaBlendFunction ( GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ONE, GR_BLEND_ZERO );
  }
  
  GlideLib_grConstantColorValue ( (m_dpfx.alpha << 24) | (r << 16) | (g << 8) | b );
}

void csGraphics3DGlide2x::FinishPolygonFX ()
{
  if (!m_iMultiPass)
  {
    GlideLib_grTexCombine (m_TMUs[0].tmu_id,
                           GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
                           GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_NONE,
                           FXFALSE,FXFALSE);
  }
  m_thTex = NULL;
  
  if (m_dpfx.mixmode & CS_FX_MASK_MIXMODE)
    GlideLib_grDepthMask( FXTRUE );
    
  if (m_dpfx.mixmode & CS_FX_KEYCOLOR )
    GlideLib_grChromakeyMode (GR_CHROMAKEY_DISABLE);
}

void csGraphics3DGlide2x::DrawPolygonFX (G3DPolygonDPFX& poly)
{
  if (poly.num >= 3 && (m_thTex || !m_renderstate.textured))
  {
    if ( poly.num  > m_vertsize )
    {
      if ( m_verts ) delete m_verts;
      CHK (m_verts = new GrVertex[poly.num]);
      m_vertsize = poly.num;
    }

    for (int i=0; i<poly.num; i++)
    {
      m_verts[i].x = poly.vertices[i].sx + SNAP;
      m_verts[i].y = poly.vertices[i].sy + SNAP;
      if (m_dpfx.gouraud)
      {
        m_verts[i].r = poly.vertices[i].r*255;
        m_verts[i].g = poly.vertices[i].g*255;
        m_verts[i].b = poly.vertices[i].b*255;
      }
      else
      {
        m_verts[i].r = 255;
        m_verts[i].g = 255;
        m_verts[i].b = 255;
      }
      m_verts[i].oow = 1./poly.vertices[i].z;
      if ( m_renderstate.textured ){
        m_verts[i].tmuvtx[1].sow = m_verts[i].tmuvtx[0].sow = poly.vertices[i].u*m_thTex->width*m_verts[i].oow;
        m_verts[i].tmuvtx[1].tow = m_verts[i].tmuvtx[0].tow = poly.vertices[i].v*m_thTex->height*m_verts[i].oow;
      }
      m_verts[i].a = poly.vertices[i].z;
    }
    
    if (poly.use_fog)
    {
      GlideLib_grFogMode ( GR_FOG_WITH_ITERATED_ALPHA );
      GlideLib_grFogColorValue ( 0 );
    }
    
    GlideLib_grDrawPlanarPolygonVertexList (poly.num, m_verts);

    if(poly.use_fog)
      GlideLib_grFogMode ( GR_FOG_DISABLE );
  }
}

void csGraphics3DGlide2x::CacheTexture (iPolygonTexture *texture)
{
  iTextureHandle* txt_handle = texture->GetTextureHandle ();
  m_pTextureCache->Add (txt_handle, false);
  m_pLightmapCache->Add (texture);
}

void csGraphics3DGlide2x::DumpCache (void)
{
  m_pTextureCache->Dump ();
  m_pLightmapCache->Dump ();
}

void csGraphics3DGlide2x::ClearCache (void)
{
  if (m_pTextureCache) m_pTextureCache->Clear ();
  if (m_pLightmapCache) m_pLightmapCache->Clear ();
}

void csGraphics3DGlide2x::DrawLine (const csVector3& v1, const csVector3& v2, float fov, int color)
{
  if (v1.z < SMALL_Z && v2.z < SMALL_Z) return ;

  float x1 = v1.x, y1 = v1.y, z1 = v1.z;
  float x2 = v2.x, y2 = v2.y, z2 = v2.z;

  if (z1 < SMALL_Z)
  {
    float t = ((float)SMALL_Z-z1) / (z2-z1);
    x1 = t*(x2-x1)+x1;
    y1 = t*(y2-y1)+y1;
    z1 = (float)SMALL_Z;
  }
  else if (z2 < SMALL_Z)
  {
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
}

bool csGraphics3DGlide2x::SetRenderState (G3D_RENDERSTATEOPTION option, long value)
{
  switch (option)
  {
    case G3DRENDERSTATE_ZBUFFERMODE:
      m_ZBufMode = value;

      if (value & CS_ZBUF_TEST)
        GlideLib_grDepthMask (FXFALSE);
      else
        GlideLib_grDepthMask (FXTRUE);    
      if (value & CS_ZBUF_FILL)      // write-only
        GlideLib_grDepthBufferFunction (GR_CMP_ALWAYS);
      else 
        GlideLib_grDepthBufferFunction (GR_CMP_LEQUAL);
      break;
      
    case G3DRENDERSTATE_DITHERENABLE:
/*
    if(value)
      m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE);
    else
      m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE);
*/
    break;
  case G3DRENDERSTATE_BILINEARMAPPINGENABLE:
    m_renderstate.bilinearmap = value;
    break;
  case G3DRENDERSTATE_TRILINEARMAPPINGENABLE:
    m_renderstate.trilinearmap = value;
    break;
  case G3DRENDERSTATE_TRANSPARENCYENABLE:
    m_renderstate.alphablend = value;
    break;
  case G3DRENDERSTATE_MIPMAPENABLE:
    m_renderstate.mipmap = value;
    break;
  case G3DRENDERSTATE_TEXTUREMAPPINGENABLE:
    m_renderstate.textured = value;
    break;
  case G3DRENDERSTATE_LIGHTINGENABLE :
    m_renderstate.lighting = value;
    break;
  case G3DRENDERSTATE_INTERLACINGENABLE :
  case G3DRENDERSTATE_MMXENABLE :
    break;
  case G3DRENDERSTATE_GOURAUDENABLE:
    m_renderstate.gouraud = value;
    break;
  default:
    return false;
  }

  return true;
}

long csGraphics3DGlide2x::GetRenderState (G3D_RENDERSTATEOPTION op)
{
  switch(op)
  {
    case G3DRENDERSTATE_ZBUFFERMODE:
      return m_ZBufMode;
    case G3DRENDERSTATE_DITHERENABLE:
      return false;
    case G3DRENDERSTATE_BILINEARMAPPINGENABLE:
      return m_renderstate.bilinearmap;
    case G3DRENDERSTATE_TRILINEARMAPPINGENABLE:
      return m_renderstate.trilinearmap;
    case G3DRENDERSTATE_TRANSPARENCYENABLE:
      return m_renderstate.alphablend;
    case G3DRENDERSTATE_MIPMAPENABLE:
      return m_renderstate.mipmap;
    case G3DRENDERSTATE_TEXTUREMAPPINGENABLE:
      return m_renderstate.textured;
    case G3DRENDERSTATE_GOURAUDENABLE:
      return m_renderstate.gouraud;
    default:
      return 0;
  }
}

void csGraphics3DGlide2x::OpenFogObject (CS_ID /*id*/, csFog* /*fog*/)
{
}

void csGraphics3DGlide2x::DrawFogPolygon (CS_ID /*id*/, G3DPolygonDFP& /*poly*/, 
  int /*fogtype*/)
{
}

void csGraphics3DGlide2x::CloseFogObject (CS_ID /*id*/)
{
}


iHalo *csGraphics3DGlide2x::CreateHalo (float r, float g, float b, unsigned char *alpha, int width, int height)
{
  csGlideAlphaMap *am = new csGlideAlphaMap( alpha, width, height );
  csGlideHalo *halo = new csGlideHalo ( r, g, b, width, height, this, am );
  return halo;
}

float csGraphics3DGlide2x::GetZBuffValue ( int x, int y )
{
  float z = m_piGlide2D->GetZBuffValue (x, y);
//  printf("%g\n", z);
  return z;
}

void csGraphics3DGlide2x::DrawPixmap ( iTextureHandle *hTex,
  int sx, int sy, int sw, int sh, int tx, int ty, int tw, int th)
{
  // Now that it's possible to implement DrawPixmap properly I would
  // highly recommend doing it using polygon drawing call. This will use
  // hardware acceleration. For now the old renderer is commented
  // out since there is no more GetMipmapData() method in iTextureHandle.
  // In any case it's easy to reanimate the old routine (you just have to
  // properly retrieve the bitmap data and dimensions) but I would recommend
  // re-implementing it using hardware acceleration. -- A.Z.

  /// Clipping code - sprites should be clipped against G2D's clipping rectangle


  /// Retrieve clipping rectangle
  static G3DPolygonDPFX spr2d;
  int ClipX1, ClipY1, ClipX2, ClipY2;
  int h = GetHeight()-1;
  m_piG2D->GetClipRect (ClipX1, ClipY1, ClipX2, ClipY2);

  // Texture coordinates (floats)
  float _tx = tx, _ty = ty, _tw = tw, _th = th;

  // Clipping
  if ((sx >= ClipX2) || (sy >= ClipY2) ||
      (sx + sw <= ClipX1) || (sy + sh <= ClipY1))
    return;                             // Sprite is totally invisible
  if (sx < ClipX1)                      // Left margin crossed?
  {
    int nw = sw - (ClipX1 - sx);        // New width
    _tx += (ClipX1 - sx) * _tw / sw;    // Adjust X coord on texture
    _tw = (_tw * nw) / sw;              // Adjust width on texture
    sw = nw; sx = ClipX1;
  } /* endif */
  if (sx + sw > ClipX2)                 // Right margin crossed?
  {
    int nw = ClipX2 - sx;               // New width
    _tw = (_tw * nw) / sw;              // Adjust width on texture
    sw = nw;
  } /* endif */
  if (sy < ClipY1)                      // Top margin crossed?
  {
    int nh = sh - (ClipY1 - sy);        // New height
    _ty += (ClipY1 - sy) * _th / sh;    // Adjust Y coord on texture
    _th = (_th * nh) / sh;              // Adjust height on texture
    sh = nh; sy = ClipY1;
  } /* endif */
  if (sy + sh > ClipY2)                 // Bottom margin crossed?
  {
    int nh = ClipY2 - sy;               // New height
    _th = (_th * nh) / sh;              // Adjust height on texture
    sh = nh;
  } /* endif */

  // now use _tx,_ty,_tw and _th instead of tx, ty, tw and th
  // since they can be fractional values
  
  int bw, bh;
  hTex->GetMipMapDimensions ( 0, bw, bh );
  spr2d.num = 4;
  spr2d.vertices[0].sx = sx;
  spr2d.vertices[0].sy = h-sy;
  spr2d.vertices[1].sx = sx + sw;
  spr2d.vertices[1].sy = h-sy;
  spr2d.vertices[2].sx = sx + sw;
  spr2d.vertices[2].sy = h-sy-sh;
  spr2d.vertices[3].sx = sx;
  spr2d.vertices[3].sy = h-sy-sh;
  
  float ntx1, ntx2, nty1, nty2;
  ntx1 = _tx / bw;
  ntx2 = (_tx + _tw) / bw;
  nty1 = _ty / bh;
  nty2 = (_ty + _th) / bh;
  
  spr2d.vertices[0].u = ntx1;
  spr2d.vertices[0].v = nty1;
  spr2d.vertices[1].u = ntx2;
  spr2d.vertices[1].v = nty1;
  spr2d.vertices[2].u = ntx2;
  spr2d.vertices[2].v = nty2;
  spr2d.vertices[3].u = ntx1;
  spr2d.vertices[3].v = nty2;

  spr2d.vertices[0].z = 1;
  spr2d.vertices[1].z = 1;
  spr2d.vertices[2].z = 1;
  spr2d.vertices[3].z = 1;
  
  spr2d.txt_handle = hTex;
  StartPolygonFX ( hTex, CS_FX_COPY | ( hTex->GetTransparent() ? CS_FX_KEYCOLOR : 0 ) );
  DrawPolygonFX ( spr2d );
  FinishPolygonFX ();
}
