/*
    Copyright (C) 1999,2000 by Eric Sunshine <sunshine@sunshineco.com>
    Copyright (C) 2003 by Mat Sutcliffe <oktal@gmx.co.uk>
    Copyright (C) 2003 by Ladislav Foldyna <foldyna@unileoben.ac.at>
    Originally written by Eric Sunshine
    UDP support added by Mat Sutcliffe
    Multicast support by Ladislav Foldyna, merged by Mat Sutcliffe

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

/// This is the BSD sockets implementation of iNetworkEndPoint.
class csSocketEndPoint
{
protected:
  bool Reliable;
  csNetworkSocket Socket;
  csNetworkDriverError LastError;
  void CloseSocket();
  void ClearError();
  bool ValidateSocket();
  bool PlatformSetBlocking(bool);
public:
  csSocketEndPoint(csNetworkSocket s, bool blocks, bool reliable);
  virtual ~csSocketEndPoint();
  virtual void Terminate();
  virtual csNetworkDriverError GetLastError() const { return LastError; }
  csNetworkSocket GetSocket() const { return Socket; }

  /**
   * Supported options:<br><ul>
   * <li>"TTL": multicast packets' Time-To-Live.</li>
   * <li>"Loop": bool 0 or 1 meaning multicast packets echo to sender.</li></ul>
   */
  virtual bool SetOption (const char *name, int value);
};


/// This is the BSD sockets implementation of iNetworkConnection.
class csSocketConnection : public iNetworkConnection, public csSocketEndPoint
{
  typedef csSocketEndPoint superclass;
  struct sockaddr thisaddr;
  struct sockaddr mcastaddr;
public:
  csSocketConnection(iBase* p, csNetworkSocket, bool blocking, bool reliable,
    struct sockaddr addr, struct sockaddr *mcastaddr = 0);
  csSocketConnection::~csSocketConnection();
  virtual bool Send(const char* data, size_t nbytes);
  virtual size_t Receive(void* buff, size_t maxbytes);
  virtual size_t Receive(void* buff, size_t maxbytes, csRef<iString> &from);
  virtual bool IsDataWaiting() const;
  virtual void Terminate() { superclass::Terminate(); }
  virtual bool IsConnected() const;
  virtual csNetworkDriverError GetLastError() const
    { return superclass::GetLastError(); }

  /**
   * Supported options:<br><ul>
   * <li>"TTL": multicast packets' Time-To-Live.</li>
   * <li>"Loop": bool 0 or 1 meaning multicast packets echo to sender.</li></ul>
   */
  virtual bool SetOption (const char *name, int value)
    { return superclass::SetOption (name, value); }

  SCF_DECLARE_IBASE;
  struct csSocket : public iNetworkSocket
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSocketConnection);
    virtual csNetworkSocket GetSocket() const;
  } scfiNetworkSocket;
};


/// This is the BSD sockets implementation of iNetworkListener.
class csSocketListener : public iNetworkListener, public csSocketEndPoint
{
  typedef csSocketEndPoint superclass;
protected:
  bool BlockingConnection;
  struct sockaddr_in mcast_in;
  struct sockaddr_in addr_in;
public:
  csSocketListener(iBase* p, csNetworkSocket s, unsigned short port,
    bool blockingListener, bool blockingConnection, bool reliable,
    unsigned long mcastaddress);
  virtual ~csSocketListener();
  virtual csPtr<iNetworkConnection> Accept();
  virtual void Terminate() { superclass::Terminate(); }
  virtual csNetworkDriverError GetLastError() const
    { return superclass::GetLastError(); }

  /**
   * Supported options:<br><ul>
   * <li>"TTL": multicast packets' Time-To-Live.</li>
   * <li>"Loop": bool 0 or 1 meaning multicast packets echo to sender.</li></ul>
   */
  virtual bool SetOption (const char *name, int value)
    { return superclass::SetOption (name, value); }

  SCF_DECLARE_IBASE;
  struct csSocket : public iNetworkSocket
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSocketListener);
    virtual csNetworkSocket GetSocket() const;
  } scfiNetworkSocket;
};

/// This is the BSD sockets implementation of iNetworkDriver.
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
   * in which case "localhost" is assumed. Protocol can be "tcp", "udp"
   * or "multicast".
   */
  virtual csPtr<iNetworkConnection> NewConnection(const char* target,
    bool reliable, bool blocking);

  /**
   * The source should be a string containing "port/protocol"
   * (eg. "888/tcp"). Protocol can be "tcp", "udp" or "multicast".
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
