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
#include "csutil/scf.h"
#include "csutil/csstring.h"
#include "g2d.h"
#include "video/canvas/directxcommon/directdetection.h"
#include "csgeom/csrect.h"
#include "csutil/util.h"
#include "isys/system.h"
#include "isys/event.h"

#ifndef DD_FALSE 
  // This is normally being done in the ddraw.h file
  #define DD_FALSE S_FALSE
#endif

#define WINDOW_STYLE (WS_CAPTION | WS_MINIMIZEBOX | WS_POPUP | WS_SYSMENU)

static DirectDetection DDetection;
static DirectDetectionDevice *DirectDevice;

//--//--//--//--//--//--//--//--//--//--//--//--//-- csGraphics2DDDraw3 --//--//

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csGraphics2DDDraw3)

SCF_EXPORT_CLASS_TABLE (csddraw)
  SCF_EXPORT_CLASS_DEP (csGraphics2DDDraw3, CS_SOFTWARE_2D_DRIVER,
    "Crystal Space 2D DirectDraw driver", "crystalspace.font.server.")
SCF_EXPORT_CLASS_TABLE_END

csGraphics2DDDraw3::csGraphics2DDDraw3(iBase *iParent) :
  csGraphics2D (iParent),
  m_lpDD (NULL),
  m_lpddsPrimary (NULL),
  m_lpddsBack (NULL),
  m_lpddPal (NULL),
  m_hWnd (NULL),
  m_hWndPalette (NULL),
  m_bPalettized (false),
  m_bPaletteChanged (false),
  m_nActivePage (0),
  m_bLocked (false),
  m_bDoubleBuffer (false),
  m_bAllowWindowed (false)
{
  m_hInstance = GetModuleHandle (NULL);
}

csGraphics2DDDraw3::~csGraphics2DDDraw3 ()
{
  Close ();
}

bool csGraphics2DDDraw3::Initialize (iSystem *pSystem)
{
  if (!csGraphics2D::Initialize (pSystem))
    return false;

  return true;
}

bool csGraphics2DDDraw3::Open (const char *Title)
{
  if (!csGraphics2D::Open (Title))
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
  m_hWnd = CreateWindow (WINDOWCLASSNAME, Title, 0,
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

  // Get ahold of the main DirectDraw object...
  DDetection.checkDevices2D ();
  DirectDevice = DDetection.findBestDevice2D ();

  if (DirectDevice == NULL)
  {
    InitFail (DD_FALSE, "Error creating DirectDevice\n");
    return false;
  }

  LPGUID pGuid = NULL;
  if (!DirectDevice->IsPrimary2D)
    pGuid = &DirectDevice->Guid2D;

  System->Printf (CS_MSG_INITIALIZATION, "Using DirectDraw %s (%s)\n",
    DirectDevice->DeviceDescription2D, DirectDevice->DeviceName2D);

  // Create a DD object for either the primary device or the secondary.
  HRESULT ddrval;
  if ((ddrval = DirectDrawCreate (pGuid, &m_lpDD, NULL)) != DD_OK)
  {
    InitFail (ddrval, "DirectDrawCreate FAILED (Code: %08lx)\n");
    return false;
  }

  Memory = NULL;
  m_hWndPalette = NULL;
  // Default to no double buffering since usually this is SLOW
  m_bDoubleBuffer = false;

  if (InitSurfaces () != DD_OK)
    return false;
  
  return true;
}

void csGraphics2DDDraw3::Close ()
{
  ReleaseAllObjects ();

  if (m_lpDD)
  {
    m_lpDD->RestoreDisplayMode ();
    if (m_lpddPal)
    {
      m_lpddPal->Release ();
      m_lpddPal = NULL;
    }
    m_lpDD->Release ();
    m_lpDD = NULL;
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

void csGraphics2DDDraw3::Print (csRect *area)
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
      hRet = m_lpddsPrimary->Flip (NULL, DDFLIP_WAIT);
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

      hRet = m_lpddsPrimary->Blt (&rcScreen, m_lpddsBack,
        area ? &rcSource : NULL, DDBLT_WAIT, NULL);

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
      &rcSource, DDBLT_WAIT, NULL);

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

  return true;
}

void csGraphics2DDDraw3::FinishDraw ()
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

void csGraphics2DDDraw3::SetColorPalette ()
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
    
    ret = m_lpDD->CreatePalette (DDPCAPS_8BIT, (PALETTEENTRY *)Palette, &m_lpddPal, NULL);
    if (ret == DD_OK) m_lpddsPrimary->SetPalette (m_lpddPal);
    
    if (!FullScreen)
    {
      HPALETTE oldPal;
      HDC dc = GetDC (NULL);
      
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
      ReleaseDC (NULL, dc);
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
  bool success;
  if (!System->PerformExtension ("SetCursor", iShape, &success))
    return false;

  return success;
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

bool csGraphics2DDDraw3::PerformExtensionV (char const* command, va_list)
{
  bool rc = true;
  if (!strcmp (command, "fullscreen"))
  {
    bool fs = va_arg (args, bool);
    if (fs != FullScreen
     && (m_bAllowWindowed || fs))
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
      m_lpddsBack = NULL;
    }
    if (m_lpddsPrimary)
    {
      m_lpddsPrimary->Release ();
      m_lpddsPrimary = NULL;
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
  HRESULT hRet;
  DDSURFACEDESC ddsd;
  DDSCAPS ddscaps;
  DDPIXELFORMAT ddpf;

  hRet = m_lpDD->SetCooperativeLevel (m_hWnd, FullScreen ?
    (DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN) : DDSCL_NORMAL);
  if (hRet != DD_OK)
    return InitFail (hRet, "SetCooperativeLevel FAILED (Code: %08lx)\n");

  // Set window style bits
  SetWindowLong (m_hWnd, GWL_STYLE, FullScreen ? 0 : WINDOW_STYLE);
  // Call SetWindowPos so that the change takes effect
  SetWindowPos (m_hWnd, HWND_TOP, 0, 0, 0, 0,
    SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

  if (FullScreen)
  {
    // Set FS video mode
    hRet = m_lpDD->SetDisplayMode (Width, Height, Depth);
    if (hRet != DD_OK)
      return InitFail (hRet, "SetDisplayMode FAILED (Code: %08lx)\n");
  }

  if (!FullScreen || !m_bDoubleBuffer)
  {
    // Create the primary surface
    ZeroMemory (&ddsd,sizeof(ddsd));
    ddsd.dwSize = sizeof (ddsd);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    if ((hRet = m_lpDD->CreateSurface (&ddsd, &m_lpddsPrimary, NULL)) != DD_OK)
      return InitFail (hRet, "Cannot create primary surface for DirectDraw (Code: %08lx)\n");

    // Create the backbuffer. In fullscreen mode by default we don't
    // use the backbuffer, but we use it in single-buffered modes.
    ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
    ddsd.dwWidth = Width;
    ddsd.dwHeight = Height;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    if ((hRet = m_lpDD->CreateSurface (&ddsd, &m_lpddsBack, NULL)) != DD_OK)
    {
      InitFail (hRet, "CreateSurface for backbuffer FAILED (Code: %08lx)\n");
      return false;
    }

    if (!FullScreen)
    {
      // Create a clipper object since this is for a Windowed render
      LPDIRECTDRAWCLIPPER pClipper;
      if ((hRet = m_lpDD->CreateClipper (0, &pClipper, NULL)) != DD_OK)
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

    hRet = m_lpDD->CreateSurface (&ddsd, &m_lpddsPrimary, NULL);
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
  
    // calculate CS's pixel format structure.
    pfmt.PixelBytes = 4;
    pfmt.PalEntries = 0;
  }
  pfmt.complete ();

  m_lpddsBack->GetSurfaceDesc (&ddsd);
  for (int i = 0; i < Height; i++)
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

  // First of all save the contents of backbuffer since it will be cleared
  char *oldBuffer = NULL;
  if (BeginDraw ())
  {
    size_t BytesPerLine = Width * ((Depth + 7) / 8);
    oldBuffer = new char [Height * BytesPerLine];
    for (int i = 0; i < Height; i++)
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
  for (int times = (m_bDoubleBuffer && FullScreen) ? 2 : 1; times; times--)
    if (BeginDraw ())
    {
      size_t BytesPerLine = Width * ((Depth + 7) / 8);
      for (int i = 0; i < Height; i++)
        memcpy (Memory + LineAddress [i], oldBuffer + i * BytesPerLine, BytesPerLine);
      FinishDraw ();
      Print (NULL);
    }
  delete [] oldBuffer;

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
  System->Printf (CS_MSG_FATAL_ERROR, szError, hRet);
  DestroyWindow (m_hWnd);
  return hRet;
}

void csGraphics2DDDraw3::ClearSystemPalette ()
{
  LOGPALETTE Palette = { 0x300,256 };
  HPALETTE BlackPal, OldPal;
  HDC hdc;

  Palette.palPalEntry [0].peFlags = 0;
  Palette.palPalEntry [255].peFlags = 0;

  for (int c = 0; c < 256; c++)
  {
    Palette.palPalEntry [c].peRed = 0;
    Palette.palPalEntry [c].peGreen = 0;
    Palette.palPalEntry [c].peBlue = 0;
    Palette.palPalEntry [c].peFlags = PC_NOCOLLAPSE;
  }

  hdc = GetDC (NULL);

  BlackPal = CreatePalette (&Palette);

  OldPal = SelectPalette (hdc, BlackPal, FALSE);
  RealizePalette (hdc);
  SelectPalette (hdc, OldPal, FALSE);
  DeleteObject (BlackPal);

  ReleaseDC (NULL, hdc);
}

bool csGraphics2DDDraw3::CreateIdentityPalette (csRGBpixel *p)
{
  int i;
  LOGPALETTE Palette = { 0x300, 256 };

  if (m_hWndPalette)
    DeleteObject (m_hWndPalette);

  Palette.palPalEntry [0].peFlags = 0;
  Palette.palPalEntry [255].peFlags = 0;

  for (i = 1; i < 255; i++)
  {
    Palette.palPalEntry [i].peRed = p [i].red;
    Palette.palPalEntry [i].peGreen = p [i].green;
    Palette.palPalEntry [i].peBlue = p [i].blue;
    Palette.palPalEntry [i].peFlags = PC_RESERVED;
  }

  m_hWndPalette = CreatePalette (&Palette);

  if (!m_hWndPalette)
    return false;
  return true;
}

long FAR PASCAL csGraphics2DDDraw3::WindowProc (HWND hWnd, UINT message,
  WPARAM wParam, LPARAM lParam)
{
  csGraphics2DDDraw3 *This = (csGraphics2DDDraw3 *)GetWindowLong (hWnd, GWL_USERDATA);
  switch (message)
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
  }
  return This->m_OldWndProc (hWnd, message, wParam, lParam);
}
