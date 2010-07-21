/*
    Copyright (C) 2010 Jelle Hellemans

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __DEMO_H__
#define __DEMO_H__

#include "crystalspace.h"

#include "csutil/custom_new_disable.h"
#include <CEGUI.h>
#include "csutil/custom_new_enable.h"

#include "ivaria/icegui.h"

#include "player.h"
#include "monster.h"

class Demo : public csApplicationFramework, public csBaseEventHandler
{
private:
  bool do_freelook;

  csRef<Player> player;

  csRefArray<Monster> monsters;

private:
  iSector *room;
  float rotX, rotY;

  csRef<iEngine> engine;
  csRef<iLoader> loader;
  csRef<iGraphics3D> g3d;
  csRef<iKeyboardDriver> kbd;
  csRef<iVirtualClock> vc;
  csRef<iView> view;
  csRef<iVFS> vfs;
  csRef<iCEGUI> cegui;
  csRef<FramePrinter> printer;

  void Frame();

  bool OnKeyboard(iEvent&);
  bool OnMouseDown(iEvent&);
  bool OnMouseMove(iEvent&);


  void CreateRoom(); 

public:
  Demo();
  ~Demo();

  // Handle exit button clicked event
  bool OnExitButtonClicked (const CEGUI::EventArgs& e);

  void OnExit();
  bool OnInitialize(int argc, char* argv[]);

  bool Application();
  
  // Declare the name of this event handler.
  CS_EVENTHANDLER_NAMES("application.demo")
      
  /* Declare that we want to receive events *after* the CEGUI plugin. */
  virtual const csHandlerID * GenericPrec (csRef<iEventHandlerRegistry> &r1, 
    csRef<iEventNameRegistry> &r2, csEventID event) const 
  {
    static csHandlerID precConstraint[2];
    
    precConstraint[0] = r1->GetGenericID("crystalspace.cegui");
    precConstraint[1] = CS_HANDLERLIST_END;
    return precConstraint;
  }

  CS_EVENTHANDLER_DEFAULT_INSTANCE_CONSTRAINTS
};

#endif
