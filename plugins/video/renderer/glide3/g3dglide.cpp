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

// G3D_GLIDE.CPP
// csGraphics3DGlide3x implementation file
// Written by Nathaniel

#if defined(OS_WIN32)
#include <windows.h>
#endif
#include <stdlib.h>
#include <glide.h>

#include "sysdef.h"
#include "cs3d/software/scan.h"
#include "StdAssrt.h"
#include "ilghtmap.h"
#include "isystem.h"
#include "ipolygon.h"
#include "icamera.h"
#include "itexture.h"
#include "IGraph3d.h"
#include "csutil/inifile.h"

//#include "render/glide3/driver2d/g2d.h"	@@@
#include "cs3d/glide3/g3dglide.h"

csIniFile *configglide3;

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
			
			MessageBox (NULL, szMsg, "Fatal Error in Glide3xRender.dll", MB_OK);
			delete szMsg;

			exit(1);
		}
	}

	MessageBox(NULL, str, "Fatal Error in Glide3xRender.dll", MB_OK);
#else
  fprintf(stderr, "FATAL ERROR: %s", str);
#endif

  exit(1);
}

//
// Interface table definition
//

BEGIN_INTERFACE_TABLE(csGraphics3DGlide3x)
    IMPLEMENTS_INTERFACE( IGraphics3D )
END_INTERFACE_TABLE()

IMPLEMENT_UNKNOWN_NODELETE(csGraphics3DGlide3x)

/**
* This Function gets information from the Current Board selected.
* Those informations are written to the Initialization Log
* It also use those informations to initialize the rendering process
* This initialisation includes:
*   - Standard init of the board
*   - Specific init for rendering pass (One or two based on the number of TMUs)
**/
void csGraphics3DGlide3x::InitializeBoard()
{
  FxI32 nTmu, nFb, nRevTmu, nRevFb, nMemTmu, nMemFb;
  const char *szHardware;

	grGet(GR_NUM_TMU, sizeof(FxI32), &nTmu);
	grGet(GR_NUM_FB, sizeof(FxI32), &nFb);
	grGet(GR_MEMORY_TMU, sizeof(FxI32), &nMemTmu);
	grGet(GR_MEMORY_FB, sizeof(FxI32), &nMemFb);
	grGet(GR_REVISION_TMU, sizeof(FxI32), &nRevTmu);
	grGet(GR_REVISION_FB, sizeof(FxI32), &nRevFb);
  szHardware = grGetString(GR_HARDWARE);
  
  SysPrintf (MSG_INITIALIZATION, "Board is a %s.\n", szHardware);
  SysPrintf (MSG_INITIALIZATION, "  SLI %sdetected.\n", (nFb>1)?"":"no ");
  SysPrintf (MSG_INITIALIZATION, "  Pixelfx Revision %d.\n", nRevFb);
  SysPrintf (MSG_INITIALIZATION, "  Pixelfx Memory : %d Mo.\n", nMemFb);
  SysPrintf (MSG_INITIALIZATION, "  Number of Texelfx Chips %d.\n", nTmu);
  SysPrintf (MSG_INITIALIZATION, "  Texelfx has revision %d and %d Mo of RAM.\n", nRevTmu, nMemTmu);
  if(nTmu==1)
  {
		m_iMultiPass=true;
    iTMUTexture=1;
    iTMULightMap=0;
  }
  else
  {
		m_iMultiPass=false;
    iTMULightMap=1;
    iTMUTexture=1;
  }

	if(configglide3->GetYesNo("Glide3x","FORCEMULTIPASS",FALSE)&&(m_iMultiPass==false))
	{
		SysPrintf (MSG_INITIALIZATION, "Forced MultiPass Rendering.\n");
		m_iMultiPass=true;
	}
	if(m_iMultiPass)
	{
		SysPrintf (MSG_INITIALIZATION, "Will use MultiPass Rendering.\n");
	}
  else
	{
		SysPrintf (MSG_INITIALIZATION, "Will use SinglePass Rendering.\n");
		SysPrintf (MSG_INITIALIZATION, "Affected %d TMU for Texture and %d for LightMap.\n",iTMUTexture,iTMULightMap);
	}
	m_TMUs = new TMUInfo[2];
	if(m_iMultiPass)
	{
		m_TMUs[0].tmu_id=m_TMUs[1].tmu_id=0;
		m_TMUs[0].minAddress = grTexMinAddress(0);
		m_TMUs[1].maxAddress = grTexMaxAddress(0);
		m_TMUs[1].minAddress = m_TMUs[0].maxAddress = (m_TMUs[1].maxAddress -  m_TMUs[0].minAddress)/2;
		m_TMUs[1].minAddress = (m_TMUs[1].minAddress+7)&(~7);
	} else {
		m_TMUs[0].tmu_id=0;
		m_TMUs[0].minAddress = grTexMinAddress(0);
		m_TMUs[0].maxAddress = grTexMaxAddress(0);
		m_TMUs[1].tmu_id=1;
		m_TMUs[1].minAddress = grTexMinAddress(1);
		m_TMUs[1].maxAddress = grTexMaxAddress(1);
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
**/
int csGraphics3DGlide3x::SelectBoard()
{
	int i;
	board=0;
/*
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
					{	// Equivalent => Most RAM
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
					{	// Equivalent => Most RAM
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
					{	// Equivalent => Most RAM
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
					{	// Equivalent => Most RAM
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
*/
	return board;
}

csGraphics3DGlide3x::csGraphics3DGlide3x(ISystem* piSystem) :
    m_pTextureCache(NULL),
    m_pLightmapCache(NULL),
    m_piSystem(piSystem)
{
  HRESULT hRes;
  IGraphics2DFactory* piFactory = NULL;
  
  configglide3 = new csIniFile("Glide3x.cfg");

  m_piSystem->AddRef();
  ASSERT( m_piSystem );

  SysPrintf (MSG_INITIALIZATION, "\nGlideRender Glide3x selected\n");

  piFactory = new csGraphics2DGlide3xFactory();
  hRes = piFactory->CreateInstance( IID_IGraphics2D, m_piSystem, (void**)&m_piG2D );
  if (FAILED(hRes))
  {
      SysPrintf(MSG_FATAL_ERROR, "Error! Couldn't create 2D graphics driver instance.");
      exit(0);
  }

  FINAL_RELEASE( piFactory );

  m_bVRetrace = configglide3->GetYesNo("Glide3x","VRETRACE",FALSE);

  FxI32 grncard=0;

  grGet(GR_NUM_BOARDS, sizeof(FxI32), &grncard);
	if(grncard==0) 
		sys_fatalerror("csGraphics3DGlide3x::Open : No 3dfx chip found");

  grGlideInit();

  SelectBoard();

  const char *szString;
	szString = grGetString(GR_VERSION);
	SysPrintf (MSG_INITIALIZATION, "Glide %s detected.\n",szString);
  SysPrintf (MSG_INITIALIZATION, "Board %d selected.\n",board);
	
	grSstSelect(board);

  InitializeBoard();

  use16BitTexture=configglide3->GetYesNo("Glide3x","USE_16BIT_TEXTURE",false);

  CHK (m_pTextureCache = new GlideTextureCache(&m_TMUs[0], (use16BitTexture)?16:8, new FixedTextureMemoryManager(m_TMUs[0].memory_size)));
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

  /*
  rstate_dither = false;
  rstate_specular = false;
  rstate_bilinearmap = false;
  rstate_trilinearmap = false;
  rstate_gouraud = false;
  rstate_flat = false;
  rstate_alphablend = false;
  rstate_mipmap = false;
  rstate_edges = false;
*/
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
  {1024,768,GR_RESOLUTION_1024x768},
	{1280,1024,GR_RESOLUTION_1280x1024},
	{1600,1200,GR_RESOLUTION_1600x1200},
	{400,300,GR_RESOLUTION_400x300},
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

STDMETHODIMP csGraphics3DGlide3x::Open(char* Title)
{
	FxU32 hwnd=NULL;
	GrScreenResolution_t iRes;

  IGlide3xGraphicsInfo* pSysGInfo = NULL;
  IGraphicsInfo*      pGraphicsInfo = NULL;

  HRESULT hRes = m_piG2D->QueryInterface(IID_IGlide3xGraphicsInfo, (void**)&pSysGInfo);
  if (FAILED(hRes))
    goto OnError;
  
  m_piG2D->QueryInterface(IID_IGraphicsInfo, (void**)&pGraphicsInfo);
  if (FAILED(hRes))
    goto OnError;

  // Open the 2D driver.
  hRes = m_piG2D->Open(Title);
  if ( FAILED(hRes) )
    goto OnError;
	
#if defined(OS_WIN32)
	//pSysGInfo->GethWnd((void**)hwnd);
#endif

  pGraphicsInfo->GetWidth(m_nWidth);
  m_nHalfWidth = m_nWidth/2;
  
  pGraphicsInfo->GetHeight(m_nHeight);
  m_nHalfHeight = m_nHeight/2;
  
  if(/*configglide3->GetYesNo("VideoDriver","FULL_SCREEN",TRUE)*/true)
	{
		iRes=::getResolutionIndex(m_nWidth, m_nHeight);
		if(iRes==-1)
			sys_fatalerror("csGraphics3DGlide3x::Open() Invalid Resolution !");
	}
  else
  {
		iRes=0;
	}
	if(!(grcontext=grSstWinOpen(hwnd,
		StatGlideRes[iRes].res,GR_REFRESH_60Hz, // We should find a way to allow to change the refresh rate
		GR_COLORFORMAT_ARGB,GR_ORIGIN_LOWER_LEFT,2,1)))
    {
   		sys_fatalerror("csGraphics3DGlide3x::Open() : Could not open Window !");
    }
  
  grGet(GR_WDEPTH_MIN_MAX,2,(FxI32*)&wLimits[0]); // Get w-buffer limits
	grRenderBuffer(GR_BUFFER_BACKBUFFER);	// RENDER IN BACKBUFFER
	grColorMask(FXTRUE,FXFALSE);			// DISABLE ALPHA BUFFER
	grDepthMask(FXTRUE);					// ENABLE ZBUFFER
	grDepthBufferMode(GR_DEPTHBUFFER_WBUFFER); // ENABLE WBUFFER
	grDepthBufferFunction(GR_CMP_LEQUAL);		// WBUFFER FUNCTION
	
	grBufferClear(0,0,wLimits[1]); // CLEAR BACKBUFFER
	grBufferSwap(0);							// PUT BACKBUFFER TO FRONT
	grCullMode(GR_CULL_DISABLE);				// CULL POSITIVE 
	grChromakeyValue(0x0000);
	
	if(m_iMultiPass)
	{ // This card has only one TMU, enable Multipass rendering	  
	  grTexCombine(m_TMUs[0].tmu_id, GR_COMBINE_FUNCTION_LOCAL,
	  GR_COMBINE_FACTOR_NONE,GR_COMBINE_FUNCTION_ZERO,
	  GR_COMBINE_FACTOR_NONE,FXFALSE,FXFALSE);	// TMU INIT
		grTexClampMode(m_TMUs[0].tmu_id,
			GR_TEXTURECLAMP_WRAP,GR_TEXTURECLAMP_WRAP);	 // TEXTURE WRAP
		grTexFilterMode(m_TMUs[0].tmu_id,
			GR_TEXTUREFILTER_BILINEAR,GR_TEXTUREFILTER_BILINEAR);	 // TEXTURE WRAP
		//	GlideLib_grTexLodBiasValue(m_TMUs[0].tmu_id,7.75);		// LOD BIAS
		grTexMipMapMode(m_TMUs[0].tmu_id,GR_MIPMAP_NEAREST,FXFALSE);	// MIPMAP Mode
		//GlideLib_grTexMipMapMode(m_TMUs[0].tmu_id,GR_MIPMAP_DISABLE,FXFALSE);
		RenderPolygon=RenderPolygonMultiPass;
	}
	else
	{ // This card has several TMUs, enable Singlepass rendering
		grTexCombine(m_TMUs[0].tmu_id, GR_COMBINE_FUNCTION_SCALE_OTHER,
			GR_COMBINE_FACTOR_LOCAL,GR_COMBINE_FUNCTION_ZERO,
			GR_COMBINE_FACTOR_NONE,FXFALSE,FXFALSE);	// TMU INIT (SCALED BY TMU1)
		grTexClampMode(m_TMUs[0].tmu_id,
			GR_TEXTURECLAMP_WRAP,GR_TEXTURECLAMP_WRAP);	 // TEXTURE WRAP
		grTexFilterMode(m_TMUs[0].tmu_id,
			GR_TEXTUREFILTER_BILINEAR,GR_TEXTUREFILTER_BILINEAR);	 // TEXTURE WRAP
		//		GlideLib_grTexLodBiasValue(m_TMUs[0].tmu_id,7.75);		// LOD BIAS		
		grTexMipMapMode(m_TMUs[0].tmu_id,GR_MIPMAP_NEAREST,FXFALSE); // MIPMAP Mode
		
		grTexCombine(m_TMUs[1].tmu_id, GR_COMBINE_FUNCTION_LOCAL,
			GR_COMBINE_FACTOR_NONE,GR_COMBINE_FUNCTION_ZERO,
			GR_COMBINE_FACTOR_NONE,FXFALSE,FXFALSE);	// TMU INIT
		grTexClampMode(m_TMUs[1].tmu_id,
			GR_TEXTURECLAMP_CLAMP,GR_TEXTURECLAMP_CLAMP);	 // TEXTURE WRAP
		grTexFilterMode(m_TMUs[1].tmu_id,
			GR_TEXTUREFILTER_BILINEAR,GR_TEXTUREFILTER_BILINEAR);	 // TEXTURE WRAP
		grTexMipMapMode(m_TMUs[1].tmu_id,GR_MIPMAP_NEAREST,FXFALSE); // MIPMAP Mode
//		grHints(0,GR_STWHINT_ST_DIFF_TMU1);
		RenderPolygon=RenderPolygonSinglePass;
	}
	grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
		GR_COMBINE_FACTOR_NONE,
		GR_COMBINE_LOCAL_CONSTANT, 
		GR_COMBINE_OTHER_NONE,FXFALSE);
	grAlphaBlendFunction(GR_BLEND_SRC_ALPHA,
		GR_BLEND_ONE_MINUS_SRC_ALPHA,GR_BLEND_ONE,GR_BLEND_ZERO);
	grColorCombine(GR_COMBINE_FUNCTION_BLEND,			// COLOR COMBINE
		GR_COMBINE_FACTOR_LOCAL,GR_COMBINE_LOCAL_ITERATED,
		GR_COMBINE_OTHER_TEXTURE,FXFALSE);

  m_nDrawMode = 0;

  // set vertex setup
  grVertexLayout(GR_PARAM_XY,  0, GR_PARAM_ENABLE);
  grVertexLayout(GR_PARAM_Q,   2 << 2, GR_PARAM_ENABLE);
  grVertexLayout(GR_PARAM_RGB, 4 << 2, GR_PARAM_ENABLE);
  grVertexLayout(GR_PARAM_A,   7 << 2, GR_PARAM_ENABLE);
  grVertexLayout(GR_PARAM_ST0, 8 << 2, GR_PARAM_ENABLE);
  grVertexLayout(GR_PARAM_ST1, 11 << 2, GR_PARAM_ENABLE);
  grVertexLayout(GR_PARAM_ST2, 14 << 2, GR_PARAM_ENABLE);

  return S_OK;

OnError:
  
  if (FAILED(hRes))
    FINAL_RELEASE(m_piG2D);
  
  FINAL_RELEASE(pGraphicsInfo);
  FINAL_RELEASE(pSysGInfo);
  
  return hRes;  
}

STDMETHODIMP csGraphics3DGlide3x::Close()
{
  ClearCache();

  if(grcontext)
    grSstWinClose(grcontext);
  return S_OK;
}

STDMETHODIMP csGraphics3DGlide3x::GetColormapFormat( G3D_COLORMAPFORMAT& g3dFormat ) 
{
  if (use16BitTexture)
    g3dFormat = G3DCOLORFORMAT_PRIVATE;
  else
    g3dFormat = G3DCOLORFORMAT_GLOBAL;
  return S_OK;
}

STDMETHODIMP csGraphics3DGlide3x::SetDimensions (int width, int height)
{
  m_nWidth = width;
  m_nHeight = height;
  m_nHalfWidth = width/2;
  m_nHalfHeight = height/2;
  grClipWindow(0, 0, width, height);

  return S_OK;
}

csGraphics3DGlide3x::~csGraphics3DGlide3x()
{
  grGlideShutdown();

  FINAL_RELEASE( m_pCamera );
  FINAL_RELEASE( m_piG2D );
  FINAL_RELEASE( m_piSystem );
}

STDMETHODIMP csGraphics3DGlide3x::BeginDraw (int DrawFlags)
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
      grBufferClear(0,0,wLimits[1]);

			/*DWORD clear_flag = 0;
						      
			if (DrawFlags & CSDRAW_CLEARZBUFFER)
			clear_flag |= D3DCLEAR_ZBUFFER;
			
			  if (DrawFlags & CSDRAW_CLEARSCREEN)
			  clear_flag |= D3DCLEAR_TARGET;
			  
				if (clear_flag)
					// To be optimized by drawing 2 triangles!	
      		{
			static	GrVertex * verts = NULL;
			if(verts==NULL)
			{
			verts=new GrVertex[4];
			verts[0].x=verts[1].x=0;
			verts[0].y=verts[2].y=0;
			verts[1].y=verts[3].y=FRAME_HEIGHT-1;
			verts[2].x=verts[3].x=FRAME_WIDTH-1;
			verts[0].oow=verts[1].oow=verts[2].oow=1.0;
			verts[0].r=verts[0].g=verts[0].b=
			verts[1].r=verts[1].g=verts[1].b=
			verts[2].r=verts[2].g=verts[2].b=
			verts[3].r=verts[3].g=verts[3].b=0;
			}
			GlideLib_grDepthBufferFunction(GR_CMP_ALWAYS);		// WBUFFER FUNCTION
			GlideLib_grDrawTriangle(&verts[0],&verts[1],&verts[2]);
			GlideLib_grDrawTriangle(&verts[3],&verts[1],&verts[2]);
			GlideLib_grDepthBufferFunction(GR_CMP_LESS);		// WBUFFER FUNCTION
			
		} */
		} /* endif */
	} /* endif */
	m_nDrawMode = DrawFlags;
	return S_OK;
}

/// End the frame
STDMETHODIMP csGraphics3DGlide3x::FinishDraw ()
{
  if (m_nDrawMode & CSDRAW_2DGRAPHICS)
		m_piG2D->FinishDraw ();

  m_nDrawMode = 0;
  return S_OK;
}

/// do the page swap.
STDMETHODIMP csGraphics3DGlide3x::Print(csRect* rect)
{
  if(m_bVRetrace)
    grBufferSwap(1);
  else
    grBufferSwap(0);
  return S_OK;
}

/// Set the mode for the Z buffer (functionality also exists in SetRenderState).
STDMETHODIMP csGraphics3DGlide3x::SetZBufMode (ZBufMode mode)
{
  return E_NOTIMPL;  	
}

#define SNAP (( float ) ( 3L << 18 ))

void csGraphics3DGlide3x::RenderPolygonSinglePass (MyGrVertex * verts, int num, bool haslight,TextureHandler*text,TextureHandler*light)
{
	int i;
	for(i=0;i<num;i++) 
	{
		verts[i].tmuvtx[0].sow *= text->width*verts[i].oow;
		verts[i].tmuvtx[0].tow *= text->height*verts[i].oow;
	}
	grTexSource(text->tmu->tmu_id, text->loadAddress,
		GR_MIPMAPLEVELMASK_BOTH, &text->info);
	if(haslight)
	{
		for(i=0;i<num;i++)
		{
			verts[i].tmuvtx[1].sow *= light->width*verts[i].oow; 
			verts[i].tmuvtx[1].tow *= light->height*verts[i].oow; 
		}
		grTexSource(light->tmu->tmu_id, light->loadAddress,
			GR_MIPMAPLEVELMASK_BOTH, &light->info);
		// Perhaps we have to modify the TexCombine for TMU 0 if no lightmap ???
	}	
	else
	{
		// Perhaps we have to modify the TexCombine for TMU 0 if no lightmap ???
	}
	grDrawVertexArrayContiguous(GR_POLYGON, num, verts, sizeof(MyGrVertex));
}

void csGraphics3DGlide3x::RenderPolygonMultiPass (MyGrVertex * verts, int num, bool haslight,TextureHandler*text,TextureHandler*light)
{
	int i;
	for(i=0;i<num;i++)
	{
		verts[i].tmuvtx[0].sow *= text->width*verts[i].oow;
		verts[i].tmuvtx[0].tow *= text->height*verts[i].oow;
	}
	grTexSource(text->tmu->tmu_id, text->loadAddress,
		GR_MIPMAPLEVELMASK_BOTH, &text->info);
  grDrawVertexArrayContiguous(GR_POLYGON, num, verts, sizeof(MyGrVertex));
	if(haslight)
	{
		for(i=0;i<num;i++)
		{
			verts[i].tmuvtx[0].sow= verts[i].tmuvtx[1].sow * light->width*verts[i].oow; 
			verts[i].tmuvtx[0].tow= verts[i].tmuvtx[1].tow * light->height*verts[i].oow; 
		}
		grTexSource(light->tmu->tmu_id, light->loadAddress,
			GR_MIPMAPLEVELMASK_BOTH, &light->info);

		grConstantColorValue(0X80FFFFFF);
    grDrawVertexArrayContiguous(GR_POLYGON, num, verts, sizeof(MyGrVertex));
		grConstantColorValue(0XFFFFFFFF);
	}
}

void csGraphics3DGlide3x::SetupPolygon( G3DPolygon& poly, float& J1, float& J2, float& J3, 
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

STDMETHODIMP csGraphics3DGlide3x::DrawPolygon(G3DPolygon& poly)
{
	MyGrVertex * verts;

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

  if (!pTex)
     return E_INVALIDARG;

	CacheTexture (pTex);
  
  poly.polygon->GetAlpha( poly_alpha );
  is_transparent = poly_alpha ? true : false;

  ITextureMap* piTM;
  IMipMapContainer* piMMC;
  
  pTex->GetTexture( &piTM );
  ASSERT( piTM );
  
  piTM->GetParent( &piMMC );
  ASSERT( piMMC );
  
  piMMC->GetTransparent (is_colorkeyed);
  
  HighColorCacheAndManage_Data* tcache;
	HighColorCacheAndManage_Data* lcache;
  
  // retrieve the cached texture handle.
  piMMC->GetHighColorCache((HighColorCache_Data **)&tcache ); 
  ASSERT( tcache );
	
  FINAL_RELEASE( piTM );
  FINAL_RELEASE( piMMC );
  
  // retrieve the lightmap from the cache.
  ILightMap* piLM = NULL;
  pTex->GetLightMap( &piLM );
  
  if ( piLM )
  {
    piLM->GetHighColorCache((HighColorCache_Data **) &lcache );
    if (!lcache)
    {
      lm_exists = false;
    }
    
    //piLM->Release();
    FINAL_RELEASE(piLM);
  }
  else
  {
    lm_exists=false;
  }

  if(is_transparent)
	{
		GrColor_t c = 0x00FFFFFF;
		c |= ((int)((float)(poly_alpha)/100.0f*255.0f) << 24);
		grConstantColorValue(c);
	}
	else
	{
		grConstantColorValue(0XFFFFFFFF);
	}
	
	verts=new MyGrVertex[poly.num];
	float q,x,y,ooz,z,u,v,lu,lv;
	TextureHandler *thLm=NULL,*thTex = (TextureHandler *)tcache->pData;
/*	if (poly.polygon->theDynLight) // Use a dynamic light if it exists
		q = (float)((poly.polygon->theDynLight->RawIntensity() * 256) >> 16);
	else*/
		q = 255;
	for(int i=0;i<poly.num;i++)
	{
		x = poly.vertices[i].sx;
		y = poly.vertices[i].sy;
		verts[i].x = x + SNAP;
		verts[i].y =/* FRAME_HEIGHT -1 - */y + SNAP; 
		x-=m_nHalfWidth;
		y-=m_nHalfHeight;
		ooz = (M*(x) + N*(y) + O);
		/*verts[i].z =*/ z = 1/ooz;
		u = (J1 * (x) + J2 * (y) + J3)*z;
		v = (K1 * (x) + K2 * (y) + K3)*z;
		verts[i].tmuvtx[0].sow= u; 
		verts[i].tmuvtx[0].tow= v; 
		verts[i].oow /*= verts[i].tmuvtx[0].oow = verts[i].tmuvtx[1].oow */= ooz;
		verts[i].r = q;
		verts[i].g = q;
		verts[i].b = q;
//		verts[i].a = poly.polygon->get_alpha(); // Not used
//		verts[i].x -= SNAP;  // You can forget it
//		verts[i].y -= SNAP;  // This one also
	}
	if(lm_exists)
	{
    float fMinU, fMinV, fMaxU, fMaxV;

    pTex->GetTextureBox( fMinU, fMinV, fMaxU, fMaxV );

    thLm = (TextureHandler *)lcache->pData;
		float lu_dif, lv_dif,lu_scale, lv_scale;
		lu_dif = fMinU;
		lv_dif = fMinV;
		lu_scale = 1.0f/(fMaxU - fMinU);
		lv_scale = 1.0f/(fMaxV - fMinV);
		for(i=0;i<poly.num;i++)
		{
			lu = (verts[i].tmuvtx[0].sow- lu_dif) * lu_scale;
			lv = (verts[i].tmuvtx[0].tow- lv_dif) * lv_scale;
			verts[i].tmuvtx[1].sow= lu; 
			verts[i].tmuvtx[1].tow= lv; 
		}
	}
	
	if(is_colorkeyed)
		grChromakeyMode(GR_CHROMAKEY_ENABLE);

  RenderPolygon(verts,poly.num,lm_exists,thTex,thLm);

  if(is_colorkeyed)
		grChromakeyMode(GR_CHROMAKEY_DISABLE);
	
	delete[] verts;

  return S_OK;
}

/// Draw a projected (non-perspective correct) polygon.
STDMETHODIMP csGraphics3DGlide3x::DrawPolygonQuick (G3DPolygon& poly, bool gouroud)
{
  HighColorCacheAndManage_Data* tcache=NULL;

  if (poly.num < 3) 
		return S_OK;
	
	CHK(MyGrVertex *verts = new MyGrVertex[poly.num]);
	
	float x, y;
	for(int i=0; i<poly.num; i++)
	{
		x = poly.vertices[i].sx;
		y = poly.vertices[i].sy;
		verts[i].x = x + SNAP;
		verts[i].y = m_nHeight -1 - y + SNAP;
		x-=m_nHalfWidth;
		y-=m_nHalfHeight;
		verts[i].r = 255;
		verts[i].g = 255;
		verts[i].b = 255;
		verts[i].oow = poly.pi_texcoords[i].z;
		verts[i].x -= SNAP;
		verts[i].y -= SNAP;
	}
	
	m_pTextureCache->Add(poly.pi_texture);
  poly.pi_texture->GetHighColorCache((HighColorCache_Data **)&tcache ); 
  ASSERT( tcache );
	
  TextureHandler *thTex = (TextureHandler *)tcache->pData;
	grTexSource(thTex->tmu->tmu_id, thTex->loadAddress,
		GR_MIPMAPLEVELMASK_BOTH, &thTex->info);
	
  grDrawVertexArrayContiguous(GR_POLYGON, poly.num, verts, sizeof(MyGrVertex));
	
	delete[] verts;

  return S_OK;
}

/// Give a texture to csGraphics3D to cache it.
STDMETHODIMP csGraphics3DGlide3x::CacheTexture (IPolygonTexture *piPT)
{
  ITextureMap* piTM = NULL;
  IMipMapContainer* piMMC = NULL;
  
  piPT->GetTexture(&piTM);
  ASSERT( piTM );
  
  piTM->GetParent(&piMMC);
  ASSERT( piMMC );
  
  ASSERT( m_pTextureCache != NULL );
  
  m_pTextureCache->Add(piMMC);

  if (m_pLightmapCache) 
  {
    m_pLightmapCache->Add(piPT);
  } 
  
  FINAL_RELEASE( piTM );
  FINAL_RELEASE( piMMC );
  
  return S_OK;  
}

/// Release a texture from the cache.
STDMETHODIMP csGraphics3DGlide3x::UncacheTexture (IPolygonTexture *piPT)
{
	(void)piPT;
  return E_NOTIMPL;
}

/// Dump the texture cache.
STDMETHODIMP csGraphics3DGlide3x::DumpCache(void)
{
	m_pTextureCache->Dump();
	m_pLightmapCache->Dump();
  return S_OK;
}

/// Clear the texture cache.
STDMETHODIMP csGraphics3DGlide3x::ClearCache(void)
{
	if(m_pTextureCache) m_pTextureCache->Clear();
	if(m_pLightmapCache) m_pLightmapCache->Clear();
  return S_OK;
}

STDMETHODIMP csGraphics3DGlide3x::DrawFltLight(G3DFltLight& light)
{
    return E_NOTIMPL;
}

STDMETHODIMP csGraphics3DGlide3x::GetCaps(G3D_CAPS *caps)
{
  if (!caps)
    return E_INVALIDARG;

  memcpy(caps, &m_Caps, sizeof(G3D_CAPS));

  return S_OK;
}

STDMETHODIMP csGraphics3DGlide3x::DrawLine (csVector3& v1, csVector3& v2, int color)
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

STDMETHODIMP csGraphics3DGlide3x::SetRenderState (G3D_RENDERSTATEOPTION op, long val)
{
  switch (op)
	{
/*
  case G3DRENDERSTATE_ZBUFFERTESTENABLE:
		if (val)
			m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);
		else
			m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS);
	break;

	case G3DRENDERSTATE_ZBUFFERFILLENABLE:
		if (val)
			m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
		else
			m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	break;
*/
	default:
		return E_NOTIMPL;
	}

	return S_OK;
}

void csGraphics3DGlide3x::SysPrintf(int mode, char* szMsg, ...)
{
  char buf[1024];
  va_list arg;

  va_start (arg, szMsg);
  vsprintf (buf, szMsg, arg);
  va_end (arg);

  m_piSystem->Print(mode, buf);
}
