/*
    Copyright (C) 2000 by Jorrit Tyberghein

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
  CS_NET_SOCKET_TYPE_TCP = 9990,		// tcp socket type
  CS_NET_SOCKET_TYPE_UDP			// udp socket type
};

enum csNetworkSocketError
{
  CS_NET_SOCKET_NOERROR,			// no errors
  CS_NET_SOCKET_CANNOT_CREATE,			// cannot create socket
  CS_NET_SOCKET_UNSUPPORTED_SOCKET_TYPE,	// unsupported socket type
  CS_NET_SOCKET_NOTCONNECTED,			// unconnected socket
  CS_NET_SOCKET_CANNOT_SETBLOCK,		// cannot set block/unblock
  CS_NET_SOCKET_CANNOT_SETREUSE,		// cannot set reuse
  CS_NET_SOCKET_CANNOT_BIND,			// cannot bind
  CS_NET_SOCKET_CANNOT_LISTEN,			// cannot listen
  CS_NET_SOCKET_CANNOT_SELECT,			// cannot select
  CS_NET_SOCKET_CANNOT_IOCTL,			// cannot ioctl
  CS_NET_SOCKET_CANNOT_ACCEPT,			// cannot accept
  CS_NET_SOCKET_NODATA,				// recv did not get data 
  CS_NET_SOCKET_CANNOT_RESOLVE,			// cannot resolve name
  CS_NET_SOCKET_CANNOT_CONNECT			// cannot connect
};

#endif
