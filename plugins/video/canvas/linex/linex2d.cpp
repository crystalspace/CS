/*
    Copyright (C) 2000 by Jorrit Tyberghein

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
#include "csutil/scf.h"
#include "cssys/unix/iunix.h"
#include "csinput/csevent.h"
#include "cs2d/linex/linex2d.h"
#include "csutil/csrect.h"
#include "isystem.h"

IMPLEMENT_FACTORY (csGraphics2DLineXLib)

EXPORT_CLASS_TABLE (linex2d)
  EXPORT_CLASS (csGraphics2DLineXLib, "crystalspace.graphics2d.linexlib",
    "X-Windows 2D graphics driver (line3d) for Crystal Space")
EXPORT_CLASS_TABLE_END

IMPLEMENT_IBASE (csGraphics2DLineXLib)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iGraphics2D)
IMPLEMENT_IBASE_END

// csGraphics2DLineXLib functions
csGraphics2DLineXLib::csGraphics2DLineXLib (iBase *iParent) :
  csGraphics2D (), cmap (0)
{
  CONSTRUCT_IBASE (iParent);
  window = 0;
  back = 0;
}

bool csGraphics2DLineXLib::Initialize (iSystem *pSystem)
{
  if (!csGraphics2D::Initialize (pSystem))
    return false;

  UnixSystem = QUERY_INTERFACE (System, iUnixSystemDriver);
  if (!UnixSystem)
  {
    CsPrintf (MSG_FATAL_ERROR, "FATAL: The system driver does not support "
                               "the iUnixSystemDriver interface\n");
    return false;
  }

  Screen* screen_ptr;

  // Open display
  dpy = XOpenDisplay (NULL);

  if (!dpy)
  {
    CsPrintf (MSG_FATAL_ERROR, "FATAL: Cannot open X display\n");
    exit (-1);
  }

  // Query system settings
  int sim_depth;
  bool do_shm;
  UnixSystem->GetExtSettings (sim_depth, do_shm, do_hwmouse);

  screen_num = DefaultScreen (dpy);
  screen_ptr = DefaultScreenOfDisplay (dpy);
  display_width = DisplayWidth (dpy, screen_num);
  display_height = DisplayHeight (dpy, screen_num);

  // Determine visual information.
  // Try in order:
  //   screen depth
  //   24 bit TrueColor
  //   16 bit TrueColor
  //   15 bit TrueColor
  //   8 bit PseudoColor
  int d = DefaultDepthOfScreen(DefaultScreenOfDisplay(dpy));
  if (XMatchVisualInfo(dpy, screen_num, d, (d == 8 ? PseudoColor : TrueColor), &vinfo)
   || XMatchVisualInfo(dpy, screen_num, 24, TrueColor, &vinfo)
   || XMatchVisualInfo(dpy, screen_num, 16, TrueColor, &vinfo)
   || XMatchVisualInfo(dpy, screen_num, 15, TrueColor, &vinfo)
   || XMatchVisualInfo(dpy, screen_num, 8, PseudoColor, &vinfo))
  {
    visual = vinfo.visual;
    // Visual is supposed to be opaque, sorry for breaking in
    vclass = visual->c_class;
  }
  else
  {
    CsPrintf (MSG_FATAL_ERROR, "FATAL: Current screen depth not supported (8, 15, 16 or 32 bpp only)\n");
    exit (1);
  }

  pfmt.RedMask = vinfo.red_mask;
  pfmt.GreenMask = vinfo.green_mask;
  pfmt.BlueMask = vinfo.blue_mask;

  complete_pixel_format ();
  pfmt.PalEntries = vinfo.colormap_size;
  if (vclass == TrueColor)
    pfmt.PalEntries = 0;
  if (vinfo.depth == 24 || vinfo.depth == 32)
    pfmt.PixelBytes = 4;
  else if (pfmt.PalEntries)
    pfmt.PixelBytes = 1;		// Palette mode
  else
    pfmt.PixelBytes = 2;		// Truecolor mode

  // Allocate the palette if not truecolor or if simulating truecolor on 8-bit display.
  if (pfmt.PalEntries)
    cmap = XCreateColormap (dpy, RootWindow (dpy, screen_num), visual, AllocAll);
  else
    cmap = 0;

  Memory = NULL;

  // If in 16-bit mode, redirect drawing routines
  if (pfmt.PixelBytes == 2)
  {
    _DrawPixel = DrawPixel16;
    _WriteChar = WriteChar16;
    _GetPixelAt = GetPixelAt16;
    _DrawSprite = DrawSprite16;
  }
  else if (pfmt.PixelBytes == 4)
  {
    _DrawPixel = DrawPixel32;
    _WriteChar = WriteChar32;
    _GetPixelAt = GetPixelAt32;
    _DrawSprite = DrawSprite32;
  } /* endif */

  memset (MouseCursor, 0, sizeof (MouseCursor));

  return true;
}

csGraphics2DLineXLib::~csGraphics2DLineXLib(void)
{
  Close();
  if (UnixSystem)
    UnixSystem->DecRef ();
}

bool csGraphics2DLineXLib::Open(const char *Title)
{
  CsPrintf (MSG_INITIALIZATION, "Crystal Space X windows driver (Line drawing).\n");
  CsPrintf (MSG_INITIALIZATION, "Using %d bit %sColor visual\n",
              vinfo.depth, (vclass == PseudoColor) ? "Pseudo" : "True");

  // Open your graphic interface
  if (!csGraphics2D::Open (Title))
    return false;

  // Set loop callback
  UnixSystem->SetLoopCallback (ProcessEvents, this);

  // Create window
  XSetWindowAttributes swa;
  swa.background_pixel = 0;
  swa.border_pixel = 0;
  swa.colormap = cmap;
  window = XCreateWindow (dpy, DefaultRootWindow(dpy), 64, 16,
    Width, Height, 4, vinfo.depth, InputOutput, visual,
    CWBackPixel | CWBorderPixel | (cmap ? CWColormap : 0), &swa);
  XMapWindow (dpy, window);
  XStoreName (dpy, window, Title);

  XGCValues values;
  gc = XCreateGC (dpy, window, 0, &values);
  XSetForeground (dpy, gc, BlackPixel (dpy, screen_num));
  XSetLineAttributes (dpy, gc, 0, LineSolid, CapButt, JoinMiter);
  XSetGraphicsExposures (dpy, gc, False);

  XSetWindowAttributes attr;
  attr.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
    FocusChangeMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask;
  XChangeWindowAttributes (dpy, window, CWEventMask, &attr);

  if (cmap)
    XSetWindowColormap (dpy, window, cmap);

  back = XCreatePixmap (dpy, DefaultRootWindow (dpy), Width, Height, vinfo.depth);
  XGCValues values_back;
  gc_back = XCreateGC (dpy, back, 0, &values_back);
  XSetForeground (dpy, gc_back, BlackPixel (dpy, screen_num));
  XSetLineAttributes (dpy, gc_back, 0, LineSolid, CapButt, JoinMiter);

  // Intern WM_DELETE_WINDOW and set window manager protocol
  // (Needed to catch user using window manager "delete window" button)
  wm_delete_window = XInternAtom (dpy, "WM_DELETE_WINDOW", False);
  XSetWMProtocols (dpy, window, &wm_delete_window, 1);

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

  CHK (Memory = new unsigned char [Width*Height*pfmt.PixelBytes]);

  Clear (0);
  return true;
}

void csGraphics2DLineXLib::Close ()
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
  if (back)
  {
    XFreePixmap (dpy, back);
    back = 0;
  }
  if (Memory)
  {
    CHKB (delete [] Memory);
    Memory = NULL;
  }

  csGraphics2D::Close ();
}

struct palent
{
  UShort idx;
  unsigned char r, g, b;
  int cnt;
};

int cmp_palent (const void* p1, const void* p2)
{
  const palent* pe1 = (palent*)p1;
  const palent* pe2 = (palent*)p2;
  if (pe1->cnt < pe2->cnt)
    return 1;
  else if (pe1->cnt > pe2->cnt)
    return -1;
  else
    return 0;
}

int find_rgb_palent (palent* pe, int r, int g, int b)
{
  int i, min, mindist;
  mindist = 1000*256*256;
  min = -1;
  register int red, green, blue, dist;
  for (i = 0 ; i < 256 ; i++)
    if (pe [i].cnt)
    {
      red = r - pe [i].r;
      green = g - pe [i].g;
      blue = b - pe [i].b;
      dist = (299 * red * red) + (587 * green * green) + (114 * blue * blue);
      if (dist == 0)
        return i;
      if (dist < mindist)
      {
        mindist = dist;
	min = i;
      }
    }
    else
      return min;
  return min;
}

bool csGraphics2DLineXLib::BeginDraw ()
{
  nr_segments = 0;
  seg_color = 0;
  return true;
}

void csGraphics2DLineXLib::Print (csRect *area)
{
  usleep (5000);
  if (nr_segments)
  {
    XSetForeground (dpy, gc_back, seg_color);
    XDrawSegments (dpy, back, gc_back, segments, nr_segments);
    nr_segments = 0;
  }

  XCopyArea (dpy, back, window, gc, 0, 0, Width, Height, 0, 0);
}

void csGraphics2DLineXLib::DrawLine (float x1, float y1, float x2, float y2, int color)
{
  if (nr_segments >= 100)
  {
    XSetForeground (dpy, gc_back, seg_color);
    XDrawSegments (dpy, back, gc_back, segments, nr_segments);
    nr_segments = 0;
  }
  else if (seg_color != color)
  {
    XSetForeground (dpy, gc_back, seg_color);
    XDrawSegments (dpy, back, gc_back, segments, nr_segments);
    nr_segments = 0;
    seg_color = color;
  }
  segments[nr_segments].x1 = (int)x1;
  segments[nr_segments].y1 = (int)y1;
  segments[nr_segments].x2 = (int)x2;
  segments[nr_segments].y2 = (int)y2;
  nr_segments++;
}

void csGraphics2DLineXLib::Write (int x, int y, int fg, int bg, const char *text)
{
  XSetForeground (dpy, gc_back, fg);
  XSetBackground (dpy, gc_back, bg);
  XDrawString (dpy, back, gc_back, x, y, text, strlen (text));
}

void csGraphics2DLineXLib::Clear (int color)
{
  XSetForeground (dpy, gc_back, color);
  XSetBackground (dpy, gc_back, color);
  XFillRectangle (dpy, back, gc_back, 0, 0, Width-1, Height-1);
}

void csGraphics2DLineXLib::SetRGB (int i, int r, int g, int b)
{
  // If there is a colormap AND we are not simulating a display with
  // a colormap then we really set the color.
  if (cmap)
  {
    XColor color;
    color.pixel = i;
    color.red = r*256;
    color.green = g*256;
    color.blue = b*256;
    color.flags = DoRed | DoGreen | DoBlue;

    XStoreColor (dpy, cmap, &color);
  }

  csGraphics2D::SetRGB (i, r, g, b);
}

bool csGraphics2DLineXLib::SetMousePosition (int x, int y)
{
  XWarpPointer (dpy, None, window, 0, 0, 0, 0, x, y);
  return true;
}

bool csGraphics2DLineXLib::SetMouseCursor (csMouseCursorID iShape)
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

// XCheckMaskEvent() doesn't get ClientMessage Events so use XCheckIfEvent()
//    with this Predicate function as a work-around ( ClientMessage events
//    are needed in order to catch "WM_DELETE_WINDOW")
static Bool AlwaysTruePredicate (Display*, XEvent*, char*) { return True; }

void csGraphics2DLineXLib::ProcessEvents (void *Param)
{
  static int button_mapping[6] = {0, 1, 3, 2, 4, 5};
  csGraphics2DLineXLib *Self = (csGraphics2DLineXLib *)Param;
  XEvent event;
  int state, key;
  bool down;

  while (XCheckIfEvent (Self->dpy, &event, AlwaysTruePredicate, 0))
    switch (event.type)
    {
      case ClientMessage:
	if (static_cast<Atom>(event.xclient.data.l[0]) == Self->wm_delete_window)
	{
	  Self->System->StartShutdown();
	}
	break;
      case ButtonPress:
        state = ((XButtonEvent*)&event)->state;
        Self->System->QueueMouseEvent (button_mapping [event.xbutton.button],
          true, event.xbutton.x, event.xbutton.y,
          ((state & ShiftMask) ? CSMASK_SHIFT : 0) |
	  ((state & Mod1Mask) ? CSMASK_ALT : 0) |
	  ((state & ControlMask) ? CSMASK_CTRL : 0));
          break;
      case ButtonRelease:
        Self->System->QueueMouseEvent (button_mapping [event.xbutton.button],
          false, event.xbutton.x, event.xbutton.y, 0);
        break;
      case MotionNotify:
        Self->System->QueueMouseEvent (0, false,
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
	Self->System->QueueKeyEvent (key, down);
        break;
      case FocusIn:
      case FocusOut:
        Self->System->QueueFocusEvent (event.type == FocusIn);
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
