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
#include "netman.h"
#include "iutil/objreg.h"
#include "iutil/eventq.h"
#include "iutil/event.h"
#include "iutil/evdefs.h"
#include "inetwork/driver.h"
#include "inetwork/socket2.h"
#include "inetwork/sockerr.h"
#include "csutil/csstring.h"
#include "csutil/hashmap.h"
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

    for ((i = connections.Length ())--; i >= 0; i--)
    {
      iNetworkConnection *conn = connections.Get (i);

      if (conn->IsConnected ())
        Poll (conn, ev.Time);
      else
        UnregisterEndPoint (conn);
    }

    for ((i = listeners.Length ())--; i >= 0; i--)
    {
      iNetworkListener *listener = listeners.Get (i);
      csRef<iNetworkConnection> newconn = listener->Accept ();
      if (newconn)
      {
        csRef<iNetworkPacket> newpkt =
          ((iNetworkPacket *) packets.Get ((int) listener))->New ();
        RegisterConnection (newconn, newpkt);
        Poll (newconn, ev.Time);
      }
    }

    for ((i = enconnections.Length ())--; i >= 0; i--)
    {
      iNetworkSocket2 *sock = enconnections.Get (i);

      if (sock->IsConnected ())
        Poll (sock, ev.Time);
      else
        UnregisterConnectedSocket (sock);
    }

    for ((i = enlisteners.Length ())--; i >= 0; i--)
    {
      iNetworkSocket2 *sock = enlisteners.Get (i);
      iNetworkSocket2 *newsock = sock->Accept ();
      if (newsock)
      {
        csRef<iNetworkPacket2> newpkt =
          ((iNetworkPacket2 *) enpackets.Get ((int) sock))->New ();
        RegisterConnectedSocket (newsock, newpkt);
      }

      Poll (sock, ev.Time);
    }

    return true;
  }
  return false;
}

void csNetworkManager::Poll (iNetworkConnection *conn, csTicks t)
{
  if (! conn->IsDataWaiting ()) return;

  iNetworkPacket *packet = (iNetworkPacket *) packets.Get ((int) conn);
  csString *string = (csString *) strings.Get ((int) conn);

  static const size_t hat = 1024;
  size_t len = string->Length ();
  string->PadRight (len + hat);
  len += conn->Receive (string->GetData (), hat);
  string->Truncate (len);

  if (len > 0)
  {
    csDataStream stream (string->GetData (), string->Length (), false);
    bool post = packet->Read (stream, conn);

    if (post)
    {
      string->DeleteAt (0, stream.GetPosition ());
      stream.SetPosition (0);

      csRef<iEvent> e = eventout->CreateEvent ();
      e->Type = csevNetwork;
      e->Network.From = conn;
      e->Network.Data = packet;
      eventout->Post (e);
    }
  }
}

void csNetworkManager::Poll (iNetworkSocket2 *sock, csTicks t)
{
  iNetworkPacket2 *packet = (iNetworkPacket2 *) enpackets.Get ((int) sock);
  csString *string = (csString *) enstrings.Get ((int) sock);

  static const size_t hat = 1024;
  size_t oldlen = string->Length ();
  string->PadRight (oldlen + hat);
  size_t len = sock->Recv (string->GetData (), hat);
  string->Truncate (oldlen + len);

  if (len > 0)
  {
    csDataStream stream (string->GetData (), string->Length (), false);
    bool post = packet->Read (stream, sock);

    if (post)
    {
      string->DeleteAt (0, stream.GetPosition ());
      stream.SetPosition (0);

      csRef<iEvent> e = eventout->CreateEvent ();
      e->Time = t;
      e->Type = csevNetwork;
      e->Network.From2 = sock;
      e->Network.Data2 = packet;
      eventout->Post (e);
    }
  }
}

csNetworkManager::~csNetworkManager ()
{
  eventq->RemoveListener (& scfiEventHandler);

  while (connections.Length () > 0)
    UnregisterEndPoint (connections.Get (0));
  while (listeners.Length () > 0)
    UnregisterEndPoint (listeners.Get (0));

  while (enconnections.Length () > 0)
    UnregisterConnectedSocket (enconnections.Get (0));
  while (enlisteners.Length () > 0)
    UnregisterListeningSocket (enlisteners.Get (0));

  SCF_DESTRUCT_EMBEDDED_IBASE (scfiEventPlug);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiEventHandler);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE();
}

void csNetworkManager::RegisterConnection (iNetworkConnection *conn, iNetworkPacket *packet)
{
  connections.Push (conn);
  packet->IncRef ();
  packets.Put ((int) conn, packet);
  csString *str = new csString ();
  strings.Put ((int) conn, str);
}

void csNetworkManager::RegisterConnectedSocket (iNetworkSocket2 *sock, iNetworkPacket2 *packet)
{
  enconnections.Push (sock);
  packet->IncRef ();
  enpackets.Put ((int) sock, packet);
  csString *str = new csString ();
  enstrings.Put ((int) sock, str);
}

bool csNetworkManager::UnregisterConnectedSocket (iNetworkSocket2 *sock)
{
  if (enconnections.Delete (sock))
  {
    ((iNetworkPacket2 *) enpackets.Get ((int) sock))->DecRef ();
    enpackets.DeleteAll ((int) sock);
    delete (csString *) enstrings.Get ((int) sock);
    enstrings.DeleteAll ((int) sock);

    return true;
  }
  else return false;
}

void csNetworkManager::RegisterListener (iNetworkListener *listener, iNetworkPacket *packet)
{
  listeners.Push (listener);
  packet->IncRef ();
  packets.Put ((int) listener, packet);
}

void csNetworkManager::RegisterListeningSocket (iNetworkSocket2 *sock, iNetworkPacket2 *packet)
{
  enlisteners.Push (sock);
  packet->IncRef ();
  enpackets.Put ((int) sock, packet);
  csString *str = new csString ();
  enstrings.Put ((int) sock, str);
}

bool csNetworkManager::UnregisterListeningSocket (iNetworkSocket2 *sock)
{
  if (enlisteners.Delete (sock))
  {
    ((iNetworkPacket *) enpackets.Get ((int) sock))->DecRef ();
    enpackets.DeleteAll ((int) sock);
    delete (csString *) enstrings.Get ((int) sock);
    enstrings.DeleteAll ((int) sock);

    return true;
  }
  else return false;
}

bool csNetworkManager::UnregisterEndPoint (iNetworkEndPoint *ep)
{
  iNetworkListener *nl = csRef<iNetworkListener>
    (SCF_QUERY_INTERFACE (ep, iNetworkListener));
  iNetworkConnection *nc = csRef<iNetworkConnection>
    (SCF_QUERY_INTERFACE (ep, iNetworkConnection));

  if (nl ? listeners.Delete (nl) : nc ? connections.Delete (nc) : false)
  {
    ((iNetworkPacket *) packets.Get ((int) ep))->DecRef ();
    packets.DeleteAll ((int) ep);
    delete (csString *) strings.Get ((int) ep);
    strings.DeleteAll ((int) ep);

    return true;
  }
  else return false;
}

bool csNetworkManager::Send (iNetworkConnection *conn, iNetworkPacket *packet)
{
  size_t size;
  char *data = packet->Write (size);
  return conn->Send (data, size);
}

bool csNetworkManager::Send (iNetworkSocket2 *sock, iNetworkPacket2 *packet)
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
    iNetworkConnection *conn = connections.Get (i);
    if (packet->FilterSocket (conn))
      if (! conn->Send (data, size)) ok = false;
  }
  return ok;
}

bool csNetworkManager::SendToAll (iNetworkPacket2 *packet)
{
  size_t size;
  char *data = packet->Write (size);
  bool ok = true;
  int i;
  for ((i = enconnections.Length ())--; i >= 0; i--)
  {
    iNetworkSocket2 *sock = enconnections.Get (i);
    if (packet->FilterSocket (sock))
      if (! sock->Send (data, size)) ok = false;
  }
  return ok;
}

