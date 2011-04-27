/*
    Copyright (C) 2009 by Jelle Hellemans

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

#ifndef __SMOKETEST_H__
#define __SMOKETEST_H__

#include "crystalspace.h"

#include <vector>
#include <string>
#include <sstream>

#include "csutil/custom_new_disable.h"
#include <CEGUI.h>
#include "csutil/custom_new_enable.h"

#include "ivaria/icegui.h"

struct iSector;

class SmokeTest : public csApplicationFramework, public csBaseEventHandler
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
  csRef<iCommandLineParser> cmdline;
  csRef<iCEGUI> cegui;
  csRef<FramePrinter> printer;

  void Frame();

  bool OnKeyboard(iEvent&);
  void CreateRoom(); 

  //commandline
  size_t selectedIndex;
  bool renderAll;
  bool renderAllToPath;

  size_t current;
  std::vector<std::string> paths;
  void ScanDirectory(const std::string& path);
  void LoadPath(size_t index, bool render = false);

  void ScreenShot(const std::string& fileName);

  csRef<iCollection> collection;

private:
  bool OnGoto (const CEGUI::EventArgs& e);
  bool OnPrevious (const CEGUI::EventArgs& e);
  bool OnNext (const CEGUI::EventArgs& e);

  CEGUI::UVector2 originalSize;
  bool onMouseEnters (const CEGUI::EventArgs& e);
  bool onMouseLeaves (const CEGUI::EventArgs& e);

public:
  SmokeTest();
  ~SmokeTest();

  void OnExit();
  bool OnInitialize(int argc, char* argv[]);

  bool Application();
  
  // Declare the name of this event handler.
  CS_EVENTHANDLER_NAMES("application.regressiontest")
      
  /* Declare that we're not terribly interested in having events
     delivered to us before or after other modules, plugins, etc. */
  CS_EVENTHANDLER_NIL_CONSTRAINTS
};

#endif
