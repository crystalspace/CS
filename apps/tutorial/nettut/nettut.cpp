/*
    ENSOCKET Plugin example
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

#include <stdio.h>
#include "cssysdef.h"
#include "csutil/scf.h"
#include "csutil/cfgfile.h"

/// include our needed headers
#include "inetwork/driver2.h"
#include "inetwork/socket2.h"
#include "inetwork/sockerr.h"

CS_IMPLEMENT_APPLICATION

/// these are our interfaces
iNetworkDriver2 *driver = NULL;
iNetworkSocket2 *server = NULL;
iNetworkSocket2 *client = NULL;

void usage ( char *arg )
{
	printf("Usage:  %s [server/client] [tcp/udp] [hostname*] [port]\n",arg);
	printf(" * - specify hostname only if you are using client mode.\n");
	
}

int main(int argc, char *argv[])
{

  int role = -1;  // 0 - server 1 - client
  int proto = 0;
  char hostname[65]; // RFCs say that DNS hostnames are not longer than 64 bytes...
  int port = 0;
  char buffer[1024];

  //  If we dont have enough command-line arguments show the usage
  if (argc < 4)
  {
	  usage(argv[0]);
	  return 0;
  }

  // if arg1 is server or client adjust role
  if (strcasecmp(argv[1],"server") == 0)
  {
	  role = 0;
  }
  if (strcasecmp(argv[1],"client") == 0)
  {
	  role = 1;
  }

  // if arg1 wasnt client or server show usage
  if (role == -1)
  {
	  printf("you must specify client or server\n");
	  usage(argv[0]);
	  return 0;
  }

  // if arg2 is udp or tcp set proto
  if (strcasecmp(argv[2],"udp") == 0)
  {
	  proto = CS_NET_SOCKET_TYPE_UDP;
  }
  if (strcasecmp(argv[2],"tcp") == 0)
  {
	  proto = CS_NET_SOCKET_TYPE_TCP;
  }

  // if arg2 wasnt tcp or udp show usage
  if (proto == 0)
  {
	printf("please specify a protocol ('tcp' or 'udp')\n");
	usage(argv[0]);
	return 0;
  }

  // if we are server then arg3 is our port
  if (role == 0) 
  {
      port = atoi(argv[3]);
  } else {
	  // if we are a client then arg4 is a client and arg5 is a port
	  if (argc < 5) {
		  printf("please specify a hostname and port\n");
		  usage(argv[0]);
		  return 0;
	  }

	  strncpy(hostname,argv[3],64); // so we dont fill our buffer
	  port = atoi(argv[4]);
  }

  if (port == 0)
  {
	  printf("you must specify a port\n");
	  usage(argv[0]);
	  return 0;
  }

  if (role == 0) 
  {
	printf("server running tcp on port %d.\n",port);
  } else {
	printf("server running udp on port %d.\n",port);
  }
  
  // This method requires you register dlls with scfreg (or manually) in scf.cfg
  csConfigFile config ("scf.cfg");
  scfInitialize (&config);

  /// load our ensocket plugin
  driver = SCF_CREATE_INSTANCE("crystalspace.network.driver.sockets2", iNetworkDriver2);
  if (!driver)
  {
	  printf("Unable to load the network plugin.\n");
	  return 0;
  }

  if (role == 0)
  {
	// act as a server
	/// create server socket of *proto* type
	server = driver->CreateSocket(proto);
	if (server == NULL)
	{
	  printf("unable to create socket\n");
	  return 0;
	}
	/// tell our server socket to wait on port *port* and que up to 5 incoming connections
	server->WaitForConnection(0,port,5);
	if (server->LastError() != CS_NET_SOCKET_NOERROR)
	{
		printf("unable to bind to port %d\n",port);
		return 0;
	}
	
	printf("waiting for connection on port %d...",port);
    
	if (proto == CS_NET_SOCKET_TYPE_TCP)
	{
		/// accept any incoming connections
		client = server->Accept();
		if (client == NULL) 
		{
		  printf("unable to accept connection\n");
		  return 0;
		}
		printf("ok\n");
	} else {
		while (1) {
			/// read up to 1k from our server connection (we'd read from the server socket if we are UDP)
			if (server->Recv(buffer,1024) > 0)
			{
				printf("client connection from %s\n",server->RemoteName());
				break;
			}
		}
		printf("ok\n");
	}
	
	if (proto == CS_NET_SOCKET_TYPE_TCP)
	{
		printf("client connection from %s\n",client->RemoteName());
		/// read up to 1k from our client connection (we'd read from the client socket if we are TCP)
		client->Recv(buffer,1024);
		/// close our client connection
		client->Close();
	} else {
		/// close our server connection
		server->Close();
	}
	
	printf("I got '%s' from the client.\n",buffer);
	
  /// kill the server
	server->DecRef();
	return 0;

  } else {
	// act as a client
	/// create a client connect of *proto* type
	client = driver->CreateSocket(proto);
	if (client == NULL)
	{
	  printf("unable to create socket\n");
	  return 0;
	}

	printf("connecting to %s port %d...",hostname,port);
	/// connect client connection to *hostname* on port *port*
	client->Connect(hostname,port);

	if (client->IsConnected() == FALSE)
	{
	  printf("unable to resolve or connect to %s\n",hostname);
	  return 0;
	}

	printf("ok\nsending message...");
	sprintf(buffer,"CrystalSpace Network Tutorial Using ENSOCKET Plugin!");
	/// send *buffer* of size *strlen(buffer)*
	client->Send(buffer,strlen(buffer));
	printf("ok\n");
	/// close client connection
	client->Close();
	/// kill our client connection
	client->DecRef();
  }

  /// kill our driver
  driver->DecRef();

  return 0;
}
