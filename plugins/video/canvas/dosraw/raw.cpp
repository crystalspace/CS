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

/*
    NOTE: Currently Allegro support is completely broken.
    I don't see a single reason for using it, anyway.
*/

#include <stdarg.h>
#include "sysdef.h"
#include "cs2d/dosraw/raw.h"
#include "cssys/djgpp/idjgpp.h"
#include "csgeom/csrect.h"
#include "isystem.h"

#ifdef USE_ALLEGRO
#  include "allegro.h"
#else
#  include "djvidsys.h"
#endif

static VideoSystem VS;

#ifdef USE_ALLEGRO
BITMAP *_vdata;
#endif

//--------------------------------------------------------- Static COM stuff ---

static DllRegisterData gRegData =
{
  &CLSID_DosRawGraphics2D,
#ifdef USE_ALLEGRO
  "crystalspace.graphics2d.allegro",
  "Crystal Space 2D graphics driver for DOS/DJGPP using Allegro video library"
#else
  "crystalspace.graphics2d.dosraw",
  "Crystal Space 2D graphics driver for DOS/DJGPP using raw framebuffer access"
#endif
};

void Raw2DRegister ()
{
  static csGraphics2DFactoryDOSRAW gG2DRawFactory;
  gRegData.pClass = &gG2DRawFactory;
  csRegisterServer (&gRegData);
}

void Raw2DUnregister ()
{
  csUnregisterServer (&gRegData);
}

//------------------------------------------------ csGraphics2DFactoryDOSRAW ---

IMPLEMENT_UNKNOWN_NODELETE (csGraphics2DFactoryDOSRAW)

BEGIN_INTERFACE_TABLE (csGraphics2DFactoryDOSRAW)
  IMPLEMENTS_INTERFACE (IGraphics2DFactory)
END_INTERFACE_TABLE ()

STDMETHODIMP csGraphics2DFactoryDOSRAW::CreateInstance (REFIID riid,
  ISystem* piSystem, void**ppv)
{
  if (!piSystem)
  {
    *ppv = 0;
    return E_INVALIDARG;
  }

  CHK (csGraphics2DDOSRAW *pNew = new csGraphics2DDOSRAW (piSystem));
  if (!pNew)
  {
    *ppv = 0;
    return E_OUTOFMEMORY;
  }

  return pNew->QueryInterface (riid, ppv);
}

STDMETHODIMP csGraphics2DFactoryDOSRAW::LockServer (COMBOOL bLock)
{
  (void)bLock;
  return S_OK;
}

//------------------------------------------------------- csGraphics2DDOSRAW ---

BEGIN_INTERFACE_TABLE (csGraphics2DDOSRAW)
  IMPLEMENTS_COMPOSITE_INTERFACE_EX (IGraphics2D, XGraphics2D)
  IMPLEMENTS_COMPOSITE_INTERFACE_EX (IGraphicsInfo, XGraphicsInfo)
END_INTERFACE_TABLE ()

IMPLEMENT_UNKNOWN_NODELETE (csGraphics2DDOSRAW)

ISystem* csGraphics2DDOSRAW::System = NULL;
IDosSystemDriver* csGraphics2DDOSRAW::DosSystem = NULL;

csGraphics2DDOSRAW::csGraphics2DDOSRAW (ISystem* piSystem) :
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

csGraphics2DDOSRAW::~csGraphics2DDOSRAW ()
{
  Close();
  FINAL_RELEASE (DosSystem);
}

void csGraphics2DDOSRAW::CsPrintf (int msgtype, char *format, ...)
{
  va_list arg;
  char buf[256];

  va_start (arg, format);
  vsprintf (buf, format, arg);
  va_end (arg);

  System->Print (msgtype, buf);
}

void csGraphics2DDOSRAW::Initialize ()
{
  csGraphics2D::Initialize ();

  // Tell videosystem not to wait vertical retrace during page flip
  VS.WaitVRetrace (false);

  long rm, gm, bm;
  if (!VS.FindMode (Width, Height, Depth, pfmt.PalEntries,
        rm, gm, bm))
  {
    CsPrintf (MSG_FATAL_ERROR, "Cannot find a suitable videomode match\n");
    exit (-1);
  }

  pfmt.RedMask = (ULong)rm;
  pfmt.GreenMask = (ULong)gm;
  pfmt.BlueMask = (ULong)bm;
  pfmt.PixelBytes = (Depth + 7) / 8;
  complete_pixel_format();
}

bool csGraphics2DDOSRAW::Open (char* Title)
{
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
          CsPrintf (MSG_INITIALIZATION, "VESA2 Banked mode selected.\n");
      }
      else
        CsPrintf (MSG_INITIALIZATION, "VESA1 mode selected.\n");
    }
  }

  if (rc)
  {
    CsPrintf (MSG_FATAL_ERROR, "ERROR! Could not set graphics mode.\n");
    exit (-1);
  }

  _vdata = create_bitmap (Width, Height);
  Memory = (unsigned char *)_vdata->dat;

  if (Memory == NULL)
  {
    CsPrintf (MSG_FATAL_ERROR, "Error initializing graphics subsystem: bad videomode!\n");
    return false;
  }

#else // USE_ALLEGRO

  switch (VS.Open ())
  {
    case 0:
      break;
    case -1:
      CsPrintf (MSG_FATAL_ERROR, "VESA ERROR: Incompatible VESA BIOS detected!\n");
      return false;
    case -2:
      CsPrintf (MSG_FATAL_ERROR, "DPMI ERROR: Cannot map videobuffer into linear address space\n");
      return false;
    case -3:
      CsPrintf (MSG_FATAL_ERROR, "DPMI ERROR: Cannot set up a selector for accessing video memory\n");
      return false;
    case -4:
      CsPrintf (MSG_FATAL_ERROR, "MEM ERROR: Not enough memory for allocating a back buffer\n");
      return false;
    default:
      CsPrintf (MSG_FATAL_ERROR, "UNKNOWN ERROR ON VIDEO SUBSYSTEM INITIALIZATION\n");
      return false;
  }

#endif // USE_ALLEGRO

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

void csGraphics2DDOSRAW::Close ()
{
#if USE_ALLEGRO
  destroy_bitmap ((BITMAP *)_vdata);
#else
  VS.Close ();
#endif // USE_ALLEGRO
  csGraphics2D::Close ();
  // Tell printf() it can work now
  DosSystem->EnablePrintf (true);
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
  return DosSystem->SetMousePosition (x, y);
}

bool csGraphics2DDOSRAW::SetMouseCursor (int iShape, ITextureHandle *hBitmap)
{
  (void)iShape; (void)hBitmap;
  return false;
}

void csGraphics2DDOSRAW::SetRGB (int i, int r, int g, int b)
{
  csGraphics2D::SetRGB (i, r, g, b);
  PaletteChanged = true;
}

bool csGraphics2DDOSRAW::BeginDraw ()
{
#if !USE_ALLEGRO
  Memory = VS.BackBuffer ();
  return (Memory != NULL);
#else
  return csGraphics2D::BeginDraw ();
#endif
}

#if !USE_ALLEGRO

void csGraphics2DDOSRAW::Clear (int color)
{
  VS.Clear (color);
}

void csGraphics2DDOSRAW::FinishDraw ()
{
  Memory = NULL;
}

int csGraphics2DDOSRAW::GetPage ()
{
  return VS.GetPage ();
}

bool csGraphics2DDOSRAW::DoubleBuffer ()
{
  return VS.DoubleBuffer ();
}

bool csGraphics2DDOSRAW::DoubleBuffer (bool Enable)
{
  return VS.DoubleBuffer (Enable);
}

#endif // USE_ALLEGRO
