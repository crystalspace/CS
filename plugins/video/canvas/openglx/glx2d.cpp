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
#include "cs3d/opengl/ogl_txtmgr.h"
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

csGraphics2DOpenGLFontServer *csGraphics2DGLX::LocalFontServer = NULL;
OpenGLTextureCache *csGraphics2DGLX::texture_cache = NULL;

// csGraphics2DGLX function
csGraphics2DGLX::csGraphics2DGLX (ISystem* piSystem) :
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

void csGraphics2DGLX::Initialize ()
{
  csGraphics2D::Initialize ();
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

  int desired_attributes[] = { GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 1, None };
 
  active_GLVisual = glXChooseVisual(dpy, screen_num, desired_attributes);
  if (active_GLVisual)
  {
    active_GLContext = glXCreateContext(dpy,active_GLVisual,0,GL_TRUE);
    cmap = XCreateColormap (dpy, RootWindow (dpy, active_GLVisual->screen),
           active_GLVisual->visual, AllocNone);
    visual = active_GLVisual->visual;
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

// Used to printf through system driver
void csGraphics2DGLX::CsPrintf (int msgtype, char *format, ...)
{
  va_list arg;
  char buf[256];

  va_start (arg, format);
  vsprintf (buf, format, arg);
  va_end (arg);

  System->Print (msgtype, buf);
}

bool csGraphics2DGLX::Open(char *Title)
{
  CsPrintf (MSG_INITIALIZATION, "Video driver GL/X version ");
  if (glGetString (GL_RENDERER))
    CsPrintf (MSG_INITIALIZATION, "(Renderer v%s) ", glGetString(GL_RENDERER) );
  if (glGetString (GL_VERSION))
    CsPrintf (MSG_INITIALIZATION, "(OpenGL v%s)", glGetString(GL_VERSION));
  CsPrintf (MSG_INITIALIZATION, "\n");

  // Open your graphic interface
  if (!csGraphics2D::Open (Title))
    return false;

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

  glXMakeCurrent(dpy, window, active_GLContext);


  if (LocalFontServer == NULL)
  {
       LocalFontServer = new csGraphics2DOpenGLFontServer(&FontList[0]);
       for (int fontindex=1; 
       		fontindex < 8;
		fontindex++)
	   LocalFontServer->AddFont(FontList[fontindex]);
  }

  if (texture_cache == NULL)
  {
    CHK (texture_cache = new OpenGLTextureCache(1<<24,24));
  }

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
  csGraphics2D::Close ();
}

void csGraphics2DGLX::Clear(int color)
{
  switch (pfmt.PixelBytes)
  {
  case 1: // paletted colors
    glClearColor(Palette[color].red,
    		Palette[color].green,
		Palette[color].blue,0.);
    break;
  case 2: // 16bit color
  case 4: // truecolor
    glClearColor( ( (color & pfmt.RedMask) >> pfmt.RedShift )     / (float)pfmt.RedBits,
               ( (color & pfmt.GreenMask) >> pfmt.GreenShift ) / (float)pfmt.GreenBits,
               ( (color & pfmt.BlueMask) >> pfmt.BlueShift )   / (float)pfmt.BlueBits,
	       0. );
    break;
  }
  glClear(GL_COLOR_BUFFER_BIT);
}

void csGraphics2DGLX::Print (csRect *area)
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
*/
//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glXSwapBuffers (dpy,window);
  glFlush ();
}

void csGraphics2DGLX::SetRGB(int i, int r, int g, int b)
{
  csGraphics2D::SetRGB (i, r, g, b);
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
}

void csGraphics2DGLX::setGLColorfromint(int color)
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

void csGraphics2DGLX::DrawLine (int x1, int y1, int x2, int y2, int color)
{
  // prepare for 2D drawing--so we need no fancy GL effects!
  glDisable (GL_TEXTURE_2D);
  glDisable (GL_BLEND);
  glDisable (GL_DEPTH_TEST);
  glBegin (GL_LINES);
  //glColor3f (1., 1., 1.);
  setGLColorfromint(color);
  glVertex2i (x1, Height-y1-1);
  glVertex2i (x2, Height-y2-1);
  glEnd ();
}

void csGraphics2DGLX::DrawHorizLine (int x1, int x2, int y, int color)
{
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

void csGraphics2DGLX::DrawPixelGL (int x, int y, int color)
{
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

void csGraphics2DGLX::WriteCharGL (int x, int y, int fg, int bg, char c)
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

void csGraphics2DGLX::DrawSpriteGL (ITextureHandle *hTex, int sx, int sy,
  int sw, int sh, int tx, int ty, int tw, int th)
{
  texture_cache->Add (hTex);

  // cache the texture if we haven't already.
  csTextureMMOpenGL* txt_mm = (csTextureMMOpenGL*)GetcsTextureMMFromITextureHandle (hTex);

  HighColorCache_Data *cachedata;
  cachedata = txt_mm->get_hicolorcache ();
  GLuint texturehandle = *( (GLuint *) (cachedata->pData) );

  glShadeModel(GL_FLAT);
  glEnable(GL_TEXTURE_2D);
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

unsigned char* csGraphics2DGLX::GetPixelAtGL (int /*x*/, int /*y*/)
{
  return NULL;
}

