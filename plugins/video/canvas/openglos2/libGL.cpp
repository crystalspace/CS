/*
    OS/2 OpenGL wrapper class library
    Copyright (C) 1999 by FRIENDS software
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

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <malloc.h>
#include "cssysdef.h"
#include "GL/gl.h"
#include "GL/pgl.h"
#include "libGL.h"

long DesktopW, DesktopH;                // Desktop width x height

glApp::glApp ()
{
  appWNlength = 0;
  AB = WinInitialize (0);
  MQ = WinCreateMsgQueue (AB, 0);
  DesktopW = WinQuerySysValue (HWND_DESKTOP, SV_CXSCREEN);
  DesktopH = WinQuerySysValue (HWND_DESKTOP, SV_CYSCREEN);
}

glApp::~glApp ()
{
  while (appWNlength--)
    WinDestroyWindow (appWN[appWNlength]);
  WinDestroyMsgQueue (MQ);
  WinTerminate (AB);
}

void glApp::Run ()
{
  QMSG qmsg;

  while (WinGetMsg (AB, &qmsg, 0, 0, 0))
    WinDispatchMsg (AB, &qmsg);
}

bool glApp::ProcessQueuedMessages ()
{
#define MSG_MASK (QS_KEY | QS_MOUSE | QS_TIMER | QS_PAINT | QS_POSTMSG)
  QMSG qmsg;

  while ((WinQueryQueueStatus (MQ) & MSG_MASK) != 0)
  {
    if (WinGetMsg (AB, &qmsg, 0, 0, 0))
      WinDispatchMsg (AB, &qmsg);
    else
      return false;
  }
  return true;
}

HWND glApp::CreateWindow (const char *Title, HMODULE ModID, ULONG ResID, ULONG Flags)
{
  const PSZ gl_window_class = (PSZ)"OpenGLview";
  ULONG flStyle;
  HWND hwndFR, hwndCL;

  if (!WinRegisterClass (AB, gl_window_class, NULL,
         CS_SIZEREDRAW | CS_MOVENOTIFY, sizeof (void *)))
    return 0;

  flStyle = Flags | FCF_SIZEBORDER | FCF_AUTOICON;
  if (flStyle & FCF_SYSMENU)
    flStyle = flStyle | FCF_MENU;
  if (flStyle & FCF_TITLEBAR)
    flStyle = flStyle | FCF_SYSMENU | FCF_MINBUTTON | FCF_MAXBUTTON;
  if (Title)
    flStyle = flStyle | FCF_TASKLIST;

  hwndFR = WinCreateStdWindow (HWND_DESKTOP, FS_NOBYTEALIGN, &flStyle,
    gl_window_class, (PSZ)Title, 0, ModID, ResID, &hwndCL);
  if (hwndFR)
  {
    // Keep track of all windows we created
    appWN[appWNlength++] = hwndFR;

    if (Flags & FCF_SYSMENU)            // Replace system menu by 1st submenu under menu bar
    {
      MENUITEM mi, smi;
      HWND hMenu = WinWindowFromID (hwndFR, FID_MENU);

      // Find handle of first submenu in menu bar
      WinSendMsg (hMenu, MM_QUERYITEM, MPFROM2SHORT (
          SHORT1FROMMR (WinSendMsg (hMenu, MM_ITEMIDFROMPOSITION, NULL, NULL)),
          FALSE), MPFROMP (&mi));
      // Change menu ID so it will not appear below title bar
      WinSetWindowUShort (hMenu, QWS_ID, 0);

      // Find System Menu and pull-down menu handles
      HWND hSysMenu = WinWindowFromID (hwndFR, FID_SYSMENU);
      if (hMenu && hSysMenu)
      {
        // Find the handle of first submenu in system menu
        WinSendMsg (hSysMenu, MM_QUERYITEM, MPFROM2SHORT (
            SHORT1FROMMR (WinSendMsg (hSysMenu, MM_ITEMIDFROMPOSITION, NULL, NULL)),
            FALSE), MPFROMP (&smi));
        HWND hSysSubMenu = smi.hwndSubMenu;

        smi.hwndSubMenu = mi.hwndSubMenu;

        // Add separator line to that submenu
        memset (&mi, 0, sizeof (mi));
        mi.iPosition = MIT_END;
        mi.afStyle = MIS_SEPARATOR;
        WinSendMsg (smi.hwndSubMenu, MM_INSERTITEM, (MPARAM) & mi, NULL);
        // Add a submenu link to previous window submenu
        mi.afStyle = MIS_SUBMENU;
        mi.hwndSubMenu = hSysSubMenu;
        WinSendMsg (smi.hwndSubMenu, MM_INSERTITEM, (MPARAM) & mi, (MPARAM) "~Window");

        // Replace system menu by this submenu
        WinSendMsg (hSysMenu, MM_SETITEM, MPFROM2SHORT (0, FALSE), MPFROMP (&smi));
      }
    }
  }
  return hwndFR;
}

glWindow::glWindow (long Width, long Height, long ContextFlags)
{
  lastError = glerOK;
  fMinimized = false;
  fAspect = false;
  fPause = true;
  fMouseVisible = true;
  fFullScreen = false;
  MouseCursorID = SPTR_ARROW;
  ActiveBuffer = 0;
  MouseButtonMask = 0;
  MouseCaptured = false;
  Pause (false);
  OldClientWindowProc = NULL;
  OldFrameWindowProc = NULL;
  BufferW = Width;
  BufferH = Height;
  CtxFlags = ContextFlags;
  hwndCL = NULLHANDLE;
  hwndFR = NULLHANDLE;
  glMN = NULLHANDLE;
  hKeyboard = NULL;
  hTerminate = NULL;
  hMouse = NULL;
  hFocus = NULL;
  hResize = NULL;
  csSwapFloatBuffers = false;
  fRedrawDisabled = false;
  AllowResize = true;
  hpal = NULLHANDLE;
  ScreenW = WinQuerySysValue (HWND_DESKTOP, SV_CXSCREEN);
  ScreenH = WinQuerySysValue (HWND_DESKTOP, SV_CYSCREEN);

  if (DosCreateEventSem (NULLHANDLE, &sRedrawComplete, 0, FALSE))
  {
    lastError = glerCreateSem;
    return;
  }
}

glWindow::~glWindow ()
{
  Unbind (false);
  DosCloseEventSem (sRedrawComplete);	// Destroy redraw semaphore
}

bool glWindow::Bind (HWND winHandle)
{
  hwndFR = winHandle;
  hwndCL = WinWindowFromID (hwndFR, FID_CLIENT);
  glAB = WinQueryAnchorBlock (winHandle);
  if (!glAB)
    return false;
  if (!hwndCL)
  {
    hwndCL = hwndFR;
    hwndFR = NULLHANDLE;
    glMN = NULLHANDLE;
  }
  else
  {
    glMN = WinWindowFromID (hwndFR, FID_MENU);
    if (!glMN)
      glMN = WinWindowFromID (hwndFR, FID_SYSMENU);
    MouseVisible (fMouseVisible);
  }

  if (!WinSetWindowPtr (hwndCL, QWL_USER, this))
  {
    lastError = glerBadWindow;
    return false;
  }

  if (hwndFR && !WinSetWindowPtr (hwndFR, QWL_USER, this))
  {
    lastError = glerBadWindow;
    return false;
  }

  OldClientWindowProc = WinSubclassWindow (hwndCL, &ClientHandler);
  OldFrameWindowProc = WinSubclassWindow (hwndFR, &FrameHandler);

  // Find a suitable visual
  int attributes [20];
  int count = 0;
  if (CtxFlags & GLCF_RGBA)
    attributes [count++] = PGL_RGBA;
  if (CtxFlags & GLCF_DBLBUFF)
    attributes [count++] = PGL_DOUBLEBUFFER;
  if (CtxFlags & GLCF_SNGBUFF)
    attributes [count++] = PGL_SINGLEBUFFER;
  if (CtxFlags & GLCF_STEREO)
    attributes [count++] = PGL_STEREO;
  if (CtxFlags & GLCF_STENCIL)
  {
    attributes [count++] = PGL_STENCIL_SIZE;
    attributes [count++] = (CtxFlags & GLCF_STENC_MASK) >> GLCF_STENC_SHFT;
  }
  if (CtxFlags & GLCF_ALPHA_MASK)
  {
    attributes [count++] = PGL_ALPHA_SIZE;
    attributes [count++] = (CtxFlags & GLCF_ALPHA_MASK) >> GLCF_ALPHA_SHFT;
  }
  if (CtxFlags & GLCF_RED_MASK)
  {
    attributes [count++] = PGL_RED_SIZE;
    attributes [count++] = (CtxFlags & GLCF_RED_MASK) >> GLCF_RED_SHFT;
  }
  if (CtxFlags & GLCF_GREEN_MASK)
  {
    attributes [count++] = PGL_GREEN_SIZE;
    attributes [count++] = (CtxFlags & GLCF_GREEN_MASK) >> GLCF_GREEN_SHFT;
  }
  if (CtxFlags & GLCF_BLUE_MASK)
  {
    attributes [count++] = PGL_BLUE_SIZE;
    attributes [count++] = (CtxFlags & GLCF_BLUE_MASK) >> GLCF_BLUE_SHFT;
  }
  if (CtxFlags & GLCF_DEPTH_MASK)
  {
    attributes [count++] = PGL_DEPTH_SIZE;
    attributes [count++] = (CtxFlags & GLCF_DEPTH_MASK) >> GLCF_DEPTH_SHFT;
  }
  if (CtxFlags & GLCF_STENC_MASK)
  {
    attributes [count++] = PGL_STENCIL_SIZE;
    attributes [count++] = (CtxFlags & GLCF_STENC_MASK) >> GLCF_STENC_SHFT;
  }
  attributes [count++] = PGL_None;

  viscfg = pglChooseConfig (glAB, attributes);
  if (!viscfg)
    return false;

  hgc = pglCreateContext (glAB, viscfg, (HGC)NULL, TRUE);
  if (!hgc)
    return false;

  return true;
}

bool glWindow::Unbind (bool Destroy)
{
  if (!hwndCL)
    return false;
  if (Destroy)
    if (hwndFR)
      WinDestroyWindow (hwndFR);
    else
      WinDestroyWindow (hwndCL);
  else
  {
    // Restore initial window state
    if (OldClientWindowProc)
      WinSubclassWindow (hwndCL, OldClientWindowProc);
    if (OldFrameWindowProc)
      WinSubclassWindow (hwndFR, OldFrameWindowProc);
  }
  hwndCL = NULLHANDLE;
  hwndFR = NULLHANDLE;
  glMN = NULLHANDLE;
  return true;
}

bool glWindow::Show (bool Visible)
{
  return WinSetWindowPos (hwndFR, HWND_TOP, 0, 0, 0, 0,
    Visible ? SWP_SHOW | SWP_ACTIVATE : SWP_HIDE);
}

// Black magic: shake window a bit so that OpenGL can catch its size/position
bool glWindow::Reset ()
{
  SWP swp;
  if (!WinQueryWindowPos (hwndFR, &swp))
    return false;
  if (!WinSetWindowPos (hwndFR, HWND_TOP, swp.x - 1, swp.y, swp.cx, swp.cy,
    SWP_SIZE | SWP_MOVE))
    return false;
  return WinSetWindowPos (hwndFR, HWND_TOP, swp.x, swp.y, swp.cx, swp.cy,
    SWP_SIZE | SWP_MOVE);
}

bool glWindow::SetPos (long X, long Y)
{
  SWP swp;
  RECTL r;

  if (!WinQueryWindowPos (hwndFR, &swp))
    return false;
  r.xLeft = swp.x;
  r.xRight = swp.x + swp.cx;
  r.yBottom = swp.y;
  r.yTop = swp.y + swp.cy;
  if (!WinCalcFrameRect (hwndFR, &r, TRUE))
    memset (&r, 0, sizeof (r));

  if (X != LONG_MIN)
  {
    long deltaX = X - r.xLeft;
    r.xLeft += deltaX;
    r.xRight += deltaX;
  }
  if (Y != LONG_MIN)
  {
    long deltaY = Y - r.yBottom;
    r.yTop += deltaY;
    r.yBottom += deltaY;
  }
  if (!WinCalcFrameRect (hwndFR, &r, FALSE))
    return false;

  return WinSetWindowPos (hwndFR, HWND_TOP, r.xLeft, r.yBottom,
    r.xRight - r.xLeft, r.yTop - r.yBottom,
    SWP_SIZE | SWP_MOVE);
}

MRESULT glWindow::ClientHandler (HWND Handle, ULONG Message, MPARAM MsgParm1, MPARAM MsgParm2)
{
  glWindow *win = (glWindow *)WinQueryWindowPtr (Handle, QWL_USER);

  if (win)
    return win->ClientMessage (Message, MsgParm1, MsgParm2);
  else
    return NULL;
}

MRESULT glWindow::FrameHandler (HWND Handle, ULONG Message, MPARAM MsgParm1, MPARAM MsgParm2)
{
  glWindow *win = (glWindow *) WinQueryWindowPtr (Handle, QWL_USER);

  if (win)
    return win->FrameMessage (Message, MsgParm1, MsgParm2);
  else
    return NULL;
}

MRESULT glWindow::ClientMessage (ULONG Message, MPARAM MsgParm1, MPARAM MsgParm2)
{
  SWP swp;
  bool m_down;
  int m_button;
  int m_capture;

  switch (Message)
  {
    case WM_PAINT:
      FrameCount++;
      glFlush ();
      if (csSwapFloatBuffers)
      {
        csSwapFloatBuffers = false;
        if (viscfg->doubleBuffer)
        {
          pglSwapBuffers (glAB, hwndCL);
          ActiveBuffer ^= 1;
        }
      }
      DosPostEventSem (sRedrawComplete);
      return OldClientWindowProc (hwndCL, Message, MsgParm1, MsgParm2);
    case WM_COMMAND:
      switch ((USHORT) MsgParm1)
      {
        case cmdAlignLeft:
          if (WinQueryWindowPos (hwndFR, &swp))
            WinSetWindowPos (hwndFR, HWND_TOP, 0, swp.y, 0, 0, SWP_MOVE);
          return 0;
        case cmdAlignRight:
          if (WinQueryWindowPos (hwndFR, &swp))
            WinSetWindowPos (hwndFR, HWND_TOP, DesktopW - swp.cx, swp.y, 0, 0, SWP_MOVE);
          return 0;
        case cmdAlignBottom:
          if (WinQueryWindowPos (hwndFR, &swp))
            WinSetWindowPos (hwndFR, HWND_TOP, swp.x, 0, 0, 0, SWP_MOVE);
          return 0;
        case cmdAlignTop:
          if (WinQueryWindowPos (hwndFR, &swp))
            WinSetWindowPos (hwndFR, HWND_TOP, swp.x, DesktopH - swp.cy, 0, 0, SWP_MOVE);
          return 0;
        case cmdAlignCenter:
          Resize (-1, -1, true);
          return 0;
        case cmdPause:
          Pause (!fPause);
          return 0;
        case cmdToggleMouse:
          MouseVisible (!fMouseVisible);
          return 0;
        case cmdFullScreen:
          FullScreen (!fFullScreen);
          return 0;
        case cmdToggleAspect:
          MaintainAspectRatio (!fAspect);
          if (fAspect)
            if (WinQueryWindowPos (hwndCL, &swp))
              Resize (swp.cx, swp.cy, false);
          return 0;
        default:
          return OldClientWindowProc (hwndCL, Message, MsgParm1, MsgParm2);
      }
    case WM_CHAR:
    {
      // err.... for some strange reasons '/' on numeric keypad
      // emits very strange codes for me ...
      unsigned short flags = SHORT1FROMMP (MsgParm1);
      if ((flags & (KC_CHAR | KC_SCANCODE)) == KC_CHAR && flags == 0x7f)
        MsgParm1 = MPARAM (int (MsgParm1) | 0x6f000000 | KC_SCANCODE);
      // Also ESC does not set KC_CHAR flag...
      if ((flags & KC_SCANCODE) && ((SHORT2FROMMP (MsgParm1) >> 8) == 1))
      {
        flags |= KC_CHAR;
        MsgParm2 = MPARAM (int (MsgParm2) | 0x1b000000);
      }
      if (hKeyboard && (flags & (KC_SCANCODE | KC_CHAR)))
      {
        unsigned short ScanCode = (flags & KC_SCANCODE) ? SHORT2FROMMP (MsgParm1) : 0;
        unsigned short CharCode = (flags & KC_CHAR) ? SHORT1FROMMP (MsgParm2) : 0;
        int RepeatCount = (ScanCode & 0xFF);
        ScanCode >>= 8;

        if (ScanCode < 128)
          if (flags & KC_KEYUP)
          {
            if (!CharCode)
              CharCode = lastKeyCode [ScanCode];
          }
          else
            lastKeyCode [ScanCode] = CharCode;

        hKeyboard (paramKeyboard, ScanCode, CharCode, !(flags & KC_KEYUP),
          RepeatCount, (flags & KC_SHIFT ? KF_SHIFT : 0) |
          (flags & KC_ALT ? KF_ALT : 0) | (flags & KC_CTRL ? KF_CTRL : 0));
        return (MRESULT) TRUE;
      }
      return (MRESULT) FALSE;
    }
    case WM_MOUSEMOVE:
      if (fActive)
        WinSetPointer (HWND_DESKTOP, fMouseVisible ? WinQuerySysPointer (HWND_DESKTOP,
          MouseCursorID, FALSE) : NULL);
      m_down = false;
      m_button = 0;
      goto ProcessMouse;
    case WM_BUTTON1DOWN:
    case WM_BUTTON1DBLCLK:
      m_down = true;
      m_button = 1;
      MouseButtonMask |= 1;
      goto ProcessMouse;
    case WM_BUTTON2DOWN:
    case WM_BUTTON2DBLCLK:
      m_down = true;
      m_button = 2;
      MouseButtonMask |= 2;
      goto ProcessMouse;
    case WM_BUTTON3DOWN:
    case WM_BUTTON3DBLCLK:
      m_down = true;
      m_button = 3;
      MouseButtonMask |= 4;
      goto ProcessMouse;
    case WM_BUTTON1UP:
      m_down = false;
      m_button = 1;
      MouseButtonMask &= ~1;
      goto ProcessMouse;
    case WM_BUTTON2UP:
      m_down = false;
      m_button = 2;
      m_capture = 2;
      MouseButtonMask &= ~2;
      goto ProcessMouse;
    case WM_BUTTON3UP:
      m_down = false;
      m_button = 3;
      m_capture = 2;
      MouseButtonMask &= ~4;
ProcessMouse:
      if (!fActive)
        return OldClientWindowProc (hwndCL, Message, MsgParm1, MsgParm2);
      if (hMouse)
      {
        unsigned short flags = SHORT2FROMMP (MsgParm2);
        hMouse (paramMouse, m_button, m_down, (short)SHORT1FROMMP (MsgParm1),
          (short)SHORT2FROMMP (MsgParm1), (flags & KC_SHIFT ? KF_SHIFT : 0) |
          (flags & KC_ALT ? KF_ALT : 0) | (flags & KC_CTRL ? KF_CTRL : 0));
      }
      if (m_down)
        WinSetActiveWindow (HWND_DESKTOP, hwndCL);
      if (MouseButtonMask && !MouseCaptured)
        MouseCaptured = WinSetCapture (HWND_DESKTOP, hwndCL);
      if (!MouseButtonMask && MouseCaptured)
        MouseCaptured = !WinSetCapture (HWND_DESKTOP, NULLHANDLE);
      return (MRESULT) TRUE;
    default:
      return OldClientWindowProc (hwndCL, Message, MsgParm1, MsgParm2);
  }
}

MRESULT glWindow::FrameMessage (ULONG Message, MPARAM MsgParm1, MPARAM MsgParm2)
{
  switch (Message)
  {
    case WM_ADJUSTWINDOWPOS:
    {
      PSWP swp;
      SWP oldpos;
      MRESULT res;
      bool cLS, cRS, cBS, cTS;
      RECTL r;

      swp = (PSWP) MsgParm1;

      if (swp->fl & (SWP_MAXIMIZE | SWP_MINIMIZE | SWP_RESTORE))
        FullScreen (false);

      if (!AllowResize)
        swp->fl &= ~SWP_SIZE;

      if (fFullScreen)
      {
        r.xLeft = 0;
        r.xRight = ScreenW;
        r.yBottom = 0;
        r.yTop = ScreenH;
        if (WinCalcFrameRect (hwndFR, &r, FALSE))
        {
          if (swp->fl & SWP_SIZE)
          {
            swp->cx = r.xRight - r.xLeft;
            swp->cy = r.yTop - r.yBottom;
          }
          if (swp->fl & SWP_MOVE)
          {
            swp->x = r.xLeft;
            swp->y = r.yBottom;
          }
        }
      }

      if ((swp->fl & SWP_SIZE) && !fMinimized)
      {
        WinQueryWindowPos (hwndFR, &oldpos);
        cLS = (oldpos.x == swp->x);
        cRS = ((oldpos.x + oldpos.cx) == (swp->x + swp->cx));
        cBS = (oldpos.y == swp->y);
        cTS = ((oldpos.y + oldpos.cy) == (swp->y + swp->cy));
      } else
      {
        cLS = false;
        cRS = false;
        cBS = false;
        cTS = false;
      }

      res = OldFrameWindowProc (hwndFR, Message, MsgParm1, MsgParm2);

      if ((long) res & AWP_MAXIMIZED)
      {
        fMinimized = false;
        cLS = false;
        cRS = false;
        cBS = false;
        cTS = false;
      }
      if ((long) res & AWP_MINIMIZED)
      {
        fMinimized = true;
        cLS = false;
        cRS = false;
        cBS = false;
        cTS = false;
      }
      if ((long) res & AWP_RESTORED)
      {
        fMinimized = false;
        cLS = false;
        cRS = false;
        cBS = false;
        cTS = false;
      }
      if ((swp->fl & SWP_SIZE) && !fMinimized)
      {
        // Adjust window aspect ratio
        if (fAspect && !fFullScreen)
        {
          r.xLeft = 0;
          r.xRight = swp->cx;
          r.yBottom = 0;
          r.yTop = swp->cy;
          WinCalcFrameRect (hwndFR, &r, TRUE);
          r.xRight -= r.xLeft;
          r.yTop -= r.yBottom;
          AdjustAspectRatio (&r.xRight, &r.yTop);
          r.xLeft = 0;
          r.yBottom = 0;
          WinCalcFrameRect (hwndFR, &r, false);
          swp->cx = r.xRight - r.xLeft;
          swp->cy = r.yTop - r.yBottom;
        }
        // Check for lowest size limit
        r.xLeft = swp->x;
        r.xRight = swp->x + swp->cx;
        r.yBottom = swp->y;
        r.yTop = swp->y + swp->cy;
        WinCalcFrameRect (hwndFR, &r, TRUE);

        r.xRight -= r.xLeft;
        if (r.xRight < 320 && r.xRight > 0)
          r.xRight = 320;
        BufferW = r.xRight;
        r.xRight += r.xLeft;

        r.yTop -= r.yBottom;
        if (r.yTop < 200 && r.yTop > 0)
          r.yTop = 200;
        BufferH = r.yTop;
        r.yTop += r.yBottom;

        WinCalcFrameRect (hwndFR, &r, FALSE);
        swp->x = r.xLeft;
        swp->cx = r.xRight - r.xLeft;
        swp->y = r.yBottom;
        swp->cy = r.yTop - r.yBottom;

        // Check for unmovable bounds
        if (cLS)
          swp->x = oldpos.x;
        else if (cRS)
          swp->x = oldpos.x + oldpos.cx - swp->cx;

        if (cBS)
          swp->y = oldpos.y;
        else if (cTS)
          swp->y = oldpos.y + oldpos.cy - swp->cy;
      }

      if (hResize && (swp->fl & SWP_SIZE))
        hResize (paramResize);
      return res;
    }
    case WM_ACTIVATE:
      if ((HWND) MsgParm2 == hwndFR)
      {
        fActive = (int) MsgParm1;
        if (hFocus)
          hFocus (paramFocus, fActive);
      }
      return OldFrameWindowProc (hwndFR, Message, MsgParm1, MsgParm2);
    case WM_DESTROY:
      lastError = glerDestroyed;
      if (hTerminate)
        hTerminate (paramTerminate);
      return FALSE;
    default:
      return OldFrameWindowProc (hwndFR, Message, MsgParm1, MsgParm2);
  }
}

bool glWindow::Resize (long Width, long Height, bool Center)
{
  SWP swp;
  RECTL r;

  if (!WinQueryWindowPos (hwndFR, &swp))
    return false;
  r.xLeft = swp.x;
  r.xRight = swp.x + swp.cx;
  r.yBottom = swp.y;
  r.yTop = swp.y + swp.cy;
  if (!WinCalcFrameRect (hwndFR, &r, TRUE))
    memset (&r, 0, sizeof (r));

  if (!fFullScreen)
  {
    if ((Width >= 0) && (Height >= 0))
    {
      AdjustAspectRatio (&Width, &Height);
      r.xRight = r.xLeft + Width;
      r.yTop = r.yBottom + Height;
    }
    if (Center)
    {
      swp.cx = r.xRight - r.xLeft;
      swp.cy = r.yTop - r.yBottom;
      r.xLeft = (ScreenW - swp.cx) / 2;
      r.yBottom = (ScreenH - swp.cy) / 2;
      r.xRight = r.xLeft + swp.cx;
      r.yTop = r.yBottom + swp.cy;
    }
    if (!WinCalcFrameRect (hwndFR, &r, FALSE))
      return false;
  }

  return WinSetWindowPos (hwndFR, NULLHANDLE, r.xLeft, r.yBottom,
    r.xRight - r.xLeft, r.yTop - r.yBottom,
    SWP_SIZE | SWP_MOVE);
}

bool glWindow::AdjustAspectRatio (long *Width, long *Height)
{
  if (!AllowResize)
  {
    *Width = BufferW;
    *Height = BufferH;
    return true;
  }

  if (!fAspect || fFullScreen)
    return true;
  ULONG K = ((*Width << 16) / BufferW + (*Height << 16) / BufferH) / 2;

  *Width = (BufferW * K) >> 16;
  *Height = (BufferH * K) >> 16;
  if (*Width > ScreenW)
  {
    *Height = (*Height * ScreenW) / *Width;
    *Width = ScreenW;
  }
  if (*Height > ScreenH)
  {
    *Width = (*Width * ScreenH) / *Height;
    *Height = ScreenH;
  }
  return true;
}

bool glWindow::FullScreen (bool State)
{
  if (fFullScreen != State)
    switch (State)
    {
      case false:
        fFullScreen = false;
        WinSetWindowPos (hwndFR, NULLHANDLE, swpFullScreen.x, swpFullScreen.y,
          swpFullScreen.cx, swpFullScreen.cy, SWP_MOVE | SWP_SIZE);
        break;
      case true:
        WinQueryWindowPos (hwndFR, &swpFullScreen);
        fFullScreen = true;
        Resize (-1, -1, false);
        break;
    }
  return true;
}

bool glWindow::DisableAccelTable ()
{
  return WinSetAccelTable (WinQueryAnchorBlock (hwndFR), NULLHANDLE, NULLHANDLE);
}

void glWindow::MouseVisible (bool State)
{
  fMouseVisible = State;
  WinPostMsg (glMN, MM_SETITEMATTR, MPFROM2SHORT (cmdToggleMouse, TRUE),
    MPFROM2SHORT(MIA_CHECKED, fMouseVisible ? MIA_CHECKED : 0));
  /* Send a pseudo mouse-move message to get mouse pointer updated */
  POINTL mouse;
  WinQueryPointerPos (HWND_DESKTOP, &mouse);
  WinMapWindowPoints (HWND_DESKTOP, hwndCL, &mouse, 1);
  RECTL winpos;
  WinQueryWindowRect (hwndCL, &winpos);
  if ((mouse.x >= winpos.xLeft) && (mouse.x < winpos.xRight)
   && (mouse.y >= winpos.yBottom) && (mouse.y < winpos.yTop))
    WinPostMsg (hwndCL, WM_MOUSEMOVE, MPFROM2SHORT (mouse.x, mouse.y),
      MPFROM2SHORT (HT_NORMAL, KC_NONE));
}

void glWindow::SetPalette (u_long *palette, int count)
{
  if (hpal)
    GpiDeletePalette (hpal);
  hpal = GpiCreatePalette (glAB, LCOL_PURECOLOR | LCOL_OVERRIDE_DEFAULT_COLORS,
    LCOLF_CONSECRGB, 256, palette);
  if (hpal)
    pglSelectColorIndexPalette (glAB, hpal, hgc);
}
