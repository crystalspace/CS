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

#include <stdarg.h>

#define SYSDEF_SOCKETS
#include "sysdef.h"
#include "csutil/scf.h"
#include "csnetdrv/sockets/netsdrv.h"
#include "csnetdrv/sockets/drvsdefs.h"

/*IMPLEMENT_FACTORY(csNetworkDriverSockets)

EXPORT_CLASS_TABLE(netsdrv)
  EXPORT_CLASS(csNetworkDriverSockets, "crystalspace.netdrv.socket", "TCP Socket Network Driver");
EXPORT_CLASS_TABLE_END

IMPLEMENT_IBASE(csNetworkDriverSockets)
  IMPLEMENTS_INTERFACE(iNetworkDriver)
IMPLEMENT_IBASE_END*/

csNetworkDriverSockets::csNetworkDriverSockets(iBase* iParent)
  : SocksReady(0), dwLastError(CS_NET_NO_ERROR), Sys(NULL)
{
  CONSTRUCT_IBASE(iParent);
}

bool csNetworkDriverSockets::Initialize(iSystem* iSys)
{
	Sys = iSys;

	for(short i = 0; i < CS_NET_MAX_SOCKETS; i++)
	{
		SocketListening[i] = false;
		SocketConnected[i] = false;
		SocketInitialized[i] = false;
	}
	return 1;
}

csNetworkDriverSockets::~csNetworkDriverSockets()
{
}

bool csNetworkDriverSockets::Open()
{
	Sys->Print(MSG_INITIALIZATION, "Network driver sockets: ");

  if(!InitSocks()) {
	  Sys->Print(MSG_INITIALIZATION, "FAILED\n");
	  return 0;
	}

  Sys->Print(MSG_INITIALIZATION, "OK\n");
	return 1;
}

bool csNetworkDriverSockets::Close()
{
	KillAll();

	if(SocksReady)
		return ReleaseSocks();
		
	return 1;
}

csNetHandle csNetworkDriverSockets::Connect(csNetworkAddress *iNetAddress)
{
	if(!SocksReady)	
	{
		dwLastError = CS_NET_NOT_INITIALIZED;
		return 0;
	}

	if(SocketConnected[csNetHandle])
	{
		dwLastError = CS_NET_ALREADY_CONNECTED;
		return 0;
	}

#if !defined(NO_SOCKETS_SUPPORT)

	struct sockaddr_in addr;
	struct hostent *hp;
	short len;

	addr.sin_family = AF_INET;
	if((hp = gethostbyname(lpNetAddress->hostnm)) == NULL)
	{
		dwLastError = CS_NET_CANNOT_RESOLVE_NAME;
		return 0;
	}
	memcpy((char*)&addr.sin_addr, (char*)hp->h_addr, hp->h_length);
	addr.sin_port = htons(lpNetAddress->port);
	len = sizeof(addr);
	if(connect(Socket[dwID], (struct sockaddr*)&addr, len) < 0)
	{
		dwLastError = CS_NET_CANNOT_CONNECT;
		return 0;
	}

#endif

	SocketConnected[dwID] = true;
	return 1;
}

void csNetworkDriverSockets::Disconnect(DWORD dwID)
{
	// This is not really a good thing going on here but i had no choice since BSD sockets cannot disconnect
	if(!SocksReady)
	{
		dwLastError = CS_NET_DRV_ERR_NOT_INITIALIZED;
		return S_FALSE;
	}
	if(!SocketConnected[dwID])
	{
		return S_OK;
	}

#if !defined(NO_SOCKETS_SUPPORT)

	char SockType;
	int ProtoType;
	socklen_t SizeOfSockType = sizeof(SockType);

	if(!getsockopt(Socket[dwID], SOL_SOCKET, SO_TYPE, &SockType, &SizeOfSockType))
	{
		dwLastError = CS_NET_DRV_ERR_CANNON_GET_SOCKOPT;
		return S_FALSE;
	}

	if(SockType == SOCK_STREAM) ProtoType = IPPROTO_TCP;
	else if(SockType == SOCK_DGRAM) ProtoType = IPPROTO_UDP;
	else
	{
		dwLastError = CS_NET_DRV_ERR_INVALID_TYPE;
		return S_FALSE;
	}

#endif

	if(Kill(dwID) == S_FALSE) return S_FALSE;

#if !defined(NO_SOCKETS_SUPPORT)

	Socket[dwID] = socket(AF_INET, SockType, ProtoType);

	if(Socket[dwID] == CS_NET_INVALID_SOCKET)
	{
		dwLastError = CS_NET_DRV_ERR_CANNOT_CREATE;
		return S_FALSE;
	}

#endif

	SocketInitialized[dwID] = true;

	return S_OK;
}

STDMETHODIMP csNetworkDriverSockets::Send(DWORD dwID, DWORD dwBytesToSend, char *lpDataBuffer)
{
	if (!SocksReady)
	{
		dwLastError = CS_NET_DRV_ERR_NOT_INITIALIZED;
		return S_FALSE;
	}
	if (!SocketConnected[dwID])
	{
		dwLastError = CS_NET_DRV_ERR_NOT_CONNECTED;
		return S_FALSE;
	}

#if !defined(NO_SOCKETS_SUPPORT)
	if(!send(Socket[dwID], lpDataBuffer, dwBytesToSend, 0)) return S_OK;
#else
	return S_OK;
#endif

	dwLastError = CS_NET_DRV_ERR_CANNOT_SEND;
	return S_FALSE;
}

STDMETHODIMP csNetworkDriverSockets::Receive(DWORD dwID, DWORD *lpdwBytesToReceive /* out */, char *lpDataBuffer)
{
	if (!SocksReady)
	{
		dwLastError = CS_NET_DRV_ERR_NOT_INITIALIZED;
		return S_FALSE;
	}
	if (!SocketConnected[dwID])
	{
		dwLastError = CS_NET_DRV_ERR_NOT_CONNECTED;
		return S_FALSE;
	}

#if !defined(NO_SOCKETS_SUPPORT)
	if((*lpdwBytesToReceive = recv(Socket[dwID], lpDataBuffer, sizeof(lpDataBuffer), 0)) != (DWORD)-1) return S_OK;
#else
	return S_OK;
#endif

	dwLastError = CS_NET_DRV_ERR_CANNOT_RECEIVE;
	return S_FALSE;
}

HRESULT csNetworkDriverSockets::InitSocks()
{
	if(SocksReady) return S_OK;

#if defined(OS_WIN32) && !defined(NO_SOCKETS_SUPPORT)

	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 0);
	
	if(WSAStartup(wVersionRequested, &wsaData) != 0)
	{
		dwLastError = CS_NET_DRV_ERR_CANNOT_GET_VERSION;
		return S_FALSE;
	}

	if((LOBYTE(wsaData.wVersion) != 2) || (HIBYTE(wsaData.wVersion) != 0))
	{
		WSACleanup();
		dwLastError = CS_NET_DRV_ERR_WRONG_VERSION;
		return S_FALSE;
	}

#endif

	SocksReady = true;
	return S_OK;
}

HRESULT csNetworkDriverSockets::ReleaseSocks()
{
	if(!SocksReady) return S_OK;

#if defined(OS_WIN32) && !defined(NO_SOCKETS_SUPPORT)
	if(WSACleanup())
	{
		dwLastError = CS_NET_DRV_ERR_CANNOT_CLEANUP;
		return S_FALSE;
	}
#endif

	SocksReady = false;

	return S_OK;
}

STDMETHODIMP csNetworkDriverSockets::SetListenState(DWORD dwID, int iPort)
{
	if(!SocksReady)
	{
		dwLastError = CS_NET_DRV_ERR_NOT_INITIALIZED;
		return S_FALSE;
	}
	if((dwID >= CS_NET_MAX_SOCKETS) || (!SocketInitialized[dwID]))
	{
		dwLastError = CS_NET_DRV_ERR_INVALID_SOCKET;
		return S_FALSE;
	}
	if(SocketConnected[dwID])
	{
		dwLastError = CS_NET_DRV_ERR_ALREADY_CONNECTED;
		return S_FALSE;
	}
	if(SocketListening[dwID])
	{
		dwLastError = CS_NET_DRV_ERR_ALREADY_LISTENING;
		return S_FALSE;
	}

#if !defined(NO_SOCKETS_SUPPORT)

	short LocalPort = lpCSListenParams->port;
	struct sockaddr_in LocalAddress;
	LocalAddress.sin_family = AF_INET;
	LocalAddress.sin_addr.s_addr = INADDR_ANY;
	LocalAddress.sin_port = htons(LocalPort);

	if(bind(Socket[dwID], (struct sockaddr*)&LocalAddress, sizeof(LocalAddress)) != 0)
	{
		dwLastError = CS_NET_DRV_ERR_CANNOT_BIND;
		return S_FALSE;
	}

	if(listen(Socket[dwID], CS_NET_LISTEN_QUEUE_SIZE) != 0)
	{
		dwLastError = CS_NET_DRV_ERR_CANNOT_LISTEN;
		return S_FALSE;
	}

#endif

	SocketListening[dwID] = true;

	return S_OK;

}

bool csNetworkDriverSockets::Spawn(DWORD dwType)
{
	if(!SocksReady)
	{
		dwLastError = CS_NET_DRV_ERR_NOT_INITIALIZED;
		return S_FALSE;
	}

	unsigned short i = 0;

	while(SocketInitialized[i])
	{
		i++;
		if(i >= CS_NET_MAX_SOCKETS)
		{
			dwLastError = CS_NET_DRV_ERR_LIMIT_REACHED;
			return S_FALSE;
		}
	}

#if !defined(NO_SOCKETS_SUPPORT)

	if(dwType == CS_NET_CONNORIENTED) Socket[i] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	else if(dwType == CS_NET_CONNLESS) Socket[i] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	else
	{
		dwLastError = CS_NET_DRV_ERR_INVALID_TYPE;
		return S_FALSE;
	}

	if(Socket[i] == CS_NET_INVALID_SOCKET)
	{
		dwLastError = CS_NET_DRV_ERR_CANNOT_CREATE;
		return S_FALSE;
	}

	unsigned long ttt = 1;

	#if defined(OS_WIN32)
	if(ioctlsocket(Socket[i], FIONBIO, &ttt))
	{
		dwLastError = CS_NET_DRV_ERR_CANNOT_SET_PARAMS;
		return S_FALSE;
	}
	#else
	if(ioctl(Socket[i], FIONBIO, &ttt))
	{
		dwLastError = CS_NET_DRV_ERR_CANNOT_SET_PARAMS;
		return S_FALSE;
	}
	#endif

#endif

	SocketInitialized[i] = true;
	SocketListening[i] = false;
	SocketConnected[i] = false;

	*lpdwID = i;

	return S_OK;
}

STDMETHODIMP csNetworkDriverSockets::Kill(DWORD dwID)
{
	if(!SocksReady)
	{
		dwLastError = CS_NET_DRV_ERR_NOT_INITIALIZED;
		return S_FALSE;
	}
	if(dwID >= CS_NET_MAX_SOCKETS)
	{
		dwLastError = CS_NET_DRV_ERR_INVALID_SOCKET;
		return S_FALSE;
	}
	if(!SocketInitialized[dwID]) return S_OK;

#if !defined(NO_SOCKETS_SUPPORT)
	#if (defined(OS_WIN32) || defined(OS_BE))
	if(closesocket(Socket[dwID]))
	{
		dwLastError = CS_NET_DRV_ERR_CANNOT_CLOSE;
		return S_FALSE;
	}
	#else
	if(close(Socket[dwID]))
	{
		dwLastError = CS_NET_DRV_ERR_CANNOT_CLOSE;
		return S_FALSE;
	}
	#endif
#endif

	SocketInitialized[dwID] = false;
	SocketConnected[dwID] = false;
	SocketListening[dwID] = false;

	return S_OK;
}

STDMETHODIMP csNetworkDriverSockets::KillAll()
{
	if(!SocksReady)
	{
		dwLastError = CS_NET_DRV_ERR_NOT_INITIALIZED;
		return S_FALSE;
	}

	DWORD i = 0;
	HRESULT hResult = S_OK;

	while(i < CS_NET_MAX_SOCKETS)
	{
		if(Kill(i) == S_FALSE) hResult = S_FALSE;
		i++;
	}

	return hResult;
}

STDMETHODIMP csNetworkDriverSockets::Accept(DWORD dwLID/*listening socket*/, DWORD *lpdwID/*server socket*/, csNetworkAddress *lpCSNetAddress/*out*/)
{
	(void) lpCSNetAddress; //Use that line to remove a warning.

	if(!SocksReady)
	{
		dwLastError = CS_NET_DRV_ERR_NOT_INITIALIZED;
		return S_FALSE;
	}
	if(dwLID >= CS_NET_MAX_SOCKETS)
	{
		dwLastError = CS_NET_DRV_ERR_INVALID_SOCKET;
		return S_FALSE;
	}
	if(!SocketListening[dwLID])
	{
		dwLastError = CS_NET_DRV_ERR_NOT_LISTENING;
		return S_FALSE;
	}

	*lpdwID = 0;

	while(SocketInitialized[*lpdwID])
	{
		(*lpdwID)++;
		if(*lpdwID >= CS_NET_MAX_SOCKETS)
		{
			*lpdwID = CS_NET_INVALID_SOCKET;
			dwLastError = CS_NET_DRV_ERR_LIMIT_REACHED;
			return S_FALSE;
		}
	}

#if !defined(NO_SOCKETS_SUPPORT)
	sockaddr VisitorAddress;
	socklen_t AddressLen = sizeof(sockaddr);

	Socket[*lpdwID] = accept(Socket[dwLID], &VisitorAddress, &AddressLen);

	if(Socket[*lpdwID] == CS_NET_INVALID_SOCKET)
	{
		dwLastError = CS_NET_DRV_ERR_CANNOT_ACCEPT;
		return S_FALSE;
	}

	unsigned long ttt = 1;

	#if defined(OS_WIN32)
	if(ioctlsocket(Socket[*lpdwID], FIONBIO, &ttt))
	{
		dwLastError = CS_NET_DRV_ERR_CANNOT_SET_PARAMS;
		return S_FALSE;
	}
	#else
	if(ioctl(Socket[*lpdwID], FIONBIO, &ttt))
	{
		dwLastError = CS_NET_DRV_ERR_CANNOT_SET_PARAMS;
		return S_FALSE;
	}
	#endif

#endif

	SocketConnected[*lpdwID] = true;
	SocketInitialized[*lpdwID] = true;
	return S_OK;
}

STDMETHODIMP csNetworkDriverSockets::GetDriverCaps(csNetworkCaps *lpCSNetDriverCaps)
{
	lpCSNetDriverCaps->bConnOriented = true;
	lpCSNetDriverCaps->bConnLess = true;
	lpCSNetDriverCaps->iMaxSockets = CS_NET_MAX_SOCKETS;
	return S_OK;
}

STDMETHODIMP csNetworkDriverSockets::GetLastError()
{
	DWORD retval = dwLastError;
	dwLastError = 0;
	return retval;
}
