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

#if defined(OS_WIN32)
#include <windows.h>
#endif	// OS_WIN32
#include <stdio.h>
#include <stdlib.h>
#include <glide.h>

#include "sysdef.h"
#include "cscom/com.h"
#if defined(OS_WIN32)
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

BEGIN_INTERFACE_TABLE(csGraphics2DGlide2x)
    IMPLEMENTS_COMPOSITE_INTERFACE_EX( IGraphics2D, XGraphics2D )
    IMPLEMENTS_COMPOSITE_INTERFACE_EX( IGraphicsInfo, XGraphicsInfo )
    IMPLEMENTS_COMPOSITE_INTERFACE_EX( IGlide2xGraphicsInfo, XGlide2xGraphicsInfo )
END_INTERFACE_TABLE()

IMPLEMENT_UNKNOWN_NODELETE(csGraphics2DGlide2x)

int csGraphics2DGlide2x::Depth=16;

/////The 2D Graphics Driver//////////////

#if defined(OS_WIN32)

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

#endif

/******************************************
 *         Linux specific things          *
 ******************************************/
#if defined(OS_LINUX)

#define XK_MISCELLANY 1
#include <X11/Xlib.h>
#include <X11/keysymdef.h>

#endif // OS_LINUX                             

csGraphics2DGlide2x::csGraphics2DGlide2x(ISystem* piSystem) : csGraphics2D (piSystem)
{

  locked = false;
	// make a window for rush or banshee ?
#if defined(OS_WIN32)
	m_hWnd=NULL;
   
  IWin32SystemDriver* piW32Sys = NULL;
 
  // QI for IWin32SystemDriver //
  HRESULT res = piSystem->QueryInterface(IID_IWin32SystemDriver, (void**)&piW32Sys);
  if (FAILED(res))
  	  sys_fatalerror("csGraphics2DWin32::Open(QI) -- ISystem passed does not support IWin32SystemDriver.");

  // Get the creation parameters //
  piW32Sys->GetInstance(&gb_hInstance);
  piW32Sys->GetCmdShow(&gb_nCmdShow);
  FINAL_RELEASE(piW32Sys);

	MungeFPCW(&wOldCW);

  CHK(glLib = new GlideLib);
#endif
	
#if defined(OS_LINUX)
        dpy = XOpenDisplay (NULL);

        //CsPrintf(MSG_CONSOLE, "Display allocated dpy=%p\n", dpy);

        screen_num = DefaultScreen (dpy);
        display_width = DisplayWidth (dpy, screen_num);
        display_height = DisplayHeight (dpy, screen_num);

        // No signals handling currently... TODO
        //init_sig ();

        //CsPrintf (MSG_INITIALIZATION, "(using X windows system for input)\n");

#endif // OS_LINUX              

	Depth=16;
	
	DrawPixel = DrawPixel16;   WriteChar = WriteChar16;
	GetPixelAt = GetPixelAt16; DrawSprite = DrawSprite16;
    
	// calculate CS's pixel format structure. 565
	pfmt.PixelBytes = 2;
	pfmt.PalEntries = 0;
	pfmt.RedMask = (1+2+4+8+16)<<11;
	pfmt.GreenMask = (1+2+4+8+16+32)<<5;
	pfmt.BlueMask = (1+2+4+8+16);
    
	complete_pixel_format();
	GraphicsReady=1;  
}

csGraphics2DGlide2x::~csGraphics2DGlide2x(void)
{
	GraphicsReady=0;
	Close();
#if defined(OS_WIN32)
	RestoreFPCW(wOldCW);
	delete glLib;
#endif
}

bool csGraphics2DGlide2x::Open(const char *Title)
{
  if (!csGraphics2D::Open (Title))
		return false;
	
	
#if defined(OS_WIN32)
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
#endif

#if defined(OS_LINUX)
        // Open window
        window = XCreateSimpleWindow (dpy, DefaultRootWindow (dpy), 64, 16,
                                      display_width, display_height, 4, 0, 0);
        XMapWindow (dpy, window);
        XGCValues values;
        gc = XCreateGC (dpy, window, 0, &values);
        XSetGraphicsExposures (dpy, gc, False);
        XSetWindowAttributes attr;
        attr.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
          FocusChangeMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask;
        XChangeWindowAttributes (dpy, window, CWEventMask, &attr);

        // Wait for expose event (why not ?)
        XEvent event;
        for (;;)
          {
            XNextEvent (dpy, &event);
            if (event.type == Expose) break; 	
          }
#endif // OS_LINUX  
	
	bPalettized = false;
	
	bPaletteChanged = false;
	
  return true;
}

void csGraphics2DGlide2x::Close(void)
{
	csGraphics2D::Close ();
}

void csGraphics2DGlide2x::Print (csRect* /*area*/)
{
}

#define GR_DRAWBUFFER GR_BUFFER_FRONTBUFFER

bool csGraphics2DGlide2x::BeginDraw(/*int Flag*/)
{
	FxBool bret;
	lfbInfo.size=sizeof(GrLfbInfo_t);
/*	switch(Flag&(CSDRAW_2DGRAPHICS_READ|CSDRAW_2DGRAPHICS_WRITE))
	{
	case CSDRAW_2DGRAPHICS_READ:
		glDrawMode=GR_LFB_READ_ONLY;
		break;
	case CSDRAW_2DGRAPHICS_WRITE:
		glDrawMode=GR_LFB_WRITE_ONLY;
		break;
	default:
		return false;
	}
*/
  glDrawMode=GR_LFB_WRITE_ONLY;
    
  if(locked)
    FinishDraw();

	bret=GlideLib_grLfbLock(glDrawMode|GR_LFB_IDLE,
		GR_DRAWBUFFER,
		GR_LFBWRITEMODE_565,
		GR_ORIGIN_ANY,
		FXFALSE,
		&lfbInfo);
	if(bret)
	{
		Memory=(unsigned char*)lfbInfo.lfbPtr;
		if(lfbInfo.origin==GR_ORIGIN_UPPER_LEFT)
		{
			for(int i = 0; i < Height; i++)
				LineAddress [i] = i * lfbInfo.strideInBytes;
		}
		else
		{
			int omi = Height-1;
			for(int i = 0; i < Height; i++)
				LineAddress [i] = (omi--) * lfbInfo.strideInBytes;
		}
    locked=true;
	}
	return bret;
}

void csGraphics2DGlide2x::FinishDraw ()
{
  Memory=NULL;
  for(int i = 0; i < Height; i++)
		LineAddress [i] = 0;
	if(locked)
    GlideLib_grLfbUnlock(glDrawMode,GR_DRAWBUFFER);
  locked = false;
}

void csGraphics2DGlide2x::SetRGB(int i, int r, int g, int b)
{
	csGraphics2D::SetRGB (i, r, g, b);
	bPaletteChanged = true;
  SetTMUPalette(0);
}

void csGraphics2DGlide2x::SetTMUPalette(int tmu)
{
  GuTexPalette p;
  RGBpaletteEntry pal;
  
  for(int i=0; i<256; i++)
  {
    pal = Palette[i];
    p.data[i]=0xFF<<24 | pal.red<<16 | pal.green<<8 | pal.blue;
  }
  
  GlideLib_grTexDownloadTable(tmu, GR_TEXTABLE_PALETTE, &p);		
}
