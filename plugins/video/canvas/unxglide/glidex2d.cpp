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

#define XK_XKB_KEYS
#include <stdarg.h>
#include "cssysdef.h"
#include "glidex2d.h"
#include "csutil/scf.h"
#include "cssys/csinput.h"
#include "csutil/csrect.h"
#include "isys/isystem.h"
#include "video/canvas/common/x11comm.h"
#include "video/canvas/common/scancode.h"
#include "video/renderer/glide/gllib.h"

#ifdef GLIDE3

IMPLEMENT_FACTORY (csGraphics2DGlideX)

EXPORT_CLASS_TABLE (glidx2d3)
  EXPORT_CLASS_DEP (csGraphics2DGlideX, "crystalspace.graphics2d.glide.x.3",
    "Glide V3/X 2D graphics driver for Crystal Space", "crystalspace.font.server.")
EXPORT_CLASS_TABLE_END

#else

IMPLEMENT_FACTORY (csGraphics2DGlideX)

EXPORT_CLASS_TABLE (glidx2d2)
  EXPORT_CLASS_DEP (csGraphics2DGlideX, "crystalspace.graphics2d.glide.x.2",
    "Glide V2/X 2D graphics driver for Crystal Space", "crystalspace.font.server.")
EXPORT_CLASS_TABLE_END

#endif

IMPLEMENT_IBASE (csGraphics2DGlideX)
  IMPLEMENTS_INTERFACE (iEventPlug)
IMPLEMENT_IBASE_END

csGraphics2DGlideX* thisPtr=NULL;

// csGraphics2DGLX functions
csGraphics2DGlideX::csGraphics2DGlideX(iBase *iParent) :
  csGraphics2DGlideCommon( iParent ), xim (NULL), cmap (0)
{
  EventOutlet = NULL;
}

bool csGraphics2DGlideX::Initialize (iSystem *pSystem)
{
  thisPtr=this;
  if (!csGraphics2DGlideCommon::Initialize (pSystem))
    return false;

  // if we are going to do glide in an Xwindow, 
  // we need to set some environment vars first...
  if (m_DoGlideInWindow)
  {
    setenv("SST_NOSHUTDOWN","1",1);
    setenv("SSTV2_NOSHUTDOWN","1",1);
    setenv("SST_VGA_PASS","1",1);
    setenv("SSTV2_VGA_PASS","1",1);
  } 

  // Query system settings
  int sim_depth;
  bool do_shm;
  GetX11Settings (pSystem, sim_depth, do_shm, do_hwmouse);

  dpy = XOpenDisplay (NULL);

  // Set user locale for national character support
  if (XSupportsLocale ())
    XSetLocaleModifiers ("");

  screen_num = DefaultScreen (dpy);
  display_width = DisplayWidth (dpy, screen_num);
  display_height = DisplayHeight (dpy, screen_num);

  // Determine visual information.
  //Visual* visual = DefaultVisual (dpy, screen_num);

  
  // calculate CS's pixel format structure. 565
  pfmt.PixelBytes = 2;
  pfmt.PalEntries = 0;
  pfmt.RedMask = (1+2+4+8+16)<<11;
  pfmt.GreenMask = (1+2+4+8+16+32)<<5;
  pfmt.BlueMask = (1+2+4+8+16);
    
  pfmt.complete ();


	/* 
  pfmt.RedMask = 0xf00;//visual->red_mask;
  pfmt.GreenMask = 0xf0;//visual->green_mask;
  pfmt.BlueMask = 0xf;//visual->blue_mask;
  pfmt.complete ();
  pfmt.PalEntries = visual->map_entries;
  if (visual->c_class == TrueColor)
    pfmt.PalEntries = 0;
  if (pfmt.PalEntries)
    pfmt.PixelBytes = 1;		// Palette mode
  else
    pfmt.PixelBytes = 2;		// Truecolor mode
	*/

  CsPrintf (MSG_INITIALIZATION, "Video driver Glide/X version ");
  CsPrintf (MSG_INITIALIZATION, "\n");
 
  // Tell system driver to call us on every frame
  System->CallOnEvents (this, CSMASK_Nothing);
  // Create the event outlet
  EventOutlet = System->CreateEventOutlet (this);

  GraphicsReady = 1;  

  return true;
}

csGraphics2DGlideX::~csGraphics2DGlideX ()
{
  // Destroy your graphic interface
  GraphicsReady=0;
  Close ();
  if (EventOutlet)
    EventOutlet->DecRef ();
}

bool csGraphics2DGlideX::Open(const char *Title)
{ 
  // Open your graphic interface
  if (!csGraphics2DGlideCommon::Open (Title))
    return false;

  // Open window
  if (m_DoGlideInWindow)
    window = XCreateSimpleWindow (dpy, DefaultRootWindow (dpy), 64, 16,
                                      Width, Height, 4, 0, 0);
 else
    window = XCreateSimpleWindow (dpy, DefaultRootWindow (dpy), 0, 0,
                                      display_width, display_height, 4, 0, 0);

  XRaiseWindow (dpy,window);
  XMapWindow (dpy, window);
  XStoreName (dpy, window, Title);
  XGCValues values;
  gc = XCreateGC (dpy, window, 0, &values);
  XSetGraphicsExposures (dpy, gc, False);
  XSetWindowAttributes attr;
  attr.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
       FocusChangeMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask 
       | StructureNotifyMask;
  XChangeWindowAttributes (dpy, window, CWEventMask, &attr);

  // Intern WM_DELETE_WINDOW and set window manager protocol
  // (Needed to catch user using window manager "delete window" button)
  wm_delete_window = XInternAtom (dpy, "WM_DELETE_WINDOW", False);
  XSetWMProtocols (dpy, window, &wm_delete_window, 1);


  // Wait for expose event (why not ?)
  XEvent event;
  for (;;)
  {
    XNextEvent (dpy, &event);
    if (event.type == Expose) break; 	
  }

  if (m_DoGlideInWindow)
  {
    // Create backing store
    if (!xim)
    {
      xim = XGetImage (dpy, window, 0, 0, Width, Height, AllPlanes, ZPixmap);
#ifdef DO_SHM
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
#endif 
      {
        Memory = new unsigned char[Width*Height*pfmt.PixelBytes];
      }
    }
  }

  SetMousePosition( Width/2, Height/2 );

  return true;
}

void csGraphics2DGlideX::Close(void)
{
  if (window)
  {
    XDestroyWindow (dpy, window);
    window = 0;
  }
  
  
  if (m_DoGlideInWindow)
  {

#ifdef DO_SHM
    shmdt(shmi.shmaddr);
#else
    delete [] Memory;
#endif

    unsetenv("SST_NOSHUTDOWN");
    unsetenv("SSTV2_NOSHUTDOWN");
    unsetenv("SST_VGA_PASS");
    unsetenv("SSTV2_VGA_PASS");
  } 

  // Close your graphic interface
  csGraphics2DGlideCommon::Close ();
}

void csGraphics2DGlideX::Print (csRect *area)
{  
  if (m_DoGlideInWindow)  
  {
    FXgetImage();
  }
  csGraphics2DGlideCommon::Print( area );
}


// simplification of the Mesa FXgetImage() function
void csGraphics2DGlideX::FXgetImage()
{

  // we only handle 16bit 
   if (Depth==16) 
   {    
          grLfbReadRegion( m_drawbuffer,       
                      0, 0,
                      Width, Height,
                      Width * 2,
                      xim->data);         
   }

   // now put image in window...
   XPutImage (dpy, window, gc, xim, 0, 0, 0, 0, Width, Height);
}

bool csGraphics2DGlideX::SetMouseCursor (csMouseCursorID iShape)
{
  if ((int)iShape < nCursor ){
    nCurCursor = (int)iShape;
    return true;
  }
  nCurCursor=-1;
  return false;
}

bool csGraphics2DGlideX::SetMousePosition (int x, int y)
{
  mx=x; my=y; if (mxold==-1) { mxold=y; myold=y; }
  XWarpPointer (dpy, None, window, 0, 0, 0, 0, x, y);
  return true;    
}

static Bool CheckKeyPress (Display *dpy, XEvent *event, XPointer arg)
{
  XEvent *curevent = (XEvent *)arg;
  (void)dpy;
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

bool csGraphics2DGlideX::HandleEvent (iEvent &/*Event*/)
{
  static int button_mapping[6] = {0, 1, 3, 2, 4, 5};
  XEvent event;
  KeySym key;
  int charcount;
  char charcode [8];
  bool down;

  while (XCheckIfEvent (dpy, &event, AlwaysTruePredicate, 0))
    switch (event.type)
    {
      case ButtonPress:
        EventOutlet->Mouse (button_mapping [event.xbutton.button],
          true, event.xbutton.x, event.xbutton.y);
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
      case ButtonRelease:
        EventOutlet->Mouse (button_mapping [event.xbutton.button],
          false, event.xbutton.x, event.xbutton.y);
        break;
      case MotionNotify:
        EventOutlet->Mouse (0, false, event.xbutton.x, event.xbutton.y);
	mx = event.xbutton.x; my = event.xbutton.y;
        break;
      case KeyPress:
      case KeyRelease:
        // Neat trick: look in event queue if we have KeyPress events ahead
	// with same keycode. If this is the case, discard the KeyUp event
	// in favour of KeyDown since this is most (sure?) an autorepeat
        XCheckIfEvent (event.xkey.display, &event, CheckKeyPress, (XPointer)&event);
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
          case XK_KP_Add:     key = CSKEY_PADPLUS; break;
          case XK_KP_Subtract:key = CSKEY_PADMINUS; break;
          case XK_KP_Multiply:key = CSKEY_PADMULT; break;
          case XK_KP_Divide:  key = CSKEY_PADDIV; break;
          case XK_KP_Begin:   key = CSKEY_CENTER; break;
          default:            key = (key <= 127) ? ScanCodeToChar [key] : 0;
        }
	EventOutlet->Key (key, charcount == 1 ? uint8 (charcode [0]) : 0, down);
        break;
      case FocusIn:
      case FocusOut:
        EventOutlet->Broadcast (cscmdFocusChanged, (void *)(event.type == FocusIn));
        break;
      case Expose:
      {
        csRect rect (event.xexpose.x, event.xexpose.y,
	  event.xexpose.x + event.xexpose.width, event.xexpose.y + event.xexpose.height);
	Print (&rect);
        break;
      }
      default:
        //if (event.type == CompletionType) shm_busy = 0;
        break;
    }
  return false;
}


