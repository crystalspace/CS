/*
    Copyright (C) 1999 by Eric Sunshine <sunshine@sunshineco.com>
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

#ifndef __CS_DRVSDEFS_H__
#define __CS_DRVSDEFS_H__

#define CS_NET_SOCKETS_MAX 64

#define CS_NET_LISTEN_QUEUE_SIZE 5

#if !defined(OS_WIN32)
typedef unsigned int csNetworkSocket;
#define CS_NET_SOCKET_INVALID ((csNetworkSocket)~0)
#else
typedef SOCKET csNetworkSocket;
#define CS_NET_SOCKET_INVALID INVALID_SOCKET
#define _WINSOCKAPI_
#endif

#endif // __CS_DRVSDEFS_H__
