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

// G2D_GLIDE.CPP
// csGraphics2DGlide2x implementation file
// Written by xtrochu and Nathaniel

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <glide.h>

#include "sysdef.h"
#include "csutil/scf.h"
#include "cssys/win32/win32itf.h"
#endif
#include "isystem.h"

extern void sys_fatalerror(char *str, HRESULT hRes = S_OK);
extern void out(char *str, ...);

#include "cs2d/winglide2/g2d.h"
#include "cs2d/winglide2/ig2d.h"
#include "cs2d/winglide2/xg2d.h"
#include "cs3d/glide2/g3dglide.h"
#include "cs3d/glide2/glidelib.h"

IMPLEMENT_FACTORY (csGraphics2DGlide2x)

EXPORT_CLASS_TABLE (glide2d)
  EXPORT_CLASS (csGraphics2DGlide2x, "crystalspace.graphics2d.glide2",
    "Glide/Win32 v2 2D graphics driver for Crystal Space")
EXPORT_CLASS_TABLE_END


/////The 2D Graphics Driver//////////////

HINSTANCE gb_hInstance;
int gb_nCmdShow;
static WORD wOldCW;

static BOOL MungeFPCW( WORD *pwOldCW )
{    
	BOOL ret = FALSE;    
	WORD wTemp, wSave; 
    __asm fstcw wSave    
	if (wSave & 0x300 ||            // Not single mode
		0x3f != (wSave & 0x3f) ||   // Exceptions enabled
		wSave & 0xC00)              // Not round to nearest mode    
	{
		__asm {            
			mov ax, wSave
	        and ax, not 300h    ;; single mode
		    or  ax, 3fh         ;; disable all exceptions
			and ax, not 0xC00   ;; round to nearest mode
			mov wTemp, ax            
			fldcw   wTemp        
		}        
		ret = TRUE;
	}    
	*pwOldCW = wSave;    
	return ret;
} 

static void RestoreFPCW(WORD wSave)
{
    __asm fldcw wSave
} 


csGraphics2DGlide2x::csGraphics2DGlide2x(iBase *iParent) : csGraphics2DGlideCommon ()
{
  CONSTRUCT_IBASE (iParent);
  // make a window for rush or banshee ?
}

csGraphics2DGlide2x::~csGraphics2DGlide2x(void)
{
  GraphicsReady=0;
  Close();
  RestoreFPCW(wOldCW);
  delete glLib;
}

bool csGraphics2DGlide2x::Initialize (iSystem *pSystem)
{
  if (!csGraphics2D::GlideCommonInitialize (pSystem))
    return false;

  m_hWnd=NULL;
   
  iWin32SystemDriver* piW32Sys = NULL;
 
  // QI for iWin32SystemDriver //
  piW32Sys = QUERY_INTERFACE (System, iWin32SystemDriver);
  if (!piW32Sys)
  	  sys_fatalerror("csGraphics2DWin32::Open(QI) -- iSystem passed does not support iWin32SystemDriver.");

  // Get the creation parameters //
  piW32Sys->GetInstance(&gb_hInstance);
  piW32Sys->GetCmdShow(&gb_nCmdShow);
  piW32Sys->DecRef ();

  MungeFPCW(&wOldCW);

  CHK(glLib = new GlideLib);
	
  // calculate CS's pixel format structure. 565
  pfmt.PixelBytes = 2;
  pfmt.PalEntries = 0;
  pfmt.RedMask = (1+2+4+8+16)<<11;
  pfmt.GreenMask = (1+2+4+8+16+32)<<5;
  pfmt.BlueMask = (1+2+4+8+16);

  complete_pixel_format();
  GraphicsReady=1;
  return true;
}

bool csGraphics2DGlide2x::Open(const char *Title)
{
  if (!csGraphics2DGlideCommon::Open (Title))
		return false;
	
	// create the window.
  DWORD exStyle = 0;
  DWORD style = WS_POPUP;
  if (!FullScreen)
	  style |= WS_CAPTION;
  
  int wwidth,wheight;
  wwidth=Width+2*GetSystemMetrics(SM_CXSIZEFRAME);
  wheight=Height+2*GetSystemMetrics(SM_CYSIZEFRAME)+GetSystemMetrics(SM_CYCAPTION);

  m_hWnd = CreateWindowEx(exStyle, WINDOWCLASSNAME, Title, style,
	                      (GetSystemMetrics(SM_CXSCREEN)-wwidth)/2,
                          (GetSystemMetrics(SM_CYSCREEN)-wheight)/2,
                          wwidth, wheight, NULL, NULL, gb_hInstance, NULL );
  if( !m_hWnd )
    sys_fatalerror("Cannot create CrystalSpace Glide window", GetLastError());
  
  ShowWindow( m_hWnd, gb_nCmdShow );
  UpdateWindow( m_hWnd );
  SetFocus( m_hWnd );

  // hwnd is window handler used by banshee/rush card
	GlideLib_grSstControlMode(GR_CONTROL_ACTIVATE);
	
    bPalettized = false;
	
  return true;
}

void csGraphics2DGlide2x::Close(void)
{
	csGraphics2DGlideCommon::Close ();
}

HWND csGraphics2DGlide2x::GethWnd ()
{
  return m_hWnd;
}
