/*****************************************************************************
  
  This code is provided as-is by Erik Namtvedt.
  Help with the CEL interface was provided by May-Day.
  Help with Win32 projects was provided by Philip Wyett.
  Help with CS in general was provided by Jorrit T.

*****************************************************************************/


#define CS_SYSDEF_PROVIDE_SOCKETS
#include "cssysdef.h"

#ifdef OS_WIN32
#include <winsock.h>
#else
#include <fcntl.h>
#endif

#include "inet.h"
#include "inetwork/socketerr.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csNetworkDriver2)
  SCF_IMPLEMENTS_INTERFACE (iNetworkDriver2)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csNetworkDriver2::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csNetworkDriver2::csNetworkDriver2 (iBase *parent) 
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);

  last_error = CS_NET_DRIVER_NOERROR;

#ifdef OS_WIN32
  WSADATA Data;
  
  if (WSAStartup(MAKEWORD(1,0),&Data) != 0) 
    last_error = CS_NET_DRIVER_NOERROR;
#endif

}


csNetworkDriver2::~csNetworkDriver2() 
{

  last_error = CS_NET_DRIVER_NOERROR;

#ifdef OS_WIN32
  if (WSACleanup() != 0)
    last_error = CS_NET_DRIVER_CANNOT_STOP;
#endif

}


int csNetworkDriver2::LastError()
{

  return last_error;

}


iNetworkSocket2 *csNetworkDriver2::CreateSocket (int socket_type) 
{

  int proto_type = -1;

  last_error = CS_NET_DRIVER_UNSUPPORTED_SOCKET_TYPE;
 
  if (socket_type == CS_NET_SOCKET_TYPE_TCP) 
  {
    last_error = CS_NET_DRIVER_NOERROR;
    proto_type = SOCK_STREAM;
 
    iNetworkSocket2 *tmpSocket = new csNetworkSocket2 (this, proto_type);
    return tmpSocket;
  }
  if (socket_type == CS_NET_SOCKET_TYPE_UDP)
  {
    last_error = CS_NET_DRIVER_NOERROR;
    proto_type = SOCK_DGRAM;

    iNetworkSocket2 *tmpSocket = new csNetworkSocket2 (this, proto_type);
    return tmpSocket;
  }
  return 0;
}

SCF_IMPLEMENT_IBASE (csNetworkSocket2)
  SCF_IMPLEMENTS_INTERFACE (iNetworkSocket2)
SCF_IMPLEMENT_IBASE_END


csNetworkSocket2::csNetworkSocket2 (iBase *parent, int socket_type) 
{
  SCF_CONSTRUCT_IBASE (parent);
  proto_type = -1;
  last_error = CS_NET_SOCKET_NOERROR;
  buffer_size = -1;
  read_buffer = (char *)malloc(1);
 
  if (socket_type == SOCK_DGRAM)
  {
    socketfd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if (socketfd != socket_error) 
    {
	proto_type = SOCK_DGRAM;
    }
    else
    {
      last_error = CS_NET_SOCKET_CANNOT_CREATE;	
    }
  } 
  if (socket_type == SOCK_STREAM) 
  {
    socketfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (socketfd != socket_error) 
    {
        proto_type = SOCK_STREAM;
    } 
    else
    {
      last_error = CS_NET_SOCKET_CANNOT_CREATE;
    }
  }
  
  socket_ready = true;
  connected = false;
  blocking = true;
  
  fd_list[0] = socketfd;
  
  FD_ZERO(&fd_mask);
  FD_SET(fd_list[0],&fd_mask);

  if (proto_type == -1)
    last_error = CS_NET_SOCKET_UNSUPPORTED_SOCKET_TYPE; 

}

csNetworkSocket2::~csNetworkSocket2 ()
{
  if (read_buffer) free(read_buffer);
  
  last_error = CS_NET_SOCKET_NOERROR;

}

int csNetworkSocket2::LastError ()
{
  return last_error;
}

int csNetworkSocket2::LastOSError ()
{

#ifdef OS_WIN32
	return GetLastError();
#else
	return errno;
#endif

}

int csNetworkSocket2::Close() 
{
  connected = false;
  FD_ZERO(&fd_mask);
  closesocket(socketfd);

  return CS_NET_SOCKET_NOERROR;

}


int csNetworkSocket2::Disconnect() 
{

  Close();

  return CS_NET_SOCKET_NOERROR;

}
	

int csNetworkSocket2::SetSocketBlock (bool block) 
{
  if (socketfd) 
  {
    const char flag = (block ? 0x00 : 0xff);
    unsigned long arg = (block ? 0 : 1);

    
    if (block) 
    {
      arg = 0;
      blocking = true;
    }
    else
    {
      arg = 1;
      blocking = false;
    }
	
#ifdef OS_WIN32
    IOCTL(socketfd, FIONBIO, &arg);
    // WIN32 doesn't support fcntl
    //fcntl(socketfd,F_SETFL,O_NONBLOCK);
    if (setsockopt(socketfd, SOL_SOCKET, WSAEWOULDBLOCK, &flag,sizeof(flag)) == 0) 
      return CS_NET_SOCKET_NOERROR;
    else
      return CS_NET_SOCKET_CANNOT_SETBLOCK;

#else
//    IOCTL(socketfd,O_NONBLOCK,&arg);
    IOCTL(socketfd, FIONBIO, &arg);
    fcntl(socketfd,F_SETFL,O_NONBLOCK);
    if (setsockopt(socketfd, SOL_SOCKET, O_NONBLOCK, &flag,sizeof(flag)) == 0)
      return CS_NET_SOCKET_NOERROR;
    else
      return CS_NET_SOCKET_CANNOT_SETBLOCK;

#endif
  }
  else
  {
    return CS_NET_SOCKET_NOTCONNECTED;
  }

  return CS_NET_SOCKET_NOERROR;

}

int csNetworkSocket2::SetSocketReuse (bool reuse) 
{
  if (socketfd) 
  {
    const char flag = (reuse ? 0x00 : 0xff);
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &flag,sizeof(flag)) == 0)
      return CS_NET_SOCKET_NOERROR;
    else
      return CS_NET_SOCKET_CANNOT_SETREUSE;

  } 
  else 
  {
    return CS_NET_SOCKET_NOTCONNECTED;
  }
}

int csNetworkSocket2::WaitForConnection (int source, int port, int que) 
{

  local_addr.sin_family = AF_INET;
  local_addr.sin_port = htons(port);
  local_addr.sin_addr.s_addr = source;
  memset(&(local_addr.sin_zero),0,8);

  if (bind(socketfd,(struct sockaddr *)&local_addr,sizeof(struct sockaddr)) == socket_error) 
  {
    return CS_NET_SOCKET_CANNOT_BIND;
  }
  if (listen(socketfd,que) == socket_error) 
  {
    return CS_NET_SOCKET_CANNOT_LISTEN;
  }
  if (!blocking) 
  {
    fd_list[0] = socketfd;
    FD_ZERO(&fd_mask);
    FD_SET(fd_list[0],&fd_mask);
  }
  return CS_NET_SOCKET_NOERROR;
}


int csNetworkSocket2::set (SOCKET socket_fd, bool bConnected, struct sockaddr_in saddr)
{
  socketfd = socket_fd;
  connected = bConnected;
  memcpy(&local_addr,&saddr,sizeof(struct sockaddr_in));

  fd_list[0] = socketfd;
  FD_ZERO(&fd_mask);
  FD_SET(fd_list[0],&fd_mask);

  return CS_NET_SOCKET_NOERROR;
}


int csNetworkSocket2::SELECT (int fds, fd_set *readfds, fd_set *writefds, fd_set *expectfds) 
{
  
  struct timeval to;
  to.tv_sec = 0;
  to.tv_usec = 0;

  last_error = CS_NET_SOCKET_NOERROR;

  int result = select(fds,readfds,writefds,expectfds,&to);

  if (result == socket_error)
  {
    last_error = CS_NET_SOCKET_CANNOT_SELECT;
    return last_error;
  }
  
  return result;
}


char *csNetworkSocket2::RemoteName() 
{

  last_error = CS_NET_SOCKET_NOERROR;

  return inet_ntoa(local_addr.sin_addr);
}

int csNetworkSocket2::IOCTL (SOCKET socketfd, long cmd, u_long *argp)
{

 int result; 

 last_error = CS_NET_SOCKET_NOERROR;

#ifdef OS_WIN32
  result = ioctlsocket(socketfd,cmd,argp);
#else
  result = ioctl(socketfd,cmd,argp);
#endif

 if (result == socket_error)
 {
   last_error = CS_NET_SOCKET_CANNOT_IOCTL;
 }

 return result;

}

iNetworkSocket2* csNetworkSocket2::Accept() 
{

  last_error = CS_NET_SOCKET_NOERROR;


  if (proto_type == SOCK_DGRAM) 
    return 0;

  if (!blocking) 
  {
    FD_ZERO(&fd_mask);
    fd_list[0] = socketfd;
    FD_SET(fd_list[0],&fd_mask);
    if (SELECT(FD_SETSIZE,&fd_mask,0,0) != 1)
    {
	return 0; 
    }
  }

  //if (FD_ISSET(fd_list[0],&fd_mask)) {
  sin_size = sizeof(struct sockaddr_in);
  
  SOCKET socket_fd = accept(socketfd,(struct sockaddr *)&remote_addr,&sin_size);
  
  if (!socket_fd) 
  {
    last_error = CS_NET_SOCKET_CANNOT_ACCEPT;
    return 0;
  }
  csNetworkSocket2 *tmpSocket = new csNetworkSocket2 (this, proto_type);
  tmpSocket->socketfd = socket_fd;
  tmpSocket->connected = true;
  memcpy(&tmpSocket->local_addr,&remote_addr,sizeof(struct sockaddr_in));
  /*
  printf("connection from %s\n",inet_ntoa(remote_addr.sin_addr));
  */

  if (!blocking) 
  {
    fd_list[0] = socketfd;
    FD_ZERO(&fd_mask);
    FD_SET(fd_list[0],&fd_mask);
  }
  
  return tmpSocket;
}

int csNetworkSocket2::Recv (char *buff, size_t size) 
{
  if ((connected) || (proto_type == SOCK_DGRAM)) 
  {
    int result = CS_NET_SOCKET_NOERROR;
    
    last_error = CS_NET_SOCKET_NOERROR;

    if (proto_type == SOCK_STREAM) 
    {
      if (!blocking) 
      {

        
        FD_ZERO(&fd_mask);
        fd_list[0] = socketfd;
        FD_SET(fd_list[0],&fd_mask);
        if (SELECT(FD_SETSIZE,&fd_mask,0,0) != 1)
        { 
          last_error = CS_NET_SOCKET_NODATA;
	  return -1;
        }

        if (FD_ISSET(socketfd,&fd_mask)) 
        {
          result = recv(socketfd,buff,1,0);
          /*
          if (result > 0) printf("%c",buff[0]);
          */

          if (result == 0)
          {
            // disconnected
            last_error = CS_NET_SOCKET_NOTCONNECTED;
            connected = false;
          }
        }

        return result;      

      } 
      else 
      {

        result = recv(socketfd,buff,size,0);
      
        if (result != 1)
        {
          last_error = CS_NET_SOCKET_NOTCONNECTED;
          connected = false;
        }
      }
      if (result > 0) 
        buff[result] = 0;

      return result;
    } 
    else // stream socket
    {
      addr_len = sizeof(struct sockaddr);
      
      if (!blocking)
      {
        FD_ZERO(&fd_mask);
        FD_SET(fd_list[0],&fd_mask);
        if (select(FD_SETSIZE,&fd_mask,0,0,0) != 1) return 0;
        if (FD_ISSET(fd_list[0],&fd_mask)) 
        {
          result = recvfrom(socketfd,buff,size,0,(struct sockaddr *)&local_addr,&addr_len);
          if (result == socket_error)
            last_error = CS_NET_SOCKET_NOTCONNECTED;
        }
      } 
      else
      {
        result = recvfrom(socketfd,buff,size,0,(struct sockaddr *)&local_addr,&addr_len);
        if (result == socket_error)
          last_error = CS_NET_SOCKET_NOTCONNECTED;

      }
      buff[result] = 0;
    }
    
    return result;
    
  }

  return 0;
}

int csNetworkSocket2::ReadLine (char *buff, size_t size ) 
{
  char inbuff[2];
  
  last_error = CS_NET_SOCKET_NOERROR;

  if (!connected)
  {
    last_error = CS_NET_SOCKET_NOTCONNECTED;
    return 0;
  }


  if ((connected) || (proto_type == SOCK_DGRAM)) 
  {
    if (!blocking)
    {
      if (buffer_size == -1)
      {
         buffer_size = size;

         read_buffer = (char *)realloc(read_buffer,size * sizeof(char));
         
         memset(read_buffer,0,size);
      }

      int result = Recv(inbuff,1);
      
      if (result == 0)
      {
        last_error = CS_NET_SOCKET_NOTCONNECTED;
        buffer_size = -1;
        strcpy(buff,read_buffer);
        return strlen(buff);
      }
      if (result  == 1)
      {
        inbuff[1] = 0;

        if (inbuff[0] == '\r')
        {	
          last_error = CS_NET_SOCKET_NOERROR;

          strcpy(buff,read_buffer);
          buffer_size = -1;
          return strlen(buff);
        }

        if (inbuff[0] == '\n')
        {
          last_error = CS_NET_SOCKET_NOERROR;

          strcpy(buff,read_buffer);
          buffer_size = -1;
          return strlen(buff);
        }

        if (inbuff[0] == '\0')
        {
          last_error = CS_NET_SOCKET_NOERROR;

          strcpy(buff,read_buffer);
          buffer_size = -1;
          return strlen(buff);
        }

        strcat(read_buffer,inbuff);

        if (strlen(read_buffer) == size)
        {
          last_error = CS_NET_SOCKET_NOERROR;

          strcpy(buff,read_buffer);
          buffer_size = -1;
          return strlen(buff);

        }
    		
        return 0;
      }

    }
    else
    {
      buffer_size = size;
      read_buffer = (char *)realloc(read_buffer,size * sizeof(char));
      memset(read_buffer,0,size);

      while (strlen(inbuff) <= size) 
      {
        if (Recv(inbuff,1) == 1) 
        {
          inbuff[1] = 0;
        
          if (inbuff[0] == '\r') break;
          if (inbuff[0] == '\n') break;
          if (inbuff[0] == '\0') break;
          strcat(read_buffer,inbuff);
        } 
        else
        {
          break;
        }
      }
    }
    strcat(read_buffer,"\0");

  }
  else
  {
    strcpy(buff,"");
    last_error = CS_NET_SOCKET_NOTCONNECTED;
    return 0;
  }

  strcpy(buff,read_buffer);
  buffer_size = -1;
  return strlen(buff);
}

int csNetworkSocket2::Send (char *buff, size_t size) 
{
  int result = CS_NET_SOCKET_NOERROR;

  if (!connected)
  {
    last_error = CS_NET_SOCKET_NOTCONNECTED;
    return 0;
  }
  
  last_error = CS_NET_SOCKET_NOERROR;

  addr_len = sizeof(struct sockaddr);
  if (connected) 
  {
    if (proto_type == SOCK_DGRAM) 
    {
#ifdef WIN32
      result = sendto(socketfd,buff,size,0,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr));
#else
      result = sendto(socketfd,buff,size,0,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr));
#endif
    } 
    else
    {
      result = send(socketfd,buff,size,0);
    }
    
  }

  if (result == socket_error) 
  {
    last_error = CS_NET_SOCKET_NOTCONNECTED;
  }
  return result;
}

int csNetworkSocket2::Connect (char *host, int port)
{

  last_error = CS_NET_SOCKET_NOERROR;

  host_ent = gethostbyname(host);
  if (host_ent == NULL) 
  {
    last_error = CS_NET_SOCKET_CANNOT_RESOLVE;
    return CS_NET_SOCKET_CANNOT_RESOLVE;
  }
  remote_addr.sin_family = AF_INET;
  remote_addr.sin_port = htons(port);
  remote_addr.sin_addr = *((struct in_addr *)host_ent->h_addr);
  memset(&(remote_addr.sin_zero),0,8);
  
  if (proto_type == SOCK_STREAM) 
  {
    if (connect(socketfd,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr)) == -1) 
    {
      last_error = CS_NET_SOCKET_CANNOT_CONNECT;
      return CS_NET_SOCKET_CANNOT_CONNECT;
    }
  }

  connected = true;
  fd_list[0] = socketfd;
  FD_ZERO(&fd_mask);
  FD_SET(fd_list[0],&fd_mask);
  
  return CS_NET_SOCKET_NOERROR;
}

bool csNetworkSocket2::IsConnected() 
{
  last_error = CS_NET_SOCKET_NOERROR;

  return connected;
}


SCF_IMPLEMENT_FACTORY (csNetworkDriver2)

SCF_EXPORT_CLASS_TABLE (ensocket)
  SCF_EXPORT_CLASS (csNetworkDriver2, "crystalspace.network.driver.sockets2", "Crystal Space Erik Namtvedt's socket implementation")
SCF_EXPORT_CLASS_TABLE_END
