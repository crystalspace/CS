#ifndef __INETWORK_SOCKETERR__
#define __INETWORK_SOCKETERR__


enum csNetworkSocketType
{
  CS_NET_SOCKET_TYPE_TCP = 9990,		// tcp socket type
  CS_NET_SOCKET_TYPE_UDP			// udp socket type
};


enum csNetworkSocketError
{
  CS_NET_SOCKET_NOERROR,			// no errors
  CS_NET_SOCKET_CANNOT_CREATE,			// cannot create socket
  CS_NET_SOCKET_UNSUPPORTED_SOCKET_TYPE,	// unsupported socket type
  CS_NET_SOCKET_NOTCONNECTED,			// unconnected socket
  CS_NET_SOCKET_CANNOT_SETBLOCK,		// cannot set block/unblock
  CS_NET_SOCKET_CANNOT_SETREUSE,		// cannot set reuse
  CS_NET_SOCKET_CANNOT_BIND,			// cannot bind
  CS_NET_SOCKET_CANNOT_LISTEN,			// cannot listen
  CS_NET_SOCKET_CANNOT_SELECT,			// cannot select
  CS_NET_SOCKET_CANNOT_IOCTL,			// cannot ioctl
  CS_NET_SOCKET_CANNOT_ACCEPT,			// cannot accept
  CS_NET_SOCKET_NODATA,				// recv did not get data 
  CS_NET_SOCKET_CANNOT_RESOLVE,			// cannot resolve name
  CS_NET_SOCKET_CANNOT_CONNECT			// cannot connect

};



#endif
