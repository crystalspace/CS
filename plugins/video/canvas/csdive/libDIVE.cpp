/*
    OS/2 DIVE class library
    Copyright (C) 1997 by Andrew Zabolotny <bit@eltech.ru>

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
#include <alloca.h>
#include "cssysdef.h"
#include "libDIVE.h"

static long MAX_PHYS_COLORS = -1;
static bool DEVICE_HAS_PALETTE = false;

diveApp::diveApp ()
{
  appWNlength = 0;
  AB = WinInitialize (0);
  MQ = WinCreateMsgQueue (AB, 0);
}

diveApp::~diveApp ()
{
  while (appWNlength--)
    WinDestroyWindow (appWN[appWNlength]);
  WinDestroyMsgQueue (MQ);
  WinTerminate (AB);
}

void diveApp::Run ()
{
  QMSG qmsg;

  while (WinGetMsg (AB, &qmsg, 0, 0, 0))
    WinDispatchMsg (AB, &qmsg);
}

bool diveApp::ProcessQueuedMessages ()
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

HWND diveApp::CreateWindow (PSZ Title, HMODULE ModID, ULONG ResID, ULONG Flags)
{
#define DIVE_WINDOW_CLASS (PSZ)"diveView"
  ULONG flStyle;
  HWND diveFR, diveCL;

  if (!WinRegisterClass (AB, DIVE_WINDOW_CLASS, NULL,
         CS_SIZEREDRAW | CS_MOVENOTIFY, sizeof (void *)))
    return 0;

  flStyle = Flags | FCF_SIZEBORDER | FCF_AUTOICON;
  if (flStyle & FCF_SYSMENU)
    flStyle = flStyle | FCF_MENU;
  if (flStyle & FCF_TITLEBAR)
    flStyle = flStyle | FCF_SYSMENU | FCF_MINMAX;
  if (Title)
    flStyle = flStyle | FCF_TASKLIST;

  diveFR = WinCreateStdWindow (HWND_DESKTOP, FS_NOBYTEALIGN, &flStyle,
    DIVE_WINDOW_CLASS, Title, 0, ModID, ResID, &diveCL);

  if (diveFR)
  {
    // Keep track of all windows we created
    appWN[appWNlength++] = diveFR;

    if (Flags & FCF_SYSMENU)            // Replace system menu by 1st submenu under menu
      // bar
    {
      MENUITEM mi, smi;
      HWND hMenu = WinWindowFromID (diveFR, FID_MENU);

      // Find handle of first submenu in menu bar
      WinSendMsg (hMenu, MM_QUERYITEM, MPFROM2SHORT (
          SHORT1FROMMR (WinSendMsg (hMenu, MM_ITEMIDFROMPOSITION, NULL, NULL)),
          false), MPFROMP (&mi));
      // Change menu ID so it will not appear below title bar
      WinSetWindowUShort (hMenu, QWS_ID, 0);

      // Find System Menu and pull-down menu handles
      HWND hSysMenu = WinWindowFromID (diveFR, FID_SYSMENU);

      if (hMenu && hSysMenu)
      {
        // Find the handle of first submenu in system menu
        WinSendMsg (hSysMenu, MM_QUERYITEM, MPFROM2SHORT (
            SHORT1FROMMR (WinSendMsg (hSysMenu, MM_ITEMIDFROMPOSITION, NULL, NULL)),
            false), MPFROMP (&smi));
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
  return diveFR;
}

diveWindow::diveWindow (long Width, long Height, FOURCC Format, long nBuff)
{
  CLUT = NULL;
  hDive = NULLHANDLE;
  lastError = derrOK;
  fFullScreen = false;
  fMinimized = false;
  fPhysCLUT = false;
  fAspect = false;
  fPause = true;
  fMouseVisible = true;
  MouseCursorID = SPTR_ARROW;
  FailedCount = 0;
  ActiveBuffer = 0;
  VisibleBuffer = 0;
  MouseButtonMask = 0;
  MouseCaptured = false;
  MaintainAspectRatio (false);
  Pause (false);
  OldClientWindowProc = NULL;
  OldFrameWindowProc = NULL;
  nBuffers = nBuff;
  BufferW = Width;
  BufferH = Height;
  BufferF = Format;
  diveCL = NULLHANDLE;
  diveFR = NULLHANDLE;
  diveMN = NULLHANDLE;
  hKeyboard = NULL;
  hTerminate = NULL;
  hMouse = NULL;
  hFocus = NULL;
  ScreenW = WinQuerySysValue (HWND_DESKTOP, SV_CXSCREEN);
  ScreenH = WinQuerySysValue (HWND_DESKTOP, SV_CYSCREEN);
  int i;
  for (i = 0; i < DIVE_MAXBUFFERS; i++)
    hBuffer[i] = NULLHANDLE;
  DirtyRect.xLeft = 1;
  DirtyRect.xRight = -1;

  if (nBuff > DIVE_MAXBUFFERS || nBuff < 1)
  {
    lastError = derrBadNBuffers;
    return;
  }
  if (MAX_PHYS_COLORS == -1)            // Query physical screen parameters
  {
    HPS hps = WinGetScreenPS (HWND_DESKTOP);
    HDC hdc = GpiQueryDevice (hps);

    DevQueryCaps (hdc, CAPS_PHYS_COLORS, 1L, &MAX_PHYS_COLORS);
    DevQueryCaps (hdc, CAPS_ADDITIONAL_GRAPHICS, 1L, (LONG *) & DEVICE_HAS_PALETTE);
    DEVICE_HAS_PALETTE &= CAPS_PALETTE_MANAGER;
    WinReleasePS (hps);
  }
  if (DiveOpen (&hDive, FALSE, NULL) != DIVE_SUCCESS)
  {
    lastError = derrCannotOpen;
    return;
  }
  if (DosCreateEventSem (NULLHANDLE, &sRedrawComplete, 0, FALSE))
  {
    lastError = derrCreateSem;
    return;
  }
}

diveWindow::~diveWindow ()
{
  Unbind (false);
  DosCloseEventSem (sRedrawComplete);   // Destroy redraw semaphore
  int i;
  for (i = 0; i <= nBuffers; i++)   // Free image buffers
    DiveFreeImageBuffer (hDive, hBuffer[i]);
  DiveClose (hDive);
  ResetPhysCLUT ();
}

bool diveWindow::Bind (HWND winHandle)
{
  diveFR = winHandle;
  diveCL = WinWindowFromID (diveFR, FID_CLIENT);
  if (diveCL == NULLHANDLE)
  {
    diveCL = diveFR;
    diveFR = NULLHANDLE;
    diveMN = NULLHANDLE;
  } else
  {
    diveMN = WinWindowFromID (diveFR, FID_MENU);
    if (!diveMN)
      diveMN = WinWindowFromID (diveFR, FID_SYSMENU);
    MouseVisible (fMouseVisible);
    Pause (fPause);
  }

  if (!WinSetWindowPtr (diveCL, QWL_USER, this))
  {
    lastError = derrBadWindow;
    return false;
  }
  if (diveFR && !WinSetWindowPtr (diveFR, QWL_USER, this))
  {
    lastError = derrBadWindow;
    return false;
  }
  // Query current window width and height
  SWP swp;

  WinQueryWindowPos (diveCL, &swp);
  WindowW = swp.cx;
  WindowH = swp.cy;

  OldClientWindowProc = WinSubclassWindow (diveCL, &ClientHandler);
  OldFrameWindowProc = WinSubclassWindow (diveFR, &FrameHandler);

  if (!WinSetVisibleRegionNotify (diveCL, TRUE))
  {
    lastError = derrBadWindow;
    return false;
  }
  if (!ResizeBuffer (BufferW, BufferH, BufferF))
  {
    lastError = derrAllocBuffer;
    return false;
  }
  if (!SetupPalette ())
  {
    lastError = derrPalette;
    return false;
  }
  return true;
}

bool diveWindow::Unbind (bool Destroy)
{
  if (!diveCL)
    return false;
  DiveSetupBlitter (hDive, NULL);
  oldDirtyRect.xLeft = -9999;
  if (Destroy)
    if (diveFR)
      WinDestroyWindow (diveFR);
    else
      WinDestroyWindow (diveCL);
  else
  {
    // Restore initial window state
    WinSetVisibleRegionNotify (diveCL, FALSE);
    if (OldClientWindowProc)
      WinSubclassWindow (diveCL, OldClientWindowProc);
    if (OldFrameWindowProc)
      WinSubclassWindow (diveFR, OldFrameWindowProc);
  }
  diveCL = NULLHANDLE;
  diveFR = NULLHANDLE;
  diveMN = NULLHANDLE;
  return true;
}

bool diveWindow::DisableAccelTable ()
{
  return WinSetAccelTable (WinQueryAnchorBlock (diveFR), NULLHANDLE, NULLHANDLE);
}

void diveWindow::MouseVisible (bool State)
{
  fMouseVisible = State;
  WinPostMsg (diveMN, MM_SETITEMATTR, MPFROM2SHORT (cmdToggleMouse, TRUE),
    MPFROM2SHORT(MIA_CHECKED, fMouseVisible ? MIA_CHECKED : 0));
  /* Send a pseudo mouse-move message to get mouse pointer updated */
  POINTL mouse;
  WinQueryPointerPos (HWND_DESKTOP, &mouse);
  WinMapWindowPoints (HWND_DESKTOP, diveCL, &mouse, 1);
  RECTL winpos;
  WinQueryWindowRect (diveCL, &winpos);
  if ((mouse.x >= winpos.xLeft) && (mouse.x < winpos.xRight)
   && (mouse.y >= winpos.yBottom) && (mouse.y < winpos.yTop))
    WinPostMsg (diveCL, WM_MOUSEMOVE, MPFROM2SHORT (mouse.x, mouse.y),
      MPFROM2SHORT (HT_NORMAL, KC_NONE));
}

u_char *diveWindow::BeginPaint (ULONG * BytesPerLine, long BufferNo)
{
  unsigned char *buff;
  ULONG TotalLines;

  if (fPause)
  {
    DosSleep (1);
    return NULL;
  }
  if (BufferNo == DIVE_NEXTBUFFER)
  {
    ActiveBuffer++;
    if (ActiveBuffer >= nBuffers)
      ActiveBuffer = 0;
  } else
    ActiveBuffer = BufferNo;
  if (DiveBeginImageBufferAccess (hDive, hBuffer[ActiveBuffer],
      (PBYTE *)&buff, BytesPerLine, &TotalLines) != DIVE_SUCCESS)
    return (NULL);
  return (buff);
}

void diveWindow::EndPaint ()
{
  DiveEndImageBufferAccess (hDive, hBuffer[ActiveBuffer]);
}

bool diveWindow::SetupBlitter ()
{
  if (fMinimized || !diveCL)
    return true;                        // Don`t setup blitter when minimized

  if ((oldDirtyRect.xLeft == DirtyRect.xLeft)
    && (oldDirtyRect.xRight == DirtyRect.xRight)
    && (oldDirtyRect.yTop == DirtyRect.yTop)
    && (oldDirtyRect.yBottom == DirtyRect.yBottom))
    return true;                        // Blitter already set up

  oldDirtyRect.xLeft = DirtyRect.xLeft;
  oldDirtyRect.xRight = DirtyRect.xRight;
  oldDirtyRect.yBottom = DirtyRect.yBottom;
  oldDirtyRect.yTop = DirtyRect.yTop;

  HPS hps = WinGetPS (diveCL);
  HRGN hrgn = GpiCreateRegion (hps, 0, NULL);

  WinQueryVisibleRegion (diveCL, hrgn);

  // First, get the rectangles and window coordinates:
  RECTL rctls[256];                     // Rectangles for visible rgn
  RGNRECT rgnCtl;                       // Region control struct
  SETUP_BLITTER SetupBlitter;           // DiveSetupBlitter struct
  POINTL pointl;
  SWP swp;

  // Get rectangles for the visible region
  rgnCtl.ircStart = 0;                  // Enumerate rectangles
  rgnCtl.crc = sizeof (rctls) / sizeof (RECTL); // Max number of rectangles
  rgnCtl.ulDirection = RECTDIR_LFRT_TOPBOT;
  if (GpiQueryRegionRects (hps, hrgn, NULL, &rgnCtl, rctls))
  {
    // Now goes the tricky part: scan all rectangles returned by
    // GpiQueryRegionRects and intersect them with DirtyRect. This way,
    // we'll get updated only the part of screen that has been changed.
	int i;
    for (i = rgnCtl.crcReturned - 1; i >= 0; i--)
    {
      if (DirtyRect.xLeft > rctls[i].xLeft)
        rctls[i].xLeft = DirtyRect.xLeft;
      if (DirtyRect.xRight < rctls[i].xRight)
        rctls[i].xRight = DirtyRect.xRight;
      if (DirtyRect.yBottom > rctls[i].yBottom)
        rctls[i].yBottom = DirtyRect.yBottom;
      if (DirtyRect.yTop < rctls[i].yTop)
        rctls[i].yTop = DirtyRect.yTop;

      if ((rctls[i].xLeft >= rctls[i].xRight)
        || (rctls[i].yBottom >= rctls[i].yTop))
      {
        // the rectangle became empty: we should remove it from the list
        rgnCtl.crcReturned--;
        memmove (&rctls[i], &rctls[i + 1], (rgnCtl.crcReturned - i) * sizeof (RECTL));
      }                                 /* endif */
    }                                   /* endfor */


    // Find the window position relative to its parent.
    WinQueryWindowPos (diveCL, &swp);
    WindowW = swp.cx;
    WindowH = swp.cy;

    // Map window position to the desktop.
    pointl.x = swp.x;
    pointl.y = swp.y;
    WinMapWindowPoints (WinQueryWindow (diveCL, QW_PARENT), HWND_DESKTOP, &pointl, 1);

    // Tell DIVE about the new settings.
    SetupBlitter.ulStructLen = sizeof (SETUP_BLITTER);
    SetupBlitter.fInvert = 0;
    SetupBlitter.fccSrcColorFormat = BufferF;
    SetupBlitter.ulSrcPosX = 0;
    SetupBlitter.ulSrcPosY = 0;
    SetupBlitter.ulSrcWidth = BufferW;
    SetupBlitter.ulSrcHeight = BufferH;
    SetupBlitter.ulDitherType = 0;
    SetupBlitter.fccDstColorFormat = FOURCC_SCRN;
    SetupBlitter.ulDstWidth = swp.cx;
    SetupBlitter.ulDstHeight = swp.cy;
    SetupBlitter.lDstPosX = 0;
    SetupBlitter.lDstPosY = 0;
    SetupBlitter.lScreenPosX = pointl.x;
    SetupBlitter.lScreenPosY = pointl.y;
    SetupBlitter.ulNumDstRects = rgnCtl.crcReturned;
    SetupBlitter.pVisDstRects = rctls;
    DiveSetupBlitter (hDive, &SetupBlitter);
  } else
    DiveSetupBlitter (hDive, NULL);
  WinReleasePS (hps);
  GpiDestroyRegion (hps, hrgn);
  return true;
}

bool diveWindow::SetupPalette ()
{
  if (BufferF != FOURCC_LUT8)
    return true;                        // Only LUT8 buffers needs palette
  DiveSetDestinationPalette (hDive, 0, MAX_PHYS_COLORS, DIVE_PALETTE_PHYSICAL);
  return true;
}

bool diveWindow::SetCLUT (ULONG * NewCLUT, int Count)
{
  if (Count > MAX_PHYS_COLORS)
    return false;

  if (CLUT == NULL)
  {
    CLUT = (ULONG *) malloc (MAX_PHYS_COLORS * sizeof (ULONG));
    if (!CLUT)
      return false;
    memset (CLUT, 0, sizeof (CLUT));
  }
  memcpy (CLUT, NewCLUT, Count * sizeof (LONG));
  DiveSetSourcePalette (hDive, 0, Count, (BYTE *) CLUT);
  return true;
}

bool diveWindow::SetPhysCLUT ()
{
  if (!CLUT)
    return false;
  HPS hps = WinGetScreenPS (HWND_DESKTOP);
  HDC hdc = GpiQueryDevice (hps);

  if (DEVICE_HAS_PALETTE)
  {
    ULONG *tmpCLUT = (ULONG *) alloca (MAX_PHYS_COLORS * sizeof (ULONG));
    ULONG *CurCol = tmpCLUT;
    int Count = 0;

	int i;
    for (i = 0; i < MAX_PHYS_COLORS; i++)
      if (CLUT[i] != 0xffffffff)
      {
        *CurCol++ = CLUT[i];
        Count++;
      }
    if (Count)
    {
      // Reset palette first
      Gre32Entry3 (hdc, 0L, 0x000060C7L);
      // Create a logical color table
      int StartIndex = (MAX_PHYS_COLORS - Count) / 2;

      GpiCreateLogColorTable (hps, LCOL_PURECOLOR | LCOL_REALIZABLE,
        LCOLF_CONSECRGB, StartIndex, Count, (LONG *) tmpCLUT);
      Gre32Entry3 (hdc, 0L, 0x000060C6L);
      WinInvalidateRect (HWND_DESKTOP, (PRECTL) NULL, TRUE);
      fPhysCLUT = true;
    }
  }
  WinReleasePS (hps);
  SetupPalette ();
  return true;
}

bool diveWindow::ResetPhysCLUT ()
{
  if (fPhysCLUT)
  {
    HPS hps = WinGetScreenPS (HWND_DESKTOP);
    HDC hdc = GpiQueryDevice (hps);

    if (DEVICE_HAS_PALETTE)
    {
      Gre32Entry3 (hdc, 0L, 0x000060C7L);
      WinInvalidateRect (HWND_DESKTOP, (PRECTL) NULL, TRUE);
    }
    WinReleasePS (hps);
    SetupPalette ();
  }
  return true;
}

bool diveWindow::Show (bool Visible)
{
  bool rc = WinSetWindowPos (diveFR, NULLHANDLE, 0, 0, 0, 0,
    Visible ? SWP_SHOW + SWP_ACTIVATE : SWP_HIDE);

  DirtyRect.xLeft = 0;
  DirtyRect.xRight = WindowWidth ();
  DirtyRect.yBottom = 0;
  DirtyRect.yTop = WindowHeight ();
  SetupBlitter ();
  return rc;
}

MRESULT diveWindow::ClientHandler (HWND Handle, ULONG Message, MPARAM MsgParm1, MPARAM MsgParm2)
{
  diveWindow *win = (diveWindow *) WinQueryWindowPtr (Handle, QWL_USER);

  if (win)
    return win->ClientMessage (Message, MsgParm1, MsgParm2);
  else
    return NULL;
}

MRESULT diveWindow::FrameHandler (HWND Handle, ULONG Message, MPARAM MsgParm1, MPARAM MsgParm2)
{
  diveWindow *win = (diveWindow *) WinQueryWindowPtr (Handle, QWL_USER);

  if (win)
    return win->FrameMessage (Message, MsgParm1, MsgParm2);
  else
    return NULL;
}

MRESULT diveWindow::ClientMessage (ULONG Message, MPARAM MsgParm1, MPARAM MsgParm2)
{
  SWP swp;
  bool m_down;
  int m_button;
  int m_capture;

  switch (Message)
  {
    case WM_PAINT:
    {
      // Obtain the rectangle that should be repainted
      HPS hps = WinBeginPaint (diveCL, NULLHANDLE, &DirtyRect);
      SetupBlitter ();
      if (DiveBlitImage (hDive, hBuffer[VisibleBuffer], DIVE_BUFFER_SCREEN) != DIVE_SUCCESS)
        FailedCount++;
      WinEndPaint (hps);
      FrameCount++;
      DosPostEventSem (sRedrawComplete);
      return (MRESULT)0;
    }
    case WM_VRNDISABLED:
      FailedCount = 0;
      DiveSetupBlitter (hDive, NULL);
      oldDirtyRect.xLeft = -9999;
      return 0;
    case WM_VRNENABLED:
      // Query current window width and height
      if (WinQueryWindowPos (diveCL, &swp)
       && ((WindowW != swp.cx) || (WindowH != swp.cy)))
      {
        WinQueryWindowPos (diveCL, &swp);
        DirtyRect.xRight = WindowW = swp.cx;
        DirtyRect.yTop = WindowH = swp.cy;
        DirtyRect.xLeft = 0;
        DirtyRect.yBottom = 0;
        FailedCount = 1;
      } /* endif */
      SetupBlitter ();
      if (FailedCount > 0)
        Switch (VisibleBuffer);
      return 0;
    case WM_REALIZEPALETTE:
      SetupPalette ();
      return 0;
    case WM_COMMAND:
      switch ((USHORT)(ULONG)MsgParm1)
      {
        case cmdAlignLeft:
          if (WinQueryWindowPos (diveFR, &swp))
            WinSetWindowPos (diveFR, NULLHANDLE, 0, swp.y, 0, 0, SWP_MOVE);
          return 0;
        case cmdAlignRight:
          if (WinQueryWindowPos (diveFR, &swp))
            WinSetWindowPos (diveFR, NULLHANDLE, ScreenW - swp.cx, swp.y, 0, 0, SWP_MOVE);
          return 0;
        case cmdAlignBottom:
          if (WinQueryWindowPos (diveFR, &swp))
            WinSetWindowPos (diveFR, NULLHANDLE, swp.x, 0, 0, 0, SWP_MOVE);
          return 0;
        case cmdAlignTop:
          if (WinQueryWindowPos (diveFR, &swp))
            WinSetWindowPos (diveFR, NULLHANDLE, swp.x, ScreenH - swp.cy, 0, 0, SWP_MOVE);
          return 0;
        case cmdAlignCenter:
          Resize (-1, -1, true);
          return 0;
        case cmdSnap1to1:
          Resize (BufferW * 1, BufferH * 1, false);
          return 0;
        case cmdSnap2to1:
          Resize (BufferW * 2, BufferH * 2, false);
          return 0;
        case cmdSnap3to1:
          Resize (BufferW * 3, BufferH * 3, false);
          return 0;
        case cmdSnap4to1:
          Resize (BufferW * 4, BufferH * 4, false);
          return 0;
        case cmdFullScreen:
          FullScreen (!fFullScreen);
          return 0;
        case cmdToggleAspect:
          MaintainAspectRatio (!fAspect);
          if (fAspect)
            if (WinQueryWindowPos (diveCL, &swp))
              Resize (swp.cx, swp.cy, false);
          return 0;
        case cmdPause:
          Pause (!fPause);
          return 0;
        case cmdToggleMouse:
          MouseVisible (!fMouseVisible);
          return 0;
        default:
          return OldClientWindowProc (diveCL, Message, MsgParm1, MsgParm2);
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
        return OldClientWindowProc (diveCL, Message, MsgParm1, MsgParm2);
      if (hMouse)
      {
        unsigned short flags = SHORT2FROMMP (MsgParm2);
        hMouse (paramMouse, m_button, m_down, (short)SHORT1FROMMP (MsgParm1),
          (short)SHORT2FROMMP (MsgParm1), (flags & KC_SHIFT ? KF_SHIFT : 0) |
          (flags & KC_ALT ? KF_ALT : 0) | (flags & KC_CTRL ? KF_CTRL : 0));
      }
      if (m_down)
        WinSetActiveWindow (HWND_DESKTOP, diveCL);
      if (MouseButtonMask && !MouseCaptured)
        MouseCaptured = WinSetCapture (HWND_DESKTOP, diveCL);
      if (!MouseButtonMask && MouseCaptured)
        MouseCaptured = !WinSetCapture (HWND_DESKTOP, NULLHANDLE);
      return (MRESULT) TRUE;
    default:
      return OldClientWindowProc (diveCL, Message, MsgParm1, MsgParm2);
  }
}

MRESULT diveWindow::FrameMessage (ULONG Message, MPARAM MsgParm1, MPARAM MsgParm2)
{
  switch (Message)
  {
    case WM_ADJUSTWINDOWPOS:
      PSWP swp;
      SWP oldpos;
      MRESULT res;
      bool cLS, cRS, cBS, cTS;
      RECTL r;

      swp = (PSWP) MsgParm1;

      if (swp->fl & (SWP_MAXIMIZE | SWP_MINIMIZE | SWP_RESTORE))
        FullScreen (false);

      if (fFullScreen)
      {
        r.xLeft = 0;
        r.xRight = ScreenW;
        r.yBottom = 0;
        r.yTop = ScreenH;
        if (WinCalcFrameRect (diveFR, &r, FALSE))
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
        WinQueryWindowPos (diveFR, &oldpos);
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

      res = OldFrameWindowProc (diveFR, Message, MsgParm1, MsgParm2);

      if (long (res) & AWP_MAXIMIZED)
      {
        fMinimized = false;
        cLS = false;
        cRS = false;
        cBS = false;
        cTS = false;
        WinEnableMenuItem (diveMN, cmdScale, FALSE);
      }
      if (long (res) & AWP_MINIMIZED)
      {
        fMinimized = true;
        DiveSetupBlitter (hDive, NULL);
        oldDirtyRect.xLeft = -9999;
      }
      if (long (res) & AWP_RESTORED)
      {
        fMinimized = false;
        cLS = false;
        cRS = false;
        cBS = false;
        cTS = false;
        WinEnableMenuItem (diveMN, cmdScale, TRUE);
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
          WinCalcFrameRect (diveFR, &r, TRUE);
          r.xRight -= r.xLeft;
          r.yTop -= r.yBottom;
          AdjustAspectRatio (&r.xRight, &r.yTop);
          r.xLeft = 0;
          r.yBottom = 0;
          WinCalcFrameRect (diveFR, &r, false);
          swp->cx = r.xRight - r.xLeft;
          swp->cy = r.yTop - r.yBottom;
        }
        // Check for lowest size limit
        r.xLeft = swp->x;
        r.xRight = swp->x + swp->cx;
        r.yBottom = swp->y;
        r.yTop = swp->y + swp->cy;
        WinCalcFrameRect (diveFR, &r, TRUE);

        r.xRight -= r.xLeft;
        if (r.xRight < BufferW / 2 && r.xRight > 0)
          r.xRight = BufferW / 2;
        r.xRight += r.xLeft;

        r.yTop -= r.yBottom;
        if (r.yTop < BufferH / 2 && r.yTop > 0)
          r.yTop = BufferH / 2;
        r.yTop += r.yBottom;

        WinCalcFrameRect (diveFR, &r, FALSE);
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
      return res;
    case WM_ACTIVATE:
      // If we`re in full-screen mode, maintain palette on activation/deactivation
      if (fFullScreen)
      {
        if ((HWND) MsgParm2 == diveFR)
          if ((USHORT)(ULONG) MsgParm1)
            SetPhysCLUT ();
          else
            ResetPhysCLUT ();
      }
      if ((HWND) MsgParm2 == diveFR)
      {
        fActive = (int) MsgParm1;
        if (hFocus)
          hFocus (paramFocus, fActive);
      }
      return OldFrameWindowProc (diveFR, Message, MsgParm1, MsgParm2);
    case WM_QUERYTRACKINFO:
      if (fFullScreen)
        return (MRESULT) FALSE;
      else
        return OldFrameWindowProc (diveFR, Message, MsgParm1, MsgParm2);
    case WM_DESTROY:
      lastError = derrDestroyed;
      if (hTerminate)
        hTerminate (paramTerminate);
      return FALSE;
    default:
      return OldFrameWindowProc (diveFR, Message, MsgParm1, MsgParm2);
  }
}

bool diveWindow::AdjustAspectRatio (long *Width, long *Height)
{
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

bool diveWindow::SetPos (long X, long Y)
{
  SWP swp;
  RECTL r;

  if (!WinQueryWindowPos (diveFR, &swp))
    return false;
  r.xLeft = swp.x;
  r.xRight = swp.x + swp.cx;
  r.yBottom = swp.y;
  r.yTop = swp.y + swp.cy;
  if (!WinCalcFrameRect (diveFR, &r, TRUE))
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
  if (!WinCalcFrameRect (diveFR, &r, FALSE))
    return false;

  return WinSetWindowPos (diveFR, NULLHANDLE, r.xLeft, r.yBottom,
    r.xRight - r.xLeft, r.yTop - r.yBottom,
    SWP_SIZE | SWP_MOVE);
}

bool diveWindow::Resize (long Width, long Height, bool Center)
{
  SWP swp;
  RECTL r;

  if (!WinQueryWindowPos (diveFR, &swp))
    return false;
  r.xLeft = swp.x;
  r.xRight = swp.x + swp.cx;
  r.yBottom = swp.y;
  r.yTop = swp.y + swp.cy;
  if (!WinCalcFrameRect (diveFR, &r, TRUE))
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
    if (!WinCalcFrameRect (diveFR, &r, FALSE))
      return false;
  }
  return WinSetWindowPos (diveFR, NULLHANDLE, r.xLeft, r.yBottom,
    r.xRight - r.xLeft, r.yTop - r.yBottom,
    SWP_SIZE | SWP_MOVE);
}

bool diveWindow::FullScreen (bool State)
{
  if (fFullScreen != State)
    switch (State)
    {
      case false:
        ResetPhysCLUT ();
        fFullScreen = false;
        WinSetWindowPos (diveFR, NULLHANDLE, swpFullScreen.x, swpFullScreen.y,
          swpFullScreen.cx, swpFullScreen.cy, SWP_MOVE | SWP_SIZE);
        break;
      case true:
        WinQueryWindowPos (diveFR, &swpFullScreen);
        fFullScreen = true;
        Resize (-1, -1, false);
        SetPhysCLUT ();
        break;
    }
  return true;
}

bool diveWindow::ResizeBuffer (long Width, long Height, FOURCC Format)
{
  int i;
  // Free image buffers
  for (i = 0; i < DIVE_MAXBUFFERS; i++)
    DiveFreeImageBuffer (hDive, hBuffer[i]);

  // Allocate image buffers
  BufferW = Width;
  BufferH = Height;
  BufferF = Format;
  for (i = 0; i < nBuffers; i++)
    if (DiveAllocImageBuffer (hDive, &hBuffer[i], BufferF,
        BufferW, BufferH, 0, NULL) != DIVE_SUCCESS)
    {
      for (i--; i >= 0; i--)
        DiveFreeImageBuffer (hDive, hBuffer[i]);
      return false;
    }
  DirtyRect.xLeft = 0;
  DirtyRect.xRight = WindowWidth ();
  DirtyRect.yBottom = 0;
  DirtyRect.yTop = WindowHeight ();
  oldDirtyRect.xLeft = -9999;
  SetupBlitter ();
  return true;
}
