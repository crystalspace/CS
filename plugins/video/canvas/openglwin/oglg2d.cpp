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
#include "csutil/sysfunc.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>

#include "csutil/win32/wintools.h"

#include "csutil/scf.h"
#include "oglg2d.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include "csutil/win32/win32.h"
#include "iutil/cmdline.h"
#include "iutil/eventq.h"

#ifndef GL_VERSION_1_1
#error OpenGL version 1.1 required! Stopping compilation.
#endif

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

// These don't exist on some older SDKs
#ifndef _WIN64
  #ifndef SetWindowLongPtrA
    #define SetWindowLongPtrA SetWindowLongA
  #endif
  #ifndef SetWindowLongPtrW
    #define SetWindowLongPtrW SetWindowLongW
  #endif
  
  #ifndef GetWindowLongPtrA
    #define GetWindowLongPtrA GetWindowLongA
  #endif
  #ifndef GetWindowLongPtrW
    #define GetWindowLongPtrW GetWindowLongW
  #endif
#endif

/*
    in fs mode, the window is topmost, means above every other
    window, all the time. but when debugging a break it is really annoying to 
    have a black window in front of your face instead of the IDE... 
    note: this hack causes taskbar flickering when "always on top"  is enabled
    and auto-hide is disabled.
 */
#ifdef CS_DEBUG
# define CS_WINDOW_Z_ORDER HWND_TOP
#else
# define CS_WINDOW_Z_ORDER HWND_TOPMOST
#endif

static void SystemFatalError (wchar_t* str, HRESULT hRes = ~0)
{
  wchar_t* newMsg = 0;
  wchar_t* szMsg;
  wchar_t szStdMessage[] = L"Last Error: ";

  if (hRes != ~0)
  {
    wchar_t* lpMsgBuf = cswinGetErrorMessageW (hRes);

    szMsg = newMsg = new wchar_t[wcslen (lpMsgBuf) + wcslen (str)
      + wcslen (szStdMessage) + 1];
    wcscpy (szMsg, str);
    wcscat (szMsg, szStdMessage);
    wcscat (szMsg, lpMsgBuf);
  
    delete[] lpMsgBuf ;
  }
  else
    szMsg = str;

  MessageBoxW (0, szMsg, L"Fatal Error in glwin32.dll", 
    MB_OK | MB_ICONERROR);

  delete[] newMsg;

  exit(1);
}

/////The 2D Graphics Driver//////////////

SCF_IMPLEMENT_IBASE_EXT (csGraphics2DOpenGL)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iOpenGLInterface)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGraphics2DOpenGL::eiOpenGLInterface)
  SCF_IMPLEMENTS_INTERFACE (iOpenGLInterface)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csGraphics2DOpenGL)


///// Windowed-mode palette stuff //////

static HPALETTE hWndPalette = 0;

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

  HDC hdc = GetDC (0);

  HPALETTE BlackPal, OldPal;
  BlackPal = CreatePalette ((LOGPALETTE *)&Palette);
  OldPal = SelectPalette (hdc,BlackPal,FALSE);
  RealizePalette (hdc);
  SelectPalette (hdc, OldPal, FALSE);
  DeleteObject (BlackPal);

  ReleaseDC (0, hdc);
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
    SystemFatalError (L"Error creating identity palette.");
}

csGraphics2DOpenGL::csGraphics2DOpenGL (iBase *iParent) :
  csGraphics2DGLCommon (iParent),
  m_nGraphicsReady (true),
  m_hWnd (0),
  m_bPalettized (false),
  m_bPaletteChanged (false),
  modeSwitched (true)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiOpenGLInterface);
}

csGraphics2DOpenGL::~csGraphics2DOpenGL (void)
{
  m_nGraphicsReady = 0;
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiOpenGLInterface);
}

void csGraphics2DOpenGL::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (CS_QUERY_REGISTRY (object_reg, iReporter));
  if (rep)
    rep->ReportV (severity, "crystalspace.canvas.openglwin", msg, arg);
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
    SystemFatalError (L"csGraphics2DOpenGL::Open(QI) -- system passed does not support iWin32Assistant.");

  // Get the creation parameters
  m_hInstance = m_piWin32Assistant->GetInstance ();
  m_nCmdShow  = m_piWin32Assistant->GetCmdShow ();

  if (Depth == 8)
  {
    pfmt.PalEntries = 256;
    pfmt.PixelBytes = 1;
  }

  // Create the event outlet
  csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
  if (q != 0)
    EventOutlet = q->CreateEventOutlet (this);

  csRef<iCommandLineParser> cmdline (
  	CS_QUERY_REGISTRY (object_reg, iCommandLineParser));
  m_bHardwareCursor = config->GetBool ("Video.SystemMouseCursor", true);
  if (cmdline->GetOption ("sysmouse")) m_bHardwareCursor = true;
  if (cmdline->GetOption ("nosysmouse")) m_bHardwareCursor = false;

  // store a copy of the refresh rate as we may need it later
  m_nDisplayFrequency = refreshRate;

  return true;
}

void csGraphics2DOpenGL::CalcPixelFormat (int pixelFormat)
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
#ifndef CS_USE_NEW_RENDERER
      0,                              /* no alpha buffer */
#else
      8,                              /* 8 bit alpha buffer */
#endif // CS_USE_NEW_RENDERER
      0,                              /* alpha bits (ignored) */
      0,                              /* no accumulation buffer */
      0, 0, 0, 0,                     /* accum bits (ignored) */
      depthBits,                   /* depth buffer */
#ifndef CS_USE_NEW_RENDERER
      1,                              /* no stencil buffer */
#else
      8,
#endif // CS_USE_NEW_RENDERER
      0,                              /* no auxiliary buffers */
      PFD_MAIN_PLANE,                 /* main layer */
      0,                              /* reserved */
      0, 0, 0                         /* no layer, visible, damage masks */
  };

  if (pixelFormat <= 0)
  {
    pixelFormat = ChoosePixelFormat (hDC, &pfd);

    if (pixelFormat == 0)
      SystemFatalError (L"ChoosePixelFormat failed.");
  }
  if (SetPixelFormat (hDC, pixelFormat, &pfd) != TRUE)
    SystemFatalError (L"SetPixelFormat failed.");

  if (DescribePixelFormat (hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd) == 0)
    SystemFatalError (L"DescribePixelFormat failed.");

  Depth = pfd.cColorBits; 
  // @@@ ColorBits are ignored. Will cause corruption
  /* [res: what "corruption"? if DescribePixelFormat() doesn't return
   * the correct color depth maybe try EnumDisplaySettings() instead.
   * but somehow the actual color depth should be retrieved. ]
   */
  depthBits = pfd.cDepthBits;

  hardwareAccelerated = !(pfd.dwFlags & PFD_GENERIC_FORMAT);
  if (!hardwareAccelerated)
  {
    Report (CS_REPORTER_SEVERITY_WARNING,
      "No hardware acceleration!");
  }

  pfmt.PixelBytes = (pfd.cColorBits == 32) ? 4 : (pfd.cColorBits + 7) >> 3;
  pfmt.RedBits = pfd.cRedBits;
  pfmt.RedShift = pfd.cRedShift;
  pfmt.RedMask = ((1 << pfd.cRedBits) - 1) << pfd.cRedShift;
  pfmt.GreenBits = pfd.cGreenBits;
  pfmt.GreenShift = pfd.cGreenShift;
  pfmt.GreenMask = ((1 << pfd.cGreenBits) - 1) << pfd.cGreenShift;
  pfmt.BlueBits = pfd.cBlueBits;
  pfmt.BlueShift = pfd.cBlueShift;
  pfmt.BlueMask = ((1 << pfd.cBlueBits) - 1) << pfd.cBlueShift;
  pfmt.PalEntries = 0;
}

struct DummyWndInfo
{
  int* pixelFormat;
  csGraphics2DOpenGL* this_;
  int samples;
};

bool csGraphics2DOpenGL::FindMultisampleFormat (int samples, 
						int& pixelFormat)
{
  if (samples == 0) return false;

  /*
    To use multisampling, a special pixel format has to determined.
    However, this determination works over a WGL ext - thus we need
    a GL context. So we create a window just for checking that
    ext.
   */
  static const char* dummyClassName = "CSGL_DummyWindow";

  HINSTANCE ModuleHandle = GetModuleHandle(0);

  WNDCLASS wc;
  wc.hCursor        = 0;
  wc.hIcon	    = 0;
  wc.lpszMenuName   = 0;
  wc.lpszClassName  = dummyClassName;
  wc.hbrBackground  = (HBRUSH)(COLOR_BTNFACE + 1);
  wc.hInstance      = ModuleHandle;
  wc.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wc.lpfnWndProc    = DummyWindow;
  wc.cbClsExtra     = 0;
  wc.cbWndExtra     = 0;

  if (!RegisterClass (&wc)) return false;

  DummyWndInfo dwi;
  dwi.pixelFormat = &pixelFormat;
  dwi.this_ = this;
  dwi.samples = samples;

  HWND wnd = CreateWindow (dummyClassName, 0, 0, CW_USEDEFAULT, 
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0,
    ModuleHandle, (LPVOID)&dwi);
  DestroyWindow (wnd);

  UnregisterClass (dummyClassName, ModuleHandle);

  return true;
}

LRESULT CALLBACK csGraphics2DOpenGL::DummyWindow (HWND hWnd, UINT message,
  WPARAM wParam, LPARAM lParam)
{
  switch(message)
  {
  case WM_CREATE:
    {
      DummyWndInfo* dwi = (DummyWndInfo*)(LPCREATESTRUCT(lParam)->lpCreateParams);

      dwi->this_->hDC = GetWindowDC (hWnd);
      dwi->this_->CalcPixelFormat (-1);

      int pixelFormat = ::GetPixelFormat (dwi->this_->hDC);
      PIXELFORMATDESCRIPTOR pfd;
      if (DescribePixelFormat (dwi->this_->hDC, pixelFormat, 
	sizeof(PIXELFORMATDESCRIPTOR), &pfd) == 0)
	SystemFatalError (L"DescribePixelFormat failed.");

      dwi->this_->hGLRC = wglCreateContext (dwi->this_->hDC);
      wglMakeCurrent (dwi->this_->hDC, dwi->this_->hGLRC);

      csGLExtensionManager& ext = dwi->this_->ext;
      ext.InitWGL_ARB_pixel_format (dwi->this_->hDC);
      if (ext.CS_WGL_ARB_pixel_format)
      {
	unsigned int numFormats = 0;
	int iAttributes[20];
	float fAttributes[] = {0.0f, 0.0f};

	HDC hDC = GetDC(hWnd);
	iAttributes[0] = WGL_DRAW_TO_WINDOW_ARB;
	iAttributes[1] = GL_TRUE;
	iAttributes[2] = WGL_ACCELERATION_ARB;
	iAttributes[3] = WGL_FULL_ACCELERATION_ARB;
	iAttributes[4] = WGL_COLOR_BITS_ARB;
	iAttributes[5] = pfd.cColorBits;
	iAttributes[6] = WGL_ALPHA_BITS_ARB;
	iAttributes[7] = pfd.cAlphaBits ;
	iAttributes[8] = WGL_DEPTH_BITS_ARB;
	iAttributes[9] = pfd.cDepthBits;
	iAttributes[10] = WGL_STENCIL_BITS_ARB;
	iAttributes[11] = pfd.cStencilBits;
	iAttributes[12] = WGL_DOUBLE_BUFFER_ARB;
	iAttributes[13] = GL_TRUE;
	iAttributes[14] = WGL_SAMPLE_BUFFERS_ARB;
	iAttributes[15] = GL_TRUE;
	iAttributes[16] = WGL_SAMPLES_ARB;
	iAttributes[17] = dwi->samples;
	iAttributes[18] = 0;
	iAttributes[19] = 0;

	if ((ext.wglChoosePixelFormatARB (hDC, iAttributes, fAttributes,
	  1, dwi->pixelFormat, &numFormats) == GL_FALSE) || (numFormats == 0))
	{
	  *dwi->pixelFormat = -1;
	}
      }

      wglMakeCurrent (dwi->this_->hDC, 0);
      wglDeleteContext (dwi->this_->hGLRC);

      ReleaseDC (hWnd, dwi->this_->hDC);
    }
    break;
  }
  return DefWindowProc (hWnd, message, wParam, lParam);
}

bool csGraphics2DOpenGL::Open ()
{
  if (is_open) return true;
  DWORD exStyle;
  DWORD style;

  int pixelFormat = -1;
  if (FindMultisampleFormat (multiSamples, pixelFormat))
  {
    // Reset all extensions. Needed for proper WGL_ext_str functioning -
    // the WGL ext string is only valid for one HDC.
    ext.Reset ();
  }

  // create the window.
  if (FullScreen)
  {
    SwitchDisplayMode (false);
  }

  m_bActivated = true;

  if (FullScreen)
  {
    exStyle = 0;/*WS_EX_TOPMOST;*/
    style = WS_POPUP | WS_VISIBLE | WS_SYSMENU;
    if (cswinIsWinNT ())
    {
      m_hWnd = CreateWindowExW (exStyle, CS_WIN32_WINDOW_CLASS_NAMEW, 0, style, 0, 0, 
	Width, Height, 0, 0, m_hInstance, 0);
    }
    else
    {
      m_hWnd = CreateWindowExA (exStyle, CS_WIN32_WINDOW_CLASS_NAME, 0, style, 0, 0, 
	Width, Height, 0, 0, m_hInstance, 0);
    }
  }
  else
  {
    exStyle = 0;
    style = WS_CAPTION | WS_MINIMIZEBOX | WS_POPUP | WS_SYSMENU;
    if (AllowResizing) 
    {
      style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
    }
    int wwidth = Width + 2 * GetSystemMetrics (SM_CXFIXEDFRAME);
    int wheight = Height + 2 * GetSystemMetrics (SM_CYFIXEDFRAME) + GetSystemMetrics (SM_CYCAPTION);
    if (cswinIsWinNT ())
    {
      m_hWnd = CreateWindowExW (exStyle, CS_WIN32_WINDOW_CLASS_NAMEW, 0, style,
	(GetSystemMetrics (SM_CXSCREEN) - wwidth) / 2, (GetSystemMetrics (SM_CYSCREEN) - wheight) / 2,
	wwidth, wheight, 0, 0, m_hInstance, 0 );
    }
    else
    {
      m_hWnd = CreateWindowExA (exStyle, CS_WIN32_WINDOW_CLASS_NAME, 0, style,
	(GetSystemMetrics (SM_CXSCREEN) - wwidth) / 2, (GetSystemMetrics (SM_CYSCREEN) - wheight) / 2,
	wwidth, wheight, 0, 0, m_hInstance, 0 );
    }
  }

  if (!m_hWnd)
    SystemFatalError (L"Cannot create Crystal Space window", GetLastError());

  SetTitle (win_title);
  
  // Subclass the window
  if (cswinIsWinNT ())
  {
    m_OldWndProc = (WNDPROC)SetWindowLongPtrW (m_hWnd, GWL_WNDPROC, (LONG_PTR) WindowProc);
    SetWindowLongPtrW (m_hWnd, GWL_USERDATA, (LONG_PTR)this);
  }
  else
  {
    m_OldWndProc = (WNDPROC)SetWindowLongPtrA (m_hWnd, GWL_WNDPROC, (LONG_PTR) WindowProc);
    SetWindowLongPtrA (m_hWnd, GWL_USERDATA, (LONG_PTR)this);
  }

  hDC = GetDC (m_hWnd);
  CalcPixelFormat (pixelFormat);

  Report (CS_REPORTER_SEVERITY_NOTIFY,
    "Using %d bits per pixel (%d color mode), %d bits depth buffer.", 
    Depth, 1 << (Depth == 32 ? 24 : Depth), depthBits);

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
    SetWindowPos (m_hWnd, CS_WINDOW_Z_ORDER, 0, 0, Width, Height, 0);
  }

  if (!csGraphics2DGLCommon::Open ())
    return false;

  if (Depth == 8)
    m_bPalettized = true;
  else
    m_bPalettized = false;
  m_bPaletteChanged = false;

  ext.InitWGL_EXT_swap_control (hDC);

  if (ext.CS_WGL_EXT_swap_control)
  {
    ext.wglSwapIntervalEXT (vsync ? 1 : 0);
    vsync = (ext.wglGetSwapIntervalEXT() != 0);
    Report (CS_REPORTER_SEVERITY_NOTIFY,
      "VSync is %s.", 
      vsync ? "enabled" : "disabled");
  }

  return true;
}

bool csGraphics2DOpenGL::RestoreDisplayMode ()
{
  if (is_open)
  {
    if (FullScreen) SwitchDisplayMode (true);
    return true;
  }
  return false;
}

void csGraphics2DOpenGL::Close (void)
{
  if (!is_open) return;

  if (hGLRC)
  {
    wglMakeCurrent (hDC, 0);
    wglDeleteContext (hGLRC);
  }

  DeleteObject (hWndPalette);
  ReleaseDC (m_hWnd, hDC);

  RestoreDisplayMode ();
  csGraphics2DGLCommon::Close ();
}

void csGraphics2DOpenGL::Print (csRect const* /*area*/)
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
      HDC dc = GetDC(0);

      SetSystemPaletteUse (dc, SYSPAL_NOSTATIC);
      PostMessage (HWND_BROADCAST, WM_SYSCOLORCHANGE, 0, 0);

      CreateIdentityPalette (Palette);
      ClearSystemPalette ();

      oldPal = SelectPalette (dc, hWndPalette, FALSE);

      RealizePalette (dc);
      SelectPalette (dc, oldPal, FALSE);
      ReleaseDC (0, dc);
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
  csRef<iWin32Assistant> winhelper (
  	CS_QUERY_REGISTRY (object_reg, iWin32Assistant));
  CS_ASSERT (winhelper != 0);
  bool rc;
  if (!m_bHardwareCursor)
  {
    winhelper->SetCursor (csmcNone);
    rc = false;
  }
  else
  {
    rc = winhelper->SetCursor (iShape);
  }
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
  if (!strcasecmp (command, "hardware_accelerated"))
  {
    bool* hasAccel = (bool*)va_arg (args, bool*);
    *hasAccel = hardwareAccelerated;
    return true;
  }
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
  if (!strcasecmp (command, "getcoords"))
  {
    csRect* r = (csRect*)va_arg (args, csRect*);
    RECT wr;
    GetWindowRect (m_hWnd, &wr);
    r->Set (wr.left, wr.top, wr.right, wr.bottom);
    return true;
  }
  if (!strcasecmp (command, "setcoords"))
  {
    if (!AllowResizing) return false;
    csRect* r = (csRect*)va_arg (args, csRect*);
    SetWindowPos (m_hWnd, 0, r->xmin, r->ymin, r->Width(), r->Height(),
      SWP_NOZORDER | SWP_NOACTIVATE);
    return true;
  }
  if (!strcasecmp (command, "setglcontext"))
  {
    wglMakeCurrent (hDC, hGLRC);
    return true;
  }
  return csGraphics2DGLCommon::PerformExtensionV (command, args);
}

void csGraphics2DOpenGL::SetTitle (const char* title)
{
  csGraphics2D::SetTitle (title);
  if (m_hWnd)
  {
    if (cswinIsWinNT ())
    {
      SetWindowTextW (m_hWnd, csCtoW (title));
    }
    else
    {
      SetWindowTextA (m_hWnd, cswinCtoA (title));
    }
  }
}

void csGraphics2DOpenGL::AlertV (int type, const char* title, const char* okMsg,
	const char* msg, va_list arg)
{
  m_piWin32Assistant->AlertV (m_hWnd, type, title, okMsg, msg, arg);
}

void csGraphics2DOpenGL::AllowResize (bool iAllow)
{
  if (FullScreen)
  {
    return;
  }
  else
  {
    if (AllowResizing != iAllow)
    {
      LONG style = GetWindowLong (m_hWnd,
	GWL_STYLE);
      RECT R;

      GetClientRect (m_hWnd, &R);
      ClientToScreen (m_hWnd, (LPPOINT)&R.left);
      ClientToScreen (m_hWnd, (LPPOINT)&R.right);

      AllowResizing = iAllow;
      if (AllowResizing)
      {
	R.left -= GetSystemMetrics (SM_CXSIZEFRAME);
	R.top -= (GetSystemMetrics (SM_CXSIZEFRAME)
	  + GetSystemMetrics (SM_CYCAPTION));
	R.right += GetSystemMetrics (SM_CXSIZEFRAME);
	R.bottom += GetSystemMetrics (SM_CXSIZEFRAME);

	style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
      }
      else
      {
	R.left -= GetSystemMetrics (SM_CXFIXEDFRAME);
	R.top -= (GetSystemMetrics (SM_CXFIXEDFRAME)
	  + GetSystemMetrics (SM_CYCAPTION));
	R.right += GetSystemMetrics (SM_CXFIXEDFRAME);
	R.bottom += GetSystemMetrics (SM_CXFIXEDFRAME);

	style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
      }
      SetWindowLong (m_hWnd, GWL_STYLE, style);

      SetWindowPos (m_hWnd, 0, R.left, R.top, R.right - R.left,
	R.bottom - R.top, SWP_NOZORDER | SWP_DRAWFRAME);
    }
  }
}

LRESULT CALLBACK csGraphics2DOpenGL::WindowProc (HWND hWnd, UINT message,
  WPARAM wParam, LPARAM lParam)
{
  csGraphics2DOpenGL *This = (csGraphics2DOpenGL *)GetWindowLongPtrA (hWnd, GWL_USERDATA);
  switch (message)
  {
    case WM_ACTIVATE:
      {
	This->Activate (!(wParam == WA_INACTIVE));
	break;
      }
    case WM_SIZE:
      {
      //If the resizing flag is SIZE_MINIMIZED, then we must not set the Height
      //and the Width of this canvas to 1!
      //Instead we have to set an empty clip rect size. Notice that
      //the clipping rectangle is inclusive the top and left edges and
      //exclusive for the right and bottom borders. 
      //So in the SIZE_MINIMZED case the call to Resize must be avoided.
      //Besides we must let the old window procedure be called,
      //which handles the WM_SIZE message as well.
      // Luca (groton@gmx.net)
        if (wParam == SIZE_MINIMIZED)
        {
          This->SetClipRect (0, 0, 1, 1);
        }
        else
        {
	RECT R;
	GetClientRect (hWnd, &R);
	This->Resize (R.right - R.left + 1, R.bottom - R.top + 1);
        }
      }
      break;
  }
  if (IsWindowUnicode (hWnd))
  {
    return CallWindowProcW ((WNDPROC)This->m_OldWndProc, hWnd, message, wParam, lParam);
  }
  else
  {
    return CallWindowProcA ((WNDPROC)This->m_OldWndProc, hWnd, message, wParam, lParam);
  }
}

void csGraphics2DOpenGL::Activate (bool activated)
{
  if (FullScreen && (activated != m_bActivated))
  {
    m_bActivated = activated;
    if (m_bActivated)
    {
      SwitchDisplayMode (false);
      ShowWindow (m_hWnd, SW_SHOWNORMAL);
      SetWindowPos (m_hWnd, CS_WINDOW_Z_ORDER, 0, 0, Width, Height, 0);
    }
    else
    {
      ShowWindow (m_hWnd, SW_SHOWMINIMIZED);
      SetWindowPos (m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE |
        SWP_NOSIZE | SWP_NOACTIVATE);
      SwitchDisplayMode (true);
    }
  }
}

void csGraphics2DOpenGL::SwitchDisplayMode (bool userMode)
{
  DEVMODE curdmode, dmode;

  if (userMode)
  {
    // set the default display mode
    if (modeSwitched)
    {
      ZeroMemory (&dmode, sizeof(dmode));
      curdmode.dmSize = sizeof (dmode);
      curdmode.dmDriverExtra = 0;
      EnumDisplaySettings (0, ENUM_REGISTRY_SETTINGS, &dmode);
      // just do something when the mode was actually switched.
      ChangeDisplaySettings (&dmode, CDS_RESET);
      modeSwitched = false;
    }
  }
  else
  {
    modeSwitched = false;
    // set the user-requested display mode
    ZeroMemory (&curdmode, sizeof(curdmode));
    curdmode.dmSize = sizeof (curdmode);
    curdmode.dmDriverExtra = 0;
    EnumDisplaySettings (0, ENUM_CURRENT_SETTINGS, &curdmode);
    memcpy (&dmode, &curdmode, sizeof (curdmode));

    // check if we already are in the desired display mode
    if (((int)curdmode.dmBitsPerPel == Depth) &&
      ((int)curdmode.dmPelsWidth == Width) &&
      ((int)curdmode.dmPelsHeight == Height) &&
      (!m_nDisplayFrequency || (dmode.dmDisplayFrequency == m_nDisplayFrequency)))
    {
      // no action necessary
      return;
    }
    dmode.dmBitsPerPel = Depth;
    dmode.dmPelsWidth = Width;
    dmode.dmPelsHeight = Height;
    if (m_nDisplayFrequency) dmode.dmDisplayFrequency = m_nDisplayFrequency;
    dmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
    
    LONG ti;
    if ((ti = ChangeDisplaySettings(&dmode, CDS_FULLSCREEN)) != DISP_CHANGE_SUCCESSFUL)
    {
      // maybe just the monitor frequency is not supported.
      // so try without setting it.
      // but first check resolution/depth w/o refresh rate
      if (((int)curdmode.dmBitsPerPel == Depth) &&
        ((int)curdmode.dmPelsWidth == Width) &&
        ((int)curdmode.dmPelsHeight == Height))
      {
	refreshRate = curdmode.dmDisplayFrequency;
        // no action necessary
        return;
      }
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
    else
    {
      modeSwitched = true;
    }
  }

  // retrieve actual refresh rate
  ZeroMemory (&curdmode, sizeof(curdmode));
  curdmode.dmSize = sizeof (curdmode);
  curdmode.dmDriverExtra = 0;
  EnumDisplaySettings (0, ENUM_CURRENT_SETTINGS, &curdmode);
  refreshRate = curdmode.dmDisplayFrequency;
}
