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

#ifndef __INETMAN_H__
#define __INETMAN_H__

#include "cscom/com.h"
#include "csnetman/nmandefs.h"	        //@@@BAD

interface ISystem;

extern const GUID IID_INetworkManager;

/**
 * This is the network manager interface for CS.
 * All network drivers must implement this interface.
 * The standard implementation is csNetworkManagerNull.
 */
interface INetworkManager : public IUnknown
{
public:
  /// Open the network manager
  STDMETHOD (Open) () PURE;
  /// Close the network manager
  STDMETHOD (Close) () PURE;
};

extern const IID IID_INetworkManagerFactory;

interface INetworkManagerFactory : public IUnknown
{
  ///
  STDMETHOD (CreateInstance) (REFIID riid, ISystem * piSystem, void **ppv) PURE;
  /// Lock or unlock from memory.
  STDMETHOD (LockServer) (BOOL bLock) PURE;
};

#endif	//__INETMAN_H__
