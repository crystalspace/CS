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
#include "sysdef.h"
#include "cscom/com.h"
#include "csgeom/csrect.h"
#include "csinput/csevent.h"
#include "isystem.h"

// shit ...
#undef SEVERITY_ERROR

#include "glos2.h"
#include "libGL.h"
#include "libGLprv.h"

//------------------------------------------------------ Loading/Maintenance ---

static unsigned int gRefCount = 0;
static DllRegisterData gRegData =
{
  &CLSID_OpenGLGraphics2D,
  "crystalspace.graphics2d.glos2",
  "Crystal Space 2D OpenGL graphics driver for OS/2"
};

#ifdef CS_STATIC_LINKED

void OS2GL2DRegister ()
{
  static csGraphics2DFactoryOS2GL gG2DGLFactory;
  gRegData.pClass = &gG2DGLFactory;
  csRegisterServer (&gRegData);
}

void OS2GL2DUnregister ()
{
  csUnregisterServer (&gRegData);
}

#else

// Initialization entry point
STDAPI DllInitialize ()
{
  csCoInitialize (0);
  gRegData.szInProcServer = "glos2.dll";
  return TRUE;
}

void STDAPICALLTYPE ModuleRelease ()
{
  gRefCount--;
}

void STDAPICALLTYPE ModuleAddRef ()
{
  gRefCount++;
}

// return S_OK if it's ok to unload us now.
STDAPI DllCanUnloadNow ()
{
  return gRefCount ? S_FALSE : S_OK;
}

// used to get a COM class object from us.
STDAPI DllGetClassObject (REFCLSID rclsid, REFIID riid, void** ppv)
{
  static csGraphics2DFactoryOS2GL gG2DGLFactory;
  if (rclsid == CLSID_OpenGLGraphics2D)
    return gG2DGLFactory.QueryInterface (riid, ppv);

  // if we get here, rclsid is a class we don't implement
  *ppv = NULL;
  return CLASS_E_CLASSNOTAVAILABLE;
}

// Called by regsvr
STDAPI DllRegisterServer ()
{
  return csRegisterServer (&gRegData);
}

// Called by regsvr
STDAPI DllUnregisterServer ()
{
  return csUnregisterServer (&gRegData);
}

#endif // CS_STATIC_LINKED

//------------------------------------------------- csGraphics2DOS2GLFactory ---

IMPLEMENT_UNKNOWN_NODELETE (csGraphics2DFactoryOS2GL)

BEGIN_INTERFACE_TABLE (csGraphics2DFactoryOS2GL)
  IMPLEMENTS_INTERFACE (IGraphics2DFactory)
END_INTERFACE_TABLE ()

STDMETHODIMP csGraphics2DFactoryOS2GL::CreateInstance (REFIID riid,
  ISystem* piSystem, void**ppv)
{
  if (!piSystem)
  {
    *ppv = 0;
    return E_INVALIDARG;
  }

  CHK (csGraphics2DOS2GL *pNew = new csGraphics2DOS2GL (piSystem));
  if (!pNew)
  {
    *ppv = 0;
    return E_OUTOFMEMORY;
  }
		
  return pNew->QueryInterface (riid, ppv);
}

STDMETHODIMP csGraphics2DFactoryOS2GL::LockServer (BOOL bLock)
{
  if (bLock)
    gRefCount++;
  else
    gRefCount--;

  return S_OK;
}

//-------------------------------------------------------- csGraphics2DOS2GL ---

BEGIN_INTERFACE_TABLE (csGraphics2DOS2GL)
  IMPLEMENTS_COMPOSITE_INTERFACE_EX (IGraphics2D, XGraphics2D)
  IMPLEMENTS_COMPOSITE_INTERFACE_EX (IGraphicsInfo, XGraphicsInfo)
END_INTERFACE_TABLE ()

IMPLEMENT_UNKNOWN_NODELETE (csGraphics2DOS2GL)

csGraphics2DOS2GL::csGraphics2DOS2GL (ISystem* piSystem) :
  csGraphics2DGLCommon (piSystem),
  HardwareCursor (true), WindowX (INT_MIN), WindowY (INT_MIN)
{
  // Initialize module handle
#ifdef CS_STATIC_LINKED
  gdMH = NULLHANDLE;
#else
  extern unsigned long dll_handle;
  gdMH = dll_handle;
#endif

  System = piSystem;

  if (!SUCCEEDED (piSystem->QueryInterface (IID_IOS2SystemDriver, (void**)&OS2System)))
  {
    CsPrintf (MSG_FATAL_ERROR, "The system driver does not support the IOS2SystemDriver interface\n");
    exit (-1);
  }

  // Initialize OpenGL
  if (!gdGLInitialize ())
  {
    CsPrintf (MSG_FATAL_ERROR, "Unable to initialize OpenGL library\n");
    exit (-1);
  }
}

csGraphics2DOS2GL::~csGraphics2DOS2GL (void)
{
  Close ();
  // Deallocate OpenGL resources
  gdGLDeinitialize ();
  FINAL_RELEASE (OS2System);
}

void csGraphics2DOS2GL::CsPrintf (int msgtype, char *format, ...)
{
  va_list arg;
  char buf[256];

  va_start (arg, format);
  vsprintf (buf, format, arg);
  va_end (arg);

  System->Print (msgtype, buf);
}

void csGraphics2DOS2GL::Initialize ()
{
  csGraphics2DGLCommon::Initialize ();

  // Query settings from system driver
  int WindowWidth, WindowHeight;
  OS2System->GetSettings (WindowX, WindowY, WindowWidth, WindowHeight, HardwareCursor);

  PixelFormat = GLCF_DBLBUFF | (1 << GLCF_DEPTH_SHFT);
  switch (Depth)
  {
    case 8:
      pfmt.PalEntries = 256;
      pfmt.PixelBytes = 1;
      pfmt.RedMask    = 0xff;
      pfmt.GreenMask  = 0xff;
      pfmt.BlueMask   = 0xff;
      break;
    case 15:
    case 16:
    case 32:
      PixelFormat |= GLCF_RGBA;
      pfmt.PalEntries = 0;
      pfmt.PixelBytes = 2;
      pfmt.RedMask    = 0x0000000f;
      pfmt.GreenMask  = 0x000000f0;
      pfmt.BlueMask   = 0x00000f00;
      break;
    default:
      CsPrintf (MSG_FATAL_ERROR, "ERROR: %d bits per pixel modes not supported\n", Depth);
      break;
  } /* endswitch */
  complete_pixel_format ();

  DrawPixel = DrawPixelGL;
  WriteChar = WriteCharGL;
  GetPixelAt = GetPixelAtGL;
  DrawSprite = DrawSpriteGL;
}

bool csGraphics2DOS2GL::Open (char *Title)
{
  // Create backing store
  CHK (Memory = new unsigned char [Width * Height * pfmt.PixelBytes]);

  PMrq rq;
  u_int rc;

  // Create PM window
  rq.Parm.CreateWindow.Title = Title;
  if ((rc = PMcall (pmcmdCreateWindow, &rq)) != pmrcOK)
  {
    CsPrintf (MSG_FATAL_ERROR, "Cannot create PM window: no resources bound to executable?\n");
    return false;
  }
  WinHandle = rq.Parm.CreateWindow.Handle;

  // Create OpenGL contect
  rq.Parm.CreateCtx.Width = Width;
  rq.Parm.CreateCtx.Height = Height;
  rq.Parm.CreateCtx.ContextFlags = PixelFormat;
  if ((rc = PMcall (pmcmdCreateGLctx, &rq)) != pmrcOK)
  {
    CsPrintf (MSG_FATAL_ERROR, "Cannot create OpenGL context\n");
    return false;
  }

  glW = rq.Parm.CreateCtx.glW;

  // Setup event handlers
  glW->SetKeyboardHandler (KeyboardHandlerStub, this);
  glW->SetMouseHandler (MouseHandlerStub, this);
  glW->SetTerminateHandler (TerminateHandlerStub, this);
  glW->SetFocusHandler (FocusHandlerStub, this);

  // Bind OpenGL context to window
  rq.Parm.BindCtx.glW = glW;
  rq.Parm.BindCtx.Handle = WinHandle;
  if ((rc = PMcall (pmcmdBindGLctx, &rq)) != pmrcOK)
  {
    CsPrintf (MSG_FATAL_ERROR, "Cannot bind OpenGL context to window!\n");
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

  glW->Select ();

  UpdatePalette = FALSE;

  if (!csGraphics2DGLCommon::Open (Title))
    return false;

  return true;
}

void csGraphics2DOS2GL::Close ()
{
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
  (void)Enable;
  return false;
}

bool csGraphics2DOS2GL::DoubleBuffer ()
{
  return true;
}

void csGraphics2DOS2GL::Clear (int color)
{
  glClear (GL_COLOR_BUFFER_BIT);
}

void csGraphics2DOS2GL::SetRGB (int i, int r, int g, int b)
{
  // set a rgb color in the palette of your graphic interface
  if (i < 0 && i > 255)
    return;
  GLPalette[i] = b | g << 8 | r << 16;
  UpdatePalette = TRUE;
  csGraphics2D::SetRGB (i, r, g, b);
}

bool csGraphics2DOS2GL::BeginDraw ()
{
  if (UpdatePalette && pfmt.PalEntries)
  {
    glW->SetPalette (GLPalette, 256);
    UpdatePalette = FALSE;
  }
  return true;
}

void csGraphics2DOS2GL::FinishDraw ()
{
}

bool csGraphics2DOS2GL::SetMousePosition (int x, int y)
{
  int fh;
  System->GetHeightSetting (fh);

  POINTL pp;
  pp.x = x;
  pp.y = fh - 1 - y;
  WinMapWindowPoints (glW->hwndCL, HWND_DESKTOP, &pp, 1);

  return WinSetPointerPos (HWND_DESKTOP, pp.x, pp.y);
}

bool csGraphics2DOS2GL::SetMouseCursor (int iShape, ITextureHandle *hBitmap)
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

void csGraphics2DOS2GL::MouseHandlerStub (void *Self, int Button, int Down,
  int x, int y, int ShiftFlags)
{
  csGraphics2DOS2GL *This = (csGraphics2DOS2GL *)Self;
  if (!This)
    return;

  int fh;
  This->System->GetHeightSetting (fh);

  This->OS2System->MouseEvent (Button, Down, x, fh - 1 - y,
    (ShiftFlags & GLKF_SHIFT ? CSMASK_SHIFT : 0) |
    (ShiftFlags & GLKF_CTRL ? CSMASK_CTRL : 0) |
    (ShiftFlags & GLKF_ALT ? CSMASK_ALT : 0));
}

void csGraphics2DOS2GL::KeyboardHandlerStub (void *Self, unsigned char ScanCode,
  int Down, unsigned char RepeatCount, int ShiftFlags)
{
  csGraphics2DOS2GL *This = (csGraphics2DOS2GL *)Self;
  if (!This)
    return;
  This->OS2System->KeyboardEvent (ScanCode, Down);
}

void csGraphics2DOS2GL::FocusHandlerStub (void *Self, bool Enable)
{
  csGraphics2DOS2GL *This = (csGraphics2DOS2GL *)Self;
  This->OS2System->FocusEvent (Enable);
}

void csGraphics2DOS2GL::TerminateHandlerStub (void *Self)
{
  csGraphics2DOS2GL *This = (csGraphics2DOS2GL *)Self;
  This->OS2System->TerminateEvent ();
}
