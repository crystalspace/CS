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

#include "cssysdef.h"
#include "csutil/ref.h"
#include "netman.h"
#include "iutil/objreg.h"
#include "iutil/eventq.h"
#include "iutil/event.h"
#include "iutil/evdefs.h"
#include "inetwork/socket2.h"
#include "inetwork/sockerr.h"
#include "csutil/csstring.h"
#include "csutil/hashmap.h"
#include "csutil/csvector.h"
#include "csutil/datastrm.h"

SCF_IMPLEMENT_IBASE (csNetworkManager)
  SCF_IMPLEMENTS_INTERFACE (iNetworkManager)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iEventHandler)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iEventPlug)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csNetworkManager::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csNetworkManager::eiEventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csNetworkManager::eiEventPlug)
  SCF_IMPLEMENTS_INTERFACE (iEventPlug)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csNetworkManager)

SCF_EXPORT_CLASS_TABLE (netman)
  SCF_EXPORT_CLASS (csNetworkManager,
  "crystalspace.network.manager.standard",
  "Crystal Space Standard Network Manager")
SCF_EXPORT_CLASS_TABLE_END

csNetworkManager::csNetworkManager (iBase *parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiEventHandler);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiEventPlug);
}

inline bool csNetworkManager::Initialize (iObjectRegistry *objreg)
{
  eventq = CS_QUERY_REGISTRY (objreg, iEventQueue);
  if (! eventq) return false;
  eventq->RegisterListener (& scfiEventHandler, CSMASK_Nothing);
  eventout = eventq->CreateEventOutlet (& scfiEventPlug);
  return true;
}

inline bool csNetworkManager::HandleEvent (iEvent &ev)
{
  if (ev.Command.Code == cscmdPreProcess)
  {
    int i;

    for ((i = listeners.Length ())--; i >= 0; i--)
    {
      iNetworkSocket2 *sock = (iNetworkSocket2 *) listeners.Get (i);
      iNetworkSocket2 *newsock = sock->Accept ();
      if (newsock) RegisterConnectedSocket
        (newsock, ((iNetworkPacket *) packets.Get ((int) newsock))->New ());

      Poll (sock, ev.Time);
    }

    for ((i = connections.Length ())--; i >= 0; i--)
    {
      iNetworkSocket2 *sock = (iNetworkSocket2 *) connections.Get (i);
      Poll (sock, ev.Time);
    }

    return true;
  }
  return false;
}

inline void csNetworkManager::Poll (iNetworkSocket2 *sock, csTicks t)
{
  iNetworkPacket *packet = (iNetworkPacket *) packets.Get ((int) sock);
  csString *string = (csString *) strings.Get ((int) sock);

  static const size_t hat = 1024;
  char buf [1025];
  int len = sock->Recv (buf, hat);

  if (len > 0)
  {
    string->Append (buf, len);
    csDataStream stream (string->GetData (), string->Length (), false);
    bool post = packet->Read (stream);

    if (post)
    {
      string->DeleteAt (0, stream.GetPosition ());
      stream.SetPosition (0);

      csRef<iEvent> e = eventout->CreateEvent ();
      e->Time = t;
      e->Type = csevNetwork;
      e->Category = 0;
      e->SubCategory = 0;
      e->Network.From = sock;
      e->Network.Data = packet;
      eventout->Post (e);
    }
  }
}

csNetworkManager::~csNetworkManager ()
{
  eventq->RemoveListener (& scfiEventHandler);

  while (connections.Length () > 0)
    UnregisterConnectedSocket ((iNetworkSocket2 *) connections.Get (0));
  while (listeners.Length () > 0)
    UnregisterListeningSocket ((iNetworkSocket2 *) listeners.Get (0));
}

void csNetworkManager::RegisterConnectedSocket (iNetworkSocket2 *sock, iNetworkPacket *packet)
{
  connections.Push (sock);
  packet->IncRef ();
  packets.Put ((int) sock, packet);
  csString *str = new csString ();
  strings.Put ((int) sock, str);
}

bool csNetworkManager::UnregisterConnectedSocket (iNetworkSocket2 *sock)
{
  if (connections.Delete (sock))
  {
    ((iNetworkPacket *) packets.Get ((int) sock))->DecRef ();
    packets.DeleteAll ((int) sock);
    delete (csString *) strings.Get ((int) sock);
    strings.DeleteAll ((int) sock);

    return true;
  }
  else return false;
}

void csNetworkManager::RegisterListeningSocket (iNetworkSocket2 *sock, iNetworkPacket *packet)
{
  listeners.Push (sock);
  packet->IncRef ();
  packets.Put ((int) sock, packet);
  csString *str = new csString ();
  strings.Put ((int) sock, str);
}

bool csNetworkManager::UnregisterListeningSocket (iNetworkSocket2 *sock)
{
  if (listeners.Delete (sock))
  {
    ((iNetworkPacket *) packets.Get ((int) sock))->DecRef ();
    packets.DeleteAll ((int) sock);
    delete (csString *) strings.Get ((int) sock);
    strings.DeleteAll ((int) sock);

    return true;
  }
  else return false;
}

bool csNetworkManager::Send (iNetworkSocket2 *sock, iNetworkPacket *packet)
{
  size_t size;
  char *data = packet->Write (size);
  return sock->Send (data, size);
}

bool csNetworkManager::SendToAll (iNetworkPacket *packet)
{
  size_t size;
  char *data = packet->Write (size);
  bool ok = true;
  int i;
  for ((i = connections.Length ())--; i >= 0; i--)
  {
    iNetworkSocket2 *sock = (iNetworkSocket2 *) connections.Get (i);
    if (packet->FilterSocket (sock))
      if (! sock->Send (data, size)) ok = false;
  }
  return ok;
}

