/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#include "cssysdef.h"
#include <windows.h>
#include <gl/gl.h>

#include "csutil/scf.h"
#include "oglg2d.h"
#include "isys/system.h"

CS_IMPLEMENT_PLUGIN

#ifndef CDS_UPDATEREGISTRY
#  define CDS_UPDATEREGISTRY  0x00000001
#endif
#ifndef CDS_TEST
#define CDS_TEST            0x00000002
#endif
#ifndef CDS_FULLSCREEN
#  define CDS_FULLSCREEN      0x00000004
#endif
#ifndef CDS_GLOBAL
#define CDS_GLOBAL          0x00000008
#endif
#ifndef CDS_SET_PRIMARY
#define CDS_SET_PRIMARY     0x00000010
#endif
#ifndef CDS_RESET
#define CDS_RESET           0x40000000
#endif
#ifndef CDS_SETRECT
#define CDS_SETRECT         0x20000000
#endif
#ifndef CDS_NORESET
#define CDS_NORESET         0x10000000
#endif

/* Return values for ChangeDisplaySettings */
#ifndef DISP_CHANGE_SUCCESSFUL
#define DISP_CHANGE_SUCCESSFUL       0
#endif
#ifndef DISP_CHANGE_RESTART
#define DISP_CHANGE_RESTART          1
#endif
#ifndef DISP_CHANGE_FAILED
#define DISP_CHANGE_FAILED          -1
#endif
#ifndef DISP_CHANGE_BADMODE
#define DISP_CHANGE_BADMODE         -2
#endif
#ifndef DISP_CHANGE_NOTUPDATED
#define DISP_CHANGE_NOTUPDATED      -3
#endif
#ifndef DISP_CHANGE_BADFLAGS
#define DISP_CHANGE_BADFLAGS        -4
#endif
#ifndef DISP_CHANGE_BADPARAM
#define DISP_CHANGE_BADPARAM        -5
#endif

#define WINDOW_STYLE (WS_POPUP | WS_MINIMIZEBOX | WS_POPUP | WS_SYSMENU)

static void sys_fatalerror(char *str, HRESULT hRes = S_OK)
{
  LPVOID lpMsgBuf;
  char* szMsg;
  char szStdMessage[] = "Last Error: ";
  if (FAILED(hRes))
  {
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
      
      MessageBox (NULL, szMsg, "Fatal Error in OpenGL2D.dll", MB_OK);
      delete szMsg;

      exit(1);
    }
  }

  MessageBox(NULL, str, "Fatal Error in OpenGL2D.dll", MB_OK);
  
  exit(1);
}

/////The 2D Graphics Driver//////////////

SCF_IMPLEMENT_FACTORY (csGraphics2DOpenGL)

SCF_EXPORT_CLASS_TABLE (glwin32)
  SCF_EXPORT_CLASS_DEP (csGraphics2DOpenGL, "crystalspace.graphics2d.glwin32",
    "Win32 OpenGL 2D graphics driver for Crystal Space", "crystalspace.font.server.")
SCF_EXPORT_CLASS_TABLE_END

///// Windowed-mode palette stuff //////

static struct {
  WORD Version;
  WORD NumberOfEntries;
  PALETTEENTRY aEntries[256];
} SysPalette = 
{
  0x300,
    256
};

static HPALETTE hWndPalette=NULL;

static void ClearSystemPalette()
{
  struct 
  {
    WORD Version;
    WORD nEntries;
    PALETTEENTRY aEntries[256];
  } Palette =
  {
    0x300,
      256
  };
  
  HPALETTE BlackPal, OldPal;
  HDC hdc;
  
  int c;
  
  for(c=0; c<256; c++)
  {
    Palette.aEntries[c].peRed = 0;
    Palette.aEntries[c].peGreen = 0;
    Palette.aEntries[c].peBlue = 0;
    Palette.aEntries[c].peFlags = PC_NOCOLLAPSE;
  }
  
  hdc = GetDC(NULL);
  
  BlackPal = CreatePalette((LOGPALETTE *)&Palette);
  
  OldPal = SelectPalette(hdc,BlackPal,FALSE);
  RealizePalette(hdc);
  SelectPalette(hdc, OldPal, FALSE);
  DeleteObject(BlackPal);
  
  ReleaseDC(NULL, hdc);
}

static void CreateIdentityPalette(csRGBpixel *p)
{
  int i;
  struct {
    WORD Version;
    WORD nEntries;
    PALETTEENTRY aEntries[256];
  } Palette = 
  {
    0x300,
      256
  };
  
  if(hWndPalette)
    DeleteObject(hWndPalette);
  
  Palette.aEntries[0].peFlags = 0;
  Palette.aEntries[0].peFlags = 0;
  
  for(i=1; i<255; i++)
  {
    Palette.aEntries[i].peRed = p[i].red;
    Palette.aEntries[i].peGreen = p[i].green;
    Palette.aEntries[i].peBlue = p[i].blue;
    Palette.aEntries[i].peFlags = PC_RESERVED;
  }
  
  hWndPalette = CreatePalette((LOGPALETTE *)&Palette);
  
  if(!hWndPalette) 
    sys_fatalerror("Error creating identity palette.");
}

csGraphics2DOpenGL::csGraphics2DOpenGL(iBase *iParent) : 
                   csGraphics2DGLCommon (iParent),
                   m_nGraphicsReady(true),
                   m_hWnd(NULL),
                   m_piWin32System(NULL),
                   m_bPalettized(false),
                   m_bPaletteChanged(false)
{
}

csGraphics2DOpenGL::~csGraphics2DOpenGL(void)
{
  m_piWin32System->DecRef ();
  m_nGraphicsReady=0;
}

bool csGraphics2DOpenGL::Initialize (iSystem *pSystem)
{
  if (!csGraphics2DGLCommon::Initialize (pSystem))
    return false;

  // QI for iWin32SystemDriver //
  m_piWin32System = SCF_QUERY_INTERFACE (System, iWin32SystemDriver);
  if (!m_piWin32System)
      sys_fatalerror("csGraphics2DDDraw3::Open(QI) -- iSystem passed does not support iWin32SystemDriver.");
  
  // Get the creation parameters //
  m_hInstance = m_piWin32System->GetInstance();
  m_nCmdShow  = m_piWin32System->GetCmdShow();

  System->GetSettings(Width, Height, Depth, FullScreen);
    
  if (Depth == 8)
  {
    pfmt.PalEntries = 256;
    pfmt.PixelBytes = 1;
  }
  
  CsPrintf (CS_MSG_INITIALIZATION, "Using %d bits per pixel (%d color mode).\n", Depth, 1 << Depth);
  return true;
}

void csGraphics2DOpenGL::CalcPixelFormat ()
{
  PIXELFORMATDESCRIPTOR pfd = {
      sizeof(PIXELFORMATDESCRIPTOR),  /* size */
      1,                              /* version */
      PFD_SUPPORT_OPENGL |
      PFD_DOUBLEBUFFER |              /* support double-buffering */
      PFD_DRAW_TO_WINDOW,
      PFD_TYPE_RGBA,                  /* color type */
      Depth,                          /* prefered color depth */
      0, 0, 0, 0, 0, 0,               /* color bits (ignored) */
      0,                              /* no alpha buffer */
      0,                              /* alpha bits (ignored) */
      0,                              /* no accumulation buffer */
      0, 0, 0, 0,                     /* accum bits (ignored) */
      16,                             /* depth buffer */
      1,                              /* no stencil buffer */
      0,                              /* no auxiliary buffers */
      PFD_MAIN_PLANE,                 /* main layer */
      0,                              /* reserved */
      0, 0, 0                         /* no layer, visible, damage masks */
  };
  
  int pixelFormat;
  pixelFormat = ChoosePixelFormat(hDC, &pfd);
  
  if (pixelFormat == 0)
    sys_fatalerror("ChoosePixelFormat failed.");
  if (SetPixelFormat(hDC, pixelFormat, &pfd) != TRUE)
    sys_fatalerror("SetPixelFormat failed.");
    
  if (DescribePixelFormat(hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd) == 0)
    sys_fatalerror("DescribePixelFormat failed.");
}

bool csGraphics2DOpenGL::Open(const char *Title)
{
  DEVMODE dmode;
  LONG ti;

  // create the window.
  DWORD exStyle = 0;
  DWORD style = WINDOW_STYLE;
  if (!FullScreen)
    style |= WS_OVERLAPPEDWINDOW;
  if (FullScreen)
  {
    ChangeDisplaySettings(NULL,0);

    EnumDisplaySettings(NULL, 0, &dmode);

    dmode.dmPelsWidth = Width;
    dmode.dmPelsHeight = Height;
    dmode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

    if((ti = ChangeDisplaySettings(&dmode, CDS_FULLSCREEN)) != DISP_CHANGE_SUCCESSFUL)
    {
      //The cases below need error handling, as they are errors.
      switch(ti)
      {
        case DISP_CHANGE_RESTART:
        //computer must restart for mode to work.
        CsPrintf (CS_MSG_INITIALIZATION, "gl2d error: must restart for display change.\n");
        break;
        case DISP_CHANGE_BADFLAGS:
        //Bad Flag settings
        CsPrintf (CS_MSG_INITIALIZATION, "gl2d error: display change bad flags.\n");
        break;
        case DISP_CHANGE_FAILED:
        //Failure to display
        CsPrintf (CS_MSG_INITIALIZATION, "gl2d error: display change failed.\n");
        break;
        case DISP_CHANGE_NOTUPDATED:
        //No Reg Write Error
        CsPrintf (CS_MSG_INITIALIZATION, "gl2d error: display change could not write registry.\n");
        break;
        default:
        //Unknown Error
        CsPrintf (CS_MSG_INITIALIZATION, "gl2d error: display change gave unknown error.\n");
        break;
      }
    }
  }

  int wwidth,wheight;
  wwidth=Width+2*GetSystemMetrics(SM_CXSIZEFRAME);
  wheight=Height+2*GetSystemMetrics(SM_CYSIZEFRAME)+GetSystemMetrics(SM_CYCAPTION);

  m_hWnd = CreateWindowEx(exStyle, WINDOWCLASSNAME, Title, style,
		(GetSystemMetrics(SM_CXSCREEN)-wwidth)/2,
		(GetSystemMetrics(SM_CYSCREEN)-wheight)/2,
		wwidth, wheight, NULL, NULL, m_hInstance, NULL );
  if( !m_hWnd )
    sys_fatalerror("Cannot create Crystal Space window", GetLastError());

  ShowWindow( m_hWnd, m_nCmdShow );
  UpdateWindow( m_hWnd );
  SetFocus( m_hWnd );


  hDC = GetDC(m_hWnd);
  CalcPixelFormat ();

  hGLRC = wglCreateContext(hDC);
  wglMakeCurrent(hDC, hGLRC);

  if (!csGraphics2DGLCommon::Open(Title))
    return false;

  if (Depth == 8)
    m_bPalettized = true;
  else
    m_bPalettized = false;
  m_bPaletteChanged = false;

  return true;
}

void csGraphics2DOpenGL::Close(void)
{
  if (hGLRC)
  {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hGLRC);
  }
  ReleaseDC(m_hWnd, hDC);

  if (FullScreen) ChangeDisplaySettings(NULL,0); 

  if(!FullScreen)
  {
/*
    // restore the original system palette.
    HDC dc = GetDC(NULL);
    SetSystemPaletteUse(dc, SYSPAL_STATIC);
    PostMessage(HWND_BROADCAST, WM_SYSCOLORCHANGE, 0, 0);
    ReleaseDC(NULL, dc);
*/
  }

  csGraphics2D::Close ();
}

void csGraphics2DOpenGL::Print (csRect* /*area*/)
{
  SwapBuffers(hDC);
  glFlush();
}

HRESULT csGraphics2DOpenGL::SetColorPalette()
{
  HRESULT ret = S_OK;
  
  if ((Depth==8) && m_bPaletteChanged)
  {
    m_bPaletteChanged = false;

   if(!FullScreen)
    {
      HPALETTE oldPal;
      HDC dc = GetDC(NULL);
      
      SetSystemPaletteUse(dc, SYSPAL_NOSTATIC);
      PostMessage(HWND_BROADCAST, WM_SYSCOLORCHANGE, 0, 0);
      
      CreateIdentityPalette(Palette);
      ClearSystemPalette();
      
      oldPal = SelectPalette(dc, hWndPalette, FALSE);
      
      RealizePalette(dc);
      SelectPalette(dc, oldPal, FALSE);
      ReleaseDC(NULL, dc);
    }

    return ret;
  }
  
  return S_OK;
}

void csGraphics2DOpenGL::SetRGB(int i, int r, int g, int b)
{
  csGraphics2D::SetRGB (i, r, g, b);
  m_bPaletteChanged = true;
}

bool csGraphics2DOpenGL::SetMouseCursor (csMouseCursorID iShape)
{
  HCURSOR hCursor;
	
  switch(iShape)
  {
    case csmcNone:     hCursor = NULL; break;
    case csmcArrow:    hCursor = LoadCursor (NULL, IDC_ARROW);    break;
    case csmcMove:     hCursor = LoadCursor (NULL, IDC_SIZEALL);  break;
    case csmcSizeNWSE: hCursor = LoadCursor (NULL, IDC_SIZENWSE); break;
    case csmcSizeNESW: hCursor = LoadCursor (NULL, IDC_SIZENESW); break;
    case csmcSizeNS:   hCursor = LoadCursor (NULL, IDC_SIZENS);   break;
    case csmcSizeEW:   hCursor = LoadCursor (NULL, IDC_SIZEWE);   break;
    case csmcStop:     hCursor = LoadCursor (NULL, IDC_NO);       break;
    case csmcWait:     hCursor = LoadCursor (NULL, IDC_WAIT);     break;
    default: hCursor = NULL;		//return false;
  }
	
	if (hCursor)
	{
		SetCursor(hCursor);
		return true;
	}
	else
	{
		SetCursor(NULL);
		return false;
	}
}

bool csGraphics2DOpenGL::SetMousePosition (int x, int y)
{
  POINT p;
  
  p.x = x;
  p.y = y;

  ClientToScreen(m_hWnd, &p);

  ::SetCursorPos(p.x, p.y);

  return true;
}
