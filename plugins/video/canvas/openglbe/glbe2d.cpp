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
#include "cscom/com.h"
#include "csinput/csevent.h"
#include "csinput/csinput.h"
#include "cssys/be/beitf.h"
#include "csgeom/csrect.h"
#include "isystem.h"

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

ISystem* csGraphics2DGLBe::System = NULL;
IBeLibSystemDriver* csGraphics2DGLBe::BeSystem = NULL;

BEGIN_INTERFACE_TABLE (csGraphics2DGLBe)
  IMPLEMENTS_COMPOSITE_INTERFACE_EX (IGraphics2D, XGraphics2D)
  IMPLEMENTS_COMPOSITE_INTERFACE_EX (IGraphicsInfo, XGraphicsInfo)
END_INTERFACE_TABLE ()

IMPLEMENT_UNKNOWN_NODELETE (csGraphics2DGLBe)

// csGraphics2DGLBe functions
csGraphics2DGLBe::csGraphics2DGLBe(ISystem* piSystem, bool bUses3D) :
  csGraphics2D (piSystem) //, cmap (0), xim (NULL)
{
	System = piSystem;
	if (FAILED (System->QueryInterface (IID_IBeLibSystemDriver, (void**)&BeSystem)))
	{
		__printfGLBe (MSG_FATAL_ERROR, "FATAL: The system driver does not support "
                              "the IBeSystemDriver interface\n");
		exit (-1);
	}
	depth=16;

	switch(depth) {
		case 15:
		case 16:
            CHK (cryst_bitmap = new BBitmap(BRect(0,0,Width-1,Height-1),(depth == 16) ? B_RGB16 : B_RGB15));
//								false ? ((depth == 16) ? B_RGB16_BIG : B_RGB15_BIG) :
//									  ((depth == 16) ? B_RGB16 : B_RGB15));
		    // calculate CS's pixel format structure.
		    pfmt.PixelBytes = 2;
		    pfmt.PalEntries = 0;
		    pfmt.RedMask   = (depth == 16) ? 0x1f <<11 : 0x1f <<10;
		    pfmt.GreenMask = (depth == 16) ? 0x3f << 5 : 0x1f << 5;
		    pfmt.BlueMask = 0x1f;
		    complete_pixel_format();
			break;
		case 8 :
		default:
			CHK (cryst_bitmap = new BBitmap(BRect(0,0,Width-1,Height-1),B_CMAP8));
			break;
	}
//  xim = NULL;
  Memory = NULL;

  __printfGLBe (MSG_INITIALIZATION, "Video driver GL/Be version ");
//  if (glGetString (GL_RENDERER))
    __printfGLBe (MSG_INITIALIZATION, "(Renderer v%s) ", "" );
//    __printfGLBe (MSG_INITIALIZATION, "(Renderer v%s) ", "hehe") );
//  if (glGetString (GL_VERSION))
    __printfGLBe (MSG_INITIALIZATION, "(OpenGL v%s)", "hhahah ");
//////    __printfGLBe (MSG_INITIALIZATION, "(OpenGL v%s)", glGetString(GL_VERSION));
  __printfGLBe (MSG_INITIALIZATION, "\n");

  DrawPixel = DrawPixelGL;
  WriteChar = WriteCharGL;
  GetPixelAt = GetPixelAtGL;
  DrawSprite = DrawSpriteGL;
}

csGraphics2DGLBe::~csGraphics2DGLBe ()
{
  // Destroy your graphic interface
  Close ();
  if (BeSystem)
    FINAL_RELEASE (BeSystem);
  CHKB (delete [] Memory);
}

bool csGraphics2DGLBe::Open(char *Title)
{
	// Open your graphic interface
	if (!csGraphics2D::Open (Title))
		return false;

	// Query system event handlers
	BeSystem->GetKeyboardHandler (KeyboardHandler, KeyboardHandlerParm);
	BeSystem->GetMouseHandler (MouseHandler, MouseHandlerParm);
	BeSystem->GetFocusHandler (FocusHandler, FocusHandlerParm);
	// Set loop callback
	BeSystem->SetLoopCallback (ProcessEvents, this);

	// Open window
	dpy = CHK (new CrystGLView(BRect(0,0,Width-1,Height-1)));
	window = CHK (new CrystGLWindow(BRect(32,32,Width+32,Height+32), "Crystal",dpy));
	
	window->Show();
	if(window->Lock()) {
		dpy->MakeFocus();
		window->Unlock();
	}

	Memory = (unsigned char *)cryst_bitmap->Bits();
/*
	// Wait for expose event
	XEvent event;
	for (;;)
	{
		XNextEvent (dpy, &event);
		if (event.type == Expose)
			break;
	}
*/
	Clear (0);
	return true;
}

void csGraphics2DGLBe::Close(void)
{
  // Close your graphic interface
  csGraphics2D::Close ();
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
	if(dpy) {
	  dpy->LockGL();
	  dpy->SwapBuffers();
	  dpy->UnlockGL();
	  glFlush ();
//printf("sdwapping\n");
	}
}

void csGraphics2DGLBe::SetRGB(int i, int r, int g, int b)
{
  csGraphics2D::SetRGB (i, r, g, b);
}

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

void csGraphics2DGLBe::DrawLine (int x1, int y1, int x2, int y2, int color)
{
  glDisable (GL_TEXTURE_2D);
  glDisable (GL_BLEND);
  glBegin (GL_LINES);
  glColor3f (1., 1., 1.);
  glVertex2i (x1, Height-y1-1);
  glVertex2i (x2, Height-y2-1);
  glEnd ();
}

void csGraphics2DGLBe::DrawHorizLine (int x1, int x2, int y, int color)
{
  glDisable (GL_TEXTURE_2D);
  glDisable (GL_BLEND);
  glBegin (GL_LINES);
  glColor3f (1., 1., 1.);
  glVertex2i (x1, Height-y-1);
  glVertex2i (x2, Height-y-1);
  glEnd ();
}

void csGraphics2DGLBe::DrawPixelGL (int x, int y, int color)
{
  glDisable (GL_TEXTURE_2D);
  glDisable (GL_BLEND);
  glBegin (GL_POINTS);
  glColor3f (1., 1., 1.);
  glVertex2i (x, Height-y-1);
  glEnd ();
}

void csGraphics2DGLBe::WriteCharGL (int x, int y, int fg, int bg, char c)
{
  //glDisable (GL_TEXTURE_2D);
  //glDisable (GL_BLEND);
  //glColor3f (1., 1., 1.);
  //glRasterPos2i (x, Height-y-1);
  //glCallLists (1, GL_BYTE, (GLbyte*)&c);

  //glDrawPixels ();
}

void csGraphics2DGLBe::DrawSpriteGL (ITextureHandle *hTex, int sx, int sy, int sw, int sh,
    int tx, int ty, int tw, int th)
{
}

unsigned char* csGraphics2DGLBe::GetPixelAtGL (int /*x*/, int /*y*/)
{
  return NULL;
}

