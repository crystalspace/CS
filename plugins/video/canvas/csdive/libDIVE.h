/*
    OS/2 DIVE class library: public header file
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

#ifndef __LIBDIVE_H__
#define __LIBDIVE_H__

//*** Definitions for Resource Compiler ****************************************

// DIVE Window resources ID
#define idDive          32100

// Messages handled by DIVE window handler
#define cmdAlign        32101
#define cmdAlignTop     32102
#define cmdAlignBottom  32103
#define cmdAlignLeft    32104
#define cmdAlignRight   32105
#define cmdAlignCenter  32106
#define cmdScale        32107
#define cmdSnap1to1     32108
#define cmdSnap2to1     32109
#define cmdSnap3to1     32110
#define cmdSnap4to1     32111
#define cmdFullScreen   32112
#define cmdToggleAspect 32113
#define cmdPause        32114
#define cmdToggleMouse  32115

#ifndef RC_INVOKED

#include <stdlib.h>
#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#include <pmddi.h>
#include <mmioos2.h>
#include <dive.h>
#include <fourcc.h>
#include <sys/types.h>

//*** Video mode structures and definitions *************************************
#define vmfWindowed	0x00000001	// 1 - window; 0 - full-screen
#define vmfNative	0x00000002	// 1 - native mode; 0 - emulated
#define vmfRGBAmode	0x00000004	// 1 - OpenGL in RGBA mode, 0 - color index

#define vmfAlphaBuffer	0x00010000	// 1 - Alpha buffer supported
#define vmfHardware2D	0x00020000	// 1 - Hardware-supported 2D acceleration
#define vmfHardware3D	0x00040000	// 1 - Hardware-supported 3D acceleration

// Read-only vmfXXX flags
#define vmfReadOnly      vmfWindowed + vmfNative

// Handy typedefs
typedef unsigned int u_int;
typedef unsigned char u_char;

struct FGVideoMode
{
  u_int Width, Height;                  // Width and Height in pixels
  // Readonly if (Flags & vmfWindowed == 0)
  u_int PixelFormat;                    // Pixel format - FOURCC_###
  // Readonly
  u_int Buffers;                        // Available videobuffers (double buffering
                                        // available if >1)
  // Read/write
  u_int Flags;                          // See vmfXXX constants above
  // Flags & vmfReadOnly are readonly
  u_int IndexBits;                      // Bits per pixel in color index mode (OpenGL
                                        // only)
  // Readonly if >0; otherwise read/write
  u_int DepthBits;                      // Bits per element of depth buffer
  // Readonly if >0; otherwise read/write
  u_int StencilBits;                    // Bits per element of stencil buffer
  // Readonly if >0; otherwise read/write
  u_int AccumBits;                      // Bits per element of stencil buffer
  // Readonly if >0; otherwise read/write
};

// DIVE errors
enum tDiveError
{
  derrOK,                               // All right
  derrBadNBuffers,                      // Invalid number of buffers
  derrBadWindow,                        // Cannot set some window parameters
  derrCannotOpen,                       // Cannot open DIVE instance
  derrAllocBuffer,                      // Cannot allocate DIVE buffer
  derrCreateSem,                        // Cannot create event semaphore
  derrPalette,                          // Cannot set up palette
  derrDestroyed                         // User dismissed the window
};

#define DIVE_MAXBUFFERS 16		// Maximal number of backbuffers
#define DIVE_NEXTBUFFER -1              // Choose next backbuffer

extern FGVideoMode *vmList;             // Pointer to an array of tVideoMode`s
extern u_int vmCount;                   // Number of videomodes in vmList
extern long DesktopW, DesktopH;         // Desktop width and height

typedef void (*tKeyboardHandler) (void *param, unsigned char ScanCode,
  unsigned char CharCode, bool Down, unsigned char RepeatCount, int ShiftFlags);
typedef void (*tMouseHandler) (void *param, int Button, bool Down,
  int x, int y, int ShiftFlags);
typedef void (*tTerminateHandler) (void *param);
typedef void (*tFocusHandler) (void *param, bool Enable);

// Mask for shift keys (used in mouse handler)
#define KF_SHIFT 0x000000001
#define KF_ALT   0x000000002
#define KF_CTRL  0x000000004

/**
 * DIVE window class
 * This class implements everything needed to use a DIVE context.
 * If you attach it to a frame window (by passing a handle of a
 * frame window in constructor) it will replace system menu (or
 * application menu if there is one) by a specialized one. If you
 * pass a handle of a different-type window, DIVE context will simply
 * attach to that window (@@ NOT TESTED! but should work). All window
 * resises, moves etc are tracked so that DIVE buffer always covers
 * entire frame window client area (or entire window area). You can
 * set up some additional callbacks to process all input to the window
 * such as mouse and keyboard.
 */
class diveWindow
{
private:
  PFNWP OldClientWindowProc, OldFrameWindowProc;
  HDIVE hDive;                          // DIVE handle
  ULONG hBuffer[DIVE_MAXBUFFERS];       // Image buffers
  FOURCC BufferF;                       // Image buffer format
  long BufferW, BufferH;                // Image buffers parameters
  long nBuffers;                        // Count of image buffers
  long ActiveBuffer;                    // ID of active image buffer
  long VisibleBuffer;                   // ID of visible image buffer
  long ScreenW, ScreenH;                // Screen size
  ULONG *CLUT;                          // The palette for 256-color images
  HEV sRedrawComplete;                  // Redraw complete event semaphore
  bool fAspect;                         // Maintain aspect ratio?
  bool fPause;                          // Set to true will freeze drawing
  bool fMouseVisible;                   // If false mouse pointer will be hidden
  ULONG MouseCursorID;			// Mouse cursor shape ID (SPTR_ARROW ...)
  bool fMinimized;                      // This is true when window is minimized
  bool fFullScreen;                     // true if we`re in full-screen
  bool fPhysCLUT;                       // true if we changed physical palette
  bool fActive;                         // Window is active
  long FailedCount;                     // Count failed attempts to redraw between
                                        // WM_VRNDIS/ENABLED
  SWP swpFullScreen;                    // Window position before full-screen
  long WindowW;                         // Client window width
  long WindowH;                         // Client window height
  RECTL DirtyRect;                      // Rectangle that should be updated
  RECTL oldDirtyRect;                   // Dirty rectangle that is already set up
  ULONG MouseButtonMask;		// Current mouse button states
  bool MouseCaptured;			// Mouse captured flag

  tKeyboardHandler hKeyboard;           // Keyboard handler if not NULL
  void *paramKeyboard;                  // Parameter passed to keyboard handler
  tTerminateHandler hTerminate;         // Called on window close if not NULL
  void *paramTerminate;                 // Parameter passed to terminate handler
  tFocusHandler hFocus;                 // Called on focus change if not NULL
  void *paramFocus;                     // Parameter passed to focus handler
  tMouseHandler hMouse;                 // Called on mouse events if not NULL
  void *paramMouse;                     // Parameter passed to mouse handler

  char lastKeyCode [128];		// Last scan->character encountered

public:
  HWND diveFR, diveCL, diveMN;          // Window frame, client and menu handles
  ULONG FrameCount;                     // Incremented on each complete redraw
  tDiveError lastError;                 // Last error condition

  /// Constructor: attach DIVE view to client of given frame handle
  diveWindow (long Width, long Height, FOURCC Format, long nBuff);

  /// Destructor: free DIVE buffers and memory
  virtual ~diveWindow ();

  /// Bind DIVE buffer to a PM window, either client or frame window
  bool Bind (HWND winHandle);

  /// Unbind from window; optionally destroy window
  bool Unbind (bool Destroy);

  /// Set *logical* color lookup table (AKA palette)
  bool SetCLUT (ULONG * NewCLUT, int Count);

  /// Set physical CLUT to logical CLUT
  bool SetPhysCLUT ();

  /// Restore default desktop palette
  bool ResetPhysCLUT ();

  /// Show the window
  bool Show (bool Visible);

  /// Disable standard accelerator table for window so we can use ALT, ALT+F4 etc
  bool DisableAccelTable ();

  /// Two virtual functions that performs PM message management for frame and client
  virtual MRESULT ClientMessage (ULONG Message, MPARAM MsgParm1, MPARAM MsgParm2);
  virtual MRESULT FrameMessage (ULONG Message, MPARAM MsgParm1, MPARAM MsgParm2);

  /// Call this to get the address of the buffer to paint in
  u_char *BeginPaint (ULONG * BytesPerLine, long BufferNo);

  /// Call this when you`re done with painting
  void EndPaint ();

  /// Enable or disable constant aspect ratio of window
  inline void MaintainAspectRatio (bool State)
  {
    if (!fFullScreen)
    {
      fAspect = State;
      WinCheckMenuItem (diveMN, cmdToggleAspect, fAspect);
    }
  }

  /// Pause window: application will skip ticks in BeginPaint ()
  inline void Pause (bool State)
  {
    fPause = State;
    WinCheckMenuItem (diveMN, cmdPause, fPause);
  }

  /// Set mouse cursor visibility when it is over DIVE window
  void MouseVisible (bool State);

  /// Set mouse cursor shape when it is over DIVE window
  inline void MouseCursor (ULONG ID)
  { MouseCursorID = ID; }

  /// Repaint DIVE buffer
  inline void Switch (long BufferNo, PRECTL rect = NULL)
  {
    ULONG Count;
    DosResetEventSem (sRedrawComplete, &Count);
    VisibleBuffer = (BufferNo == DIVE_NEXTBUFFER) ? ActiveBuffer : BufferNo;
    WinInvalidateRect (diveCL, rect, FALSE);
  }

  /// Wait until buffer redraw is complete. Recommended before calling Switch()
  inline void WaitSwitch ()
  {
    DosWaitEventSem (sRedrawComplete, 1000);
  }

  /// Return active buffer
  inline long ActiveBuff ()
  {
    return ActiveBuffer;
  }

  /// Set window title: This function requires a message queue
  inline bool SetTitle (char *Title)
  {
    return WinSetWindowText (diveFR, (PSZ) Title);
  }

  /// Send a message to frame window (any of cmdXXX works too)
  inline void Command (ULONG CommandCode)
  {
    WinPostMsg (diveCL, WM_COMMAND, MPFROMSHORT (CommandCode), MPFROM2SHORT (CMDSRC_OTHER, CommandCode));
  }

  /// Adjust these variables so that Width/Height == BufferWidth/BufferHeight
  bool AdjustAspectRatio (long *Width, long *Height);

  /// Resize *client* window, compute and set new frame window size
  bool Resize (long Width, long Height, bool Center);

  /// Resize *image buffer*; all previous images will be lost
  bool ResizeBuffer (long Width, long Height, FOURCC Format);

  /// Switch to/from full-screen mode
  bool FullScreen (bool State);

  /// Set DIVE window position
  bool SetPos (long X, long Y);

  /// Define Keyboard Handler
  inline void SetKeyboardHandler (tKeyboardHandler Handler, void *param)
  {
    hKeyboard = Handler;
    paramKeyboard = param;
  }

  /// Define Terminate Handler
  inline void SetTerminateHandler (tTerminateHandler Handler, void *param)
  {
    hTerminate = Handler;
    paramTerminate = param;
  }

  /// Define mouse handler
  inline void SetMouseHandler (tMouseHandler Handler, void *param)
  {
    hMouse = Handler;
    paramMouse = param;
  }

  /// Define window focus change handler
  inline void SetFocusHandler (tFocusHandler Handler, void *param)
  {
    hFocus = Handler;
    paramFocus = param;
  }

  /// Return if window is paused
  inline bool isPaused ()
  {
    return fPause;
  }

  /// Return 'constant aspect ratio' flag
  inline bool isAspectRatioConst ()
  {
    return fAspect;
  }

  /// Return minimized status
  inline bool isMinimized ()
  {
    return fMinimized;
  }

  /// Return logical buffer width
  inline long BufferWidth ()
  {
    return BufferW;
  }

  /// Return logical buffer height
  inline long BufferHeight ()
  {
    return BufferH;
  }

  /// Return physical client window width
  inline long WindowWidth ()
  {
    return WindowW;
  }

  /// Return physical client window height
  inline long WindowHeight ()
  {
    return WindowH;
  }
private:
  bool SetupBlitter ();
  bool SetupPalette ();
  static MRESULT EXPENTRY ClientHandler (HWND Handle, ULONG Message, MPARAM MsgParm1, MPARAM MsgParm2);
  static MRESULT EXPENTRY FrameHandler (HWND Handle, ULONG Message, MPARAM MsgParm1, MPARAM MsgParm2);
};

// DIVE simple application class; you can use it for simple applications
class diveApp
{
public:
  HAB AB;
  HMQ MQ;
  HWND appWN[16];
  u_int appWNlength;

  diveApp ();
  ~diveApp ();
  HWND CreateWindow (PSZ Title, HMODULE ModID, ULONG ResID, ULONG Flags);
  void Run ();
  bool ProcessQueuedMessages ();
};

#endif // RC_INVOKED

#endif // __LIBDIVE_H__
