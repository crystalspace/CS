/*
    Copyright (C) 1998 by Jorrit Tyberghein

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
#include "sysdef.h"
#include "cs2d/beglide2/glidebe2d.h"
#include "csutil/scf.h"
#include "csinput/csevent.h"
#include "csinput/csinput.h"
#include "cssys/be/beitf.h"
#include "csutil/csrect.h"
#include "cs3d/glide2/glidelib.h"
#include "isystem.h"

IMPLEMENT_FACTORY (csGraphics2DBeGlide)

EXPORT_CLASS_TABLE (glidebe2d)
  EXPORT_CLASS (csGraphics2DBeGlide, "crystalspace.graphics2d.glidebe",
    "BeOS/Glide2 2D graphics driver for Crystal Space")
EXPORT_CLASS_TABLE_END

IMPLEMENT_IBASE (csGraphics2DBeGlide)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iGraphics2D)
IMPLEMENT_IBASE_END

// replace this with config stuff...
bool DoGlideInWindow=true; 

// csGraphics2DGLX functions
csGraphics2DBeGlide::csGraphics2DBeGlide(iBase *iParent) :
  csGraphics2DGlideCommon (iParent)//, xim (NULL), cmap (0)
{
}

bool csGraphics2DBeGlide::Initialize(iSystem *pSystem)
{
  if (!csGraphics2D::Initialize (pSystem))
    return false;

  BeSystem = QUERY_INTERFACE (System, iBeLibSystemDriver);
  if (!BeSystem))
  {
    CsPrintf (MSG_FATAL_ERROR, "FATAL: The system driver does not support "
                               "the iBeSystemDriver interface\n");
    return false;
  }

  // get current screen depth
  curr_color_space = BScreen(B_MAIN_SCREEN_ID).ColorSpace();
  ApplyDepthInfo(curr_color_space);
  curr_page = 0;
  
  CsPrintf (MSG_INITIALIZATION, "Video driver Glide/X version ");
  CsPrintf (MSG_INITIALIZATION, "\n");
 
  GraphicsReady=1;  
  m_DoGlideInWindow = FALSE;
  
  // temporary bitmap
  CHK (cryst_bitmap = new BBitmap(BRect(0,0,Width-1,Height-1), curr_color_space));
  return true;
}

csGraphics2DBeGlide::~csGraphics2DBeGlide () 
{
  // Gotta call inherited destructor here?
  
  // Destroy your graphic interface
  GraphicsReady=0;
  if (BeSystem)
    BeSystem->DecRef ();
}

bool csGraphics2DBeGlide::Open(const char *Title)
{ 

  // Open your graphic interface
  if (!csGraphics2DGlideCommon::Open (Title))
    return false;

  // Query system event handlers
  BeSystem->GetKeyboardHandler (KeyboardHandler, KeyboardHandlerParm);
  BeSystem->GetMouseHandler (MouseHandler, MouseHandlerParm);
  BeSystem->GetFocusHandler (FocusHandler, FocusHandlerParm);
	
  // Set loop callback
  BeSystem->SetLoopCallback (ProcessEvents, this);

  // Open window
  dpy = CHK (new CrystGlideView(BRect(0,0,Width-1,Height-1)));
  window = CHK (new CrystGlideWindow(BRect(32,32,Width+32,Height+32), Title, dpy, this));
	printf ("2d driver. hwnd is %x \n", window);
  window->Show();
  if(window->Lock()) {
	dpy->MakeFocus();
	window->Unlock();
  }	
  
  // temporary bitmap
  BeMemory = (unsigned char *)cryst_bitmap->Bits();
  
  return true;
}

void csGraphics2DBeGlide::Close(void)
{
  // Close your graphic interface
  csGraphics2D::Close ();
}

void csGraphics2DBeGlide::Print (csRect *area)
{
  if (m_DoGlideInWindow)  //temporary
  {
    FXgetImage();
  }
}

#define GR_DRAWBUFFER GR_BUFFER_FRONTBUFFER

bool csGraphics2DBeGlide::BeginDraw(/*int Flag*/)
{
  csGraphics2D::BeginDraw ();
  if (FrameBufferLocked != 1)
    return true;

  FxBool bret;
  lfbInfo.size=sizeof(GrLfbInfo_t);
  
  glDrawMode=GR_LFB_WRITE_ONLY;

  if(locked) FinishDraw();
  
  bret=GlideLib_grLfbLock(glDrawMode|GR_LFB_IDLE,
                          GR_DRAWBUFFER,
                          GR_LFBWRITEMODE_565,
                          GR_ORIGIN_ANY,
                          FXFALSE,
                          &lfbInfo);
  if(bret)
    {
      Memory=(unsigned char*)lfbInfo.lfbPtr;
      if(lfbInfo.origin==GR_ORIGIN_UPPER_LEFT)
        {
          for(int i = 0; i < Height; i++)
            LineAddress [i] = i * lfbInfo.strideInBytes;
        }
      else
        {
          int omi = Height-1;
          for(int i = 0; i < Height; i++)
            LineAddress [i] = (omi--) * lfbInfo.strideInBytes;
        }
      locked=true;
    }
  return bret;

}


// simplification of the Mesa FXgetImage() function
void csGraphics2DBeGlide::FXgetImage()
{

  // we only handle 16bit 
   if (Depth==16) 
   {    
          grLfbReadRegion( GR_BUFFER_FRONTBUFFER,       
                      0, 0,
                      Width, Height,
                      Width * 2,
                      BeMemory);         
   }

   // now put image in window...
	if( window->Lock()) {
//		dpy->Sync();
		dpy->DrawBitmapAsync(cryst_bitmap);
		dpy->Flush();
		
		window->Unlock();
//after=system_time();
//blit_time=after-before;
//printf("blit time is %i \n", blit_time);
	}

}

void csGraphics2DBeGlide::FinishDraw ()
{
  csGraphics2D::FinishDraw ();
  if (FrameBufferLocked)
    return;

  Memory=NULL;
  for (int i = 0; i < Height; i++) LineAddress [i] = 0;
  if (locked) 
    GlideLib_grLfbUnlock(glDrawMode,GR_DRAWBUFFER);
  
  locked = false;
}

void csGraphics2DBeGlide::ProcessEvents (void *Param)
{
//  static int button_mapping[6] = {0, 1, 3, 2, 4, 5};
  csGraphics2DBeGlide *Self = (csGraphics2DBeGlide *)Param;
#if 0
  XEvent event;
  int state, key;
  bool down;

  while (XCheckMaskEvent (Self->dpy, ~0, &event))
    switch (event.type)
    {
      case ButtonPress:
        state = ((XButtonEvent*)&event)->state;
        Self->UnixSystem->MouseEvent (button_mapping[event.xbutton.button],
          true, event.xbutton.x, event.xbutton.y,
          (state & ShiftMask ? CSMASK_SHIFT : 0) |
	  (state & Mod1Mask ? CSMASK_ALT : 0) |
	  (state & ControlMask ? CSMASK_CTRL : 0));
          break;
      case ButtonRelease:
        Self->UnixSystem->MouseEvent (button_mapping [event.xbutton.button],
          false, event.xbutton.x, event.xbutton.y, 0);
        break;
      case MotionNotify:
        Self->UnixSystem->MouseEvent (0, false, event.xbutton.x, event.xbutton.y, 0);
        break;
      case KeyPress:
      case KeyRelease:
        down = (event.type == KeyPress);
        key = XLookupKeysym (&event.xkey, 0);
        state = event.xkey.state;
        switch (key)
        {
          case XK_Meta_L:
	  case XK_Meta_R:
	  case XK_Alt_L:
          case XK_Alt_R:      key = CSKEY_ALT; break;
          case XK_Control_L:
          case XK_Control_R:  key = CSKEY_CTRL; break;
          case XK_Shift_L:
          case XK_Shift_R:    key = CSKEY_SHIFT; break;
          case XK_Up:         key = CSKEY_UP; break;
          case XK_Down:       key = CSKEY_DOWN; break;
          case XK_Left:       key = CSKEY_LEFT; break;
          case XK_Right:      key = CSKEY_RIGHT; break;
          case XK_BackSpace:  key = CSKEY_BACKSPACE; break;
          case XK_Insert:     key = CSKEY_INS; break;
          case XK_Delete:     key = CSKEY_DEL; break;
          case XK_Page_Up:    key = CSKEY_PGUP; break;
          case XK_Page_Down:  key = CSKEY_PGDN; break;
          case XK_Home:       key = CSKEY_HOME; break;
          case XK_End:        key = CSKEY_END; break;
          case XK_Escape:     key = CSKEY_ESC; break;
          case XK_Tab:        key = CSKEY_TAB; break;
          case XK_Return:     key = CSKEY_ENTER; break;
          case XK_F1:         key = CSKEY_F1; break;
          case XK_F2:         key = CSKEY_F2; break;
          case XK_F3:         key = CSKEY_F3; break;
          case XK_F4:         key = CSKEY_F4; break;
          case XK_F5:         key = CSKEY_F5; break;
          case XK_F6:         key = CSKEY_F6; break;
          case XK_F7:         key = CSKEY_F7; break;
          case XK_F8:         key = CSKEY_F8; break;
          case XK_F9:         key = CSKEY_F9; break;
          case XK_F10:        key = CSKEY_F10; break;
          case XK_F11:        key = CSKEY_F11; break;
          case XK_F12:        key = CSKEY_F12; break;
          default:            break;
        }
	Self->UnixSystem->KeyboardEvent (key, down);
        break;
      case FocusIn:
      case FocusOut:
        Self->UnixSystem->FocusEvent (event.type == FocusIn);
        break;
      case Expose:
      {
        csRect rect (event.xexpose.x, event.xexpose.y,
	  event.xexpose.x + event.xexpose.width, event.xexpose.y + event.xexpose.height);
	Self->Print (&rect);
	break;
      }
      default:
        //if (event.type == CompletionType) shm_busy = 0;
        break;
    }
#endif
}
/*
void csGraphics2DBeGlide::DrawLine (int x1, int y1, int x2, int y2, int color)
{
  // can't do this while framebuffer is locked...
  if (locked) return;
 
  GrVertex a,b;
  a.x=x1; a.y=y1;
  b.x=x2; b.y=y2;

  grConstantColorValue(color);
  grDrawLine(&a,&b);
}

void csGraphics2DBeGlide::DrawPixelGlide (int x, int y, int color)
{
  // can't do this while framebuffer is locked...
  if (locked) return;

  GrVertex p;
  p.x=x; p.y=y;

  grConstantColorValue(color);
  grDrawPoint(&p);
}

void csGraphics2DBeGlide::WriteCharGlide (int x, int y, int fg, int bg, char c)
{
  // not implemented yet...
}

void csGraphics2DBeGlide::DrawSpriteGlide (iTextureHandle *hTex, int sx, int sy,
  int sw, int sh, int tx, int ty, int tw, int th)
{
  // not implemented yet...
}

unsigned char* csGraphics2DBeGlide::GetPixelAtGlide (int x, int y)
{
  // not implemented yet...
  return NULL;
}
*/
void csGraphics2DBeGlide::ApplyDepthInfo(color_space this_color_space)
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
//			defer bitmap creation
//  		CHK (cryst_bitmap = new BBitmap(BRect(0,0,Width-1,Height-1), B_RGB16));
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
//			defer bitmap creation
//  		CHK (cryst_bitmap = new BBitmap(BRect(0,0,Width-1,Height-1), B_RGB32));
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
  	// an unimplemented colorspace, give up and die
  	printf("Unimplemented color depth in Be 2D driver, depth = %i\n", Depth);
  	exit(1);
  		break;
  }
}
