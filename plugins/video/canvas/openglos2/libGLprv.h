/*
    OS/2 OpenGL wrapper class library: Private header file
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

#ifndef __LIBGLPRV_H__
#define __LIBGLPRV_H__

/*----------------------------------------------- PM worker thread interface -*/
enum
{
  pmcmdCreateWindow,                    // Create a window
  pmcmdDestroyWindow,                   // Destroy the window
  pmcmdCreateGLctx,                     // Create GL context
  pmcmdDestroyGLctx,                    // Destroy GL context
  pmcmdBindGLctx,                       // Bind GL ctx to a window
  pmcmdUnbindGLctx,                     // Unbind GL ctx from window
  pmcmdShowWindow,                      // Show/hide GL window
  pmcmdLocateWindow,                    // Set GL window position
  pmcmdResizeWindow,                    // Resize GL window
  pmcmdSelectWindow,			// Select context associated with window
  pmcmdResetWindow			// Reset window size/position
};

enum
{
  pmrcOK,                               // PM thread return code: OK
  pmrcWindowCreationError,              // return code:	Cannot create PM window
  pmrcGLfailure,                        // return code:	OpenGL subsystem failure
  pmrcNotInitialized,                   // return code: Subsystem not initialized
  pmrcBadWindow,                        // return code: Bad window handle
  pmrcNotBound                          // return code: GL context not bound to any window
};

struct PMrq
{
  u_int rc;                             // PM server return code
  union
  {
    struct
    {
      const char *Title;
      HWND Handle;
    } CreateWindow;
    struct
    {
      HWND Handle;
    } DestroyWindow;
    struct
    {
      long Width;
      long Height;
      long ContextFlags;
      glWindow *glW;
    } CreateCtx;
    struct
    {
      glWindow *glW;
    } DestroyCtx;
    struct
    {
      glWindow *glW;
      HWND Handle;
      long DesktopW;
      long DesktopH;
    } BindCtx;
    struct
    {
      glWindow *glW;
      int State;
    } ShowWin;
    struct
    {
      glWindow *glW;
      int Width, Height;
      bool Center;
    } Resize;
    struct
    {
      glWindow *glW;
      int x, y;
    } Locate;
    struct
    {
      glWindow *glW;
    } ResetWin;
  } Parm;
};

// Do a single transaction with PM thread
extern u_int PMcall (long Command, void *rq);

extern HMODULE gdMH;                    // Device driver module (DLL) handle

extern bool gdGLInitialize ();
extern bool gdGLDeinitialize ();

#endif // __LIBGLPRV_H__
