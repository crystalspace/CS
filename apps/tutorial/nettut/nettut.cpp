/*
    This program demonstrates very basic TCP usage of the ENSOCKET plugin
*/

#include <stdio.h>
#include "cssysdef.h"
#include "csutil/scf.h"
#include "csutil/cfgfile.h"

#include "inetwork/driver2.h"
#include "inetwork/socket2.h"
#include "inetwork/socketerr.h"


CS_IMPLEMENT_APPLICATION

iNetworkDriver2 *driver = NULL;
iNetworkSocket2 *server = NULL;
iNetworkSocket2 *client = NULL;

void usage ( char *arg )
{
	printf("Usage:  %s [server/client] [hostname*] [port]\n",arg);
	printf(" * - specify hostname only if you are using client mode.\n");
	
}

int main(int argc, char *argv[])
{

  int role = -1;  // 0 - server 1 - client
  
  char hostname[65]; // RFCs say that DNS hostnames are not longer than 64 bytes...
  int port = 0;
  char buffer[1024];

  if (argc < 3)
  {
	  usage(argv[0]);
	  return 0;
  }

  if (stricmp(argv[1],"server") == 0)
  {
	  role = 0;
  }
  if (stricmp(argv[1],"client") == 0)
  {
	  role = 1;
  }

  if (role == -1)
  {
	  printf("you must specify client or server\n");
	  usage(argv[0]);
	  return 0;
  }

  if (role == 0) 
  {
      port = atoi(argv[2]);
  } else {
	  if (argc < 4) {
		  printf("please specify a hostname and port\n");
		  usage(argv[0]);
		  return 0;
	  }

	  strncpy(hostname,argv[2],64); // so we dont fill our buffer
	  port = atoi(argv[3]);
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

  driver = SCF_CREATE_INSTANCE("crystalspace.network.driver2.sockets", iNetworkDriver2);
  if (!driver)
  {
	  printf("Unable to load the network plugin.\n");
	  return 0;
  }


  if (role == 0)
  {
	// act as a server
	server = driver->CreateSocket(CS_NET_SOCKET_TYPE_TCP);
	if (server == NULL)
	{
	  printf("unable to create socket\n");
	  return 0;
	}

	server->WaitForConnection(0,port,5);
	if (server->LastError() != CS_NET_SOCKET_NOERROR)
	{
		printf("unable to bind to port %d\n",port);
		return 0;
	}
	
	printf("waiting for connection on port %d...",port);
    
	client = server->Accept();
	if (client == NULL) 
	{
	  printf("unable to accept connection\n");
	  return 0;
	}
	printf("client connection from %s\n",client->RemoteName());
	client->Recv(buffer,1024);
	
	printf("I got '%s' from the client.\n",buffer);
	client->Close();
    
	server->DecRef();
	return 0;

  } else {
	// act as a client
	client = driver->CreateSocket(CS_NET_SOCKET_TYPE_TCP);
	if (client == NULL)
	{
	  printf("unable to create socket\n");
	  return 0;
	}

	printf("connecting to %s port %d...",hostname,port);
	client->Connect(hostname,port);
	if (client->IsConnected() == FALSE)
	{
	  printf("unable to resolve %s\n",hostname);
	  return 0;
	}
	sprintf(buffer,"CrystalSpace Network Tutorial Using ENSOCKET Plugin!");
	client->Send(buffer,strlen(buffer));
	client->Close();
	client->DecRef();

  }

  
  driver->DecRef();

  return 0;
}
