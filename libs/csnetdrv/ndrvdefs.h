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

#ifndef __NETDEFS_H__
#define __NETDEFS_H__

#define CS_NET_SUCCESS	S_OK
#define CS_NET_FAIL		S_FALSE

#define CS_NET_CONNORIENTED	0
#define CS_NET_CONNLESS		1

enum CS_NET_ERR
{
	CS_NET_ERR_NOT_INITIALIZED = 0,
	CS_NET_ERR_ALREADY_CONNECTED,
	CS_NET_ERR_CANNOT_RESOLVE_NAME,
	CS_NET_ERR_CANNOT_CONNECT,
	CS_NET_ERR_NOT_CONNECTED,
	CS_NET_ERR_CANNOT_SEND,
	CS_NET_ERR_CANNOT_GET_VERSION,
	CS_NET_ERR_WRONG_VERSION,
	CS_NET_ERR_CANNOT_CLEANUP,
	CS_NET_ERR_INVALID_SOCKET,
	CS_NET_ERR_ALREADY_LISTENING,
	CS_NET_ERR_CANNOT_BIND,
	CS_NET_ERR_CANNOT_LISTEN,
	CS_NET_ERR_LIMIT_REACHED,
	CS_NET_ERR_INVALID_TYPE,
	CS_NET_ERR_CANNOT_CREATE,
	CS_NET_ERR_CANNOT_CLOSE,
	CS_NET_ERR_NOT_LISTENING,
	CS_NET_ERR_CANNON_GET_SOCKOPT,
	CS_NET_ERR_CANNOT_ACCEPT
};

typedef struct _CS_NET_ADDRESS
{
	char hostnm[512];
	int port;
} CS_NET_ADDRESS, *LPCS_NET_ADDRESS;

typedef struct _CS_NET_LISTENPARAMS
{
	char reserved[128];
	int port;
} CS_NET_LISTENPARAMS, *LPCS_NET_LISTENPARAMS;

typedef struct _CS_NET_DRIVERCAPS
{
	bool bConnOriented;
	bool bConnLess;
	unsigned int iMaxSockets;
} CS_NET_DRIVERCAPS, *LPCS_NET_DRIVERCAPS;

#endif	//__NETDEFS_H__

