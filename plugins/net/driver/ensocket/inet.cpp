/*
    ensocket plugin
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

#define CS_SYSDEF_PROVIDE_SELECT
#include "cssysdef.h"
#include "csutil/sockets.h"
#include "inet.h"
#include "inetwork/sockerr.h"

#if defined(OS_WIN32) && !defined(__CYGWIN__)
#define WINSOCK
#define CS_SOCKET2_ERROR SOCKET_ERROR
#define CS_SOCKET2_GET_LAST_ERROR WSAGetLastError()
#define CS_SOCKET2_EWOULDBLOCK WSAEWOULDBLOCK
#define CS_SOCKET2_IOCTL ioctlsocket
#else
#include <fcntl.h>
#define SOCKET int
#define closesocket close
#define CS_SOCKET2_ERROR (-1)
#define CS_SOCKET2_GET_LAST_ERROR errno
#define CS_SOCKET2_EWOULDBLOCK EWOULDBLOCK
#define CS_SOCKET2_IOCTL ioctl
#endif

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csNetworkDriver2)


SCF_IMPLEMENT_IBASE (csNetworkDriver2)
  SCF_IMPLEMENTS_INTERFACE (iNetworkDriver2)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csNetworkDriver2::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csNetworkSocket2)
  SCF_IMPLEMENTS_INTERFACE (iNetworkSocket2)
SCF_IMPLEMENT_IBASE_END

csNetworkDriver2::csNetworkDriver2 (iBase *parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  last_error = CS_NET_DRIVER_NOERROR;

#ifdef WINSOCK
  WSADATA Data;
  if (WSAStartup(MAKEWORD(1,0),&Data) != 0)
    last_error = CS_NET_DRIVER_CANNOT_INIT;
#endif
}

csNetworkDriver2::~csNetworkDriver2()
{
#ifdef WINSOCK
  WSACleanup();
#endif

  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE();
}

int csNetworkDriver2::LastError() const
{
  return last_error;
}

iNetworkSocket2 *csNetworkDriver2::CreateSocket (int socket_type)
{
  iNetworkSocket2* s = 0;
  if (socket_type == CS_NET_SOCKET_TYPE_TCP)
  {
    last_error = CS_NET_DRIVER_NOERROR;
    s = new csNetworkSocket2 (this, SOCK_STREAM);
  }
  else if (socket_type == CS_NET_SOCKET_TYPE_UDP)
  {
    last_error = CS_NET_DRIVER_NOERROR;
    s = new csNetworkSocket2 (this, SOCK_DGRAM);
  }
  else
    last_error = CS_NET_DRIVER_UNSUPPORTED_SOCKET_TYPE;
  return s;
}

csNetworkSocket2::csNetworkSocket2 (iBase *parent, int sock_type, SOCKET sock)
{
  SCF_CONSTRUCT_IBASE (parent);
  proto_type = sock_type;
  last_error = CS_NET_SOCKET_NOERROR;
  read_buffer = (char *)malloc(1);
  buffer_size = -1;
  buffer_nread = 0;
  connected = false;
  blocking = true;
  broadcasting = false;
  last_error = CS_NET_SOCKET_NOERROR;

  if (sock != (SOCKET)-1)
    socketfd = sock;
  else if (proto_type == SOCK_DGRAM)
    socketfd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
  else if (proto_type == SOCK_STREAM)
    socketfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  else
    last_error = CS_NET_SOCKET_UNSUPPORTED_SOCKET_TYPE;

  if (last_error == CS_NET_SOCKET_NOERROR &&
      socketfd == (SOCKET)CS_SOCKET2_ERROR)
    last_error = CS_NET_SOCKET_CANNOT_CREATE;

  socket_ready = (last_error == CS_NET_SOCKET_NOERROR);
}

csNetworkSocket2::~csNetworkSocket2 ()
{
  if (read_buffer)
    free(read_buffer);
  SCF_DESTRUCT_IBASE();
}

int csNetworkSocket2::LastError () const
{
  return last_error;
}

int csNetworkSocket2::Close()
{
  socket_ready = false;
  connected = false;
  closesocket(socketfd);
  last_error = CS_NET_SOCKET_NOERROR;
  return last_error;
}

int csNetworkSocket2::Disconnect()
{
  Close();
  return last_error;
}

int csNetworkSocket2::SetSocketBlock (bool block)
{
  if (socketfd)
  {
    blocking = block;
    unsigned long arg = (block ? 0 : 1);
    IOCTL(socketfd, FIONBIO, &arg);
    last_error = CS_NET_SOCKET_NOERROR;
  }
  else
    last_error = CS_NET_SOCKET_NOTCONNECTED;
  return last_error;
}

int csNetworkSocket2::SetSocketReuse (bool reuse)
{
  if (socketfd)
  {
    char const flag = (reuse ? 0x00 : 0xff);
    if (setsockopt(socketfd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag)) == 0)
      last_error = CS_NET_SOCKET_NOERROR;
    else
      last_error = CS_NET_SOCKET_CANNOT_SETREUSE;
  }
  else
    last_error = CS_NET_SOCKET_NOTCONNECTED;
  return last_error;
}

int csNetworkSocket2::SetSocketBroadcast (bool broadcast)
{
  if (socketfd)
  {
    if(proto_type != SOCK_DGRAM)
    {
      last_error = CS_NET_SOCKET_BROADCAST_ERROR;
    }
    else
    {
      broadcasting = broadcast;
      if (setsockopt(socketfd, SOL_SOCKET, SO_BROADCAST, (char *)&broadcast, sizeof(int)) == 0)
        last_error = CS_NET_SOCKET_NOERROR;
      else
        last_error = CS_NET_SOCKET_BROADCAST_ERROR;
    }
  }
  else
    last_error = CS_NET_SOCKET_BROADCAST_ERROR;
  return last_error;
}

int csNetworkSocket2::SetBroadcastOptions (int port, const char* addr)
{
  if (socketfd)
  {
    if(proto_type != SOCK_DGRAM)
    {
      last_error = CS_NET_SOCKET_BROADCAST_ERROR;
    }
    else
    {
      broadcast_addr.sin_family = AF_INET;
      broadcast_addr.sin_port = htons(port);
      if(addr)
        broadcast_addr.sin_addr.s_addr = inet_addr(addr);
      else
	broadcast_addr.sin_addr.s_addr = INADDR_BROADCAST;
      last_error = CS_NET_SOCKET_NOERROR;
    }
  }
  else
    last_error = CS_NET_SOCKET_BROADCAST_ERROR;
  return last_error;
}

int csNetworkSocket2::WaitForConnection (int source, int port, int que)
{
  local_addr.sin_family = AF_INET;
  local_addr.sin_port = htons(port);
  local_addr.sin_addr.s_addr = source;
  memset(&local_addr.sin_zero,0,8);

  last_error = CS_NET_SOCKET_NOERROR;

  if (bind(socketfd, (struct sockaddr*)&local_addr, sizeof(struct sockaddr)) ==
      CS_SOCKET2_ERROR)
    last_error = CS_NET_SOCKET_CANNOT_BIND;
  else if (proto_type != SOCK_DGRAM && listen(socketfd,que) == CS_SOCKET2_ERROR)
      last_error = CS_NET_SOCKET_CANNOT_LISTEN;

  return last_error;
}

int csNetworkSocket2::SELECT (int fds, fd_set *readfds, fd_set *writefds,
  fd_set *expectfds)
{
  struct timeval to;
  to.tv_sec = 0;
  to.tv_usec = 0;

  int const result = select(fds,readfds,writefds,expectfds,&to);
  if (result != CS_SOCKET2_ERROR)
    last_error = CS_NET_SOCKET_NOERROR;
  else if (CS_SOCKET2_GET_LAST_ERROR == CS_SOCKET2_EWOULDBLOCK)
    last_error = CS_NET_SOCKET_WOULDBLOCK;
  else
    last_error = CS_NET_SOCKET_CANNOT_SELECT;

  return result;
}

char const* csNetworkSocket2::RemoteName() const
{
  return inet_ntoa(local_addr.sin_addr);
}

int csNetworkSocket2::IOCTL (SOCKET socketfd, long cmd, u_long *argp)
{
  int result = CS_SOCKET2_IOCTL(socketfd, cmd, argp);
  if (result == CS_SOCKET2_ERROR)
    last_error = CS_NET_SOCKET_CANNOT_IOCTL;
  else
    last_error = CS_NET_SOCKET_NOERROR;
  return last_error;
}

iNetworkSocket2* csNetworkSocket2::Accept()
{
  if (proto_type == SOCK_DGRAM)
  {
    last_error = CS_NET_SOCKET_UNSUPPORTED_SOCKET_TYPE;
    return 0;
  }

  socklen_t sin_size = sizeof(struct sockaddr_in);
  SOCKET socket_fd = accept(socketfd,(struct sockaddr*)&remote_addr,&sin_size);
  if (socket_fd == (SOCKET)CS_SOCKET2_ERROR)
  {
    if (CS_SOCKET2_GET_LAST_ERROR == CS_SOCKET2_EWOULDBLOCK)
      last_error = CS_NET_SOCKET_WOULDBLOCK;
    else
      last_error = CS_NET_SOCKET_CANNOT_ACCEPT;
    return 0;
  }

  csNetworkSocket2 *tmpSocket =
    new csNetworkSocket2 (this, proto_type, socket_fd);
  tmpSocket->connected = true;
  memcpy(&tmpSocket->local_addr,&remote_addr,sizeof(struct sockaddr_in));
  tmpSocket->SetSocketBlock(blocking);

  last_error = CS_NET_SOCKET_NOERROR;
  return tmpSocket;
}

int csNetworkSocket2::Recv (char *buff, size_t size)
{
  int result = CS_SOCKET2_ERROR;
  last_error = CS_NET_SOCKET_NOERROR;

  if (!connected && proto_type == SOCK_STREAM)
    last_error = CS_NET_SOCKET_NOTCONNECTED;
  else // (connected || proto_type == SOCK_DGRAM)
  {
    if (proto_type == SOCK_STREAM)
      result = recv(socketfd, buff, size, 0);
    else
    {
      socklen_t addr_len = sizeof(struct sockaddr);
      result = recvfrom(socketfd, buff, size, 0, (struct sockaddr*)&local_addr,
        &addr_len);
    }

    if (result == CS_SOCKET2_ERROR)
    {
      if (CS_SOCKET2_GET_LAST_ERROR == CS_SOCKET2_EWOULDBLOCK)
        last_error = CS_NET_SOCKET_WOULDBLOCK;
      else
      {
        last_error = CS_NET_SOCKET_NOTCONNECTED;
        connected = false;
      }
    }
  }

  if (result >= 0 && result < (int)size)
    buff[result] = '\0';
  return result;
}

int csNetworkSocket2::ReadLine (char *buff, size_t size)
{
  int result = CS_SOCKET2_ERROR;
  last_error = CS_NET_SOCKET_NOERROR;

  if (!connected && proto_type == SOCK_STREAM)
    last_error = CS_NET_SOCKET_NOTCONNECTED;
  else // (connected || proto_type == SOCK_DGRAM)
  {
    char* p;
    if (blocking)
    {
      p = buff;
      buffer_nread = 0;
    }
    else
    {
      if (buffer_size < (int)size)
        read_buffer = (char*)realloc(read_buffer, size * sizeof(char));
      p = read_buffer + buffer_nread;
    }

    bool eol = false;
    while (!eol)
    {
      if (buffer_nread >= (int)size)
	eol = true;
      else
      {
        char c;
        if (Recv(&c,1) == 1)
        {
          if (c == '\r' || c == '\n' || c == '\0')
	    eol = true;
          else
	  {
            *p++ = c;
            buffer_nread++;
	  }
        }
        else
          break;
      }
    }

    if (blocking)
    {
      if (last_error != CS_NET_SOCKET_NOERROR)
        result = buffer_nread;
    }
    else
    {
      if (!eol && last_error == CS_NET_SOCKET_WOULDBLOCK)
        result = 0;
      else if (last_error != CS_NET_SOCKET_NOERROR)
      {
        memcpy(buff, read_buffer, buffer_nread);
        result = buffer_nread;
	buffer_nread = 0;
      }
      else
        buffer_nread = 0;
    }
  }

  if (result >= 0 && result < (int)size)
    buff[result] = '\0';
  return result;
}

int csNetworkSocket2::Send (char const* buff, size_t size)
{
  int result = CS_SOCKET2_ERROR;
  last_error = CS_NET_SOCKET_NOERROR;
  if (!connected && proto_type == SOCK_STREAM)
    last_error = CS_NET_SOCKET_NOTCONNECTED;
  else // (connected || proto_type == SOCK_DGRAM)
  {
    if (proto_type == SOCK_DGRAM)
    {																
      if(broadcasting)
       result = sendto(socketfd, buff, size, 0, (struct sockaddr*)&broadcast_addr,
          sizeof(struct sockaddr));
      else
      if(connected)
       result = sendto(socketfd, buff, size, 0, (struct sockaddr*)&remote_addr,
         sizeof(struct sockaddr));
      else 
       // in this case we send data to user that is not connected (if user sends broadcast data)
       result = sendto(socketfd, buff, size, 0, (struct sockaddr*)&local_addr,
          sizeof(struct sockaddr));
    }
    else
      result = send(socketfd, buff, size, 0);

    if (result == CS_SOCKET2_ERROR)
    {
      if (CS_SOCKET2_GET_LAST_ERROR == CS_SOCKET2_EWOULDBLOCK)
        last_error = CS_NET_SOCKET_WOULDBLOCK;
      else
      {
        last_error = CS_NET_SOCKET_NOTCONNECTED;
        connected = false;
      }
    }
  }
  return result;
}

int csNetworkSocket2::Connect (char const* host, int port)
{
  struct hostent const* host_ent = gethostbyname(CS_CONST_CAST(char*,host));
  if (host_ent == 0)
    last_error = CS_NET_SOCKET_CANNOT_RESOLVE;
  else
  {
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(port);
    remote_addr.sin_addr = *((struct in_addr *)host_ent->h_addr);
    memset(&remote_addr.sin_zero,0,8);

    if (connect(socketfd, (struct sockaddr*)&remote_addr,
      sizeof(struct sockaddr)) == CS_SOCKET2_ERROR)
      last_error = CS_NET_SOCKET_CANNOT_CONNECT;
    else
    {
      connected = true;
      last_error = CS_NET_SOCKET_NOERROR;
    }
  }
  return last_error;
}

bool csNetworkSocket2::IsConnected() const
{
  return connected;
}
