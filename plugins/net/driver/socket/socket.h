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

#ifndef __CS_SOCKET_H__
#define __CS_SOCKET_H__

#include "inetwork/socket.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"

class csSocketEndPoint
{
protected:
  csNetworkSocket Socket;
  csNetworkDriverError LastError;
  void CloseSocket();
  void ClearError();
  bool ValidateSocket();
  bool PlatformSetBlocking(bool);
public:
  csSocketEndPoint(csNetworkSocket s, bool blocks);
  virtual ~csSocketEndPoint();
  virtual void Terminate();
  virtual csNetworkDriverError GetLastError() const { return LastError; }
  csNetworkSocket GetSocket() const { return Socket; }
};


class csSocketConnection : public iNetworkConnection, public csSocketEndPoint
{
  typedef csSocketEndPoint superclass;
public:
  csSocketConnection(iBase* p, csNetworkSocket, bool blocking);
  virtual bool Send(const void* data, size_t nbytes);
  virtual size_t Receive(void* buff, size_t maxbytes);
  virtual void Terminate() { superclass::Terminate(); }
  virtual csNetworkDriverError GetLastError() const
    { return superclass::GetLastError(); }

  SCF_DECLARE_IBASE;
  struct csSocket : public iNetworkSocket
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSocketConnection);
    virtual csNetworkSocket GetSocket() const;
  } scfiNetworkSocket;
};


class csSocketListener : public iNetworkListener, public csSocketEndPoint
{
  typedef csSocketEndPoint superclass;
protected:
  bool BlockingConnection;
public:
  csSocketListener(iBase* p, csNetworkSocket s, unsigned short port,
    bool blockingListener, bool blockingConnection);
  virtual csPtr<iNetworkConnection> Accept();
  virtual void Terminate() { superclass::Terminate(); }
  virtual csNetworkDriverError GetLastError() const
    { return superclass::GetLastError(); }

  SCF_DECLARE_IBASE;
  struct csSocket : public iNetworkSocket
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSocketListener);
    virtual csNetworkSocket GetSocket() const;
  } scfiNetworkSocket;
};


class csSocketDriver : public iNetworkDriver
{
protected:
  iObjectRegistry* object_reg;
  csNetworkDriverError LastError;
  void ClearError();
  bool PlatformDriverStart();
  bool PlatformDriverStop();
  csNetworkSocket CreateSocket(bool reliable);
  unsigned long ResolveAddress(const char*);
public:
  csSocketDriver(iBase*);
  virtual ~csSocketDriver();
  void Open();
  void Close();
  virtual csNetworkDriverCapabilities GetCapabilities() const;
  virtual csNetworkDriverError GetLastError () const { return LastError; }

  /**
   * The target should be a string containing: "host:port/protocol"
   * (eg. "localhost:888/tcp"). Host can be an IP address
   * (eg. "192.168.0.1"), a hostname (eg. "localhost"), or the empty string,
   * in which case "localhost" is assumed. Protocol is optional, can be
   * "tcp" or "udp", and defaults to the value of the reliable flag.
   */
  virtual csPtr<iNetworkConnection> NewConnection(const char* target,
    bool reliable, bool blocking);

  /**
   * The source should be a string containing "port/protocol"
   * (eg. "888/tcp"). Protocol is optional, can be "tcp" or "udp", and
   * defaults to the value of the reliable flag.
   */
  virtual csPtr<iNetworkListener> NewListener(const char* source,
    bool reliable, bool blockingListener, bool blockingConnection);

  SCF_DECLARE_IBASE;

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSocketDriver);
    virtual bool Initialize (iObjectRegistry*);
  } scfiComponent;
  friend struct eiComponent;
  struct eiEventHandler : public iEventHandler
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSocketDriver);
    virtual bool HandleEvent (iEvent&);
  } scfiEventHandler;
  friend struct eiEventHandler;
};

#endif // __CS_SOCKET_H__
