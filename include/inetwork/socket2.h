/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

#ifndef __INETWORK_SOCKET2__
#define __INETWORK_SOCKET2__

#include "csutil/scf.h"

SCF_VERSION (iNetworkSocket2, 0, 0, 1);

#ifndef OS_WIN32
        #ifdef OS_UNIX
        #endif 

        #define SOCKET int
        #define closesocket close
        #define socket_error -1
#else
#endif

struct iNetworkSocket2 : public iBase
{
  virtual bool IsConnected () = 0;
  virtual int LastError() = 0;
  virtual int SetSocketBlock (bool block) = 0;
  virtual int SetSocketReuse (bool reuse) = 0;
  virtual int Connect (char *host, int port) = 0; // connect to remote host
  virtual int Send (char *buff, size_t size) = 0; // send data
  virtual int Recv (char *buff, size_t size) = 0; // recv data
  virtual int Close () = 0; // close the socket
  virtual int Disconnect () = 0; // disconnect/close
  virtual int WaitForConnection (int source, int port, int que) = 0;
  virtual iNetworkSocket2 *Accept () = 0; // accept the incoming connection
  virtual int set (SOCKET socket_fd, bool bConnected, struct sockaddr_in saddr) = 0;
  virtual int ReadLine (char *buff, size_t size) = 0;
  virtual char *RemoteName () = 0;
};

#endif
