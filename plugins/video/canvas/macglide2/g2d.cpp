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
#include "csutil/scf.h"
#if defined(OS_WIN32)
#include "cssys/win32/win32itf.h"
#endif
#include "isystem.h"

extern void sys_fatalerror(char *str, HRESULT hRes = S_OK);
extern void out(char *str, ...);

#include "cs2d/macglide2/g2d.h"
#include "cs2d/macglide2/ig2d.h"
#include "cs2d/macglide2/xg2d.h"
#include "cs3d/glide2/g3dglide.h"
#include "cs3d/glide2/glidelib.h"

IMPLEMENT_FACTORY (csGraphics2DGlide2x)

EXPORT_CLASS_TABLE (glide2)
  EXPORT_CLASS (csGraphics2DGlide2x, "crystalspace.graphics2d.glide2",
    "Glide v2 2D graphics driver for Crystal Space")
EXPORT_CLASS_TABLE_END

/////The 2D Graphics Driver//////////////

#if defined(OS_WIN32)

HINSTANCE gb_hInstance;
int gb_nCmdShow;
static WORD wOldCW;

static COMBOOL MungeFPCW( WORD *pwOldCW )
{    
	COMBOOL ret = FALSE;    
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

csGraphics2DGlide2x::csGraphics2DGlide2x(iBase *iParent) : csGraphics2DGlideCommon ()
{
  CONSTRUCT_IBASE (iParent);
}

bool csGraphics2DGlide2x::Initialize (iSystem *pSystem)
{
  if (!csGraphics2DGlideCommon::Initialize (pSystem))
    return false;

  // make a window for rush or banshee ?
#if defined(OS_WIN32)
  m_hWnd=NULL;
   
  // QI for iWin32SystemDriver //
  iWin32SystemDriver *piW32Sys = QUERY_INTERFACE (System, iWin32SystemDriver);
  if (!piW32Sys)
  	  sys_fatalerror("csGraphics2DWin32::Open(QI) -- iSystem passed does not support iWin32SystemDriver.");

  // Get the creation parameters //
  piW32Sys->GetInstance(&gb_hInstance);
  piW32Sys->GetCmdShow(&gb_nCmdShow);
  piW32Sys->DecRef ();

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

  // calculate CS's pixel format structure. 565
  pfmt.PixelBytes = 2;
  pfmt.PalEntries = 0;
  pfmt.RedMask = (1+2+4+8+16)<<11;
  pfmt.GreenMask = (1+2+4+8+16+32)<<5;
  pfmt.BlueMask = (1+2+4+8+16);
    
  complete_pixel_format();
  GraphicsReady = 1;
  return true;
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
  if (!csGraphics2DGlideCommon::Open (Title))
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
	
  return true;
}

void csGraphics2DGlide2x::Close(void)
{
	csGraphics2DGlideCommon::Close ();
}

#if defined(OS_WIN32)
HWND csGraphics2DGlide2x::GethWnd ()
{
  return m_hWnd;
}
#endif

#if defined(OS_LINUX)
Display *csGraphics2DGlide2x::GetDisplay ()
{
  return dpy;
}
#endif    
