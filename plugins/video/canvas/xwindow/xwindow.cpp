/*
    Copyright (C) 1998 by Jorrit Tyberghein 
    Copyright (C) 2001 by Samuel Humphreys

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

#define XK_XKB_KEYS
#include <stdarg.h>
#include "cssysdef.h"
#include "xwindow.h"
#include "csgeom/csrect.h"
#include "csutil/cfgacc.h"
#include "csutil/scf.h"
#include "isys/system.h"
#include "isys/plugin.h"
#include "iutil/cfgmgr.h"
#include "iutil/cmdline.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include "video/canvas/common/scancode.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csXWindow)

SCF_EXPORT_CLASS_TABLE (xwin)
  SCF_EXPORT_CLASS (csXWindow, "crystalspace.window.x",
    "X-Window plugin for Crystal Space")
SCF_EXPORT_CLASS_TABLE_END

SCF_IMPLEMENT_IBASE(csXWindow)
  SCF_IMPLEMENTS_INTERFACE(iXWindow)
  SCF_IMPLEMENTS_INTERFACE (iEventPlug)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iEventHandler)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csXWindow::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csXWindow::eiEventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

#define CS_XEXT_XF86VM_SCF_ID "crystalspace.window.x.extf86vm"
#define CS_XEXT_XF86VM "XFree86-VidModeExtension"

csXWindow::csXWindow (iBase* parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiEventHandler);

  EmptyMouseCursor = 0;
  memset (&MouseCursor, 0, sizeof (MouseCursor));
  wm_win = ctx_win = 0;
  win_title = NULL;
  EventOutlet = NULL;

  wm_width = wm_height = 0;

  xvis = NULL;
  gc = 0;
  screen_num = 0;
  Canvas = NULL;

  keyboard_grabbed = 0;
  xf86vm = NULL;
}

void csXWindow::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  iReporter* rep = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (rep)
    rep->ReportV (severity, "crystalspace.window.x", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

csXWindow::~csXWindow ()
{
  if (xf86vm) 
    xf86vm->DecRef ();
  if (EventOutlet)
    EventOutlet->DecRef ();
}

bool csXWindow::Initialize (iObjectRegistry *object_reg)
{
  this->object_reg = object_reg;
  csConfigAccess Config(object_reg, "/config/video.cfg");
  iCommandLineParser* cmdline = CS_QUERY_REGISTRY (object_reg,
						   iCommandLineParser);
  do_hwmouse = Config->GetBool ("Video.SystemMouseCursor", true);
  if (cmdline->GetOption ("sysmouse")) do_hwmouse = true;
  if (cmdline->GetOption ("nosysmouse")) do_hwmouse = false;
  // Open display
  dpy = XOpenDisplay (NULL);

  if (!dpy)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "FATAL: Cannot open X display");
    exit (-1);
  }

  // Set user locale for national character support
  if (XSupportsLocale ())
    XSetLocaleModifiers ("");

  screen_num = DefaultScreen (dpy);

  memset (MouseCursor, 0, sizeof (MouseCursor));

  // Create the event outlet
  iEventQueue* q = CS_QUERY_REGISTRY(object_reg, iEventQueue);
  if (q != 0)
    EventOutlet = q->CreateEventOutlet (this);

  int opcode, first_event, first_error;
  if (XQueryExtension (dpy, CS_XEXT_XF86VM, 
		       &opcode, &first_event, &first_error))
  {
    iPluginManager* plugin_mgr = CS_QUERY_REGISTRY(object_reg, iPluginManager);
    xf86vm = CS_LOAD_PLUGIN (plugin_mgr, 
			     CS_XEXT_XF86VM_SCF_ID, NULL, iXExtF86VM);
  }
  return true;
}


bool csXWindow::Open ()
{
  XSetWindowAttributes swa;
  memset (&swa, 0, sizeof(swa));
//--------------------------------------------------------------------
  unsigned long cw_ctx_mask = (CWOverrideRedirect | 
			       CWBorderPixel | 
			       (cmap ? CWColormap : 0) | 
			       CWEventMask );

  unsigned long cw_wm_mask  = (CWBorderPixel | 
			       (cmap ? CWColormap : 0) | 
			       CWEventMask );

//--------------------------------------------------------------------
  unsigned long swa_ctx_mask = (KeyPressMask | 
				KeyReleaseMask |
				ButtonPressMask | 
				ButtonReleaseMask | 
				PointerMotionMask | 
				ExposureMask | 
				KeymapStateMask );
  
  unsigned long swa_wm_mask = (StructureNotifyMask |
			       FocusChangeMask |
			       KeyPressMask | 
			       KeyReleaseMask |
			       ButtonPressMask | 
			       ButtonReleaseMask | 
			       PointerMotionMask );
  
    
//--------------------------------------------------------------------
  unsigned long si_ctx_mask  = (KeyPressMask | 
				KeyReleaseMask |
				ButtonPressMask | 
				ButtonReleaseMask | 
				PointerMotionMask | 
				ExposureMask | 
				KeymapStateMask );

  unsigned long si_wm_mask  = (StructureNotifyMask | 
			       FocusChangeMask |
			       KeyPressMask | 
			       KeyReleaseMask |
			       ButtonPressMask | 
			       ButtonReleaseMask | 
			       PointerMotionMask );

//--------------------------------------------------------------------

  if (!xvis || !Canvas)
  {
    if (!xvis)
      Report (CS_REPORTER_SEVERITY_ERROR, "No XVisualInfo!");
    if (!Canvas)
      Report (CS_REPORTER_SEVERITY_ERROR, "No Canvas!\n");
    return false;
  }

  // Create window
  swa.colormap = cmap;
  swa.override_redirect = True;
  swa.background_pixel = 0;
  swa.border_pixel = 0;

  swa.event_mask = swa_wm_mask;
  wm_win = XCreateWindow (dpy, 
			  RootWindow (dpy, screen_num), 
			  8, 8, wm_width, wm_height, 
			  4,
			  xvis->depth, 
			  InputOutput, 
			  xvis->visual, 
			  cw_wm_mask, 
			  &swa);

  XStoreName (dpy, wm_win, win_title);
  XSelectInput (dpy, wm_win, si_wm_mask);

  // Intern WM_DELETE_WINDOW and set window manager protocol
  // (Needed to catch user using window manager "delete window" button)
  wm_delete_window = XInternAtom (dpy, "WM_DELETE_WINDOW", False);
  XSetWMProtocols (dpy, wm_win, &wm_delete_window, 1);

  XClassHint *class_hint = XAllocClassHint();
  class_hint->res_name = win_title;
  class_hint->res_class = win_title;
  XmbSetWMProperties (dpy, wm_win,
		      NULL, NULL, NULL, 0, 
		      NULL, NULL, class_hint);

  XFree (class_hint);

  swa.event_mask = swa_ctx_mask;
  ctx_win = XCreateWindow (dpy, 
			   wm_win, 
			   0, 0, wm_width, wm_height, 
			   0, 
			   xvis->depth, 
			   InputOutput, 
			   xvis->visual,
			   cw_ctx_mask,
			   &swa);

  XGCValues values;
  gc = XCreateGC (dpy, ctx_win, 0, &values);
  XSetForeground (dpy, gc, BlackPixel (dpy, screen_num));
  XSetLineAttributes (dpy, gc, 0, LineSolid, CapButt, JoinMiter);
  XSetGraphicsExposures (dpy, gc, False);

  XSelectInput (dpy, ctx_win, si_ctx_mask);
  if (cmap)
    XSetWindowColormap (dpy, ctx_win, cmap);

  // Allow window resizes
  Canvas->AllowResize (true);

  XWMHints wm_hints;
  wm_hints.flags = InputHint | StateHint | WindowGroupHint;
  wm_hints.input = True;
  wm_hints.window_group = wm_win;
  wm_hints.initial_state = NormalState;
  XSetWMHints (dpy, wm_win, &wm_hints);

  Atom wm_client_leader = XInternAtom (dpy, "WM_CLIENT_LEADER", False);
  XChangeProperty (dpy, ctx_win, wm_client_leader, XA_WINDOW, 32,
		   PropModeReplace, (const unsigned char*)&wm_win, 1);
  XmbSetWMProperties (dpy, ctx_win, win_title, win_title,
                      NULL, 0, NULL, NULL, NULL);
  XmbSetWMProperties (dpy, wm_win, win_title, win_title,
                      NULL, 0, NULL, NULL, NULL);
  XMapWindow (dpy, ctx_win);
  XMapRaised (dpy, wm_win);

  // Create a empty mouse cursor
  char zero = 0;
  EmptyPixmap = XCreatePixmapFromBitmapData(dpy, ctx_win, &zero, 1,1,0,0,1);
  XColor Black;
  memset (&Black, 0, sizeof (Black));
  EmptyMouseCursor = XCreatePixmapCursor (dpy, EmptyPixmap, EmptyPixmap,
    &Black, &Black, 0, 0);

  // Create mouse cursors
  MouseCursor [csmcArrow] = XCreateFontCursor (dpy, XC_left_ptr);
//MouseCursor [csmcLens] = XCreateFontCursor (dpy,
  MouseCursor [csmcCross] = XCreateFontCursor (dpy, 33/*XC_crosshair*/);
  MouseCursor [csmcPen] = XCreateFontCursor (dpy, /*XC_hand2*/XC_pencil);
  MouseCursor [csmcMove] = XCreateFontCursor (dpy, XC_fleur);
  /// Diagonal (\) resizing cursor
//MouseCursor [csmcSizeNWSE] = XCreateFontCursor (dpy,
  /// Diagonal (/) resizing cursor
//MouseCursor [csmcSizeNESW] = XCreateFontCursor (dpy,
  /// Vertical sizing cursor
//MouseCursor [csmcSizeNS] = XCreateFontCursor (dpy, XC_sb_v_double_arrow);
  /// Horizontal sizing cursor
//MouseCursor [csmcSizeEW] = XCreateFontCursor (dpy, XC_sb_h_double_arrow);
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

  // Now disable window resizes.
  // Note that if we do this before expose event, with some window managers
  // (e.g. Window Maker) it will be unable to resize the window at all.
  Canvas->AllowResize (false);

  // Tell event queue to call us on every frame
  iEventQueue* q = CS_QUERY_REGISTRY(object_reg, iEventQueue);
  if (q != 0)
    q->RegisterListener (&scfiEventHandler, CSMASK_Nothing);

  if (xf86vm)
  {
    xf86vm->SetWindows (ctx_win, wm_win);
    return xf86vm->Open (dpy, screen_num, xvis, cmap);
  }
  return true;
}

void csXWindow::SetVideoMode (bool full, bool up, bool down)
{
  if (xf86vm)
  {
    int w, h;
    if (!xf86vm->SetFullScreen (full) && !up && !down)
      return;

    if (ctx_win && wm_win)
    {
      if (full && up)
	xf86vm->IncVideoMode ();
      if (full && down)
	xf86vm->DecVideoMode ();

      xf86vm->GetDimensions (w, h);
      if (w != wm_width || h != wm_height)
      {
	wm_width = w;
	wm_height = h;
	if (Canvas->Resize (wm_width, wm_height))
	  XResizeWindow (dpy, ctx_win, wm_width, wm_height);
      }
    }
  }
}

void csXWindow::Close ()
{
  if (xf86vm)
    xf86vm->Close ();

  if (EmptyMouseCursor)
  {
    XFreeCursor (dpy, EmptyMouseCursor);
    EmptyMouseCursor = 0;
    XFreePixmap (dpy, EmptyPixmap);
    EmptyPixmap = 0;
  }
  int i;
  for (i = sizeof (MouseCursor) / sizeof (Cursor) - 1; i >= 0; i--)
  {
    if (MouseCursor [i])
      XFreeCursor (dpy, MouseCursor [i]);
    MouseCursor [i] = None;
  }
  if (ctx_win)
  {
    XDestroyWindow (dpy, ctx_win);
    ctx_win = 0;
  }
  if (wm_win)
  {
    XDestroyWindow (dpy, wm_win);
    wm_win = 0;
  }
}

void csXWindow::SetTitle (const char* title)
{ 
  delete [] win_title;
  win_title = new char [strlen (title) + 1]; 
  strcpy (win_title, title); 
}

void csXWindow::AllowResize (bool iAllow)
{
  XSizeHints normal_hints;
  normal_hints.flags = PMinSize | PMaxSize | PSize | PResizeInc;
  normal_hints.width = wm_width;
  normal_hints.height = wm_height;
  normal_hints.width_inc = 2;
  normal_hints.height_inc = 2;
  if (iAllow)
  {
    normal_hints.min_width = 32;
    normal_hints.min_height = 32;
    normal_hints.max_width = DisplayWidth (dpy, screen_num);
    normal_hints.max_height = DisplayHeight (dpy, screen_num);
  }
  else
  {
    normal_hints.min_width =
    normal_hints.max_width = wm_width;
    normal_hints.min_height =
    normal_hints.max_height = wm_height;
  }
  XSetWMNormalHints (dpy, wm_win, &normal_hints);
  allow_resize = iAllow;
}

void csXWindow::SetCanvas (iGraphics2D *canvas)
{ 
  Canvas = canvas; 
  wm_width = Canvas->GetWidth ();
  wm_height = Canvas->GetHeight ();
}


static Bool CheckKeyPress (Display* /*dpy*/, XEvent *event, XPointer arg)
{
  XEvent *curevent = (XEvent *)arg;
  if ((event->type == KeyPress)
   && (event->xkey.keycode == curevent->xkey.keycode)
   && (event->xkey.state == curevent->xkey.state))
    return true;
  return false;
}

// XCheckMaskEvent() doesn't get ClientMessage Events so use XCheckIfEvent()
// with this Predicate function as a work-around (ClientMessage events
// are needed in order to catch "WM_DELETE_WINDOW")
static Bool AlwaysTruePredicate (Display*, XEvent*, char*)
{
  return True;
}

bool csXWindow::HandleEvent (iEvent &Event)
{
  static int button_mapping[6] = {0, 1, 3, 2, 4, 5};
  XEvent event;
  KeySym key;
  int charcount;
  char charcode [8];
  bool down;
  bool resize = false;

  if ((Event.Type == csevBroadcast)
   && (Event.Command.Code == cscmdCommandLineHelp))
  {
    printf ("Options for X-Window Plugin:\n");
    printf ("  -[no]sysmouse      use/don't use system mouse cursor "
      "(default=%s)\n", do_hwmouse ? "use" : "don't");
    return true;
  }

  while (XCheckIfEvent (dpy, &event, AlwaysTruePredicate, 0))
    switch (event.type)
    {
      case ConfigureNotify:
	if (event.xconfigure.window == wm_win)
 	{
          if (wm_width  != event.xconfigure.width
           || wm_height != event.xconfigure.height)
	  {
	    resize = true;
	    wm_width  = event.xconfigure.width;
	    wm_height = event.xconfigure.height;
	  }
	}
	break;
      case MappingNotify:
        XRefreshKeyboardMapping (&event.xmapping);
	break;
      case ClientMessage:
	if (STATIC_CAST(Atom, event.xclient.data.l[0]) == wm_delete_window)
	{
	  EventOutlet->Broadcast (cscmdContextClose, (iGraphics2D *)this);
	  EventOutlet->Broadcast (cscmdQuit);
	}
	break;
      case ButtonPress:
        EventOutlet->Mouse (button_mapping [event.xbutton.button],
          true, event.xbutton.x, event.xbutton.y);
        break;
      case ButtonRelease:
        EventOutlet->Mouse (button_mapping [event.xbutton.button],
          false, event.xbutton.x, event.xbutton.y);
        break;
      case MotionNotify:
        EventOutlet->Mouse (0, false, event.xbutton.x, event.xbutton.y);
        break;
      case KeyPress:
      case KeyRelease:
        // Neat trick: look in event queue if we have KeyPress events ahead
	// with same keycode. If this is the case, discard the KeyUp event
	// in favour of KeyDown since this is most (sure?) an autorepeat
        XCheckIfEvent (event.xkey.display, &event, CheckKeyPress,
	  (XPointer)&event);
        down = (event.type == KeyPress);
        charcount = XLookupString ((XKeyEvent *)&event, charcode,
				   sizeof (charcode), &key, NULL);
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
#ifdef XK_ISO_Left_Tab
          case XK_ISO_Left_Tab:
#endif
          case XK_KP_Tab:
          case XK_Tab:        key = CSKEY_TAB; break;
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
          case XK_KP_Add:     
	    {
	      if (xf86vm && xf86vm->IsFullScreen () &&
		  down && (event.xkey.state & Mod1Mask))
		SetVideoMode (true, true, false);
	      else
		key = CSKEY_PADPLUS; 
	      break;
	    }
          case XK_KP_Subtract:
	    {
	      if (xf86vm && xf86vm->IsFullScreen () &&
		  down && (event.xkey.state & Mod1Mask))
		SetVideoMode (true, false, true);
	      else
		key = CSKEY_PADMINUS; 
	      break;
	    }
          case XK_KP_Multiply:
	    {
#ifdef CS_DEBUG
	      if (xf86vm && down && event.xkey.state & Mod1Mask)
		SetVideoMode (!xf86vm->IsFullScreen (), false, false);
	      else
#endif
		key = CSKEY_PADMULT; 
	      break;
	    }
          case XK_KP_Divide:  key = CSKEY_PADDIV; break;
          case XK_KP_Begin:   key = CSKEY_CENTER; break;
	  case XK_KP_Enter:
          case XK_Return:
	    {     
#ifndef CS_DEBUG
	      if (xf86vm && down && event.xkey.state & Mod1Mask)
		SetVideoMode (!xf86vm->IsFullScreen (), false, false);
	      else
#endif
		key = CSKEY_ENTER;
	      break;
	    }
          default:            key = (key <= 127) ? ScanCodeToChar [key] : 0;
        }
        if (key)
          EventOutlet->Key(key, charcount == 1 ? uint8(charcode[0]) : 0, down);
        break;
      case FocusIn:
	{
	  EventOutlet->Broadcast (cscmdFocusChanged, 
				  (void *)(event.type == FocusIn));
#ifndef CS_DEBUG
	  if (xf86vm && !keyboard_grabbed && 
	      event.xfocus.window == wm_win) 
	  {
	    XGrabKeyboard (dpy,
			   ctx_win,
			   false,
			   GrabModeAsync,
			   GrabModeAsync,
			   CurrentTime);
	    keyboard_grabbed = 2;
	  }
#endif
	  break;
	}
      case FocusOut:
	{
	  EventOutlet->Broadcast (cscmdFocusChanged, 
				  (void *)(event.type == FocusIn));
#ifndef CS_DEBUG
	  if (xf86vm && keyboard_grabbed && !--keyboard_grabbed) 
	    XUngrabKeyboard (dpy, CurrentTime);
#endif
	  break;
	}
      case Expose:
	if (!resize)
        {
	  csRect rect (event.xexpose.x, event.xexpose.y,
		       event.xexpose.x + event.xexpose.width,
		       event.xexpose.y + event.xexpose.height);
	  Canvas->Print (&rect);
	}
	break;
      default:
        break;
    }

  if (resize)
    if (Canvas->Resize (wm_width, wm_height))
      XResizeWindow (dpy, ctx_win, wm_width, wm_height);
  return false;
}

bool csXWindow::SetMousePosition (int x, int y)
{
  XWarpPointer (dpy, None, ctx_win, 0, 0, 0, 0, x, y);
  return true;
}

bool csXWindow::SetMouseCursor (csMouseCursorID iShape)
{
  if (do_hwmouse
   && (iShape >= 0)
   && (iShape <= csmcWait)
   && (MouseCursor [iShape] != None))
  {
    XDefineCursor (dpy, ctx_win, MouseCursor [iShape]);
    return true;
  }
  else
  {
    XDefineCursor (dpy, ctx_win, EmptyMouseCursor);
    return (iShape == csmcNone);
  } /* endif */
}
