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
#include "glbe2d.h"
#include "CrystGLWindow.h"
#include "isys/system.h"
#include <Screen.h>

CS_IMPLEMENT_PLUGIN

IMPLEMENT_FACTORY (csGraphics2DGLBe)

EXPORT_CLASS_TABLE (glbe2d)
  EXPORT_CLASS_DEP (csGraphics2DGLBe, "crystalspace.graphics2d.glbe",
    "Crystal Space 2D driver for OpenGL using GL on BeOS",
    "crystalspace.font.server.")
EXPORT_CLASS_TABLE_END

csGraphics2DGLBe::csGraphics2DGLBe(iBase* p) :
  superclass(p), view(0), window(0)
{
}

csGraphics2DGLBe::~csGraphics2DGLBe()
{
}

bool csGraphics2DGLBe::Initialize(iSystem* p)
{
  bool ok = csGraphics2DGLCommon::Initialize(p);
  if (ok)
  {
    CsPrintf (MSG_INITIALIZATION, "Crystal Space BeOS OpenGL 2D driver.\n");
    // Get current screen information.
    BScreen screen(B_MAIN_SCREEN_ID);
    screen_frame = screen.Frame();
    curr_color_space = screen.ColorSpace();
    ApplyDepthInfo(curr_color_space);
  }
  return ok;
}

bool csGraphics2DGLBe::Open(char const* title)
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

  view = new CrystGLView(BRect(0, 0, vw, vh), System);
  window = new CrystGLWindow(win_rect, title, view, System, this);

  window->Show();
  if (window->Lock())
  {
    view->MakeFocus();
    window->Unlock();
  }
  window->Flush();
  System->SystemExtension("BeginUI");

  return superclass::Open(title);
}

void csGraphics2DGLBe::Close()
{
  window->Lock();
  window->Quit();
  window = NULL;
  superclass::Close();
}

bool csGraphics2DGLBe::BeginDraw()
{
  superclass::BeginDraw();
  if (FrameBufferLocked == 1)
    view->LockGL();
  return true;
}

void csGraphics2DGLBe::FinishDraw()
{
  superclass::FinishDraw();
  if (FrameBufferLocked == 0)
    view->UnlockGL();
}

void csGraphics2DGLBe::Print(csRect*)
{
  if (view != 0)
  {
    view->LockGL();
    view->SwapBuffers();
    view->UnlockGL();
    glFinish();
  }
}

bool csGraphics2DGLBe::SetMouseCursor(csMouseCursorID shape)
{
  return System->SystemExtension("SetCursor", shape);
}

void csGraphics2DGLBe::ApplyDepthInfo(color_space cs)
{
  unsigned long RedMask, GreenMask, BlueMask;
  switch (cs)
  {
    case B_RGB15:
      Depth	= 15;
      RedMask   = 0x1f << 10;
      GreenMask = 0x1f << 5;
      BlueMask  = 0x1f;

      pfmt.PixelBytes = 2;
      pfmt.PalEntries = 0;
      pfmt.RedMask    = RedMask;
      pfmt.GreenMask  = GreenMask;
      pfmt.BlueMask   = BlueMask;
      pfmt.PalEntries = 0;

      pfmt.complete ();
      break;
    case B_RGB16:
      Depth	= 16;
      RedMask   = 0x1f << 11;
      GreenMask = 0x3f << 5;
      BlueMask  = 0x1f;

      pfmt.PixelBytes = 2;
      pfmt.PalEntries = 0;
      pfmt.RedMask    = RedMask;
      pfmt.GreenMask  = GreenMask;
      pfmt.BlueMask   = BlueMask;
      pfmt.PalEntries = 0;

      pfmt.complete ();
      break;
    case B_RGB32:
    case B_RGBA32:
      Depth	= 32;
      RedMask   = 0xff << 16;
      GreenMask = 0xff << 8;
      BlueMask  = 0xff;

      pfmt.PixelBytes = 4;
      pfmt.PalEntries = 0;
      pfmt.RedMask    = RedMask;
      pfmt.GreenMask  = GreenMask;
      pfmt.BlueMask   = BlueMask;
      pfmt.PalEntries = 0;

      pfmt.complete ();
      break;
    default:
      printf("Unimplemented color depth in Be 2D OpenGL driver (depth=%i)\n",
        Depth);
      exit(1);
      break;
  }
}
