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
#include "cscom/com.h"
#include "csnetdrv/sockets/netsdrv.h"
#include "csnetdrv/sockets/drvsdefs.h"
#include "isystem.h"

IMPLEMENT_UNKNOWN_NODELETE (csNetworkDriverSockets)

BEGIN_INTERFACE_TABLE (csNetworkDriverSockets)
  IMPLEMENTS_INTERFACE (INetworkDriver)
END_INTERFACE_TABLE ()

csNetworkDriverSockets::csNetworkDriverSockets(ISystem* piSystem)
{
	m_piSystem = piSystem;
	SocksReady = false;
	dwLastError = 0;

	for(int i = 0; i < CS_NET_MAX_SOCKETS; i++)
	{
		SocketListening[CS_NET_MAX_SOCKETS] = false;
		SocketConnected[CS_NET_MAX_SOCKETS] = false;
		SocketInitialized[CS_NET_MAX_SOCKETS] = false;
	}
}

csNetworkDriverSockets::~csNetworkDriverSockets()
{
}

void csNetworkDriverSockets::SysPrintf(int mode, char* szMsg, ...)
{
	char buf[1024];
	va_list arg;

	va_start(arg, szMsg);
	vsprintf(buf, szMsg, arg);
	va_end(arg);

	m_piSystem->Print(mode, buf);
}

STDMETHODIMP csNetworkDriverSockets::Open()
{
	SysPrintf(MSG_INITIALIZATION, "\nNetwork driver stuff:\n");

	if (InitSocks() == CS_NET_SUCCESS) SysPrintf(MSG_INITIALIZATION, "Network driver initialisation finished\n");
	else SysPrintf(MSG_INITIALIZATION, "Network driver initialisation failed!\n");

	SysPrintf(MSG_INITIALIZATION, "\n");

	return CS_NET_SUCCESS;
}

STDMETHODIMP csNetworkDriverSockets::Close()
{
	KillAll();

	if(SocksReady)
	{
		if(ReleaseSocks() != CS_NET_SUCCESS) return CS_NET_FAIL;
	}
	return S_OK;
}

STDMETHODIMP csNetworkDriverSockets::Connect(DWORD dwID, CS_NET_ADDRESS *lpNetAddress)
{
	if(!SocksReady)
	{
		dwLastError = CS_NET_ERR_NOT_INITIALIZED;
		return CS_NET_FAIL;
	}

	if(SocketConnected[dwID])
	{
		dwLastError = CS_NET_ERR_ALREADY_CONNECTED;
		return CS_NET_FAIL;
	}

#if !defined(NO_SOCKETS_SUPPORT)

	struct sockaddr_in addr;
	struct hostent *hp;
	int len;

	addr.sin_family = AF_INET;
	if((hp = gethostbyname(lpNetAddress->hostnm)) == NULL)
	{
		dwLastError = CS_NET_ERR_CANNOT_RESOLVE_NAME;
		return S_FALSE;
	}
	memcpy((char*)&addr.sin_addr, (char*)hp->h_addr, hp->h_length);
	addr.sin_port = htons(lpNetAddress->port);
	len = sizeof(addr);
	if(connect(Socket[dwID], (struct sockaddr*)&addr, len) < 0)
	{
		dwLastError = CS_NET_ERR_CANNOT_CONNECT;
		return CS_NET_FAIL;
	}

#endif

	SocketConnected[dwID] = true;
	return CS_NET_SUCCESS;
}

STDMETHODIMP csNetworkDriverSockets::Disconnect(DWORD dwID)
{
	// This is not really a good thing going on here but i had no choice since BSD sockets cannot disconnect
	if(!SocksReady)
	{
		dwLastError = CS_NET_ERR_NOT_INITIALIZED;
		return CS_NET_FAIL;
	}
	if (!SocketConnected[dwID]) return S_FALSE;
	{
		return CS_NET_SUCCESS;
	}

#if !defined(NO_SOCKETS_SUPPORT)

	char SockType;
	int ProtoType;
	int SizeOfSockType = sizeof(SockType);

	if(!getsockopt(Socket[dwID], SOL_SOCKET, SO_TYPE, &SockType, &SizeOfSockType))
	{
		dwLastError = CS_NET_ERR_CANNON_GET_SOCKOPT;
		return CS_NET_FAIL;
	}

	if(SockType == SOCK_STREAM) ProtoType = IPPROTO_TCP;
	else if(SockType == SOCK_DGRAM) ProtoType = IPPROTO_UDP;
	else
	{
		dwLastError = CS_NET_ERR_INVALID_TYPE;
		return CS_NET_FAIL;
	}

#endif

	if(Kill(dwID) == CS_NET_FAIL) return CS_NET_FAIL;

#if !defined(NO_SOCKETS_SUPPORT)

	Socket[dwID] = socket(AF_INET, SockType, ProtoType);

	if(Socket[dwID] == CS_NET_INVALID_SOCKET)
	{
		dwLastError = CS_NET_ERR_CANNOT_CREATE;
		return CS_NET_FAIL;
	}

#endif

	SocketInitialized[dwID] = true;

	return CS_NET_SUCCESS;
}

STDMETHODIMP csNetworkDriverSockets::Send(DWORD dwID, DWORD dwBytesToSend, char *lpDataBuffer)
{
	if (!SocksReady)
	{
		dwLastError = CS_NET_ERR_NOT_INITIALIZED;
		return CS_NET_FAIL;
	}
	if (!SocketConnected[dwID]) return S_FALSE;
	{
		dwLastError = CS_NET_ERR_NOT_CONNECTED;
		return CS_NET_FAIL;
	}

#if !defined(NO_SOCKETS_SUPPORT)
	if(!send(Socket[dwID], lpDataBuffer, dwBytesToSend, 0)) return S_OK;
#else
	return S_OK;
#endif

	dwLastError = CS_NET_ERR_CANNOT_SEND;
	return S_FALSE;
}

STDMETHODIMP csNetworkDriverSockets::Receive(DWORD dwID, DWORD *lpdwBytesToReceive /* out */, char *lpDataBuffer)
{
	if (!SocksReady)
	{
		dwLastError = CS_NET_ERR_NOT_INITIALIZED;
		return CS_NET_FAIL;
	}
	if (!SocketConnected[dwID]) return S_FALSE;
	{
		dwLastError = CS_NET_ERR_NOT_CONNECTED;
		return CS_NET_FAIL;
	}

#if !defined(NO_SOCKETS_SUPPORT)
	if(!(*lpdwBytesToReceive = recv(Socket[dwID], lpDataBuffer, sizeof(lpDataBuffer), 0))) return S_OK;
#else
	return S_OK;
#endif

	dwLastError = CS_NET_ERR_CANNOT_SEND;
	return S_FALSE;
}

HRESULT csNetworkDriverSockets::InitSocks()
{
	if(SocksReady) return CS_NET_SUCCESS;

#if defined(OS_WIN32) && !defined(NO_SOCKETS_SUPPORT)

	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 0);
	
	if(WSAStartup(wVersionRequested, &wsaData) != 0)
	{
		dwLastError = CS_NET_ERR_CANNOT_GET_VERSION;
		return CS_NET_FAIL;
	}

	if((LOBYTE(wsaData.wVersion) != 1) || (HIBYTE(wsaData.wVersion) != 1))
	{
		WSACleanup();
		dwLastError = CS_NET_ERR_WRONG_VERSION;
		return CS_NET_FAIL;
	}

#endif

	SocksReady = true;
	return CS_NET_SUCCESS;
}

HRESULT csNetworkDriverSockets::ReleaseSocks()
{
	if(!SocksReady) return CS_NET_SUCCESS;

#if defined(OS_WIN32) && !defined(NO_SOCKETS_SUPPORT)
	if(WSACleanup())
	{
		dwLastError = CS_NET_ERR_CANNOT_CLEANUP;
		return CS_NET_FAIL;
	}
#endif

	SocksReady = false;

	return CS_NET_SUCCESS;
}

STDMETHODIMP csNetworkDriverSockets::SetListenState(DWORD dwID, CS_NET_LISTENPARAMS *lpCSListenParams)
{
	if(!SocksReady)
	{
		dwLastError = CS_NET_ERR_NOT_INITIALIZED;
		return CS_NET_FAIL;
	}
	if((dwID >= CS_NET_MAX_SOCKETS) || (!SocketInitialized[dwID]))
	{
		dwLastError = CS_NET_ERR_INVALID_SOCKET;
		return CS_NET_FAIL;
	}
	if(SocketConnected[dwID])
	{
		dwLastError = CS_NET_ERR_ALREADY_CONNECTED;
		return CS_NET_FAIL;
	}
	if(SocketListening[dwID])
	{
		dwLastError = CS_NET_ERR_ALREADY_LISTENING;
		return CS_NET_FAIL;
	}

#if !defined(NO_SOCKETS_SUPPORT)

	int LocalPort = lpCSListenParams->port;
	struct sockaddr_in LocalAddress;
	LocalAddress.sin_family = AF_INET;
	LocalAddress.sin_addr.s_addr = INADDR_ANY;
	LocalAddress.sin_port = htons(LocalPort);

	if(!bind(Socket[dwID], (struct sockaddr*)&LocalAddress, sizeof(LocalAddress)))
	{
		dwLastError = CS_NET_ERR_CANNOT_BIND;
		return CS_NET_FAIL;
	}

	if(!listen(Socket[dwID], CS_NET_LISTEN_QUEUE_SIZE))
	{
		dwLastError = CS_NET_ERR_CANNOT_LISTEN;
		return CS_NET_FAIL;
	}

#endif

	SocketListening[dwID] = true;

	return CS_NET_SUCCESS;

}

STDMETHODIMP csNetworkDriverSockets::Spawn(DWORD *lpdwID /*out*/, DWORD dwType)
{
	if(!SocksReady)
	{
		dwLastError = CS_NET_ERR_NOT_INITIALIZED;
		return CS_NET_FAIL;
	}

	int i = 0;

	while(SocketInitialized[i])
	{
		i++;
		if(i >= CS_NET_MAX_SOCKETS)
		{
			dwLastError = CS_NET_ERR_LIMIT_REACHED;
			return CS_NET_FAIL;
		}
	}

#if !defined(NO_SOCKETS_SUPPORT)

	if(dwType == CS_NET_CONNORIENTED) Socket[i] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	else if(dwType == CS_NET_CONNLESS) Socket[i] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	else
	{
		dwLastError = CS_NET_ERR_INVALID_TYPE;
		return CS_NET_FAIL;
	}

	if(Socket[i] == CS_NET_INVALID_SOCKET)
	{
		dwLastError = CS_NET_ERR_CANNOT_CREATE;
		return CS_NET_FAIL;
	}

#endif

	SocketInitialized[i] = true;

	*lpdwID = i;

	return CS_NET_SUCCESS;
}

STDMETHODIMP csNetworkDriverSockets::Kill(DWORD dwID)
{
	if(!SocksReady)
	{
		dwLastError = CS_NET_ERR_NOT_INITIALIZED;
		return CS_NET_FAIL;
	}
	if(dwID >= CS_NET_MAX_SOCKETS)
	{
		dwLastError = CS_NET_ERR_INVALID_SOCKET;
		return CS_NET_FAIL;
	}
	if(!SocketInitialized[dwID]) return CS_NET_SUCCESS;

#if !defined(NO_SOCKETS_SUPPORT)
	#if defined(OS_WIN32)
	if(closesocket(Socket[dwID]))
	{
		dwLastError = CS_NET_ERR_CANNOT_CLOSE;
		return CS_NET_FAIL;
	}
	#elif defined(OS_OS2)
	if(soclose(Socket[dwID]))
	{
		dwLastError = CS_NET_ERR_CANNOT_CLOSE;
		return CS_NET_FAIL;
	}
	#else
	if(close(Socket[dwID]))
	{
		dwLastError = CS_NET_ERR_CANNOT_CLOSE;
		return CS_NET_FAIL;
	}
	#endif
#endif

	SocketInitialized[dwID] = false;
	SocketConnected[dwID] = false;
	SocketListening[dwID] = false;

	return CS_NET_SUCCESS;
}

STDMETHODIMP csNetworkDriverSockets::KillAll()
{
	if(!SocksReady)
	{
		dwLastError = CS_NET_ERR_NOT_INITIALIZED;
		return CS_NET_FAIL;
	}

	DWORD i = 0;
	HRESULT hResult = CS_NET_SUCCESS;

	while(i < CS_NET_MAX_SOCKETS)
	{
		if(Kill(i) == CS_NET_FAIL) hResult = CS_NET_FAIL;
		i++;
	}

	return hResult;
}

STDMETHODIMP csNetworkDriverSockets::Accept(DWORD dwLID/*listening socket*/, DWORD *lpdwID/*server socket*/, CS_NET_ADDRESS *lpCSNetAddress/*out*/)
{
	if(!SocksReady)
	{
		dwLastError = CS_NET_ERR_NOT_INITIALIZED;
		return CS_NET_FAIL;
	}
	if(dwLID >= CS_NET_MAX_SOCKETS)
	{
		dwLastError = CS_NET_ERR_INVALID_SOCKET;
		return CS_NET_FAIL;
	}
	if(!SocketListening[dwLID])
	{
		dwLastError = CS_NET_ERR_NOT_LISTENING;
		return CS_NET_FAIL;
	}

	*lpdwID = 0;

	while(SocketInitialized[*lpdwID])
	{
		(*lpdwID)++;
		if(*lpdwID >= CS_NET_MAX_SOCKETS)
		{
			*lpdwID = CS_NET_INVALID_SOCKET;
			dwLastError = CS_NET_ERR_LIMIT_REACHED;
			return CS_NET_FAIL;
		}
	}

#if !defined(NO_SOCKETS_SUPPORT)
	sockaddr VisitorAddress;
	int AddressLen;

	Socket[*lpdwID] = accept(Socket[dwLID], &VisitorAddress, &AddressLen);
	if(Socket[*lpdwID] == CS_NET_INVALID_SOCKET)
	{
		dwLastError = CS_NET_ERR_CANNOT_ACCEPT;
		return CS_NET_FAIL;
	}
#endif

	SocketConnected[*lpdwID] = true;
	return CS_NET_SUCCESS;
}

STDMETHODIMP csNetworkDriverSockets::GetDriverCaps(CS_NET_DRIVERCAPS *lpCSNetDriverCaps)
{
	lpCSNetDriverCaps->bConnOriented = true;
	lpCSNetDriverCaps->bConnLess = true;
	lpCSNetDriverCaps->iMaxSockets = CS_NET_MAX_SOCKETS;
	return CS_NET_SUCCESS;
}

STDMETHODIMP csNetworkDriverSockets::GetLastError()
{
	DWORD retval = dwLastError;
	dwLastError = 0;
	return retval;
}
