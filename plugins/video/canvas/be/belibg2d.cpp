/*
    Copyright (C) 1998,1999 by Jorrit Tyberghein
    Written by Xavier Planet.
    Modified for better DDraw conformance by David Huen.
    Overhauled and re-engineered by Eric Sunshine <sunshine@sunshineco.com>

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

#include <sys/param.h>
#include "sysdef.h"
#include "csutil/scf.h"
#include "cs2d/be/belibg2d.h"
#include "cs2d/be/CrystWindow.h"
#include "csutil/csrect.h"
#include "isystem.h"

IMPLEMENT_FACTORY (csGraphics2DBeLib)

EXPORT_CLASS_TABLE (be2d)
  EXPORT_CLASS (csGraphics2DBeLib, "crystalspace.graphics2d.be",
    "Crystal Space 2D driver for BeOS")
EXPORT_CLASS_TABLE_END

IMPLEMENT_IBASE (csGraphics2DBeLib)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iGraphics2D)
IMPLEMENT_IBASE_END

csGraphics2DBeLib::csGraphics2DBeLib (iBase* p) :
  superclass(), be_system(0), view(0), window(0), bitmap(0)
{
  CONSTRUCT_IBASE(p);
}

csGraphics2DBeLib::~csGraphics2DBeLib()
{
  if (be_system != 0)
    be_system->DecRef();
  delete bitmap;
}

bool csGraphics2DBeLib::Initialize (iSystem* p)
{
  bool ok = superclass::Initialize(p);
  if (ok)
  {
    System->Print (MSG_INITIALIZATION, "Crystal Space BeOS version.\n");
    be_system = QUERY_INTERFACE (System, iBeLibSystemDriver);
    if (be_system != 0)
    {
      // Get current screen information.
      BScreen screen(B_MAIN_SCREEN_ID);
      screen_frame = screen.Frame();
      curr_color_space = screen.ColorSpace();
      ApplyDepthInfo(curr_color_space);

      // Create frame-buffer.
      BRect const r(0, 0, Width - 1, Height - 1);
      CHK (bitmap = new BBitmap(r, curr_color_space));
    }
    else
    {
      CsPrintf (MSG_FATAL_ERROR, "FATAL: System driver does not "
        "implement the iBeLibSystemDriver interface\n");
      ok = false;
    }
  }
  return ok;
}

bool csGraphics2DBeLib::Open(const char* title)
{
  bool ok = superclass::Open (title);
  if (ok)
  {
    int const INSET = 32;
    int const sw = screen_frame.IntegerWidth();
    int const sh = screen_frame.IntegerHeight();
    int const vw = Width  - 1;
    int const vh = Height - 1;
    BRect win_rect(INSET, INSET, vw + INSET, vh + INSET);

    if (vw <= sw && vh <= sh)
    {
      float const x = floor((sw - vw) / 2); // Center window horizontally.
      float const y = floor((sh - vh) / 4); // A pleasing vertical position.
      win_rect.Set(x, y, x + vw, y + vh);
    }

    view = CHK(new CrystView(BRect(0, 0, vw, vh), be_system));
    window = CHK(new CrystWindow(win_rect, title, view, System, be_system));
	
    window->Show();
    if(window->Lock())
    {
      view->MakeFocus();
      window->Unlock();
    }
	
    Memory = (unsigned char*)bitmap->Bits();
  }
  return ok;
}

void csGraphics2DBeLib::Close()
{
  window->Lock();
  window->Quit();
  window = NULL;
  superclass::Close();
}

void csGraphics2DBeLib::Print (csRect*)
{
  if (window->Lock())
  {
    view->DrawBitmap (bitmap);
    window->Unlock();
  }
}

bool csGraphics2DBeLib::SetMouseCursor (csMouseCursorID shape)
{
  return be_system->SetMouseCursor(shape);
}

void csGraphics2DBeLib::ApplyDepthInfo(color_space cs)
{
  unsigned long RedMask, GreenMask, BlueMask;
  switch (cs)
  {
    case B_RGB15: 
      Depth	= 15;
      RedMask   = 0x1f << 10;
      GreenMask = 0x1f << 5;
      BlueMask  = 0x1f;

      _DrawPixel = DrawPixel16;
      _WriteChar = WriteChar16;
      _GetPixelAt= GetPixelAt16;
      _DrawSprite= DrawSprite16;

      pfmt.PixelBytes = 2;
      pfmt.PalEntries = 0;
      pfmt.RedMask    = RedMask;
      pfmt.GreenMask  = GreenMask;
      pfmt.BlueMask   = BlueMask;

      complete_pixel_format();
      break;
    case B_RGB16:
      Depth	= 16;
      RedMask   = 0x1f << 11;
      GreenMask = 0x3f << 5;
      BlueMask  = 0x1f;

      _DrawPixel = DrawPixel16;
      _WriteChar = WriteChar16;
      _GetPixelAt= GetPixelAt16;
      _DrawSprite= DrawSprite16;

      pfmt.PixelBytes = 2;
      pfmt.PalEntries = 0;
      pfmt.RedMask    = RedMask;
      pfmt.GreenMask  = GreenMask;
      pfmt.BlueMask   = BlueMask;

      complete_pixel_format();
      break;
    case B_RGB32:
    case B_RGBA32:
      Depth	= 32;
      RedMask   = 0xff << 16;
      GreenMask = 0xff << 8;
      BlueMask  = 0xff;

      _DrawPixel = DrawPixel32;
      _WriteChar = WriteChar32;
      _GetPixelAt= GetPixelAt32;
      _DrawSprite= DrawSprite32;

      pfmt.PixelBytes = 4;
      pfmt.PalEntries = 0;
      pfmt.RedMask    = RedMask;
      pfmt.GreenMask  = GreenMask;
      pfmt.BlueMask   = BlueMask;

      complete_pixel_format();
      break;
    default:
      printf ("Unimplemented color depth in Be 2D driver (depth=%i)\n", Depth);
      exit(1);
      break;
  }
}
