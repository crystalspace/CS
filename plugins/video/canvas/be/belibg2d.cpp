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
#include "cssys/be/beitf.h"
#include "csutil/csrect.h"
#include "isystem.h"

IMPLEMENT_FACTORY (csGraphics2DBeLib)

EXPORT_CLASS_TABLE (be2d)
  EXPORT_CLASS (csGraphics2DBeLib, "crystalspace.graphics2d.be",
    "BeOS 2D driver for Crystal Space")
EXPORT_CLASS_TABLE_END

IMPLEMENT_IBASE (csGraphics2DBeLib)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iGraphics2D)
IMPLEMENT_IBASE_END

csGraphics2DBeLib::csGraphics2DBeLib (iBase *iParent) :
  csGraphics2D(), curr_page(0), double_buffered(true)
{
  CONSTRUCT_IBASE (iParent);
}

csGraphics2DBeLib::~csGraphics2DBeLib()
{
  Close();
  // FIXME: Free the bitmaps in cryst_bitmap.
  be_system->DecRef();
}

bool csGraphics2DBeLib::Initialize (iSystem *pSystem)
{
  if (!csGraphics2D::Initialize (pSystem))
    return false;

  be_system = QUERY_INTERFACE (System, iBeLibSystemDriver);
  if (!be_system)
  {
    CsPrintf (MSG_FATAL_ERROR, "FATAL: The system driver does not "
      "implement the iBeLibSystemDriver interface\n");
    return false;
  }
  
  // set up interface flags
  // locker is set up in crystwindow as are these flags
#if 0
  fConnected = false;
  fConnectionDisabled=false;
  fDrawingThreadSuspended=false;
#endif

  System->Print (MSG_INITIALIZATION, "Crystal Space BeOS version.\n");

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
  return true;
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

  if (vw <= sw && vh <= sh) {
    float const x = floor((sw - vw) / 2); // Center window horizontally.
    float const y = floor((sh - vh) / 4); // A pleasing vertical position.
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
  csGraphics2D::BeginDraw ();
  if (FrameBufferLocked != 1)
    return true;

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
  csGraphics2D::FinishDraw ();
  if (FrameBufferLocked)
    return;

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

bool csGraphics2DBeLib::SetMouseCursor (csMouseCursorID shape, iTextureHandle* bitmap)
{
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
  		Depth	  = 16;
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
		Depth	  = 32;
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
  		printf("Unimplemented color depth in Be 2D driver, depth = %i\n", Depth);
  		exit(1);
  		break;
  }
}

bool csGraphics2DBeLib::DirectConnect (direct_buffer_info *info)
{
// FIXME: Re-implement/re-enable DirectWindow mode.
#if 0
	if (!fConnected && fConnectionDisabled) {
		return S_OK;
	}
	locker->Lock();
	
	switch (info->buffer_state & B_DIRECT_MODE_MASK) {
		case B_DIRECT_START:
			printf("DirectConnect: B_DIRECT_START \n");
			fConnected = true;
			if (fDrawingThreadSuspended)	{
				while (resume_thread(find_thread("LoopThread")) == B_BAD_THREAD_STATE)	{
					//	this is done to cope with thread setting fDrawingThreadSuspended then getting
					//	rescheduled before it can suspend itself.  It just makes repeated attempts to
					//	resume that thread.
					snooze(1000);
				}
				fDrawingThreadSuspended = false;
			}
				
		case B_DIRECT_MODIFY:
			printf("DirectConnect: B_DIRECT_MODIFY \n");
			Memory = (unsigned char *) info->bits;
			printf("Memory allocated is %p. bytes_per_row is %ld \n", Memory, info->bytes_per_row);
			curr_color_space = info->pixel_format;
			ApplyDepthInfo(curr_color_space);
			
		// Create scanline address array
		if (LineAddress == NULL)	{
//			printf ("IXBeLibGraphicsInfo::DirectConnect() -- Creating LineAddress[].\n");
			CHK (LineAddress = new int [Height]);
			if (LineAddress == NULL)	{
			    printf ("IXBeLibGraphicsInfo::DirectConnect() -- Couldn't create LineAddress[].\n");
			    exit (1);
			}
		}
		
		//	initialise array
		{
//		printf("Window bounds: %ld %ld %ld %ld \n", info->window_bounds.left, info->window_bounds.top, info->window_bounds.right, info->window_bounds.bottom);
		int i,addr,bpl = info->bytes_per_row;
		for (i = 0, addr = info->window_bounds.left * pfmt.PixelBytes + info->window_bounds.top * bpl; 
			i < Height; 
			i++, addr += bpl)
			LineAddress[i] = addr;
		}
		break;
		
		case B_DIRECT_STOP:
			printf("DirectConnect: B_DIRECT_STOP \n");
			fConnected = false;
		break;
	}
	
	locker->Unlock();
//    printf("leaving IXBeLibGraphicsInfo::DirectConnected \n");
#endif
  return true;
}
