/*
    OS/2 support for Crystal Space 3D library
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Andrew Zabolotny <bit@eltech.ru>

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
#include "isys/system.h"
#include "iutil/cfgfile.h"
#include "iutil/cmdline.h"
#include "iutil/objreg.h"

// shit ...
#undef SEVERITY_ERROR

#include "glos2.h"
#include "libGL.h"
#include "libGLprv.h"

//-------------------------------------------------------- csGraphics2DOS2GL ---

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csGraphics2DOS2GL)

SCF_EXPORT_CLASS_TABLE (glos2)
  SCF_EXPORT_CLASS_DEP (csGraphics2DOS2GL, "crystalspace.graphics2d.glos2",
    "OS/2 OpenGL 2D graphics driver for Crystal Space", "crystalspace.font.server.")
SCF_EXPORT_CLASS_TABLE_END

csGraphics2DOS2GL::csGraphics2DOS2GL (iBase *iParent) :
  csGraphics2DGLCommon (iParent),
  HardwareCursor (true), WindowX (INT_MIN), WindowY (INT_MIN)
{
  // Initialize module handle
#ifdef CS_STATIC_LINKED
  gdMH = NULLHANDLE;
#else
  extern unsigned long dll_handle;
  gdMH = dll_handle;
#endif

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

csGraphics2DOS2GL::~csGraphics2DOS2GL (void)
{
  Close ();
  // Deallocate OpenGL resources
  gdGLDeinitialize ();
}

bool csGraphics2DOS2GL::Initialize (iObjectRegistry *object_reg)
{
  if (!csGraphics2DGLCommon::Initialize (object_reg))
    return false;

  iSystem* sys = CS_GET_SYSTEM (object_reg);	//@@@
  sys->PerformExtension ("StartGUI");

  // Initialize OpenGL
  if (!gdGLInitialize ())
  {
    printf ("Unable to initialize OpenGL library\n");
    return false;
  }

  PixelFormat = GLCF_DBLBUFF | (1 << GLCF_DEPTH_SHFT);
  if (Depth > 8)
    PixelFormat |= GLCF_RGBA;
  else
  {
    pfmt.PixelBytes = 1;
    pfmt.PalEntries = 256;
  }

  csConfigAccess Config(object_reg, "/config/video.cfg");
  WindowX = Config->GetInt ("Video.WindowX", INT_MIN);
  WindowY = Config->GetInt ("Video.WindowY", INT_MIN);
  HardwareCursor = Config->GetBool ("Video.SystemMouseCursor", true);
  iCommandLineParser* cmdline = CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser);

  const char *val;
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

  if (cmdline->GetOption ("sysmouse"))
    HardwareCursor = true;
  if (cmdline->GetOption ("nosysmouse"))
    HardwareCursor = false;

  EventOutlet = sys->CreateEventOutlet (this);

  return true;
}

bool csGraphics2DOS2GL::HandleEvent (iEvent &Event)
{
  if ((Event.Type == csevBroadcast)
   && (Event.Command.Code == cscmdCommandLineHelp)
   && object_reg)
  {
    printf ("Options for OS/2 OpenGL canvas driver:\n");
    printf ("  -winpos=<x>,<y>    set window position in percent of screen (default=center)\n");
    printf ("  -[no]sysmouse      use/don't use system mouse cursor (default=%s)\n",
      HardwareCursor ? "use" : "don't");
    return true;
  }
  return false;
}

bool csGraphics2DOS2GL::Open ()
{
  if (is_open) return true;
  PMrq rq;
  u_int rc;

  // Create PM window
  rq.Parm.CreateWindow.Title = win_title;
  if ((rc = PMcall (pmcmdCreateWindow, &rq)) != pmrcOK)
  {
    printf ("Cannot create PM window: no resources bound to executable?\n");
    return false;
  }
  WinHandle = rq.Parm.CreateWindow.Handle;

  // Create OpenGL contect
  rq.Parm.CreateCtx.Width = Width;
  rq.Parm.CreateCtx.Height = Height;
  rq.Parm.CreateCtx.ContextFlags = PixelFormat;
  if ((rc = PMcall (pmcmdCreateGLctx, &rq)) != pmrcOK)
  {
    printf ("Cannot create OpenGL context\n");
    return false;
  }

  glW = rq.Parm.CreateCtx.glW;

  // Setup event handlers
  glW->SetKeyboardHandler (KeyboardHandlerStub, this);
  glW->SetMouseHandler (MouseHandlerStub, this);
  glW->SetTerminateHandler (TerminateHandlerStub, this);
  glW->SetFocusHandler (FocusHandlerStub, this);
  glW->SetResizeHandler (ResizeHandlerStub, this);

  // Bind OpenGL context to window
  rq.Parm.BindCtx.glW = glW;
  rq.Parm.BindCtx.Handle = WinHandle;
  rq.Parm.BindCtx.DesktopW = DesktopW;
  rq.Parm.BindCtx.DesktopH = DesktopH;
  if ((rc = PMcall (pmcmdBindGLctx, &rq)) != pmrcOK)
  {
    printf ("Cannot bind OpenGL context to window!\n");
    return false;
  }

  if ((WindowX != INT_MIN) || (WindowY != INT_MIN))
  {
    rq.Parm.Locate.glW = glW;
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

  // Show window
  rq.Parm.ShowWin.glW = glW;
  rq.Parm.ShowWin.State = true;
  if ((rc = PMcall (pmcmdShowWindow, &rq)) != pmrcOK)
    return false;

  // Select window for drawing
  rq.Parm.ShowWin.glW = glW;
  if ((rc = PMcall (pmcmdSelectWindow, &rq)) != pmrcOK)
    return false;

  // Black magic: without this we'll get weird results, at least with
  // OpenGL 1.1 "gold" beta release.
  rq.Parm.ResetWin.glW = glW;
  PMcall (pmcmdResetWindow, &rq);

  if (FullScreen)
    glW->Command (cmdFullScreen);

  UpdatePalette = false;
  AllowCanvasResize (false);

  if (!csGraphics2DGLCommon::Open ())
    return false;

  return true;
}

void csGraphics2DOS2GL::Close ()
{
  if (!is_open) return;
  PMrq rq;

  if (!glW)
    return;

  // Destroy OpenGL context
  rq.Parm.DestroyCtx.glW = glW;
  PMcall (pmcmdDestroyGLctx, &rq);

  // Destroy PM window
  rq.Parm.DestroyWindow.Handle = WinHandle;
  PMcall (pmcmdDestroyWindow, &rq);

  glW = NULL;
  csGraphics2DGLCommon::Close ();
}

void csGraphics2DOS2GL::Print (csRect *area)
{
  glW->Flush (NULL);
  glW->WaitFlush ();
}

int csGraphics2DOS2GL::GetPage ()
{
  return glW->ActiveBuff ();
}

bool csGraphics2DOS2GL::DoubleBuffer (bool Enable)
{
  return Enable;
}

bool csGraphics2DOS2GL::GetDoubleBufferState ()
{
  return true;
}

void csGraphics2DOS2GL::SetRGB (int i, int r, int g, int b)
{
  // set a rgb color in the palette of your graphic interface
  if (i < 0 && i > 255)
    return;
  GLPalette[i] = b | g << 8 | r << 16;
  UpdatePalette = TRUE;
  csGraphics2DGLCommon::SetRGB (i, r, g, b);
}

bool csGraphics2DOS2GL::BeginDraw ()
{
  if (UpdatePalette && pfmt.PalEntries)
  {
    glW->SetPalette (GLPalette, 256);
    UpdatePalette = FALSE;
  }

  return csGraphics2DGLCommon::BeginDraw ();
}

bool csGraphics2DOS2GL::SetMousePosition (int x, int y)
{
  POINTL pp;
  pp.x = x;
  pp.y = Height - 1 - y;
  WinMapWindowPoints (glW->hwndCL, HWND_DESKTOP, &pp, 1);

  return WinSetPointerPos (HWND_DESKTOP, pp.x, pp.y);
}

bool csGraphics2DOS2GL::SetMouseCursor (csMouseCursorID iShape)
{
  if (!HardwareCursor)
  {
    glW->MouseVisible (FALSE);
    return false;
  } /* endif */

  switch (iShape)
  {
    case csmcNone:
      glW->MouseVisible (FALSE);
      return true;
    case csmcArrow:
      glW->MouseCursor (SPTR_ARROW);
      glW->MouseVisible (TRUE);
      return true;
    case csmcLens:
    case csmcCross:
    case csmcPen:
      glW->MouseVisible (FALSE);
      return false;
    case csmcMove:
      glW->MouseCursor (SPTR_MOVE);
      glW->MouseVisible (TRUE);
      return true;
    case csmcSizeNWSE:
      glW->MouseCursor (SPTR_SIZENWSE);
      glW->MouseVisible (TRUE);
      return true;
    case csmcSizeNESW:
      glW->MouseCursor (SPTR_SIZENESW);
      glW->MouseVisible (TRUE);
      return true;
    case csmcSizeNS:
      glW->MouseCursor (SPTR_SIZENS);
      glW->MouseVisible (TRUE);
      return true;
    case csmcSizeEW:
      glW->MouseCursor (SPTR_SIZEWE);
      glW->MouseVisible (TRUE);
      return true;
    case csmcStop:
      glW->MouseCursor (SPTR_ILLEGAL);
      glW->MouseVisible (TRUE);
      return true;
    case csmcWait:
      glW->MouseCursor (SPTR_WAIT);
      glW->MouseVisible (TRUE);
      return true;
    default:
      glW->MouseVisible (FALSE);
      return false;
  } /* endswitch */
}

void csGraphics2DOS2GL::AllowCanvasResize (bool iAllow)
{
  glW->AllowWindowResize (AllowResize = iAllow);
}

void csGraphics2DOS2GL::MouseHandlerStub (void *Self, int Button, bool Down,
  int x, int y, int ShiftFlags)
{
  csGraphics2DOS2GL *This = (csGraphics2DOS2GL *)Self;
  if (!This)
    return;

  This->EventOutlet->Mouse (Button, Down, x, This->Height - 1 - y);
}

void csGraphics2DOS2GL::KeyboardHandlerStub (void *Self, unsigned char ScanCode,
  unsigned char CharCode, int Down, unsigned char RepeatCount, int ShiftFlags)
{
  csGraphics2DOS2GL *This = (csGraphics2DOS2GL *)Self;
  int KeyCode = ScanCode < 128 ? ScanCodeToChar [ScanCode] : 0;
  // OS/2 ENTER has char code 13 while Crystal Space uses '\n' ...
  if (KeyCode == CSKEY_ENTER) CharCode = CSKEY_ENTER;

  // WM_CHAR does not support Ctrl+# ...
  iSystem* sys = CS_GET_SYSTEM (This->object_reg);	//@@@
  if (sys->GetKeyState (CSKEY_CTRL) && !CharCode
   && (KeyCode > 96) && (KeyCode < 127))
    CharCode = KeyCode - 96;

  This->EventOutlet->Key (KeyCode, CharCode, Down);
}

void csGraphics2DOS2GL::FocusHandlerStub (void *Self, bool Enable)
{
  csGraphics2DOS2GL *This = (csGraphics2DOS2GL *)Self;
  This->EventOutlet->Broadcast (cscmdFocusChanged, (void *)Enable);
}

void csGraphics2DOS2GL::TerminateHandlerStub (void *Self)
{
  csGraphics2DOS2GL *This = (csGraphics2DOS2GL *)Self;
  This->EventOutlet->Broadcast (cscmdContextClose, (iGraphics2D *)This);
  This->EventOutlet->Broadcast (cscmdQuit);
}

void csGraphics2DOS2GL::ResizeHandlerStub (void *Self)
{
  csGraphics2DOS2GL *This = (csGraphics2DOS2GL *)Self;
  if (This->Width != This->glW->BufferWidth ()
   || This->Height != This->glW->BufferHeight ())
  {
    This->Width = This->glW->BufferWidth ();
    This->Height = This->glW->BufferHeight ();
    This->EventOutlet->Broadcast (cscmdContextResize, (iGraphics2D *)This);
    This->SetClipRect (0, 0, This->Width - 1, This->Height - 1);
  }
}
