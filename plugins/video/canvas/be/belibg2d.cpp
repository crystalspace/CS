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
  color_space curr_color_space;
  unsigned long RedMask, GreenMask, BlueMask;
  
  // this sets system parameters (screen size, "depth", fullscreen mode, default pixelformats)
  csGraphics2D::Initialize();

  // get current screen depth
  curr_color_space = BScreen(B_MAIN_SCREEN_ID).ColorSpace();
  
  // set bitmap depth, overrides the Depth setting in cryst.cfg.
  switch (curr_color_space) {
  	case B_RGB15:
  		Depth = 15;
  		break;
  	case B_RGB16:
  		Depth = 16;
  		break;
  	case B_RGB32:
  	case B_RGBA32:
  		Depth = 32;
  		break;
  	default:
  	// an unimplemented colorspace, go with defaults
  	printf("Unrecognised screen color space, using defaults instead");
  		break;
  }
  
  switch (Depth) {
  	case 15: 
  		CHK (cryst_bitmap = new BBitmap(BRect(0,0,Width-1,Height-1), B_RGB15));
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
  	case 16:
  		CHK (cryst_bitmap = new BBitmap(BRect(0,0,Width-1,Height-1), B_RGB16));
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
  	case 32:
  		CHK (cryst_bitmap = new BBitmap(BRect(0,0,Width-1,Height-1), B_RGB32));
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
  	case 8:
  	case 24:
  	default:
  	// an unimplemented colorspace, give up and die
  	printf("Unimplemented color depth in Be 2D driver, depth = %i\n", Depth);
  	exit(1);
  		break;
  }
}

bool csGraphics2DBeLib::Open(char *Title)
{
	// Open your graphic interface
	if (!csGraphics2D::Open (Title)) return false;
	
	dpy = CHK (new CrystView(BRect(0,0,Width-1,Height-1)));
	window = CHK (new CrystWindow(BRect(32,32,Width+32,Height+32), Title, dpy));
	
	window->Show();
	if(window->Lock()) {
		dpy->MakeFocus();
		window->Unlock();
	}

	Memory = (unsigned char *)cryst_bitmap->Bits();
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
  Clear (0);
  return true;
}

void csGraphics2DBeLib::Close(void)
{
  // Close your graphic interface
  csGraphics2D::Close ();
}

void csGraphics2DBeLib::Print (csRect *area)
{
	if( window->Lock()) {
		dpy->DrawBitmap(cryst_bitmap);
//		dpy->DrawBitmapAsync(cryst_bitmap);
		window->Unlock();
	}
//      XPutImage (dpy, window, gc, xim, 0, 0, 0, 0, Width, Height);
}

void csGraphics2DBeLib::SetRGB(int i, int r, int g, int b)
{
  csGraphics2D::SetRGB (i, r, g, b);
}

  

  


