/*
    ENSOCKET Plugin
    Copyright (C) 2002 by Erik Namtvedt

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

#ifndef __INETWORK_SOCKETERR__
#define __INETWORK_SOCKETERR__

enum csNetworkSocketType
{
  /// TCP Socket Type
  CS_NET_SOCKET_TYPE_TCP = 9990,
  /// UDP Socket Type
  CS_NET_SOCKET_TYPE_UDP			    
};

enum csNetworkSocketError
{
  /// No error
  CS_NET_SOCKET_NOERROR = 0,
  /// Unable to create the socket
  CS_NET_SOCKET_CANNOT_CREATE = 9900,
  /// Unsupported socket type
  CS_NET_SOCKET_UNSUPPORTED_SOCKET_TYPE,
  /// The socket isn't connected
  CS_NET_SOCKET_NOTCONNECTED,
  /// Unable to block/unblock on socket
  CS_NET_SOCKET_CANNOT_SETBLOCK,
  /// Unable to reuse the socket
  CS_NET_SOCKET_CANNOT_SETREUSE,
  /// Unable to bind to the port
  CS_NET_SOCKET_CANNOT_BIND,
  /// Unable to listen to the socket
  CS_NET_SOCKET_CANNOT_LISTEN,
  /// Unable to select on the socket
  CS_NET_SOCKET_CANNOT_SELECT,
  /// Unable to ioctl the socket
  CS_NET_SOCKET_CANNOT_IOCTL,
  /// Unable to accept incoming connection
  CS_NET_SOCKET_CANNOT_ACCEPT,
  /// No data was received
  CS_NET_SOCKET_NODATA,
  /// Unable to resolve address
  CS_NET_SOCKET_CANNOT_RESOLVE,
  /// Unable to connect
  CS_NET_SOCKET_CANNOT_CONNECT
};

#endif
