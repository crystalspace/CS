/*
    DOS support for Crystal Space 3D library
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by David N. Arnold <derek_arnold@fuse.net>
    Written by Andrew Zabolotny <bit@eltech.ru>

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
#include "cssysdef.h"
#include "raw.h"
#include "csgeom/csrect.h"
#include "isys/system.h"

#include "djvidsys.h"

static VideoSystem VS;

//------------------------------------------------------- csGraphics2DDOSRAW ---

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csGraphics2DDOSRAW)

SCF_EXPORT_CLASS_TABLE (dosraw)
  SCF_EXPORT_CLASS_DEP (csGraphics2DDOSRAW, "crystalspace.graphics2d.dosraw",
    "DOS/DJGPP 2D graphics driver for Crystal Space", "crystalspace.font.server.")
SCF_EXPORT_CLASS_TABLE_END

csGraphics2DDOSRAW::csGraphics2DDOSRAW (iBase *p) : csGraphics2D (p)
{
  Memory = NULL;
}

csGraphics2DDOSRAW::~csGraphics2DDOSRAW ()
{
  Close();
}

bool csGraphics2DDOSRAW::Initialize (iSystem *pSystem)
{
  if (!csGraphics2D::Initialize (pSystem))
    return false;

  // Tell videosystem not to wait vertical retrace during page flip
  VS.WaitVRetrace (false);

  long rm, gm, bm;
  if (!VS.FindMode (Width, Height, Depth, pfmt.PalEntries,
        rm, gm, bm))
  {
    CsPrintf (CS_MSG_FATAL_ERROR, "Cannot find a suitable videomode match\n");
    exit (-1);
  }

  pfmt.RedMask = (ULong)rm;
  pfmt.GreenMask = (ULong)gm;
  pfmt.BlueMask = (ULong)bm;
  pfmt.PixelBytes = (Depth + 7) / 8;
  pfmt.complete ();
  return true;
}

bool csGraphics2DDOSRAW::Open (const char* Title)
{
  if (is_open) return true;
  if (!csGraphics2D::Open (Title))
    return false;
  PaletteChanged = false;

#if USE_ALLEGRO
  allegro_init ();

  int rc;
  set_color_depth (Depth);
  if (Width == 320 && Height == 200 && Depth == 8)
    rc = set_gfx_mode (GFX_MODEX, 320, 200, 0, 600); // triple buffering
  else
  {
    rc = set_gfx_mode (GFX_AUTODETECT, Width, Height, 0, 0);
    if (rc)
    {
      rc = set_gfx_mode (GFX_VESA1, Width, Height, 0, 0);
      if (rc)
      {
        rc = set_gfx_mode (GFX_VESA2B, Width, Height, 0, 0);
        if (!rc)
          CsPrintf (CS_MSG_INITIALIZATION, "VESA2 Banked mode selected.\n");
      }
      else
        CsPrintf (CS_MSG_INITIALIZATION, "VESA1 mode selected.\n");
    }
  }

  if (rc)
  {
    CsPrintf (CS_MSG_FATAL_ERROR, "ERROR! Could not set graphics mode.\n");
    exit (-1);
  }

  _vdata = create_bitmap (Width, Height);
  Memory = (unsigned char *)_vdata->dat;

  if (Memory == NULL)
  {
    CsPrintf (CS_MSG_FATAL_ERROR, "Error initializing graphics subsystem: bad videomode!\n");
    return false;
  }

#else // USE_ALLEGRO

  switch (VS.Open ())
  {
    case 0:
      break;
    case -1:
      CsPrintf (CS_MSG_FATAL_ERROR, "VESA ERROR: Incompatible VESA BIOS detected!\n");
      return false;
    case -2:
      CsPrintf (CS_MSG_FATAL_ERROR, "DPMI ERROR: Cannot map videobuffer into linear address space\n");
      return false;
    case -3:
      CsPrintf (CS_MSG_FATAL_ERROR, "DPMI ERROR: Cannot set up a selector for accessing video memory\n");
      return false;
    case -4:
      CsPrintf (CS_MSG_FATAL_ERROR, "MEM ERROR: Not enough memory for allocating a back buffer\n");
      return false;
    default:
      CsPrintf (CS_MSG_FATAL_ERROR, "UNKNOWN ERROR ON VIDEO SUBSYSTEM INITIALIZATION\n");
      return false;
  }

#endif // USE_ALLEGRO

  // Tell printf() to shut up
  System->PerformExtension ("EnablePrintf", false);

  // Update drawing routine addresses
  switch (pfmt.PixelBytes)
  {
    case 1:
      break;
    case 2:
      _DrawPixel = DrawPixel16;
      _WriteString = WriteString16;
      _GetPixelAt = GetPixelAt16;
      break;
    case 4:
      _DrawPixel = DrawPixel32;
      _WriteString = WriteString32;
      _GetPixelAt = GetPixelAt32;
      break;
    default:
      CsPrintf (CS_MSG_WARNING, "WARNING: No 2D routines for selected mode!\n");
      break;
  } /* endif */

  // Clear all videopages
  ClearAll (0);

  return true;
}

void csGraphics2DDOSRAW::Close ()
{
  if (!is_open) return;
#if USE_ALLEGRO
  destroy_bitmap ((BITMAP *)_vdata);
#else
  VS.Close ();
#endif // USE_ALLEGRO
  csGraphics2D::Close ();
  // Tell printf() it can work now
  System->PerformExtension ("EnablePrintf", true);
}

void csGraphics2DDOSRAW::Print (csRect *area)
{
#if USE_ALLEGRO
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
    blit (_vdata, screen, area->xmin, area->ymin, area->xmin, area->ymin,
      area->Width (), area->Height ());
  else
    blit (_vdata, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
            please check above "if" for correctness
                    then remove this text
#else
  if (PaletteChanged && pfmt.PalEntries)
  {
    VS.SetPalette (Palette, 256);
    PaletteChanged = false;
  }
  if (area)
    VS.Flush (area->xmin, area->ymin, area->Width (), area->Height ());
  else
    VS.Flush (0, 0, Width, Height);
#endif // USE_ALLEGRO
}

bool csGraphics2DDOSRAW::SetMousePosition (int x, int y)
{
  return System->PerformExtension ("SetMousePosition", x, y);
}

bool csGraphics2DDOSRAW::SetMouseCursor (csMouseCursorID iShape)
{
  (void)iShape;
  return false;
}

void csGraphics2DDOSRAW::SetRGB (int i, int r, int g, int b)
{
  csGraphics2D::SetRGB (i, r, g, b);
  PaletteChanged = true;
}

bool csGraphics2DDOSRAW::BeginDraw ()
{
  csGraphics2D::BeginDraw ();
  if (FrameBufferLocked != 1)
    return true;

#if !USE_ALLEGRO
  Memory = VS.BackBuffer ();
  return (Memory != NULL);
#else
  return true;
#endif
}

#if !USE_ALLEGRO

void csGraphics2DDOSRAW::Clear (int color)
{
  VS.Clear (color);
}

void csGraphics2DDOSRAW::FinishDraw ()
{
  csGraphics2D::FinishDraw ();
  if (FrameBufferLocked)
    return;

  Memory = NULL;
}

int csGraphics2DDOSRAW::GetPage ()
{
  return VS.GetPage ();
}

bool csGraphics2DDOSRAW::GetDoubleBufferState ()
{
  return VS.GetDoubleBufferState ();
}

bool csGraphics2DDOSRAW::DoubleBuffer (bool Enable)
{
  return VS.DoubleBuffer (Enable);
}

#endif // USE_ALLEGRO
