/*
    Copyright (C) 1998 by Jorrit Tyberghein

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
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "sysdef.h"
#include "csutil/inifile.h"
#include "cssys/unix/unix.h"
#include "cssys/common/system.h"
#include "igraph3d.h"

/*
 * Signal handler to clean up and give some
 * final debugging information.
 */
extern void debug_dump ();
extern void cleanup ();
void handler (int sig)
{
  static bool in_exit = false;
  if (in_exit)
    exit (1);
  in_exit = true;

  int err = errno;
#if defined (__USE_GNU)
  fprintf (stderr, "\n%s signal caught; last error: %s\n",
    strsignal (sig), strerror (err));
#elif defined (OS_LINUX) || defined (OS_FREEBSD)
  fprintf (stderr, "\n%s signal caught; last error: %s\n",
    sys_siglist [sig], strerror (err));
#else
  fprintf (stderr, "\nSignal %d caught; last error: %s\n",
    sig, strerror (err));
#endif

  if (sig != SIGINT)
    debug_dump ();

  SysSystemDriver::Shutdown = true;
  cleanup ();
  exit (1);
}

void init_sig ()
{
  signal (SIGHUP, handler);
  signal (SIGINT, handler);
  signal (SIGTRAP, handler);
  signal (SIGABRT, handler);
  signal (SIGALRM, handler);
  signal (SIGTERM, handler);
  signal (SIGPIPE, handler);
  signal (SIGSEGV, handler);
  signal (SIGBUS, handler);
  signal (SIGFPE, handler);
  signal (SIGILL, handler);
}

//------------------------------------------------------ The System driver ---//

BEGIN_INTERFACE_TABLE (SysSystemDriver)
  IMPLEMENTS_COMPOSITE_INTERFACE (System)
  IMPLEMENTS_COMPOSITE_INTERFACE (UnixSystemDriver)
END_INTERFACE_TABLE ()

IMPLEMENT_UNKNOWN_NODELETE (SysSystemDriver)

SysSystemDriver::SysSystemDriver () : csSystemDriver (), Callback (NULL)
{
  // Initialize signal handler for clean shutdown
  init_sig ();
}

void SysSystemDriver::SetSystemDefaults ()
{
  csSystemDriver::SetSystemDefaults ();
  SimDepth = 0;
  UseSHM = true;
  HardwareCursor = true;
  if (config)
  {
    SimDepth = config->GetYesNo ("VideoDriver", "SIMULATE_DEPTH", SimDepth);
    if (config)
      UseSHM = config->GetYesNo ("VideoDriver", "XSHM", UseSHM);
    if (config)
      HardwareCursor = config->GetYesNo ("VideoDriver", "SYS_MOUSE_CURSOR", HardwareCursor);
  }
}

bool SysSystemDriver::ParseArg (int argc, char* argv[], int &i)
{
  if (strcasecmp ("-shm", argv[i]) == 0)
    UseSHM = true;
  else if (strcasecmp ("-noshm", argv[i]) == 0)
    UseSHM = false;
  else if (strcasecmp ("-sdepth", argv[i]) == 0)
  {
    i++;
    sscanf (argv[i], "%d", &SimDepth);
    if (SimDepth != 8 && SimDepth != 15 && SimDepth != 16 && SimDepth != 32)
    {
      Printf (MSG_FATAL_ERROR, "Crystal Space can't run in this simulated depth! (use 8, 15, 16, or 32)!\n");
      exit (0);
    }
  }
  else
    return csSystemDriver::ParseArg (argc, argv, i);
  return true;
}

void SysSystemDriver::Help ()
{
  csSystemDriver::Help ();
  Printf (MSG_STDOUT, "  -sdepth <depth>    set simulated depth (8, 15, 16, or 32) (default=none)\n");
  Printf (MSG_STDOUT, "  -shm/noshm         SHM extension (default '%sshm')\n",
    UseSHM ? "" : "no");
}

void SysSystemDriver::FocusHandler (int Enable)
{
  if (EventQueue)
    do_focus (Enable);
}

void SysSystemDriver::Loop(void)
{
  while (!Shutdown && !ExitLoop)
  {
    static long prev_time = -1;
    long new_prev_time = Time ();
    NextFrame ((prev_time == -1) ? 0 : new_prev_time - prev_time, Time ());
    prev_time = new_prev_time;
    if (Callback)
      Callback (CallbackParam);
  }
}

void SysSystemDriver::Sleep (int SleepTime)
{
  usleep (SleepTime * 1000);
}

//------------------------------------------------------ XUnixSystemDriver ---//

IMPLEMENT_COMPOSITE_UNKNOWN_AS_EMBEDDED (SysSystemDriver, UnixSystemDriver)

STDMETHODIMP SysSystemDriver::XUnixSystemDriver::GetSettings (int &SimDepth,
  bool &UseSHM, bool &HardwareCursor)
{
  METHOD_PROLOGUE (SysSystemDriver, UnixSystemDriver)
  SimDepth = pThis->SimDepth;
  UseSHM = pThis->UseSHM;
  HardwareCursor = pThis->HardwareCursor;
  return S_OK;
}

STDMETHODIMP SysSystemDriver::XUnixSystemDriver::SetLoopCallback
  (LoopCallback Callback, void *Param)
{
  METHOD_PROLOGUE(SysSystemDriver, UnixSystemDriver)
  pThis->Callback = Callback;
  pThis->CallbackParam = Param;
  return S_OK;
}

STDMETHODIMP SysSystemDriver::XUnixSystemDriver::KeyboardEvent
  (int Key, bool Down)
{
  METHOD_PROLOGUE (SysSystemDriver, UnixSystemDriver)
  ((SysKeyboardDriver *)pThis->Keyboard)->Handler (Key, Down);
  return S_OK;
}

STDMETHODIMP SysSystemDriver::XUnixSystemDriver::MouseEvent
  (int Button, int Down, int x, int y, int ShiftFlags)
{
  METHOD_PROLOGUE (SysSystemDriver, UnixSystemDriver)
  ((SysMouseDriver *)pThis->Mouse)->Handler (Button, Down, x, y, ShiftFlags);
  return S_OK;
}

STDMETHODIMP SysSystemDriver::XUnixSystemDriver::FocusEvent (int Enable)
{
  METHOD_PROLOGUE(SysSystemDriver, UnixSystemDriver)
  pThis->FocusHandler (Enable);
  return S_OK;
}

//------------------------------------------------------ SysKeyboardDriver ---//

SysKeyboardDriver::SysKeyboardDriver() : csKeyboardDriver ()
{
  // Nothing to do
}

void SysKeyboardDriver::Handler(int Key, bool Down)
{
  if (!Ready ())
    return;

  if (Key)
  {
    if (Down)
      do_keypress (System->Time (), Key);
    else
      do_keyrelease (System->Time (), Key);
  } /* endif */
}

//--------------------------------------------------------- SysMouseDriver ---//

// Mouse fonctions
SysMouseDriver::SysMouseDriver() : csMouseDriver ()
{
  // Nothing to do
}

void SysMouseDriver::Handler (int Button, int Down, int x, int y,
  int ShiftFlags)
{
  if (!Ready ())
    return;

  if (Button == 0)
    do_mousemotion (::System->Time (), x, y);
  else if (Down)
    do_buttonpress (::System->Time (), Button, x, y, ShiftFlags & CSMASK_SHIFT,
      ShiftFlags & CSMASK_ALT, ShiftFlags & CSMASK_CTRL);
  else
    do_buttonrelease (::System->Time (), Button, x, y);
}

