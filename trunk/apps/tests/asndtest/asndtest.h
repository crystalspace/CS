/*
    Copyright (C) 2006 by Søren Bøg

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

#ifndef __ASNDTEST_H__
#define __ASNDTEST_H__

#include <crystalspace.h>
#include <isndsys/ss_renderer.h>
#include <isndsys/ss_loader.h>
#include <isndsys/ss_source.h>
#include <isndsys/ss_stream.h>
#include <isndsys/ss_data.h>
#include <isndsys/ss_listener.h>

class ASndTest : public csApplicationFramework, public csBaseEventHandler
{
private:
  csRef<iEngine> engine;
  csRef<iLoader> loader;
  csRef<iVFS> vfs;
  csRef<iGraphics3D> g3d;
  csRef<iKeyboardDriver> kbd;
  csRef<iVirtualClock> vc;
  csRef<iSndSysRenderer> sndrenderer;
  csRef<iSndSysLoader> sndloader;
  csRef<FramePrinter> printer;

  iSector* world;
  void CreateWorld ();

  csRef<iView> view;
  float rotYaw;
  void Frame ();

  csRef<iSndSysSource3D> movingsound;
  csRef<iSndSysSource3DDoppler> movingsounddoppler;
  csRef<iMeshWrapper> movingsoundsprite;
  float movingsoundstep;
  csVector3 movingsoundposition;

  csRef<iSndSysListenerDoppler> listenerdoppler;

public:
  ASndTest ();
  ~ASndTest ();

  void OnExit ();
  bool OnKeyboard (iEvent&);
  bool OnInitialize (int argc, char* argv[]);

  bool Application ();

  CS_EVENTHANDLER_NAMES("application.asndtest")
  CS_EVENTHANDLER_NIL_CONSTRAINTS
};

#endif // __ASNDTEST_H__
