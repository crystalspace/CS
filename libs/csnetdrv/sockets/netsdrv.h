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

///
class csNetworkDriverSockets : public iNetworkDriver
{
protected:
	bool SocketListening[CS_NET_MAX_SOCKETS];
	bool SocketConnected[CS_NET_MAX_SOCKETS];
	bool SocketInitialized[CS_NET_MAX_SOCKETS];
  bool SocketInUse

	CS_NET_SOCKET Socket[CS_NET_MAX_SOCKETS];

	bool SocksReady;

	csNetworkError dwLastError;

	bool InitSocks();

	bool ReleaseSocks();

	bool Spawn(csNetHandle *oHandle, size_t iType);

public:
	/// The System interface. 
	iSystem* Sys;

	csNetworkDriverSockets(iBase* iParent);

	virtual ~csNetworkDriverSockets();

  bool Initialize(iSystem* iSys);

	bool Open();

	bool Close();

  csNetHandle Spawn(csNetworkCaps *caps);

	bool Connect(csNetworkAddress *lpNetAddress);

	void Disconnect(csNetHandle iHandle);

	void Send(csNetHandle iHandle, iString *iStr);

	void Receive(csNetHandle iHandle, iString *iStr);

	void SetListenState(csNetHandle iHandle, int iPort);

	void Accept(csNetHandle iListen, csNetHandle *iServer, csNetworkAddress *oAddress);

	void Kill(csNetHandle Handle);

	void KillAll();

	void GetDriverCaps(csNetworkCaps *lpCSNetDriverCaps);

	int GetLastError();

	DECLARE_IBASE;
};

#endif	//__NETWORK_SOCKES_H__
