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
#include "cssys/csevent.h"
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
  joybutt = 0;
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

  video_mode = MGL_findMode (Width, Height, Depth);
  if (video_mode == -1)
  {
    System->Printf (MSG_FATAL_ERROR, "The mode %dx%dx%d is not available (%s)!\n",
      Width, Height, Depth, MGL_errorMsg (MGL_result ()));
    return false;
  }

  do_hwmouse = System->ConfigGetYesNo ("VideoDriver", "SystemMouseCursor", true);
  do_hwbackbuf = System->ConfigGetYesNo ("VideoDriver", "HardwareBackBuffer", true);

  // Tell system driver to call us on every frame
  System->CallOnEvents (this, CSMASK_Nothing);

  return true;
}

csGraphics2DMGL::~csGraphics2DMGL ()
{
}

// Ugly
static iSystem *SysDriver;

static int MGLAPI MGL_suspend_callback (MGLDC *dc, int flags)
{
  if (flags == MGL_DEACTIVATE)
  {
    SysDriver->SuspendResume (true);
    return MGL_SUSPEND_APP;
  }
  else if (flags == MGL_REACTIVATE)
    SysDriver->SuspendResume (false);
  return MGL_NO_SUSPEND_APP;
}

bool csGraphics2DMGL::Open (const char *Title)
{
  // Open your graphic interface
  if (!csGraphics2D::Open (Title))
    return false;

  SysDriver = System;
  MGL_setSuspendAppCallback (MGL_suspend_callback);

  numPages = MGL_availablePages (video_mode);
  // We don't need more than 2 pages
  if (numPages > 2) numPages = 2;

  if ((dc = MGL_createDisplayDC (video_mode, numPages, MGL_DEFAULT_REFRESH)) == NULL)
  {
    System->Printf (MSG_FATAL_ERROR, MGL_errorMsg (MGL_result ()));
    return false;
  }
  MGL_makeCurrentDC (dc);

  System->EnablePrintf (false);

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
  joybutt = 0; paletteChanged = true;
  memset (&joyposx, 0, sizeof (joyposx));
  memset (&joyposy, 0, sizeof (joyposy));

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

  System->EnablePrintf (true);
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
    MGL_setActivePage (dc, videoPage ^ 1);
    MGL_setVisualPage (dc, videoPage, false);
    return;
  }

  videoPage = 0;
  MGL_setActivePage (dc, 0);
  MGL_setVisualPage (dc, 0, false);

  backdc = do_hwbackbuf ? MGL_createOffscreenDC (dc, Width, Height) : NULL;
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
    pixel_format_t pf;
    MGL_getPixelFormat (dc, &pf);
    backdc = MGL_createMemoryDC (Width, Height, Depth, &pf);
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

  if (paletteChanged)
  {
    MGL_realizePalette (dc, 256, 0, false);
    paletteChanged = false;
  }

  MGL_beginDirectAccess ();
  Memory = (UByte *)backdc->surface;

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
    MGL_setActivePage (dc, videoPage ^ 1);
    MGL_setVisualPage (dc, videoPage, true);
  }
  else
    MGL_bitBltCoord (dc, backdc,
      area->xmin, area->ymax, area->xmax, area->ymin,
      area->xmin, area->ymax, MGL_REPLACE_MODE);
}

void csGraphics2DMGL::SetRGB(int i, int r, int g, int b)
{
  csGraphics2D::SetRGB (i, r, g, b);
  MGL_setPaletteEntry (dc, i, r, g, b);
  paletteChanged = true;
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

int csGraphics2DMGL::TranslateKey (int mglKey)
{
  //@@bug: ASCII keys with code 0xE0 (in particular, small cyrillic 'r')
  //are unusable. @@todo: bug Kendall Bennett about it.
  if (EVT_asciiCode (mglKey) == 0xe0
   || EVT_asciiCode (mglKey) == 0)
  {
    mglKey = EVT_scanCode (mglKey);
    int key = 0;
    switch (mglKey)
    {
      case KB_padMinus:   key = CSKEY_PADMINUS; break;
      case KB_padPlus:    key = CSKEY_PADPLUS; break;
      case KB_padTimes:   key = CSKEY_PADMULT; break;
      case KB_padDivide:  key = CSKEY_PADDIV; break;
      case KB_padLeft:
      case KB_left:       key = CSKEY_LEFT; break;
      case KB_padRight:
      case KB_right:      key = CSKEY_RIGHT; break;
      case KB_padUp:
      case KB_up:         key = CSKEY_UP; break;
      case KB_padDown:
      case KB_down:       key = CSKEY_DOWN; break;
      case KB_padInsert:
      case KB_insert:     key = CSKEY_INS; break;
      case KB_padDelete:
      case KB_delete:     key = CSKEY_DEL; break;
      case KB_padHome:
      case KB_home:       key = CSKEY_HOME; break;
      case KB_padEnd:
      case KB_end:        key = CSKEY_END; break;
      case KB_padPageUp:
      case KB_pageUp:     key = CSKEY_PGUP; break;
      case KB_padPageDown:
      case KB_pageDown:   key = CSKEY_PGDN; break;
      case KB_padCenter:  key = CSKEY_CENTER; break;
      case KB_F1:         key = CSKEY_F1; break;
      case KB_F2:         key = CSKEY_F2; break;
      case KB_F3:         key = CSKEY_F3; break;
      case KB_F4:         key = CSKEY_F4; break;
      case KB_F5:         key = CSKEY_F5; break;
      case KB_F6:         key = CSKEY_F6; break;
      case KB_F7:         key = CSKEY_F7; break;
      case KB_F8:         key = CSKEY_F8; break;
      case KB_F9:         key = CSKEY_F9; break;
      case KB_F10:        key = CSKEY_F10; break;
      case KB_F11:        key = CSKEY_F11; break;
      case KB_F12:        key = CSKEY_F12; break;
      case KB_leftShift:
      case KB_rightShift: key = CSKEY_SHIFT; break;
      case KB_leftCtrl:
      case KB_rightCtrl:  key = CSKEY_CTRL; break;
      case KB_leftAlt:
      case KB_rightAlt:   key = CSKEY_ALT; break;
    }
    return key;
  }
  else
  {
    if (EVT_asciiCode (mglKey) == '\r')
      return CSKEY_ENTER;
    return EVT_asciiCode (mglKey);
  }
}

bool csGraphics2DMGL::HandleEvent (csEvent &/*Event*/)
{
  event_t evt;

  // This is relatively slow (joystick reading)
  // and should occur only if the user really requested it
  //EVT_pollJoystick ();

  while (EVT_getNext (&evt, EVT_EVERYEVT))
  {
    switch (evt.what)
    {
      case EVT_KEYDOWN:
      case EVT_KEYREPEAT:
      case EVT_KEYUP:
        evt.message = TranslateKey (evt.message);
        if (evt.message)
          System->QueueKeyEvent (evt.message, evt.what != EVT_KEYUP);
        break;
      case EVT_MOUSEDOWN:
      case EVT_MOUSEUP:
        if (evt.message & EVT_LEFTBMASK)
          System->QueueMouseEvent (1, !(evt.modifiers & EVT_LEFTBUT),
            evt.where_x, evt.where_y);
        if (evt.message & EVT_RIGHTBMASK)
          System->QueueMouseEvent (2, !(evt.modifiers & EVT_RIGHTBUT),
            evt.where_x, evt.where_y);
        if (evt.message & EVT_MIDDLEBMASK)
          System->QueueMouseEvent (3, !(evt.modifiers & EVT_MIDDLEBUT),
            evt.where_x, evt.where_y);
        break;
      case EVT_MOUSEMOVE:
        System->QueueMouseEvent (0, false, evt.where_x, evt.where_y);
        break;
      case EVT_JOYCLICK:
        if (joybutt != evt.message)
        {
          int diff = joybutt ^ evt.message;
          joybutt = evt.message;
          if (diff & EVT_JOY1_BUTTONA)
            System->QueueJoystickEvent (1, 1, !!(joybutt & EVT_JOY1_BUTTONA),
              evt.where_x, evt.where_y);
          if (diff & EVT_JOY1_BUTTONB)
            System->QueueJoystickEvent (1, 2, !!(joybutt & EVT_JOY1_BUTTONB),
              evt.where_x, evt.where_y);
          if (diff & EVT_JOY2_BUTTONA)
            System->QueueJoystickEvent (2, 1, !!(joybutt & EVT_JOY2_BUTTONA),
              evt.relative_x, evt.relative_y);
          if (diff & EVT_JOY2_BUTTONB)
            System->QueueJoystickEvent (2, 2, !!(joybutt & EVT_JOY2_BUTTONB),
              evt.relative_x, evt.relative_y);
        }
        break;
      case EVT_JOYMOVE:
        if (joyposx [0] != evt.where_x || joyposy [0] != evt.where_y)
        {
          System->QueueJoystickEvent (1, 0, false, evt.where_x, evt.where_y);
          joyposx [0] = evt.where_x; joyposy [0] = evt.where_y;
        }
        if (joyposx [1] != evt.relative_x || joyposy [1] != evt.relative_y)
        {
          System->QueueJoystickEvent (2, 0, false, evt.relative_x, evt.relative_y);
          joyposx [1] = evt.relative_x; joyposy [1] = evt.relative_y;
        }
        break;
    }
  }
  return false;
}

void csGraphics2DMGL::Clear (int color)
{
  MGL_setBackColor (color);
  MGL_clearDevice ();
}
