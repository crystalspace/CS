/*
    Copyright (C) 2000 by Thomas Riemer

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

#ifndef __CS_INETMAN_H__
#define __CS_INETMAN_H__

#include "csutil/scf.h"
#include "iplugin.h"
#include "iproto.h"

/**
 * This is a pain... the error handling has to match that in inetdrv.h
 * what a royal pain in the neck.
 * These have to be synced with the errors in inetdrv.h (Thomas Riemer)
 * (Note from ES: Error handling will be revised in the future to be easily
 * extensible and will be string-based or class-based with string ID's.)
 */
enum csNetworkManagerError 
{
  /// Errors from netdriver
  CS_NETMAN_ERR_NO_ERROR,
  CS_NETMAN_ERR_CANNOT_RESOLVE_ADDRESS,
  CS_NETMAN_ERR_CANNOT_CONNECT,
  CS_NETMAN_ERR_CANNOT_SEND,
  CS_NETMAN_ERR_INVALID_SOCKET,
  CS_NETMAN_ERR_CANNOT_BIND,
  CS_NETMAN_ERR_CANNOT_LISTEN,
  CS_NETMAN_ERR_CANNOT_CREATE,
  CS_NETMAN_ERR_CANNOT_ACCEPT,
  CS_NETMAN_ERR_CANNOT_SET_BLOCKING_MODE,
  CS_NETMAN_ERR_CANNOT_RECEIVE,
  CS_NETMAN_ERR_CANNOT_PARSE_ADDRESS,
  CS_NETMAN_ERR_CANNOT_GET_VERSION,
  CS_NETMAN_ERR_WRONG_VERSION,
  CS_NETMAN_ERR_CANNOT_CLEANUP,

  /// Errors from Netmanager layer.
  CS_NETMAN_PROTO_NOT_IMPL,
  CS_NETMAN_NO_PROTOCOL, 
  CS_NETMAN_OUT_OF_PORTS
};

#define NETPORT_PROTO_UNKNOWN (-1)
#define NETPORT_PROTO_TCP     (1)
#define NETPORT_PROTO_UDP     (2)
#define NETPORT_PROTO_SOCKET  (3)

/* Various roles that a port might play */
#define NETPORT_ROLE_UNKNOWN    (-1)

/* output connection role - connection originates from app
  i.e. for server to server connection. or for 
  client side app. */
#define NETPORT_ROLE_OUTBOUND    (1)

/* server connection role - port needs to be listened to */
#define NETPORT_ROLE_SERVER      (2)

/* server received a connection from outside  that has to be manhandled*/  
#define NETPORT_ROLE_CONNECTION  (3)

/* If the connection arrives from an external site, then the 
   parent is defined as the port that it first arrived on */
#define NETPORT_NOPARENT        (-1)
#define NETPORT_PARENT_SERVER   (-2)

/* Initial the port so that we don't accidently look at a port 
   like port 0  - doh! */
#define NETPORT_PORT_UNKNOWN    (-1)
 
/* The status of the network socket */
#define NETPORT_STATUS_CLEAR    (-1)
#define NETPORT_STATUS_CONNECTED (1)


/* Standard Error condition */
#define NETMAN_ERROR            (-1)
#define NETMAN_SUCCESS          (0)

// Events passed to protocol layer... ::NetEventNotify.

#define NETMAN_SERVER_STOP          (1)
#define NETMAN_SERVER_START         (2)
#define NETMAN_CONNECTION_CONNECT       (3) 
#define NETMAN_CONNECTION_DISCONNECT    (4) 
#define NETMAN_OUTBOUND_CONNECT     (5)
#define NETMAN_OUTBOUND_DISCONNECT  (6)

SCF_VERSION (iNetworkManager, 0, 0, 1);

/**
 * This is the network manager interface for CS.  It represents a plug-in
 * network manager module.  All network managers must implement this interface.
 * @@@ Please add additional comments to methods using Doc++!
 */
struct iNetworkManager : public iPlugIn
{
  virtual void AssignHostName(const char *hostname) = 0;

  virtual int AssignServer(int ipPortNumber,  
			   int ipProtocol,  int maxConnects) =0;

  virtual void SetPollCounter(int counter) = 0;

  virtual void Update() = 0;
  virtual bool HandleEvent()= 0;

  virtual int StopAllConnections() = 0;
  virtual int StopAllServers() =0;
  virtual int StopServer(int csNetPort) =0;
  virtual int ResignNetPort(int csNetPort) =0;

  virtual int AssignClient(const char *hostname, int port, int protocol) =0;
  virtual int AssignClient(const char *hostandport, int protocol) =0;

  virtual void NetControl(int NetPort, int len, const char *msg)=0;

  virtual void Reset() =0;
  
  virtual int SendMsg(int csNetPort, int len, const char *msg) =0;
  virtual int SendMsg(int csNetPort, const char *hostname, int len, const char *msg)=0;

  virtual void Broadcast(const char *) =0;
  virtual void Broadcast(int csNetPort, int len, const char *msg) =0;

  // Glue to NSTP
  virtual void AssignProtocol(iNetSpaceProtocol *AssignedProtocol) =0;
    
  /// Utility stuff
  virtual int GetLastError () = 0;
  virtual void GetCapabilities () = 0;

  // iPlugIn interface.
  virtual bool Initialize (iSystem*) = 0;
  virtual bool Open () = 0;
  virtual bool Close () = 0;
};

#endif // __CS_INETMAN_H__
