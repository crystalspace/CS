/*
    Copyright (C) 1998 by Jorrit Tyberghein and Dan Ogles
  
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

// GLIDELIB.CPP
// Glide Helper Class
// Written by xtrochu and Nathaniel

#include "cssysdef.h"
#include "gllib.h"
#include "cssys/system.h"

#if defined(OS_WIN32)
#include <windows.h>

GlideLib * glLib;

extern void sys_fatalerror(char *str, HRESULT hRes = S_OK);

// This Define should only work with VC ?
#if !defined(COMP_VC)
#error Please define the LOADFUNCTIONPROC macro for your compiler
#else
#define LOADFUNCTIONPROC(name,decl,stack) \
	{ \
	(name)=decl GetProcAddress(hModule,"_" #name "@" #stack); \
	if((name)==NULL) \
		sys_fatalerror("GlideLib::GlideLib() : Could not find " #name " function"); \
	}
#endif

GlideLib::GlideLib(void)
{
  char * dllpath = "glide2x.dll"/*config->GetStr("Glide","GLIDEDLL","glide2x.dll")*/;
  hModule=LoadLibrary(dllpath);
  if(hModule==NULL)
    sys_fatalerror("GlideLib::GlideLib could not load glide2x.dll");
  LOADFUNCTIONPROC(grGlideInit,(void (__stdcall*)(void)),0);
  LOADFUNCTIONPROC(grGlideShutdown,(void (__stdcall*)(void)),0);
  LOADFUNCTIONPROC(grGlideGetVersion,(void (__stdcall*)(char[80])),4);
  LOADFUNCTIONPROC(grSstSelect,(void (__stdcall*)(int)),4);
  LOADFUNCTIONPROC(grDrawPlanarPolygon,(void (__stdcall *)( int , const int [], const GrVertex [] )),12);
  LOADFUNCTIONPROC(grDrawPlanarPolygonVertexList,(void (__stdcall *)( int , const GrVertex [] )),8);
  LOADFUNCTIONPROC(grDrawPolygon,(void (__stdcall *)( int , const int [], const GrVertex [] )),12);
  LOADFUNCTIONPROC(grDrawPolygonVertexList,(void (__stdcall *)( int , const GrVertex [] )),8);
  LOADFUNCTIONPROC(grDrawPoint,(void (__stdcall *)( const GrVertex * )),4);
  LOADFUNCTIONPROC(grDrawLine,(void (__stdcall *)( const GrVertex *, const GrVertex * )),8);
  LOADFUNCTIONPROC(grDrawTriangle,(void (__stdcall *)( const GrVertex *, const GrVertex *, const GrVertex * )),12);
  LOADFUNCTIONPROC(grBufferClear,(void (__stdcall *)( GrColor_t , GrAlpha_t , FxU16  )),12);
  LOADFUNCTIONPROC(grBufferNumPending,(int  (__stdcall *)( void )),0);
  LOADFUNCTIONPROC(grBufferSwap,(void (__stdcall *)( int  )),4);
  LOADFUNCTIONPROC(grRenderBuffer,(void (__stdcall *)( GrBuffer_t  )),4);
  LOADFUNCTIONPROC(grErrorSetCallback,(void (__stdcall *)( GrErrorCallbackFnc_t  )),4);
  LOADFUNCTIONPROC(grSstWinClose,(void (__stdcall *)( void )),0);
  LOADFUNCTIONPROC(grSstControl,(FxBool (__stdcall *)( FxU32  )),4);
  LOADFUNCTIONPROC(grSstQueryHardware,(FxBool (__stdcall *)( GrHwConfiguration * )),4);
  LOADFUNCTIONPROC(grSstQueryBoards,(FxBool (__stdcall *)( GrHwConfiguration * )),4);
  LOADFUNCTIONPROC(grSstOrigin,(void (__stdcall *)(GrOriginLocation_t  )),4);
  LOADFUNCTIONPROC(grSstScreenHeight,(FxU32 (__stdcall *)( void )),0);
  LOADFUNCTIONPROC(grSstScreenWidth,(FxU32 (__stdcall *)( void )),0);
  LOADFUNCTIONPROC(grSstStatus,(FxU32 (__stdcall *)( void )),0);
  LOADFUNCTIONPROC(grLfbWriteRegion,(FxBool (__stdcall *)( GrBuffer_t , FxU32 , FxU32 , GrLfbSrcFmt_t , FxU32 , FxU32 , FxI32 , void * )),32);
  LOADFUNCTIONPROC(grLfbReadRegion,(FxBool (__stdcall *)( GrBuffer_t , FxU32 , FxU32 , FxU32 , FxU32 , FxU32 , void * )),28);
  LOADFUNCTIONPROC(grSstWinOpen,(FxBool (__stdcall *)(FxU32 , GrScreenResolution_t , GrScreenRefresh_t, GrColorFormat_t, GrOriginLocation_t , int , int )),28);
  LOADFUNCTIONPROC(grColorCombine,(void (__stdcall *)(GrCombineFunction_t , GrCombineFactor_t , GrCombineLocal_t , GrCombineOther_t , FxBool  )),20);
  LOADFUNCTIONPROC(grColorMask,(void (__stdcall *)( FxBool , FxBool  )),8);
  LOADFUNCTIONPROC(grCullMode,(void (__stdcall *)( GrCullMode_t  )),4);
  LOADFUNCTIONPROC(grDepthBufferFunction,(void (__stdcall *)( GrCmpFnc_t  )),4);
  LOADFUNCTIONPROC(grDepthBufferMode,(void (__stdcall *)( GrDepthBufferMode_t  )),4);
  LOADFUNCTIONPROC(grDepthMask,(void (__stdcall *)( FxBool  )),4);
  LOADFUNCTIONPROC(grTexCombineFunction,(void (__stdcall *)(GrChipID_t , GrTextureCombineFnc_t )),8);
  LOADFUNCTIONPROC(grLfbLock,(FxBool (__stdcall *)( GrLock_t , GrBuffer_t , GrLfbWriteMode_t , GrOriginLocation_t , FxBool , GrLfbInfo_t * )),24);
  LOADFUNCTIONPROC(grLfbUnlock,(FxBool (__stdcall *)( GrLock_t , GrBuffer_t  )),8);
  LOADFUNCTIONPROC(grTexCalcMemRequired,(FxU32 (__stdcall *)(GrLOD_t,GrLOD_t,GrAspectRatio_t,GrTextureFormat_t)),16);
  LOADFUNCTIONPROC(grTexTextureMemRequired,(FxU32 (__stdcall *)( FxU32,GrTexInfo*)),8);
  LOADFUNCTIONPROC(grTexMinAddress,(FxU32 (__stdcall*)(GrChipID_t)),4);
  LOADFUNCTIONPROC(grTexMaxAddress,(FxU32 (__stdcall*)(GrChipID_t)),4);
  LOADFUNCTIONPROC(grTexSource,(void (__stdcall *)(GrChipID_t,FxU32,FxU32,GrTexInfo*)),16);
  LOADFUNCTIONPROC(grTexDownloadMipMap,(void (__stdcall *)(GrChipID_t,FxU32,FxU32,GrTexInfo*)),16);
  LOADFUNCTIONPROC(grTexDownloadMipMapLevel,(void (__stdcall *)(GrChipID_t,FxU32,GrLOD_t,GrLOD_t,GrAspectRatio_t,GrTextureFormat_t, FxU32, void *)),32);
  LOADFUNCTIONPROC(grTexDownloadTable,(void (__stdcall *)(GrChipID_t,GrTexTable_t,void *)),12);
  LOADFUNCTIONPROC(grTexClampMode,(void (__stdcall *)(GrChipID_t,GrTextureClampMode_t,GrTextureClampMode_t)),12);
  LOADFUNCTIONPROC(grTexCombine,(void (__stdcall *)(GrChipID_t,GrCombineFunction_t,GrCombineFactor_t,GrCombineFunction_t,GrCombineFactor_t,FxBool,FxBool)),28);
  LOADFUNCTIONPROC(grConstantColorValue,(void (__stdcall *)( GrColor_t  )),4);
  LOADFUNCTIONPROC(grAlphaCombine,(void (__stdcall *)(GrCombineFunction_t,GrCombineFactor_t,GrCombineLocal_t,GrCombineOther_t,FxBool)),20);
  LOADFUNCTIONPROC(grAlphaBlendFunction,(void (__stdcall *)(GrAlphaBlendFnc_t,GrAlphaBlendFnc_t,GrAlphaBlendFnc_t,GrAlphaBlendFnc_t)),16);
  LOADFUNCTIONPROC(grTexLodBiasValue,(void (__stdcall *)(GrChipID_t,float)),8);
  LOADFUNCTIONPROC(grTexFilterMode,(void (__stdcall *)(GrChipID_t,GrTextureFilterMode_t,GrTextureFilterMode_t)),12);
  LOADFUNCTIONPROC(grTexMipMapMode,(void (__stdcall *)(GrChipID_t,GrMipMapMode_t,FxBool)),12);
  LOADFUNCTIONPROC(grChromakeyMode,(void (__stdcall *)(GrChromakeyMode_t mode)), 4);
  LOADFUNCTIONPROC(grChromakeyValue,(void (__stdcall *)(GrColor_t color)), 4);
  LOADFUNCTIONPROC(grHints,(void (__stdcall *)(GrHint_t,FxU32)),8);
  LOADFUNCTIONPROC(grFogColorValue,(void (__stdcall *)(GrColor_t)),4);
  LOADFUNCTIONPROC(grFogMode,(void (__stdcall *)(GrFogMode_t)),4);
  LOADFUNCTIONPROC(grFogTable,(void (__stdcall *)(GrFog_t)),4);
  LOADFUNCTIONPROC(grClipWindow,(void (__stdcall *)(FxU32 minx, FxU32 miny, FxU32 maxx, FxU32 maxy )),16);
  LOADFUNCTIONPROC(grGlideGetState,(void (__stdcall *)(GrState*)),4);
  LOADFUNCTIONPROC(grGlideSetState,(void (__stdcall *)(GrState*)),4);
  LOADFUNCTIONPROC(grDisableAllEffects,(void (__stdcall *)(void)),0);
}

/*
	LOADFUNCTIONPROC(grDrawPlanarPolygon,(),);
	void (__stdcall *)(GrHint_t,FxU32);
	void (__stdcall *)(GrChipID_t,GrTextureFilterMode_t,GrTextureFilterMode_t);
	void (__stdcall *)(GrChipID_t,GrMipMapMode_t,FxBool);
*/

GlideLib::~GlideLib()
{
	FreeLibrary(hModule);
}


#endif // OS_WIN32
