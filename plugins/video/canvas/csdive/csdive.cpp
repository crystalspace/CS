/*
    OS/2 support for Crystal Space 3D library
    Copyright (C) 1998 by Andrew Zabolotny <bit@eltech.ru>

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
#include <limits.h>
#include "cssysdef.h"
#include "csutil/scf.h"
#include "csgeom/csrect.h"
#include "csutil/cfgacc.h"
#include "video/canvas/common/scancode.h"
#include "video/canvas/common/os2-keys.h"
#include "iutil/cfgfile.h"
#include "iutil/cmdline.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/csinput.h"
#include "csdive.h"
#include "libDIVE.h"
#include "libDIVEprv.h"
#include "cssys/os2/os2help.h"

#if defined (PROC_X86)
inline void memsetd (void *dest, unsigned int value, size_t count)
{
  asm
  (	"	cld;"
	"	rep;"
	"	stosl;"
	: : "D" (dest), "c" (count), "a" (value)
  );
}
#endif

//------------------------------------------------------ csGraphics2DOS2DIVE ---

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csGraphics2DOS2DIVE)

SCF_EXPORT_CLASS_TABLE (csdive)
  SCF_EXPORT_CLASS_DEP (csGraphics2DOS2DIVE, "crystalspace.graphics2d.dive",
    "OS/2 DIVE 2D graphics driver for Crystal Space", "crystalspace.font.server.")
SCF_EXPORT_CLASS_TABLE_END

SCF_IMPLEMENT_IBASE_EXT (csGraphics2DOS2DIVE)
  SCF_IMPLEMENTS_INTERFACE (iEventPlug)
SCF_IMPLEMENT_IBASE_EXT_END

csGraphics2DOS2DIVE::csGraphics2DOS2DIVE (iBase *iParent) :
  csGraphics2D (iParent),
  dblbuff (true), HardwareCursor (true),
  WindowX (INT_MIN), WindowY (INT_MIN),
  WindowWidth (-1), WindowHeight (-1)
{
  // Initialize module handle
#ifdef CS_STATIC_LINKED
  gdMH = NULLHANDLE;
#else
  extern unsigned long dll_handle;
  gdMH = dll_handle;
#endif

  ActivePage = 0;
  dW = NULL;
  EventOutlet = NULL;

  // Initialize scancode->char conversion table with additional codes
  ScanCodeToChar [SCANCODE_RALT]        = CSKEY_ALT;
  ScanCodeToChar [SCANCODE_RCTRL]       = CSKEY_CTRL;
  ScanCodeToChar [SCANCODE_GRAYUP]      = CSKEY_UP;
  ScanCodeToChar [SCANCODE_GRAYDOWN]    = CSKEY_DOWN;
  ScanCodeToChar [SCANCODE_GRAYLEFT]    = CSKEY_LEFT;
  ScanCodeToChar [SCANCODE_GRAYRIGHT]   = CSKEY_RIGHT;
  ScanCodeToChar [SCANCODE_GRAYPGUP]    = CSKEY_PGUP;
  ScanCodeToChar [SCANCODE_GRAYPGDN]    = CSKEY_PGDN;
  ScanCodeToChar [SCANCODE_GRAYINS]     = CSKEY_INS;
  ScanCodeToChar [SCANCODE_GRAYDEL]     = CSKEY_DEL;
  ScanCodeToChar [SCANCODE_GRAYHOME]    = CSKEY_HOME;
  ScanCodeToChar [SCANCODE_GRAYEND]     = CSKEY_END;
  ScanCodeToChar [SCANCODE_GRAYENTER]   = CSKEY_ENTER;
  ScanCodeToChar [SCANCODE_GRAYSLASH]   = CSKEY_PADDIV;
}

csGraphics2DOS2DIVE::~csGraphics2DOS2DIVE ()
{
  Close ();
  // Deallocate DIVE resources
  gdDiveDeinitialize ();
  if (EventOutlet)
    EventOutlet->DecRef ();
  if (KeyboardDriver)
    KeyboardDriver->DecRef ();
}

bool csGraphics2DOS2DIVE::Initialize (iObjectRegistry* object_reg)
{
  if (!csGraphics2D::Initialize (object_reg))
    return false;

  iOs2Helper* os2helper = CS_QUERY_REGISTRY (object_reg, iOs2Helper);
  CS_ASSERT (os2helper != NULL);
  os2helper->StartGUI ();

  // Initialize DIVE
  if (!gdDiveInitialize ())
  {
    printf ("Unable to initialize OS/2 DIVE\n");
    return false;
  }

  csConfigAccess Config(object_reg, "/config/video.cfg");
  WindowX = Config->GetInt ("Video.WindowX", INT_MIN);
  WindowY = Config->GetInt ("Video.WindowY", INT_MIN);
  WindowWidth = Config->GetInt ("Video.WindowWidth", -1);
  WindowHeight = Config->GetInt ("Video.WindowHeight", -1);
  HardwareCursor = Config->GetBool ("Video.SystemMouseCursor", true);
  iCommandLineParser* cmdline = CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser);

  const char *val;
  if ((val = cmdline->GetOption ("winsize")))
  {
    int wres, hres;
    if (sscanf (val, "%d,%d", &wres, &hres) == 2)
    {
      WindowWidth = wres;
      WindowHeight = hres;
    }
    else
      printf ("Bad value `%s' for -winsize command-line parameter (W,H expected)\n", val);
  }

  if ((val = cmdline->GetOption ("winpos")))
  {
    int xpos, ypos;
    if (sscanf (val, "%d,%d", &xpos, &ypos) == 2)
    {
      WindowX = xpos;
      WindowY = ypos;
    }
    else
      printf ("Bad value `%s' for -winpos command-line parameter (X,Y expected)\n", val);
  }

  if (cmdline->GetOption ("sysmouse")) HardwareCursor = true;
  if (cmdline->GetOption ("nosysmouse")) HardwareCursor = false;

  iEventQueue* q = CS_QUERY_REGISTRY(object_reg, iEventQueue);
  if (q != 0)
    EventOutlet = q->CreateEventOutlet (this);

  KeyboardDriver = CS_QUERY_REGISTRY(object_reg, iKeyboardDriver);
  if (KeyboardDriver != 0)
    KeyboardDriver->IncRef ();

  return true;
}

bool csGraphics2DOS2DIVE::HandleEvent (iEvent &Event)
{
  if ((Event.Type == csevBroadcast)
   && (Event.Command.Code == cscmdCommandLineHelp)
   && object_reg)
  {
    printf ("Options for OS/2 DIVE driver:\n");
    printf ("  -winpos=<x>,<y>    set window position in percent of screen (default=center)\n");
    printf ("  -winsize=<w>,<h>   set window size (default=max fit mode multiple)\n");
    printf ("  -[no]sysmouse      use/don't use system mouse cursor (default=%s)\n",
      HardwareCursor ? "use" : "don't");
    return true;
  }
  return false;
}

bool csGraphics2DOS2DIVE::Open ()
{
  if (is_open) return true;
  if (!csGraphics2D::Open ())
    return false;

  int i;

  // Set up FGVideoMode
  // Check first all available 16-bit modes for 'native' flag
  // if so, use it. Otherwise choose R5G6B5 format.
  PixelFormat = FOURCC_LUT8;
  switch (Depth)
  {
    case 32:
      for (i = 0; i < (int)vmCount; i++)
        if (vmList[i].PixelFormat == FOURCC_RGB4
         || vmList[i].PixelFormat == FOURCC_BGR4)
        {
          if (vmList[i].Flags & vmfNative)
          {
            PixelFormat = vmList[i].PixelFormat;
            break;
          }

          if (PixelFormat == FOURCC_LUT8)
            PixelFormat = vmList[i].PixelFormat;
        } /* endif */
      if (PixelFormat != FOURCC_LUT8)
        break;
    case 16:
      for (i = 0; i < (int)vmCount; i++)
      {
        if (vmList[i].PixelFormat == FOURCC_R565
         || vmList[i].PixelFormat == FOURCC_R664
         || vmList[i].PixelFormat == FOURCC_R555)
        {
          if (vmList[i].Flags & vmfNative)
          {
            PixelFormat = vmList[i].PixelFormat;
            break;
          }

          if ((vmList[i].PixelFormat == FOURCC_R565)
           || ((vmList[i].PixelFormat == FOURCC_R664) && (PixelFormat != FOURCC_R565))
           || (PixelFormat == FOURCC_LUT8))
            PixelFormat = vmList[i].PixelFormat;
        } /* endif */
      } /* endfor */
      if (PixelFormat != FOURCC_LUT8)
        break;
    case 15:
      for (i = 0; i < (int)vmCount; i++)
        if (vmList[i].PixelFormat == FOURCC_R555)
        {
          PixelFormat = vmList[i].PixelFormat;
          break;
        }
      if (PixelFormat != FOURCC_LUT8)
        break;
    case 8:
      break;
    default:
      printf ("ERROR: %d bits per pixel modes not supported\n", Depth);
      break;
  } /* endswitch */

  switch (PixelFormat)
  {
    case FOURCC_LUT8:
      pfmt.PalEntries = 256;
      pfmt.PixelBytes = 1;
      pfmt.RedMask    = 0xff;
      pfmt.GreenMask  = 0xff;
      pfmt.BlueMask   = 0xff;
      break;
    case FOURCC_R565:
      pfmt.PalEntries = 0;
      pfmt.PixelBytes = 2;
      pfmt.RedMask    = 0xf800;
      pfmt.GreenMask  = 0x07e0;
      pfmt.BlueMask   = 0x001f;
      break;
    case FOURCC_R664:
      pfmt.PalEntries = 0;
      pfmt.PixelBytes = 2;
      pfmt.RedMask    = 0xfc00;
      pfmt.GreenMask  = 0x03f0;
      pfmt.BlueMask   = 0x000f;
      break;
    case FOURCC_R555:
      pfmt.PalEntries = 0;
      pfmt.PixelBytes = 2;
      pfmt.RedMask    = 0x7c00;
      pfmt.GreenMask  = 0x03e0;
      pfmt.BlueMask   = 0x001f;
      break;
    case FOURCC_RGB3:
      pfmt.PalEntries = 0;
      pfmt.PixelBytes = 3;
      pfmt.RedMask    = 0x000000ff;
      pfmt.GreenMask  = 0x0000ff00;
      pfmt.BlueMask   = 0x00ff0000;
      break;
    case FOURCC_BGR3:
      pfmt.PalEntries = 0;
      pfmt.PixelBytes = 3;
      pfmt.RedMask    = 0x00ff0000;
      pfmt.GreenMask  = 0x0000ff00;
      pfmt.BlueMask   = 0x000000ff;
      break;
    case FOURCC_RGB4:
      pfmt.PalEntries = 0;
      pfmt.PixelBytes = 4;
      pfmt.RedMask    = 0x000000ff;
      pfmt.GreenMask  = 0x0000ff00;
      pfmt.BlueMask   = 0x00ff0000;
      break;
    case FOURCC_BGR4:
      pfmt.PalEntries = 0;
      pfmt.PixelBytes = 4;
      pfmt.RedMask    = 0x00ff0000;
      pfmt.GreenMask  = 0x0000ff00;
      pfmt.BlueMask   = 0x000000ff;
      break;
  } /* endswitch */
  pfmt.complete ();

  if ((Depth >> 3) != pfmt.PixelBytes)
  {
    printf ("WARNING: %d bpp mode requested, but not available: using %d bpp mode\n",
      Depth, pfmt.PixelBytes << 3);
    Depth = pfmt.PixelBytes << 3;
  }

  PMrq rq;
  u_int rc;
  FGVideoMode Mode = // selected mode with double buffering
  { Width, Height, PixelFormat, 2, vmfWindowed };

  printf ("Using %c%c%c%c pixel format\n",
    PixelFormat, PixelFormat >> 8, PixelFormat >> 16, PixelFormat >> 24);

  // Create PM window
  rq.Parm.CreateWindow.Title = win_title;
  if ((rc = PMcall (pmcmdCreateWindow, &rq)) != pmrcOK)
  {
    printf ("Cannot create PM window: no resources bound to driver?\n");
    return false;
  }
  WinHandle = rq.Parm.CreateWindow.Handle;

  // Create DIVE contect
  rq.Parm.CreateCtx.Mode = &Mode;
  if ((rc = PMcall (pmcmdCreateDIVEctx, &rq)) != pmrcOK)
  {
    printf ("Cannot create DIVE context\n");
    return false;
  }

  dW = rq.Parm.CreateCtx.dW;

  // Setup event handlers
  dW->SetKeyboardHandler (KeyboardHandlerStub, this);
  dW->SetMouseHandler (MouseHandlerStub, this);
  dW->SetTerminateHandler (TerminateHandlerStub, this);
  dW->SetFocusHandler (FocusHandlerStub, this);

  // Bind DIVE context to window
  rq.Parm.BindCtx.dW = dW;
  rq.Parm.BindCtx.Handle = WinHandle;
  rq.Parm.BindCtx.DesktopW = DesktopW;
  rq.Parm.BindCtx.DesktopH = DesktopH;
  if ((rc = PMcall (pmcmdBindDIVEctx, &rq)) != pmrcOK)
  {
    printf ("Cannot bind DIVE context to window!\n");
    return false;
  }

  if ((WindowWidth != -1) && (WindowHeight != -1))
  {
    rq.Parm.Resize.dW = dW;
    rq.Parm.Resize.Width = WindowWidth;
    rq.Parm.Resize.Height = WindowHeight;
    rq.Parm.Resize.Center = true;
    PMcall (pmcmdResizeWindow, &rq);
  }
  if ((WindowX != INT_MIN) || (WindowY != INT_MIN))
  {
    rq.Parm.Locate.dW = dW;
    if (WindowX != INT_MIN)
      rq.Parm.Locate.x = ((DesktopW * WindowX) / 100) & ~1;
    else
      rq.Parm.Locate.x = INT_MIN;
    if (WindowY != INT_MIN)
      rq.Parm.Locate.y = (DesktopH * WindowY) / 100;
    else
      rq.Parm.Locate.y = INT_MIN;
    PMcall (pmcmdLocateWindow, &rq);
  }

  if (FullScreen)
    dW->Command (cmdFullScreen);

  // Update drawing routine addresses
  switch (pfmt.PixelBytes)
  {
    case 1:
      break;
    case 2:
      _DrawPixel = DrawPixel16;
      _WriteString = WriteString16;
      _GetPixelAt = GetPixelAt16;
      break;
    case 4:
      _DrawPixel = DrawPixel32;
      _WriteString = WriteString32;
      _GetPixelAt = GetPixelAt32;
      break;
    default:
      printf ("WARNING: No 2D routines for selected mode!\n");
      break;
  } /* endif */

  // Show window
  rq.Parm.ShowWin.dW = dW;
  rq.Parm.ShowWin.State = 1;
  if ((rc = PMcall (pmcmdShowWindow, &rq)) != pmrcOK)
    return false;

  UpdatePalette = FALSE;
  return true;
}

void csGraphics2DOS2DIVE::Close (void)
{
  if (!is_open) return;
  PMrq rq;

  if (!dW)
    return;

  // Destroy DIVE context
  rq.Parm.DestroyCtx.dW = dW;
  PMcall (pmcmdDestroyDIVEctx, &rq);

  // Destroy PM window
  rq.Parm.DestroyWindow.Handle = WinHandle;
  PMcall (pmcmdDestroyWindow, &rq);

  dW = NULL;
}

void csGraphics2DOS2DIVE::Print (csRect *area)
{
  // If we're in double-buffering mode, wait previous frame to complete
  if (dblbuff)
    dW->WaitSwitch ();

  // Now switch to next buffer
  if (area)
  {
    // EXPERIMENTAL FEATURE: although DIVE docs states that SetupBlitter
    // is a longplay operation, on some (all?) videocards it is still
    // faster to do a SetupBlitter() with a small area (that should be redrawn)
    // followed by a blit rather than blitting constantly entire image.

    RECTL rect;
    rect.xLeft = area->xmin;
    rect.yBottom = dW->BufferHeight () - 1 - area->ymax;
    rect.xRight = area->xmax + 1;
    rect.yTop = dW->BufferHeight () - 1 - area->ymin + 1;

    // re-scale
    float AspectX = (float)dW->WindowWidth () / (float)dW->BufferWidth ();
    float AspectY = (float)dW->WindowHeight () / (float)dW->BufferHeight ();
    rect.xLeft = (int) (rect.xLeft * AspectX);
    rect.xRight = (int) (rect.xRight * AspectX);
    rect.yBottom = (int) (rect.yBottom * AspectY);
    rect.yTop = (int) (rect.yTop * AspectY);
    // Increment top & right margin by one pixel to cover calculation errors
    rect.xRight++; rect.yTop++;
    dW->Switch (ActivePage, &rect);
  }
  else
    dW->Switch (ActivePage);

  // If we're in single-buffered mode, wait right now for buffer to be printed
  if (!dblbuff)
    dW->WaitSwitch ();
  else
    ActivePage ^= 1;
}

int csGraphics2DOS2DIVE::GetPage ()
{
  return ActivePage;
}

bool csGraphics2DOS2DIVE::DoubleBuffer (bool Enable)
{
  dblbuff = Enable;
  return true;
}

bool csGraphics2DOS2DIVE::GetDoubleBufferState ()
{
  return dblbuff;
}

void csGraphics2DOS2DIVE::Clear (int color)
{
  switch (pfmt.PixelBytes)
  {
    case 1:
      color &= 0xff;
      color |= (color << 8) | (color << 16) | (color << 24);
      break;
    case 2:
      color &= 0xffff;
      color |= (color << 16);
      break;
  }
  memsetd (Memory, color, (Width * Height * pfmt.PixelBytes) >> 2);
}

void csGraphics2DOS2DIVE::SetRGB (int i, int r, int g, int b)
{
  // set a rgb color in the palette of your graphic interface
  if (i < 0 && i > 255)
    return;
  DivePalette[i] = b | g << 8 | r << 16;
  UpdatePalette = TRUE;
  csGraphics2D::SetRGB (i, r, g, b);
}

bool csGraphics2DOS2DIVE::BeginDraw ()
{
  csGraphics2D::BeginDraw ();
  if (FrameBufferLocked != 1)
    return true;

  ULONG bpl;
  Memory = dW->BeginPaint (&bpl, ActivePage);
  // if paused, return false
  if (Memory == NULL)
  {
    csGraphics2D::FinishDraw ();
    return false;
  } /* endif */

  if (bpl != LineAddressFrameW)
  {
    int i,addr;
    for (i = 0, addr = 0; i < Height; i++, addr += bpl)
      LineAddress [i] = addr;
    LineAddressFrameW = bpl;
  } /* endif */

  if (UpdatePalette && pfmt.PalEntries)
  {
    dW->SetCLUT(DivePalette, pfmt.PalEntries);
    UpdatePalette = FALSE;
  }
  return true;
}

void csGraphics2DOS2DIVE::FinishDraw ()
{
  csGraphics2D::FinishDraw ();
  if (FrameBufferLocked)
    return;

  dW->EndPaint ();
  Memory = NULL;
}

bool csGraphics2DOS2DIVE::SetMousePosition (int x, int y)
{
  int ww = dW->WindowWidth (), wh = dW->WindowHeight ();
  if ((ww <= 0) || (wh <= 0))
    return false;

  POINTL pp;
  pp.x = (x * ww) / Width;
  pp.y = ((Height - 1 - y) * wh) / Height;
  WinMapWindowPoints (dW->diveCL, HWND_DESKTOP, &pp, 1);

  return WinSetPointerPos (HWND_DESKTOP, pp.x, pp.y);
}

bool csGraphics2DOS2DIVE::SetMouseCursor (csMouseCursorID iShape)
{
  if (!HardwareCursor)
  {
    dW->MouseVisible (FALSE);
    return false;
  } /* endif */

  switch (iShape)
  {
    case csmcNone:
      dW->MouseVisible (FALSE);
      return true;
    case csmcArrow:
      dW->MouseCursor (SPTR_ARROW);
      dW->MouseVisible (TRUE);
      return true;
    case csmcLens:
    case csmcCross:
    case csmcPen:
      dW->MouseVisible (FALSE);
      return false;
    case csmcMove:
      dW->MouseCursor (SPTR_MOVE);
      dW->MouseVisible (TRUE);
      return true;
    case csmcSizeNWSE:
      dW->MouseCursor (SPTR_SIZENWSE);
      dW->MouseVisible (TRUE);
      return true;
    case csmcSizeNESW:
      dW->MouseCursor (SPTR_SIZENESW);
      dW->MouseVisible (TRUE);
      return true;
    case csmcSizeNS:
      dW->MouseCursor (SPTR_SIZENS);
      dW->MouseVisible (TRUE);
      return true;
    case csmcSizeEW:
      dW->MouseCursor (SPTR_SIZEWE);
      dW->MouseVisible (TRUE);
      return true;
    case csmcStop:
      dW->MouseCursor (SPTR_ILLEGAL);
      dW->MouseVisible (TRUE);
      return true;
    case csmcWait:
      dW->MouseCursor (SPTR_WAIT);
      dW->MouseVisible (TRUE);
      return true;
    default:
      dW->MouseVisible (FALSE);
      return false;
  } /* endswitch */
}

void csGraphics2DOS2DIVE::MouseHandlerStub (void *Self, int Button, bool Down,
  int x, int y, int /*ShiftFlags*/)
{
  csGraphics2DOS2DIVE *This = (csGraphics2DOS2DIVE *)Self;
  if (!This)
    return;
  int ww = This->dW->WindowWidth (), wh = This->dW->WindowHeight ();
  if ((ww <= 0) || (wh <= 0))
    return;

  x = (x * This->Width) / ww;
  y = ((wh - 1 - y) * This->Height) / wh;

  This->EventOutlet->Mouse (Button, Down, x, y);
}

void csGraphics2DOS2DIVE::KeyboardHandlerStub (void *Self, unsigned char ScanCode,
  unsigned char CharCode, bool Down, unsigned char RepeatCount, int ShiftFlags)
{
  csGraphics2DOS2DIVE *This = (csGraphics2DOS2DIVE *)Self;
  int KeyCode = ScanCode < 128 ? ScanCodeToChar [ScanCode] : 0;
  // OS/2 ENTER has char code 13 while Crystal Space uses '\n' ...
  if (KeyCode == CSKEY_ENTER) CharCode = CSKEY_ENTER;

  // WM_CHAR does not support Ctrl+# ...
  if (KeyboardDriver->GetKeyState (CSKEY_CTRL) && !CharCode
   && (KeyCode > 96) && (KeyCode < 127))
    CharCode = KeyCode - 96;

  This->EventOutlet->Key (KeyCode, CharCode, Down);
}

void csGraphics2DOS2DIVE::FocusHandlerStub (void *Self, bool Enable)
{
  csGraphics2DOS2DIVE *This = (csGraphics2DOS2DIVE *)Self;
  This->EventOutlet->Broadcast (cscmdFocusChanged, (void *)Enable);
}

void csGraphics2DOS2DIVE::TerminateHandlerStub (void *Self)
{
  csGraphics2DOS2DIVE *This = (csGraphics2DOS2DIVE *)Self;
  This->EventOutlet->Broadcast (cscmdContextClose, (iGraphics2D *)This);
  This->EventOutlet->Broadcast (cscmdQuit);
}
