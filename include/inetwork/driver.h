/*
    Copyright (C) 1999,2000 by Eric Sunshine <sunshine@sunshineco.com>
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

#ifndef __INETWORK_DRIVER_H__
#define __INETWORK_DRIVER_H__

#include "csutil/scf.h"

/**
 * Potential network driver error codes.
 */
enum csNetworkDriverError
{
  CS_NET_ERR_NO_ERROR,
  CS_NET_ERR_CANNOT_RESOLVE_ADDRESS,
  CS_NET_ERR_CANNOT_CONNECT,
  CS_NET_ERR_CANNOT_SEND,
  CS_NET_ERR_INVALID_SOCKET,
  CS_NET_ERR_CANNOT_BIND,
  CS_NET_ERR_CANNOT_LISTEN,
  CS_NET_ERR_CANNOT_CREATE,
  CS_NET_ERR_CANNOT_ACCEPT,
  CS_NET_ERR_CANNOT_SET_BLOCKING_MODE,
  CS_NET_ERR_CANNOT_RECEIVE,
  CS_NET_ERR_CANNOT_PARSE_ADDRESS,
  CS_NET_ERR_CANNOT_GET_VERSION,
  CS_NET_ERR_WRONG_VERSION,
  CS_NET_ERR_CANNOT_CLEANUP
};

/**
 * Network driver capabilities structure.
 */
struct csNetworkDriverCapabilities
{
  bool ConnectionReliable;
  bool ConnectionUnreliable;
  bool BehaviorBlocking;
  bool BehaviorNonBlocking;
};


SCF_VERSION (iNetworkEndPoint, 0, 0, 1);

/**
 * This is the network end-point interface for CS.  It represents one end of
 * a network connection or potential connection (such as a listener).  All
 * network end-points must implement this interface.
 */
struct iNetworkEndPoint : public iBase
{
  /// Terminates the connection; destroying the object also auto-terminates.
  virtual void Terminate() = 0;

  /// Retrieve the code for the last error encountered.
  virtual csNetworkDriverError GetLastError() const = 0;
};


SCF_VERSION (iNetworkConnection, 0, 0, 1);

/**
 * This is the network connection interface for CS.  It represents a single
 * network connection.  All network connections must implement this interface.
 */
struct iNetworkConnection : public iNetworkEndPoint
{
  /// Send nbytes of data over the connection.
  virtual bool Send(const void* data, size_t nbytes) = 0;

  /**
   * Receive data from the connection.  If the connection is in blocking
   * mode, then the function does not return until data has been read, an
   * error has occurred, or the connection was closed.  In non-blocking mode,
   * Receive returns immediately.  If data is available then it returns the
   * number of bytes (<= maxbytes) which was read.  If data is not available
   * and the connection is non-blocking, then it returns 0 and GetLastError()
   * returns CS_NET_ERR_NO_ERROR.
   */
  virtual size_t Receive(void* buff, size_t maxbytes) = 0;
};


SCF_VERSION (iNetworkListener, 0, 0, 1);

/**
 * This is the network listener interface for CS.  It represents a single
 * network listening post.  All network listeners must implement this
 * interface.
 */
struct iNetworkListener : public iNetworkEndPoint
{
  /**
   * Accepts a connection request.  If the listener is in blocking mode, then
   * the function does not return until a connection has been established or
   * an error has occurred.  If in non-blocking mode, then it returns
   * immediately.  The return value is either an accepted connection or NULL.
   * If the connection is non-blocking, NULL is returned, and GetLastError()
   * returns CS_NET_ERR_NO_ERROR then no connection was pending.  Otherwise
   * an error occurred, and GetLastError() returns the appropriate error code.
   */
  virtual iNetworkConnection* Accept() = 0;
};


SCF_VERSION (iNetworkDriver, 0, 0, 1);

/**
 * This is the network driver interface for CS.  It represents a plug-in
 * network driver module.  All network drivers must implement this interface.
 */
struct iNetworkDriver : public iBase
{
  /**
   * Create a new network connection.  The 'target' parameter is driver
   * dependent.  For example, with a socket driver, the target might be
   * "host:port#"; with a modem driver it might be "comport:phone#"; etc.
   * The 'reliable' flag determines whether a reliable connection is made
   * (sometimes known as connection-oriented) or an unreliable one (sometimes
   * known as connectionless).  The 'blocking' flag determines whether
   * operations on the connection return immediately in all cases or wait
   * until the operation can be completed successfully.  Returns the new
   * connection object or NULL if the connection failed.
   */
  virtual iNetworkConnection* NewConnection(const char* target,
    bool reliable, bool blocking) = 0;

  /**
   * Create a new network listener.  The 'source' parameter is driver
   * dependent.  For example, with a socket driver, the target might be
   * "port#"; with a modem driver it might be "comport"; etc.  The 'reliable'
   * determines whether or not a reliable connection is made.  The
   * 'blockingListener' flag determines whether or not the Accept() method
   * blocks while when called.  The 'blockingConnection' flag determines
   * whether or not methods in the resulting connection object block.
   */
  virtual iNetworkListener* NewListener(const char* source,
    bool reliable, bool blockingListener, bool blockingConnection) = 0;

  /**
   * Get network driver capabilities.  This function returns information
   * describing the capabilities of the driver.
   */
  virtual csNetworkDriverCapabilities GetCapabilities() const = 0;

  /// Retrieve the code for the last error encountered.
  virtual csNetworkDriverError GetLastError() const = 0;
};

#endif // __INETWORK_DRIVER_H__
