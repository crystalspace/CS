/*
    Copyright (C) 1999,2000 by Eric Sunshine <sunshine@sunshineco.com>
    Written by Eric Sunshine <sunshine@sunshineco.com>

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

#define CS_SYSDEF_PROVIDE_SOCKETS
#include "cssysdef.h"
#include "socket.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "csutil/util.h"

#define CS_NET_LISTEN_QUEUE_SIZE 5

CS_IMPLEMENT_PLATFORM_PLUGIN

SCF_IMPLEMENT_FACTORY(csSocketDriver)

SCF_EXPORT_CLASS_TABLE(cssocket)
  SCF_EXPORT_CLASS(csSocketDriver, "crystalspace.network.driver.sockets",
    "Crystal Space BSD Sockets Network Driver")
SCF_EXPORT_CLASS_TABLE_END

SCF_IMPLEMENT_IBASE(csSocketConnection)
  SCF_IMPLEMENTS_INTERFACE(iNetworkConnection)
  SCF_IMPLEMENTS_INTERFACE(iNetworkEndPoint)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iNetworkSocket)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE(csSocketConnection::csSocket)
  SCF_IMPLEMENTS_INTERFACE(iNetworkSocket)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE(csSocketListener)
  SCF_IMPLEMENTS_INTERFACE(iNetworkListener)
  SCF_IMPLEMENTS_INTERFACE(iNetworkEndPoint)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iNetworkSocket)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE(csSocketListener::csSocket)
  SCF_IMPLEMENTS_INTERFACE(iNetworkSocket)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE(csSocketDriver)
  SCF_IMPLEMENTS_INTERFACE(iNetworkDriver)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iEventHandler)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSocketDriver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSocketDriver::eiEventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_EMBEDDED_IBASE_END


// csSocketEndPoint -----------------------------------------------------------

csSocketEndPoint::~csSocketEndPoint() { CloseSocket(); }
void csSocketEndPoint::Terminate()    { CloseSocket(); }
void csSocketEndPoint::ClearError()   { LastError = CS_NET_ERR_NO_ERROR; }

csSocketEndPoint::csSocketEndPoint(csNetworkSocket s, bool blocks) :
  Socket(s), LastError(CS_NET_ERR_NO_ERROR)
{
  if (!PlatformSetBlocking(blocks))
  {
    LastError = CS_NET_ERR_CANNOT_SET_BLOCKING_MODE;
    CloseSocket();
  }
}

void csSocketEndPoint::CloseSocket()
{
  ClearError();
  if (Socket != CS_NET_SOCKET_INVALID)
  {
      CS_CLOSESOCKET(Socket);
      Socket = CS_NET_SOCKET_INVALID;
  }
}

bool csSocketEndPoint::ValidateSocket()
{
  ClearError();
  const bool ok = (Socket != CS_NET_SOCKET_INVALID);
  if (!ok)
    LastError = CS_NET_ERR_INVALID_SOCKET;
  return ok;
}

#if !defined(OS_BE)

bool csSocketEndPoint::PlatformSetBlocking(bool blocks)
{
  unsigned long flag = (blocks ? 0 : 1);
  return (CS_IOCTLSOCKET(Socket, FIONBIO, &flag) == 0);
}

#else

bool csSocketEndPoint::PlatformSetBlocking(bool blocks)
{
  const unsigned char flag = (blocks ? 0x00 : 0xff);
  return (setsockopt(Socket, SOL_SOCKET, SO_NONBLOCK, &flag,
    sizeof(flag)) >= 0);
}

#endif

// csSocketConnection ---------------------------------------------------------

csSocketConnection::csSocketConnection(
  iBase* p, csNetworkSocket s, bool blocking) : csSocketEndPoint(s, blocking)
{
  SCF_CONSTRUCT_IBASE(p);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiNetworkSocket);
}

bool csSocketConnection::Send(const void* data, size_t nbytes)
{
  bool ok = false;
  if (ValidateSocket())
  {
    if (send(Socket, (char*)data, nbytes, 0) != -1)
      ok = true;
    else
      LastError = CS_NET_ERR_CANNOT_SEND;
  }
  return ok;
}

size_t csSocketConnection::Receive(void* buff, size_t maxbytes)
{
  size_t received = 0;
  if (ValidateSocket())
  {
    received = recv(Socket, (char*)buff, maxbytes, 0);
    if (received == (size_t)-1)
    {
      received = 0;
      if (CS_GETSOCKETERROR != EWOULDBLOCK)
        LastError = CS_NET_ERR_CANNOT_RECEIVE;
    }
  }
  return received;
}

csNetworkSocket csSocketConnection::csSocket::GetSocket() const
{ return scfParent->GetSocket(); }


// csSocketListener -----------------------------------------------------------

csSocketListener::csSocketListener(iBase* p, csNetworkSocket s,
  unsigned short port, bool blockingListener, bool blockingConnection) :
  csSocketEndPoint(s, blockingListener), BlockingConnection(blockingConnection)
{
  SCF_CONSTRUCT_IBASE(p);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiNetworkSocket);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);

  bool ok = false;
  if (bind(Socket, (struct sockaddr*)&addr, sizeof(addr)) == -1)
    LastError = CS_NET_ERR_CANNOT_BIND;
  else if (listen(Socket, CS_NET_LISTEN_QUEUE_SIZE) == -1)
    LastError = CS_NET_ERR_CANNOT_LISTEN;
  else
    ok = true;

  if (!ok)
    CloseSocket();
}

iNetworkConnection* csSocketListener::Accept()
{
  iNetworkConnection* connection = NULL;
  if (ValidateSocket())
  {
    struct sockaddr addr;
    socklen_t addrlen = sizeof(sockaddr);
    csNetworkSocket s = accept(Socket, &addr, &addrlen);
    if (s != CS_NET_SOCKET_INVALID)
      connection = new csSocketConnection(scfParent, s, BlockingConnection);
    else if (CS_GETSOCKETERROR != EWOULDBLOCK)
      LastError = CS_NET_ERR_CANNOT_ACCEPT;
  }
  return connection;
}

csNetworkSocket csSocketListener::csSocket::GetSocket() const
{ return scfParent->GetSocket(); }


// csSocketDriver -------------------------------------------------------------

csSocketDriver::csSocketDriver(iBase* p) : LastError(CS_NET_ERR_NO_ERROR)
{
  SCF_CONSTRUCT_IBASE(p);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiEventHandler);
}
csSocketDriver::~csSocketDriver() {}
void csSocketDriver::ClearError() { LastError = CS_NET_ERR_NO_ERROR; }

void csSocketDriver::Open()
{
  ClearError();
  PlatformDriverStart();
}

void csSocketDriver::Close()
{
  ClearError();
  PlatformDriverStop();
}

bool csSocketDriver::eiComponent::Initialize(iObjectRegistry* object_reg)
{
  scfParent->object_reg = object_reg;
  iEventQueue* q = CS_QUERY_REGISTRY(object_reg, iEventQueue);
  if (q != 0)
  {
    q->RegisterListener(&(scfParent->scfiEventHandler),
    	CSMASK_Command | CSMASK_Broadcast);
    q->DecRef ();
  }
  return true;
}

csNetworkSocket csSocketDriver::CreateSocket(bool reliable)
{
  csNetworkSocket s = (reliable ?
    socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) :
    socket(AF_INET, SOCK_DGRAM,  IPPROTO_UDP));
  if (s == CS_NET_SOCKET_INVALID)
    LastError = CS_NET_ERR_CANNOT_CREATE;
  return s;
}

unsigned long csSocketDriver::ResolveAddress(const char* host)
{
  if (host == 0 || *host == 0) host = "127.0.0.1";
  unsigned long address = ntohl(inet_addr((char*)host));
  if (address == (unsigned long)-1)
  {
    const struct hostent* const p = gethostbyname((char*)host);
    if (p != 0)
      address = ntohl(*(unsigned long*)(p->h_addr_list[0]));
    else
    {
      address = 0;
      LastError = CS_NET_ERR_CANNOT_RESOLVE_ADDRESS;
    }
  }
  return address;
}

iNetworkConnection* csSocketDriver::NewConnection(
  const char* target, bool reliable, bool blocking)
{
  ClearError();
  iNetworkConnection* connection = NULL;
  if (target != NULL) // Unparse target "host:port#".
  {
    char* host = NULL;
    unsigned short port = 0;
    const char* p = strchr(target, ':');
    if (p != NULL)
    {
      host = strdup(target);
      host[p - target] = '\0';
      port = atoi(p + 1);
    }

    if (host == 0 || port == 0)
      LastError = CS_NET_ERR_CANNOT_PARSE_ADDRESS;
    else
    {
      const unsigned long address = ResolveAddress(host);
      if (address != 0)
      {
        csNetworkSocket s = CreateSocket(reliable);
        if (s != CS_NET_SOCKET_INVALID)
        {
	  struct sockaddr_in addr;
	  memset(&addr, 0, sizeof(addr));
	  addr.sin_family = AF_INET;
	  addr.sin_addr.s_addr = htonl(address);
	  addr.sin_port = htons(port);
	  if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) != -1)
	    connection = new csSocketConnection(this, s, blocking);
	  else
	    LastError = CS_NET_ERR_CANNOT_CONNECT;
	}
      }
    }
    if (host != NULL)
      free(host);
  }
  return connection;
}

iNetworkListener* csSocketDriver::NewListener(const char* source,
  bool reliable, bool blockingListener, bool blockingConnection)
{
  ClearError();
  iNetworkListener* listener = NULL;
  const unsigned short port = atoi(source);
  if (port == 0)
    LastError = CS_NET_ERR_CANNOT_PARSE_ADDRESS;
  else
  {
    csNetworkSocket s = CreateSocket(reliable);
    if (s != CS_NET_SOCKET_INVALID)
      listener = new csSocketListener(this, s, port, blockingListener,
        blockingConnection);
  }
  return listener;
}

csNetworkDriverCapabilities csSocketDriver::GetCapabilities() const
{
  csNetworkDriverCapabilities c;
  c.ConnectionReliable = true;
  c.ConnectionUnreliable = true;
  c.BehaviorBlocking = true;
  c.BehaviorNonBlocking = true;
  return c;
}

#if !defined(OS_WIN32)

bool csSocketDriver::PlatformDriverStart() { return true; }
bool csSocketDriver::PlatformDriverStop()  { return true; }

#else

bool csSocketDriver::PlatformDriverStart()
{
  bool ok = false;
  WSADATA wsaData;
  WORD wVersionRequested = MAKEWORD(2, 0);
  if (WSAStartup(wVersionRequested, &wsaData) != 0)
    LastError = CS_NET_ERR_CANNOT_GET_VERSION;
  else if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 0)
  {
    WSACleanup();
    LastError = CS_NET_ERR_WRONG_VERSION;
  }
  else
    ok = true;
  return ok;
}

bool csSocketDriver::PlatformDriverStop()
{
  bool ok = WSACleanup();
  if (!ok)
    LastError = CS_NET_ERR_CANNOT_CLEANUP;
  return ok;
}

#endif

bool csSocketDriver::eiEventHandler::HandleEvent (iEvent &e)
{
  if (e.Type == csevCommand || e.Type == csevBroadcast)
  {
    switch (e.Command.Code)
    {
      case cscmdSystemOpen:
        scfParent->Open();
        break;
      case cscmdSystemClose:
        scfParent->Close();
        break;
    }
  }
  return false;
}
