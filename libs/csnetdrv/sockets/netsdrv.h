/*
    Copyright (C) 1999 by Eric Sunshine <sunshine@sunshineco.com>
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

#ifndef __CS_NETSDRV_H__
#define __CS_NETSDRV_H__

#include "inetdrv.h"
#include "drvsdefs.h"

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
  virtual csNetworkDriverError GetLastError() const;
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
  DECLARE_IBASE;
};


class csSocketListener : public iNetworkListener, public csSocketEndPoint
{
  typedef csSocketEndPoint superclass;
protected:
  bool BlockingConnection;
public:
  csSocketListener(iBase* p, csNetworkSocket s, unsigned short port,
    bool blockingListener, bool blockingConnection);
  virtual iNetworkConnection* Accept();
  virtual void Terminate() { superclass::Terminate(); }
  virtual csNetworkDriverError GetLastError() const
    { return superclass::GetLastError(); }
  DECLARE_IBASE;
};


class csSocketDriver : public iNetworkDriver
{
protected:
  iSystem* Sys;
  csNetworkDriverError LastError;
  void ClearError();
  bool PlatformDriverStart();
  bool PlatformDriverStop();
  csNetworkSocket CreateSocket(bool reliable);
  unsigned long ResolveAddress(const char*);
public:
  csSocketDriver(iBase*);
  virtual ~csSocketDriver();
  virtual bool Initialize(iSystem*);
  virtual bool Open();
  virtual bool Close();
  virtual csNetworkDriverCapabilities GetCapabilities() const;
  virtual csNetworkDriverError GetLastError () const;

  /// The target should be a <host,port#> tuple.  It should be a string
  /// containing: "host:port#" (ie. "localhost:888").  Host can be an IP
  /// address (ie. "192.168.0.1"), a hostname (ie. "localhost"), or the empty
  /// string, in which case "localhost" is assumed.
  virtual iNetworkConnection* NewConnection(const char* target,
    bool reliable, bool blocking);

  /// The source should be a port# on which to listen.  It should be a string
  /// containing: "port#" (ie. "888").
  virtual iNetworkListener* NewListener(const char* source,
    bool reliable, bool blockingListener, bool blockingConnection);

  DECLARE_IBASE;
};

#endif // __CS_NETSDRV_H__
