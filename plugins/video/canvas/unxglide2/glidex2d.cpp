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
#include "cs2d/unxglide2/glidex2d.h"
#include "cscom/com.h"
#include "csinput/csevent.h"
#include "csinput/csinput.h"
#include "cssys/unix/iunix.h"
#include "csutil/inifile.h"
#include "csgeom/csrect.h"
#include "cs3d/glide2/glidelib.h"
#include "isystem.h"

BEGIN_INTERFACE_TABLE (csGraphics2DGlideX)
  IMPLEMENTS_COMPOSITE_INTERFACE_EX (IGraphics2D, XGraphics2D)
  IMPLEMENTS_COMPOSITE_INTERFACE_EX (IGraphicsInfo, XGraphicsInfo)
END_INTERFACE_TABLE ()

IMPLEMENT_UNKNOWN_NODELETE (csGraphics2DGlideX)

  // replace this with config stuff...
bool DoGlideInWindow=true; 

bool locked;

csGraphics2DGlideX* thisPtr=NULL;

// csGraphics2DGLX functions
csGraphics2DGlideX::csGraphics2DGlideX(ISystem* piSystem) :
  csGraphics2D (piSystem), xim (NULL), cmap (0)
{
  System = piSystem;
  if (FAILED (System->QueryInterface (IID_IUnixSystemDriver, (void**)&UnixSystem)))
  {
    CsPrintf (MSG_FATAL_ERROR, "FATAL: The system driver does not support "
                               "the IUnixSystemDriver interface\n");
    exit (-1);
  }
}

void csGraphics2DGlideX::Initialize()
{
  thisPtr=this;
  csGraphics2D::Initialize ();
  Screen* screen_ptr;

  // see if we need to go fullscreen or not...
  csIniFile* config = new csIniFile("cryst.cfg");
  m_DoGlideInWindow = (!config->GetYesNo("VideoDriver","FULL_SCREEN",FALSE));
  CHK (delete config);

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
  UnixSystem->GetSettings (sim_depth, do_shm, do_hwmouse);

  dpy = XOpenDisplay (NULL);

  screen_num = DefaultScreen (dpy);
  screen_ptr = DefaultScreenOfDisplay (dpy);
  display_width = DisplayWidth (dpy, screen_num);
  display_height = DisplayHeight (dpy, screen_num);

  // Determine visual information.
  //Visual* visual = DefaultVisual (dpy, screen_num);

  
  Depth=16;
	  
  DrawPixel = DrawPixelGlide; WriteChar = WriteCharGlide;
  GetPixelAt = GetPixelAt16;  DrawSprite = DrawSpriteGlide;
    
  // calculate CS's pixel format structure. 565
  pfmt.PixelBytes = 2;
  pfmt.PalEntries = 0;
  pfmt.RedMask = (1+2+4+8+16)<<11;
  pfmt.GreenMask = (1+2+4+8+16+32)<<5;
  pfmt.BlueMask = (1+2+4+8+16);
    
  complete_pixel_format();


	/* 
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
	*/

  CsPrintf (MSG_INITIALIZATION, "Video driver Glide/X version ");
  CsPrintf (MSG_INITIALIZATION, "\n");
 
  GraphicsReady=1;  

}

csGraphics2DGlideX::~csGraphics2DGlideX ()
{
  // Destroy your graphic interface
  GraphicsReady=0;
  Close ();
  if (UnixSystem)
    FINAL_RELEASE (UnixSystem);
  CHKB (delete [] Memory);
}

// Used to printf through system driver
void csGraphics2DGlideX::CsPrintf (int msgtype, char *format, ...)
{
  va_list arg;
  char buf[256];

  va_start (arg, format);
  vsprintf (buf, format, arg);
  va_end (arg);

  System->Print (msgtype, buf);
}

bool csGraphics2DGlideX::Open(char *Title)
{ 

  // Open your graphic interface
  if (!csGraphics2D::Open (Title))
    return false;

  // Set loop callback
  UnixSystem->SetLoopCallback (ProcessEvents, this);

  // Open window
  if (m_DoGlideInWindow)
    window = XCreateSimpleWindow (dpy, DefaultRootWindow (dpy), 64, 16,
                                      Width, Height, 4, 0, 0);
  else
    window = XCreateSimpleWindow (dpy, DefaultRootWindow (dpy), 64, 16,
                                      display_width, display_height, 4, 0, 0);
  XMapWindow (dpy, window);
  XStoreName (dpy, window, Title);
  XGCValues values;
  gc = XCreateGC (dpy, window, 0, &values);
  XSetGraphicsExposures (dpy, gc, False);
  XSetWindowAttributes attr;
  attr.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
       FocusChangeMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask;
  XChangeWindowAttributes (dpy, window, CWEventMask, &attr);

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
        CHK (Memory = new unsigned char[Width*Height*pfmt.PixelBytes]);
      }
    }
  }

  bPalettized = false;
  bPaletteChanged = false;
	
  return true;
}

void csGraphics2DGlideX::Close(void)
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
  
  
  if (m_DoGlideInWindow)
  {

#ifdef DO_SHM
    shmdt(shmi.shmaddr);
#else
    CHK (delete [] Memory);
#endif

    unsetenv("SST_NOSHUTDOWN");
    unsetenv("SSTV2_NOSHUTDOWN");
    unsetenv("SST_VGA_PASS");
    unsetenv("SSTV2_VGA_PASS");
  } 

  // Close your graphic interface
  csGraphics2D::Close ();
}

void csGraphics2DGlideX::Print (csRect *area)
{
  if (m_DoGlideInWindow)  
  {
    FXgetImage();
  }
}


#define GR_DRAWBUFFER GR_BUFFER_FRONTBUFFER

bool csGraphics2DGlideX::BeginDraw(/*int Flag*/)
{
  FxBool bret;
  lfbInfo.size=sizeof(GrLfbInfo_t);
  
  glDrawMode=GR_LFB_WRITE_ONLY;

  if(locked) FinishDraw();

    
  bret=GlideLib_grLfbLock(glDrawMode|GR_LFB_IDLE,
                          GR_DRAWBUFFER,
                          GR_LFBWRITEMODE_565,
                          GR_ORIGIN_UPPER_LEFT,
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
void csGraphics2DGlideX::FXgetImage()
{

  // we only handle 16bit 
   if (Depth==16) 
   {    
          grLfbReadRegion( GR_BUFFER_FRONTBUFFER,       
                      0, 0,
                      Width, Height,
                      Width * 2,
                      xim->data);         
   }

   // now put image in window...
   XPutImage (dpy, window, gc, xim, 0, 0, 0, 0, Width, Height);

}


void csGraphics2DGlideX::FinishDraw ()
{
 
  Memory=NULL;
  for (int i = 0; i < Height; i++) LineAddress [i] = 0;
  if (locked) 
    GlideLib_grLfbUnlock(glDrawMode,GR_DRAWBUFFER);
  
  locked = false;
}


void csGraphics2DGlideX::SetRGB(int i, int r, int g, int b)
{
  csGraphics2D::SetRGB (i, r, g, b);
  bPaletteChanged = true;
  SetTMUPalette(0);
}


void csGraphics2DGlideX::SetTMUPalette(int tmu)
{
  GuTexPalette p;
  RGBpaletteEntry pal;
  
  for(int i=0; i<256; i++)
  {
    pal = Palette[i];
    p.data[i]=0xFF<<24 | pal.red<<16 | pal.green<<8 | pal.blue;
  }
  
  GlideLib_grTexDownloadTable(tmu, GR_TEXTABLE_PALETTE, &p);		
}

bool csGraphics2DGlideX::SetMouseCursor (int iShape, ITextureHandle* /*iBitmap*/)
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

void csGraphics2DGlideX::DrawLine (int x1, int y1, int x2, int y2, int color)
{
  // can't do this while framebuffer is locked...
  if (locked) return;
 
  GrVertex a,b;
  a.x=x1; a.y=y1;
  b.x=x2; b.y=y2;

  grConstantColorValue(color);
  grDrawLine(&a,&b);
}

void csGraphics2DGlideX::DrawPixelGlide (int x, int y, int color)
{
  // can't do this while framebuffer is locked...
  if (locked) return;

  GrVertex p;
  p.x=x; p.y=y;

  grConstantColorValue(color);
  grDrawPoint(&p);
}

void csGraphics2DGlideX::WriteCharGlide (int x, int y, int fg, int bg, char c)
{
  //if (!locked)
//thisPtr->BeginDraw();
//  if (locked) thisPtr->FinishDraw();
//  thisPtr->BeginDraw();
  thisPtr->WriteChar16(x,y,fg,bg,c);

  // not implemented yet...
}

void csGraphics2DGlideX::DrawSpriteGlide (ITextureHandle *hTex, int sx, int sy,
  int sw, int sh, int tx, int ty, int tw, int th)
{
 // if (!locked) thisPtr->BeginDraw();
//  if (locked) thisPtr->FinishDraw();
  //thisPtr->BeginDraw();
  thisPtr->DrawSprite16(hTex,sx,sy,sw,sh,tx,ty,tw,th);
  // not implemented yet...
}

unsigned char* csGraphics2DGlideX::GetPixelAtGlide (int x, int y)
{
  // not implemented yet...
   //static FxBool bret;
//   static unsigned char ch;
   //static GrLfbInfo_t lfbInfo;

/*   if (!locked)
   {
     lfbInfo.size=sizeof(GrLfbInfo_t);


     bret=GlideLib_grLfbLock(GR_LFB_READ_ONLY|GR_LFB_IDLE,
                          GR_DRAWBUFFER,
                          GR_LFBWRITEMODE_565,
                          GR_ORIGIN_ANY,
                          FXFALSE,
                          &lfbInfo);
     locked=bret;
   }*/


   if (!locked) thisPtr->BeginDraw();

   return thisPtr->GetPixelAt16(x,y);
/*   if(locked)
   {
     //Memory=(unsigned char*)lfbInfo.lfbPtr;
     return (unsigned char *)thisPtr->Memory+(y*thisPtr->lfbInfo.strideInBytes+x);

     //GlideLib_grLfbUnlock(GR_LFB_READ_ONLY,GR_DRAWBUFFER);
   }

   else return NULL;*/
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

void csGraphics2DGlideX::ProcessEvents (void *Param)
{
  static int button_mapping[6] = {0, 1, 3, 2, 4, 5};
  csGraphics2DGlideX *Self = (csGraphics2DGlideX *)Param;
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
