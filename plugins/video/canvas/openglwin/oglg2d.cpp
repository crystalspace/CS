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
#include "cssys/sysfunc.h"
#include <windows.h>
#include <GL/gl.h>

#include "csutil/scf.h"
#include "oglg2d.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include "cssys/win32/win32.h"
#include "iutil/cmdline.h"

CS_IMPLEMENT_PLUGIN

#ifndef CDS_UPDATEREGISTRY
#define CDS_UPDATEREGISTRY  0x00000001
#endif
#ifndef CDS_TEST
#define CDS_TEST            0x00000002
#endif
#ifndef CDS_FULLSCREEN
#define CDS_FULLSCREEN      0x00000004
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

#ifndef ENUM_CURRENT_SETTINGS
#define ENUM_CURRENT_SETTINGS       ((DWORD)-1)
#endif

static void SystemFatalError (char *str, HRESULT hRes = S_OK)
{
  LPVOID lpMsgBuf;
  char* szMsg;
  char szStdMessage[] = "Last Error: ";

  if (FAILED(hRes))
  {
    DWORD dwResult = FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER
    	| FORMAT_MESSAGE_FROM_SYSTEM, NULL, hRes,  MAKELANGID(LANG_NEUTRAL,
      SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL );

    if (dwResult != 0)
    {
      szMsg = new char[strlen((const char*)lpMsgBuf) + strlen(str)
      	+ strlen(szStdMessage) + 1];
      strcpy( szMsg, str );
      strcat( szMsg, szStdMessage );
      strcat( szMsg, (const char*)lpMsgBuf );

      LocalFree( lpMsgBuf );

      MessageBox (NULL, szMsg, "Fatal Error in glwin32.dll", 
	MB_OK | MB_ICONERROR);
      delete szMsg;

      exit(1);
    }
  }

  MessageBox (NULL, str, "Fatal Error in glwin32.dll", 
    MB_OK | MB_ICONERROR);

  exit (1);
}

/////The 2D Graphics Driver//////////////

SCF_IMPLEMENT_IBASE_EXT (csGraphics2DOpenGL)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iOpenGLInterface)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGraphics2DOpenGL::eiOpenGLInterface)
  SCF_IMPLEMENTS_INTERFACE (iOpenGLInterface)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csGraphics2DOpenGL)

SCF_EXPORT_CLASS_TABLE (glwin32)
  SCF_EXPORT_CLASS_DEP (csGraphics2DOpenGL, "crystalspace.graphics2d.glwin32",
    "Win32 OpenGL 2D graphics driver for Crystal Space", "crystalspace.font.server.")
SCF_EXPORT_CLASS_TABLE_END

///// Windowed-mode palette stuff //////

static struct
{
  WORD Version;
  WORD NumberOfEntries;
  PALETTEENTRY aEntries[256];
} SysPalette =
{
  0x300,
    256
};

static HPALETTE hWndPalette = NULL;

static void ClearSystemPalette ()
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

  int c;
  for (c = 0; c < 256; c++)
  {
    Palette.aEntries[c].peRed = 0;
    Palette.aEntries[c].peGreen = 0;
    Palette.aEntries[c].peBlue = 0;
    Palette.aEntries[c].peFlags = PC_NOCOLLAPSE;
  }

  HDC hdc = GetDC (NULL);

  HPALETTE BlackPal, OldPal;
  BlackPal = CreatePalette ((LOGPALETTE *)&Palette);
  OldPal = SelectPalette (hdc,BlackPal,FALSE);
  RealizePalette (hdc);
  SelectPalette (hdc, OldPal, FALSE);
  DeleteObject (BlackPal);

  ReleaseDC (NULL, hdc);
}

static void CreateIdentityPalette (csRGBpixel *p)
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

  if (hWndPalette)
    DeleteObject (hWndPalette);

  Palette.aEntries[0].peFlags = 0;
  Palette.aEntries[0].peFlags = 0;

  int i;
  for (i = 1; i < 255; i++)
  {
    Palette.aEntries[i].peRed = p[i].red;
    Palette.aEntries[i].peGreen = p[i].green;
    Palette.aEntries[i].peBlue = p[i].blue;
    Palette.aEntries[i].peFlags = PC_RESERVED;
  }

  hWndPalette = CreatePalette ((LOGPALETTE *)&Palette);

  if (!hWndPalette)
    SystemFatalError ("Error creating identity palette.");
}

csGraphics2DOpenGL::csGraphics2DOpenGL (iBase *iParent) :
                   csGraphics2DGLCommon (iParent),
                   m_nGraphicsReady (true),
                   m_hWnd (NULL),
                   m_piWin32Assistant (NULL),
                   m_bPalettized (false),
                   m_bPaletteChanged (false)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiOpenGLInterface);
}

csGraphics2DOpenGL::~csGraphics2DOpenGL (void)
{
  if (m_piWin32Assistant)
    m_piWin32Assistant->DecRef ();
  m_nGraphicsReady = 0;
}

void csGraphics2DOpenGL::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  iReporter* rep = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (rep)
  {
    rep->ReportV (severity, "crystalspace.canvas.openglwin", msg, arg);
    rep->DecRef ();
  }
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

bool csGraphics2DOpenGL::Initialize (iObjectRegistry *object_reg)
{
  if (!csGraphics2DGLCommon::Initialize (object_reg))
    return false;

  m_piWin32Assistant = CS_QUERY_REGISTRY (object_reg, iWin32Assistant);
  if (!m_piWin32Assistant)
    SystemFatalError ("csGraphics2DOpenGL::Open(QI) -- system passed does not support iWin32Assistant.");

  // Get the creation parameters
  m_hInstance = m_piWin32Assistant->GetInstance ();
  m_nCmdShow  = m_piWin32Assistant->GetCmdShow ();

  if (Depth == 8)
  {
    pfmt.PalEntries = 256;
    pfmt.PixelBytes = 1;
  }

  iCommandLineParser* cmdline = CS_QUERY_REGISTRY (object_reg, iCommandLineParser);
  m_bHardwareCursor = config->GetBool ("Video.SystemMouseCursor", true);
  if (cmdline->GetOption ("sysmouse")) m_bHardwareCursor = true;
  if (cmdline->GetOption ("nosysmouse")) m_bHardwareCursor = false;
  cmdline->DecRef ();

  m_nDepthBits = config->GetInt ("Video.OpenGL.DepthBits", 32);
  m_nDisplayFrequency = config->GetInt ("Video.DisplayFrequency", 0);

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
      m_nDepthBits,                   /* depth buffer */
      1,                              /* no stencil buffer */
      0,                              /* no auxiliary buffers */
      PFD_MAIN_PLANE,                 /* main layer */
      0,                              /* reserved */
      0, 0, 0                         /* no layer, visible, damage masks */
  };

  int pixelFormat = ChoosePixelFormat (hDC, &pfd);

  if (pixelFormat == 0)
    SystemFatalError ("ChoosePixelFormat failed.");
  if (SetPixelFormat (hDC, pixelFormat, &pfd) != TRUE)
    SystemFatalError ("SetPixelFormat failed.");

  if (DescribePixelFormat (hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd) == 0)
    SystemFatalError ("DescribePixelFormat failed.");

  //Depth = pfd.cColorBits; // @@@ ColorBits are ignored. Will cause corruption
  m_nDepthBits = pfd.cDepthBits;
}

bool csGraphics2DOpenGL::Open ()
{
  if (is_open) return true;
  DWORD exStyle;
  DWORD style;

  // create the window.
  if (FullScreen)
  {
    ChangeDisplaySettings (NULL, 0);

    SwitchDisplayMode ();
  }

  m_bActivated = true;

  if (FullScreen)
  {
    exStyle = 0;/*WS_EX_TOPMOST;*/
    style = WS_POPUP | WS_VISIBLE | WS_SYSMENU;
    m_hWnd = CreateWindowEx (exStyle, CS_WIN32_WINDOW_CLASS_NAME, win_title, style, 0, 0, 
      Width, Height, NULL, NULL, m_hInstance, NULL);
  }
  else
  {
    exStyle = 0;
    style = WS_CAPTION | WS_MINIMIZEBOX | WS_POPUP | WS_SYSMENU;
    int wwidth = Width + 2 * GetSystemMetrics (SM_CXFIXEDFRAME);
    int wheight = Height + 2 * GetSystemMetrics (SM_CYFIXEDFRAME) + GetSystemMetrics (SM_CYCAPTION);
    m_hWnd = CreateWindowEx (exStyle, CS_WIN32_WINDOW_CLASS_NAME, win_title, style,
      (GetSystemMetrics (SM_CXSCREEN) - wwidth) / 2, (GetSystemMetrics (SM_CYSCREEN) - wheight) / 2,
      wwidth, wheight, NULL, NULL, m_hInstance, NULL );
  }

  if (!m_hWnd)
    SystemFatalError ("Cannot create Crystal Space window", GetLastError());

  // Subclass the window
  m_OldWndProc = (WNDPROC)SetWindowLong (m_hWnd, GWL_WNDPROC, (LONG) WindowProc);
  SetWindowLong (m_hWnd, GWL_USERDATA, (LONG)this);

  hDC = GetDC (m_hWnd);
  CalcPixelFormat ();

  Report (CS_REPORTER_SEVERITY_NOTIFY,
    "Using %d bits per pixel (%d color mode), %d bits depth buffer.", 
      Depth, 1 << (Depth == 32 ? 24 : Depth), m_nDepthBits);

  hGLRC = wglCreateContext (hDC);
  wglMakeCurrent (hDC, hGLRC);

  UpdateWindow (m_hWnd);
  ShowWindow (m_hWnd, m_nCmdShow);
  SetForegroundWindow (m_hWnd);
  SetFocus (m_hWnd);

  if (FullScreen)
  {
    /* 
     * from the Windows Shell docs:
     * "It is possible to cover the taskbar by explicitly setting the size 
     * of the window rectangle equal to the size of the screen with 
     * SetWindowPos."
     */
    SetWindowPos (m_hWnd, HWND_TOPMOST, 0, 0, Width, Height, 0);
  }

  if (!csGraphics2DGLCommon::Open ())
    return false;

  if (Depth == 8)
    m_bPalettized = true;
  else
    m_bPalettized = false;
  m_bPaletteChanged = false;

  return true;
}

bool csGraphics2DOpenGL::RestoreDisplayMode ()
{
  if (is_open)
  {
    ChangeDisplaySettings (NULL, 0);
    is_open = false;
    return true;
  }
  return false;
}

void csGraphics2DOpenGL::Close (void)
{
  if (!is_open) return;

  if (hGLRC)
  {
    wglDeleteContext (hGLRC);
    wglMakeCurrent (NULL, NULL);
  }

  ReleaseDC (m_hWnd, hDC);

  RestoreDisplayMode ();

  csGraphics2D::Close ();
}

void csGraphics2DOpenGL::Print (csRect* /*area*/)
{
  glFlush();
  SwapBuffers(hDC);
}

HRESULT csGraphics2DOpenGL::SetColorPalette ()
{
  HRESULT ret = S_OK;

  if ((Depth==8) && m_bPaletteChanged)
  {
    m_bPaletteChanged = false;

    if (!FullScreen)
    {
      HPALETTE oldPal;
      HDC dc = GetDC(NULL);

      SetSystemPaletteUse (dc, SYSPAL_NOSTATIC);
      PostMessage (HWND_BROADCAST, WM_SYSCOLORCHANGE, 0, 0);

      CreateIdentityPalette (Palette);
      ClearSystemPalette ();

      oldPal = SelectPalette (dc, hWndPalette, FALSE);

      RealizePalette (dc);
      SelectPalette (dc, oldPal, FALSE);
      ReleaseDC (NULL, dc);
    }

    return ret;
  }

  return S_OK;
}

void csGraphics2DOpenGL::SetRGB (int i, int r, int g, int b)
{
  csGraphics2D::SetRGB (i, r, g, b);
  m_bPaletteChanged = true;
}

bool csGraphics2DOpenGL::SetMouseCursor (csMouseCursorID iShape)
{
  iWin32Assistant* winhelper = CS_QUERY_REGISTRY (object_reg, iWin32Assistant);
  CS_ASSERT (winhelper != NULL);
  bool rc;
  if (!m_bHardwareCursor) {
    winhelper->SetCursor (csmcNone);
    rc = false;
  } else {
    rc = winhelper->SetCursor (iShape);
  }
  winhelper->DecRef ();
  return rc;
}

bool csGraphics2DOpenGL::SetMousePosition (int x, int y)
{
  POINT p;

  p.x = x;
  p.y = y;

  ClientToScreen (m_hWnd, &p);

  ::SetCursorPos (p.x, p.y);

  return true;
}

bool csGraphics2DOpenGL::PerformExtensionV (char const* command, va_list args)
{
  if (!strcasecmp (command, "configureopengl"))
  {
    // Ugly hack needed to work around an interference between the 3dfx opengl
    // driver on voodoo cards <= 2 and the win32 console window
    if (GetFullScreen() && config->GetBool (
    	"Video.OpenGL.Win32.DisableConsoleWindow", false) )
    {
      m_piWin32Assistant->DisableConsole ();
      Report (CS_REPORTER_SEVERITY_NOTIFY,
      	"*** Disabled Win32 console window to avoid OpenGL interference.");
    }
    csGraphics2DGLCommon::PerformExtensionV (command, args);
    return true;
  }
  return csGraphics2DGLCommon::PerformExtensionV (command, args);
}

void csGraphics2DOpenGL::SetTitle (const char* title)
{
  csGraphics2D::SetTitle (title);
  if (m_hWnd)
    SetWindowText (m_hWnd, title);
}

void csGraphics2DOpenGL::AlertV (int type, const char* title, const char* okMsg,
	const char* msg, va_list arg)
{
  m_piWin32Assistant->AlertV (m_hWnd, type, title, okMsg, msg, arg);
}

LRESULT CALLBACK csGraphics2DOpenGL::WindowProc (HWND hWnd, UINT message,
  WPARAM wParam, LPARAM lParam)
{
  csGraphics2DOpenGL *This = (csGraphics2DOpenGL *)GetWindowLong (hWnd, GWL_USERDATA);
  switch (message)
  {
    case WM_ACTIVATE:
      This->Activate (!(wParam == WA_INACTIVE));
    break;
  }
/*  switch (message)
  {
    case WM_PAINT:
      if (!This->FullScreen || !This->m_bDoubleBuffer)
      {
        RECT rect;
        if (GetUpdateRect (hWnd, &rect, FALSE))
        {
          PAINTSTRUCT ps;
          BeginPaint (hWnd, &ps);
          This->Refresh (rect);
          EndPaint (hWnd, &ps);
          return TRUE;
        }
      }
      break;
    case WM_SYSKEYDOWN:
      // Catch Alt+Enter
      if ((TCHAR)wParam == VK_RETURN)
      {
        This->PerformExtension ("fullscreen", !This->FullScreen);
        return TRUE;
      }
      break;
    case WM_SYSCOMMAND:
      // For some strange reason if we don't intercept this message
      // the system produces an ugly beep when switching from fullscreen
      if (wParam == SC_KEYMENU)
        return TRUE;
      break;
  }*/
  return CallWindowProc (This->m_OldWndProc, hWnd, message, wParam, lParam);
}

void csGraphics2DOpenGL::Activate (bool activated)
{
  if (FullScreen && (activated != m_bActivated))
  {
    m_bActivated = activated;
    if (m_bActivated)
    {
      SwitchDisplayMode ();
      ShowWindow (m_hWnd, SW_SHOWNORMAL);
      SetWindowPos (m_hWnd, HWND_TOPMOST, 0, 0, Width, Height, 0);
    }
    else
    {
      ShowWindow (m_hWnd, SW_SHOWMINIMIZED);
      SetWindowPos (m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE |
        SWP_NOSIZE | SWP_NOACTIVATE);
      ChangeDisplaySettings (NULL, 0);
    }
  }
}

void csGraphics2DOpenGL::SwitchDisplayMode ()
{
  DEVMODE dmode;
  LONG ti;

  EnumDisplaySettings (NULL, ENUM_CURRENT_SETTINGS, &dmode);

  dmode.dmBitsPerPel = Depth;
  dmode.dmPelsWidth = Width;
  dmode.dmPelsHeight = Height;
  if (m_nDisplayFrequency) dmode.dmDisplayFrequency = m_nDisplayFrequency;
  dmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
  
  if ((ti = ChangeDisplaySettings(&dmode, CDS_FULLSCREEN)) != DISP_CHANGE_SUCCESSFUL)
  {
    // maybe just the monitor frequency is not supported.
    // so try without setting it
    dmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
    ti = ChangeDisplaySettings(&dmode, CDS_FULLSCREEN);
  }
  if (ti != DISP_CHANGE_SUCCESSFUL)
  {
    //The cases below need error handling, as they are errors.
    switch (ti)
    {
      case DISP_CHANGE_RESTART:
        //computer must restart for mode to work.
        Report (CS_REPORTER_SEVERITY_WARNING,
          "gl2d error: must restart for display change.");
        break;
      case DISP_CHANGE_BADFLAGS:
        //Bad Flag settings
        Report (CS_REPORTER_SEVERITY_WARNING,
	      "gl2d error: display change bad flags.");
        break;
      case DISP_CHANGE_FAILED:
        //Failure to display
        Report (CS_REPORTER_SEVERITY_WARNING,
          "gl2d error: display change failed.");
        break;
      case DISP_CHANGE_NOTUPDATED:
        //No Reg Write Error
        Report (CS_REPORTER_SEVERITY_WARNING,
          "gl2d error: display change could not write registry.");
        break;
      default:
        //Unknown Error
        Report (CS_REPORTER_SEVERITY_WARNING,
          "gl2d error: display change gave unknown error.");
        break;
    }
  }
}