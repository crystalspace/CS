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

#ifndef __NETWORK_SOCKETS_H__
#define __NETWORK_SOCKETS_H__

// NETWORK.H
// csNetworkSockets class.

#include "csutil/scf.h"
#include "csnetdrv/sockets/drvsdefs.h"
#include "inetdrv.h"

extern const CLSID CLSID_SocketsNetworkDriver;

///
class csNetworkDriverSockets : public INetworkDriver
{
protected:
	bool SocketListening[CS_NET_MAX_SOCKETS];
	bool SocketConnected[CS_NET_MAX_SOCKETS];
	bool SocketInitialized[CS_NET_MAX_SOCKETS];

	CS_NET_SOCKET Socket[CS_NET_MAX_SOCKETS];

	bool SocksReady;

	DWORD dwLastError;

	HRESULT InitSocks();

	HRESULT ReleaseSocks();

	void SysPrintf(int mode, char* str, ...);

public:
	/// The System interface. 
	iSystem* m_piSystem;

	csNetworkDriverSockets(iSystem* piSystem);

	virtual ~csNetworkDriverSockets();

	STDMETHODIMP Open();

	STDMETHODIMP Close();

	STDMETHODIMP Connect(DWORD dwID, csNetworkAddress *lpNetAddress);

	STDMETHODIMP Disconnect(DWORD dwID);

	STDMETHODIMP Send(DWORD dwID, DWORD dwBytesToSend, char *lpDataBuffer);

	STDMETHODIMP Receive(DWORD dwID, DWORD *lpdwBytesToReceive /* in/out */, char *lpDataBuffer);

	STDMETHODIMP SetListenState(DWORD dwID, int iPort);

	STDMETHODIMP Accept(DWORD dwLID/*listening socket*/, DWORD *lpdwID/*server socket*/, csNetworkAddress *lpCSNetAddress/*out*/);

	STDMETHODIMP Spawn(DWORD *lpdwID /*out*/, DWORD dwType);

	STDMETHODIMP Kill(DWORD dwID);

	STDMETHODIMP KillAll();

	STDMETHODIMP GetDriverCaps(csNetworkCaps *lpCSNetDriverCaps);

	STDMETHODIMP GetLastError();

	DECLARE_IUNKNOWN()
	DECLARE_INTERFACE_TABLE(csNetworkDriverSockets)
};

///
class csNetworkDriverSocketsFactory : public INetworkDriverFactory
{
  ///
  STDMETHODIMP CreateInstance (REFIID riid, iSystem * piSystem, void **ppv);

  /// Lock or unlock from memory.
  STDMETHODIMP LockServer (COMBOOL bLock);

  DECLARE_IUNKNOWN ()
  DECLARE_INTERFACE_TABLE (csNetworkDriverSocketsFactory)
};

#endif	//__NETWORK_SOCKES_H__
