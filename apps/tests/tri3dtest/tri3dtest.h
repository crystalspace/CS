/*
    Copyright (C) 2007 by Scott Johnson

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This application is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this application; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _APP_TRI3DTEST_H_
#define _APP_TRI3DTEST_H_

#include <iostream>
#include <string>
#include "crystalspace.h"
#include "csgeom/triangulate3d.h"

class Tri3DTest : public csApplicationFramework, public csBaseEventHandler {
	private:
		iObjectRegistry* object_reg;
		csRef<iPluginManager> plugManager;
		csRef<iEngine> engine;
		csRef<iGraphics3D> g3d;
    csRef<iView> view;
		csRef<iReporter> report;
		
	public:
		Tri3DTest();

		// Crystal Space Functions
		bool OnInitialize(int argc, char* argv[]);
		bool Application();

  CS_EVENTHANDLER_NAMES("application.tri3dtest")
  CS_EVENTHANDLER_NIL_CONSTRAINTS
};

#endif
