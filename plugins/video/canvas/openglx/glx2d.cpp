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
#include "cs2d/openglx/glx2d.h"
#include "cscom/com.h"
#include "csinput/csevent.h"
#include "csinput/csinput.h"
#include "cssys/unix/iunix.h"
#include "csgeom/csrect.h"
#include "isystem.h"
#include "itexture.h"

BEGIN_INTERFACE_TABLE (csGraphics2DGLX)
  IMPLEMENTS_COMPOSITE_INTERFACE_EX (IGraphics2D, XGraphics2D)
  IMPLEMENTS_COMPOSITE_INTERFACE_EX (IGraphicsInfo, XGraphicsInfo)
END_INTERFACE_TABLE ()

IMPLEMENT_UNKNOWN_NODELETE (csGraphics2DGLX)

// csGraphics2DGLX function
csGraphics2DGLX::csGraphics2DGLX (ISystem* piSystem) :
  csGraphics2DGLCommon (piSystem), xim (NULL), cmap (0)
{
  if (FAILED (System->QueryInterface (IID_IUnixSystemDriver, (void**)&UnixSystem)))
  {
    CsPrintf (MSG_FATAL_ERROR, "FATAL: The system driver does not support "
                               "the IUnixSystemDriver interface\n");
    exit (-1);
  }
}

void csGraphics2DGLX::Initialize ()
{
  csGraphics2DGLCommon::Initialize ();
  Screen* screen_ptr;

  // Query system settings
  int sim_depth;
  bool do_shm;
  UnixSystem->GetSettings (sim_depth, do_shm, do_hwmouse);

  dpy = XOpenDisplay (NULL);
  if (!dpy)
  {
    CsPrintf (MSG_FATAL_ERROR, "FATAL: Cannot open X display\n");
    exit (-1);
  }

  screen_num = DefaultScreen (dpy);
  screen_ptr = DefaultScreenOfDisplay (dpy);
  display_width = DisplayWidth (dpy, screen_num);
  display_height = DisplayHeight (dpy, screen_num);

  // Determine visual information.
  Visual* visual = DefaultVisual (dpy, screen_num);

  int desired_attributes[] = { 
  	GLX_RGBA, 
  	GLX_DOUBLEBUFFER, 
	GLX_DEPTH_SIZE, 1, 
	GLX_RED_SIZE, 4,
	GLX_BLUE_SIZE, 4,
	GLX_GREEN_SIZE, 4,
	None };
 
  // find a visual that supports all the features we need
  active_GLVisual = glXChooseVisual(dpy, screen_num, desired_attributes);

  // if a visual was found that we can use, make a graphics context which
  // will be bound to the application window.  If a visual was not
  // found, then try to figure out why it failed
  if (active_GLVisual)
  {
    active_GLContext = glXCreateContext(dpy,active_GLVisual,0,GL_TRUE);
    cmap = XCreateColormap (dpy, RootWindow (dpy, active_GLVisual->screen),
           active_GLVisual->visual, AllocNone);
    visual = active_GLVisual->visual;
  }
  else
  {
    CsPrintf (MSG_FATAL_ERROR, "FATAL: Could not find proper GLX visual\n");
 
    // what attribute was not supplied? we know that trying to get
    // all the attributes at once doesn't work.  provide more user info by
    // trying each of the pieces and seeing if any single piece is not provided 
 
    // try to get a visual with 12 bit color
    int color_attributes[] = { GLX_RGBA,GLX_RED_SIZE,4,GLX_BLUE_SIZE,4,GLX_GREEN_SIZE,4,None};
    if (!glXChooseVisual(dpy, screen_num, color_attributes) )
    {
      CsPrintf(MSG_FATAL_ERROR, "Graphics display does not support at least 12 bit color\n");
    }

    // try to get visual with a depth buffer
    int depthbuffer_attributes[] = {GLX_RGBA, GLX_DEPTH_SIZE,1, None};
    if (!glXChooseVisual(dpy,screen_num,depthbuffer_attributes) )
    {
      CsPrintf(MSG_FATAL_ERROR,"Graphics display does not support a depth buffer\n");
    }

    // try to get a visual with double buffering
    int doublebuffer_attributes[] = {GLX_RGBA, GLX_DOUBLEBUFFER, None};
    if (!glXChooseVisual(dpy,screen_num,doublebuffer_attributes) )
    {
      CsPrintf(MSG_FATAL_ERROR,"Graphics display does not provide double buffering\n");
    }
  }

  pfmt.RedMask = 0xf00;//visual->red_mask;
  pfmt.GreenMask = 0xf0;//visual->green_mask;
  pfmt.BlueMask = 0xf;//visual->blue_mask;
  complete_pixel_format ();
  pfmt.PalEntries = visual->map_entries;
  if (visual->c_class == TrueColor)
    pfmt.PalEntries = 0;
  if (pfmt.PalEntries)
    pfmt.PixelBytes = 1;		// Palette mode
  else
    pfmt.PixelBytes = 2;		// Truecolor mode

  DrawPixel = DrawPixelGL;
  WriteChar = WriteCharGL;
  GetPixelAt = GetPixelAtGL;
  DrawSprite = DrawSpriteGL;
}

csGraphics2DGLX::~csGraphics2DGLX ()
{
  // Destroy your graphic interface
  Close ();
  if (UnixSystem)
    FINAL_RELEASE (UnixSystem);
  CHKB (delete [] Memory);
}

bool csGraphics2DGLX::Open(char *Title)
{
  CsPrintf (MSG_INITIALIZATION, "Video driver GL/X version ");
  if (glXIsDirect(dpy,active_GLContext))
    CsPrintf (MSG_INITIALIZATION, "(direct renderer) ");

  // Set loop callback
  UnixSystem->SetLoopCallback (ProcessEvents, this);

  // Create window
  XSetWindowAttributes winattr;
  winattr.border_pixel = 0;
  winattr.colormap = cmap;
  winattr.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
    FocusChangeMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask;
  window = XCreateWindow(dpy,RootWindow(dpy,active_GLVisual->screen), 0, 0,
    Width,Height, 0 /*border width */,
    active_GLVisual->depth, InputOutput, active_GLVisual->visual,
    CWBorderPixel | CWColormap | CWEventMask, &winattr);
  XMapWindow (dpy, window);
  XStoreName (dpy, window, Title);

  // Create mouse cursors
  XColor Black;
  memset (&Black, 0, sizeof (Black));
  EmptyPixmap = XCreatePixmap (dpy, window, 1, 1, 1);
  EmptyMouseCursor = XCreatePixmapCursor (dpy, EmptyPixmap, EmptyPixmap,
    &Black, &Black, 0, 0);
  MouseCursor [csmcArrow] = XCreateFontCursor (dpy, XC_left_ptr);
//MouseCursor [csmcLens] = XCreateFontCursor (dpy,
  MouseCursor [csmcCross] = XCreateFontCursor (dpy, 33/*XC_crosshair*/);
  MouseCursor [csmcPen] = XCreateFontCursor (dpy, XC_hand2/*XC_pencil*/);
  MouseCursor [csmcMove] = XCreateFontCursor (dpy, XC_fleur);
  /// Diagonal (\) resizing cursor
//MouseCursor [csmcSizeNWSE] = XCreateFontCursor (dpy,
  /// Diagonal (/) resizing cursor
//MouseCursor [csmcSizeNESW] = XCreateFontCursor (dpy,
  /// Vertical sizing cursor
  MouseCursor [csmcSizeNS] = XCreateFontCursor (dpy, XC_sb_v_double_arrow);
  /// Horizontal sizing cursor
  MouseCursor [csmcSizeEW] = XCreateFontCursor (dpy, XC_sb_h_double_arrow);
  /// Invalid operation cursor
//MouseCursor [csmcStop] = XCreateFontCursor (dpy, XC_pirate);
  /// Wait (longplay operation) cursor
  MouseCursor [csmcWait] = XCreateFontCursor (dpy, XC_watch);

  // Wait for expose event
  XEvent event;
  for (;;)
  {
    XNextEvent (dpy, &event);
    if (event.type == Expose)
      break;
  }

  // Create backing store
  if (!xim)
  {
    xim = XGetImage (dpy, window, 0, 0, Width, Height, AllPlanes, ZPixmap);
#   ifdef DO_SHM
    if (do_shm && !XShmQueryExtension (dpy))
    {
      do_shm = false;
      //CsPrintf (MSG_INITIALIZATION, "Shm extension not available on display!\n");
      printf ("Shm extension not available on display!\n");
    }
    if (do_shm)
    {
      shm_image = *xim;
      shmi.shmid = shmget (IPC_PRIVATE, xim->bytes_per_line*xim->height,
                               IPC_CREAT|0777);
      shmi.shmaddr = (char*)shmat (shmi.shmid, 0, 0);
      shmi.readOnly = FALSE;
      XShmAttach (dpy, &shmi);

      // Delete memory segment. The memory stays available until the last client
      // detaches from it.
      XSync (dpy, False);
      shmctl (shmi.shmid, IPC_RMID, 0);

      shm_image.data = shmi.shmaddr;
      Memory = (unsigned char*)shmi.shmaddr;
      shm_image.obdata = (char*)&shmi;
    }
    else
#   endif /* DO_SHM */
    {
      CHK (Memory = new unsigned char[Width*Height*pfmt.PixelBytes]);
    }
  }

  // this makes the context we created in Initialize() the current
  // context, so that all subsequent OpenGL calls will set state and
  // draw stuff on this context.  You could of couse make
  // some more contexts and switch around between several of them...
  // but we use only one here.
  glXMakeCurrent(dpy, window, active_GLContext);

  // Open your graphic interface
  if (!csGraphics2DGLCommon::Open (Title))
    return false;

  Clear (0);
  return true;
}

void csGraphics2DGLX::Close(void)
{
  if (EmptyMouseCursor)
  {
    XFreeCursor (dpy, EmptyMouseCursor);
    EmptyMouseCursor = 0;
    XFreePixmap (dpy, EmptyPixmap);
    EmptyPixmap = 0;
  }
  for (int i = sizeof (MouseCursor) / sizeof (Cursor) - 1; i >= 0; i--)
  {
    if (MouseCursor [i])
      XFreeCursor (dpy, MouseCursor [i]);
    MouseCursor [i] = None;
  }
  if (window)
  {
    XDestroyWindow (dpy, window);
    window = 0;
  }
  // Close your graphic interface
  csGraphics2DGLCommon::Close ();
}


void csGraphics2DGLX::Print (csRect *area)
{
/*  int i = Width*Height;
  while (i > 0)
  {
    *dst++ = (*src & 0xf00) >> 4;
    *dst++ = (*src & 0xf0);
    *dst++ = (*src & 0xf) << 4;
    src++;
    i--;
  }
  glDrawPixels(Width,Height,GL_RGB,GL_UNSIGNED_BYTE,real_Memory);
*/
//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glXSwapBuffers (dpy,window);
  glFlush (); // not needed?
}

bool csGraphics2DGLX::SetMousePosition (int x, int y)
{
  XWarpPointer (dpy, None, window, 0, 0, 0, 0, x, y);
  return true;
}

bool csGraphics2DGLX::SetMouseCursor (int iShape, ITextureHandle* /*iBitmap*/)
{
  if (do_hwmouse
   && (iShape >= 0)
   && (iShape <= csmcWait)
   && (MouseCursor [iShape] != None))
  {
    XDefineCursor (dpy, window, MouseCursor [iShape]);
    return true;
  }
  else
  {
    XDefineCursor (dpy, window, EmptyMouseCursor);
    return (iShape == csmcNone);
  } /* endif */
}

static Bool CheckKeyPress (Display *dpy, XEvent *event, XPointer arg)
{
  XEvent *curevent = (XEvent *)arg;
  if ((event->type == KeyPress)
   && (event->xkey.keycode == curevent->xkey.keycode)
   && (event->xkey.state == curevent->xkey.state))
    return true;
  return false;
}

void csGraphics2DGLX::ProcessEvents (void *Param)
{
  static int button_mapping[6] = {0, 1, 3, 2, 4, 5};
  csGraphics2DGLX *Self = (csGraphics2DGLX *)Param;
  XEvent event;
  int state, key;
  bool down;

  while (XCheckMaskEvent (Self->dpy, ~0, &event))
    switch (event.type)
    {
      case ButtonPress:
        state = ((XButtonEvent*)&event)->state;
        Self->UnixSystem->MouseEvent (button_mapping [event.xbutton.button],
          true, event.xbutton.x, event.xbutton.y,
          ((state & ShiftMask) ? CSMASK_SHIFT : 0) |
	  ((state & Mod1Mask) ? CSMASK_ALT : 0) |
	  ((state & ControlMask) ? CSMASK_CTRL : 0));
          break;
      case ButtonRelease:
        Self->UnixSystem->MouseEvent (button_mapping [event.xbutton.button],
          false, event.xbutton.x, event.xbutton.y, 0);
        break;
      case MotionNotify:
        Self->UnixSystem->MouseEvent (0, false,
	  event.xbutton.x, event.xbutton.y, 0);
        break;
      case KeyPress:
      case KeyRelease:
        // Neat trick: look in event queue if we have KeyPress events ahead
	// with same keycode. If this is the case, discard the KeyUp event
	// in favour of KeyDown since this is most (sure?) an autorepeat
        XCheckIfEvent (event.xkey.display, &event, CheckKeyPress, (XPointer)&event);
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
	  case XK_KP_Up:
	  case XK_KP_8:
          case XK_Up:         key = CSKEY_UP; break;
	  case XK_KP_Down:
	  case XK_KP_2:
          case XK_Down:       key = CSKEY_DOWN; break;
	  case XK_KP_Left:
	  case XK_KP_4:
          case XK_Left:       key = CSKEY_LEFT; break;
	  case XK_KP_Right:
	  case XK_KP_6:
          case XK_Right:      key = CSKEY_RIGHT; break;
          case XK_BackSpace:  key = CSKEY_BACKSPACE; break;
	  case XK_KP_Insert:
	  case XK_KP_0:
          case XK_Insert:     key = CSKEY_INS; break;
	  case XK_KP_Delete:
	  case XK_KP_Decimal:
          case XK_Delete:     key = CSKEY_DEL; break;
	  case XK_KP_Page_Up:
	  case XK_KP_9:
          case XK_Page_Up:    key = CSKEY_PGUP; break;
	  case XK_KP_Page_Down:
	  case XK_KP_3:
          case XK_Page_Down:  key = CSKEY_PGDN; break;
	  case XK_KP_Home:
	  case XK_KP_7:
          case XK_Home:       key = CSKEY_HOME; break;
	  case XK_KP_End:
	  case XK_KP_1:
          case XK_End:        key = CSKEY_END; break;
          case XK_Escape:     key = CSKEY_ESC; break;
          case XK_Tab:        key = CSKEY_TAB; break;
	  case XK_KP_Enter:
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
}
