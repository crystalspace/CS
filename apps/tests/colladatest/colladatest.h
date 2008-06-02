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

#ifndef _APP_COLLADATEST_H_
#define _APP_COLLADATEST_H_

#include <iostream>
#include <string>
#include "crystalspace.h"

#define TESTDIR "E:$/Source$/CS-COLLADA$/data$/colladatest$/"
#define COLLADATESTFILE "colladatest.dae"
#define CSTESTFILE "/lev/colladatest/colladatest.cslib"

class ColladaTest : public csApplicationFramework, public csBaseEventHandler {
	private:
		iObjectRegistry* object_reg;
		csRef<iDocumentSystem> docSystem;
		csRef<iVFS> fileSystem;
		csRef<iDocument> colladaDocument;
		csRef<iFile> colladaFile;
		csRef<iPluginManager> plugManager;
		csRef<iColladaConvertor> colladaConv;

	public:
		ColladaTest();

		// Crystal Space Functions
		bool OnInitialize(int argc, char* argv[]);
		bool Application();

  CS_EVENTHANDLER_NAMES("application.colladaconvertor")
  CS_EVENTHANDLER_NIL_CONSTRAINTS
};

#endif

