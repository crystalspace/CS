/*
    Copyright (C) 2001 by Jorrit Tyberghein
  
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

#include "cssysdef.h"
#include "cstool/initapp.h"
#include "isys/system.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "iengine/engine.h"
#include "ivaria/reporter.h"
#include "ivaria/stdrep.h"
#include "ivaria/conout.h"
#include "isys/vfs.h"
#include "imap/parser.h"
#include "isys/plugin.h"

bool csInitializeApplication (iObjectRegistry* object_reg, bool use_reporter,
	bool use_reporter_listener)
{
  iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);

  iGraphics3D* g3d = CS_QUERY_PLUGIN_ID (plugin_mgr, CS_FUNCID_VIDEO,
  	iGraphics3D);
  if (g3d)
  {
    object_reg->Register (g3d);
    if (g3d->GetDriver2D ())
      object_reg->Register (g3d->GetDriver2D ());
    g3d->DecRef ();
  }

  iEngine* engine = CS_QUERY_PLUGIN (plugin_mgr, iEngine);
  if (engine)
  {
    object_reg->Register (engine);
    engine->DecRef ();
  }

  iVFS* vfs = CS_QUERY_PLUGIN_ID (plugin_mgr, CS_FUNCID_VFS, iVFS);
  if (vfs)
  {
    object_reg->Register (vfs);
    vfs->DecRef ();
  }
  
  iConsoleOutput* console = CS_QUERY_PLUGIN_ID (plugin_mgr,
  	CS_FUNCID_CONSOLE, iConsoleOutput);
  if (console)
  {
    object_reg->Register (console);
    console->DecRef ();
  }

  iLoader* loader = CS_QUERY_PLUGIN_ID (plugin_mgr,
  	CS_FUNCID_LVLLOADER, iLoader);
  if (loader)
  {
    object_reg->Register (loader);
    loader->DecRef ();
  }

  iReporter* reporter = CS_QUERY_PLUGIN_ID (plugin_mgr, CS_FUNCID_REPORTER,
  	iReporter);
  if (!reporter && use_reporter)
  {
    reporter = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.utilities.reporter",
    	CS_FUNCID_REPORTER, iReporter);
    if (!reporter)
      return false;
    
  }
  if (reporter)
  {
    object_reg->Register (reporter);
    reporter->DecRef ();
  }

  iStandardReporterListener* stdrep = CS_QUERY_PLUGIN (plugin_mgr,
  	iStandardReporterListener);
  if (!stdrep && use_reporter_listener)
  {
    stdrep = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.utilities.stdrep",
    	"StdRep", iStandardReporterListener);
    if (!stdrep)
      return false;

  }
  if (stdrep)
  {
    stdrep->SetDefaults ();
    object_reg->Register (stdrep);
    stdrep->DecRef ();
  }

  return true;
}


