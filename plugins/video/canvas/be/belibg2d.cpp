/*
    Copyright (C) 1998 by Jorrit Tyberghein
    original Be version written by Xavier Pianet
     modified by David Huen to conform more closely to ddraw version.

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
#include "cssys/be/beitf.h"
#include "cs2d/be/CrystWindow.h"
#include "csgeom/csrect.h"
#include "isystem.h"

BEGIN_INTERFACE_TABLE(csGraphics2DBeLib)
  IMPLEMENTS_COMPOSITE_INTERFACE_EX( IGraphics2D, XGraphics2D )
  IMPLEMENTS_COMPOSITE_INTERFACE_EX( IGraphicsInfo, XGraphicsInfo )
  IMPLEMENTS_COMPOSITE_INTERFACE_EX( IBeLibGraphicsInfo, XBeLibGraphicsInfo )
END_INTERFACE_TABLE()

IMPLEMENT_UNKNOWN_NODELETE(csGraphics2DBeLib)

//	currently no palette stuff

// csGraphics2DXLib functions
csGraphics2DBeLib::csGraphics2DBeLib(ISystem* piSystem, bool bUses3D) : csGraphics2D (piSystem)
{
//  Screen* screen_ptr;
  HRESULT rc;
  IBeLibSystemDriver* piBeLib = NULL;

  rc = piSystem->QueryInterface ((REFIID)IID_IBeLibSystemDriver, (void**)&piBeLib);
  if (FAILED (rc))
  {
    //CsPrintf (MSG_FATAL_ERROR, "csGraphics2DBeLib::Open() -- ISystem does not support IBeLibSystemDriver.");
    printf ("csGraphics2DBeLib::Open() -- ISystem does not support IBeLibSystemDriver.");
    exit (1);
  }
  
  // set up interface flags
  // locker is set up in crystwindow as are these flags
  fConnected = false;
  fConnectionDisabled=false;
  fDrawingThreadSuspended=false;

  //CsPrintf (MSG_INITIALIZATION, "Crystal Space X windows version");
  printf ("Crystal Space BeOS version.\n");

}

csGraphics2DBeLib::~csGraphics2DBeLib(void)
{
  // Destroy your graphic interface
  Close();
  //if (Memory && sim_depth != 0) CHKB (delete [] Memory);
}

void csGraphics2DBeLib::Initialize ()
{
  // this sets system parameters (screen size, "depth", fullscreen mode, default pixelformats)
  csGraphics2D::Initialize();

  // get current screen depth
  curr_color_space = BScreen(B_MAIN_SCREEN_ID).ColorSpace();
  ApplyDepthInfo(curr_color_space);
  // create buffers
  curr_page = 0;
  int i;
  for (i=0; i < NO_OF_BUFFERS; i++)
  	CHK (cryst_bitmap[i] = new BBitmap(BRect(0,0,Width-1,Height-1), curr_color_space));
}

bool csGraphics2DBeLib::Open(char *Title)
{/*
	// This call loads the Be settings of the graphics interface
	curr_color_space = BScreen(B_MAIN_SCREEN_ID).ColorSpace();	
	ApplyDepthInfo(curr_color_space);
*/	
	// This call initializes the LineAddress array and needs Height, Width and pfmt.
	if (!csGraphics2D::Open (Title)) return false;// remove for direct buffer access
	
	dpy = CHK (new CrystView(BRect(0,0,Width-1,Height-1)));
	window = CHK (new CrystWindow(BRect(32,32,Width+32,Height+32), Title, dpy, this));
	
	window->Show();
	if(window->Lock()) {
		dpy->MakeFocus();
		window->Unlock();
	}
	
//	CHK (cryst_bitmap = new BBitmap(BRect(0,0,Width-1,Height-1), curr_color_space));
	Memory = (unsigned char *)cryst_bitmap[curr_page]->Bits();// comment this for direct framebuffer access.
//	for(int i = 0; i < FRAME_HEIGHT; i++)
//		LineAddress [i] = i * cryst_bitmap->BytesPerRow();

//  return 1;
  // Open window
//   = new BWindow (BRect(64, 16, Width, Height), "Yes man", TITLED_WINDOW);

//  attr.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
//  	FocusChangeMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask;
//  XChangeWindowAttributes (dpy, window, CWEventMask, &attr);

  // Create backing store
//  if (!xim)
//  {
//    xim = XGetImage (dpy, window, 0, 0, Width, Height, AllPlanes, ZPixmap);
//    real_Memory = (unsigned char*)(xim->data);

    // If not simulating depth then Memory is equal to real_Memory.
    // If simulating then we allocate a new Memory array in the faked format.
//    if (!sim_depth) Memory = real_Memory;
//    else
//    {
//      CHK (Memory = new unsigned char [Width*Height*pfmt.PixelBytes]);
//    }
//CHK (Memory = new unsigned char [Width*Height*pfmt.PixelBytes]);
//  }
//  Clear (0);// this blows up BDirectWindow as it gets to draw before BDirectWindow is set up!
  return true;
}

void csGraphics2DBeLib::Close(void)
{
  // Close your graphic interface
  csGraphics2D::Close ();
}

bool csGraphics2DBeLib::BeginDraw ()
{
	// check that I do have a draw buffer!
	if (Memory == NULL) return false;
/*	
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
	}*/ //uncomment for direct buffer access
	return true;
}

void csGraphics2DBeLib::FinishDraw ()
{
	// unlock 2D driver object
	locker->Unlock();
}

void csGraphics2DBeLib::Print (csRect *area)
{
//bigtime_t before, after, blit_time;//dhdebug
//before=system_time();
	if( window->Lock()) {
		dpy->Sync();
		dpy->DrawBitmapAsync(cryst_bitmap[curr_page]);
		dpy->Flush();
		
		//advance video page
		curr_page = (++curr_page)%NO_OF_BUFFERS;
		Memory = (unsigned char *)cryst_bitmap[curr_page]->Bits();
		
		window->Unlock();
//after=system_time();
//blit_time=after-before;
//printf("blit time is %i \n", blit_time);
	} //remove this for direct frame buffer access
//      XPutImage (dpy, window, gc, xim, 0, 0, 0, 0, Width, Height);
}

int csGraphics2DBeLib::GetPage ()
{
  return curr_page;
}

void csGraphics2DBeLib::SetRGB(int i, int r, int g, int b)
{
  csGraphics2D::SetRGB (i, r, g, b);
}

void csGraphics2DBeLib::ApplyDepthInfo(color_space this_color_space)
{
  unsigned long RedMask, GreenMask, BlueMask;
  
  switch (this_color_space) {
  	case B_RGB15: 
//			defer bitmap creation
//  		CHK (cryst_bitmap = new BBitmap(BRect(0,0,Width-1,Height-1), B_RGB15));
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
//			defer bitmap creation
//  		CHK (cryst_bitmap = new BBitmap(BRect(0,0,Width-1,Height-1), B_RGB16));
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
//			defer bitmap creation
//  		CHK (cryst_bitmap = new BBitmap(BRect(0,0,Width-1,Height-1), B_RGB32));
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
  	// an unimplemented colorspace, give up and die
  	printf("Unimplemented color depth in Be 2D driver, depth = %i\n", Depth);
  	exit(1);
  		break;
  }
}
  


