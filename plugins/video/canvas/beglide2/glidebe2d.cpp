/*
    Copyright (C) 1999-2001 by Eric Sunshine <sunshine@sunshineco.com>

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
#include "glidebe2d.h"
#include "crystglidewindow.h"
#include "csgeom/csrect.h"
#include "isys/system.h"
#include <Screen.h>

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csGraphics2DBeGlide)

SCF_EXPORT_CLASS_TABLE (glidebe2d)
  SCF_EXPORT_CLASS_DEP (csGraphics2DBeGlide,
    "crystalspace.graphics2d.glide.be.2",
    "Crystal Space Glide2 2D driver for BeOS",
    "crystalspace.font.server.")
SCF_EXPORT_CLASS_TABLE_END

csGraphics2DBeGlide::csGraphics2DBeGlide(iBase* p) :
  superclass(p), view(0), window(0), bitmap(0)
{
}

csGraphics2DBeGlide::~csGraphics2DBeGlide()
{
}

bool csGraphics2DBeGlide::Initialize(iSystem* isys)
{
  bool ok = superclass::Initialize(isys);
  if (ok)
  {
    System->Printf(CS_MSG_INITIALIZATION,
      "Crystal Space BeOS Glide2 2D Driver.\n");

    // Get current screen information.
    BScreen screen(B_MAIN_SCREEN_ID);
    screen_frame = screen.Frame();
    curr_color_space = screen.ColorSpace();
    ApplyDepthInfo(curr_color_space);

    // Create frame-buffer.
    BRect const r(0, 0, Width - 1, Height - 1);
    bitmap = new BBitmap(r, curr_color_space);
    memset(bitmap->Bits(), 0, bitmap->BitsLength()); // Clear to black.
  }
  return ok;
}

bool csGraphics2DBeGlide::Open(char const* title)
{
  bool ok = superclass::Open(title);
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

    view = new CrystGlideView(BRect(0, 0, vw, vh), System, bitmap);
    window = new CrystGlideWindow(win_rect, title, view, System, this);

    window->Show();
    if (window->Lock())
    {
      view->MakeFocus();
      window->Unlock();
    }
    window->Flush();

    Memory = (unsigned char*)bitmap->Bits();
    System->SystemExtension("BeginUI");
  }
  return ok;
}

void csGraphics2DBeGlide::Close()
{
  window->Lock();
  window->Quit();
  window = NULL;
  delete bitmap;
  bitmap = NULL;
  superclass::Close();
}

void csGraphics2DBeGlide::Print (csRect* r)
{
  if (m_DoGlideInWindow)
    FXgetImage(r);
  superclass::Print(r);
}

bool csGraphics2DBeGlide::SetMouseCursor(csMouseCursorID shape)
{
  return System->SystemExtension("SetCursor", shape);
}

void csGraphics2DBeGlide::FXgetImage(csRect* cr)
{
  // @@@FIXME: We only handle 16-bit (blah).
  if (Depth == 16)
    grLfbReadRegion(GR_BUFFER_FRONTBUFFER,0,0,Width,Height,Width*2,Memory);

  if (window->Lock())
  {
    // Adjust for BeOS coordinate system by decrementing values from csRect.
    BRect br;
    if (cr == 0)
      br = view->Bounds();
    else
      br.Set(cr->xmin, cr->ymin, cr->xmax - 1, cr->ymax - 1);
    view->Draw(br);
    window->Unlock();
  }
}

void csGraphics2DBeGlide::ApplyDepthInfo(color_space cs)
{
  unsigned long RedMask, GreenMask, BlueMask;
  switch (cs)
  {
    case B_RGB15:
      Depth     = 15;
      RedMask   = 0x1f << 10;
      GreenMask = 0x1f << 5;
      BlueMask  = 0x1f;

      _DrawPixel = DrawPixel16;
      _WriteString = WriteString16;
      _GetPixelAt= GetPixelAt16;

      pfmt.PixelBytes = 2;
      pfmt.PalEntries = 0;
      pfmt.RedMask    = RedMask;
      pfmt.GreenMask  = GreenMask;
      pfmt.BlueMask   = BlueMask;

      pfmt.complete();
      break;
    case B_RGB16:
      Depth     = 16;
      RedMask   = 0x1f << 11;
      GreenMask = 0x3f << 5;
      BlueMask  = 0x1f;

      _DrawPixel = DrawPixel16;
      _WriteString = WriteString16;
      _GetPixelAt= GetPixelAt16;

      pfmt.PixelBytes = 2;
      pfmt.PalEntries = 0;
      pfmt.RedMask    = RedMask;
      pfmt.GreenMask  = GreenMask;
      pfmt.BlueMask   = BlueMask;

      pfmt.complete();
      break;
    case B_RGB32:
    case B_RGBA32:
      Depth     = 32;
      RedMask   = 0xff << 16;
      GreenMask = 0xff << 8;
      BlueMask  = 0xff;

      _DrawPixel = DrawPixel32;
      _WriteString = WriteString32;
      _GetPixelAt= GetPixelAt32;

      pfmt.PixelBytes = 4;
      pfmt.PalEntries = 0;
      pfmt.RedMask    = RedMask;
      pfmt.GreenMask  = GreenMask;
      pfmt.BlueMask   = BlueMask;

      pfmt.complete();
      break;
    default:
      printf("Unimplemented color depth in Be2D driver (depth=%i)\n", Depth);
      exit(1);
      break;
  }
}

void csGraphics2DBeGlide::SetFullScreen(bool enable)
{
  grSstControl(enable ? GR_CONTROL_ACTIVATE : GR_CONTROL_DEACTIVATE);
  m_DoGlideInWindow = !enable;
}
