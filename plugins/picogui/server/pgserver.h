/*
    Crystal Space PicoGUI Server Plugin
    (C) 2003 Mat Sutcliffe <oktal@gmx.co.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
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

#ifndef __CS_PICOGUI_SERVER_PGSERVER_H__
#define __CS_PICOGUI_SERVER_PGSERVER_H__

#include "ivaria/gserver.h"
#include "iutil/comp.h"
#include "ivideo/graph2d.h"
#include "csutil/array.h"
#include "iutil/eventh.h"

class csPicoGUIServer : public iGUIServer
{
 private:
  csRef<iGUIConnection> Conn;
  bool Draw, Key, Mouse;
  csArray<csGUIClientHandle> Clients;

 protected:
  bool Initialize (iObjectRegistry *);
  bool HandleEvent (iEvent &);

 public:
  SCF_DECLARE_IBASE;

  csPicoGUIServer (iBase *parent);
  virtual ~csPicoGUIServer ();

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csPicoGUIServer);
    bool Initialize (iObjectRegistry *or)
      { return scfParent->Initialize (or); }
  } scfiComponent;
  friend struct eiComponent;

  struct eiEventHandler : public iEventHandler
  {
    SCF_DECLARE_EMBEDDED_IBASE (csPicoGUIServer);
    bool HandleEvent (iEvent &ev) { return scfParent->HandleEvent (ev); }
  } scfiEventHandler;
  friend struct eiEventHandler;

  virtual void SetDrawState (bool s) { Draw = s; }
  virtual void SetKeyboardState (bool s) { Key = s; }
  virtual void SetMouseState (bool s) { Mouse = s; }
  virtual void SetConnection (iGUIConnection *c) { Conn = c; }
  virtual void ClientConnected (csGUIClientHandle h)
    { Clients.Push (h); }
  virtual void ClientDisconnected (csGUIClientHandle h)
    { Clients.Delete (h); }
};

#endif
