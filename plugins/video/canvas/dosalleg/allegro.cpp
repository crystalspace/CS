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

//------------------------------------------------------- csGraphics2DDOSAlleg ---

IMPLEMENT_FACTORY (csGraphics2DDOSAlleg)

EXPORT_CLASS_TABLE (allegro)
  EXPORT_CLASS (csGraphics2DDOSAlleg, "crystalspace.graphics2d.allegro",
    "DOS/DJGPP Allegro 2D graphics driver for Crystal Space")
EXPORT_CLASS_TABLE_END

IMPLEMENT_IBASE (csGraphics2DDOSAlleg)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iGraphics2D)
IMPLEMENT_IBASE_END

iSystem* csGraphics2DDOSAlleg::System = NULL;
IDosSystemDriver* csGraphics2DDOSAlleg::DosSystem = NULL;

csGraphics2DDOSAlleg::csGraphics2DDOSAlleg (iBase *iParent) :
  csGraphics2D ()
{
  CONSTRUCT_IBASE (iParent);
}

csGraphics2DDOSAlleg::~csGraphics2DDOSAlleg ()
{
  Close ();
  if (DosSystem)
    DosSystem->DecRef ();
}

bool csGraphics2DDOSAlleg::Initialize (iSystem *pSystem)
{
  if (!csGraphics2D::Initialize (pSystem))
    return false;

  DosSystem = QUERY_INTERFACE (System, iDosSystemDriver);
  if (!DosSystem)
  {
    CsPrintf (MSG_FATAL_ERROR, "The system driver does not support "
                             "the IDosSystemDriver interface\n");
    return false;
  }

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
  return true;
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
      _DrawPixel = DrawPixel16;
      _WriteChar = WriteChar16;
      _GetPixelAt = GetPixelAt16;
      _DrawSprite = DrawSprite16;
      break;
    case 4:
      _DrawPixel = DrawPixel32;
      _WriteChar = WriteChar32;
      _GetPixelAt = GetPixelAt32;
      _DrawSprite = DrawSprite32;
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

bool csGraphics2DDOSAlleg::SetMouseCursor (csMouseCursorID iShape, iTextureHandle *hBitmap)
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
