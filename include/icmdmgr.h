/*
    Copyright (C) 2000 by Thoams Riemer <triemer@apt4g.a3nyc.com>

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

#ifndef __ICMDMGR_H__
#define __ICMDMGR_H__

#include "csutil/scf.h"
#include "isystem.h"
#include "iplugin.h"

typedef struct 
{
  int Cmd;
  int NetPort;
} csNetCmd;

SCF_VERSION (iCmdMgr, 0, 0, 1);

struct iCmdMgr: public iBase
{
  public:
    virtual int GetStatus () =0;
    virtual void SetStatus(int status) =0;
};


SCF_VERSION (iCMDMGR, 0, 0, 2);

struct iCMDMGR: public iPlugIn
{
  virtual bool Initialize (iSystem *iSys) = 0;
  virtual bool Open() =0;
  virtual bool Close() = 0;
  virtual void ReceiveCmd(csNetCmd *Cmd) = 0;
  virtual void SendCmd(csNetCmd *Cmd) = 0;
  /// Debugging routine to print out the command.
  virtual void DisplayCmd(csNetCmd *Cmd) = 0;

  /// Start a server going.
  virtual int InitiateServer(char *hostname,
			     int ipPortNumber, 
			     int ServerType, 
			     int MaxConnections) = 0;

  /// Start a Client going.
  virtual int InitiateClient(char *HostName,int ipPortNumber, int ClientType) = 0;
};


#endif 



