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
#include "video/canvas/directxcommon/directdetection.h"
#include "csgeom/csrect.h"
#include "g2d_dd8.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include "cssys/win32/win32.h"

#ifndef DD_FALSE
  #define DD_FALSE S_FALSE
#endif

#define WINDOW_STYLE (WS_CAPTION | WS_MINIMIZEBOX | WS_POPUP | WS_SYSMENU)

static DirectDetection DDetection;
static DirectDetectionDevice *DirectDevice;

CS_IMPLEMENT_PLATFORM_PLUGIN

SCF_IMPLEMENT_FACTORY (csGraphics2DDDraw8)

SCF_EXPORT_CLASS_TABLE (ddraw8)
  SCF_EXPORT_CLASS_DEP (csGraphics2DDDraw8, "crystalspace.graphics2d.direct3d.dx8",
    "DirectDraw DX8 2D graphics driver for Crystal Space", "crystalspace.font.server.")
SCF_EXPORT_CLASS_TABLE_END

SCF_IMPLEMENT_IBASE_EXT (csGraphics2DDDraw8)
  SCF_IMPLEMENTS_INTERFACE (iGraphics2DDDraw8)
SCF_IMPLEMENT_IBASE_EXT_END

csGraphics2DDDraw8::csGraphics2DDDraw8 (iBase *iParent) :
  csGraphics2D (iParent),
  m_lpDD7 (NULL),
  m_lpddsPrimary (NULL),
  m_lpddsBack (NULL),
  m_lpddsBackLeft (NULL),
  m_lpddClipper (NULL),
  m_lpddPal (NULL),
  m_hWnd (NULL),
  m_hWndPalette (NULL),
  m_bUses3D (false),
  m_bPalettized (false),
  m_bPaletteChanged (false),
  m_nActivePage (0),
  m_bLocked (false),
  m_bDoubleBuffer (false),
  m_bAllowWindowed (false)
{
  m_hInstance = GetModuleHandle (NULL);
  D3DCallback = NULL;
}

csGraphics2DDDraw8::~csGraphics2DDDraw8 ()
{
  Close ();
}

bool csGraphics2DDDraw8::Initialize(iObjectRegistry *object_reg)
{
  // Call original Initialize() function
  if (!csGraphics2D::Initialize(object_reg))
    return false;

  return true;
}

void csGraphics2DDDraw8::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  iReporter* rep = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (rep)
  {
    rep->ReportV (severity, "crystalspace.canvas.ddraw8", msg, arg);
    rep->DecRef ();
  }
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

bool csGraphics2DDDraw8::Open ()
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

  // Create the window.
  m_hWnd = CreateWindow (CS_WIN32_WINDOW_CLASS_NAME, win_title, 0,
    m_rcWindow.left, m_rcWindow.top, m_rcWindow.right - m_rcWindow.left,
    m_rcWindow.bottom - m_rcWindow.top, NULL, NULL, m_hInstance, NULL);
  ASSERT (m_hWnd);

  // Subclass the window
  m_OldWndProc = (WNDPROC)GetWindowLong (m_hWnd, GWL_WNDPROC);
  SetWindowLong (m_hWnd, GWL_WNDPROC, (LONG)&WindowProc);
  SetWindowLong (m_hWnd, GWL_USERDATA, (LONG)this);

  // Decide whenever we allow windowed mode at all
  HDC hdc = GetDC (m_hWnd);
  m_bAllowWindowed = (GetDeviceCaps (hdc, BITSPIXEL) == Depth);
  ReleaseDC (m_hWnd, hdc);
  if (m_bAllowWindowed)
    FullScreen = false;

  // Create the DirectDraw device //
  if (!m_bUses3D)
  {
    DDetection.checkDevices2D ();
    DirectDevice = DDetection.findBestDevice2D ();
  }
  else
  {
    DDetection.checkDevices3D ();
    DirectDevice = DDetection.findBestDevice3D (FullScreen);
  }

  if (DirectDevice == NULL)
  {
    InitFail (DD_FALSE, "Error creating DirectDevice\n");
    return false;
  }

  LPGUID pGuid = NULL;
  if (!DirectDevice->IsPrimary2D)
    pGuid = &DirectDevice->Guid2D;

  Report (CS_REPORTER_SEVERITY_NOTIFY, "Using DirectDraw %s (%s)",
    DirectDevice->DeviceDescription2D, DirectDevice->DeviceName2D);

  // Create a DD object for either the primary device or the secondary.
  HRESULT ddrval;
  LPDIRECTDRAW lpDD;
  if ((ddrval = DirectDrawCreate (pGuid, &lpDD, NULL)) != DD_OK)
  {
    InitFail (ddrval, "DirectDrawCreate FAILED (Code: %08lx)\n");
    return false;
  }

  // here can be set the new DDSCL_FPUSETUP setting to speed up rendering
  if ((ddrval = lpDD->QueryInterface (IID_IDirectDraw7, (LPVOID *)&m_lpDD7)) != DD_OK)
  {
    InitFail (ddrval, "Cannot get DirectDraw7 interface (Code: %08lx)\n");
    return false;
  }
  // We don't need the raw DirectDraw object anymore ...
  lpDD->Release ();

  Memory = NULL;
  m_hWndPalette = NULL;

  if (InitSurfaces () != DD_OK)
    return false;

  return true;
}

void csGraphics2DDDraw8::Close ()
{
  if (!is_open) return;
  ReleaseAllObjects ();

  if (m_lpDD7)
  {
    m_lpDD7->RestoreDisplayMode ();
    if (m_lpddPal)
    {
      m_lpddPal->Release ();
      m_lpddPal = NULL;
    }
    m_lpDD7->Release ();
    m_lpDD7 = NULL;
  }

  if (!FullScreen)
  {
    // restore the original system palette.
    HDC dc = GetDC (NULL);
    SetSystemPaletteUse (dc, SYSPAL_STATIC);
    PostMessage (HWND_BROADCAST, WM_SYSCOLORCHANGE, 0, 0);
    ReleaseDC (NULL, dc);
  }

  csGraphics2D::Close ();
}

int csGraphics2DDDraw8::GetPage ()
{
  return m_bDoubleBuffer ? m_nActivePage : 0;
}

bool csGraphics2DDDraw8::DoubleBuffer (bool Enable)
{
  if (FullScreen)
  {
    m_bDoubleBuffer = Enable;
    ChangeCoopLevel ();
    return true;
  }

  return !Enable;
}

bool csGraphics2DDDraw8::GetDoubleBufferState ()
{
  return (FullScreen && m_bDoubleBuffer);
}

bool csGraphics2DDDraw8::BeginDraw ()
{
  csGraphics2D::BeginDraw ();
  if (FrameBufferLocked != 1)
    return true;

  SetColorPalette ();

  DDSURFACEDESC2 ddsd;
  ddsd.dwSize = sizeof (ddsd);
  ddsd.lpSurface = NULL;

  HRESULT ret = DDERR_WASSTILLDRAWING;
  while (ret == DDERR_WASSTILLDRAWING)
    ret = m_lpddsBack->Lock (NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR, NULL);

  if (ret != DD_OK)
  {
    FinishDraw ();
    return false;
  }

  Memory = (unsigned char *)ddsd.lpSurface;
  m_bLocked = true;

  return (Memory != NULL);
}

void csGraphics2DDDraw8::FinishDraw ()
{
  csGraphics2D::FinishDraw ();
  if (FrameBufferLocked)
    return;

  if (m_bLocked)
  {
    m_lpddsBack->Unlock (NULL);
    m_bLocked = false;
    Memory = NULL;
  }
}

void csGraphics2DDDraw8::Print (csRect* area)
{
  bool loop = true;
  while (loop)
  {
    HRESULT ddrval;

    if (FullScreen && m_bDoubleBuffer)
    {
      ddrval = m_lpddsPrimary->Flip (NULL, 0);
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

      HDC hdc = 0;
      HPALETTE oldPal = 0;
      if (m_bPalettized)
      {
        hdc = GetDC (m_hWnd);
        oldPal = SelectPalette (hdc, m_hWndPalette, FALSE);
        RealizePalette (hdc);
      }

      ddrval = m_lpddsPrimary->Blt (&rcScreen, m_lpddsBack,
        area ? &rcSource : NULL, DDBLT_WAIT, NULL);

      if (m_bPalettized)
        SelectPalette (hdc, oldPal, FALSE);
    }

    switch (ddrval)
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

void csGraphics2DDDraw8::Refresh (RECT &rect)
{
  if ((FullScreen && m_bDoubleBuffer)
   || (rect.right <= rect.left) || (rect.bottom <= rect.top))
    return;

  bool loop = true;
  while (loop)
  {
    HRESULT ddrval;

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

    ddrval = m_lpddsPrimary->Blt (&rcScreen, m_lpddsBack,
      &rcSource, DDBLT_WAIT, NULL);

    if (m_bPalettized)
      SelectPalette (hdc, oldPal, FALSE);

    switch (ddrval)
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

void csGraphics2DDDraw8::SetColorPalette ()
{
  HRESULT ret;

  if ((Depth == 8) && m_bPaletteChanged)
  {
    m_bPaletteChanged = false;

    if (m_lpddPal)
    {
      m_lpddPal->Release ();
      m_lpddPal = NULL;
    }

    ret = m_lpDD7->CreatePalette (DDPCAPS_8BIT, (PALETTEENTRY *)Palette, &m_lpddPal, NULL);
    if (ret == DD_OK) m_lpddsPrimary->SetPalette (m_lpddPal);

    if (!FullScreen)
    {
      HPALETTE oldPal;
      HDC dc = GetDC(NULL);

      SetSystemPaletteUse (dc, SYSPAL_NOSTATIC);
      PostMessage (HWND_BROADCAST, WM_SYSCOLORCHANGE, 0, 0);

      CreateIdentityPalette (Palette);
      ClearSystemPalette ();

      oldPal = SelectPalette (dc, m_hWndPalette, FALSE);

      RealizePalette (dc);
      SelectPalette (dc, oldPal, FALSE);
      ReleaseDC (NULL, dc);
    }
  }
}

void csGraphics2DDDraw8::SetRGB (int i, int r, int g, int b)
{
  csGraphics2D::SetRGB (i, r, g, b);
  m_bPaletteChanged = true;
}

bool csGraphics2DDDraw8::SetMouseCursor (csMouseCursorID iShape)
{
  iWin32Assistant* winhelper = CS_QUERY_REGISTRY (object_reg, iWin32Assistant);
  CS_ASSERT (winhelper != NULL);
  bool rc = winhelper->SetCursor (iShape);
  winhelper->DecRef ();
  return rc;
}

bool csGraphics2DDDraw8::SetMousePosition (int x, int y)
{
  POINT p;

  p.x = x;
  p.y = y;

  ClientToScreen (m_hWnd, &p);
  SetCursorPos (p.x, p.y);

  return true;
}

bool csGraphics2DDDraw8::PerformExtensionV (char const* command, va_list args)
{
  bool rc = true;
  if (!strcmp (command, "fullscreen"))
  {
    bool fs = va_arg (args, bool);
    if (fs != FullScreen && (m_bAllowWindowed || fs))
    {
      // Save window position
      if (!FullScreen)
        GetWindowRect (m_hWnd, &m_rcWindow);
      FullScreen = fs;
      ChangeCoopLevel ();
    }
  }
  else
    rc = false;
  return rc;
}

HRESULT csGraphics2DDDraw8::ReleaseAllObjects ()
{
  if (m_lpDD7)
  {
    if (m_lpddsBack)
    {
      m_lpddsBack->Release ();
      m_lpddsBack = NULL;
    }
    if (m_lpddsPrimary)
    {
      m_lpddsPrimary->Release ();
      m_lpddsPrimary = NULL;
    }
    if (m_lpddClipper)
    {
      m_lpddClipper->Release ();
      m_lpddClipper = NULL;
    }
    m_lpDD7->SetCooperativeLevel (m_hWnd, DDSCL_NORMAL);
  }
  return DD_OK;
}

HRESULT csGraphics2DDDraw8::InitSurfaces ()
{
  HRESULT ddrval;
  DDSURFACEDESC2 ddsd;
  DDSCAPS2 ddscaps;
  DDPIXELFORMAT ddpf;

  ddrval = m_lpDD7->SetCooperativeLevel (m_hWnd, FullScreen ?
    (DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN) : DDSCL_NORMAL);
  if (ddrval != DD_OK)
    return InitFail (ddrval, "SetCooperativeLevel FAILED (Code: %08lx)\n");

  // Set window style bits
  SetWindowLong (m_hWnd, GWL_STYLE, FullScreen ? 0 : WINDOW_STYLE);
  // Call SetWindowPos so that the change takes effect
  SetWindowPos (m_hWnd, HWND_TOP, 0, 0, 0, 0,
    SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

  if (FullScreen)
  {
    // Set FS video mode
    ddrval = m_lpDD7->SetDisplayMode (Width, Height, Depth, 0, 0);
    if (ddrval != DD_OK)
      return InitFail (ddrval, "SetDisplayMode FAILED (Code: %08lx)\n");
  }

  if (!FullScreen || !m_bDoubleBuffer)
  {
    // Create the primary surface
    ZeroMemory (&ddsd, sizeof (ddsd));
    ddsd.dwSize = sizeof (ddsd);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    if ((ddrval = m_lpDD7->CreateSurface (&ddsd, &m_lpddsPrimary, NULL)) != DD_OK)
      return InitFail (ddrval, "Cannot create primary surface for DirectDraw (Code: %08lx)\n");

    // Create the backbuffer. In fullscreen mode by default we don't
    // use the backbuffer, but we use it in single-buffered modes.
    ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
    ddsd.dwWidth = Width;
    ddsd.dwHeight = Height;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

    if (DirectDevice->Only2D)
      ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
    else
      ddsd.ddsCaps.dwCaps |= DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;

    if ((ddrval = m_lpDD7->CreateSurface (&ddsd, &m_lpddsBack, NULL)) != DD_OK)
    {
      InitFail (ddrval, "CreateSurface for backbuffer FAILED (Code: %08lx)\n");
      return false;
    }

    if (!FullScreen)
    {
      // Create a clipper object since this is for a Windowed render
      if ((ddrval = DirectDrawCreateClipper (0, &m_lpddClipper, NULL)) != DD_OK)
        return InitFail (ddrval, "CreateClipper FAILED (Code: %08lx)\n");

      // Associate the clipper with the window
      m_lpddClipper->SetHWnd (0, m_hWnd);
      m_lpddsPrimary->SetClipper (m_lpddClipper);
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

    // set flags if this is a 3d device
    if (!DirectDevice->Only2D)
      ddsd.ddsCaps.dwCaps |= DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;

    ddrval = m_lpDD7->CreateSurface (&ddsd, &m_lpddsPrimary, NULL);
    if (ddrval != DD_OK)
      return InitFail (ddrval, "Cannot create primary surface for DirectDraw (Code: %08lx)\n");

    ZeroMemory (&ddscaps, sizeof (ddscaps));
    ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
    ddrval = m_lpddsPrimary->GetAttachedSurface (&ddscaps, &m_lpddsBack);
    if (ddrval != DD_OK)
      return InitFail (ddrval, "GetAttachedSurface FAILED (Code: %08lx)\n");
  }

  // get the pixel format
  memset (&ddpf, 0, sizeof (ddpf));
  ddpf.dwSize = sizeof (ddpf);
  ddrval = m_lpddsPrimary->GetPixelFormat (&ddpf);
  if (ddrval != DD_OK)
    return InitFail (ddrval, "Can't get pixel format (Code: %08lx)\n");

  pfmt.RedMask = ddpf.dwRBitMask;
  pfmt.GreenMask = ddpf.dwGBitMask;
  pfmt.BlueMask = ddpf.dwBBitMask;
  Depth = ddpf.dwRGBBitCount;

  if (Depth == 8)
  {
    pfmt.PalEntries = 256;
    pfmt.PixelBytes = 1;
  }
  else if (Depth == 16)
  {
    _DrawPixel = DrawPixel16;
    _WriteString = WriteString16;
    _GetPixelAt = GetPixelAt16;

    // Set pixel format
    pfmt.PixelBytes = 2;
    pfmt.PalEntries = 0;
  }
  else if (Depth == 32)
  {
    _DrawPixel = DrawPixel32;
    _WriteString = WriteString32;
    _GetPixelAt = GetPixelAt32;

    pfmt.PixelBytes = 4;
    pfmt.PalEntries = 0;
  }
  pfmt.complete ();

  m_lpddsBack->GetSurfaceDesc (&ddsd);

  int i;
  for (i = 0; i < Height; i++)
    LineAddress [i] = i * ddsd.lPitch;

  m_bPalettized = (Depth == 8);
  m_bPaletteChanged = false;

  return DD_OK;
}

HRESULT csGraphics2DDDraw8::ChangeCoopLevel ()
{
  HRESULT hRet;

  char *oldBuffer = NULL;
  int i;

  if (BeginDraw ())
  {
    size_t BytesPerLine = Width * ((Depth + 7) / 8);
    oldBuffer = new char [Height * BytesPerLine];
    for (i = 0; i < Height; i++)
      memcpy (oldBuffer + i * BytesPerLine, Memory + LineAddress [i], BytesPerLine);
    FinishDraw ();
  }

  if (FAILED (hRet = ReleaseAllObjects ()))
    return InitFail (hRet, "ReleaseAllObjects FAILED (Code: %08lx)\n");

  if (!FullScreen)
  {
    m_lpDD7->RestoreDisplayMode ();
    SetWindowPos (m_hWnd, HWND_NOTOPMOST, m_rcWindow.left, m_rcWindow.top,
      (m_rcWindow.right - m_rcWindow.left),
      (m_rcWindow.bottom - m_rcWindow.top), SWP_SHOWWINDOW);
  }

  hRet = InitSurfaces ();

  int times;
  for (times = (m_bDoubleBuffer && FullScreen) ? 2 : 1; times; times--)
    if (BeginDraw ())
    {
      size_t BytesPerLine = Width * ((Depth + 7) / 8);

      for (i = 0; i < Height; i++)
        memcpy (Memory + LineAddress [i], oldBuffer + i * BytesPerLine, BytesPerLine);
      FinishDraw ();
      Print (NULL);
    }
  delete [] oldBuffer;

  if (D3DCallback)
    D3DCallback (D3DCallbackData);

  return hRet;
}

HRESULT csGraphics2DDDraw8::InitFail (HRESULT ddrval, LPCTSTR szError)
{
  ReleaseAllObjects ();

  if (m_lpDD7)
    m_lpDD7->RestoreDisplayMode ();

  Report (CS_REPORTER_SEVERITY_ERROR, szError, ddrval);
  DestroyWindow (m_hWnd);

  return ddrval;
}

void csGraphics2DDDraw8::ClearSystemPalette ()
{
  LOGPALETTE Palette = { 0x300,256 };
  HPALETTE BlackPal, OldPal;
  HDC hdc;

  Palette.palPalEntry [0].peFlags = 0;
  Palette.palPalEntry [255].peFlags = 0;

	int i;
  for (i = 0; i < 256; i++)
  {
    Palette.palPalEntry [i].peRed = 0;
    Palette.palPalEntry [i].peGreen = 0;
    Palette.palPalEntry [i].peBlue = 0;
    Palette.palPalEntry [i].peFlags = PC_NOCOLLAPSE;
  }

  hdc = GetDC (NULL);

  BlackPal = CreatePalette (&Palette);

  OldPal = SelectPalette (hdc, BlackPal, FALSE);
  RealizePalette (hdc);
  SelectPalette (hdc, OldPal, FALSE);
  DeleteObject (BlackPal);

  ReleaseDC (NULL, hdc);
}

bool csGraphics2DDDraw8::CreateIdentityPalette (csRGBpixel *p)
{
  LOGPALETTE Palette = { 0x300, 256 };

  if (m_hWndPalette)
    DeleteObject (m_hWndPalette);

  Palette.palPalEntry [0].peFlags = 0;
  Palette.palPalEntry [255].peFlags = 0;

	int i;
  for (i = 1; i < 255; i++)
  {
    Palette.palPalEntry [i].peRed = p [i].red;
    Palette.palPalEntry [i].peGreen = p [i].green;
    Palette.palPalEntry [i].peBlue = p [i].blue;
    Palette.palPalEntry [i].peFlags = PC_RESERVED;
  }

  m_hWndPalette = CreatePalette (&Palette);

  if (! m_hWndPalette)
    return false;

  return true;
}

long FAR PASCAL csGraphics2DDDraw8::WindowProc (HWND hWnd, UINT message,
  WPARAM wParam, LPARAM lParam)
{
  csGraphics2DDDraw8 *This = (csGraphics2DDDraw8 *)GetWindowLong (hWnd, GWL_USERDATA);

  switch (message)
  {
    case WM_PAINT:
      if (! This->FullScreen || !This->m_bDoubleBuffer)
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
      if ((TCHAR)wParam == VK_RETURN)
      {
        This->PerformExtension ("fullscreen", ! This->FullScreen);
        return TRUE;
      }
      break;
    case WM_SYSCOMMAND:
      if (wParam == SC_KEYMENU)
        return TRUE;
      break;
  }
  return This->m_OldWndProc (hWnd, message, wParam, lParam);
}

//
// Interface
//

void csGraphics2DDDraw8::GetDirectDrawDriver (LPDIRECTDRAW7* lplpDirectDraw)
{
  *lplpDirectDraw = m_lpDD7;
}

void csGraphics2DDDraw8::GetDirectDrawPrimary (LPDIRECTDRAWSURFACE7* lplpDirectDrawPrimary)
{
  *lplpDirectDrawPrimary = m_lpddsPrimary;
}

void csGraphics2DDDraw8::GetDirectDrawBackBuffer (LPDIRECTDRAWSURFACE7* lplpDirectDrawBackBuffer)
{
  *lplpDirectDrawBackBuffer = m_lpddsBack;
}

extern DirectDetectionDevice* DirectDevice;

void csGraphics2DDDraw8::GetDirectDetection (IDirectDetectionInternal** lplpDDetection)
{
  *lplpDDetection = STATIC_CAST(IDirectDetectionInternal*, DirectDevice);
}

void csGraphics2DDDraw8::SetFor3D(bool For3D)
{
  m_bUses3D = For3D;
}

void csGraphics2DDDraw8::SetModeSwitchCallback (void (*Callback) (void *), void *Data)
{
  D3DCallback = Callback;
  D3DCallbackData = Data;
}
