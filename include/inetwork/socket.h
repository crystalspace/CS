/*
    Copyright (C) 2000 by Eric Sunshine <sunshine@sunshineco.com>
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

#ifndef __CS_INETWORK_SOCKET_H__
#define __CS_INETWORK_SOCKET_H__

#include "inetwork/driver.h"
#include "cssys/sockets.h"

SCF_VERSION (iNetworkSocket, 0, 0, 1);

/**
 * This interface represents a network socket.  iNetworkConnection and
 * iNetworkListener are abstract network interfaces which might be implemented
 * in any number of ways, such as via the BSD Sockets, serial modem, direct
 * connection, neural link (ESP), or even smoke signals.  Modules which
 * implement these interfaces using the BSD Sockets protocol can also choose to
 * implement the iNetworkSocket protocol, which provides access to the
 * low-level <n>socket</n> identifier which can be used to make calls to
 * various low-level Socket functions, such as send(), recv(), ioctl(), etc.
 * Use SCF_QUERY_INTERFACE(), to find out if an iNetworkConnection or an
 * iNetworkListener implements iNetworkSocket.  If SCF_QUERY_INTERFACE()
 * returns null, then the connection or listener is implemented via some
 * non-BSD Socket mechanism.  On the other hand, if SCF_QUERY_INTERFACE()
 * returns an iNetworkSocket, then the connection or listener is BSD
 * Socket-based, and you can use the methods of iNetworkSocket to manipulate
 * the connection or listener in ways not provided by the abstract
 * iNetworkConnection and iNetworkListener interfaces.
 */
struct iNetworkSocket : public iBase
{
  /**
   * Retrieve the socket associated with this object.  The returned socket is
   * the low-level platform-dependent BSD Socket identifier.  The exact
   * representation may vary from platform (typically it is an unsigned
   * integer), though you normally need not worry about the precise
   * representation.  The returned Socket identifier can be used in cases where
   * you need to make calls to low-level Socket functions, such as send(),
   * recv(), ioctl(), etc.
   */
  virtual csNetworkSocket GetSocket() const = 0;
};

#endif // __CS_INETWORK_SOCKET_H__
