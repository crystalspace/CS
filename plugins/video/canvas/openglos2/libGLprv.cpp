/*
    OS/2 OpenGL wrapper class library: Private PM related functions
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

#define INCL_DOS
#define INCL_WIN
#include <os2.h>
#include <process.h>
#include <signal.h>
#include <stdlib.h>
#include "sysdef.h"
#include "libGL.h"
#include "libGLprv.h"

#define WM_OpenGLCommand	0x9F37	// Choose a unused msg #
#define defWindowStyle		FCF_TITLEBAR|FCF_SYSMENU|FCF_ACCELTABLE

static volatile TID PMtid;              // PM thread ID
static volatile HWND PMmng = NULLHANDLE;// PM request manager window
static glApp *volatile dA;              // PM application object
static volatile HMTX semBusy;           // PM thread requests semaphore
static volatile HEV semSleep;           // PM thread will post this
HMODULE gdMH = NULLHANDLE;              // Module (DLL) handle

static MRESULT EXPENTRY PMmanager (HWND Handle, ULONG Message, MPARAM MsgParm1, MPARAM MsgParm2)
{
  PMrq *rqData = (PMrq *) MsgParm2;
  ULONG rc = pmrcOK;

  switch (Message)
  {
    case WM_OpenGLCommand:
      switch ((ULONG) MsgParm1)
      {
        case pmcmdCreateWindow:
        {
          // Create a OpenGL window
          HWND wnd = dA->CreateWindow (rqData->Parm.CreateWindow.Title, gdMH, idGL, defWindowStyle);

          if (wnd)
            rqData->Parm.CreateWindow.Handle = wnd;
          else
            rc = pmrcWindowCreationError;
          break;
        }
        case pmcmdDestroyWindow:
        {
          WinDestroyWindow (rqData->Parm.DestroyWindow.Handle);
          break;
        }
        case pmcmdCreateGLctx:
        {
          // Create a GL context
          CHK (rqData->Parm.CreateCtx.glW = new glWindow (rqData->Parm.CreateCtx.Width,
            rqData->Parm.CreateCtx.Height, rqData->Parm.CreateCtx.ContextFlags));
          if (!rqData->Parm.CreateCtx.glW)
          {
            rc = pmrcGLfailure;
            break;
          }
          if (rqData->Parm.CreateCtx.glW->lastError != glerOK)
          {
            CHK (delete rqData->Parm.CreateCtx.glW);

            rqData->Parm.CreateCtx.glW = NULL;
            rc = pmrcGLfailure;
            break;
          }
          break;
        }
        case pmcmdDestroyGLctx:
        {
          if (rqData->Parm.DestroyCtx.glW)
            CHKB (delete rqData->Parm.DestroyCtx.glW)
          else
            rc = pmrcNotInitialized;
          break;
        }
        case pmcmdBindGLctx:
        {
          if (!rqData->Parm.BindCtx.glW->Bind (rqData->Parm.BindCtx.Handle))
          {
            rc = pmrcBadWindow;
            break;
          }
          // If we have a *frame* window, we have full control of it
          if (rqData->Parm.BindCtx.glW->hwndFR)
          {
            // Disable GL window accelerator table, if we want to use OS/2 reserved keys
            rqData->Parm.BindCtx.glW->DisableAccelTable ();
            // Center the window
            rqData->Parm.BindCtx.glW->Command (cmdAlignCenter);
          }
          break;
        }
        case pmcmdUnbindGLctx:
        {
          if (!rqData->Parm.BindCtx.glW->Unbind (FALSE))
          {
            rc = pmrcNotBound;
            break;
          }
          break;
        }
        case pmcmdShowWindow:
        {
          rqData->Parm.ShowWin.glW->Show (rqData->Parm.ShowWin.State);
          break;
        }
        case pmcmdLocateWindow:
        {
          rqData->Parm.Locate.glW->SetPos (rqData->Parm.Locate.x, rqData->Parm.Locate.y);
          break;
        }
      }
      if (rqData)
      {
        rqData->rc = rc;
        DosPostEventSem (semSleep);
      }
      return NULL;
    default:
      return WinDefWindowProc (Handle, Message, MsgParm1, MsgParm2);
  }
}

static void PMthread (void *)
{
  CHK (dA = new glApp ());
  if (dA)
  {
    PMmng = dA->CreateWindow (NULL, NULLHANDLE, 0, 0);
    if (PMmng)
    {
      WinSubclassWindow (PMmng, PMmanager);
      dA->Run ();
      PMmng = NULLHANDLE;
    }
    CHK (delete dA);
  }
}

u_int PMcall (long Command, void *rq)
{
  ULONG tmp;
  HWND pmt;
  PMrq *req = (PMrq *) rq;

  if ((pmt = PMmng) == NULL)
    return pmrcNotInitialized;
  DosRequestMutexSem (semBusy, SEM_INDEFINITE_WAIT);
  DosResetEventSem (semSleep, &tmp);
  WinPostMsg (pmt, WM_OpenGLCommand, (MPARAM) Command, (MPARAM) rq);
  DosWaitEventSem (semSleep, SEM_INDEFINITE_WAIT);
  DosReleaseMutexSem (semBusy);
  return req->rc;
}

bool gdGLInitialize ()
{
  if (PMtid)
    return TRUE;
  PMtid = _beginthread (PMthread, NULL, 0x40000, NULL);
  for (int i = 0; i < 100; i++)
  {
    DosSleep (30);
    if (PMmng)
    {
      if (DosCreateMutexSem (NULL, (PHMTX) &semBusy, 0, FALSE))
        return FALSE;
      if (DosCreateEventSem (NULL, (PHEV) &semSleep, 0, FALSE))
        return FALSE;
      return TRUE;
    }
  }
  return FALSE;
}

bool gdGLDeinitialize ()
{
  WinPostQueueMsg (dA->MQ, WM_QUIT, (MPARAM) 0, (MPARAM) 0);
  DosWaitThread ((PTID) &PMtid, DCWW_WAIT);
  DosCloseEventSem (semSleep);
  DosCloseMutexSem (semBusy);
  return TRUE;
}
