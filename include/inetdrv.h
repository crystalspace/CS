/*
    Copyright (C) 1998, 1999 by Serguei 'Snaar' Narojnyi
    Copyright (C) 1998, 1999 by Jorrit Tyberghein
    Written by Serguei 'Snaar' Narojnyi

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

#ifndef __INETWORK_H__
#define __INETWORK_H__

#include "csutil/scf.h"
#include "isystem.h"
#include "istring.h"

#define CS_NET_CONNORIENTED	0
#define CS_NET_CONNLESS		1

enum csNetworkError
{
  CS_NET_NO_ERROR=0,
  CS_NET_NOT_INITIALIZED,
  CS_NET_ALREADY_CONNECTED,
  CS_NET_CANNOT_RESOLVE_NAME,
  CS_NET_CANNOT_CONNECT,
  CS_NET_NOT_CONNECTED,
  CS_NET_CANNOT_SEND,
  CS_NET_CANNOT_GET_VERSION,
  CS_NET_WRONG_VERSION,
  CS_NET_CANNOT_CLEANUP,
  CS_NET_INVALID_SOCKET,
  CS_NET_ALREADY_LISTENING,
  CS_NET_CANNOT_BIND,
  CS_NET_CANNOT_LISTEN,
  CS_NET_LIMIT_REACHED,
  CS_NET_INVALID_TYPE,
  CS_NET_CANNOT_CREATE,
  CS_NET_CANNOT_CLOSE,
  CS_NET_NOT_LISTENING,
  CS_NET_CANNON_GET_SOCKOPT,
  CS_NET_CANNOT_ACCEPT,
  CS_NET_CANNOT_SET_PARAMS,
  CS_NET_CANNOT_RECEIVE
};

struct csNetworkAddress
{
  char hostnm[512];
  int port;
};

struct csNetworkCaps
{
  bool ConnOriented;
  bool ConnLess;
  unsigned int iMaxSockets;
};

/// This is a connection handle
typedef unsigned int csNetHandle;

/**
 * This is the network interface for CS.
 * All network drivers must implement this interface.
 * The standard implementation is csNetworkDriverNull.
 */
SCF_INTERFACE (iNetworkDriver, 0, 0, 1) : public iBase
{
public:

  virtual bool Initialize (iSystem *iSys) = 0;

  /// Open the network driver
  virtual bool Open () = 0;
  /// Close the network driver
  virtual bool Close () = 0;

  csNetHandle Spawn(csNetworkCaps *caps);

  virtual bool Connect (csNetworkAddress *iNetAddress) = 0;

  virtual void Disconnect (csNetHandle iHandle) = 0;

  virtual void Send (csNetHandle iHandle, iString *iStr) = 0;

  virtual void Receive (csNetHandle iHandle, iString *iStr) = 0;

  virtual void SetListenState (csNetHandle iHandle, int iPort) = 0;

  virtual void Accept (csNetHandle iListen, csNetHandle *iServer, csNetworkAddress *oAddress) = 0;

  virtual void Kill (csNetHandle Handle) = 0;

  virtual void KillAll () = 0;

  virtual void GetDriverCaps (csNetworkCaps *oCaps) = 0;

  virtual int GetLastError () = 0;

//virtual void SetOnReceiveFunction (void (*iFunction) (void *), void *iParm) = 0;

//virtual void (*) (void *) GetOnReceiveFunction () = 0;
};

#endif	//__INETWORK_H__
