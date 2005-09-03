/*
    Copyright (C) 2005 Dan Hardfeldt and Seth Yastrov

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

#ifndef __CEGUITEST_H__
#define __CEGUITEST_H__

#include <crystalspace.h>
#include "CEGUI.h"
#include "ivaria/cegui.h"

struct iSector;

class CEGUITest : public csApplicationFramework, public csBaseEventHandler
{
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

  void ProcessFrame();
  void FinishFrame();

  bool OnKeyboard(iEvent&);
  void CreateRoom(); 

public:
  CEGUITest();
  ~CEGUITest();

  void OnExit();
  bool OnInitialize(int argc, char* argv[]);

  bool Application();
};

#endif
