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

#ifndef __INETWORK_SOCKET2__
#define __INETWORK_SOCKET2__

#include "csutil/scf.h"

SCF_VERSION (iNetworkSocket2, 0, 0, 1);

#ifndef OS_WIN32
#ifdef OS_UNIX
#endif 
#define SOCKET int
#define closesocket close
#define socket_error -1
#else
#endif

struct iNetworkSocket2 : public iBase
{
  /// Return status of the tcp connection
  virtual bool IsConnected () = 0;
  /**
	* Returns true if the connection is TCP and the socket
	* is connected.  Returns false if the connection is TCP
	* and the socket is not connected, or the connection is
	* UDP.
	*/

  /// Return the last error we ran into
  virtual int LastError() = 0;
  /**
	* Returns the last error we ran into.
	* (see CS_NET_SOCKET_XXXXX in sockerr.h)
	*/

  /// Set the block operation of the socket
  virtual int SetSocketBlock (bool block) = 0;
  /**
    * Sets the socket blocking operation if block is true,
	* or unsets the blocking operation if block is false.
	* Returns CS_NET_SOCKET_NOERROR if there is no error.
	*/

  /// Set the reuse option of the socket
  virtual int SetSocketReuse (bool reuse) = 0;
  /**
    * Sets the reuse option on a server socket if reuse
	* is true, otherwise the reuse option is unset.
	* Returns CS_NET_SOCKET_NOERROR if there is no error.
	*/

  /// Connect to host on port
  virtual int Connect (char *host, int port) = 0; 
  /**
    * Connect to the hostname host on port port.
	* Returns CS_NET_SOCKET_NOERROR if there is no error.
	*/

  /// Send data to remote connection
  virtual int Send (char *buff, size_t size) = 0;
  /**
    * Send size bytes of buff to remote connection.
	* Returns number of bytes sent or -1 on error.
	* Use LastError() for the actual error.
	*/

  /// Read data from remote connect
  virtual int Recv (char *buff, size_t size) = 0;
  /**
    * Read size of bytes from connection into buff.
	* Returns number of bytes stored in buff or -1
	* on error.
	* This function doesn't block on UDP connections.
	* If connection is UDP then this function returns
	* CS_NET_SOCKET_NODATA if nothing was read.
	* Use LastError() for the actual error.
	*/

  /// Close the socket connection
  virtual int Close () = 0;
  /**
    * Closes the network socket.
	* The socket is unusible after this call.
	*/

  /// Disconnect the socket connection
  virtual int Disconnect () = 0;
  /** 
    * Disconnects the socket connection
	* Then calls closes the socket.
	*/

  /// Bind and Listen to incoming connections
  virtual int WaitForConnection (int source, int port, int que) = 0;
  /**
    * Binds to the interface source on port port.
	* Listens on bound socket - queing up to que
	* number of connections.
	* Returns CS_NET_SOCKET_NOERROR if there is no error.
	*/

  /// Accept incoming connections
  virtual iNetworkSocket2 *Accept () = 0;
  /**
	* Returns a NetworkSocket2 instance for the
	* new connection.
	* Returns NULL on error.
	* Use LastError() for the actual error.
	*/

  /// Read bytes until length of buff is size or \r or \0 are received
  virtual int ReadLine (char *buff, size_t size) = 0;
  /**
    * Performs a buffered read on either TCP or UDP
	* connections until size bytes is reached or
	* '\r' or '\0' is received.
	* Returns number of bytes read into buff when
	* complete, or returns 0.
	* Use LastError() for the actual error.
	*/

  /// Return the remote hostname or IP of connection
  virtual char *RemoteName () = 0;
  /**
    * Returns a string containing either the IP
	* or hostname of the remote connection.
	* Use LastError() for the actual error.
	*/
};

#endif
