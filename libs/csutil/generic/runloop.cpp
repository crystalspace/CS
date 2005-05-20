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

class csDefaultQuitEventHandler : public iEventHandler
{
private:
  bool shutdown;
public:
  SCF_DECLARE_IBASE;
  csDefaultQuitEventHandler() : shutdown(false) { SCF_CONSTRUCT_IBASE(0); }
  virtual ~csDefaultQuitEventHandler() { SCF_DESTRUCT_IBASE(); }
  bool ShouldShutdown() const { return shutdown; }
  virtual bool HandleEvent(iEvent& e)
  {
    if (e.Type == csevBroadcast && csCommandEventHelper::GetCode(&e) == cscmdQuit)
    {
      shutdown = true;
      return true;
    }
    return false;
  }
};

SCF_IMPLEMENT_IBASE(csDefaultQuitEventHandler)
  SCF_IMPLEMENTS_INTERFACE(iEventHandler)
SCF_IMPLEMENT_IBASE_END

bool csDefaultRunLoop (iObjectRegistry* r)
{
  csRef<iEventQueue> q (CS_QUERY_REGISTRY(r, iEventQueue));
  if (!q)
    return false;
  csRef<iVirtualClock> vc (CS_QUERY_REGISTRY(r, iVirtualClock));

  csDefaultQuitEventHandler eh;
  q->RegisterListener(&eh, CSMASK_Broadcast);

  while (!eh.ShouldShutdown())
  {
    if (vc)
      vc->Advance();
    q->Process();
  }

  q->RemoveListener (&eh);
  return true;
}
