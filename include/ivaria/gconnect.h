/*
    Crystal Space GUI Client/Server Connection Interface
    (C) 2003 Mat Sutcliffe <oktal@gmx.co.uk>

*/

#ifndef __CS_IVARIA_GCLIENT_H__
#define __CS_IVAR_A_GCLIENT_H__

#include "csutil/scf.h"

typedef int csGUIClientHandle;

SCF_VERSION (iGUIConnection, 0, 0, 1);

/**
 * A class implementing this interface is used to connect
 * a GUI server to one or more clients.
 * For a "loopback" style setup (where the client(s) and server are on the same
 * computer), there can be one single instance of this class.
 * If the client(s) and server are on different computers, there should be
 * one instance of this class on each computer.
 */
struct iGUIConnection : public iBase
{
  /// The server calls this every frame for each client.
  virtual size_t GetDataFromClient (csGUIClientHandle, void *, size_t) = 0;

  /// The server calls this whenever it wants to send data to a client.
  virtual bool SendDataToClient (csGUIClientHandle, void *buf, size_t len) = 0;

  /**
   * The client helper calls this when it wants to see if the server has
   * sent any data. It writes the data to the given buffer and returns the
   * number of bytes actually written.
   */
  virtual size_t GetDataFromServer (void *data, size_t len) = 0;

  /**
   * The client helper calls then when it wants to send data to the server.
   * It reads len bytes from the given buffer.
   */
  virtual bool SendDataToServer (void *buf, size_t len) = 0;
};

#endif
