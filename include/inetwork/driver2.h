#ifndef _INETWORK_DRIVER2__
#define _INETWORK_DRIVER2__

#include "csutil/scf.h"

SCF_VERSION (iNetworkDriver2, 0, 0, 1);


enum csNetworkDriverError
{
  CS_NET_DRIVER_NOERROR,			// no errors
  CS_NET_DRIVER_CANNOT_INIT,			// cannot start driver
  CS_NET_DRIVER_CANNOT_STOP,			// cannot stop driver
  CS_NET_DRIVER_CANNOT_CREATE_SOCKET,		// cannot create new socket
  CS_NET_DRIVER_UNSUPPORTED_SOCKET_TYPE		// unsupported socket type
};


struct iNetworkSocket2;

struct iNetworkDriver2 : public iBase
{
  virtual iNetworkSocket2 *CreateSocket (int socket_type) = 0;
  virtual int LastError() = 0;
};

#endif
