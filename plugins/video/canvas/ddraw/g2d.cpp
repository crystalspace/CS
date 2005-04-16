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
#include "csplugincommon/directx/guids.h"

#include "csutil/sysfunc.h"
#include "csutil/scf.h"
#include "csutil/csstring.h"
#include "csgeom/csrect.h"
#include "csutil/util.h"
#include "iutil/event.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include "csutil/win32/win32.h"
#include "iutil/cmdline.h"

#include "csplugincommon/directx/directdetection.h"

#include "csutil/win32/wintools.h"
#include "csplugincommon/canvas/softfontcache.h"

#include "g2d.h"

#ifndef DD_FALSE
  // This is normally being done in the ddraw.h file
  #define DD_FALSE S_FALSE
#endif

#define WINDOW_STYLE (WS_CAPTION | WS_MINIMIZEBOX | WS_POPUP | WS_SYSMENU)
#define FULLSCREEN_STYLE (WS_POPUP | WS_SYSMENU)

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

//--//--//--//--//--//--//--//--//--//--//--//--//-- csGraphics2DDDraw3 --//--//

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csGraphics2DDDraw3)


csGraphics2DDDraw3::csGraphics2DDDraw3(iBase *iParent) :
  csGraphics2D (iParent),
  m_lpDD (0),
  m_lpddsPrimary (0),
  m_lpddsBack (0),
  m_lpddPal (0),
  m_hWnd (0),
  m_hWndPalette (0),
  m_piWin32Assistant (0),
  m_bPalettized (false),
  m_bPaletteChanged (false),
  m_nActivePage (0),
  m_bLocked (false),
  m_bDoubleBuffer (false),
  m_bAllowWindowed (false)
{
  m_hInstance = GetModuleHandle (0);
}

csGraphics2DDDraw3::~csGraphics2DDDraw3 ()
{
  Close ();
}

void csGraphics2DDDraw3::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (rep)
    rep->ReportV (severity, "crystalspace.canvas.ddraw", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

bool csGraphics2DDDraw3::Initialize (iObjectRegistry *object_reg)
{
  if (!csGraphics2D::Initialize (object_reg))
    return false;

  m_piWin32Assistant = CS_QUERY_REGISTRY (object_reg, iWin32Assistant);
  if (!m_piWin32Assistant)
  {
    MessageBox (0, 
      "csGraphics2DDDraw3::Open(QI) -- system passed does not support iWin32Assistant.",
      0,
      MB_OK | MB_ICONERROR);
    exit(1);
  }

  DDetection.object_reg = object_reg;

  csRef<iCommandLineParser> cmdline = CS_QUERY_REGISTRY (object_reg,
						   iCommandLineParser);
  m_bHardwareCursor = config->GetBool ("Video.SystemMouseCursor", true);
  if (cmdline->GetOption ("sysmouse")) m_bHardwareCursor = true;
  if (cmdline->GetOption ("nosysmouse")) m_bHardwareCursor = false;

  return true;
}

bool csGraphics2DDDraw3::Open ()
{
  if (is_open) return true;
  if (!csGraphics2D::Open ())
    return false;

  // Compute window size/position on desktop
  int wwidth, wheight;
  wwidth = Width + 2 * GetSystemMetrics (SM_CXFIXEDFRAME);
  wheight = Height + 2 * GetSystemMetrics (SM_CYFIXEDFRAME) +
    GetSystemMetrics (SM_CYCAPTION);

  // Save the window size/pos for switching modes
  m_rcWindow.left = (GetSystemMetrics (SM_CXSCREEN) - wwidth) / 2;
  m_rcWindow.top = (GetSystemMetrics (SM_CYSCREEN) - wheight) / 2;
  m_rcWindow.right = m_rcWindow.left + wwidth;
  m_rcWindow.bottom = m_rcWindow.top + wheight;

  // create the window.
  if (cswinIsWinNT ())
  {
    m_hWnd = CreateWindowW (CS_WIN32_WINDOW_CLASS_NAMEW, 0, 0,
      m_rcWindow.left, m_rcWindow.top, m_rcWindow.right - m_rcWindow.left,
      m_rcWindow.bottom - m_rcWindow.top, 0, 0, m_hInstance, 0);
  }
  else
  {
    m_hWnd = CreateWindowA (CS_WIN32_WINDOW_CLASS_NAME, 0, 0,
      m_rcWindow.left, m_rcWindow.top, m_rcWindow.right - m_rcWindow.left,
      m_rcWindow.bottom - m_rcWindow.top, 0, 0, m_hInstance, 0);
  }
  ASSERT (m_hWnd);

  SetTitle (win_title);

  // Subclass the window
  if (cswinIsWinNT ())
  {
    m_OldWndProc = (WNDPROC)GetWindowLongPtrW (m_hWnd, GWL_WNDPROC);
    SetWindowLongPtrW (m_hWnd, GWL_WNDPROC, (LONG_PTR)WindowProc);
    SetWindowLongPtrW (m_hWnd, GWL_USERDATA, (LONG_PTR)this);
  }
  else
  {
    m_OldWndProc = (WNDPROC)GetWindowLongA (m_hWnd, GWL_WNDPROC);
    SetWindowLongPtrA (m_hWnd, GWL_WNDPROC, (LONG_PTR)WindowProc);
    SetWindowLongPtrA (m_hWnd, GWL_USERDATA, (LONG_PTR)this);
  }

  // Get ahold of the main DirectDraw object...
  DDetection.CheckDevices2D ();
  DirectDevice = DDetection.FindBestDevice (DisplayNumber);

  if (DirectDevice == 0)
  {
    InitFail (DD_FALSE, "Error creating DirectDevice\n");
    return false;
  }

  const GUID* pGuid = 0;
  if (!DirectDevice->IsPrimary2D)
    pGuid = &DirectDevice->Guid2D;

  Report (CS_REPORTER_SEVERITY_NOTIFY, "Using DirectDraw %s (%s)",
    DirectDevice->DeviceDescription2D, DirectDevice->DeviceName2D);

  // Create a DD object for either the primary device or the secondary.
  HRESULT ddrval;
  if ((ddrval = DirectDrawCreate ((LPGUID)pGuid, &m_lpDD, 0)) != DD_OK)
  {
    InitFail (ddrval, "DirectDrawCreate FAILED (Code: %08lx)\n");
    return false;
  }

  Memory = 0;
  m_hWndPalette = 0;
  // Default to no double buffering since usually this is SLOW
  m_bDoubleBuffer = false;

  HDC DC = GetDC (0);
  int desktopDepth = GetDeviceCaps (DC, BITSPIXEL);
  ReleaseDC (0, DC);
  if (InitSurfaces () != DD_OK)
    return false;

  // Determine if switching from FS to windowed is allowed
  m_bAllowWindowed = !FullScreen || (desktopDepth == Depth);
  if (FullScreen)
  {
    if (m_bAllowWindowed)
      Report (CS_REPORTER_SEVERITY_NOTIFY, "Windowed mode allowed");
    else
      Report (CS_REPORTER_SEVERITY_NOTIFY, "Windowed mode not allowed");
  }

  return true;
}

void csGraphics2DDDraw3::Close ()
{
  if (!is_open) return;
  ReleaseAllObjects ();

  if (m_lpDD)
  {
    m_lpDD->RestoreDisplayMode ();
    if (m_lpddPal)
    {
      m_lpddPal->Release ();
      m_lpddPal = 0;
    }
    m_lpDD->Release ();
    m_lpDD = 0;
  }

  if (m_hWnd != 0)
    DestroyWindow (m_hWnd);

  if (!FullScreen)
  {
    // restore the original system palette.
    HDC dc = GetDC (0);
    SetSystemPaletteUse (dc, SYSPAL_STATIC);
    PostMessage (HWND_BROADCAST, WM_SYSCOLORCHANGE, 0, 0);
    ReleaseDC (0, dc);
  }

  csGraphics2D::Close ();
}

int csGraphics2DDDraw3::GetPage ()
{
  return m_bDoubleBuffer ? m_nActivePage : 0;
}

bool csGraphics2DDDraw3::DoubleBuffer (bool Enable)
{
  if (FullScreen)
  {
    m_bDoubleBuffer = Enable;
    ChangeCoopLevel ();
    return true;
  }

  return !Enable;
}

bool csGraphics2DDDraw3::GetDoubleBufferState ()
{
  return (FullScreen && m_bDoubleBuffer);
}

void csGraphics2DDDraw3::Print (csRect const* area)
{
  bool loop = true;

  // do we have a primary surface?
  if(!m_lpddsPrimary)
	  return;

  while (loop)
  {
    HRESULT hRet;

    if (FullScreen && m_bDoubleBuffer)
    {
      // We are in fullscreen mode, so perform a flip.
      hRet = m_lpddsPrimary->Flip (0, DDFLIP_WAIT);
      m_nActivePage ^= 1;
    }
    else
    {
      // If we are in windowed mode, perform a blt.
      RECT rcScreen, rcSource;
      if (area)
      {
        rcScreen.left = rcSource.left = area->xmin;
        rcScreen.top = rcSource.top = area->ymin;
        rcScreen.right = rcSource.right = area->xmax;
        rcScreen.bottom = rcSource.bottom = area->ymax;
      }
      else
        GetClientRect (m_hWnd, &rcScreen);

      if (!FullScreen)
      {
        ClientToScreen (m_hWnd, (POINT *)&rcScreen.left);
        ClientToScreen (m_hWnd, (POINT *)&rcScreen.right);
      }

      hdc = 0;
      HPALETTE oldPal = 0;
      if (m_bPalettized)
      {
        hdc = GetDC (m_hWnd);
        oldPal = SelectPalette (hdc, m_hWndPalette, FALSE);
        RealizePalette (hdc);
      }

      hRet = m_lpddsPrimary->Blt (&rcScreen, m_lpddsBack,
        area ? &rcSource : 0, DDBLT_WAIT, 0);

      if (m_bPalettized)
        SelectPalette (hdc, oldPal, FALSE);
    }

    switch (hRet)
    {
      case DDERR_SURFACELOST:
        if (m_lpddsPrimary->Restore () != DD_OK)
          loop = false;
        if (m_lpddsBack
         && m_lpddsBack->IsLost () != DD_OK
         && m_lpddsBack->Restore () != DD_OK)
          loop = false;
        break;
      case DDERR_WASSTILLDRAWING:
        break;
      case DD_OK:
      default:
        loop = false;
        break;
    }
  }
}

void csGraphics2DDDraw3::Refresh (RECT &rect)
{
  if ((FullScreen && m_bDoubleBuffer)
   || (rect.right <= rect.left) || (rect.bottom <= rect.top))
    return;

  if (!m_lpddsPrimary) return;

  bool loop = true;
  while (loop)
  {
    HRESULT hRet;

    RECT rcScreen, rcSource;
    rcScreen.left = rcSource.left = rect.left;
    rcScreen.top = rcSource.top = rect.top;
    rcScreen.right = rcSource.right = rect.right;
    rcScreen.bottom = rcSource.bottom = rect.bottom;

    if (!FullScreen)
    {
      ClientToScreen (m_hWnd, (POINT *)&rcScreen.left);
      ClientToScreen (m_hWnd, (POINT *)&rcScreen.right);
    }

    HDC hdc = 0;
    HPALETTE oldPal = 0;
    if (m_bPalettized)
    {
      hdc = GetDC (m_hWnd);
      oldPal = SelectPalette (hdc, m_hWndPalette, FALSE);
      RealizePalette (hdc);
    }

    hRet = m_lpddsPrimary->Blt (&rcScreen, m_lpddsBack,
      &rcSource, DDBLT_WAIT, 0);

    if (m_bPalettized)
      SelectPalette (hdc, oldPal, FALSE);

    switch (hRet)
    {
      case DDERR_SURFACELOST:
        if (m_lpddsPrimary->Restore () != DD_OK)
          loop = false;
        if (m_lpddsBack
         && m_lpddsBack->IsLost () != DD_OK
         && m_lpddsBack->Restore () != DD_OK)
          loop = false;
        break;
      case DDERR_WASSTILLDRAWING:
        break;
      case DD_OK:
      default:
        loop = false;
        break;
    }
  }
}

bool csGraphics2DDDraw3::BeginDraw ()
{
  csGraphics2D::BeginDraw ();
  if (FrameBufferLocked != 1)
    return true;

  SetColorPalette ();

  DDSURFACEDESC ddsd;
  ddsd.dwSize = sizeof (ddsd);
  ddsd.lpSurface = 0;

  HRESULT ret = DDERR_WASSTILLDRAWING;
  while (ret == DDERR_WASSTILLDRAWING)
    ret = m_lpddsBack->Lock (0, &ddsd, DDLOCK_SURFACEMEMORYPTR, 0);

  if (ret != DD_OK)
  {
    FinishDraw ();
    return false;
  }

  Memory = (unsigned char *)ddsd.lpSurface;
  m_bLocked = true;

  return true;
}

void csGraphics2DDDraw3::FinishDraw ()
{
  csGraphics2D::FinishDraw ();
  if (FrameBufferLocked)
    return;

  if (m_bLocked)
  {
    m_lpddsBack->Unlock (0);
    m_bLocked = false;
    Memory = 0;
  }
}

void csGraphics2DDDraw3::SetColorPalette ()
{
  HRESULT ret;

  if ((Depth == 8) && m_bPaletteChanged)
  {
    m_bPaletteChanged = false;

    if (m_lpddPal)
    {
      m_lpddPal->Release ();
      m_lpddPal = 0;
    }

    ret = m_lpDD->CreatePalette (DDPCAPS_8BIT, (PALETTEENTRY *)Palette, &m_lpddPal, 0);
    if (ret == DD_OK) m_lpddsPrimary->SetPalette (m_lpddPal);

    if (!FullScreen)
    {
      HPALETTE oldPal;
      HDC dc = GetDC (0);

      SetSystemPaletteUse (dc, SYSPAL_NOSTATIC);
      PostMessage (HWND_BROADCAST, WM_SYSCOLORCHANGE, 0, 0);

      if (!CreateIdentityPalette (Palette))
      {
        InitFail (DD_FALSE, "Error creating Identity Palette.\n");
        return;
      }

      ClearSystemPalette ();

      oldPal = SelectPalette (dc, m_hWndPalette, FALSE);

      RealizePalette (dc);
      SelectPalette (dc, oldPal, FALSE);
      ReleaseDC (0, dc);
    }
  }
}

void csGraphics2DDDraw3::SetRGB (int i, int r, int g, int b)
{
  csGraphics2D::SetRGB (i, r, g, b);
  m_bPaletteChanged = true;
}

bool csGraphics2DDDraw3::SetMouseCursor (csMouseCursorID iShape)
{
  bool rc;
  if (!m_bHardwareCursor)
  {
    m_piWin32Assistant->SetCursor (csmcNone);
    rc = false;
  }
  else
  {
    rc = m_piWin32Assistant->SetCursor (iShape);
  }
  return rc;
}

bool csGraphics2DDDraw3::SetMouseCursor (iImage *image, const csRGBcolor* keycolor, 
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

bool csGraphics2DDDraw3::SetMousePosition (int x, int y)
{
  POINT p;

  p.x = x;
  p.y = y;

  ClientToScreen (m_hWnd, &p);
  SetCursorPos (p.x, p.y);

  return true;
}

bool csGraphics2DDDraw3::PerformExtensionV (char const* command, va_list args)
{
  bool rc = true;
  if (!strcmp (command, "fullscreen"))
  {
    bool fs = bool(va_arg (args, int));
    if (fs != FullScreen
     && (m_bAllowWindowed || fs))
    {
      // Save window position
      if (!FullScreen)
        GetWindowRect (m_hWnd, &m_rcWindow);
      FullScreen = fs;
      if (FAILED(ChangeCoopLevel ()))
	Report (CS_REPORTER_SEVERITY_WARNING,
	  "ChangeCoopLevel() failed!");
    } 
    else
    {
      if (!(m_bAllowWindowed || fs))
      {
	Report (CS_REPORTER_SEVERITY_NOTIFY,
	  "Windowed mode not available!");
      }
    }
  }
  else
    rc = false;
  return rc;
}

//-----------------------------------------------------------------------------
// Name: ReleaseAllObjects ()
// Desc: Release all DDraw objects we use
//-----------------------------------------------------------------------------
HRESULT csGraphics2DDDraw3::ReleaseAllObjects ()
{
  if (m_lpDD)
  {
    if (m_lpddsBack)
    {
      m_lpddsBack->Release ();
      m_lpddsBack = 0;
    }
    if (m_lpddsPrimary)
    {
      m_lpddsPrimary->Release ();
      m_lpddsPrimary = 0;
    }
    m_lpDD->SetCooperativeLevel (m_hWnd, DDSCL_NORMAL);
  }
  return DD_OK;
}

//-----------------------------------------------------------------------------
// Name: InitSurfaces()
// Desc: Create all the needed DDraw surfaces and set the coop level
//-----------------------------------------------------------------------------
HRESULT csGraphics2DDDraw3::InitSurfaces ()
{
  HRESULT hRet = 0;
  DDSURFACEDESC ddsd;
  DDSCAPS ddscaps;
  DDPIXELFORMAT ddpf;

  if (FullScreen)
  {
    // Set FS video mode
#if DIRECTDRAW_VERSION >= 0x0700
    // changing the display frequency is a dd7+ feature
    LPDIRECTDRAW7 lpDD7;
    if (m_lpDD->QueryInterface (IID_IDirectDraw7, (LPVOID*)&lpDD7) == S_OK)
    {
      hRet = lpDD7->SetDisplayMode (Width, Height, Depth, refreshRate, 0);
      lpDD7->Release ();
    }
#else
    int lpDD7 = 0;
#endif
    // either we have no dd7 or the modeswitch failed
    if ((!lpDD7) || (hRet != DD_OK))
    {
      // maybe just the monitor frequency is not supported.
      // so try without setting it
      hRet = m_lpDD->SetDisplayMode (Width, Height, Depth);
    }
    if (hRet != DD_OK)
    {
      Report (CS_REPORTER_SEVERITY_WARNING,
        "SetDisplayMode FAILED (Code: %08lx); will try windowed mode", hRet);
      FullScreen = false;
    }
  }

  hRet = m_lpDD->SetCooperativeLevel (m_hWnd, FullScreen ?
    (DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN) : DDSCL_NORMAL);
  if (hRet != DD_OK)
    return InitFail (hRet, "SetCooperativeLevel FAILED (Code: %08lx)\n");

  // Set window style bits
  SetWindowLong (m_hWnd, GWL_STYLE, FullScreen ? FULLSCREEN_STYLE : WINDOW_STYLE);
  // Call SetWindowPos so that the change takes effect
  SetWindowPos (m_hWnd, HWND_TOP, 0, 0, 0, 0,
    SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

  if (!FullScreen || !m_bDoubleBuffer)
  {
    // Create the primary surface
    ZeroMemory (&ddsd,sizeof(ddsd));
    ddsd.dwSize = sizeof (ddsd);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    if ((hRet = m_lpDD->CreateSurface (&ddsd, &m_lpddsPrimary, 0)) != DD_OK)
      return InitFail (hRet, "Cannot create primary surface for DirectDraw (Code: %08lx)\n");

    // Create the backbuffer. In fullscreen mode by default we don't
    // use the backbuffer, but we use it in single-buffered modes.
    ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
    ddsd.dwWidth = Width;
    ddsd.dwHeight = Height;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    if ((hRet = m_lpDD->CreateSurface (&ddsd, &m_lpddsBack, 0)) != DD_OK)
    {
      InitFail (hRet, "CreateSurface for backbuffer FAILED (Code: %08lx)\n");
      return false;
    }

    if (!FullScreen)
    {
      // Create a clipper object since this is for a Windowed render
      LPDIRECTDRAWCLIPPER pClipper;
      if ((hRet = m_lpDD->CreateClipper (0, &pClipper, 0)) != DD_OK)
        return InitFail (hRet, "CreateClipper FAILED (Code: %08lx)\n");

      // Associate the clipper with the window
      pClipper->SetHWnd (0, m_hWnd);
      m_lpddsPrimary->SetClipper (pClipper);
      pClipper->Release ();
    }
  }
  else
  {
    // Create the primary surface with 1 back buffer
    ZeroMemory (&ddsd, sizeof (ddsd));
    ddsd.dwSize = sizeof (ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
    ddsd.dwBackBufferCount = 1;

    hRet = m_lpDD->CreateSurface (&ddsd, &m_lpddsPrimary, 0);
    if (hRet != DD_OK)
      return InitFail (hRet, "Cannot create primary surface for DirectDraw FAILED (Code: %08lx)\n");

    ZeroMemory (&ddscaps, sizeof (ddscaps));
    ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
    hRet = m_lpddsPrimary->GetAttachedSurface (&ddscaps, &m_lpddsBack);
    if (hRet != DD_OK)
      return InitFail (hRet, "GetAttachedSurface FAILED (Code: %08lx)\n");
  }

  // get the pixel format
  memset (&ddpf, 0, sizeof (ddpf));
  ddpf.dwSize = sizeof (ddpf);
  hRet = m_lpddsPrimary->GetPixelFormat (&ddpf);
  if (hRet != DD_OK)
    return InitFail (hRet, "Can't get pixel format (Code: %08lx)\n");

  pfmt.RedMask = ddpf.dwRBitMask;
  pfmt.GreenMask = ddpf.dwGBitMask;
  pfmt.BlueMask = ddpf.dwBBitMask;
  pfmt.AlphaMask = ddpf.dwRGBAlphaBitMask;
  Depth = ddpf.dwRGBBitCount;

  if (Depth == 8)
  {
    pfmt.PalEntries = 256;
    pfmt.PixelBytes = 1;
  }
  else if (Depth == 16)
  {
    _DrawPixel = DrawPixel16;
    _GetPixelAt = GetPixelAt16;

    // Set pixel format
    pfmt.PixelBytes = 2;
    pfmt.PalEntries = 0;
  }
  else if (Depth == 32)
  {
    _DrawPixel = DrawPixel32;
    _GetPixelAt = GetPixelAt32;

    // calculate CS's pixel format structure.
    pfmt.PixelBytes = 4;
    pfmt.PalEntries = 0;
  }
  pfmt.complete ();

  if (fontCache) delete fontCache; fontCache = 0;
  CreateDefaultFontCache ();
  fontCache->SetClipRect (ClipX1, ClipY1, ClipX2, ClipY2);

  m_lpddsBack->GetSurfaceDesc (&ddsd);
  int i;
  for (i = 0; i < Height; i++)
    LineAddress [i] = i * ddsd.lPitch;

  m_bPalettized = (Depth == 8);
  m_bPaletteChanged = false;

  return DD_OK;
}

//-----------------------------------------------------------------------------
// Name: ChangeCoopLevel()
// Desc: Called when the user wants to toggle between Full-Screen & Windowed
//-----------------------------------------------------------------------------
HRESULT csGraphics2DDDraw3::ChangeCoopLevel ()
{
  HRESULT hRet;
  int i;

  // First of all save the contents of backbuffer since it will be cleared
  char *oldBuffer = 0;
  if (BeginDraw ())
  {
    size_t BytesPerLine = Width * ((Depth + 7) / 8);
    oldBuffer = new char [Height * BytesPerLine];
    for (i = 0; i < Height; i++)
      memcpy (oldBuffer + i * BytesPerLine, Memory + LineAddress [i], BytesPerLine);
    FinishDraw ();
  }

  // Release all objects that need to be re-created for the new device
  if (FAILED (hRet = ReleaseAllObjects ()))
    return InitFail (hRet, "ReleaseAllObjects FAILED (Code: %08lx)\n");

  // In case we're coming from a fullscreen mode, restore the window size
  if (!FullScreen)
  {
    m_lpDD->RestoreDisplayMode ();
    SetWindowPos (m_hWnd, HWND_NOTOPMOST, m_rcWindow.left, m_rcWindow.top,
      (m_rcWindow.right - m_rcWindow.left),
      (m_rcWindow.bottom - m_rcWindow.top), SWP_SHOWWINDOW);
  }

  // Re-create the surfaces
  hRet = InitSurfaces ();

  // Now restore the contents of backbuffer
  if (oldBuffer)
  {
    int times;
    for (times = (m_bDoubleBuffer && FullScreen) ? 2 : 1; times; times--)
      if (BeginDraw ())
      {
	size_t BytesPerLine = Width * ((Depth + 7) / 8);
	for (i = 0; i < Height; i++)
	  memcpy (Memory + LineAddress [i], oldBuffer + i * BytesPerLine, BytesPerLine);
	FinishDraw ();
	Print (0);
      }
    delete [] oldBuffer;
  }

  return hRet;
}

//-----------------------------------------------------------------------------
// Name: InitFail()
// Desc: This function is called if an initialization function fails
//-----------------------------------------------------------------------------
HRESULT csGraphics2DDDraw3::InitFail (HRESULT hRet, LPCTSTR szError)
{
  ReleaseAllObjects ();
  if (m_lpDD)
    m_lpDD->RestoreDisplayMode ();
  Report (CS_REPORTER_SEVERITY_ERROR, szError, hRet);
  DestroyWindow (m_hWnd);
  return hRet;
}

void csGraphics2DDDraw3::ClearSystemPalette ()
{
  LOGPALETTE *Palette;
  HPALETTE BlackPal, OldPal;
  HDC hdc;

  Palette = (LOGPALETTE*)malloc(sizeof(LOGPALETTE)+sizeof(PALETTEENTRY)*256);

  Palette->palNumEntries = 256;
  Palette->palVersion = 0x300;

  Palette->palPalEntry [0].peRed = 0;
  Palette->palPalEntry [0].peGreen = 0;
  Palette->palPalEntry [0].peBlue = 0;
  Palette->palPalEntry [0].peFlags = 0;
  Palette->palPalEntry [255].peRed = 255;
  Palette->palPalEntry [255].peGreen = 255;
  Palette->palPalEntry [255].peBlue = 255;
  Palette->palPalEntry [255].peFlags = 0;

  int c;
  for (c = 1; c < 255; c++)
  {
    Palette->palPalEntry [c].peRed = 0;
    Palette->palPalEntry [c].peGreen = 0;
    Palette->palPalEntry [c].peBlue = 0;
    Palette->palPalEntry [c].peFlags = 0;
  }

  hdc = GetDC (0);

  BlackPal = CreatePalette (Palette);
  free((void*)Palette);

  OldPal = SelectPalette (hdc, BlackPal, FALSE);
  RealizePalette (hdc);
  SelectPalette (hdc, OldPal, FALSE);
  DeleteObject (BlackPal);

  ReleaseDC (0, hdc);
}

bool csGraphics2DDDraw3::CreateIdentityPalette (csRGBpixel *p)
{
  int i;
  LOGPALETTE *Palette;

  if (m_hWndPalette)
    DeleteObject (m_hWndPalette);

  Palette = (LOGPALETTE*)malloc(sizeof(LOGPALETTE)+sizeof(PALETTEENTRY)*256);

  Palette->palNumEntries = 256;
  Palette->palVersion = 0x300;

  Palette->palPalEntry [0].peRed = 0;
  Palette->palPalEntry [0].peGreen = 0;
  Palette->palPalEntry [0].peBlue = 0;
  Palette->palPalEntry [0].peFlags = 0;
  Palette->palPalEntry [255].peRed = 255;
  Palette->palPalEntry [255].peGreen = 255;
  Palette->palPalEntry [255].peBlue = 255;
  Palette->palPalEntry [255].peFlags = 0;

  for (i = 1; i < 255; i++)
  {
    Palette->palPalEntry [i].peRed = p [i].red;
    Palette->palPalEntry [i].peGreen = p [i].green;
    Palette->palPalEntry [i].peBlue = p [i].blue;
    Palette->palPalEntry [i].peFlags = PC_RESERVED | PC_NOCOLLAPSE;
  }

  m_hWndPalette = CreatePalette (Palette);

  free((void*)Palette);

  if (!m_hWndPalette)
    return false;
  return true;
}

LRESULT CALLBACK csGraphics2DDDraw3::WindowProc (HWND hWnd, UINT message,
  WPARAM wParam, LPARAM lParam)
{
  csGraphics2DDDraw3 *This = (csGraphics2DDDraw3 *)GetWindowLongPtrA (hWnd, GWL_USERDATA);
  switch (message)
  {
    case WM_PAINT:
      if (!This->FullScreen || !This->m_bDoubleBuffer)
      {
        PAINTSTRUCT ps;
        BeginPaint (hWnd, &ps);
	This->Refresh (ps.rcPaint);
        EndPaint (hWnd, &ps);
        return TRUE;
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
    case WM_DESTROY:
      This->m_hWnd = 0;
      break;
  }
  if (cswinIsWinNT ())
  {
    return CallWindowProcW ((WNDPROC)This->m_OldWndProc, hWnd, message, wParam, lParam);
  }
  else
  {
    return CallWindowProcA ((WNDPROC)This->m_OldWndProc, hWnd, message, wParam, lParam);
  }
}

void csGraphics2DDDraw3::SetTitle (const char* title)
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

void csGraphics2DDDraw3::AlertV (int type, const char* title, 
    const char* okMsg, const char* msg, va_list args)
{
  if (FullScreen)
  {
    m_lpDD->SetCooperativeLevel (m_hWnd, DDSCL_NORMAL);  
  }
  m_piWin32Assistant->AlertV (m_hWnd, type, title, okMsg, msg, args);
  if (FullScreen)
  {
    ChangeCoopLevel();
  }
}
