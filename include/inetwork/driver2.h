/*
    ensocket plugin
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

#ifndef __CS_INETWORK_DRIVER2_H__
#define __CS_INETWORK_DRIVER2_H__

#if !defined(COMP_VC) && !defined(COMP_BC)
# warning Use of iNetworkDriver2 is deprecated. Use iNetworkDriver instead.
#endif

#include "csutil/scf.h"
struct iNetworkSocket2;

SCF_VERSION (iNetworkDriver2, 0, 0, 2);

/// Network driver error codes.
enum csNetworkDriverError
{
  /// No error.
  CS_NET_DRIVER_NOERROR,
  /// Cannot start driver.
  CS_NET_DRIVER_CANNOT_INIT,
  /// Cannot create new socket.
  CS_NET_DRIVER_CANNOT_CREATE_SOCKET,
  /// Unsupported socket type.
  CS_NET_DRIVER_UNSUPPORTED_SOCKET_TYPE
};

/// Network driver factory interface.
struct iNetworkDriver2 : public iBase
{
  /// Create a NetworkSocket of socket_type.
  virtual iNetworkSocket2 *CreateSocket (int socket_type) = 0;
  /// Return the last driver error.
  virtual int LastError() const = 0;
};

#endif // __CS_INETWORK_DRIVER2_H__
