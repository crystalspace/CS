/*
    OS/2 DIVE class library: Private header file
    Copyright (C) 1997 by FRIENDS software
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

#ifndef __LIBDIVEPRV_H__
#define __LIBDIVEPRV_H__

/*----------------------------------------------- PM worker thread interface -*/
enum
{
  pmcmdCreateWindow,                    // PM thread command: Create a window
  pmcmdDestroyWindow,                   // PM thread command: Destroy the window
  pmcmdCreateDIVEctx,                   // PM thread command: Create DIVE context
  pmcmdDestroyDIVEctx,                  // PM thread command: Destroy DIVE context
  pmcmdBindDIVEctx,                     // PM thread command: Bind DIVE ctx to a window
  pmcmdUnbindDIVEctx,                   // PM thread command: Unbind DIVE ctx from window
  pmcmdShowWindow,                      // PM thread command: Show/hide DIVE window
  pmcmdResizeWindow,                    // PM thread command: Resize DIVE window
  pmcmdLocateWindow                     // PM thread command: Set DIVE window position
};

enum
{
  pmrcOK,                               // PM thread return code: OK
  pmrcWindowCreationError,              // return code:	Cannot create PM window
  pmrcDIVEfailure,                      // return code:	DIVE subsystem failure
  pmrcNotInitialized,                   // return code: Subsystem not initialized
  pmrcBadWindow,                        // return code: Bad window handle
  pmrcNotBound                          // return code: DIVE context not bound to any window
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
      FGVideoMode *Mode;
      diveWindow *dW;
    } CreateCtx;
    struct
    {
      diveWindow *dW;
    } DestroyCtx;
    struct
    {
      diveWindow *dW;
      HWND Handle;
      long DesktopW;
      long DesktopH;
    } BindCtx;
    struct
    {
      diveWindow *dW;
      int State;
    } ShowWin;
    struct
    {
      diveWindow *dW;
      int Width, Height;
      bool Center;
    } Resize;
    struct
    {
      diveWindow *dW;
      int x, y;
    } Locate;
  } Parm;
};

// Do a single transaction with PM thread
extern u_int PMcall (long Command, void *rq);

extern HMODULE gdMH;                    // Device driver module (DLL) handle

extern bool gdDiveInitialize ();
extern bool gdDiveDeinitialize ();

#endif // __LIBDIVEPRV_H__
