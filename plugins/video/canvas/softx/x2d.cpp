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
#include "cssysdef.h"
#include "csutil/sysfunc.h"
#include "x2d.h"
#include "csgeom/csrect.h"
#include "csutil/cfgacc.h"
#include "csutil/event.h"
#include "iutil/plugin.h"
#include "iutil/cfgmgr.h"
#include "iutil/cmdline.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csGraphics2DXLib)


SCF_IMPLEMENT_IBASE_EXT (csGraphics2DXLib)
  SCF_IMPLEMENTS_INTERFACE (iEventPlug)
SCF_IMPLEMENT_IBASE_EXT_END



#define CS_XWIN_SCF_ID "crystalspace.window.x"
#define CS_XEXT_SHM_SCF_ID "crystalspace.window.x.extshm"
#define CS_XEXT_SHM "MIT-SHM"


csGraphics2DXLib::csGraphics2DXLib (iBase *iParent) :
  csGraphics2D (iParent), xim (0),
  dpy (0), cmap (0), real_Memory (0),
  sim_lt8 (0), sim_lt16 (0)
{
  EventOutlet = 0;
}

csGraphics2DXLib::~csGraphics2DXLib(void)
{
  xshm = 0;

  Close();

  delete [] sim_lt8;
  delete [] sim_lt16;
}

void csGraphics2DXLib::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (CS_QUERY_REGISTRY (object_reg, iReporter));
  if (rep)
    rep->ReportV (severity, "crystalspace.canvas.softx", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

bool csGraphics2DXLib::Initialize (iObjectRegistry *object_reg)
{
  if (!csGraphics2D::Initialize (object_reg))
    return false;

  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (object_reg, iPluginManager));

  xwin = CS_LOAD_PLUGIN (plugin_mgr, CS_XWIN_SCF_ID, iXWindow);
  if (!xwin)
  {
    Report (CS_REPORTER_SEVERITY_ERROR,
	      "Couldn't find 'xwin' plugin!");
    return false;
  }
  dpy = xwin->GetDisplay ();
  screen_num = xwin->GetScreen ();

  bool do_shm;
  // Query system settings
  csConfigAccess Config(object_reg, "/config/video.cfg");
  csRef<iCommandLineParser> cmdline (CS_QUERY_REGISTRY (object_reg,
						   iCommandLineParser));
  sim_depth = Config->GetInt ("Video.SimulateDepth", 0);

  do_shm = Config->GetBool ("Video.XSHM", true);
  if (cmdline->GetOption ("XSHM")) do_shm = true;
  if (cmdline->GetOption ("noXSHM")) do_shm = false;

  if (do_shm)
  {
    int opcode, first_event, first_error;
    if (XQueryExtension (dpy, CS_XEXT_SHM,
			&opcode, &first_event, &first_error))
    {
      xshm = CS_LOAD_PLUGIN (plugin_mgr, CS_XEXT_SHM_SCF_ID, iXExtSHM);
      if (xshm)
	xshm->SetDisplayScreen (dpy, screen_num);
    }
    else
    {
      Report (CS_REPORTER_SEVERITY_WARNING,
	      "No shared memory X-extension detected....disabling\n");
    }
  }

  csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
  if (q != 0)
  {
    // Tell event queue to call us on broadcast messages
    q->RegisterListener (scfiEventHandler, CSMASK_Broadcast);
    // Create the event outlet
    EventOutlet = q->CreateEventOutlet (this);
  }
  return true;
}

bool csGraphics2DXLib::Open()
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

  Report (CS_REPORTER_SEVERITY_NOTIFY, "Crystal Space X windows driver");
  if (xshm)
    Report (CS_REPORTER_SEVERITY_NOTIFY, "(Using SHM extension plugin)");

  Report (CS_REPORTER_SEVERITY_NOTIFY, "Using %u bit %sColor visual",
              xvis.depth, (xvis.visual->c_class == PseudoColor) ? "Pseudo" : "True");

  // Open your graphic interface
  if (!csGraphics2D::Open ())
    return false;

  if (!AllocateMemory ())
    return false;

  Clear (0);
  return true;
}

void csGraphics2DXLib::Close ()
{
  if (!is_open) return;

  if (xshm)
    xshm->DestroyMemory (); // decref before we close the win since we use the dpy in xshm

  if (xwin)
    xwin->Close ();

  if (Memory && sim_depth && !xshm)
  {
    delete [] Memory;
    Memory = 0;
  }

  csGraphics2D::Close ();
}

bool csGraphics2DXLib::CreateVisuals ()
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
  pfmt.AlphaMask = 0;

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

  // First check if we can disable simulation because the format is the same.
  if ((sim_depth == 8 && pfmt.PixelBytes == 1)
   || (sim_depth == 15 && pfmt.PixelBytes == 2 && pfmt.RedMask == (0x1f<<10))
   || (sim_depth == 16 && pfmt.PixelBytes == 2 && pfmt.RedMask == (0x1f<<11))
   || (sim_depth == 32 && pfmt.PixelBytes == 4))
    sim_depth = 0;

  // If we are going to simulate another depth then the real information
  // is put aside and we will fill pfmt with faked values.
  if (sim_depth != 0)
  {
    real_pfmt = pfmt;
    switch (sim_depth)
    {
      case 8:
        pfmt.RedMask = pfmt.GreenMask = pfmt.BlueMask = pfmt.AlphaMask = 0;
      	pfmt.PalEntries = 256;
      	pfmt.PixelBytes = 1;
      	break;
      case 15:
      	pfmt.RedMask   = 0x1f << 10;
      	pfmt.GreenMask = 0x1f << 5;
      	pfmt.BlueMask  = 0x1f;
        pfmt.AlphaMask = 0;
      	pfmt.PalEntries = 0;
      	pfmt.PixelBytes = 2;
      	break;
      case 16:
      	pfmt.RedMask   = 0x1f << 11;
      	pfmt.GreenMask = 0x3f << 5;
      	pfmt.BlueMask  = 0x1f;
        pfmt.AlphaMask = 0;
      	pfmt.PalEntries = 0;
      	pfmt.PixelBytes = 2;
      	break;
      case 32:
      	pfmt.RedMask = 0xff << 16;
      	pfmt.GreenMask = 0xff << 8;
      	pfmt.BlueMask = 0xff;
        pfmt.AlphaMask = 0xff << 24;
      	pfmt.PalEntries = 0;
      	pfmt.PixelBytes = 4;
      	break;
    }
    pfmt.complete ();
  }

  // Allocate the palette if not truecolor or if
  // simulating truecolor on 8-bit display.
  if ((sim_depth == 0 && pfmt.PalEntries) ||
      (sim_depth != 0 && real_pfmt.PalEntries))
    cmap = XCreateColormap (dpy, RootWindow (dpy, screen_num),
			    xvis.visual, AllocAll);

  // If we are simulating truecolor on an 8-bit display then we will create
  // a truecolor 3:3:2 colormap. This is ugly but simulated depth is for
  // testing only. So who cares?
  if ((sim_depth == 15 || sim_depth == 16 || sim_depth == 32) && cmap)
  {
    int i;
    for (i = 0 ; i < 256 ; i++)
    {
      XColor color;
      color.pixel = i;
      color.red   = (((i&0xe0)>>5)<<5)*256;
      color.green = (((i&0x1c)>>2)<<5)*256;
      color.blue  = ((i&0x3)<<6)*256;
      color.flags = DoRed | DoGreen | DoBlue;
      XStoreColor (dpy, cmap, &color);
    }
  }

  // Create the simulated lookup tables if any.
  if (sim_depth == 15 && real_pfmt.PixelBytes == 1)
  {
    // Simulate 15-bit on 8-bit.
    sim_lt8 = new unsigned char [65536];
    int i, r, g, b;
    for (i = 0 ; i < 32768 ; i++)
    {
      r = (i&0x7c00)>>10; r >>= 2;
      g = (i&0x03e0)>>5;  g >>= 2;
      b = (i&0x001f);     b >>= 3;
      sim_lt8[i] = (r<<5) | (g<<2) | b;
    }
  }
  else if ((sim_depth == 16 || sim_depth == 32) && real_pfmt.PixelBytes == 1)
  {
    // Simulate 16-bit on 8-bit.
    sim_lt8 = new unsigned char [65536];
    int i, r, g, b;
    for (i = 0 ; i < 65536 ; i++)
    {
      r = (i&0xf800)>>11; r >>= 2;
      g = (i&0x07e0)>>5;  g >>= 3;
      b = (i&0x001f);     b >>= 3;
      sim_lt8[i] = (r<<5) | (g<<2) | b;
    }
  }
  else if (sim_depth == 32 && real_pfmt.PixelBytes == 2)
  {
    // Simulate 32-bit on 16-bit.
    // No lookup tables needed.
  }

  // If in 16-bit mode, redirect drawing routines
  if (pfmt.PixelBytes == 2)
  {
    _DrawPixel = DrawPixel16;
    _GetPixelAt = GetPixelAt16;
  }
  else if (pfmt.PixelBytes == 4)
  {
    _DrawPixel = DrawPixel32;
    _GetPixelAt = GetPixelAt32;
  } /* endif */

  return true;
}

struct palent
{
  uint16 idx;
  unsigned char r, g, b;
  int cnt;
};

static int cmp_palent (const void* p1, const void* p2)
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

static int find_rgb_palent (palent* pe, int r, int g, int b)
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

void csGraphics2DXLib::restore_332_palette ()
{
  if ((sim_depth != 15 && sim_depth != 16 && sim_depth != 32)
   || (real_pfmt.PixelBytes != 1))
    return;

  Report (CS_REPORTER_SEVERITY_DEBUG, "Restore 3:3:2 palette");

  int i, r, g, b;

  if (sim_depth == 15)
    for (i = 0 ; i < 32768 ; i++)
    {
      r = (i&0x7c00)>>10; r >>= 2;
      g = (i&0x03e0)>>5;  g >>= 2;
      b = (i&0x001f);     b >>= 3;
      sim_lt8[i] = (r<<5) | (g<<2) | b;
    }
  else
    for (i = 0 ; i < 65536 ; i++)
    {
      r = (i&0xf800)>>11; r >>= 2;
      g = (i&0x07e0)>>5;  g >>= 3;
      b = (i&0x001f);     b >>= 3;
      sim_lt8[i] = (r<<5) | (g<<2) | b;
    }

  for (i = 0 ; i < 256 ; i++)
  {
    XColor color;
    color.pixel = i;
    color.red   = (((i&0xe0)>>5)<<5)*256;
    color.green = (((i&0x1c)>>2)<<5)*256;
    color.blue  = ((i&0x3)<<6)*256;
    color.flags = DoRed | DoGreen | DoBlue;
    XStoreColor (dpy, cmap, &color);
  }
  Report (CS_REPORTER_SEVERITY_DEBUG, "Done!");
}

void csGraphics2DXLib::recompute_simulated_palette ()
{
  if ((sim_depth != 15 && sim_depth != 16 && sim_depth != 32)
   || (real_pfmt.PixelBytes != 1))
    return;

  Report (CS_REPORTER_SEVERITY_DEBUG, "Recompute simulated palette");

  palent* pe;
  int i;

  // Create a palette of 16K entries
  pe = new palent [65536];
  if (sim_depth == 32)
    for (i = 0 ; i < 65536 ; i++)
    {
      pe[i].idx = i;
      pe[i].r = ((i&0xf800) >> 11) << 3;
      pe[i].g = ((i&0x07e0) >> 5) << 2;
      pe[i].b = ((i&0x001f) >> 0) << 3;
      pe[i].cnt = 0;
    }
  else
    for (i = 0 ; i < 65536 ; i++)
    {
      pe[i].idx = i;
      pe[i].r = ((i&pfmt.RedMask) >> pfmt.RedShift) << (8-pfmt.RedBits);
      pe[i].g = ((i&pfmt.GreenMask) >> pfmt.GreenShift) << (8-pfmt.GreenBits);
      pe[i].b = ((i&pfmt.BlueMask) >> pfmt.BlueShift) << (8-pfmt.BlueBits);
      pe[i].cnt = 0;
    }

  // Count how many times each color appears.
  if (sim_depth == 15 || sim_depth == 16)
  {
    uint16* m = (uint16*)Memory;
    i = Width*Height;
    while (i > 0)
    {
      pe[*m++].cnt++;
      i--;
    }
  }
  else if (sim_depth == 32)
  {
    uint32* m = (uint32*)Memory;
    i = Width*Height;
    while (i > 0)
    {
      uint32 col = *m++;
      uint16 scol =
      	(((col & pfmt.RedMask) >> (pfmt.RedShift+3)) << 11) |
      	(((col & pfmt.GreenMask) >> (pfmt.GreenShift+2)) << 5) |
      	(((col & pfmt.BlueMask) >> (pfmt.BlueShift+3)));
      pe[scol].cnt++;
      i--;
    }
  }

  // Fake that color 0 (black) is used very much so that
  // it will appear in front of all colors after sorting.
  pe[0].cnt = Width*Height+1;

  // Sort based on count.
  qsort ((void*)pe, 65536, sizeof (palent), cmp_palent);

  palent* pe_new = new palent[257];
  pe_new[0].r = 0;
  pe_new[0].g = 0;
  pe_new[0].b = 0;
  pe_new[0].cnt = 1;

  int j = 1;
  for (i = 1 ; i < 65536 ; i++)
  {
    int r = pe[i].r;
    int g = pe[i].g;
    int b = pe[i].b;
    int k, min, mindist;
    mindist = 1000*256*256;
    min = -1;
    register int dr, dg, db, dist;
    for (k = 0 ; k < j ; k++)
    {
      dr = r - pe_new[k].r;
      dg = g - pe_new[k].g;
      db = b - pe_new[k].b;
      dist = (299*dr*dr) + (587*dg*dg) + (114*db*db);
      if (dist == 0) { min = k; mindist = dist; break; }
      if (dist < mindist) { mindist = dist; min = k; }
    }
    if (mindist >= 16333)
    {
      pe_new[j].r = r;
      pe_new[j].g = g;
      pe_new[j].b = b;
      pe_new[j].cnt = 1;
      j++;
      if (j >= 255) break;
    }
  }

  // Fake that color 255 (white) is set to white.
  pe_new[255].r = 255;
  pe_new[255].g = 255;
  pe_new[255].b = 255;

  Report (CS_REPORTER_SEVERITY_DEBUG, "Recomputing lookup table...");

  // Recompute the lookup table from 15/16-bit to 8-bit.
  int r, g, b;
  if (sim_depth == 15)
    for (i = 0 ; i < 65536 ; i++)
    {
      r = (i&0x7c00)>>10; r <<= 3;
      g = (i&0x03e0)>>5;  g <<= 3;
      b = (i&0x001f);     b <<= 3;
      sim_lt8[i] = find_rgb_palent (pe_new, r, g, b);
    }
  else
    for (i = 0 ; i < 65536 ; i++)
    {
      r = (i&0xf800)>>11; r <<= 3;
      g = (i&0x07e0)>>5;  g <<= 2;
      b = (i&0x001f);     b <<= 3;
      sim_lt8[i] = find_rgb_palent (pe_new, r, g, b);
    }

  // Allocate colors for the 256 most often occuring
  // colors on the display. This also makes the first 256
  // elements of pe the palette.
  for (i = 0 ; i < 256 ; i++)
  {
    XColor color;
    color.pixel = i;
    color.red   = pe_new[i].r*256;
    color.green = pe_new[i].g*256;
    color.blue  = pe_new[i].b*256;
    color.flags = DoRed | DoGreen | DoBlue;
    XStoreColor (dpy, cmap, &color);
  }

  delete [] pe;
  delete [] pe_new;
  Report (CS_REPORTER_SEVERITY_DEBUG, "Done!");
}

void csGraphics2DXLib::recompute_grey_palette ()
{
  if ((sim_depth != 15 && sim_depth != 16 && sim_depth != 32)
   || (real_pfmt.PixelBytes != 1))
    return;

  Report (CS_REPORTER_SEVERITY_DEBUG, "Compute grey palette");

  palent* pe;
  int i;

  // Create a grey palette
  pe = new palent [256];
  for (i = 0 ; i < 256 ; i++)
  {
    pe[i].idx = i;
    pe[i].r = i;
    pe[i].g = i;
    pe[i].b = i;
    pe[i].cnt = 1;
  }

  Report (CS_REPORTER_SEVERITY_DEBUG, "Recomputing lookup table...");

  // Recompute the lookup table from 15/16-bit to 8-bit.
  int r, g, b;
  if (sim_depth == 15)
    for (i = 0 ; i < 65536 ; i++)
    {
      r = (i&0x7c00)>>10; r <<= 3;
      g = (i&0x03e0)>>5;  g <<= 3;
      b = (i&0x001f);     b <<= 3;
      sim_lt8[i] = find_rgb_palent (pe, r, g, b);
    }
  else
    for (i = 0 ; i < 65536 ; i++)
    {
      r = (i&0xf800)>>11; r <<= 3;
      g = (i&0x07e0)>>5;  g <<= 2;
      b = (i&0x001f);     b <<= 3;
      sim_lt8[i] = find_rgb_palent (pe, r, g, b);
    }

  // Allocate colors for the 256 most often occuring
  // colors on the display. This also makes the first 256
  // elements of pe the palette.
  for (i = 0 ; i < 256 ; i++)
  {
    XColor color;
    color.pixel = i;
    color.red   = pe[i].r*256;
    color.green = pe[i].g*256;
    color.blue  = pe[i].b*256;
    color.flags = DoRed | DoGreen | DoBlue;
    XStoreColor (dpy, cmap, &color);
  }

  delete [] pe;
  Report (CS_REPORTER_SEVERITY_DEBUG, "Done!");
}

void csGraphics2DXLib::SetRGB(int i, int r, int g, int b)
{
  // If there is a colormap AND we are not simulating a display with
  // a colormap then we really set the color.
  if (cmap && !sim_depth)
  {
    XColor color;
    color.pixel = i;
    color.red = r*256;
    color.green = g*256;
    color.blue = b*256;
    color.flags = DoRed | DoGreen | DoBlue;

    XStoreColor (dpy, cmap, &color);
  }

  // If we are simulating an 8-bit display on truecolor displays then we
  // need to delete the lookup table. It has to be recomputed after changing
  // a palette entry.
  if (sim_depth == 8)
    if (sim_lt16)
    {
      delete [] sim_lt16;
      sim_lt16 = 0;
    }

  csGraphics2D::SetRGB (i, r, g, b);
}

void csGraphics2DXLib::Print (csRect const* area)
{
  // If we won't tell X-Windows to flush, the applications starts to response
  // slowly in some circumstances (on some machines, OSes, configurations etc)
  // Since we flush at START of this routine, we're actually flushing PREVIOUS
  // frame, thus after doing XPutImage the server gets a chance (during
  // computation of next frame) to flush the image without our "help".
  XFlush (dpy); XSync (dpy, false);

  if (sim_depth)
  {
    if (sim_depth == 16 && real_pfmt.PixelBytes == 4)
    {
      uint16* src = (uint16*)Memory;
      uint32* dst = (uint32*)real_Memory;
      int i = Width * Height;
      while (i > 0)
      {
        uint16 rgb = *src++;
        *dst++ = (((rgb>>11)&31)<<(real_pfmt.RedShift+3)) | (((rgb>>5)&63)<<(real_pfmt.GreenShift+2)) | (((rgb>>0)&31)<<(real_pfmt.BlueShift+3));
        i--;
      }
    }
    else if (sim_depth == 15 && real_pfmt.PixelBytes == 4)
    {
      uint16* src = (uint16*)Memory;
      uint32* dst = (uint32*)real_Memory;
      int i = Width * Height;
      while (i > 0)
      {
        uint16 rgb = *src++;
        *dst++ = (((rgb>>10)&31)<<(real_pfmt.RedShift+3)) | (((rgb>>5)&31)<<(real_pfmt.GreenShift+3)) | (((rgb>>0)&31)<<(real_pfmt.BlueShift+3));
        i--;
      }
    }
    else if (sim_depth == 16 && real_pfmt.PixelBytes == 2)
    {
      uint16* src = (uint16*)Memory;
      uint16* dst = (uint16*)real_Memory;
      int i = Width * Height;
      while (i > 0)
      {
        uint16 rgb = *src++;
        *dst++ = (((rgb>>11)&31)<<10) | (((rgb>>6)&31)<<5) | (((rgb>>0)&31)<<0);
        i--;
      }
    }
    else if (sim_depth == 15 && real_pfmt.PixelBytes == 2)
    {
      uint16* src = (uint16*)Memory;
      uint16* dst = (uint16*)real_Memory;
      int i = Width * Height;
      while (i > 0)
      {
        uint16 rgb = *src++;
        *dst++ = (((rgb>>10)&31)<<11) | (((rgb>>5)&31)<<(5+1)) | (((rgb>>0)&31)<<0);
        i--;
      }
    }
    else if (sim_depth == 15 || sim_depth == 16)
    {
      uint16* src = (uint16*)Memory;
      unsigned char* dst = real_Memory;
      int i = Width * Height;
      while (i > 0)
      {
        *dst++ = sim_lt8[*src++];
        i--;
      }
    }
    else if (sim_depth == 32 && real_pfmt.PixelBytes == 1)
    {
      uint32* src = (uint32*)Memory;
      unsigned char* dst = real_Memory;
      int i = Width*Height;
      while (i > 0)
      {
        uint32 col = *src++;
        uint16 scol =
      	  (((col & pfmt.RedMask) >> (pfmt.RedShift+3)) << 11) |
      	  (((col & pfmt.GreenMask) >> (pfmt.GreenShift+2)) << 5) |
      	  (((col & pfmt.BlueMask) >> (pfmt.BlueShift+3)));
        *dst++ = sim_lt8[scol];
        i--;
      }
    }
    else if (sim_depth == 32 && real_pfmt.PixelBytes == 2)
    {
      uint32* src = (uint32*)Memory;
      uint16* dst = (uint16*)real_Memory;
      int i = Width*Height;
      while (i > 0)
      {
        uint32 col = *src++;
        uint16 scol =
      	  (((col & pfmt.RedMask) >> (pfmt.RedShift+(8-real_pfmt.RedBits))) << real_pfmt.RedShift) |
      	  (((col & pfmt.GreenMask) >> (pfmt.GreenShift+(8-real_pfmt.GreenBits))) << real_pfmt.GreenShift) |
      	  (((col & pfmt.BlueMask) >> (pfmt.BlueShift+(8-real_pfmt.BlueBits))) << real_pfmt.BlueShift);
        *dst++ = scol;
        i--;
      }
    }
    else if (sim_depth == 8)
    {
      // If it is the first time we come here we will create
      // the lookup-table from 8-bit to 15/16-bit. We can only do
      // it here because the palette can change (gamma correction).
      // If the palette changes this lookup table is automatically made
      // invalid (by SetRGB).
      // In case we are in 32-bit mode we also create the lookup-table
      // from 8-bit to 16-bit and we change 16-bit to 32-bit on the fly.
      if (!sim_lt16)
      {
        sim_lt16 = new uint16 [256];
	int i, r, g, b;
	for (i = 0 ; i < 256 ; i++)
	{
	  r = Palette[i].red;
	  g = Palette[i].green;
	  b = Palette[i].blue;
          if (real_pfmt.PixelBytes == 2)
	    sim_lt16[i] =
		((r >> (8-real_pfmt.RedBits))   << real_pfmt.RedShift) |
		((g >> (8-real_pfmt.GreenBits)) << real_pfmt.GreenShift) |
		((b >> (8-real_pfmt.BlueBits))  << real_pfmt.BlueShift);
	  else
	    sim_lt16[i] =
		((r >> (8-5)) << 11) |
		((g >> (8-6)) << 5) |
		((b >> (8-5)) << 0);
        }
      }
      if (real_pfmt.PixelBytes == 2)
      {
        unsigned char* src = Memory;
        uint16* dst = (uint16*)real_Memory;
        int i = Width*Height;
        while (i > 0)
        {
          *dst++ = sim_lt16[*src++];
          i--;
        }
      }
      else if (real_pfmt.PixelBytes == 4)
      {
        unsigned char* src = Memory;
        uint32* dst = (uint32*)real_Memory;
        int i = Width*Height;
        while (i > 0)
        {
          uint16 rgb = sim_lt16[*src++];
          *dst++ = (((rgb>>11)&31)<<(real_pfmt.RedShift+3)) | (((rgb>>5)&63)<<(real_pfmt.GreenShift+2)) | (((rgb>>0)&31)<<(real_pfmt.BlueShift+3));
          i--;
        }
      }
    }
  } // end  if (sim_depth)

  if (xshm)
    xshm->Print (window, gc, area);
  else if (area)
    XPutImage (dpy, window, gc, xim,
	       area->xmin, area->ymin, area->xmin, area->ymin,
	       area->Width (), area->Height ());
  else
    XPutImage (dpy, window, gc, xim, 0, 0, 0, 0, Width, Height);
}

bool csGraphics2DXLib::AllocateMemory ()
{
  bool mem_valid = TryAllocateMemory ();
  if (!mem_valid && xshm)
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY,
	    "SHM available but could not allocate. Trying without SHM.");
    xshm = 0;
    mem_valid = TryAllocateMemory();
  }

  if (!mem_valid)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Unable to allocate memory!");
    return false;
  }
  return true;
}

bool csGraphics2DXLib::TryAllocateMemory ()
{
  if (xshm)
    real_Memory = xshm->CreateMemory (Width, Height);
  else
  {
    int disp_depth = DefaultDepth(dpy,screen_num);
    int bitmap_pad = (disp_depth + 7) / 8;
    bitmap_pad = (bitmap_pad == 3) ? 32 : bitmap_pad*8;
    xim = XCreateImage(dpy, DefaultVisual(dpy,screen_num),
		       disp_depth, ZPixmap, 0, 0,
		       Width, Height, bitmap_pad, 0);
    xim->data = new char[xim->bytes_per_line*xim->height];
    real_Memory = (unsigned char*)(xim->data);
  }
  if (!real_Memory)
    return false;

  // If not simulating depth then Memory is equal to real_Memory.
  // If simulating then we allocate a new Memory array in the faked format.
  if (!sim_depth)
    Memory = real_Memory;
  else
    Memory = new unsigned char [Width * Height * pfmt.PixelBytes];

  return true;
}

bool csGraphics2DXLib::Resize (int width, int height)
{
  if (!is_open)
    return csGraphics2D::Resize (width, height);

  if (!AllowResizing)
    return false;

  csGraphics2D::Resize (width, height);

  if (xshm)
    xshm->DestroyMemory ();
  else
  {
    delete [] real_Memory;
    XDestroyImage (xim);
    xim = 0;
  }
  if (!AllocateMemory())
    return false;
//    Clear (0);
//    XSync (dpy, false);
  EventOutlet->Broadcast (cscmdContextResize, (intptr_t)this);
  return true;
}

void csGraphics2DXLib::SetFullScreen (bool yesno)
{
  csGraphics2D::SetFullScreen (yesno);
  xwin->SetFullScreen (yesno);
}

void csGraphics2DXLib::AllowResize (bool iAllow)
{
  AllowResizing = iAllow;
  xwin->AllowResize (iAllow);
}


bool csGraphics2DXLib::HandleEvent (iEvent &Event)
{
  if ((Event.Type == csevBroadcast)
      && (csCommandEventHelper::GetCode(&Event) == cscmdCommandLineHelp)
      && object_reg)
  {
    csPrintf ("Options for X-Windows 2D graphics driver:\n");
    csPrintf ("  -sdepth=<depth>    set simulated depth (8, 15, 16, or 32) (default=none)\n");
    csPrintf ("  -XSHM/noXSHM       SHM extension (default '%sXSHM')\n",
      xshm ? "" : "no");
    return true;
  }
  return false;
}

bool csGraphics2DXLib::PerformExtensionV (char const* command, va_list)
{
  if (!strcasecmp (command, "sim_pal"))
  {
    recompute_simulated_palette ();
    return true;
  }
  else if (!strcasecmp (command, "sim_grey"))
  {
    recompute_grey_palette ();
    return true;
  }
  else if (!strcasecmp (command, "sim_332"))
  {
    restore_332_palette ();
    return true;
  }
  else if (!strcasecmp (command, "fullscreen"))
  {
    xwin->SetFullScreen (!xwin->GetFullScreen ());
    return true;
  }
  else if (!strcasecmp (command, "flush"))
  {
    XSync (dpy, False);
    return true;
  }
  return false;
}

#undef CS_XWIN_SCF_ID
#undef CS_XEXT_SHM
