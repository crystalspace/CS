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

#ifndef _INETWORK_DRIVER2__
#define _INETWORK_DRIVER2__

#include "csutil/scf.h"

SCF_VERSION (iNetworkDriver2, 0, 0, 1);

enum csNetworkDriverError
{
  CS_NET_DRIVER_NOERROR,						// no errors
  CS_NET_DRIVER_CANNOT_INIT,					// cannot start driver
  CS_NET_DRIVER_CANNOT_STOP,					// cannot stop driver
  CS_NET_DRIVER_CANNOT_CREATE_SOCKET,			// cannot create new socket
  CS_NET_DRIVER_UNSUPPORTED_SOCKET_TYPE			// unsupported socket type
};

struct iNetworkSocket2;

struct iNetworkDriver2 : public iBase
{
  /// Create a NetworkSocket of socket_type
  virtual iNetworkSocket2 *CreateSocket (int socket_type) = 0;
  /// Return the last error the driver ran into
  virtual int LastError() = 0;
};

#endif
