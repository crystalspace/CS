/*
    Copyright (C) 2002 by Matze Braun

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
#ifndef __CS_CSSYS_SOCKETS_H__
#define __CS_CSSYS_SOCKETS_H__

/** \file
 *      For TCP/IP sockets definitions.  Specifically, should define the
 *      following macros, constants, typedefs, and prototypes:
 *	inet_addr(), gethostbyname(), ntohl(), etc.
 *	socket(), listen(), bind(), etc. -- the standard socket functions
 *	csNetworkSocket -- typedef or macro for socket descriptor type
 *	struct sockaddr -- standard socket address type (and cousins)
 *	socklen_t -- typedef or macro
 *	CS_NET_SOCKET_INVALID -- value representing invalid socket
 *	CS_CLOSESOCKET -- name of function to close a socket
 *	CS_IOCTLSOCKET -- name of "ioctl" function for sockets
 *	CS_GETSOCKETERROR -- name of function or variable for socket error code
 */ 

#include <sys/types.h>

#if defined(CS_PLATFORM_WIN32) && !defined(__CYGWIN__)
#  include <winsock.h>
#  define CS_NET_SOCKET_INVALID INVALID_SOCKET
#  define CS_IOCTLSOCKET ioctlsocket
#  define CS_CLOSESOCKET closesocket
#  if defined(__CYGWIN__) && defined(EWOULDBLOCK)
#   undef EWOULDBLOCK
#  endif
#  define EWOULDBLOCK WSAEWOULDBLOCK
#  define CS_GETSOCKETERROR ::WSAGetLastError()
#elif defined(CS_PLATFORM_UNIX) || defined(__CYGWIN__)
#  include <sys/socket.h>
#  include <unistd.h>
#  define BSD_COMP 1
#  include <sys/ioctl.h>
#  include <arpa/inet.h>
#  include <sys/time.h>
#  include <netinet/in.h>
#  include <netdb.h>
#endif
#if !defined (CS_IOCTLSOCKET)
#  define CS_IOCTLSOCKET ioctl
#endif
#if !defined (CS_CLOSESOCKET)
#  define CS_CLOSESOCKET close
#endif
#if !defined (CS_GETSOCKETERROR)
#  define CS_GETSOCKETERROR errno
#endif

typedef unsigned int csNetworkSocket;

#if !defined (CS_NET_SOCKET_INVALID)
#  define CS_NET_SOCKET_INVALID ((csNetworkSocket)~0)
#endif

// Platforms which supply a socklen_t type should define CS_HAVE_SOCKLEN_T.
// Note that we invoke the typedef only if some other entity has not already
// #defined socklen_t, since the #define would cause problems (for example,
// `typedef int int'). Some system-supplied headers will #define
// __socklen_t_defined if the socklen_t typedef has already been setup, so we
// check that as well.
#if !defined(CS_HAVE_SOCKLEN_T) && \
    !defined(socklen_t) && \
    !defined(__socklen_t_defined)
  typedef int socklen_t;
#endif

#endif // __CS_CSSYS_SOCKETS_H__
