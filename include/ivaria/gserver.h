/*
    Crystal Space GUI Server Interface
    (C) 2003 Mat Sutcliffe <oktal@gmx.co.uk>

*/

#ifndef __CS_IVARIA_GSERVER_H__
#define __CS_IVAR_A_GSERVER_H__

#include "csutil/scf.h"

typedef int csGUIClientHandle;

SCF_VERSION (iGUIConnection, 0, 0, 1);

/**
 * This interface connects a client to a server. If the client and server
 * exist on different computers, an implementation of this interface that
 * communicates over the network might be used. If they are both on the
 * same computers, a simple read to and write from buffer may be used.
 */
struct iGUIConnection : public iBase
{
};

SCF_VERSION (iGUIServer, 0, 0, 1);

/**
 * This interface represents the server side of a client/server GUI system.
 * This is the part that draws to the screen and accepts input from the user.
 * It uses an iGraphics2D for drawing, and an iEventQueue for getting input,
 * both of which it finds in the object registry.
 */
struct iGUIServer : public iBase
{
  /// Tell the server to start or stop drawing.
  virtual void SetDrawState (bool) = 0;

  /// Tell the server to start or stop accepting keyboard input.
  virtual void SetKeyboardState (bool) = 0;

  /// Tell the server to start or stop accepting mouse input.
  virtual void SetMouseState (bool) = 0;

  /// Tell it to use this connection class to communicate with the clients.
  virtual void SetConnection (iGUIConnection *) = 0;

  /// The iGUIConnection calls this to notify the server that a client joined.
  virtual void ClientConnected (csGUIClientHandle) = 0;

  /// The iGUIConnection calls this to notify the server that a client quit.
  virtual void ClientDisconnected (csGUIClientHandle) = 0;
};

#endif
