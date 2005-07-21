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


#include "csutil/win32/wintools.h"
#include "csutil/win32/win32.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>

#include "csutil/scf.h"
#include "oglg2d.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"

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
    In fs mode, the window is topmost, means above every other
    window, all the time. But when debugging it is really annoying to 
    have a black window in front of your face instead of the IDE... 
    Note: this hack may cause taskbar flickering when "always on top" is
    enabled and auto-hide is disabled for the task bar.
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
  wchar_t szStdMessage[] = L"\nLast Error: ";

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
  } Palette;

  Palette.Version = 0x300;
  Palette.nEntries = 256;

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
  } Palette;

  Palette.Version = 0x300;
  Palette.nEntries = 256;

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

int csGraphics2DOpenGL::FindPixelFormatGDI (HDC hDC, 
					    csGLPixelFormatPicker& picker)
{
  int newPixelFormat = 0;
  GLPixelFormat& format = currentFormat;
  while (picker.GetNextFormat (format))
  {
    PIXELFORMATDESCRIPTOR pfd = {
	sizeof(PIXELFORMATDESCRIPTOR),  /* size */
	1,                              /* version */
	PFD_SUPPORT_OPENGL |
	PFD_DOUBLEBUFFER |              /* support double-buffering */
	PFD_DRAW_TO_WINDOW,
	PFD_TYPE_RGBA,                  /* color type */
	format[glpfvColorBits],         /* prefered color depth */
	0, 0, 0, 0, 0, 0,               /* color bits (ignored) */
	format[glpfvAlphaBits],         /* no alpha buffer */
	0,                              /* alpha bits (ignored) */
	format[glpfvAccumColorBits],	/* accumulation buffer */
	0, 0, 0,			/* accum bits (ignored) */
	format[glpfvAccumAlphaBits],	/* accum alpha bits */
	format[glpfvDepthBits],         /* depth buffer */
	format[glpfvStencilBits],	/* stencil buffer */
	0,                              /* no auxiliary buffers */
	PFD_MAIN_PLANE,                 /* main layer */
	0,                              /* reserved */
	0, 0, 0                         /* no layer, visible, damage masks */
    };

    newPixelFormat = ChoosePixelFormat (hDC, &pfd);

    if (newPixelFormat == 0)
      SystemFatalError (L"ChoosePixelFormat failed.", GetLastError());

    if (DescribePixelFormat (hDC, newPixelFormat, 
      sizeof(PIXELFORMATDESCRIPTOR), &pfd) == 0)
      SystemFatalError (L"DescribePixelFormat failed.", GetLastError());

    if (!(pfd.dwFlags & PFD_GENERIC_FORMAT) ||
      (pfd.dwFlags & PFD_GENERIC_ACCELERATED))
    {
      return newPixelFormat;
    }
  }

  return newPixelFormat;
}

int csGraphics2DOpenGL::FindPixelFormatWGL (csGLPixelFormatPicker& picker)
{
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
  dwi.pixelFormat = -1;
  dwi.this_ = this;
  dwi.chosenFormat = &currentFormat;
  dwi.picker = &picker;

  HWND wnd = CreateWindow (dummyClassName, 0, 0, CW_USEDEFAULT, 
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0,
    ModuleHandle, (LPVOID)&dwi);
  DestroyWindow (wnd);

  UnregisterClass (dummyClassName, ModuleHandle);

  ext.Reset();

  return dwi.pixelFormat;
}

LRESULT CALLBACK csGraphics2DOpenGL::DummyWindow (HWND hWnd, UINT message,
  WPARAM wParam, LPARAM lParam)
{
  switch(message)
  {
  case WM_CREATE:
    {
      DummyWndInfo* dwi = (DummyWndInfo*)(LPCREATESTRUCT(lParam)->lpCreateParams);

      HDC hDC = GetDC (hWnd);
      int acceleration = dwi->this_->config->GetBool (
	"Video.OpenGL.FullAcceleration", true) ? WGL_FULL_ACCELERATION_ARB : 
	WGL_GENERIC_ACCELERATION_ARB;
      csGLPixelFormatPicker& picker = *dwi->picker;

      int pixelFormat = dwi->this_->FindPixelFormatGDI (hDC, picker);
      PIXELFORMATDESCRIPTOR pfd;
      if (DescribePixelFormat (hDC, pixelFormat, 
	sizeof(PIXELFORMATDESCRIPTOR), &pfd) == 0)
	SystemFatalError (L"DescribePixelFormat failed.");

      if (SetPixelFormat (hDC, pixelFormat, 0) != TRUE)
      {
	HRESULT spfErr = (HRESULT)GetLastError();
	SystemFatalError (L"SetPixelFormat failed.", spfErr);
      }

      HGLRC hGLRC = wglCreateContext (hDC);
      wglMakeCurrent (hDC, hGLRC);

      csGLExtensionManager& ext = dwi->this_->ext;
      ext.Open();

      dwi->this_->detector.DoDetection (hWnd, hDC);
      dwi->this_->driverdb.Open (dwi->this_, "preinit");

      ext.InitWGL_ARB_pixel_format (hDC);
      if (ext.CS_WGL_ARB_pixel_format)
      {
	unsigned int numFormats = 0;
	int iAttributes[26];
	float fAttributes[] = {0.0f, 0.0f};

	GLPixelFormat format;
	memcpy (format, dwi->chosenFormat, sizeof (GLPixelFormat));
	do
	{
	  int index = 0;
	  iAttributes[index++] = WGL_DRAW_TO_WINDOW_ARB;
	  iAttributes[index++] = GL_TRUE;
	  iAttributes[index++] = WGL_SUPPORT_OPENGL_ARB;
	  iAttributes[index++] = GL_TRUE;
	  iAttributes[index++] = WGL_ACCELERATION_ARB;
	  iAttributes[index++] = acceleration;
	  iAttributes[index++] = WGL_DOUBLE_BUFFER_ARB;
	  iAttributes[index++] = GL_TRUE;
	  iAttributes[index++] = WGL_SAMPLE_BUFFERS_ARB;
	  iAttributes[index++] = 
	    (format[glpfvMultiSamples] != 0) ? GL_TRUE : GL_FALSE;
	  iAttributes[index++] = WGL_SAMPLES_ARB;
	  iAttributes[index++] = format[glpfvMultiSamples];
	  iAttributes[index++] = WGL_COLOR_BITS_ARB;
	  iAttributes[index++] = pfd.cColorBits;
  	  iAttributes[index++] = WGL_ALPHA_BITS_ARB;
  	  iAttributes[index++] = pfd.cAlphaBits;
  	  iAttributes[index++] = WGL_DEPTH_BITS_ARB;
	  iAttributes[index++] = pfd.cDepthBits;
	  iAttributes[index++] = WGL_STENCIL_BITS_ARB;
	  iAttributes[index++] = pfd.cStencilBits;	  
	  iAttributes[index++] = WGL_ACCUM_BITS_ARB;
	  iAttributes[index++] = pfd.cAccumBits;
	  iAttributes[index++] = WGL_ACCUM_ALPHA_BITS_ARB;
	  iAttributes[index++] = pfd.cAccumAlphaBits;
	  iAttributes[index++] = 0;
	  iAttributes[index++] = 0;

	  if ((ext.wglChoosePixelFormatARB (hDC, iAttributes, fAttributes,
	    1, &dwi->pixelFormat, &numFormats) == GL_TRUE) && (numFormats != 0))
	  {
	    int queriedAttrs[] = {WGL_SAMPLES_ARB};
	    const int queriedAttrNum = sizeof(queriedAttrs) / sizeof(int);
	    int attrValues[queriedAttrNum];

	    if (ext.wglGetPixelFormatAttribivARB (hDC, dwi->pixelFormat, 0,
	      queriedAttrNum, queriedAttrs, attrValues) == GL_TRUE)
	    {
	      (*dwi->chosenFormat)[glpfvMultiSamples] = attrValues[0];
	      break;
	    }
	  }
	}
	while (picker.GetNextFormat (format));
      }

      dwi->this_->driverdb.Close ();

      wglMakeCurrent (hDC, 0);
      wglDeleteContext (hGLRC);

      ReleaseDC (hWnd, hDC);
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

  csRef<iVerbosityManager> verbosemgr (
    CS_QUERY_REGISTRY (object_reg, iVerbosityManager));
  if (verbosemgr) 
    detector.SetVerbose (verbosemgr->Enabled ("renderer.windows.gldriver"));
  
  // create the window.
  if (FullScreen)
  {
    SwitchDisplayMode (false);
  }

  int pixelFormat = -1;
  csGLPixelFormatPicker picker (this);
  /*
    Check if the WGL pixel format check should be used at all.
    It appears that some drivers take "odd" choices when using the WGL
    pixel format path (e.g. returning Accum-capable formats even if none
    was requested).
   */
  bool doWGLcheck = false;
  {
    GLPixelFormat format;
    if (picker.GetNextFormat (format))
    {
      doWGLcheck = (format[glpfvMultiSamples] != 0);
      picker.Reset ();
    }
  }
  if (doWGLcheck)
    pixelFormat = FindPixelFormatWGL (picker);

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
  if (pixelFormat == -1)
  {
    picker.Reset();
    pixelFormat = FindPixelFormatGDI (hDC, picker);
  }

  PIXELFORMATDESCRIPTOR pfd;
  if (DescribePixelFormat (hDC, pixelFormat, 
    sizeof(PIXELFORMATDESCRIPTOR), &pfd) == 0)
    SystemFatalError (L"DescribePixelFormat failed.", GetLastError());

  if (SetPixelFormat (hDC, pixelFormat, &pfd) != TRUE)
  {
    HRESULT spfErr = (HRESULT)GetLastError();
    SystemFatalError (L"SetPixelFormat failed.", spfErr);
  }

  currentFormat[glpfvColorBits] = pfd.cColorBits;
  currentFormat[glpfvAlphaBits] = pfd.cAlphaBits;
  currentFormat[glpfvDepthBits] = pfd.cDepthBits;
  currentFormat[glpfvStencilBits] = pfd.cStencilBits;
  currentFormat[glpfvAccumColorBits] = pfd.cAccumBits;
  currentFormat[glpfvAccumAlphaBits] = pfd.cAccumAlphaBits;

  Depth = pfd.cColorBits; 

  hardwareAccelerated = !(pfd.dwFlags & PFD_GENERIC_FORMAT) ||
    (pfd.dwFlags & PFD_GENERIC_ACCELERATED);

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
  pfmt.AlphaBits = pfd.cAlphaBits;
  pfmt.AlphaShift = pfd.cAlphaShift;
  pfmt.AlphaMask = ((1 << pfd.cAlphaBits) - 1) << pfd.cAlphaShift;
  pfmt.PalEntries = 0;

  hGLRC = wglCreateContext (hDC);
  wglMakeCurrent (hDC, hGLRC);

  UpdateWindow (m_hWnd);
  ShowWindow (m_hWnd, m_nCmdShow);
  SetForegroundWindow (m_hWnd);
  SetFocus (m_hWnd);
  
  /* Small hack to emit "no HW acceleration" message on both GDI Generic and
   * sucky Direct3D default OpenGL */
  hardwareAccelerated &= 
    (strncmp ((char*)glGetString (GL_VENDOR), "Microsoft", 9) != 0);
  if (!hardwareAccelerated)
  {
    Report (CS_REPORTER_SEVERITY_WARNING,
      "No hardware acceleration!");
  }

  detector.DoDetection (m_hWnd, hDC);
  Report (CS_REPORTER_SEVERITY_NOTIFY,
    "GL driver: %s %s", detector.GetDriverDLL(), 
    detector.GetDriverVersion() ? detector.GetDriverVersion() : 
      "<version unknown>");

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

const char* csGraphics2DOpenGL::GetRendererString (const char* str)
{
  if (strcmp (str, "win32_driver") == 0)
  {
    return detector.GetDriverDLL();
  }
  else if (strcmp (str, "win32_driverversion") == 0)
  {
    return detector.GetDriverVersion();
  }
  else
    return csGraphics2DGLCommon::GetRendererString (str);
}

const char* csGraphics2DOpenGL::GetVersionString (const char* ver)
{
  if (strcmp (ver, "win32_driver") == 0)
  {
    return detector.GetDriverVersion();
  }
  else
    return csGraphics2DGLCommon::GetVersionString (ver);
}

void csGraphics2DOpenGL::Close (void)
{
  if (!is_open) return;
  
  csGraphics2DGLCommon::Close ();

  if (hGLRC)
  {
    wglMakeCurrent (hDC, 0);
    wglDeleteContext (hGLRC);
  }

  DeleteObject (hWndPalette);
  ReleaseDC (m_hWnd, hDC);

  if (m_hWnd != 0)
    DestroyWindow (m_hWnd);

  RestoreDisplayMode ();
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

bool csGraphics2DOpenGL::SetMouseCursor (iImage *image, const csRGBcolor* keycolor, 
					 int hotspot_x, int hotspot_y,
					 csRGBcolor fg, csRGBcolor bg)
{
  if (!m_bHardwareCursor)
  {
    m_piWin32Assistant->SetCursor (csmcNone);
    return false;
  }
  HCURSOR cur = cursors.GetMouseCursor (image, keycolor, hotspot_x, 
    hotspot_y, fg, bg);
  if (cur == 0)
  {
    m_piWin32Assistant->SetCursor (csmcNone);
    return false;
  }
  return m_piWin32Assistant->SetHCursor (cur);
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

bool csGraphics2DOpenGL::Resize (int width, int height)
{
  if (!csGraphics2DGLCommon::Resize (width, height)) return false;

  if (is_open && !FullScreen)
  {
    RECT R;
    GetClientRect (m_hWnd, &R);
    if (R.right - R.left != Width || R.bottom - R.top != Height)
    {
      // We only resize the window when the canvas is different. This only
      // happens on a manual Resize() from the app, as this is called after
      // the window resizing is finished     
      int wwidth = Width + 2 * GetSystemMetrics (SM_CXSIZEFRAME);
      int wheight = Height + 2 * GetSystemMetrics (SM_CYSIZEFRAME)
        + GetSystemMetrics (SM_CYCAPTION);

      // To prevent a second resize of the canvas, we temporarily disable resizing
      AllowResizing = false;

      SetWindowPos (m_hWnd, 0, (GetSystemMetrics (SM_CXSCREEN) - wwidth) / 2,
        (GetSystemMetrics (SM_CYSCREEN) - wheight) / 2, wwidth, wheight, SWP_NOZORDER);
      
      // Reset. AllowResizing must be true in order to reach this point anyway
      AllowResizing = true;
    }
  }
  return true;
}

void csGraphics2DOpenGL::SetFullScreen (bool b)
{
  if (FullScreen == b) return;
  FullScreen = b;

  if (is_open)
  {
    // Now actually change the window/display settings
    DWORD style;
    if (FullScreen)
    {
      SwitchDisplayMode (false);
      style = WS_POPUP | WS_VISIBLE | WS_SYSMENU;
      SetWindowLong (m_hWnd, GWL_STYLE, style);
      SetWindowPos (m_hWnd, CS_WINDOW_Z_ORDER, 0, 0, Width, Height, SWP_FRAMECHANGED);
      ShowWindow (m_hWnd, SW_SHOW);
    }
    else
    {
      SwitchDisplayMode (true);
      style = WS_CAPTION | WS_MINIMIZEBOX | WS_POPUP | WS_SYSMENU;
      int wwidth, wheight;
      if (AllowResizing)
      {
        style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
        wwidth = Width + 2 * GetSystemMetrics (SM_CXSIZEFRAME);
        wheight = Height + 2 * GetSystemMetrics (SM_CYSIZEFRAME)
          + GetSystemMetrics (SM_CYCAPTION);
      }
      else
      {
        wwidth = Width + 2 * GetSystemMetrics (SM_CXFIXEDFRAME);
        wheight = Height + 2 * GetSystemMetrics (SM_CYFIXEDFRAME)
          + GetSystemMetrics (SM_CYCAPTION);
      }
      SetWindowLong (m_hWnd, GWL_STYLE, style);
      SetWindowPos (m_hWnd, HWND_NOTOPMOST, (GetSystemMetrics (SM_CXSCREEN) - wwidth) / 2,
        (GetSystemMetrics (SM_CYSCREEN) - wheight) / 2, wwidth, wheight, SWP_FRAMECHANGED);
      ShowWindow (m_hWnd, SW_SHOW);
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
	/*
	 * If the resizing flag is SIZE_MINIMIZED, then we must not change the Height
	 * or Width of this canvas!
	 * So in the SIZE_MINIMZED case the call to Resize must be avoided.
	 * Besides we must let the old window procedure be called,
	 * which handles the WM_SIZE message as well.
	 * - Luca (groton@gmx.net)
	 */
        if (wParam != SIZE_MINIMIZED)
        {
	  RECT R;
	  GetClientRect (hWnd, &R);
	  This->Resize (R.right - R.left, R.bottom - R.top);
        }
      }
      break;
    case WM_DESTROY:
      This->m_hWnd = 0;
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
      ShowWindow (m_hWnd, SW_RESTORE);
      SetWindowPos (m_hWnd, CS_WINDOW_Z_ORDER, 0, 0, Width, Height, 0);
      wglMakeCurrent (hDC, hGLRC);
    }
    else
    {
      wglMakeCurrent (0, 0);
      SetWindowPos (m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE |
        SWP_NOSIZE | SWP_NOACTIVATE);
      ShowWindow (m_hWnd, SW_MINIMIZE);
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
