/*
    ENSOCKET Plugin
    Copyright (C) 2002 by Erik Namtvedt

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

#ifndef __CS_INET_H__
#define __CS_INET_H__

#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "inetwork/driver2.h"
#include "inetwork/socket2.h"

#if !defined(OS_WIN32) || defined(__CYGWIN__)
#define SOCKET int
#else
#include <winsock.h>
#endif

class csNetworkSocket2 : public iNetworkSocket2
{
 public:
  SCF_DECLARE_IBASE;

  csNetworkSocket2 (iBase* parent, int socket_type, SOCKET fd = (SOCKET)-1);
  virtual ~csNetworkSocket2();

  virtual int LastError() const;
  virtual bool IsConnected() const;
  virtual int SetSocketBlock( bool block );
  virtual int SetSocketReuse( bool reuse );
  virtual int SetSocketBroadcast( bool broadcast);
  virtual int SetBroadcastOptions(int port, const char* addr);
  virtual int Connect( char const* host, int port );
  virtual int Send( char const* buff, size_t size );
  virtual int Recv( char* buff, size_t size );
  virtual int Close();
  virtual int Disconnect();
  virtual int WaitForConnection( int source, int port, int que );
  virtual iNetworkSocket2* Accept();

  virtual int ReadLine( char* buff, size_t size );
  virtual char const* RemoteName() const;

 private:
  SOCKET socketfd;
  int proto_type;
  int last_error;
  char* read_buffer;
  int buffer_size;
  int buffer_nread;
  bool socket_ready;
  bool connected;
  bool blocking;
  bool broadcasting;
  struct sockaddr_in local_addr;
  struct sockaddr_in remote_addr;
  struct sockaddr_in broadcast_addr;

  virtual int SELECT( int fds, fd_set* readfds, fd_set* writefds,
    fd_set* exceptfds );
  virtual int IOCTL( SOCKET socketfd, long cmd, u_long* argp );
};

class csNetworkDriver2 : public iNetworkDriver2
{
 friend class csNetworkSocket2;

 public:
  SCF_DECLARE_IBASE;

  csNetworkDriver2 (iBase* parent);
  virtual ~csNetworkDriver2();

  virtual iNetworkSocket2* CreateSocket (int socket_type);
  virtual int LastError() const;

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csNetworkDriver2);
    virtual bool Initialize (iObjectRegistry*) { return true; }
  } scfiComponent;

  private:
    int last_error;
};

#endif // __CS_INET_H__
