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

// GLIDELIB.H
// RunTime Linking Routine with Glide DLLs
// Written by xtrochu and Nathaniel

#if !defined(GLIDELIB_H_INCLUDED)

#include <glide.h>

#if defined(OS_WIN32)

#include <windows.h>
/// Glide Library 
class GlideLib
{
	/// Handle to DLL
	HMODULE hModule;	
public:
	void (__stdcall *grGlideInit)(void);
	void (__stdcall *grGlideShutdown)(void);
	void (__stdcall *grGlideGetVersion)( char version[80] );
	FxBool (__stdcall *grSstWinOpen)(FxU32 hWnd,
          GrScreenResolution_t screen_resolution,
          GrScreenRefresh_t    refresh_rate,
          GrColorFormat_t      color_format,
          GrOriginLocation_t   origin_location,
          int nColBuffers,
          int nAuxBuffers);
	void (__stdcall *grDrawPlanarPolygon)( int nverts, const int ilist[], const GrVertex vlist[] );
	void (__stdcall *grDrawPlanarPolygonVertexList)( int nverts, const GrVertex vlist[] );
	void (__stdcall *grDrawPolygon)( int nverts, const int ilist[], const GrVertex vlist[] );
	void (__stdcall *grDrawPolygonVertexList)( int nverts, const GrVertex vlist[] );
	void (__stdcall *grDrawPoint)( const GrVertex *pt );
	void (__stdcall *grDrawLine)( const GrVertex *v1, const GrVertex *v2 );
	void (__stdcall *grDrawTriangle)( const GrVertex *a, const GrVertex *b, const GrVertex *c );
	void (__stdcall *grBufferClear)( GrColor_t color, GrAlpha_t alpha, FxU16 depth );
	int  (__stdcall *grBufferNumPending)( void );
	void (__stdcall *grBufferSwap)( int swap_interval );
	void (__stdcall *grRenderBuffer)( GrBuffer_t buffer );
	void (__stdcall *grErrorSetCallback)( GrErrorCallbackFnc_t fnc );
	void (__stdcall *grSstWinClose)( void );
	FxBool (__stdcall *grSstControl)( FxU32 code );
	FxBool (__stdcall *grSstQueryHardware)( GrHwConfiguration *hwconfig );
	FxBool (__stdcall *grSstQueryBoards)( GrHwConfiguration *hwconfig );
	void (__stdcall *grSstOrigin)(GrOriginLocation_t  origin);
	void (__stdcall *grSstSelect)( int which_sst );
	FxU32 (__stdcall *grSstScreenHeight)( void );
	FxU32 (__stdcall *grSstScreenWidth)( void );
	FxU32 (__stdcall *grSstStatus)( void );
	FxBool (__stdcall *grLfbWriteRegion)( GrBuffer_t dst_buffer, FxU32 dst_x, FxU32 dst_y, GrLfbSrcFmt_t src_format, FxU32 src_width, FxU32 src_height, FxI32 src_stride, void *src_data );
	void (__stdcall *grColorCombine)(
               GrCombineFunction_t function, GrCombineFactor_t factor,
               GrCombineLocal_t local, GrCombineOther_t other,
               FxBool invert );
	void (__stdcall *grColorMask)( FxBool rgb, FxBool a );
	void (__stdcall *grCullMode)( GrCullMode_t mode );
	void (__stdcall *grDepthBufferFunction)( GrCmpFnc_t function );
	void (__stdcall *grDepthBufferMode)( GrDepthBufferMode_t mode );
	void (__stdcall *grDepthMask)( FxBool mask );
	void (__stdcall *grTexCombineFunction)(GrChipID_t tmu, GrTextureCombineFnc_t fnc);
	FxBool (__stdcall *grLfbLock)( GrLock_t type, GrBuffer_t buffer, GrLfbWriteMode_t writeMode, GrOriginLocation_t origin, FxBool pixelPipeline, GrLfbInfo_t *info );
	FxBool (__stdcall *grLfbUnlock)( GrLock_t type, GrBuffer_t buffer );
	FxU32 (__stdcall *grTexCalcMemRequired)(GrLOD_t lodmin, GrLOD_t lodmax,GrAspectRatio_t aspect, GrTextureFormat_t fmt);
	FxU32 (__stdcall *grTexTextureMemRequired)( FxU32     evenOdd,GrTexInfo *info );
	FxU32 (__stdcall *grTexMinAddress)( GrChipID_t tmu );
	FxU32 (__stdcall *grTexMaxAddress)( GrChipID_t tmu );
	void (__stdcall *grTexSource)( GrChipID_t tmu,FxU32 startAddress,FxU32 evenOdd,GrTexInfo  *info );
	void (__stdcall *grTexClampMode)(GrChipID_t tmu,GrTextureClampMode_t s_clampmode,GrTextureClampMode_t t_clampmode);
	void (__stdcall *grTexCombine)(GrChipID_t tmu,GrCombineFunction_t rgb_function,GrCombineFactor_t rgb_factor,GrCombineFunction_t alpha_function,GrCombineFactor_t alpha_factor,FxBool rgb_invert,FxBool alpha_invert);
	void (__stdcall *grTexDownloadMipMap)( GrChipID_t tmu,FxU32 startAddress,FxU32 evenOdd,GrTexInfo  *info );
  void (__stdcall *grTexDownloadMipMapLevel)(GrChipID_t tmu, FxU32 startAddress, GrLOD_t thisLOD, GrLOD_t largeLOD, GrAspectRatio_t aspect, GrTextureFormat_t format, FxU32 evenOdd, void  *data);
	void (__stdcall *grTexDownloadTable)( GrChipID_t tmu,GrTexTable_t type_table, void *data);
  void (__stdcall *grConstantColorValue)( GrColor_t value );
	void (__stdcall *grAlphaCombine)(GrCombineFunction_t function, GrCombineFactor_t factor, GrCombineLocal_t local, GrCombineOther_t other, FxBool invert);
	void (__stdcall *grAlphaBlendFunction)(GrAlphaBlendFnc_t rgb_sf,   GrAlphaBlendFnc_t rgb_df, GrAlphaBlendFnc_t alpha_sf, GrAlphaBlendFnc_t alpha_df);
	void (__stdcall *grTexLodBiasValue)(GrChipID_t tmu, float bias );
	void (__stdcall *grTexFilterMode)(GrChipID_t tmu,GrTextureFilterMode_t minfilter_mode,GrTextureFilterMode_t magfilter_mode);
	void (__stdcall *grTexMipMapMode)(GrChipID_t tmu, GrMipMapMode_t mode,FxBool lodBlend );
	void (__stdcall *grChromakeyMode)(GrChromakeyMode_t mode);
  void (__stdcall *grChromakeyValue)(GrColor_t color);
	void (__stdcall *grHints)(GrHint_t hintType, FxU32 hintMask);
  void (__stdcall *grClipWindow)(FxU32 minx, FxU32 miny, FxU32 maxx, FxU32 maxy );

	GlideLib(void);
	~GlideLib();
};


extern GlideLib * glLib;

#define GlideLib_grGlideInit glLib->grGlideInit
#define GlideLib_grGlideShutdown glLib->grGlideShutdown
#define GlideLib_grGlideGetVersion glLib->grGlideGetVersion
#define GlideLib_grSstWinOpen glLib->grSstWinOpen
#define GlideLib_grSstSelect glLib->grSstSelect
#define GlideLib_grDrawPlanarPolygon glLib->grDrawPlanarPolygon
#define GlideLib_grDrawPlanarPolygonVertexList glLib->grDrawPlanarPolygonVertexList
#define GlideLib_grDrawPolygon glLib->grDrawPolygon
#define GlideLib_grDrawPolygonVertexList glLib->grDrawPolygonVertexList
#define GlideLib_grDrawPoint glLib->grDrawPoint
#define GlideLib_grDrawLine glLib->grDrawLine
#define GlideLib_grDrawTriangle glLib->grDrawTriangle
#define GlideLib_grBufferClear glLib->grBufferClear
#define GlideLib_grBufferNumPending glLib->grBufferNumPending
#define GlideLib_grBufferSwap glLib->grBufferSwap
#define GlideLib_grRenderBuffer glLib->grRenderBuffer
#define GlideLib_grErrorSetCallback glLib->grErrorSetCallback
#define GlideLib_grSstWinClose glLib->grSstWinClose
#define GlideLib_grSstControl glLib->grSstControl
#define GlideLib_grSstQueryHardware glLib->grSstQueryHardware
#define GlideLib_grSstQueryBoards glLib->grSstQueryBoards
#define GlideLib_grSstOrigin glLib->grSstOrigin
#define GlideLib_grSstScreenHeight glLib->grSstScreenHeight
#define GlideLib_grSstScreenWidth glLib->grSstScreenWidth
#define GlideLib_grSstStatus glLib->grSstStatus
#define GlideLib_grLfbWriteRegion glLib->grLfbWriteRegion
#define GlideLib_grColorCombine glLib->grColorCombine
#define GlideLib_grColorMask glLib->grColorMask
#define GlideLib_grCullMode glLib->grCullMode
#define GlideLib_grDepthBufferFunction glLib->grDepthBufferFunction
#define GlideLib_grDepthBufferMode glLib->grDepthBufferMode
#define GlideLib_grDepthMask glLib->grDepthMask
#define GlideLib_grTexCombineFunction glLib->grTexCombineFunction
#define GlideLib_grLfbLock glLib->grLfbLock
#define GlideLib_grLfbUnlock glLib->grLfbUnlock
#define GlideLib_grTexCalcMemRequired glLib->grTexCalcMemRequired
#define GlideLib_grTexTextureMemRequired glLib->grTexTextureMemRequired
#define GlideLib_grTexMinAddress glLib->grTexMinAddress
#define GlideLib_grTexMaxAddress glLib->grTexMaxAddress
#define GlideLib_grTexSource glLib->grTexSource
#define GlideLib_grTexClampMode glLib->grTexClampMode
#define GlideLib_grTexCombine glLib->grTexCombine
#define GlideLib_grTexDownloadMipMap glLib->grTexDownloadMipMap
#define GlideLib_grTexDownloadMipMapLevel glLib->grTexDownloadMipMapLevel
#define GlideLib_grTexDownloadTable glLib->grTexDownloadTable
#define GlideLib_grConstantColorValue glLib->grConstantColorValue
#define GlideLib_grAlphaCombine glLib->grAlphaCombine
#define GlideLib_grAlphaBlendFunction glLib->grAlphaBlendFunction
#define GlideLib_grTexLodBiasValue glLib->grTexLodBiasValue
#define GlideLib_grTexFilterMode glLib->grTexFilterMode
#define GlideLib_grTexMipMapMode glLib->grTexMipMapMode
#define GlideLib_grChromakeyMode glLib->grChromakeyMode
#define GlideLib_grChromakeyValue glLib->grChromakeyValue
#define GlideLib_grHints glLib->grHints
#define GlideLib_grClipWindow glLib->grClipWindow
#define GlideLib_grFogTable glLib->grFogTable
#define GlideLib_grFogColorValue glLib->grFogColorValue
#define GlideLib_grFogMode glLib->grFogMode

#else // !OS_WIN32

#define GlideLib_grGlideInit grGlideInit
#define GlideLib_grGlideShutdown grGlideShutdown
#define GlideLib_grGlideGetVersion grGlideGetVersion
#define GlideLib_grSstWinOpen grSstWinOpen
#define GlideLib_grSstSelect grSstSelect
#define GlideLib_grDrawPlanarPolygon grDrawPlanarPolygon
#define GlideLib_grDrawPlanarPolygonVertexList grDrawPlanarPolygonVertexList
#define GlideLib_grDrawPolygon grDrawPolygon
#define GlideLib_grDrawPolygonVertexList grDrawPolygonVertexList
#define GlideLib_grDrawPoint grDrawPoint
#define GlideLib_grDrawLine grDrawLine
#define GlideLib_grDrawTriangle grDrawTriangle
#define GlideLib_grBufferClear grBufferClear
#define GlideLib_grBufferNumPending grBufferNumPending
#define GlideLib_grBufferSwap grBufferSwap
#define GlideLib_grRenderBuffer grRenderBuffer
#define GlideLib_grErrorSetCallback grErrorSetCallback
#define GlideLib_grSstWinClose grSstWinClose
#define GlideLib_grSstControl grSstControl
#define GlideLib_grSstQueryHardware grSstQueryHardware
#define GlideLib_grSstQueryBoards grSstQueryBoards
#define GlideLib_grSstOrigin grSstOrigin
#define GlideLib_grSstScreenHeight grSstScreenHeight
#define GlideLib_grSstScreenWidth grSstScreenWidth
#define GlideLib_grSstStatus grSstStatus
#define GlideLib_grLfbWriteRegion grLfbWriteRegion
#define GlideLib_grColorCombine grColorCombine
#define GlideLib_grColorMask grColorMask
#define GlideLib_grCullMode grCullMode
#define GlideLib_grDepthBufferFunction grDepthBufferFunction
#define GlideLib_grDepthBufferMode grDepthBufferMode
#define GlideLib_grDepthMask grDepthMask
#define GlideLib_grTexCombineFunction grTexCombineFunction
#define GlideLib_grLfbLock grLfbLock
#define GlideLib_grLfbUnlock grLfbUnlock
#define GlideLib_grTexCalcMemRequired grTexCalcMemRequired
#define GlideLib_grTexTextureMemRequired grTexTextureMemRequired
#define GlideLib_grTexMinAddress grTexMinAddress
#define GlideLib_grTexMaxAddress grTexMaxAddress
#define GlideLib_grTexSource grTexSource
#define GlideLib_grTexClampMode grTexClampMode
#define GlideLib_grTexCombine grTexCombine
#define GlideLib_grTexDownloadMipMap grTexDownloadMipMap
#define GlideLib_grTexDownloadMipMapLevel grTexDownloadMipMapLevel
#define GlideLib_grTexDownloadTable grTexDownloadTable
#define GlideLib_grConstantColorValue grConstantColorValue
#define GlideLib_grAlphaCombine grAlphaCombine
#define GlideLib_grAlphaBlendFunction grAlphaBlendFunction
#define GlideLib_grTexLodBiasValue grTexLodBiasValue
#define GlideLib_grTexFilterMode grTexFilterMode
#define GlideLib_grTexMipMapMode grTexMipMapMode
#define GlideLib_grChromakeyMode grChromakeyMode
#define GlideLib_grChromakeyValue grChromakeyValue
#define GlideLib_grHints grHints
#define GlideLib_grClipWindow grClipWindow
#define GlideLib_grFogTable grFogTable
#define GlideLib_grFogColorValue grFogColorValue
#define GlideLib_grFogMode grFogMode

#endif // OS_WIN32

#define GLIDELIB_H_INCLUDED
#endif

// To be correct with the doc
#define GlideLib_grSstControlMode GlideLib_grSstControl
