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
#include "cs2d/openglbe/glbe2d.h"
#include "cs3d/opengl/ogl_txtmgr.h"  //dh: is this needed?
#include "cscom/com.h"
#include "csinput/csevent.h"
#include "csinput/csinput.h"
#include "cssys/be/beitf.h"
#include "csgeom/csrect.h"
#include "isystem.h"
#include "itexture.h"
/*
// Used to printf through system driver
void __printfGLBe (int msgtype, char *format, ...)
{
  va_list arg;
  char buf[256];

  va_start (arg, format);
  vsprintf (buf, format, arg);
  va_end (arg);

  csGraphics2DGLBe::System->Print (msgtype, buf);
}
*/	// moved into glcommon2d.h

//ISystem* csGraphics2DGLBe::System = NULL;
//IBeLibSystemDriver* csGraphics2DGLBe::BeSystem = NULL;

BEGIN_INTERFACE_TABLE (csGraphics2DGLBe)
  IMPLEMENTS_COMPOSITE_INTERFACE_EX (IGraphics2D, XGraphics2D)
  IMPLEMENTS_COMPOSITE_INTERFACE_EX (IGraphicsInfo, XGraphicsInfo)
END_INTERFACE_TABLE ()

IMPLEMENT_UNKNOWN_NODELETE (csGraphics2DGLBe)

//csGraphics2DOpenGLFontServer *csGraphics2DGLBe::LocalFontServer = NULL;	//since moved into glcommon2d.h
//OpenGLTextureCache *csGraphics2DGLBe::texture_cache = NULL;// dh:  compiler complains of non-ANSI.  removed to test effect

// csGraphics2DGLBe functions
csGraphics2DGLBe::csGraphics2DGLBe(ISystem* piSystem /*, bool bUses3D*/) :
  csGraphics2DGLCommon (piSystem) //, cmap (0), xim (NULL)
{
	System = piSystem;
	if (FAILED (System->QueryInterface (IID_IBeLibSystemDriver, (void**)&BeSystem)))
	{
		/*__printfGLBe*/CsPrintf (MSG_FATAL_ERROR, "FATAL: The system driver does not support "
                              "the IBeSystemDriver interface\n");
		exit (-1);
	}
}

csGraphics2DGLBe::~csGraphics2DGLBe ()
{
  // Destroy your graphic interface
  Close ();
  if (BeSystem)
    FINAL_RELEASE (BeSystem);
  CHKB (delete [] Memory);
}

void csGraphics2DGLBe::Initialize ()
{
  // call inherited Initialize
  csGraphics2DGLCommon::Initialize ();
/*  
  // Get the system parameters  
  system->GetWidthSetting (Width);
  system->GetHeightSetting (Height);
  system->GetDepthSetting (Depth);
  system->GetFullScreenSetting (FullScreen);

  Font = 0;
  SetClipRect (0, 0, Width, Height);
  pfmt.PalEntries = 256;
  pfmt.PixelBytes = 1;
  */
  // get current screen depth
  curr_color_space = BScreen(B_MAIN_SCREEN_ID).ColorSpace();
  ApplyDepthInfo(curr_color_space);
  curr_page = 0;

/*  
  // Mark all slots in palette as free
  for (int i = 0; i < 256; i++)
  {
    PaletteAlloc[i] = false;
    Palette[i].red = 0;
    Palette[i].green = 0;
    Palette[i].blue = 0;
  }
*/
}

bool csGraphics2DGLBe::Open(char *Title)
{
	// GLX version has init string here.
	CsPrintf (MSG_INITIALIZATION, "Video driver BeGL/Glide version. ");
  
/*	// Open your graphic interface
	if (!csGraphics2D::Open (Title))	//dh: this doesn't do anything for OpenGL!
		return false;*/

	// Query system event handlers
	BeSystem->GetKeyboardHandler (KeyboardHandler, KeyboardHandlerParm);
	BeSystem->GetMouseHandler (MouseHandler, MouseHandlerParm);
	BeSystem->GetFocusHandler (FocusHandler, FocusHandlerParm);
	// Set loop callback
	BeSystem->SetLoopCallback (ProcessEvents, this);

	// Open window
	dpy = CHK (new CrystGLView(BRect(0,0,Width-1,Height-1)));
	window = CHK (new CrystGLWindow(BRect(32,32,Width+32,Height+32), "Crystal",dpy, this));
	
	window->Show();
	if(window->Lock()) {
		dpy->MakeFocus();
		window->Unlock();
	}

	// Open your graphic interface
	// while the inherited Open() implements the fontserver,
	// it also calls the csGraphics2D::Open() which makes a LineAddress array
	// that we don't use.
	if (!csGraphics2DGLCommon::Open (Title))
		return false;

	Clear (0);
	return true;
}

void csGraphics2DGLBe::Close(void)
{
  // Close your graphic interface
  csGraphics2D::Close ();
}

int csGraphics2DGLBe::GetPage ()
{
  return curr_page;
}

bool csGraphics2DGLBe::DoubleBuffer (bool Enable)
{
  return true;
}

bool csGraphics2DGLBe::BeginDraw ()
{
/*	// check that I do have a draw buffer!
	if (Memory == NULL) return false;*/ //dh: removed, no draw buffer in GL
/*	
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
	}*/ //uncomment for conventional DirectConnected
	dpy->LockGL();
	return true;
}

void csGraphics2DGLBe::FinishDraw ()
{
	// unlock 2D driver object
//	window->locker->Unlock();// uncomment for conventional DirectConnected.
	dpy->UnlockGL();
	curr_page = (++curr_page)%2;
}

void csGraphics2DGLBe::Print (csRect *area)
{
/*
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0,Width,0,Height,-1,1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glBegin(GL_LINE_STRIP); glVertex2i(0,0); glVertex2i(20,20); glEnd();
  glRasterPos2f(0.,0.);
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);

  UShort * src = (UShort *)Memory;
  unsigned char* dst = (unsigned char*)real_Memory;
  int i = Width*Height;
  while (i > 0)
  {
    *dst++ = (*src & 0xf00) >> 4;
    *dst++ = (*src & 0xf0);
    *dst++ = (*src & 0xf) << 4;
    src++;
    i--;
  }
  glDrawPixels(Width,Height,GL_RGB,GL_UNSIGNED_BYTE,real_Memory);
//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glXSwapBuffers (dpy,window);
*/
	// draw current buffer
	if(dpy) {
	  dpy->LockGL();
	  dpy->SwapBuffers();
	  dpy->UnlockGL();
	  glFinish ();
//printf("sdwapping\n");
	}
}
/*
void csGraphics2DGLBe::SetRGB(int i, int r, int g, int b)
{
  csGraphics2D::SetRGB (i, r, g, b);
}
*/	//dh: moved into glcommon2d.cpp
void csGraphics2DGLBe::ProcessEvents (void *Param)
{
  csGraphics2DGLBe *Self = (csGraphics2DGLBe *)Param;
printf("Events...\n");
#if 0
  XEvent event;
  int state, key;
  bool down;
  while (XCheckMaskEvent (Self->dpy, ~0, &event))
    switch (event.type)
    {
      case ButtonPress:
        state = ((XButtonEvent*)&event)->state;
        Self->MouseHandler (Self->MouseHandlerParm, ((XButtonEvent*)&event)->button,
          true, ((XButtonEvent*)&event)->x, ((XButtonEvent*)&event)->y,
          (state & ShiftMask ? CSMASK_SHIFT : 0) |
	  (state & Mod1Mask ? CSMASK_ALT : 0) |
	  (state & ControlMask ? CSMASK_CTRL : 0));
          break;
      case ButtonRelease:
        Self->MouseHandler (Self->MouseHandlerParm, ((XButtonEvent*)&event)->button,
          false, ((XButtonEvent*)&event)->x, ((XButtonEvent*)&event)->y, 0);
        break;
      case MotionNotify:
        Self->MouseHandler (Self->MouseHandlerParm, 0,
	  false, ((XButtonEvent*)&event)->x, ((XButtonEvent*)&event)->y, 0);
        break;
      case KeyPress:
      case KeyRelease:
        down = (event.type == KeyPress);
        key = XLookupKeysym ((XKeyEvent*)&event, 0);
        state = ((XKeyEvent*)&event)->state;
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
	Self->KeyboardHandler (Self->KeyboardHandlerParm, key, down);
        break;
      default:
        //if (event.type == CompletionType) shm_busy = 0;
        break;
    }
#endif
}
/*
void csGraphics2DGLBe::setGLColorfromint(int color)
{
  switch (pfmt.PixelBytes)
  {
  case 1: // paletted colors
    glColor3i(Palette[color].red,
    		Palette[color].green,
		Palette[color].blue);
    break;
  case 2: // 16bit color
  case 4: // truecolor
    glColor3f( ( (color & pfmt.RedMask) >> pfmt.RedShift )     / (float)pfmt.RedBits,
               ( (color & pfmt.GreenMask) >> pfmt.GreenShift ) / (float)pfmt.GreenBits,
               ( (color & pfmt.BlueMask) >> pfmt.BlueShift )   / (float)pfmt.BlueBits);
    break;
  }
}

void csGraphics2DGLBe::DrawLine (int x1, int y1, int x2, int y2, int color)
{
  glDisable (GL_TEXTURE_2D);
  glDisable (GL_BLEND);
  glBegin (GL_LINES);
  setGLColorfromint(color);
  glVertex2i (x1, Height-y1-1);
  glVertex2i (x2, Height-y2-1);
  glEnd ();
}

void csGraphics2DGLBe::DrawHorizLine (int x1, int x2, int y, int color)
{
  // the following taken from GLX version
  // prepare for 2D drawing--so we need no fancy GL effects!
  glDisable (GL_TEXTURE_2D);
  glDisable (GL_BLEND);
  glDisable (GL_DEPTH_TEST);
  //glColor3f (1., 1., 1.);
  setGLColorfromint(color);
  glBegin (GL_LINES);
  glVertex2i (x1, Height-y-1);
  glVertex2i (x2, Height-y-1);
  glEnd ();
}

void csGraphics2DGLBe::DrawPixelGL (int x, int y, int color)
{
  // the following taken from GLX version
  // prepare for 2D drawing--so we need no fancy GL effects!
  glDisable (GL_TEXTURE_2D);
  glDisable (GL_BLEND);
  glDisable (GL_DEPTH_TEST);
  //glColor3f (1., 1., 1.);
  setGLColorfromint(color);
  glBegin (GL_POINTS);
  glVertex2i (x, Height-y-1);
  glEnd ();
}

void csGraphics2DGLBe::WriteCharGL (int x, int y, int fg, int bg, char c)
{
  // prepare for 2D drawing--so we need no fancy GL effects!
  glDisable (GL_TEXTURE_2D);
  glDisable (GL_BLEND);
  glDisable (GL_DEPTH_TEST);
  
  setGLColorfromint(fg);

  // FIXME: without the 0.5 shift rounding errors in the
  // openGL renderer can misalign text!
  // maybe we should modify the glOrtho() in glrender to avoid
  // having to does this fractional shift?
  //glRasterPos2i (x, Height-y-1-FontList[Font].Height);
  glRasterPos2f (x+0.5, Height-y-0.5-FontList[Font].Height);

  LocalFontServer->WriteCharacter(c,Font);
}

void csGraphics2DGLBe::DrawSpriteGL (ITextureHandle *hTex, int sx, int sy, int sw, int sh,
    int tx, int ty, int tw, int th)
{
  texture_cache->Add (hTex);

  // cache the texture if we haven't already.
  csTextureMMOpenGL* txt_mm = (csTextureMMOpenGL*)GetcsTextureMMFromITextureHandle (hTex);

  HighColorCache_Data *cachedata;
  cachedata = txt_mm->get_hicolorcache ();
  GLuint texturehandle = *( (GLuint *) (cachedata->pData) );

  glShadeModel(GL_FLAT);
  glEnable(GL_TEXTURE_2D);
  glColor4f(1.,1.,1.,1.);
  if (txt_mm->get_transparent())
  {
    glEnable (GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  }
  else
    glDisable (GL_BLEND);
  glDisable (GL_DEPTH_TEST);
  glBindTexture(GL_TEXTURE_2D,texturehandle);
  
  int bitmapwidth=0, bitmapheight=0;
  hTex->GetBitmapDimensions(bitmapwidth,bitmapheight);

  // convert texture coords given above to normalized (0-1.0) texture coordinates
  float ntx1,nty1,ntx2,nty2;
  ntx1 = tx/bitmapwidth;
  ntx2 = (tx+tw)/bitmapwidth;
  nty1 = ty/bitmapheight;
  nty2 = (ty+th)/bitmapheight;

  // draw the bitmap
  glBegin(GL_TRIANGLE_FAN);
  glTexCoord2f(ntx1,nty1);
  glVertex2i(sx,Height-sy-1);
  glTexCoord2f(ntx2,nty1);
  glVertex2i(sx+sw,Height-sy-1);
  glTexCoord2f(ntx2,nty2);
  glVertex2i(sx+sw,Height-sy-sh-1);
  glTexCoord2f(ntx1,nty2);
  glVertex2i(sx,Height-sy-sh-1);
  glEnd();
}
*/
//unsigned char* csGraphics2DGLBe::GetPixelAtGL (int /*x*/, int /*y*/)
//{
//  return NULL;
//}

void csGraphics2DGLBe::ApplyDepthInfo(color_space this_color_space)
{
  unsigned long RedMask, GreenMask, BlueMask;
  
  // for OpenGL there is only one set.
  DrawPixel = DrawPixelGL;
  WriteChar = WriteCharGL;
  GetPixelAt = GetPixelAtGL;
  DrawSprite = DrawSpriteGL;
  
  switch (this_color_space) {
  	case B_RGB15: 
//			defer bitmap creation
//  		CHK (cryst_bitmap = new BBitmap(BRect(0,0,Width-1,Height-1), B_RGB15));
		Depth	  = 15;
  		RedMask   = 0x1f << 10;
  		GreenMask = 0x1f << 5;
  		BlueMask  = 0x1f;
/*  		
  		DrawPixel = DrawPixel16;
  		WriteChar = WriteChar16;
  		GetPixelAt= GetPixelAt16;
  		DrawSprite= DrawSprite16;
*/  		
  		pfmt.PixelBytes = 2;
  		pfmt.PalEntries = 0;
  		pfmt.RedMask    = RedMask;
  		pfmt.GreenMask  = GreenMask;
  		pfmt.BlueMask   = BlueMask;
  		pfmt.PalEntries = 0;
  		
  		complete_pixel_format();
  		break;
  	case B_RGB16:
//			defer bitmap creation
//  		CHK (cryst_bitmap = new BBitmap(BRect(0,0,Width-1,Height-1), B_RGB16));
  		Depth	  = 16;
  		RedMask   = 0x1f << 11;
  		GreenMask = 0x3f << 5;
  		BlueMask  = 0x1f;
/*  		
  		DrawPixel = DrawPixel16;
  		WriteChar = WriteChar16;
  		GetPixelAt= GetPixelAt16;
  		DrawSprite= DrawSprite16;
*/  		
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
//			defer bitmap creation
//  		CHK (cryst_bitmap = new BBitmap(BRect(0,0,Width-1,Height-1), B_RGB32));
		Depth	  = 32;
  		RedMask   = 0xff << 16;
  		GreenMask = 0xff << 8;
  		BlueMask  = 0xff;
/*  		
  		DrawPixel = DrawPixel32;
  		WriteChar = WriteChar32;
  		GetPixelAt= GetPixelAt32;
  		DrawSprite= DrawSprite32;
*/  		
  		pfmt.PixelBytes = 4;
  		pfmt.PalEntries = 0;
  		pfmt.RedMask    = RedMask;
  		pfmt.GreenMask  = GreenMask;
  		pfmt.BlueMask   = BlueMask;
  		pfmt.PalEntries = 0;
  		
  		complete_pixel_format();
  		break;
  	default:
  	// an unimplemented colorspace, give up and die
  	printf("Unimplemented color depth in Be 2D driver, depth = %i\n", Depth);
  	exit(1);
  		break;
  }
}
  



