/*
    DOS Allegro support for Crystal Space 3D library
    Copyright (C) 1999 by Dan Bogdanov <dan@pshg.edu.ee>

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

#include <stdarg.h>
#include "sysdef.h"
#include "cs2d/dosalleg/allegro.h"
#include "cssys/djgpp/idjgpp.h"
#include "csgeom/csrect.h"
#include "isystem.h"
#include "allegro.h"
BITMAP *_cs_alleg2d;

//--------------------------------------------------------- Static COM stuff ---

static DllRegisterData gRegData =
{
  &CLSID_DosRawGraphics2D,
  "crystalspace.graphics2d.allegro",
  "Crystal Space 2D graphics driver for DOS/DJGPP using Allegro video library"
};

void Alleg2DRegister ()
{
  static csGraphics2DFactoryDOSAlleg gG2DAllegFactory;
  gRegData.pClass = &gG2DAllegFactory;
  csRegisterServer (&gRegData);
}

void Alleg2DUnregister ()
{
  csUnregisterServer (&gRegData);
}

//------------------------------------------------ csGraphics2DFactoryDOSAlleg ---

IMPLEMENT_UNKNOWN_NODELETE (csGraphics2DFactoryDOSAlleg)

BEGIN_INTERFACE_TABLE (csGraphics2DFactoryDOSAlleg)
  IMPLEMENTS_INTERFACE (IGraphics2DFactory)
END_INTERFACE_TABLE ()

STDMETHODIMP csGraphics2DFactoryDOSAlleg::CreateInstance (REFIID riid,
  ISystem* piSystem, void**ppv)
{
  if (!piSystem)
  {
    *ppv = 0;
    return E_INVALIDARG;
  }

  CHK (csGraphics2DDOSAlleg *pNew = new csGraphics2DDOSAlleg (piSystem));
  if (!pNew)
  {
    *ppv = 0;
    return E_OUTOFMEMORY;
  }

  return pNew->QueryInterface (riid, ppv);
}

STDMETHODIMP csGraphics2DFactoryDOSAlleg::LockServer (COMBOOL bLock)
{
  (void)bLock;
  return S_OK;
}

//------------------------------------------------------- csGraphics2DDOSAlleg ---

BEGIN_INTERFACE_TABLE (csGraphics2DDOSAlleg)
  IMPLEMENTS_COMPOSITE_INTERFACE_EX (IGraphics2D, XGraphics2D)
  IMPLEMENTS_COMPOSITE_INTERFACE_EX (IGraphicsInfo, XGraphicsInfo)
END_INTERFACE_TABLE ()

IMPLEMENT_UNKNOWN_NODELETE (csGraphics2DDOSAlleg)

ISystem* csGraphics2DDOSAlleg::System = NULL;
IDosSystemDriver* csGraphics2DDOSAlleg::DosSystem = NULL;

csGraphics2DDOSAlleg::csGraphics2DDOSAlleg (ISystem* piSystem) :
  csGraphics2D (piSystem)
{
  Memory = NULL;
  System = piSystem;

  if (FAILED (System->QueryInterface (IID_IDosSystemDriver, (void**)&DosSystem)))
  {
    CsPrintf (MSG_FATAL_ERROR, "The system driver does not support "
                             "the IDosSystemDriver interface\n");
    exit (-1);
  }
}

csGraphics2DDOSAlleg::~csGraphics2DDOSAlleg ()
{
  Close();
  FINAL_RELEASE (DosSystem);
}

void csGraphics2DDOSAlleg::CsPrintf (int msgtype, char *format, ...)
{
  va_list arg;
  char buf[256];

  va_start (arg, format);
  vsprintf (buf, format, arg);
  va_end (arg);

  System->Print (msgtype, buf);
}

void csGraphics2DDOSAlleg::Initialize ()
{
  csGraphics2D::Initialize ();
  switch (Depth)
  {
    case 8:
      pfmt.RedMask = pfmt.GreenMask = pfmt.BlueMask = 0;
      pfmt.PalEntries = 256; pfmt.PixelBytes = 1;
      break;
    case 15:
      pfmt.RedMask   = 0x1f << 10;
      pfmt.GreenMask = 0x1f << 5;
      pfmt.BlueMask  = 0x1f;
      pfmt.PalEntries = 0;
      pfmt.PixelBytes = 2;
      break;
    case 16:
      pfmt.RedMask   = 0x1f << 11;
      pfmt.GreenMask = 0x3f << 5;
      pfmt.BlueMask  = 0x1f;
      pfmt.PalEntries = 0;
      pfmt.PixelBytes = 2;
      break;
    case 32:
      pfmt.RedMask   = 0xff << 16;
      pfmt.GreenMask = 0xff << 8;
      pfmt.BlueMask  = 0xff;
      pfmt.PalEntries = 0;
      pfmt.PixelBytes = 4;
      break;
  }
  complete_pixel_format();
}

bool csGraphics2DDOSAlleg::Open (char* Title)
{
  if (!csGraphics2D::Open (Title))
    return false;
  PaletteChanged = false;

  allegro_init ();
  set_color_depth (Depth);

  // I removed the mode searching code, cause Allegro GFX_AUTODETECT
  // selects the fastest mode automatically
  if (set_gfx_mode (GFX_AUTODETECT, Width, Height, 0, 0))
  {
    CsPrintf (MSG_FATAL_ERROR, "ERROR! Could not set graphics mode.\n");
    exit (-1);
  }

  _cs_alleg2d = create_bitmap_ex(Depth,Width,Height);
  Memory = (unsigned char *)_cs_alleg2d->dat;

  if (Memory == NULL)
  {
    CsPrintf (MSG_FATAL_ERROR, "Error initializing graphics subsystem: bad videomode!\n");
    return false;
  }

  // Tell printf() to shut up
  DosSystem->EnablePrintf (false);

  // Update drawing routine addresses
  switch (pfmt.PixelBytes)
  {
    case 1:
      break;
    case 2:
      DrawPixel = DrawPixel16;
      WriteChar = WriteChar16;
      GetPixelAt = GetPixelAt16;
      DrawSprite = DrawSprite16;
      break;
    case 4:
      DrawPixel = DrawPixel32;
      WriteChar = WriteChar32;
      GetPixelAt = GetPixelAt32;
      DrawSprite = DrawSprite32;
      break;
    default:
      CsPrintf (MSG_WARNING, "WARNING: No 2D routines for selected mode!\n");
      break;
  } /* endif */

  // Clear all videopages
  ClearAll (0);

  return true;
}

void csGraphics2DDOSAlleg::Close ()
{
  destroy_bitmap ((BITMAP *)_cs_alleg2d);
  csGraphics2D::Close ();
  // Tell printf() it can work now
  DosSystem->EnablePrintf (true);
}

void csGraphics2DDOSAlleg::Print (csRect *area)
{
  if (PaletteChanged && pfmt.PalEntries)
  {
    outportb (0x3c8, 0);
    for (int i = 0; i <= 255; i++)
    {
      outportb (0x3c9, Palette[i].red >> 2);
      outportb (0x3c9, Palette[i].green >> 2);
      outportb (0x3c9, Palette[i].blue >> 2);
    } /* endfor */
    PaletteChanged = false;
  }
  if (area)
    blit (_cs_alleg2d, screen, area->xmin, area->ymin, area->xmin, area->ymin,area->Width (), area->Height ());
  else
    blit (_cs_alleg2d, screen, 0, 0, 0, 0, Width, Height);
}

bool csGraphics2DDOSAlleg::SetMousePosition (int x, int y)
{
  return DosSystem->SetMousePosition (x, y);
}

bool csGraphics2DDOSAlleg::SetMouseCursor (int iShape, ITextureHandle *hBitmap)
{
  (void)iShape; (void)hBitmap;
  return false;
}

void csGraphics2DDOSAlleg::SetRGB (int i, int r, int g, int b)
{
  csGraphics2D::SetRGB (i, r, g, b);
  PaletteChanged = true;
}

bool csGraphics2DDOSAlleg::BeginDraw ()
{
  return csGraphics2D::BeginDraw ();
}
