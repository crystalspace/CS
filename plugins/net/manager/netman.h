/*
    Copyright (C) 2002 by Mat Sutcliffe <oktal@gmx.co.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
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

#include "csutil/ref.h"
#include "csutil/refarr.h"
#include "inetwork/netman.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/event.h"
#include "csutil/hashmap.h"
#include "csutil/csvector.h"

struct iEventQueue;
struct iObjectRegistry;
struct iEvent;

class csNetworkManager : public iNetworkManager
{
 private:
  csRefArray<iNetworkConnection> connections;
  csRefArray<iNetworkListener> listeners;
  csHashMap packets, strings;

  csRefArray<iNetworkSocket2> enconnections, enlisteners;
  csHashMap enpackets, enstrings;

  csRef<iEventQueue> eventq;
  csRef<iEventOutlet> eventout;

  void Poll (iNetworkConnection *, csTicks);
  void Poll (iNetworkSocket2 *, csTicks);

 protected:
  bool Initialize (iObjectRegistry *);
  bool HandleEvent (iEvent &);

 public:
  SCF_DECLARE_IBASE;

  csNetworkManager (iBase *);
  virtual ~csNetworkManager ();

  virtual void RegisterConnection (iNetworkConnection *, iNetworkPacket *);
  virtual void RegisterListener (iNetworkListener *, iNetworkPacket *);
  virtual bool UnregisterEndPoint (iNetworkEndPoint *);

  virtual bool Send (iNetworkConnection *, iNetworkPacket *);
  virtual bool SendToAll (iNetworkPacket *);

  virtual void RegisterConnectedSocket (iNetworkSocket2 *, iNetworkPacket2 *);
  virtual bool UnregisterConnectedSocket (iNetworkSocket2 *);
  virtual void RegisterListeningSocket (iNetworkSocket2 *, iNetworkPacket2 *);
  virtual bool UnregisterListeningSocket (iNetworkSocket2 *);

  virtual bool Send (iNetworkSocket2 *, iNetworkPacket2 *);
  virtual bool SendToAll (iNetworkPacket2 *);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csNetworkManager);
    virtual bool Initialize (iObjectRegistry* r)
    {
      return scfParent->Initialize(r);
    }
  } scfiComponent;
  friend struct eiComponent;

  struct eiEventHandler : public iEventHandler
  {
    SCF_DECLARE_EMBEDDED_IBASE (csNetworkManager);
    virtual bool HandleEvent (iEvent &ev)
      { return scfParent->HandleEvent (ev); }
  } scfiEventHandler;
  friend struct eiEventHandler;

  struct eiEventPlug : public iEventPlug
  {
    SCF_DECLARE_EMBEDDED_IBASE (csNetworkManager);
    virtual unsigned GetPotentiallyConflictingEvents () { return 0; }
    virtual unsigned QueryEventPriority (unsigned i) { return 0; }
  } scfiEventPlug;
  friend struct eiEventPlug;
};

