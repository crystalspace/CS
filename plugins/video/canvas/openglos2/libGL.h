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

#ifndef __LIBGL_H__
#define __LIBGL_H__

//*** Definitions for Resource Compiler ****************************************

// OpenGL Window resources ID
#define idGL            32100

// Messages handled by GL window handler
#define cmdAlign        32101
#define cmdAlignTop     32102
#define cmdAlignBottom  32103
#define cmdAlignLeft    32104
#define cmdAlignRight   32105
#define cmdAlignCenter  32106
#define cmdPause        32107
#define cmdToggleMouse  32108
#define cmdToggleAspect	32109
#define cmdFullScreen	32110

#ifndef RC_INVOKED

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#include <os2.h>

#include <GL/gl.h>
#include <GL/pgl.h>

// Handy typedefs
typedef unsigned long u_long;
typedef unsigned int u_int;
typedef unsigned char u_char;

/// OpenGL simple application class
class glApp
{
public:
  HAB AB;
  HMQ MQ;
  HWND appWN[16];
  u_int appWNlength;

  glApp ();
  ~glApp ();
  HWND CreateWindow (const char *Title, HMODULE ModID, ULONG ResID, ULONG Flags);
  void Run ();
  bool ProcessQueuedMessages ();
};

// OpenGL wrapper errors
enum tOpenGLError
{
  glerOK,                               // All right
  glerBadNBuffers,                      // Invalid number of buffers
  glerBadWindow,                        // Cannot set some window parameters
  glerCannotOpen,                       // Cannot open GL instance
  glerAllocBuffer,                      // Cannot allocate GL buffer
  glerCreateSem,                        // Cannot create event semaphore
  glerPalette,                          // Cannot set up palette
  glerDestroyed,                        // User dismissed the window
};

extern long DesktopW, DesktopH;         // Desktop width and height

typedef void (*tKeyboardHandler) (void *param, unsigned char ScanCode,
  unsigned char CharCode, int Down, unsigned char RepeatCount, int ShiftFlags);
typedef void (*tMouseHandler) (void *param, int Button, bool Down, int x, int y,
  int ShiftFlags);
typedef void (*tTerminateHandler) (void *param);
typedef void (*tFocusHandler) (void *param, bool Enable);
typedef void (*tResizeHandler) (void *param);

// GL Context bit flags currently supported by OpenGL wrapper
#define GLCF_RGBA	0x80000000	// want RGBA context
#define GLCF_DBLBUFF	0x40000000	// want double buffering
#define GLCF_SNGBUFF	0x20000000	// want single buffering
#define GLCF_STEREO	0x10000000	// want stereo buffers
#define GLCF_STENCIL	0x08000000	// want stencil buffer
#define GLCF_ALPHA_MASK	0x0000000F	// alpha bits
#define GLCF_ALPHA_SHFT	0
#define GLCF_DEPTH_MASK	0x000000F0	// depth bits
#define GLCF_DEPTH_SHFT	4
#define GLCF_RED_MASK	0x00000F00	// red bits
#define GLCF_RED_SHFT	8
#define GLCF_GREEN_MASK	0x0000F000	// green bits
#define GLCF_GREEN_SHFT	12
#define GLCF_BLUE_MASK	0x000F0000	// blue bits
#define GLCF_BLUE_SHFT	16
#define GLCF_STENC_MASK	0x00F00000	// stencil bits
#define GLCF_STENC_SHFT	24

// Mask for shift keys (used in mouse handler)
#define KF_SHIFT	0x00000001
#define KF_ALT		0x00000002
#define KF_CTRL		0x00000004

/**
 * OpenGL window class
 * This class implements everything needed to create and use a OpenGL context
 * under OS/2. If you attach it to a frame window (by passing a handle of a
 * frame window in constructor) it will replace system menu (or
 * application menu if there is one) by a specialized one. If you
 * pass a handle of a different-type window, OpenGL context will simply
 * attach to that window (@@ NOT TESTED! but should work). Window
 * resises, moves etc are disabled because OS/2 OpenGL does not support
 * resizeable output as DIVE does (why???). You can set up some additional
 * callbacks to process all input to the window such as mouse and keyboard.
 */
class glWindow
{
private:
  PFNWP OldClientWindowProc, OldFrameWindowProc;
  long CtxFlags;			// Desired context flags
  long BufferW, BufferH;                // Image buffers parameters
  long ActiveBuffer;                    // ID of active image buffer
  HEV sRedrawComplete;                  // Redraw complete event semaphore
  bool fAspect;				// Maintain window aspect ratio
  bool fPause;                          // Set to TRUE will freeze drawing
  bool fFullScreen;                     // true if we`re in full-screen
  bool fMouseVisible;                   // If FALSE mouse pointer will be hidden
  ULONG MouseCursorID;			// Mouse cursor shape ID (SPTR_ARROW ...)
  bool fMinimized;                      // This is TRUE when window is minimized
  bool fActive;                         // Window is active
  bool csSwapFloatBuffers;			// Do we have to swap buffers on WM_PAINT?
  bool fRedrawDisabled;			// Set between VRN_DISABLED and VRN_ENABLED
  long MouseButtonMask;			// Current mouse button states
  bool MouseCaptured;			// Mouse captured flag
  bool AllowResize;			// Allow window to resize

  tKeyboardHandler hKeyboard;           // Keyboard handler if not NULL
  void *paramKeyboard;                  // Parameter passed to keyboard handler
  tTerminateHandler hTerminate;         // Called on window close if not NULL
  void *paramTerminate;                 // Parameter passed to terminate handler
  tFocusHandler hFocus;                 // Called on focus change if not NULL
  void *paramFocus;                     // Parameter passed to focus handler
  tMouseHandler hMouse;                 // Called on mouse events if not NULL
  void *paramMouse;                     // Parameter passed to mouse handler
  tResizeHandler hResize;		// Window resize handler
  void *paramResize;
  HPAL hpal;				// GPI palette handle
  HGC hgc;				// GL PM context
  int ScreenW, ScreenH;			// Screen size
  SWP swpFullScreen;			// Saved window position

  char lastKeyCode [128];		// Last scan->character encountered

public:
  HAB glAB;				// Application anchor block
  HWND hwndFR, hwndCL, glMN;		// Window frame, client and menu handles
  u_long FrameCount;			// Incremented on each complete redraw
  tOpenGLError lastError;		// Last error condition
  PVISUALCONFIG viscfg;			// GL context configuration

  /// Constructor: attach OpenGL context to client of given frame handle
  glWindow (long Width, long Height, long ContextFlags);

  /// Destructor: free all OpenGL buffers and memory
  virtual ~glWindow ();

  /// Bind OpenGL buffer to a PM window, either client or frame window
  bool Bind (HWND winHandle);

  /// Unbind from window; optionally destroy window
  bool Unbind (bool Destroy);

  /// Select this OpenGL context for drawing
  void Select ()
  {
    pglMakeCurrent (glAB, hgc, hwndCL);
  }

  /// Allow window to be resized?
  void AllowWindowResize (bool iAllow)
  {
    AllowResize = iAllow;
  }

  /// Show the window
  bool Show (bool Visible);

  /// Disable standard accelerator table for window so we can use ALT, ALT+F4 etc
  bool DisableAccelTable ();

  /// Two virtual functions that performs PM message management for frame and client
  virtual MRESULT ClientMessage (ULONG Message, MPARAM MsgParm1, MPARAM MsgParm2);
  virtual MRESULT FrameMessage (ULONG Message, MPARAM MsgParm1, MPARAM MsgParm2);

  /// Set OpenGL palette for color index modes
  void SetPalette (u_long *palette, int count);

  /// Switch to/from full-screen mode
  bool FullScreen (bool State);

  /// Resize *client* window, compute and set new frame window size
  bool Resize (long Width, long Height, bool Center);

  /// Adjust these variables so that Width/Height == BufferWidth/BufferHeight
  bool AdjustAspectRatio (long *Width, long *Height);

  /// Reset size/position
  bool Reset ();

  /// Pause window: application should check IsPaused() itself
  inline void Pause (bool State)
  {
    fPause = State;
    WinCheckMenuItem (glMN, cmdPause, fPause);
  }

  /// Set mouse cursor visibility when it is over GL window
  void MouseVisible (bool State);

  /// Set mouse cursor shape when it is over GL window
  inline void MouseCursor (ULONG ID)
  { MouseCursorID = ID; }

  /// Flush OpenGL buffer
  void Flush (PRECTL rect)
  {
    ULONG Count;
    DosResetEventSem (sRedrawComplete, &Count);
    csSwapFloatBuffers = true;
    WinInvalidateRect (hwndCL, rect, FALSE);
  }

  /// Wait until flush is complete. Recommended before calling Flush()
  inline void WaitFlush ()
  {
    DosWaitEventSem (sRedrawComplete, 100);
  }

  /// Return active buffer
  inline long ActiveBuff ()
  {
    return ActiveBuffer;
  }

  /// Set window title: This function requires a message queue
  inline bool SetTitle (char *Title)
  {
    return WinSetWindowText (hwndFR, (PSZ) Title);
  }

  /// Send a message to frame window (any of cmdXXX works too)
  inline void Command (ULONG CommandCode)
  {
    WinPostMsg (hwndCL, WM_COMMAND, MPFROMSHORT (CommandCode), MPFROM2SHORT (CMDSRC_OTHER, CommandCode));
  }

  /// Set OpenGL window position
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

  /// Set resize event handler
  inline void SetResizeHandler (tResizeHandler Handler, void *param)
  {
    hResize = Handler;
    paramResize = param;
  }

  /// Return if window is paused
  inline bool isPaused ()
  {
    return fPause;
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

  /// Enable or disable constant aspect ratio of window
  inline void MaintainAspectRatio (bool State)
  {
    if (!fFullScreen)
    {
      fAspect = State;
      WinCheckMenuItem (glMN, cmdToggleAspect, fAspect);
    }
  }

private:
  static MRESULT EXPENTRY ClientHandler (HWND Handle, ULONG Message, MPARAM MsgParm1, MPARAM MsgParm2);
  static MRESULT EXPENTRY FrameHandler (HWND Handle, ULONG Message, MPARAM MsgParm1, MPARAM MsgParm2);
};

#endif // RC_INVOKED

#endif // __LIBGL_H__
