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
#include "cscom/com.h"
#include "cs2d/be/belibg2d.h"
#include "cs2d/be/CrystWindow.h"
#include "cssys/be/beitf.h"
#include "csgeom/csrect.h"
#include "isystem.h"

BEGIN_INTERFACE_TABLE(csGraphics2DBeLib)
  IMPLEMENTS_COMPOSITE_INTERFACE_EX( IGraphics2D, XGraphics2D )
  IMPLEMENTS_COMPOSITE_INTERFACE_EX( IGraphicsInfo, XGraphicsInfo )
  IMPLEMENTS_COMPOSITE_INTERFACE_EX( IBeLibGraphicsInfo, XBeLibGraphicsInfo )
END_INTERFACE_TABLE()

IMPLEMENT_UNKNOWN_NODELETE(csGraphics2DBeLib)

csGraphics2DBeLib::csGraphics2DBeLib(ISystem* piSystem) :
  csGraphics2D(piSystem), curr_page(0), double_buffered(true)
{
  HRESULT const rc = piSystem->QueryInterface(
    (REFIID)IID_IBeLibSystemDriver, (void**)&be_system);
  if (FAILED (rc)) {
    system->Print (MSG_FATAL_ERROR, "FATAL: The system driver does not "
      "implement the IBeLibSystemDriver interface\n");
    exit (1);
  }
  
  // set up interface flags
  // locker is set up in crystwindow as are these flags
#if 0
  fConnected = false;
  fConnectionDisabled=false;
  fDrawingThreadSuspended=false;
#endif

  system->Print (MSG_INITIALIZATION, "Crystal Space BeOS version.\n");
}

csGraphics2DBeLib::~csGraphics2DBeLib()
{
  Close();
  // FIXME: Free the bitmaps in cryst_bitmap.
  be_system->Release();
}

void csGraphics2DBeLib::Initialize ()
{
  csGraphics2D::Initialize();

  // Get current screen information.
  BScreen screen(B_MAIN_SCREEN_ID);
  screen_frame = screen.Frame();
  curr_color_space = screen.ColorSpace();
  ApplyDepthInfo(curr_color_space);

  // Create buffers.
  double_buffered = true;
  curr_page = 0;
  BRect const r(0, 0, Width - 1, Height - 1);
  for (int i=0; i < BUFFER_COUNT; i++)
  	CHK (cryst_bitmap[i] = new BBitmap(r, curr_color_space));
}

bool csGraphics2DBeLib::Open(const char* title)
{
  if (!csGraphics2D::Open (title)) return false;

  int const INSET = 32;
  int const sw = screen_frame.IntegerWidth();
  int const sh = screen_frame.IntegerHeight();
  int const vw = Width  - 1;
  int const vh = Height - 1;
  BRect win_rect(INSET, INSET, vw + INSET, vh + INSET);

  if (vw <= sw && vh <= sh) {	// Center the window.
    float const x = floor((sw - vw) / 2);
    float const y = floor((sh - vh) / 2);
    win_rect.Set(x, y, x + vw, y + vh);
  }

  dpy = CHK(new CrystView(BRect(0, 0, vw, vh), be_system));
  window = CHK(new CrystWindow(win_rect, title, dpy, this, system, be_system));
	
  window->Show();
  if(window->Lock()) {
    dpy->MakeFocus();
    window->Unlock();
  }
	
  Memory = (unsigned char *)cryst_bitmap[curr_page]->Bits();
  return true;
}

void csGraphics2DBeLib::Close()
{
  window->Lock();
  window->Quit();
  window = NULL;
  csGraphics2D::Close();
}

bool csGraphics2DBeLib::BeginDraw()
{
  if (Memory == NULL) return false;
#if 0
  // if fConnectionDisabled, then I will suspend the drawing thread and await shutdown.
  // fConnectionDisabled is set true by the csGraphics2DBeLib destructor.
  // If true, then the Window is being destroyed so this thread should go as well!
  // The application object may try to kill it too but it doesn't matter: you can only die once!
  if (fConnectionDisabled) kill_thread(find_thread(NULL));

  // lock 2D driver object
  locker->Lock();

  // this implements the fConnected feature with suspend_thread
  // it is only feasible because this is the only place that suspends that thread.
  // if you put in suspension elsewhere, use a proper semaphore
  if (!fConnected)	{
    fDrawingThreadSuspended = true;
    locker->Unlock();
    suspend_thread(find_thread(NULL));	
  }
#endif
  return true;
}

void csGraphics2DBeLib::FinishDraw ()
{
#if 0
	locker->Unlock();
#endif
}

void csGraphics2DBeLib::Print (csRect* area)
{
	if(window->Lock()) {
		if(!double_buffered)
			dpy->DrawBitmap(cryst_bitmap[curr_page]);
		else {
			dpy->Sync();
			dpy->DrawBitmapAsync(cryst_bitmap[curr_page]);
			dpy->Flush();
			curr_page = ++curr_page % BUFFER_COUNT;
			Memory = (unsigned char*)cryst_bitmap[curr_page]->Bits();
		}
		window->Unlock();
	}
}

bool csGraphics2DBeLib::DoubleBuffer (bool Enable)
{
  double_buffered = Enable;
  return true;
}

bool csGraphics2DBeLib::DoubleBuffer ()
{
  return double_buffered;
}

int csGraphics2DBeLib::GetPage ()
{
  return curr_page;
}

bool csGraphics2DBeLib::SetMouseCursor (int shape, ITextureHandle* bitmap) {
  return (be_system->SetMouseCursor(shape, bitmap) == S_OK);
}

void csGraphics2DBeLib::ApplyDepthInfo(color_space this_color_space)
{
  unsigned long RedMask, GreenMask, BlueMask;
  
  switch (this_color_space) {
  	case B_RGB15: 
		Depth	  = 15;
  		RedMask   = 0x1f << 10;
  		GreenMask = 0x1f << 5;
  		BlueMask  = 0x1f;
  		
  		DrawPixel = DrawPixel16;
  		WriteChar = WriteChar16;
  		GetPixelAt= GetPixelAt16;
  		DrawSprite= DrawSprite16;
  		
  		pfmt.PixelBytes = 2;
  		pfmt.PalEntries = 0;
  		pfmt.RedMask    = RedMask;
  		pfmt.GreenMask  = GreenMask;
  		pfmt.BlueMask   = BlueMask;
  		
  		complete_pixel_format();
  		break;
  	case B_RGB16:
  		Depth	  = 16;
  		RedMask   = 0x1f << 11;
  		GreenMask = 0x3f << 5;
  		BlueMask  = 0x1f;
  		
  		DrawPixel = DrawPixel16;
  		WriteChar = WriteChar16;
  		GetPixelAt= GetPixelAt16;
  		DrawSprite= DrawSprite16;
  		
  		pfmt.PixelBytes = 2;
  		pfmt.PalEntries = 0;
  		pfmt.RedMask    = RedMask;
  		pfmt.GreenMask  = GreenMask;
  		pfmt.BlueMask   = BlueMask;
  		
  		complete_pixel_format();
  		break;
  	case B_RGB32:
  	case B_RGBA32:
		Depth	  = 32;
  		RedMask   = 0xff << 16;
  		GreenMask = 0xff << 8;
  		BlueMask  = 0xff;
  		
  		DrawPixel = DrawPixel32;
  		WriteChar = WriteChar32;
  		GetPixelAt= GetPixelAt32;
  		DrawSprite= DrawSprite32;
  		
  		pfmt.PixelBytes = 4;
  		pfmt.PalEntries = 0;
  		pfmt.RedMask    = RedMask;
  		pfmt.GreenMask  = GreenMask;
  		pfmt.BlueMask   = BlueMask;
  		
  		complete_pixel_format();
  		break;
  	default:
  		printf("Unimplemented color depth in Be 2D driver, depth = %i\n", Depth);
  		exit(1);
  		break;
  }
}
