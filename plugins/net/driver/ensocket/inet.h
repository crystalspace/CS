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

#ifndef _INET_
#define _INET_

#ifndef OS_WIN32
#define SOCKET int
#define closesocket close
#define socket_error -1 
#else
#include <winsock.h>
#define socket_error SOCKET_ERROR
#endif

#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "inetwork/driver2.h"
#include "inetwork/socket2.h"

class csNetworkSocket2;
class csNetworkDriver2;
class csNetworkSocket2 : public iNetworkSocket2
{
 public:
  SCF_DECLARE_IBASE;

  csNetworkSocket2 (iBase *parent, int socket_type);	// constructor
  virtual ~csNetworkSocket2();	// distructor
	
  virtual int LastError();															
  virtual int LastOSError();														
  virtual bool IsConnected();														
  virtual int SetSocketBlock( bool block );										
  virtual int SetSocketReuse( bool reuse );											
  virtual int Connect( char *host, int port );										
  virtual int Send( char *buff, size_t size );									
  virtual int Recv( char *buff, size_t size );										
  virtual int Close();															
  virtual int Disconnect();														
  virtual int WaitForConnection( int source, int port, int que );					
  virtual iNetworkSocket2 *Accept();												
  
  virtual int ReadLine( char *buff, size_t size );									
  virtual char *RemoteName();														
  
 private:
  SOCKET socketfd;	// socket descriptor
  int proto_type;
  int last_error;
  char *read_buffer;
  int buffer_size;
  bool socket_ready;
  bool connected;
  bool blocking;
  fd_set fd_mask;
  SOCKET fd_list[1];
  struct sockaddr_in local_addr;
  struct sockaddr_in remote_addr;
  struct hostent *host_ent;
  socklen_t sin_size;
  socklen_t addr_len;

  virtual int set( SOCKET socket_fd, bool bConnected, struct sockaddr_in saddr );
  virtual int SELECT( int fds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds );
  virtual int IOCTL( SOCKET socketfd, long cmd, u_long *argp );
};

class csNetworkDriver2 : public iNetworkDriver2
{
 friend class csNetworkSocket2;

 public:
  csNetworkDriver2 (iBase *parent);	// constructor
  virtual ~csNetworkDriver2();	// distructor

 public:
  SCF_DECLARE_IBASE;
  virtual iNetworkSocket2 *CreateSocket (int socket_type);						
  virtual int LastError();														

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csNetworkDriver2);
    virtual bool Initialize (iObjectRegistry *)
    {
      return true;
    }
  } scfiComponent;

  private:
    int last_error;
};

#endif // _INET_
