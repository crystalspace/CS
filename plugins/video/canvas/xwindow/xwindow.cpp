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
#include "csutil/sysfunc.h"
#include "csutil/event.h"
#include "csutil/eventhandlers.h"
#include "xwindow.h"
#include "csgeom/csrect.h"
#include "csgfx/bakekeycolor.h"
#include "csgfx/imageautoconvert.h"
#include "csgfx/imagemanipulate.h"
#include "csutil/callstack.h"
#include "csutil/cfgacc.h"
#include "csutil/csuctransform.h"
#include "csutil/eventnames.h"
#include "csutil/scf.h"
#include "iutil/plugin.h"
#include "iutil/cfgmgr.h"
#include "iutil/cmdline.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include "igraphic/image.h"
#include "csplugincommon/canvas/cursorconvert.h"

#include "MwmUtil.h"

#ifdef HAVE_XCURSOR
#include <X11/Xcursor/Xcursor.h>
#endif
#ifdef HAVE_XRENDER
#include <X11/extensions/Xrender.h>
#endif

// Define this if you want keyboard-grabbing behavior enabled.  For now it is
// disabled by default.  In the future, we should probably provide an API for
// setting this at run-time (though this API, when properly generalized to be
// platform-neutral, does not belong in iGraphics2D; but rather in some input-
// or keyboard-related interface).
#undef CS_XWIN_GRAB_KEYBOARD



SCF_IMPLEMENT_FACTORY (csXWindow)

#define CS_XEXT_XF86VM_SCF_ID "crystalspace.window.x.extf86vm"
#define CS_XEXT_XF86VM "XFree86-VidModeExtension"

csXWindow::csXWindow (iBase* parent) : scfImplementationType (this, parent),
  keyboardIM (0), keyboardIC (0),
  hwMouse (csGraphics2D::hwmcOff),
  haveRGBAcursors (false)
{
  EmptyMouseCursor = 0;
  memset (&MouseCursor, 0, sizeof (MouseCursor));
  wm_win = ctx_win = 0;
  EventOutlet = 0;

  wm_width = wm_height = 0;

  dpy = 0;
  xvis = 0;
  gc = 0;
  screen_num = 0;
  Canvas = 0;

  keyboard_grabbed = 0;
  oldErrorHandler = 0;
}

void csXWindow::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csReportV (object_reg, severity, "crystalspace.window.x", msg, arg);
  va_end (arg);
}

csXWindow::~csXWindow ()
{
  if (scfiEventHandler)
  {
    csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (object_reg));
    if (q != 0)
      q->RemoveListener (scfiEventHandler);
  }

  if (oldErrorHandler != 0)
  {
    XSetErrorHandler (oldErrorHandler);
  }
}

static int MyErrorHandler (Display* dpy, XErrorEvent* ev)
{
  char errorText[256];
  XGetErrorText (dpy, ev->error_code, errorText, sizeof (errorText));
  csFPrintf (stderr, "X error!\n");
  csFPrintf (stderr, " type:         %d\n", ev->type);
  csFPrintf (stderr, " serial:       %ld\n", ev->serial);
  csFPrintf (stderr, " error code:   %d (%s)\n", ev->error_code, errorText);
  csFPrintf (stderr, " request code: %d\n", ev->request_code);
  csFPrintf (stderr, " minor code:   %d\n", ev->minor_code);
  csFPrintf (stderr, " resource:     %jx\n", (uintmax_t)ev->resourceid);
  fflush (stderr);
  
  csCallStack* stack = csCallStackHelper::CreateCallStack (1);
  if (stack != 0)
  {
    stack->Print (stderr);
    fflush (stderr);
    stack->Free ();
  }
#ifdef CS_DEBUG
  CS_DEBUG_BREAK;
#else
  abort ();
#endif
  return 0;
}

bool csXWindow::Initialize (iObjectRegistry *object_reg)
{
  this->object_reg = object_reg;
  this->name_reg = csEventNameRegistry::GetRegistry (object_reg);
  csConfigAccess Config(object_reg, "/config/video.cfg");
  // Open display
  dpy = XOpenDisplay (0);

  if (!dpy)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "FATAL: Cannot open X display");
    return false;
  }
  
  bool Xsynchronous = Config->GetBool ("Video.X.Sync", false);
  if (Xsynchronous)
  {
    XSynchronize (dpy, True);
    oldErrorHandler = XSetErrorHandler (MyErrorHandler);
  }

  // Set user locale for national character support
  if (XSupportsLocale ())
    XSetLocaleModifiers ("");

  screen_num = DefaultScreen (dpy);

  memset (MouseCursor, 0, sizeof (MouseCursor));

  // Create the event outlet
  csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (object_reg));
  if (q != 0)
    EventOutlet = q->CreateEventOutlet (this);

  int opcode, first_event, first_error;
  if (XQueryExtension (dpy, CS_XEXT_XF86VM, &opcode,
                       &first_event, &first_error))
  {
    csRef<iPluginManager> plugin_mgr (
    	csQueryRegistry<iPluginManager> (object_reg));
    xf86vm = csLoadPlugin<iXExtF86VM> (plugin_mgr, CS_XEXT_XF86VM_SCF_ID);
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
			       CWBackPixel |
			       (cmap ? CWColormap : 0) |
			       CWEventMask );

  unsigned long cw_wm_mask  = (CWBorderPixel |
			       CWBackPixel |
			       (cmap ? CWColormap : 0) |
			       CWEventMask );

//--------------------------------------------------------------------
  unsigned long swa_ctx_mask = (KeyPressMask |
				KeyReleaseMask |
				ButtonPressMask |
				ButtonReleaseMask |
				PointerMotionMask |
				ExposureMask |
				VisibilityChangeMask |
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
				VisibilityChangeMask |
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
      Report (CS_REPORTER_SEVERITY_ERROR, "No Canvas!");
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

  xaMotifWmHints = XInternAtom (dpy, _XA_MOTIF_WM_HINTS, False);

  // Intern WM_DELETE_WINDOW and set window manager protocol
  // (Needed to catch user using window manager "delete window" button)
  wm_delete_window = XInternAtom (dpy, "WM_DELETE_WINDOW", False);
  XSetWMProtocols (dpy, wm_win, &wm_delete_window, 1);

  XClassHint *class_hint = XAllocClassHint ();
  class_hint->res_name = const_cast<char*> (win_title.GetData());
  class_hint->res_class = const_cast<char*> (win_title.GetData());
  XmbSetWMProperties (dpy, wm_win, 0, 0, 0, 0, 0, 0, class_hint);

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

  // Set up keyboard input method/context
  keyboardIM = XOpenIM (dpy, 0, 0, 0);
  if (keyboardIM == 0)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "XOpenIM() failed");
    return false;
  }
  keyboardIC = XCreateIC (keyboardIM, 
			  XNClientWindow, ctx_win,
			  XNFocusWindow, ctx_win,
			  XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
			  (void*)0);
  if (keyboardIC == 0)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "XCreateIC() failed");
    return false;
  }

  XGCValues values;
  gc = XCreateGC (dpy, ctx_win, 0, &values);
  XSetForeground (dpy, gc, BlackPixel (dpy, screen_num));
  XSetLineAttributes (dpy, gc, 0, LineSolid, CapButt, JoinMiter);
  XSetGraphicsExposures (dpy, gc, False);

  unsigned long filterEvents = 0;
  XGetICValues (keyboardIC, XNFilterEvents, &filterEvents, (void*)0);
  XSelectInput (dpy, ctx_win, si_ctx_mask | filterEvents);
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
    0, 0, 0, 0, 0);
  XmbSetWMProperties (dpy, wm_win, win_title, win_title,
    0, 0, 0, 0, 0);
  XMapWindow (dpy, ctx_win);
  XMapRaised (dpy, wm_win);

  // Create a empty mouse cursor
  char zero = 0;
  EmptyPixmap = XCreatePixmapFromBitmapData (dpy, ctx_win, &zero, 1,
    1, 0, 0, 1);
  XColor Black;
  memset (&Black, 0, sizeof (Black));
  EmptyMouseCursor = XCreatePixmapCursor (dpy, EmptyPixmap, EmptyPixmap,
    &Black, &Black, 0, 0);
    
#ifdef HAVE_XRENDER
  {
    int eventBase, errorBase;
    int major, minor;
    haveRGBAcursors = XRenderQueryExtension (dpy, &eventBase, &errorBase)
      && XRenderQueryVersion (dpy, &major, &minor)
      && ((major > 0) || (minor >= 5));
  }
#endif

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
  if (!scfiEventHandler)
    scfiEventHandler.AttachNew (new EventHandler (this));
  csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (object_reg));
  if (q != 0)
  {
    csEventID events[3] = { csevFrame(name_reg), csevCommandLineHelp(name_reg), 
      CS_EVENTLIST_END };
    q->RegisterListener (scfiEventHandler, events);
  }

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
  else
  {
    if (Canvas)
      Report (CS_REPORTER_SEVERITY_WARNING,
        "Unable to set video mode: xf86vm extension support was not found!");
    else
      csPrintf ("WARNING: Unable to set video mode: xf86vm extension support was not found!\n");
  }
}

void csXWindow::Close ()
{
  if (xf86vm)
    xf86vm->Close ();
  
  if (keyboardIC) XDestroyIC (keyboardIC);
  if (keyboardIM) XCloseIM (keyboardIM);

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

  cachedCursors.DeleteAll ();
  
  if (ctx_win)
  {
    XFreeGC (dpy, gc);
    XDestroyWindow (dpy, ctx_win);
    ctx_win = 0;
  }

  if (wm_win)
  {
    XDestroyWindow (dpy, wm_win);
    wm_win = 0;
  }
  XSync (dpy, true);
}

void csXWindow::SetIcon (iImage *image)
{
  if (!dpy || !wm_win) return;
  //check and prepare the data we will work on
  if (!image) { return; }
  
  static const int tooLargeSize = 48;
  
  csRefArray<iImage> iconImages;
  {
    CS::ImageAutoConvert imageRGBA (image, CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA);
    iconImages.Push (imageRGBA);
    /* Apparently not all Window Managers like icons of all sizes
       (eg Compiz doesn't display icons larger than 48x48 in some places)
       - so also provide a scaled down version of the icon */
    {
      int imgW = imageRGBA->GetWidth();
      int imgH = imageRGBA->GetHeight();
      if ((imgW > tooLargeSize) || (imgH > tooLargeSize))
      {
	int newW, newH;
	if (imgW > imgH)
	{
	  newW = tooLargeSize;
	  newH = int ((float(imgH)/float(imgW)) * tooLargeSize);
	}
	else
	{
	  newW = int ((float(imgW)/float(imgH)) * tooLargeSize);
	  newH = tooLargeSize;
	}
	iconImages.Insert (0,
	  csImageManipulate::Rescale (imageRGBA, newW, newH));
      }
    }
  }
  

  /*
    Convert image data into format expected by _NET_WM_ICON.

    From the spec (http://standards.freedesktop.org/wm-spec/latest/ar01s05.html):
    "This is an array of 32bit packed CARDINAL ARGB with high byte being A, 
    low byte being B. The first two cardinals are width, height. Data is in rows, 
    left to right and top to bottom. "
   */
  size_t iconDataSize = 0;
  for (size_t i = 0; i < iconImages.GetSize(); i++)
  {
    iconDataSize += 2 + iconImages[i]->GetWidth()*iconImages[i]->GetHeight();
  }
  unsigned long* iconData = (unsigned long*)cs_malloc (iconDataSize * sizeof (unsigned long));
  unsigned long* iconDataPtr = iconData;
  for (size_t i = 0; i < iconImages.GetSize(); i++)
  {
    iImage* img = iconImages[i];
    int imgW = img->GetWidth();
    int imgH = img->GetHeight();
    
    iconDataPtr[0] = imgW;
    iconDataPtr[1] = imgH;
    const csRGBpixel* src = (csRGBpixel*)img->GetImageData();
    unsigned long* dst = iconDataPtr+2;
    for (size_t nPixels = imgW*imgH; nPixels-- > 0; )
    {
      *dst =
	src->blue | (src->green << 8) | (src->red << 16) | (src->alpha << 24);
      src++;
      dst++;
    }
    iconDataPtr = dst;
  }
  
  Atom iconAtom = XInternAtom (dpy, "_NET_WM_ICON", False);
  XChangeProperty (dpy, wm_win, iconAtom, XA_CARDINAL, 32,
    PropModeReplace, (unsigned char*)iconData,
    iconDataSize);  
  
  cs_free (iconData);
}

void csXWindow::SetTitle (const char* title)
{
  win_title = title;
  if (dpy && wm_win)   
    XStoreName (dpy, wm_win, win_title);
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
  if (wm_win) XSetWMNormalHints (dpy, wm_win, &normal_hints);
  allow_resize = iAllow;
}

void csXWindow::Resize (int w, int h)
{
  if ((wm_width == w) && (wm_height == h)) return;
  wm_width = w;
  wm_height = h;
  if (ctx_win) XResizeWindow (dpy, ctx_win, w, h);
  AllowResize (allow_resize); // updates size in window hints
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

static int TranslateXMouseButton (int xbutton)
{
  /* [res] Mappings were determined July 2010 with Xorg-server 1.7
     It seems that with Xorg 7.0 the buttons 6/7 where changed to be wheel
     buttons as well. Prior to that, these buttons were the "extra" (thumb)
     buttons instead.
  */
  static int buttonMapping[] =
  {
    csmbNone,
    csmbLeft, csmbMiddle, csmbRight,	// Standard buttons
    csmbWheelUp, csmbWheelDown,		// Wheel, vertical (the common kind)
    csmbHWheelLeft, csmbHWheelRight,	// Wheel, horizontal (the rare kind)
    csmbExtra1, csmbExtra2		// Thumb buttons
  };
  const int buttonMapCount =
    sizeof (buttonMapping) / sizeof (buttonMapping[0]);
  
  if (xbutton < buttonMapCount)
    return buttonMapping[xbutton];
  else
    return xbutton-1; // Since mouse buttons are 0-based
}

static utf32_char KeyEventCharacter (XIC ic, XKeyEvent* keyEvent)
{
  utf32_char ch[2] = {0, 0};
  
#if defined(X_HAVE_UTF8_STRING)
  char charcode [CS_UC_MAX_UTF8_ENCODED];
  int charcount;
  charcount = Xutf8LookupString (ic, keyEvent, charcode, sizeof (charcode), 0, 0);
  if (charcount > 0)
    csUnicodeTransform::UTF8to32 (ch, sizeof (ch)/sizeof (utf32_char),
				  (utf8_char*)charcode, charcount);
#else
  wchar_t charcode [4];
  int charcount;
  charcount = XwcLookupString (ic, keyEvent, charcode, sizeof (charcode), 0, 0);
  if (charcount == 1)
    ch[0] = charcode[0];
#endif
  return ch[0];
}

bool csXWindow::HandleEvent (iEvent &Event)
{
  XEvent event;
  KeySym xKey;
  utf32_char csKeyRaw = 0, csKeyCooked = 0;
  bool prevdown, down;
  bool resize = false;

  while (XCheckIfEvent (dpy, &event, AlwaysTruePredicate, 0))
  {
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
        if (static_cast<Atom> (event.xclient.data.l[0]) == wm_delete_window)
        {
          EventOutlet->Broadcast (csevCanvasClose (name_reg, Canvas),
            (intptr_t)this);
          EventOutlet->Broadcast (csevQuit (name_reg));
        }
        break;
      case ButtonPress:
        EventOutlet->Mouse (TranslateXMouseButton (event.xbutton.button),
          true, event.xbutton.x, event.xbutton.y);
        break;
      case ButtonRelease:
        EventOutlet->Mouse (TranslateXMouseButton (event.xbutton.button),
          false, event.xbutton.x, event.xbutton.y);
        break;
      case MotionNotify:
        EventOutlet->Mouse (csmbNone, false, event.xbutton.x, event.xbutton.y);
        break;
      case KeyPress:
      case KeyRelease:
        // Neat trick: look in event queue if we have KeyPress events ahead
        // with same keycode. If this is the case, discard the KeyUp event
        // in favour of KeyDown since this is most (sure?) an autorepeat
        prevdown = (event.type == KeyPress);
        XCheckIfEvent (event.xkey.display, &event, CheckKeyPress,
          (XPointer)&event);
        down = (event.type == KeyPress);
	/* Use XLookupString() instead of XLookupKeysym() as the former does
	   some extra handling of NumLock keys */
	XLookupString ((XKeyEvent *)&event, 0, 0, &xKey, 0);
#define HANDLE_MAP_KEY(rawCode, cookedCode) 		\
	{						\
          csKeyRaw = rawCode;			       	\
	  if (cookedCode != 0) 				\
	    csKeyCooked = cookedCode;			\
	  else						\
	  {						\
	    XKeyEvent myEvent (*((XKeyEvent *)&event));	\
	    myEvent.type = KeyPress;			\
	    csKeyCooked = 				\
	      KeyEventCharacter (keyboardIC, &myEvent);	\
	  }						\
	}
#define MAP_KEY(xKey, rawCode, cookedCode) 		\
      case xKey:		                       	\
        HANDLE_MAP_KEY(rawCode, cookedCode);          	\
        break
    
        switch (xKey)
        {
          MAP_KEY (XK_Meta_L, CSKEY_ALT_LEFT, CSKEY_ALT);
          MAP_KEY (XK_Meta_R, CSKEY_ALT_RIGHT, CSKEY_ALT);
      
          MAP_KEY (XK_Alt_L, CSKEY_ALT_LEFT, CSKEY_ALT);
          MAP_KEY (XK_Alt_R, CSKEY_ALT_RIGHT, CSKEY_ALT);
      
          MAP_KEY (XK_Control_L, CSKEY_CTRL_LEFT, CSKEY_CTRL);
          MAP_KEY (XK_Control_R, CSKEY_CTRL_RIGHT, CSKEY_CTRL);
      
          MAP_KEY (XK_Shift_L, CSKEY_SHIFT_LEFT, CSKEY_SHIFT);
          MAP_KEY (XK_Shift_R, CSKEY_SHIFT_RIGHT, CSKEY_SHIFT);
      
          MAP_KEY (XK_KP_Up, CSKEY_PAD8, CSKEY_UP);
          MAP_KEY (XK_KP_8, CSKEY_PAD8, 0);
          MAP_KEY (XK_Up, CSKEY_UP, CSKEY_UP);
      
          MAP_KEY (XK_KP_Down, CSKEY_PAD2, CSKEY_DOWN);
          MAP_KEY (XK_KP_2, CSKEY_PAD2, 0);
          MAP_KEY (XK_Down, CSKEY_DOWN, CSKEY_DOWN);
      
          MAP_KEY (XK_KP_Left, CSKEY_PAD4, CSKEY_LEFT);
          MAP_KEY (XK_KP_4, CSKEY_PAD4, 0);
          MAP_KEY (XK_Left, CSKEY_LEFT, CSKEY_LEFT);
      
          MAP_KEY (XK_KP_Right, CSKEY_PAD6, CSKEY_RIGHT);
          MAP_KEY (XK_KP_6, CSKEY_PAD6, 0);
          MAP_KEY (XK_Right, CSKEY_RIGHT, CSKEY_RIGHT);
      
          MAP_KEY (XK_BackSpace, CSKEY_BACKSPACE, CSKEY_BACKSPACE);
      
          MAP_KEY (XK_KP_Insert, CSKEY_PAD0, CSKEY_INS);
          MAP_KEY (XK_KP_0, CSKEY_PAD0, 0);
          MAP_KEY (XK_Insert, CSKEY_INS, CSKEY_INS);
    
          MAP_KEY (XK_KP_Delete, CSKEY_PADDECIMAL, CSKEY_DEL);
          MAP_KEY (XK_KP_Decimal, CSKEY_PADDECIMAL, 0); 
          MAP_KEY (XK_Delete, CSKEY_DEL, CSKEY_DEL);
      
          MAP_KEY (XK_KP_Page_Up, CSKEY_PAD9, CSKEY_PGUP);
          MAP_KEY (XK_KP_9, CSKEY_PAD9, 0);
          MAP_KEY (XK_Page_Up, CSKEY_PGUP, CSKEY_PGUP);
      
          MAP_KEY (XK_KP_Page_Down, CSKEY_PAD3, CSKEY_PGDN);
          MAP_KEY (XK_KP_3, CSKEY_PAD3, 0);
          MAP_KEY (XK_Page_Down, CSKEY_PGDN, CSKEY_PGDN);
      
          MAP_KEY (XK_KP_Home, CSKEY_PAD7, CSKEY_HOME);
          MAP_KEY (XK_KP_7, CSKEY_PAD7, 0);
          MAP_KEY (XK_Home, CSKEY_HOME, CSKEY_HOME);
    
          MAP_KEY (XK_KP_End, CSKEY_PAD1, CSKEY_END);
          MAP_KEY (XK_KP_1, CSKEY_PAD1, 0);
          MAP_KEY (XK_End, CSKEY_END, CSKEY_END);

          MAP_KEY (XK_Escape, CSKEY_ESC, CSKEY_ESC);
#ifdef XK_ISO_Left_Tab
          MAP_KEY (XK_ISO_Left_Tab, CSKEY_TAB, CSKEY_TAB);
#endif
          MAP_KEY (XK_KP_Tab, CSKEY_TAB, CSKEY_TAB);
          MAP_KEY (XK_Tab, CSKEY_TAB, CSKEY_TAB);
      
          MAP_KEY (XK_F1, CSKEY_F1, CSKEY_F1);
          MAP_KEY (XK_F2, CSKEY_F2, CSKEY_F2);
          MAP_KEY (XK_F3, CSKEY_F3, CSKEY_F3);
          MAP_KEY (XK_F4, CSKEY_F4, CSKEY_F4);
          MAP_KEY (XK_F5, CSKEY_F5, CSKEY_F5);
          MAP_KEY (XK_F6, CSKEY_F6, CSKEY_F6);
          MAP_KEY (XK_F7, CSKEY_F7, CSKEY_F7);
          MAP_KEY (XK_F8, CSKEY_F8, CSKEY_F8);
          MAP_KEY (XK_F9, CSKEY_F9, CSKEY_F9);
          MAP_KEY (XK_F10, CSKEY_F10, CSKEY_F10);
          MAP_KEY (XK_F11, CSKEY_F11, CSKEY_F11);
          MAP_KEY (XK_F12, CSKEY_F12, CSKEY_F12);

          //lock keys
          MAP_KEY (XK_Caps_Lock, CSKEY_CAPSLOCK, CSKEY_CAPSLOCK);
          //this is a particular capslock not present in querty keyboards which
          //acts like if you were keeping pressed shift actually (so it modifies
          //also numbers keys and punctuation and doesn't only do uppercase).
          //As it's not handled in cs it's mapped to the normal capslock
          MAP_KEY (XK_Shift_Lock, CSKEY_CAPSLOCK, CSKEY_CAPSLOCK);
          MAP_KEY (XK_Num_Lock, CSKEY_PADNUM, CSKEY_PADNUM);
          MAP_KEY (XK_Scroll_Lock, CSKEY_SCROLLLOCK, CSKEY_SCROLLLOCK);
	  
      case XK_KP_Add:
	    {
	      if (xf86vm && xf86vm->IsFullScreen () &&
          down && (event.xkey.state & Mod1Mask))
        SetVideoMode (true, true, false);
	      else
	      {
		HANDLE_MAP_KEY (CSKEY_PADPLUS, 0);
	      }
	      break;
	    }
      case XK_KP_Subtract:
	    {
	      if (xf86vm && xf86vm->IsFullScreen () &&
          down && (event.xkey.state & Mod1Mask))
        SetVideoMode (true, false, true);
	      else
	      {
		HANDLE_MAP_KEY (CSKEY_PADMINUS, 0);
	      }
	      break;
	    }
      case XK_KP_Multiply:
	    {
#ifdef CS_DEBUG
	      if (xf86vm && down && event.xkey.state & Mod1Mask)
          SetVideoMode (!xf86vm->IsFullScreen (), false, false);
	      else
#endif
	      {
		HANDLE_MAP_KEY (CSKEY_PADMULT, 0);
	      }
	      break;
	    }
      MAP_KEY (XK_KP_Divide, CSKEY_PADDIV, 0);
      MAP_KEY (XK_KP_Begin, CSKEY_PAD5, 0);
      MAP_KEY (XK_KP_5, CSKEY_PAD5, 0);
      case XK_KP_Enter:
      case XK_Return:
	    {
#ifndef CS_DEBUG
	      if (xf86vm && down && event.xkey.state & Mod1Mask)
          SetVideoMode (!xf86vm->IsFullScreen (), false, false);
	      else
#endif
	      {
          csKeyRaw = (xKey == XK_Return) ? CSKEY_ENTER : CSKEY_PADENTER;
          csKeyCooked = CSKEY_ENTER;
	      }
	      break;
	    }
      default:            
	    {
	      XKeyEvent myEvent (*((XKeyEvent *)&event));
	      myEvent.type = KeyPress;
	      csKeyCooked = KeyEventCharacter (keyboardIC, &myEvent);
	      XKeyEvent rawEvent (myEvent);
	      rawEvent.state = 0;
	      csKeyRaw = KeyEventCharacter (keyboardIC, &rawEvent);
	    }
#undef MAP_KEY
#undef HANDLE_MAP_KEY
        }
        if (csKeyRaw || csKeyCooked)
          EventOutlet->Key(csKeyRaw, csKeyCooked, down, prevdown != down);
        break;
      case FocusIn:
      {
        EventOutlet->Broadcast (csevFocusGained (name_reg), true);
#ifndef CS_DEBUG
#ifdef CS_XWIN_GRAB_KEYBOARD
	      if (xf86vm && !keyboard_grabbed && event.xfocus.window == wm_win)
	      {
	        XGrabKeyboard (dpy, ctx_win, false, GrabModeAsync,
            GrabModeAsync, CurrentTime);
	        keyboard_grabbed = 2;
	      }
#endif
#endif
	      break;
	    }
      case FocusOut:
      {
        EventOutlet->Broadcast (csevFocusLost (name_reg), false);
#ifndef CS_DEBUG
#ifdef CS_XWIN_GRAB_KEYBOARD
        if (xf86vm && keyboard_grabbed && !--keyboard_grabbed)
          XUngrabKeyboard (dpy, CurrentTime);
#endif
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
      case VisibilityNotify:
	      switch(event.xvisibility.state)
	      {
	        case VisibilityUnobscured:
	          EventOutlet->Broadcast (csevCanvasExposed (name_reg, Canvas), 0);
	          break;
	        case VisibilityPartiallyObscured:
	          EventOutlet->Broadcast (csevCanvasExposed (name_reg, Canvas), 0);
            break;
          case VisibilityFullyObscured:
	          EventOutlet->Broadcast (csevCanvasHidden (name_reg, Canvas), 0);
	          break;
	       }
	       break;
      case UnmapNotify:
	      EventOutlet->Broadcast (csevCanvasHidden (name_reg, Canvas), 0);
	      break;
      case MapNotify:
	      EventOutlet->Broadcast (csevCanvasExposed (name_reg, Canvas), 0);
	      break;
      case SelectionRequest:
        storedEvent = event;
        EventOutlet->Broadcast (csEventNameRegistry::GetID(name_reg,
          "crystalspace.xwindow.clipboard.selection.request"));
        break;
      case SelectionNotify:
        storedEvent = event;
        EventOutlet->Broadcast (csEventNameRegistry::GetID(name_reg,
          "crystalspace.xwindow.clipboard.selection.notify"));
        break;
      case SelectionClear:
        storedEvent = event;
        EventOutlet->Broadcast (csEventNameRegistry::GetID(name_reg,
          "crystalspace.xwindow.clipboard.selection.clear"));
        break;
      default:
        break;
    }
  }

  if (resize)
  {
    if (Canvas->Resize (wm_width, wm_height))
      XResizeWindow (dpy, ctx_win, wm_width, wm_height);
  }
  return false;
}

bool csXWindow::SetMousePosition (int x, int y)
{
  XWarpPointer (dpy, None, ctx_win, 0, 0, 0, 0, x, y);
  return true;
}

Cursor csXWindow::GetXCursor (csMouseCursorID shape)
{
  Cursor cur = None;
  if ((shape >= 0) && (shape <= csMouseCursorID (lastCursor)))
  {
    cur = MouseCursor[shape];
    if (cur == None)
    {
    #ifdef HAVE_XCURSOR
      /* With Xcursor there's some more choice when it comes to cursors,
       * prefer those for some cursors */
      const char* cursorName = nullptr;
      switch (shape)
      {
      case csmcSizeNWSE:	cursorName = "bd_double_arrow";	break;
      case csmcSizeNESW:	cursorName = "fd_double_arrow";	break;
      case csmcStop:		cursorName = "crossed_circle";	break;
      default:			break;
      }
      if (cursorName)
	cur = XcursorLibraryLoadCursor (dpy, cursorName);
    #endif
      if (!cur)
      {
	static const int cursorGlyphs[cursorNum] =
	{
	  XC_left_ptr, 		// Arrow
	  -1,			// Lens
	  XC_crosshair,		// Cross
	  XC_pencil,		// Pen
	  XC_fleur,		// Move
	  XC_sizing,		// SizeNWSE
	  XC_sizing,		// SizeNESW
	  XC_sb_v_double_arrow,	// SizeNS
	  XC_sb_h_double_arrow,	// SizeEW
	  XC_X_cursor,		// Stop
	  XC_watch		// Wait
	};
	if (cursorGlyphs[shape] >= 0)
	{
	  cur = XCreateFontCursor (dpy, cursorGlyphs[shape]);
	}
      }
      MouseCursor [shape] = cur;
    }
  }

  return cur;
}

bool csXWindow::SetMouseCursor (csMouseCursorID iShape)
{
  Cursor cursor = GetXCursor (iShape);
  if ((hwMouse != csGraphics2D::hwmcOff) && (cursor != None))
  {
    XDefineCursor (dpy, ctx_win, cursor);
    return true;
  }
  else
  {
    XDefineCursor (dpy, ctx_win, EmptyMouseCursor);
    return (iShape == csmcNone);
  } /* endif */
}

Cursor csXWindow::CreateRGBACursor (iImage *image, int hotspotX, int hotspotY)
{
#ifdef HAVE_XRENDER
  int imgW = image->GetWidth();
  int imgH = image->GetHeight();
  
  /* Convert RGBA data to required format
     (It seems the XImage allows fiddling with the pixel format ...
     but in practice this doesn't seem to make a difference.) */
  uint32* dataARGB32 = (uint32*)cs_malloc (imgW*imgH*sizeof(uint32));
  {
    const csRGBpixel* srcPtr = (csRGBpixel*)image->GetImageData();
    uint32* dstPtr = dataARGB32;
    int n = imgW*imgH;
    while (n-- > 0)
    {
      *dstPtr = (srcPtr->alpha << 24)
	      | (srcPtr->red << 16)
	      | (srcPtr->green << 8)
	      | (srcPtr->blue);
      srcPtr++;
      dstPtr++;
    }
  }
  
  XImage ximg;
  memset (&ximg, 0, sizeof (ximg));
  ximg.width = imgW;
  ximg.height = imgH;
  ximg.format = ZPixmap;
  ximg.data = (char*)dataARGB32;
#ifdef CS_LITTLE_ENDIAN
  ximg.byte_order = LSBFirst;
#else
  ximg.byte_order = MSBFirst;
#endif
  ximg.bitmap_unit = 32;
  ximg.bitmap_bit_order = ximg.byte_order;
  ximg.bitmap_pad = 32;
  ximg.depth = 32;
  ximg.bytes_per_line = imgW*4;
  ximg.bits_per_pixel = 32;
  ximg.red_mask = 0xff0000;
  ximg.green_mask = 0x00ff00;
  ximg.blue_mask = 0x0000ff;
  if (!XInitImage (&ximg))
  {
    cs_free (dataARGB32);
    return None;
  }
  Pixmap pixm = XCreatePixmap (dpy, RootWindow (dpy, screen_num),
			       imgW, imgH, 32);
  GC gc = XCreateGC (dpy, pixm, 0, nullptr);
  XPutImage (dpy, pixm, gc, &ximg, 0, 0, 0, 0,
	     imgW, imgH);
  XFreeGC (dpy, gc);
  cs_free (dataARGB32);

  XRenderPictFormat* pictFmt = XRenderFindStandardFormat (dpy, PictStandardARGB32);
  Picture pic = XRenderCreatePicture (dpy, pixm, pictFmt, 0, nullptr);
  XFreePixmap (dpy, pixm);
  Cursor cur = XRenderCreateCursor (dpy, pic, hotspotX, hotspotY);
  XRenderFreePicture (dpy, pic);
  return cur;
#else
  return None;
#endif
}

bool csXWindow::SetMouseCursor (iImage *image, const csRGBcolor* keycolor,
                                int hotspot_x, int hotspot_y,
                                csRGBcolor fg, csRGBcolor bg)
{
  if (hwMouse == csGraphics2D::hwmcOff) return false;
  
  if (!image) return false;

  // Check for cached cursor - we can only cache images with a name
  if (image->GetName ())
  {
    csRef<CursorWrapper> cur (cachedCursors.Get (image, csRef<CursorWrapper> ()));
    if (cur) 
    {
      XDefineCursor (dpy, ctx_win, *cur);
      return true;
    }
  }
  
  // Set an RGBA cursor, if we can
  if (haveRGBAcursors)
  {
    csRef<iImage> imageRGBA (CS::ImageAutoConvert (image, CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA));
    if (keycolor)
    {
      imageRGBA = csBakeKeyColor::Image (imageRGBA, *keycolor);
    }
    
    Cursor cur = CreateRGBACursor (imageRGBA, hotspot_x, hotspot_y);
    if (cur)
    {
      // Select + cache cursor
      XDefineCursor (dpy, ctx_win, cur);
      csRef<CursorWrapper> curWrap;
      curWrap.AttachNew (new CursorWrapper (dpy, cur));
      cachedCursors.Put (image, curWrap);
      return true;
    }
  }
  
  if (hwMouse == csGraphics2D::hwmcRGBAOnly) return false;

  // Fall back to a monochrome pointer. csCursorConverter takes care of
  // the conversion.
  uint8* source;
  uint8* mask;
  if (!csCursorConverter::ConvertTo1bpp (image, source, mask, fg, bg, 
    keycolor, true)) 
    return false;

  // Create Xwindow compatible Pixmaps
  Pixmap srcPixmap = XCreatePixmapFromBitmapData (dpy, ctx_win, (char *) source,
    image->GetWidth(), image->GetHeight(), 1, 0, 1);
  Pixmap maskPixmap = XCreatePixmapFromBitmapData (dpy, ctx_win, (char *) mask,
    image->GetWidth(), image->GetHeight(), 1, 0, 1);
  delete[] source;
  delete[] mask;

  // Create foreground color.  X colors are 16 bit so we must scale them to
  // keep relative values
  XColor xfg; 
  xfg.red = fg.red * 257; xfg.green = fg.green * 257; xfg.blue = fg.blue * 257;

  // Create foreground color.  X colors are 16 bit so we must scale them to
  // keep relative values
  XColor xbg; 
  xbg.red = bg.red * 257; xbg.green = bg.green * 257; xbg.blue = bg.blue * 257;
    
  // Create XWindow mouse cursor
  Cursor mouseCursor = XCreatePixmapCursor (dpy, srcPixmap, maskPixmap, 
                                            &xfg, &xbg, hotspot_x, hotspot_y);
  
  // Select the cursor
  XDefineCursor (dpy, ctx_win, mouseCursor);

  // Cache a pointer to the cursor (will it will be deleted in the destructor?)
  csRef<CursorWrapper> curWrap;
  curWrap.AttachNew (new CursorWrapper (dpy, mouseCursor));
  cachedCursors.Put (image, curWrap);

  return true;
}

bool csXWindow::AlertV (int type, const char* title, const char* okMsg,
			const char* msg, va_list arg)
{
#ifdef HAVE_GTK
  if (AlertV_GTK (type, title, okMsg, msg, arg)) return true;
#endif
#ifdef HAVE_XAW
  if (AlertV_Xaw (type, title, okMsg, msg, arg)) return true;
#endif
  return false;
}

bool csXWindow::SetWindowDecoration (iNativeWindow::WindowDecoration decoration, bool flag)
{
  const unsigned long hintsPropItems = sizeof (PropMotifWmHints) / sizeof (long);
  PropMotifWmHints newHints;
  
  Atom type;
  int format;
  unsigned long items;
  unsigned long bytesRemaining;
  PropMotifWmHints* wmHints = nullptr;
  XGetWindowProperty (dpy, wm_win, xaMotifWmHints,
		      0, hintsPropItems,
		      False, AnyPropertyType,
		      &type, &format, &items, &bytesRemaining,
		      reinterpret_cast<unsigned char**> (&wmHints));
		      
  if (type != None)
    memcpy (&newHints, wmHints, sizeof (PropMotifWmHints));
  else
    memset (&newHints, 0, sizeof (PropMotifWmHints));
  if (wmHints) XFree (wmHints);
  
  bool decoKnown = false;
  // Decoration flags that trigger a visible title bar
  const unsigned long titleFlags = MWM_DECOR_TITLE;
  switch (decoration)
  {
  case iNativeWindow::decoCaption:
    {
      if (newHints.flags & MWM_HINTS_DECORATIONS)
      {
	/* If MWM_DECOR_ALL is set, all other flags that are set
	   _are supposed to disable_ these decorations.
	   If MWM_DECOR_ALL is not set, flags are enabling. */
	if (newHints.decorations & MWM_DECOR_ALL)
	{
	  if (flag)
	    newHints.decorations &= ~titleFlags;
	  else
	    newHints.decorations |= titleFlags;
	}
	else
	{
	  if (flag)
	    newHints.decorations |= titleFlags;
	  else
	    newHints.decorations &= ~titleFlags;
	}
      }
      else
      {
	newHints.flags |= MWM_HINTS_DECORATIONS;
	/* Metacity seems to always show the caption when ALL is set,
	   so initialize decorations with only the title flags. */
	newHints.decorations = (flag ? titleFlags : 0);
      }
      decoKnown = true;
    }
    break;
  default:
    break;
  }
  
  if (decoKnown)
  {
    XChangeProperty (dpy, wm_win, xaMotifWmHints, xaMotifWmHints, 32,
      PropModeReplace, (unsigned char*)&newHints,
      hintsPropItems);
  }
  return decoKnown;
}

bool csXWindow::GetWindowDecoration (iNativeWindow::WindowDecoration decoration, bool& result)
{
  const unsigned long hintsPropItems = sizeof (PropMotifWmHints) / sizeof (long);
  
  bool ret = false;
  Atom type;
  int format;
  unsigned long items;
  unsigned long bytesRemaining;
  PropMotifWmHints* wmHints = nullptr;
  XGetWindowProperty (dpy, wm_win, xaMotifWmHints,
		      0, hintsPropItems,
		      False, AnyPropertyType,
		      &type, &format, &items, &bytesRemaining,
		      reinterpret_cast<unsigned char**> (&wmHints));

  if (type != None)
  {
    switch (decoration)
    {
    case iNativeWindow::decoCaption:
      {
	if (wmHints->flags & MWM_HINTS_DECORATIONS)
	{
	  if (wmHints->decorations & MWM_DECOR_ALL)
	    result = (wmHints->decorations & MWM_DECOR_TITLE) == 0;
	  else
	    result = (wmHints->decorations & MWM_DECOR_TITLE) != 0;
	  ret = true;
	}
      }
      break;
    default:
      break;
    }
  }
  
  if (wmHints) XFree (wmHints);
  return ret;
}

bool csXWindow::GetWorkspaceDimensions (int& width, int& height)
{
  bool ret (false);
  Atom xaNetWorkArea (XInternAtom (dpy, "_NET_WORKAREA", True));
  if (xaNetWorkArea != None)
  {
    Atom type;
    int format;
    unsigned long items;
    unsigned long bytesRemaining;
    long* workArea = nullptr;
    int result (XGetWindowProperty (dpy, RootWindow (dpy, screen_num), xaNetWorkArea,
                                    0, 4,
                                    False, AnyPropertyType,
                                    &type, &format, &items, &bytesRemaining,
                                    reinterpret_cast<unsigned char**> (&workArea)));
    if ((result == Success) && (type == XA_CARDINAL) && (format == 32)
      && (items == 4))
    {
      width = workArea[2] - workArea[0];
      height = workArea[3] - workArea[1];
      ret = true;
    }
    
    if (workArea) XFree (workArea);
  }
  return ret;
}

bool csXWindow::AddWindowFrameDimensions (int& width, int& height)
{
  bool ret (false);
  if (wm_win)
  {
    // @@@ FIXME: Should create a dummy top-level window if no wm_win exists yet?
    Atom xaNetFrameExtents (XInternAtom (dpy, "_NET_FRAME_EXTENTS", True));
    if (xaNetFrameExtents != None)
    {
      Atom type;
      int format;
      unsigned long items;
      unsigned long bytesRemaining;
      long* frameExtents = nullptr;
      int result (XGetWindowProperty (dpy, wm_win, xaNetFrameExtents,
                                      0, 4,
                                      False, AnyPropertyType,
                                      &type, &format, &items, &bytesRemaining,
                                      reinterpret_cast<unsigned char**> (&frameExtents)));
      if ((result == Success) && (type == XA_CARDINAL) && (format == 32)
        && (items == 4))
      {
        width += frameExtents[0] + frameExtents[1];
        height += frameExtents[2] + frameExtents[3];
        ret = true;
      }
      
      if (frameExtents) XFree (frameExtents);
    }
    
  }
  return ret;
}
