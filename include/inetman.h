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

#ifndef __CS_INETMAN_H__
#define __CS_INETMAN_H__

#include "csutil/scf.h"
#include "iplugin.h"

SCF_VERSION (iNetworkManager, 0, 0, 1);

/**
 * This is the network manager interface for CS.  It represents a plug-in
 * network manager module.  All network managers must implement this interface.
 */
struct iNetworkManager : public iPlugIn
{
  /// Should be called every frame
  virtual void Refresh () = 0;

  /// Game management
  virtual bool CreateGame (int nplayers) = 0;
  virtual bool KillGame () = 0;
  virtual void JoinGame () = 0;
  virtual void LeaveGame () = 0;
  virtual void EnumerateGames () = 0;
  virtual void GetGameData () = 0;
  virtual void SetGameData () = 0;

  /// Player management
  virtual void CreatePlayer () = 0;
  virtual void KillPlayer () = 0;
  virtual void GetPlayerData () = 0;
  virtual void SetPlayerData () = 0;
  virtual void AddPlayerToGroup () = 0;
  virtual void RemovePlayerFromGroup () = 0;

  /// Group management
  virtual void CreateGroup () = 0;
  virtual void KillGroup () = 0;
  virtual void GetGroupData () = 0;
  virtual void SetGroupData () = 0;

  /// Message management
  virtual void Send () = 0;
  virtual void Receive () = 0;
  virtual void GetMessageCount () = 0;
  virtual void PeekMessage () = 0;

  /// Utility stuff
  virtual int GetLastError () = 0;
  virtual void GetCapabilities () = 0;

  // iPlugIn interface.
  virtual bool Initialize (iSystem*) = 0;
  virtual bool Open () = 0;
  virtual bool Close () = 0;
};

#endif // __CS_INETMAN_H__
