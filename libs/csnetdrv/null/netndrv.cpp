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
#include "sysdef.h"
#include "cscom/com.h"
#include "csnetdrv/null/netndrv.h"
#include "csnetdrv/null/drvndefs.h"
#include "isystem.h"

//	Temporaly added to use "sleep"
//#include <winbase.h>

IMPLEMENT_UNKNOWN_NODELETE (csNetworkDriverNull)

BEGIN_INTERFACE_TABLE(csNetworkDriverNull)
  IMPLEMENTS_INTERFACE(INetworkDriver)
END_INTERFACE_TABLE()

csNetworkDriverNull::csNetworkDriverNull(ISystem* /*piSystem*/)
{
}

csNetworkDriverNull::~csNetworkDriverNull()
{
}

STDMETHODIMP csNetworkDriverNull::Open()
{
	return S_OK;
}

STDMETHODIMP csNetworkDriverNull::Close()
{
	return S_OK;
}

STDMETHODIMP csNetworkDriverNull::Connect(DWORD /*dwID*/, CS_NET_ADDRESS * /*lpNetAddress*/)
{
	return S_OK;
}

STDMETHODIMP csNetworkDriverNull::Disconnect(DWORD /*dwID*/)
{
	return S_OK;
}

STDMETHODIMP csNetworkDriverNull::Send(DWORD /*dwID*/, DWORD /*dwBytesToSend*/, char * /*lpDataBuffer*/)
{
	return S_OK;
}

STDMETHODIMP csNetworkDriverNull::Receive(DWORD /*dwID*/, DWORD * /*lpdwBytesToReceive*/ /* in/out */, char * /*lpDataBuffer*/)
{
	return S_OK;
}

STDMETHODIMP csNetworkDriverNull::SetListenState(DWORD /*dwID*/, CS_NET_LISTENPARAMS * /*lpCSListenParams*/)
{
	return S_OK;
}

STDMETHODIMP csNetworkDriverNull::Accept(DWORD /*dwLID*//*listening socket*/, DWORD * /*lpdwID*//*server socket*/, CS_NET_ADDRESS * /*lpCSNetAddress*//*out*/)
{
	return S_OK;
}

STDMETHODIMP csNetworkDriverNull::Spawn(DWORD * /*lpdwID*/ /*out*/, DWORD /*dwType*/)
{
	return S_OK;
}

STDMETHODIMP csNetworkDriverNull::Kill(DWORD /*dwID*/)
{
	return S_OK;
}

STDMETHODIMP csNetworkDriverNull::KillAll()
{
	return S_OK;
}

STDMETHODIMP csNetworkDriverNull::GetDriverCaps(CS_NET_DRIVERCAPS *lpCSNetDriverCaps)
{
	lpCSNetDriverCaps->bConnOriented = false;
	lpCSNetDriverCaps->bConnLess = false;
	lpCSNetDriverCaps->iMaxSockets = 0;
	return S_OK;
}

STDMETHODIMP csNetworkDriverNull::GetLastError()
{
	return 0;
}
