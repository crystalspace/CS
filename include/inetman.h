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

#include "csutil/scf.h"

/**
 * This is the network manager interface for CS.
 * All network drivers must implement this interface.
 * The standard implementation is csNetworkManagerNull.
 */
SCF_INTERFACE (iNetworkManager, 0, 0, 1) : public iBase
{
  /// Open the network manager (should not be called by user)
  virtual bool Open () = 0;
  /// Close the network manager (should not be called by user)
  virtual void Close () = 0;

  /// Should be called every frame
  virtual void DoFrame () = 0;

  /// Game management
  virtual void CreateGame (unsigned short iNumPlayers) = 0;

  virtual void KillGame () = 0;

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
  virtual void GetLastError () = 0;

  virtual void GetCaps () = 0;
};

#endif	//__INETMAN_H__
