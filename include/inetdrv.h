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

#ifndef __INETWORK_H__
#define __INETWORK_H__

#include "cscom/com.h"
#include "csnetdrv/ndrvdefs.h" //@@@BAD  Why?

interface ISystem;

extern const GUID IID_INetworkDriver;

/**
 * This is the network interface for CS.
 * All network drivers must implement this interface.
 * The standard implementation is csNetworkDriverNull.
 */
interface INetworkDriver : public IUnknown
{
public:

  /// Open the network driver
  STDMETHOD (Open) () PURE;
  /// Close the network driver
  STDMETHOD (Close) () PURE;

  STDMETHOD (Connect) (DWORD dwID, CS_NET_ADDRESS *lpNetAddress) PURE;

  STDMETHOD (Disconnect) (DWORD dwID) PURE;

  STDMETHOD (Send) (DWORD dwID, DWORD dwBytesToSend, char *lpDataBuffer) PURE;

  STDMETHOD (Receive) (DWORD dwID, DWORD *lpdwBytesToReceive /* in/out */, char *lpDataBuffer) PURE;

  STDMETHOD (SetListenState) (DWORD dwID, CS_NET_LISTENPARAMS *lpCSListenParams) PURE;

  STDMETHOD (Accept) (DWORD dwLID/*listening socket*/, DWORD *lpdwID/*server socket*/, CS_NET_ADDRESS *lpCSNetAddress/*out*/) PURE;

  STDMETHOD (Spawn) (DWORD *lpdwID /*out*/, DWORD dwType) PURE;

  STDMETHOD (Kill) (DWORD dwID) PURE;

  STDMETHOD (KillAll) () PURE;

  STDMETHOD (GetDriverCaps) (CS_NET_DRIVERCAPS *lpCSNetDriverCaps) PURE;

  STDMETHOD (GetLastError) () PURE;

//STDMETHOD (SetOnReceiveFunction) (void *lpFunction) PURE;

//STDMETHOD (GetOnReceiveFunction) (void *lpFunction) PURE;
};

extern const IID IID_INetworkDriverFactory;

interface INetworkDriverFactory:
public IUnknown
{
  ///
  STDMETHOD (CreateInstance) (REFIID riid, ISystem * piSystem, void **ppv) PURE;

  /// Lock or unlock from memory.
  STDMETHOD (LockServer) (COMBOOL bLock) PURE;
};

#endif	//__INETWORK_H__
