/*
    Copyright (C) 1998,1999 by Jorrit Tyberghein
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

#include <stdarg.h>
#include <sys/param.h>
#include "sysdef.h"
#include "csutil/scf.h"
#include "cs2d/openglbe/glbe2d.h"
#include "cs2d/openglbe/CrystGLWindow.h"
#include "cssys/be/beitf.h"
#include "isystem.h"

IMPLEMENT_FACTORY (csGraphics2DGLBe)

EXPORT_CLASS_TABLE (glbe2d)
  EXPORT_CLASS (csGraphics2DGLBe, "crystalspace.graphics2d.glbe",
    "BeOS OpenGL 2D graphics driver for Crystal Space")
EXPORT_CLASS_TABLE_END

csGraphics2DGLBe::csGraphics2DGLBe(iBase *iParent) :
  csGraphics2DGLCommon (iParent)
{
}

csGraphics2DGLBe::~csGraphics2DGLBe ()
{
  Close ();
  be_system->DecRef();
}

bool csGraphics2DGLBe::Initialize (iSystem *pSystem)
{
  if (!csGraphics2DGLCommon::Initialize (pSystem))
    return false;

  CsPrintf (MSG_INITIALIZATION, "Crystal Space BeOS OpenGL version.\n");

  be_system = QUERY_INTERFACE (System, iBeLibSystemDriver);
  if (!be_system)
  {
    CsPrintf (MSG_FATAL_ERROR, "FATAL: The system driver does not "
      "implement the iBeLibSystemDriver interface\n");
    return false;
  }

  // Get current screen information.
  BScreen screen(B_MAIN_SCREEN_ID);
  screen_frame = screen.Frame();
  curr_color_space = screen.ColorSpace();
  ApplyDepthInfo(curr_color_space);

  return true;
}

bool csGraphics2DGLBe::Open(const char* title)
{
  int const INSET = 32;
  int const sw = screen_frame.IntegerWidth();
  int const sh = screen_frame.IntegerHeight();
  int const vw = Width  - 1;
  int const vh = Height - 1;
  BRect win_rect(INSET, INSET, vw + INSET, vh + INSET);

  if (vw <= sw && vh <= sh) {
    float const x = floor((sw - vw) / 2); // Center window horizontally.
    float const y = floor((sh - vh) / 4); // A pleasing vertical position.
    win_rect.Set(x, y, x + vw, y + vh);
  }

  dpy = CHK(new CrystGLView(BRect(0, 0, vw, vh), be_system));
  window = CHK(new CrystGLWindow(win_rect, title, dpy, this, system, be_system));
	
  window->Show();
  if (window->Lock()) {
    dpy->MakeFocus();
    window->Unlock();
  }

  // FIXME: The application crashes unless the superclass Open() is called
  // after the earlier initialization.  Normally this call would appear at the
  // top of this method.
  if (!csGraphics2DGLCommon::Open (title))
    return false;

  Clear(0);
  return true;
}

void csGraphics2DGLBe::Close()
{
  window->Lock();
  window->Quit();
  window = NULL;
  csGraphics2DGLCommon::Close();
}

bool csGraphics2DGLBe::BeginDraw ()
{
  csGraphics2D::BeginDraw ();
  if (FrameBufferLocked != 1)
    return true;

#if 0
  // if fConnectionDisabled, then I will suspend the drawing thread and await shutdown.
  // fConnectionDisabled is set true by the csGraphics2DBeLib destructor.
  // If true, then the Window is being destroyed so this thread should go as well!
  // The application object may try to kill it too but it doesn't matter: you can only die once!
  if (window->fConnectionDisabled) kill_thread(find_thread(NULL));

  // lock 2D driver object
  window->locker->Lock();

  // this implements the fConnected feature with suspend_thread
  // it is only feasible because this is the only place that suspends that thread.
  // if you put in suspension elsewhere, use a proper semaphore
  if (!window->fConnected)	{
    window->fDrawingThreadSuspended = true;
    window->locker->Unlock();
    suspend_thread(find_thread(NULL));	
  } //uncomment for conventional DirectConnected
#endif
  dpy->LockGL();
  return true;
}

void csGraphics2DGLBe::FinishDraw ()
{
  csGraphics2D::FinishDraw ();
  if (FrameBufferLocked)
    return;

//window->locker->Unlock();// uncomment for conventional DirectConnected.
  dpy->UnlockGL();
}

void csGraphics2DGLBe::Print (csRect *area)
{
  if(dpy) {
    dpy->LockGL();
    dpy->SwapBuffers();
    dpy->UnlockGL();
    glFinish();
  }
}

void csGraphics2DGLBe::ApplyDepthInfo(color_space this_color_space)
{
  unsigned long RedMask, GreenMask, BlueMask;
  
  switch (this_color_space) {
  	case B_RGB15: 
		Depth	  = 15;
  		RedMask   = 0x1f << 10;
  		GreenMask = 0x1f << 5;
  		BlueMask  = 0x1f;

  		pfmt.PixelBytes = 2;
  		pfmt.PalEntries = 0;
  		pfmt.RedMask    = RedMask;
  		pfmt.GreenMask  = GreenMask;
  		pfmt.BlueMask   = BlueMask;
  		pfmt.PalEntries = 0;
  		
  		complete_pixel_format();
  		break;
  	case B_RGB16:
  		Depth	  = 16;
  		RedMask   = 0x1f << 11;
  		GreenMask = 0x3f << 5;
  		BlueMask  = 0x1f;

  		pfmt.PixelBytes = 2;
  		pfmt.PalEntries = 0;
  		pfmt.RedMask    = RedMask;
  		pfmt.GreenMask  = GreenMask;
  		pfmt.BlueMask   = BlueMask;
  		pfmt.PalEntries = 0;
  		  		
  		complete_pixel_format();
  		break;
  	case B_RGB32:
  	case B_RGBA32:
		Depth	  = 32;
  		RedMask   = 0xff << 16;
  		GreenMask = 0xff << 8;
  		BlueMask  = 0xff;

  		pfmt.PixelBytes = 4;
  		pfmt.PalEntries = 0;
  		pfmt.RedMask    = RedMask;
  		pfmt.GreenMask  = GreenMask;
  		pfmt.BlueMask   = BlueMask;
  		pfmt.PalEntries = 0;
  		
  		complete_pixel_format();
  		break;
  	default:
	  	printf("Unimplemented color depth in Be 2D OpenGL driver, depth = %i\n", Depth);
	  	exit(1);
  		break;
  }
}
