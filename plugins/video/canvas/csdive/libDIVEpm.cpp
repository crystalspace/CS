/*
    OS/2 DIVE class library: PM related functions
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

#define INCL_DOS
#define INCL_WIN
#include <os2.h>
#include <process.h>
#include <signal.h>
#include "cssysdef.h"
#include "libDIVE.h"
#include "libDIVEprv.h"

#define WM_DiveCommand	0x9F37		// Choose a unused msg #
#define defWindowStyle  FCF_TITLEBAR | FCF_SYSMENU | FCF_ACCELTABLE

static volatile TID PMtid = 0;          // PM thread ID
static volatile HWND PMmng = NULLHANDLE;// PM request manager window
static diveApp *volatile dA;            // PM application object
static volatile HMTX semBusy;           // PM thread requests semaphore
static volatile HEV semSleep;           // PM thread will post this
FGVideoMode *vmList;                    // Pointer to an array of FGVideoMode
u_int vmCount;                          // Number of videomodes in vmList
long DesktopW, DesktopH;                // Desktop width x height
HMODULE gdMH = NULLHANDLE;              // Module (DLL) handle

static MRESULT EXPENTRY PMmanager (HWND Handle, ULONG Message, MPARAM MsgParm1, MPARAM MsgParm2)
{
  PMrq *rqData = (PMrq *) MsgParm2;
  ULONG rc = pmrcOK;

  switch (Message)
  {
    case WM_DiveCommand:
      switch ((ULONG) MsgParm1)
      {
        case pmcmdCreateWindow:
        {
          // Create a DIVE window
          HWND wnd = dA->CreateWindow ((PSZ) rqData->Parm.CreateWindow.Title, gdMH, idDive, defWindowStyle);

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
        case pmcmdCreateDIVEctx:
        {
          // Create a DIVE context
          rqData->Parm.CreateCtx.dW = new diveWindow (rqData->Parm.CreateCtx.Mode->Width,
            rqData->Parm.CreateCtx.Mode->Height, (FOURCC) rqData->Parm.CreateCtx.Mode->PixelFormat,
            rqData->Parm.CreateCtx.Mode->Buffers);
          if (!rqData->Parm.CreateCtx.dW)
          {
            rc = pmrcDIVEfailure;
            break;
          }
          if (rqData->Parm.CreateCtx.dW->lastError != derrOK)
          {
            delete rqData->Parm.CreateCtx.dW;

            rqData->Parm.CreateCtx.dW = NULL;
            rc = pmrcDIVEfailure;
            break;
          }
          break;
        }
        case pmcmdDestroyDIVEctx:
        {
          if (rqData->Parm.DestroyCtx.dW)
            delete rqData->Parm.DestroyCtx.dW;
          else
            rc = pmrcNotInitialized;
          break;
        }
        case pmcmdBindDIVEctx:
        {
          int i, Scale, W, H;

          if (!rqData->Parm.BindCtx.dW->Bind (rqData->Parm.BindCtx.Handle))
          {
            rc = pmrcBadWindow;
            break;
          }
          // If we have a *frame* window, we have full control of it
          if (rqData->Parm.BindCtx.dW->diveFR)
          {
            // Disable DIVE window accelerator table, if we want to use OS/2 reserved keys
            rqData->Parm.BindCtx.dW->DisableAccelTable ();

            // Compute optimal window scale
            Scale = rqData->Parm.BindCtx.DesktopW / rqData->Parm.BindCtx.dW->BufferWidth ();
            i = rqData->Parm.BindCtx.DesktopH / rqData->Parm.BindCtx.dW->BufferHeight ();
            if (Scale > i)
              Scale = i;
            if (Scale == 0)
            {
              W = rqData->Parm.BindCtx.DesktopW;
              H = rqData->Parm.BindCtx.DesktopH;
            } else
            {
              W = rqData->Parm.BindCtx.dW->BufferWidth () * Scale;
              H = rqData->Parm.BindCtx.dW->BufferHeight () * Scale;
            }
            rqData->Parm.BindCtx.dW->Resize (W, H, true);
            if ((W == rqData->Parm.BindCtx.DesktopW) && (H == rqData->Parm.BindCtx.DesktopH))
              rqData->Parm.BindCtx.dW->FullScreen (true);
          }
          break;
        }
        case pmcmdUnbindDIVEctx:
        {
          if (!rqData->Parm.BindCtx.dW->Unbind (false))
          {
            rc = pmrcNotBound;
            break;
          }
          break;
        }
        case pmcmdShowWindow:
        {
          rqData->Parm.ShowWin.dW->Show (rqData->Parm.ShowWin.State);
          break;
        }
        case pmcmdResizeWindow:
        {
          rqData->Parm.Resize.dW->Resize (rqData->Parm.Resize.Width,
            rqData->Parm.Resize.Height, rqData->Parm.Resize.Center);
          break;
        }
        case pmcmdLocateWindow:
        {
          rqData->Parm.Locate.dW->SetPos (rqData->Parm.Locate.x, rqData->Parm.Locate.y);
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
//  DosSetPriority (PRTYS_THREAD, PRTYC_REGULAR, PRTYD_MAXIMUM, 0);
  dA = new diveApp ();
  if (dA)
  {
    PMmng = dA->CreateWindow (NULL, NULLHANDLE, 0, 0);
    if (PMmng)
    {
      WinSubclassWindow (PMmng, PMmanager);
      dA->Run ();
      PMmng = NULLHANDLE;
    }
    delete dA;
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
  WinPostMsg (pmt, WM_DiveCommand, (MPARAM) Command, (MPARAM) rq);
  DosWaitEventSem (semSleep, SEM_INDEFINITE_WAIT);
  DosReleaseMutexSem (semBusy);
  return req->rc;
}

static bool QueryDIVE ()
{
  // Query DIVE capabilities and record supported resolutions & color depths
  DIVE_CAPS DiveCaps;

  DiveCaps.pFormatData = NULL;
  DiveCaps.ulFormatLength = 0;
  DiveCaps.ulStructLen = sizeof (DIVE_CAPS);

  if (DiveQueryCaps (&DiveCaps, DIVE_BUFFER_SCREEN) != DIVE_ERR_INSUFFICIENT_LENGTH)
    return false;
  DiveCaps.pFormatData = malloc (DiveCaps.ulFormatLength);
  if (DiveQueryCaps (&DiveCaps, DIVE_BUFFER_SCREEN) != DIVE_SUCCESS)
    return false;

  DesktopW = DiveCaps.ulHorizontalResolution;
  DesktopH = DiveCaps.ulVerticalResolution;
  bool native;
  for (u_int t = 0; t < 2; t++)
  {
    if (t > 0)
      vmList = (FGVideoMode *) malloc (vmCount * sizeof (FGVideoMode));
    vmCount = 0;
    native = false;
    for (u_int i = 0; i < DiveCaps.ulInputFormats; i++)
    {
      FOURCC fcc = ((FOURCC *) DiveCaps.pFormatData)[i];
      if (t == 1)
      {
        FGVideoMode *Mode = &vmList [vmCount];

        Mode->Width = DesktopW;
        Mode->Height = DesktopH;
        Mode->PixelFormat = fcc;
        Mode->Buffers = DIVE_MAXBUFFERS;
        Mode->Flags = vmfWindowed | vmfHardware2D | (DiveCaps.fccColorEncoding == fcc ? vmfNative : 0);
        Mode->IndexBits = 0;
      }
      if (DiveCaps.fccColorEncoding == fcc)
        native = true;
      vmCount++;
    }

    // If native color format is not in supported list, add it anyway
    if (!native)
    {
      if (t == 1)
      {
        FGVideoMode *Mode = &vmList [vmCount];

        Mode->Width = DesktopW;
        Mode->Height = DesktopH;
        Mode->PixelFormat = DiveCaps.fccColorEncoding;
        Mode->Buffers = DIVE_MAXBUFFERS;
        Mode->Flags = vmfWindowed | vmfHardware2D | vmfNative;
        Mode->IndexBits = 0;
      }
      vmCount++;
    }
  }
  free (DiveCaps.pFormatData);
  return true;
}

bool gdDiveInitialize ()
{
  if (PMtid)
    return true;
  if (!QueryDIVE ())
    return false;
  PMtid = _beginthread (PMthread, NULL, 0x8000, NULL);
  int i;
  for (i = 0; i < 100; i++)
  {
    DosSleep (30);
    if (PMmng)
    {
      if (DosCreateMutexSem (NULL, (PHMTX) &semBusy, 0, FALSE))
        return false;
      if (DosCreateEventSem (NULL, (PHEV) &semSleep, 0, FALSE))
        return false;
      return true;
    }
  }
  return false;
}

bool gdDiveDeinitialize ()
{
  if (!PMtid)
    return true;
  WinPostQueueMsg (dA->MQ, WM_QUIT, (MPARAM) 0, (MPARAM) 0);
  DosWaitThread ((PTID) &PMtid, DCWW_WAIT);
  DosCloseEventSem (semSleep);
  DosCloseMutexSem (semBusy);
  PMtid = 0;
  return true;
}
