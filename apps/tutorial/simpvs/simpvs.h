/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __SIMPVS_H__
#define __SIMPVS_H__

#include <crystalspace.h>
#include "ivaria/icegui.h"

class Simple : public csApplicationFramework, public csBaseEventHandler
{
private:
  csRef<iEngine> engine;
  csRef<iLoader> loader;
  csRef<iGraphics3D> g3d;
  csRef<iKeyboardDriver> kbd;
  csRef<iVirtualClock> vc;
  csRef<iVFS> vfs;
  csRef<iView> view;
  csRef<iCEGUI> cegui;
  csRef<FramePrinter> printer;

  iSector* room;

  csString mode;

  void Frame ();
  
  void SaveVideoPreference();
  
  bool Setup ();
  void OnExit ();
  
  csEventID KeyboardDown;
  csEventID Quit;
  
  void CreateGui();

  bool SetSoftware (const CEGUI::EventArgs& e);
  bool SetOpenGL (const CEGUI::EventArgs& e);

public:
  Simple ();
  virtual ~Simple ();

  void Start ();

  bool OnInitialize (int argc, char* argv[]);
  bool Application ();

  bool OnKeyboard (iEvent&);

  CS_EVENTHANDLER_PHASE_3D("crystalspace.simpvs")
};

#endif // __SIMPVS_H__

