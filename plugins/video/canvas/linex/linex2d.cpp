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
#include "cssysdef.h"
#include "cssys/sysfunc.h"
#include "linex2d.h"
#include "csgeom/csrect.h"
#include "csutil/cfgacc.h"
#include "csutil/csstring.h"
#include "csutil/scf.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/plugin.h"
#include "iutil/cfgmgr.h"
#include "iutil/cmdline.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csGraphics2DLineXLib)
SCF_IMPLEMENT_FACTORY (csLineX2DFontServer)

SCF_EXPORT_CLASS_TABLE (linex2d)
  SCF_EXPORT_CLASS_DEP (csGraphics2DLineXLib, "crystalspace.graphics2d.linex2d",
    "X-Windows 2D graphics driver for Crystal Space", "crystalspace.font.server.")
  SCF_EXPORT_CLASS (csLineX2DFontServer, "crystalspace.font.server.linex2d",
    "Private X-Windows font server for LineX2D canvas")
SCF_EXPORT_CLASS_TABLE_END

SCF_IMPLEMENT_IBASE_EXT (csGraphics2DLineXLib)
  SCF_IMPLEMENTS_INTERFACE (iEventPlug)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_IBASE (csLineX2DFontServer)
  SCF_IMPLEMENTS_INTERFACE (iFontServer)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csLineX2DFontServer::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

#define XWIN_SCF_ID "crystalspace.window.x"
Display* csGraphics2DLineXLib::dpy = NULL;

csGraphics2DLineXLib::csGraphics2DLineXLib (iBase *iParent) :
  csGraphics2D (iParent), window (0), cmap (0)
{
  xwin = NULL;
  EventOutlet = NULL;
}

void csGraphics2DLineXLib::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  iReporter* rep = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (rep)
  {
    rep->ReportV (severity, "crystalspace.canvas.linex", msg, arg);
    rep->DecRef ();
  }
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

bool csGraphics2DLineXLib::Initialize (iObjectRegistry *object_reg)
{
  if (!csGraphics2D::Initialize (object_reg))
    return false;

  iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  xwin = CS_LOAD_PLUGIN (plugin_mgr, XWIN_SCF_ID, iXWindow);
  if (!xwin)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, 
	    "Could not create an instance of %s\n", XWIN_SCF_ID);
    return false;
  }
  dpy = xwin->GetDisplay ();
  screen_num = xwin->GetScreen ();

  // Do a trick: unload the system font server since its useless for us
  iFontServer *fs = (iFontServer*)(CS_QUERY_REGISTRY (object_reg, iFontServer));
  if (fs)
  {
    iComponent *fsc = SCF_QUERY_INTERFACE (fs, iComponent);
    if (fsc)
    {
      plugin_mgr->UnloadPlugin (fsc);
      fsc->DecRef ();
    }
    fs->DecRef ();
  }
  // Also DecRef the FontServer since csGraphics2D::Initialize IncRef'ed it
  if (FontServer)
    FontServer->DecRef ();

  // Load our specific font server instead
  FontServer = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.font.server.linex2d",
    iFontServer);

  iEventQueue* q = CS_QUERY_REGISTRY(object_reg, iEventQueue);
  if (q != 0)
  {
    // Tell event queue to call us on broadcast events
    //@@@ why is this needed???
    //q->RegisterListener (&scfiEventHandler, CSMASK_Broadcast);
    // Create the event outlet
    EventOutlet = q->CreateEventOutlet (this);
    q->DecRef ();
  }
  plugin_mgr->DecRef ();
  return true;
}

csGraphics2DLineXLib::~csGraphics2DLineXLib(void)
{
  Close();
  if (EventOutlet)
    EventOutlet->DecRef();
  if (xwin)
    xwin->DecRef();
}

bool csGraphics2DLineXLib::Open()
{
  if (is_open) return true;

  if (!CreateVisuals ())
    return false;

  xwin->SetVisualInfo (&xvis);
  xwin->SetColormap (cmap);
  xwin->SetCanvas ((iGraphics2D *)this);

  if (!xwin->Open ())
  {
    Report (CS_REPORTER_SEVERITY_ERROR, 
	    "Failed to open the X-Window!");
    return false;
  }
  window = xwin->GetWindow ();
  gc = xwin->GetGC ();

  Report (CS_REPORTER_SEVERITY_NOTIFY, "Crystal Space X windows driver (Line drawing).");
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Using %d bit %sColor visual",
              xvis.depth, (xvis.visual->c_class == PseudoColor) ? "Pseudo" : "True");

  // Open your graphic interface
  if (!csGraphics2D::Open ())
    return false;

  if (!AllocateMemory ())
    return false;

  Clear (0);

  return true;
}

bool csGraphics2DLineXLib::CreateVisuals ()
{
  // Determine visual information.
  // Try in order:
  //   screen depth
  //   24 bit TrueColor
  //   16 bit TrueColor
  //   15 bit TrueColor
  //   8 bit PseudoColor
  int d = DefaultDepthOfScreen (XScreenOfDisplay (dpy, screen_num));
  if (!(XMatchVisualInfo(dpy, screen_num, d, (d == 8 ? PseudoColor : TrueColor), &xvis)
	|| XMatchVisualInfo(dpy, screen_num, 24, TrueColor, &xvis)
	|| XMatchVisualInfo(dpy, screen_num, 16, TrueColor, &xvis)
	|| XMatchVisualInfo(dpy, screen_num, 15, TrueColor, &xvis)
	|| XMatchVisualInfo(dpy, screen_num, 8, PseudoColor, &xvis)))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "FATAL: Current screen depth not supported (8, 15, 16 or 32 bpp only)");
    return false;
  }

  pfmt.RedMask = xvis.red_mask;
  pfmt.GreenMask = xvis.green_mask;
  pfmt.BlueMask = xvis.blue_mask;

  pfmt.complete ();
  pfmt.PalEntries = xvis.colormap_size;
  if (xvis.visual->c_class == TrueColor)
    pfmt.PalEntries = 0;
  if (xvis.depth == 24 || xvis.depth == 32)
    pfmt.PixelBytes = 4;
  else if (pfmt.PalEntries)
    pfmt.PixelBytes = 1;		// Palette mode
  else
    pfmt.PixelBytes = 2;		// Truecolor mode

  // Allocate the palette if not truecolor or if simulating truecolor on 8-bit display.
  if (pfmt.PalEntries)
    cmap = XCreateColormap (dpy, RootWindow (dpy, screen_num), xvis.visual, AllocAll);
  else
    cmap = 0;

  // If in 16-bit mode, redirect drawing routines
  if (pfmt.PixelBytes == 2)
  {
    _DrawPixel = DrawPixel16;
    _WriteString = WriteString16;
    _GetPixelAt = GetPixelAt16;
  }
  else if (pfmt.PixelBytes == 4)
  {
    _DrawPixel = DrawPixel32;
    _WriteString = WriteString32;
    _GetPixelAt = GetPixelAt32;
  } /* endif */

  return true;
}

bool csGraphics2DLineXLib::AllocateMemory ()
{
  back = XCreatePixmap (dpy, RootWindow (dpy, screen_num), Width, Height, xvis.depth);
  XGCValues values_back;
  gc_back = XCreateGC (dpy, back, 0, &values_back);
  XSetForeground (dpy, gc_back, BlackPixel (dpy, screen_num));
  XSetLineAttributes (dpy, gc_back, 0, LineSolid, CapButt, JoinMiter);

  Memory = new unsigned char [Width*Height*pfmt.PixelBytes];

  if (!Memory)
    return false;

  return true;
}

void csGraphics2DLineXLib::DeAllocateMemory ()
{
  if (back)
  {
    XFreePixmap (dpy, back);
    back = 0;
  }
  if (Memory)
  {
    delete [] Memory;
    Memory = NULL;
  }
}

void csGraphics2DLineXLib::Close ()
{
  if (!is_open) return;

  if (xwin)
    xwin->Close ();

  if (back)
  {
    XFreePixmap (dpy, back);
    back = 0;
  }
  if (Memory)
  {
    delete [] Memory;
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

int cmp_palent_line (const void* p1, const void* p2)
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

int find_rgb_palent_line (palent* pe, int r, int g, int b)
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
  return csGraphics2D::BeginDraw ();
}

bool csGraphics2DLineXLib::PerformExtensionV (char const* command, va_list)
{
  if (!strcasecmp (command, "fullscreen"))
  {
    xwin->SetFullScreen (!xwin->GetFullScreen ());
    return true;
  }
  else if (!strcasecmp (command, "flush"))
  {
    XSync (dpy, False);
    return true;
  }
  return true;
}

void csGraphics2DLineXLib::Print (csRect* /*area*/)
{
  XFlush (dpy); XSync (dpy, false);

//usleep (5000);
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
  if (seg_color != color)
  {
    XSetForeground (dpy, gc_back, seg_color);
    XDrawSegments (dpy, back, gc_back, segments, nr_segments);
    nr_segments = 0;
    seg_color = color;
  }
  else if (nr_segments >= 20)
  {
    XSetForeground (dpy, gc_back, seg_color);
    XDrawSegments (dpy, back, gc_back, segments, nr_segments);
    nr_segments = 0;
  }
  segments[nr_segments].x1 = (int)x1;
  segments[nr_segments].y1 = (int)y1;
  segments[nr_segments].x2 = (int)x2;
  segments[nr_segments].y2 = (int)y2;
  nr_segments++;
}

void csGraphics2DLineXLib::Write (iFont *font, int x, int y, int fg, int bg,
  const char *text)
{
  int FontW, FontH;
  XFontStruct *xfont = (XFontStruct *)font->GetGlyphBitmap (' ', FontW, FontH);
  XSetFont (dpy, gc_back, xfont->fid);

  if (bg >= 0)
    DrawBox (x, y, XTextWidth (xfont, text, strlen (text)), FontH, bg);
  XSetForeground (dpy, gc_back, fg);
  XSetBackground (dpy, gc_back, fg); // `fg' rather than `bg' here is not an error
  XDrawString (dpy, back, gc_back, x, y + xfont->ascent, text, strlen (text));
}

void csGraphics2DLineXLib::Clear (int color)
{
  XSetForeground (dpy, gc_back, color);
  XSetBackground (dpy, gc_back, color);
  XFillRectangle (dpy, back, gc_back, 0, 0, Width, Height);
}

void csGraphics2DLineXLib::DrawBox (int x, int y, int w, int h, int color)
{
  XSetForeground (dpy, gc_back, color);
  XSetBackground (dpy, gc_back, color);
  XFillRectangle (dpy, back, gc_back, x, y, w, h);
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

bool csGraphics2DLineXLib::Resize (int width, int height)
{
  if (!is_open)
    return csGraphics2D::Resize (width, height);
  if (!AllowResizing)
    return false;
  if (!csGraphics2D::Resize (width, height))
    return false;

  XSync (dpy, False);
  DeAllocateMemory ();
  if (!AllocateMemory())
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Unable to allocate memory!");
    return false;
  }
  EventOutlet->Broadcast (cscmdContextResize, (iGraphics2D *)this);

  return true;
}

void csGraphics2DLineXLib::SetFullScreen (bool yesno)
{ 
  csGraphics2D::SetFullScreen (yesno); 
  xwin->SetFullScreen (yesno); 
}

void csGraphics2DLineXLib::AllowResize (bool iAllow)
{ 
  AllowResizing = iAllow; 
  xwin->AllowResize (iAllow); 
}

//--------------------------------------------- The dummy font server --------//

SCF_IMPLEMENT_IBASE (csLineX2DFontServer::csLineX2DFont)
  SCF_IMPLEMENTS_INTERFACE (iFont)
SCF_IMPLEMENT_IBASE_END

csLineX2DFontServer::csLineX2DFont::csLineX2DFont ()
{
  SCF_CONSTRUCT_IBASE (NULL);
}

csLineX2DFontServer::csLineX2DFont::~csLineX2DFont ()
{
  if (xfont)
    XFreeFont (csGraphics2DLineXLib::dpy, xfont);
}

void csLineX2DFontServer::csLineX2DFont::Load ()
{
  xfont = XLoadQueryFont (csGraphics2DLineXLib::dpy,
    "-*-helvetica-bold-r-*-*-12-*-*-*-*-*-*-*");
  if (!xfont) return;
  FontW = xfont->max_bounds.width;
  FontH = xfont->ascent + xfont->descent;
}

bool csLineX2DFontServer::csLineX2DFont::GetGlyphSize (uint8 c, int &oW, int &oH)
{
  XCharStruct *cs = &xfont->per_char [c - xfont->min_char_or_byte2];
  oW = cs->width;
  oH = cs->ascent + cs->descent;
  return true;
}

uint8 *csLineX2DFontServer::csLineX2DFont::GetGlyphBitmap (uint8 c, int &oW, int &oH)
{
  (void)c;
  oW = FontW; oH = FontH;
  return (uint8 *)xfont;
}

void csLineX2DFontServer::csLineX2DFont::GetDimensions (const char *text, int &oW, int &oH)
{
  oW = XTextWidth (xfont, text, strlen (text));
  oH = FontH;
}

int csLineX2DFontServer::csLineX2DFont::GetLength (const char *text, int maxwidth)
{
  int i;
  for (i = 0; text [i]; i++)
    if (XTextWidth (xfont, text, i + 1) > maxwidth)
      return i;
  return i;
}

//----------

csLineX2DFontServer::csLineX2DFontServer (iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  font.xfont = NULL;
}

iFont *csLineX2DFontServer::LoadFont (const char *filename)
{
  (void)filename;
  if (!font.xfont)
    font.Load ();
  if (font.xfont)
  {
    font.IncRef ();
    return &font;
  }
  return NULL;
}
