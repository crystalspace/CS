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

#ifndef __INETWORK_SOCKET_H__
#define __INETWORK_SOCKET_H__

#include "inetwork/driver.h"

//-----------------------------------------------------------------------------
// Files including this file must also #define SYSDEF_SOCKETS before
// including cssysdef.h
//-----------------------------------------------------------------------------

SCF_VERSION (iNetworkSocket, 0, 0, 1);

/**
 * This interface represents a network socket.
 */
struct iNetworkSocket: public iBase
{
  /// Retrieve the socket associated with this object.
  virtual csNetworkSocket GetSocket() const = 0;
};

#endif // __INETWORK_SOCKET_H__
