
/*
    (C) 2002 Mat Sutcliffe <oktal@gmx.co.uk>

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

#ifndef __CS_INETWORK_NETMAN_H__
#define __CS_INETWORK_NETMAN_H__

#include "csutil/scf.h"

struct iNetworkSocket2;
class csDataStream;

SCF_VERSION (iNetworkPacket, 0, 0, 1);

/**
 * The application writes an implementation of the network packet
 * interface and a new instance of that implementation is passed
 * to the network manager with each new connection registered.
 * These packets are used as buffers for incoming data.
 * Other instances are passed to the transmission methods and
 * their data is sent to remote hosts.
 */
struct iNetworkPacket : public iBase
{
  /// Read in data received over the network.
  /// Returns true if the entire packet has been read,
  /// and a network event should be posted.
  /// Otherwise, further calls to Read() are neccessary.
  /// Resets length to the amount of data actually read.
  /// If not all the data was read, the remaining data can be
  /// assumed to be part of the next packet.
  /// Write-only packets may ignore this.
  virtual bool Read (csDataStream &data) = 0;

  /// Returns the packet as flat data which can be sent over the network.
  /// Sets length to the length of the buffer.
  /// Read-only packets may ignore this.
  virtual char* Write (size_t &length) = 0;

  /// Return a new instance of this implementation.
  /// Used for packets associated with listeners to create a new packet
  /// for each new connection created from the listener.
  virtual iNetworkPacket* New () = 0;

  /// Return true from this function if this packet should be sent over
  /// the given socket, false otherwise.
  /// Used by iNetworkManager::SendToAll() to filter out unwanted sockets.
  /// Read-only packets may ignore this.
  virtual bool FilterSocket (iNetworkSocket2 *) = 0;
};

SCF_VERSION (iNetworkManager, 0, 0, 1);

/**
 * The network manager polls for incoming data on connections registered
 * with it and new connections on listeners registered with it.
 * If data is waiting, it is added to the buffer of the packet associated
 * with the connection. Once a packet is full, a network event is posted
 * on the queue for the appliction to pick up.
 * The event type is csevNetwork and event.Command.Info is a pointer
 * to the packet where the data was stored.
 */
struct iNetworkManager : public iBase
{
  /// Register a connection for polling with its associated packet.
  virtual void RegisterConnectedSocket
    (iNetworkSocket2 *, iNetworkPacket *) = 0;

  /// Unregister a connection.
  virtual bool UnregisterConnectedSocket (iNetworkSocket2 *) = 0;

  /// Register a listener for polling with its associated packet.
  virtual void RegisterListeningSocket
    (iNetworkSocket2 *, iNetworkPacket *) = 0;

  /// Unregister a listener.
  virtual bool UnregisterListeningSocket (iNetworkSocket2 *) = 0;

  /// Send a packet on a connected socket.
  virtual bool Send (iNetworkSocket2 *, iNetworkPacket *) = 0;

  /// Send a packet to all registered, connected sockets
  /// that iNetworkPacket::FilterSocket() returns true for.
  virtual bool SendToAll (iNetworkPacket *) = 0;
};

#endif // __CS_INETWORK_NETMAN_H__

