/*
    Copyright (C) 2000 by Andrew Zabolotny

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

#include "sysdef.h"
#include "csutil/csrect.h"
#include "isystem.h"

// Some mumbo-jumbo with CHK since MGL also defines this macros :-)
#undef CHK
#include "mgl2d.h"
#include "mgl/mgl40.h"
#undef CHK
#define CHK(x) x

IMPLEMENT_FACTORY (csGraphics2DMGL)

EXPORT_CLASS_TABLE (mgl2d)
  EXPORT_CLASS (csGraphics2DMGL, "crystalspace.graphics2d.mgl",
    "SciTech MGL 2D graphics driver for Crystal Space")
EXPORT_CLASS_TABLE_END

IMPLEMENT_IBASE (csGraphics2DMGL)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iGraphics2D)
IMPLEMENT_IBASE_END

// csGraphics2DMGL functions
csGraphics2DMGL::csGraphics2DMGL (iBase *iParent) :
  csGraphics2D ()
{
  CONSTRUCT_IBASE (iParent);
  dc = backdc = NULL;
}

bool csGraphics2DMGL::Initialize (iSystem *pSystem)
{
  if (!csGraphics2D::Initialize (pSystem))
    return false;

  if (MGL_init (".", NULL) == 0)
  {
    System->Printf (MSG_FATAL_ERROR, MGL_errorMsg (MGL_result ()));
    return false;
  }

  int video_mode = MGL_findMode (Width, Height, Depth);
  if (video_mode == -1)
  {
    System->Printf (MSG_FATAL_ERROR, "The mode %dx%dx%d is not available (%s)!\n",
      Width, Height, Depth, MGL_errorMsg (MGL_result ()));
    return false;
  }

  if (!FullScreen)
    MGL_changeDisplayMode (grWINDOWED);

  numPages = MGL_availablePages (mode);
  // We don't need more than 2 pages
  if (numPages > 2) numPages = 2;

  do_hwmouse = System->ConfigGetYesNo ("VideoDriver", "SystemMouseCursor", true);
  do_hwbackbuf = System->ConfigGetYesNo ("VideoDriver", "HardwareBackBuffer", true);

  return true;
}

csGraphics2DMGL::~csGraphics2DMGL ()
{
}

bool csGraphics2DMGL::Open(const char *Title)
{
  // Open your graphic interface
  if (!csGraphics2D::Open (Title))
    return false;

  if ((dc = MGL_createDisplayDC (mode, numPages, MGL_DEFAULT_REFRESH)) == NULL)
  {
    System->Printf (MSG_FATAL_ERROR, MGL_errorMsg (MGL_result ()));
    return false;
  }
  MGL_makeCurrentDC (dc);

  pixel_format_t pf;
  MGL_getPixelFormat (dc, &pf);

  pfmt.PalEntries = (Depth > 8) ? 0 : 256;
  pfmt.PixelBytes = Depth / 8;
  pfmt.RedMask = pf.redMask << pf.redPos;
  pfmt.GreenMask = pf.greenMask << pf.greenPos;
  pfmt.BlueMask = pf.blueMask << pf.bluePos;

  pfmt.complete ();

  // If in 16-bit mode, redirect drawing routines
  if (pfmt.PixelBytes == 2)
  {
    _DrawPixel = DrawPixel16;
    _WriteChar = WriteChar16;
    _GetPixelAt = GetPixelAt16;
  }
  else if (pfmt.PixelBytes == 4)
  {
    _DrawPixel = DrawPixel32;
    _WriteChar = WriteChar32;
    _GetPixelAt = GetPixelAt32;
  } /* endif */

  // Reset member variables
  videoPage = 0; allowDB = true;

  // We always use one palette in all our contexts
  MGL_checkIdentityPalette (false);
  // Allocate the back buffer
  AllocateBackBuffer ();

  return true;
}

void csGraphics2DMGL::Close ()
{
  if (dc)
  {
    DeallocateBackBuffer ();
    MGL_destroyDC (dc);
    dc = NULL;
    MGL_exit ();
  }
  csGraphics2D::Close ();
}

void csGraphics2DMGL::AllocateBackBuffer ()
{
  if (backdc)
    DeallocateBackBuffer ();

  if (do_hwbackbuf && (numPages > 1) && allowDB)
  {
    backdc = dc;
    bbtype = bbSecondVRAMPage;
    MGL_makeCurrentDC (backdc);
    MGL_setActivePage (videoPage ^ 1);
    MGL_setVisualPage (videoPage);
    return;
  }

  videoPage = 0;
  MGL_setActivePage (0);
  MGL_setVisualPage (0);

  backdc = do_hwbackbuf ? MGL_createOffscreenDC () : NULL;
  if (backdc)
  {
    // Check if this backbuffer covers our needs
    if (MGL_surfaceAccessType (backdc) != MGL_LINEAR_ACCESS)
      MGL_destroyDC (backdc), backdc = NULL;
    else
      bbtype = bbOffscreenVRAM;
  }
  if (!backdc)
  {
    backdc = MGL_createMemoryDC ();
    bbtype = bbSystemMemory;
  }

  MGL_makeCurrentDC (backdc);
}

void csGraphics2DMGL::DeallocateBackBuffer ()
{
  if (!backdc)
    return;

  if (backdc != dc)
    MGL_destroyDC (backdc);
  backdc = NULL;
}

bool csGraphics2DMGL::DoubleBuffer (bool Enable)
{
  DeallocateBackBuffer ();
  allowDB = Enable;
  AllocateBackBuffer ();
  return Enable == (bbtype == bbSecondVRAMPage);
}

bool csGraphics2DMGL::BeginDraw ()
{
  csGraphics2D::BeginDraw ();
  if (FrameBufferLocked != 1)
    return true;

  MGL_beginDirectAccess ();
  Memory = backdc->surface;

  if (LineAddress [1] != backdc->mi.bytesPerLine)
  {
    int i, addr;
    for (i = 0, addr = 0; i < Height; i++, addr += backdc->mi.bytesPerLine)
      LineAddress [i] = addr;
  }

  return true;
}

void csGraphics2DMGL::FinishDraw ()
{
  csGraphics2D::FinishDraw ();
  if (FrameBufferLocked)
    return;

  MGL_endDirectAccess ();
  Memory = NULL;
}

void csGraphics2DMGL::Print (csRect *area)
{
  csRect entire_screen (0, 0, Width, Height);
  if (!area)
    area = &entire_screen;

  if (bbtype == bbSecondVRAMPage)
  {
    videoPage ^= 1;
    MGL_setActivePage (videoPage ^ 1);
    MGL_setVisualPage (videoPage);
  }
  else
    MGL_bitBltCoord (dc, backdc,
      area.xmin, area.ymax, area.xmax, area.ymin,
      area.xmin, area.ymax, MGL_REPLACE_MODE);
}

void csGraphics2DMGL::SetRGB(int i, int r, int g, int b)
{
  csGraphics2D::SetRGB (i, r, g, b);
}

bool csGraphics2DMGL::SetMousePosition (int x, int y)
{
  return true;
}

bool csGraphics2DMGL::SetMouseCursor (csMouseCursorID iShape)
{
  if (!do_hwmouse)
    return (iShape == csmcNone);

  //@@todo
  return false;
}

void csGraphics2DMGL::ProcessEvents (void *Param)
{
}

void csGraphics2DMGL::Clear (int color)
{
  MGL_setBackColor (color);
  MGL_ClearDevice ();
}
