/*
	Copyright (C) 1998, 1999 by Serguei 'Snaar' Narojnyi
	Copyright (C) 1998, 1999 by Jorrit Tyberghein
	Written by Serguei 'Snaar' Narojnyi

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

#ifndef __DRVSDEFS_H__
#define __DRVSDEFS_H__

#include "inetdrv.h"

#define CS_NET_MAX_SOCKETS			64

#define CS_NET_LISTEN_QUEUE_SIZE	5

#if defined(OS_WIN32)
#define CS_NET_SOCKET				SOCKET
#define CS_NET_INVALID_SOCKET		INVALID_SOCKET
#define _WINSOCKAPI_
#else
#define CS_NET_SOCKET				unsigned int
#define CS_NET_INVALID_SOCKET		((CS_NET_SOCKET)~0)
#endif

#endif	//__DRVSDEFS_H__
