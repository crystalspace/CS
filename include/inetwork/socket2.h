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

#ifndef __CS_INETWORK_SOCKET2_H__
#define __CS_INETWORK_SOCKET2_H__

#include "csutil/scf.h"

#if !defined(COMP_VC) && !defined(COMP_BC)
# warning This file is deprecated please use iNetworkSocket
#endif

SCF_VERSION (iNetworkSocket2, 0, 0, 2);

/**
 * A network socket.  Network sockets are created via invocations of
 * iNetworkDriver2::CreateSocket().
 */
struct iNetworkSocket2 : public iBase
{
  /**
   * Returns true if the connection is TCP and the socket is connected.
   * Returns false if the connection is TCP and the socket is not connected, or
   * the connection is UDP.
   */
  virtual bool IsConnected () const = 0;

  /// Returns last driver error (see CS_NET_SOCKET_FOO in inetwork/driver2.h).
  virtual int LastError() const = 0;

  /**
   * Sets the socket blocking operation if block is true, or unsets the
   * blocking operation if block is false.  Returns CS_NET_SOCKET_NOERROR if
   * there is no error.
   */
  virtual int SetSocketBlock (bool block) = 0;

  /**
   * Sets the reuse option on a server socket if reuse is true, otherwise the
   * reuse option is unset.  Returns CS_NET_SOCKET_NOERROR if there is no
   * error.
   */
  virtual int SetSocketReuse (bool reuse) = 0;

  /**
   * Sets the broadcast option on socket if broadcast is true, otherwise the
   * broadcast option is unset. Returns CS_NET_SOCKET_NOERROR if there is no
   * error. Function working only for UDP socket type.
   */
  virtual int SetSocketBroadcast( bool broadcast) = 0;

  /**
   * Sets the broadcast port and address (0 means broadcast to all).
     Returns CS_NET_SOCKET_NOERROR if there is no error.
     Function working only for UDP socket type.
   */
  virtual int SetBroadcastOptions(int port, const char* addr = 0) = 0;
  /**
   * Connect to the hostname host on port port.  Returns CS_NET_SOCKET_NOERROR
   * if there is no error.
   */
  virtual int Connect (char const* host, int port) = 0; 

  /**
   * Send size bytes of buff to remote connection.  Returns number of bytes
   * sent or -1 on error.  Use LastError() for the actual error.
   */
  virtual int Send (char const* buff, size_t size) = 0;

  /**
   * Read size of bytes from connection into buff.  Returns number of bytes
   * stored in buff or -1 on error.  This function doesn't block on UDP
   * connections.  If connection is UDP then this function returns
   * CS_NET_SOCKET_NODATA if nothing was read.  Use LastError() for the actual
   * error.
   */
  virtual int Recv (char* buff, size_t size) = 0;

  /// Closes the network socket.  The socket is unusible after this call.
  virtual int Close () = 0;

  /// Disconnects the socket connection.  Then calls closes the socket.
  virtual int Disconnect () = 0;

  /**
   * Binds to the interface source on port port.  `source' is a numeric
   * representation of a host address stored in network byte order, such as
   * those returned by ::inet_addr().  `port' is the port number in host byte
   * order on which to listen.  Listens on bound socket - queing up to `queue'
   * number of connections.  Returns CS_NET_SOCKET_NOERROR if there is no
   * error.
   */
  virtual int WaitForConnection (int source, int port, int queue) = 0;

  /**
   * Returns a NetworkSocket2 instance for the new connection.  Returns 0 on
   * error.  Use LastError() for the actual error.
   */
  virtual iNetworkSocket2 *Accept () = 0;

  /**
   * Performs a buffered read on either TCP or UDP connections until size bytes
   * is reached or '\r' or '\0' is received.  Returns number of bytes read into
   * buff when complete.  Returns 0 when line is incomplete.  Returns -1 upon
   * error.  Use LastError() for the actual error.
   */
  virtual int ReadLine (char *buff, size_t size) = 0;

  /**
   * Returns a string containing either the IP or hostname of the remote
   * connection.
   */
  virtual char const* RemoteName () const = 0;
};

#endif // __CS_INETWORK_SOCKET2_H__
