/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#include "cssysdef.h"
#include "csutil/sysfunc.h"
#include "csutil/event.h"
#include "csutil/eventnames.h"
#include "csutil/scf_implementation.h"
#include "csutil/cfgacc.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/evdefs.h"
#include "iutil/objreg.h"
#include "iutil/virtclk.h"

/*
 * Implementation of csDefaultRunLoop() for platforms which do not otherwise
 * require any special facilities for creating an application run-loop.  Note
 * that this is only useful for platforms which can perform all user
 * interaction without contact with the host environment beyond that provided
 * by the various plugin modules (usually canvases).  Other platforms which
 * require more in-depth interaction with the host-environment in order to
 * implement an application run-loop will have to provide their own
 * implementation of csDefaultRunLoop() in place of this one.  Examples of
 * environments which can not use this default implementation are Apple's Cocoa
 * frameworks in which the application run-loop is already provided by the
 * NSApplication class, and BeOS's Application Kit in which the application
 * run-loop is already provided by the BApplication class.
 */

class csDefaultQuitEventHandler : 
  public scfImplementation1<csDefaultQuitEventHandler, iEventHandler>
{
private:
  csEventID Quit;
  bool shutdown;
public:
  csDefaultQuitEventHandler(iObjectRegistry *reg) 
    : scfImplementationType (this), shutdown(false) 
  {
    Quit = csevQuit (reg);
  }
  virtual ~csDefaultQuitEventHandler() { }
  bool ShouldShutdown() const { return shutdown; }
  virtual bool HandleEvent(iEvent& e)
  {
    if (e.Name == Quit)
    {
      shutdown = true;
      return true;
    }
    return false;
  }
  CS_EVENTHANDLER_NAMES("crystalspace.runloop")
  CS_EVENTHANDLER_NIL_CONSTRAINTS

  friend bool csDefaultRunLoop (iObjectRegistry *);
};

#include "csutil/custom_new_disable.h"
bool csDefaultRunLoop (iObjectRegistry* r)
{
  csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (r));
  if (!q)
    return false;
  csRef<iVirtualClock> vc (csQueryRegistry<iVirtualClock> (r));

  csRef<csDefaultQuitEventHandler> eh;
  eh.AttachNew (new csDefaultQuitEventHandler (r));
  q->RegisterListener(eh, eh->Quit);

  // Fetch the minimal elapsed ticks per second.
  csConfigAccess cfgacc (r, "/config/system.cfg");
  csTicks min_elapsed = (csTicks) cfgacc->GetInt (
    "System.MinimumElapsedTicks", 0);

  while (!eh->ShouldShutdown())
  {
    if (vc)
      vc->Advance();

    csTicks previous = csGetTicks ();
    q->Process();

    // Limit fps.
    csTicks elapsed = csGetTicks () - previous;
    if (elapsed < min_elapsed)
      csSleep (min_elapsed - elapsed);
  }

  q->RemoveListener (eh);
  return true;
}
#include "csutil/custom_new_enable.h"
