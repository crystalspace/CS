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

#ifndef __CS_INETWORK_DRIVER_H__
#define __CS_INETWORK_DRIVER_H__

#include "csutil/scf.h"
#include "csutil/ref.h"

struct iString;

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
  CS_NET_ERR_CANNOT_CLEANUP,
  CS_NET_ERR_NO_SUCH_OPTION,
  CS_NET_ERR_CANNOT_SET_OPTION
};

/**
 * Network driver capabilities structure.
 * \deprecated
 * This is deprecated since the reliable flag is deprecated,
 * and all drivers should support blocking and non-blocking.
 */
struct csNetworkDriverCapabilities
{
  bool ConnectionReliable;
  bool ConnectionUnreliable;
  bool BehaviorBlocking;
  bool BehaviorNonBlocking;
};


SCF_VERSION (iNetworkEndPoint, 0, 1, 1);

/**
 * This is a network end-point interface.  It represents one end of
 * a network connection or potential connection (such as a listener).  All
 * network end-points must implement this interface.
 */
struct iNetworkEndPoint : public iBase
{
  /// Terminate the connection.  Destroying the object also auto-terminates.
  virtual void Terminate() = 0;

  /**
   * Set an driver-specific option for the end-point.  Each driver may support
   * options beyond those provided by these abstract interfaces.  For example,
   * the cssocket driver supports 'ttl' and 'loop' options.  Consult
   * driver-specific documentation as necessary.
   */
  virtual bool SetOption (const char *name, int value) = 0;

  /// Retrieve the code for the last error encountered.
  virtual csNetworkDriverError GetLastError() const = 0;
};


SCF_VERSION (iNetworkConnection, 0, 1, 1);

/**
 * This is a network connection interface.  It represents a single
 * network connection.  All network connections must implement this interface.
 */
struct iNetworkConnection : public iNetworkEndPoint
{
  /// Send nbytes of data over the connection.
  virtual bool Send(const char* data, size_t nbytes) = 0;

  /// See if the connection is still connected.
  virtual bool IsConnected () const = 0;

  /**
   * Receive data from the connection.  If the endpoint is conifgured to block,
   * then the function does not return until data has been read, an
   * error has occurred, or the connection was closed.  If non-blocking, then
   * Receive() returns immediately.  If data is available then, it returns the
   * number of bytes read (<= maxbytes).  If data is not available
   * and the connection is non-blocking, then it returns 0 and GetLastError()
   * returns CS_NET_ERR_NO_ERROR.
   */
  virtual size_t Receive(void* buff, size_t maxbytes) = 0;

  /**
   * This version of Receive() is valid only for multicast connections.
   * It returns a 'from' parameter indicating the sender of the data.
   */
  virtual size_t Receive(void* buff, size_t maxbytes, csRef<iString>& from)=0;

  /**
   * This provides a lightweight alternative to performing brute-force polling
   * of Receive() to find out if any data is available.  This method will never
   * block.  You should call this function first to see if data is available,
   * and only call Receive() if true is returned.
   */
  virtual bool IsDataWaiting() const = 0;
};


SCF_VERSION (iNetworkListener, 0, 1, 1);

/**
 * This is a network listener interface.  It represents a single
 * network listening post.  All network listeners must implement this
 * interface.
 */
struct iNetworkListener : public iNetworkEndPoint
{
  /**
   * Accepts a connection request.  If the listener is configured to block,
   * the function does not return until a connection has been established or
   * an error has occurred.  If non-blocking, then it returns
   * immediately.  The return value is either an accepted connection or 0.
   * If the connection is non-blocking, 0 is returned, and GetLastError()
   * returns CS_NET_ERR_NO_ERROR then no connection was pending.  Otherwise
   * an error occurred, and GetLastError() returns the appropriate error code.
   */
  virtual csPtr<iNetworkConnection> Accept() = 0;
};


SCF_VERSION (iNetworkDriver, 0, 0, 2);

/**
 * This is a network driver interface.  It represents a plug-in
 * network driver module.  All network drivers must implement this interface.
 */
struct iNetworkDriver : public iBase
{
  /**
   * Create a new network connection.  The 'target' parameter is driver
   * dependent.
   *
   * For example, with a socket driver, the target might be
   * "host:port/protocol" (i.e. "server.game.net:666/tcp"); etc.  with a modem
   * driver it might be "device:phone-number" (i.e. "com1:555-1234"); The
   * cssocket driver, for instance, supports protcols "tcp", "udp", and
   * "multicast".
   *
   * The 'blocking' flag determines whether operations on the connection return
   * immediately in all cases or wait until the operation can be completed
   * successfully.
   *
   * \return the new connection object or 0 if the connection failed.
   *
   * \deprecated
   * The 'reliable' flag is deprecated.  This feature is now specified as part
   * of the target string.  (For instance, with the cssocket driver, a protocol
   * of "tcp" is reliable, whereas "udp" is not.)
   */
  virtual csPtr<iNetworkConnection> NewConnection(const char* target,
    bool reliable = true, bool blocking = false) = 0;

  /**
   * Create a new network listener.  The 'source' parameter is driver
   * dependent.
   *
   * For example, with a socket driver, the source might be "port/protocol"
   * (i.e. "666/tcp"); with a modem driver it might be "device" (i.e. "com1");
   * etc.  The cssocket driver, for instance, supports protcols "tcp", "udp",
   * and "multicast".
   * 
   * The 'blockingListener' flag determines whether or not the Accept() method
   * blocks while being called.  The 'blockingConnection' flag determines
   * whether or not methods in the resulting connection object block.
   * 
   * \return the new listener object or 0 if the operation failed.
   *
   * \deprecated
   * The 'reliable' flag is deprecated.  This feature is now specified as part
   * of the target string.  (For instance, with the cssocket driver, a protocol
   * of "tcp" is reliable, whereas "udp" is not.)
   */
  virtual csPtr<iNetworkListener> NewListener(const char* source,
    bool reliable = true, bool blockingListener = false,
    bool blockingConnection = false) = 0;

  /**
   * Get network driver capabilities.  This function returns information
   * describing the capabilities of the driver.
   *
   * \deprecated
   * This function is deprecated since the reliable flag is deprecated,
   * and all drivers should support blocking and non-blocking.
   */
  virtual csNetworkDriverCapabilities GetCapabilities() const = 0;

  /**
   *  Retrieve the code for the last error encountered.
   */
  virtual csNetworkDriverError GetLastError() const = 0;
};

#endif // __CS_INETWORK_DRIVER_H__
