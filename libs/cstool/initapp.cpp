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
#include "cssys/sysdriv.h"
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
#include "igraphic/imageio.h"
#include "ivideo/fontserv.h"
#include "imap/parser.h"
#include "isys/plugin.h"
#include "iutil/eventq.h"
#include "iutil/evdefs.h"
#include "iutil/virtclk.h"
#include "iutil/eventq.h"
#include "iutil/cmdline.h"
#include "iutil/cfgmgr.h"

static SysSystemDriver* global_sys = NULL;
static char* global_config_name = NULL;
static int global_argc = 0;
static const char* const * global_argv = 0;
static iEventHandler* installed_event_handler = NULL;

iObjectRegistry* csInitializer::CreateEnvironment ()
{
  if (!InitializeSCF ()) return NULL;
  iObjectRegistry* object_reg = CreateObjectRegistry ();
  if (!object_reg) return NULL;
  if (!CreatePluginManager (object_reg)) return NULL;
  if (!CreateEventQueue (object_reg)) return NULL;
  if (!CreateVirtualClock (object_reg)) return NULL;
  if (!CreateCommandLineParser (object_reg)) return NULL;
  if (!CreateConfigManager (object_reg)) return NULL;
  return object_reg;
}

bool csInitializer::InitializeSCF ()
{
  global_sys = new SysSystemDriver ();
  return true;
}

iPluginManager* csInitializer::CreatePluginManager (
	iObjectRegistry* object_reg)
{
  return CS_QUERY_REGISTRY (object_reg, iPluginManager);
}

iEventQueue* csInitializer::CreateEventQueue (iObjectRegistry* object_reg)
{
  return CS_QUERY_REGISTRY (object_reg, iEventQueue);
}

iVirtualClock* csInitializer::CreateVirtualClock (iObjectRegistry* object_reg)
{
  return CS_QUERY_REGISTRY (object_reg, iVirtualClock);
}

iCommandLineParser* csInitializer::CreateCommandLineParser (
  	iObjectRegistry* object_reg)
{
  return CS_QUERY_REGISTRY (object_reg, iCommandLineParser);
}

iConfigManager* csInitializer::CreateConfigManager (
	iObjectRegistry* object_reg)
{
  return CS_QUERY_REGISTRY (object_reg, iConfigManager);
}

iObjectRegistry* csInitializer::CreateObjectRegistry ()
{
  return global_sys->GetObjectRegistry ();
}

bool csInitializer::RequestPlugins (iObjectRegistry* /*object_reg*/,
	const char* config_name,
	int argc, const char* const argv[],
	...)
{
  if (config_name) global_config_name = csStrNew (config_name);
  else global_config_name = NULL;
  global_argc = argc;
  global_argv = argv;

  va_list arg;
  va_start (arg, argv);
  char* plugName = va_arg (arg, char*);
  while (plugName != NULL)
  {
    // scfId and version are unused for now.
    int scfId = va_arg (arg, scfInterfaceID);
    int version = va_arg (arg, int);
    global_sys->RequestPlugin (plugName);
    plugName = va_arg (arg, char*);
  }
  va_end (arg);
  return true;
}

bool csInitializer::Initialize (iObjectRegistry* object_reg)
{
  bool rc = global_sys->Initialize (global_argc, global_argv,
    global_config_name);
  if (!rc) return false;

  // Setup the object registry.
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
  else
  {
    iGraphics2D* g2d = CS_QUERY_PLUGIN_ID (plugin_mgr, CS_FUNCID_CANVAS,
  	iGraphics2D);
    if (g2d) { object_reg->Register (g2d); g2d->DecRef (); }
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

  iImageIO* imloader = CS_QUERY_PLUGIN_ID (plugin_mgr,
  	CS_FUNCID_IMGLOADER, iImageIO);
  if (imloader)
  {
    object_reg->Register (imloader);
    imloader->DecRef ();
  }

  iFontServer* fntsvr = CS_QUERY_PLUGIN_ID (plugin_mgr,
  	CS_FUNCID_FONTSERVER, iFontServer);
  if (fntsvr)
  {
    object_reg->Register (fntsvr);
    fntsvr->DecRef ();
  }

  iReporter* reporter = CS_QUERY_PLUGIN_ID (plugin_mgr, CS_FUNCID_REPORTER,
  	iReporter);
  if (reporter)
  {
    object_reg->Register (reporter);
    reporter->DecRef ();
  }

  iStandardReporterListener* stdrep = CS_QUERY_PLUGIN (plugin_mgr,
  	iStandardReporterListener);
  if (stdrep)
  {
    stdrep->SetDefaults ();
    object_reg->Register (stdrep);
    stdrep->DecRef ();
  }

  return true;
}

bool csInitializer::SetupEventHandler (iObjectRegistry* object_reg,
	iEventHandler* evhdlr, unsigned int eventmask)
{
  iEventQueue* q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
  if (q)
  {
    q->RegisterListener (evhdlr, eventmask);
    installed_event_handler = evhdlr;
    return true;
  }
  else return false;
}

class csAppEventHandler : public iEventHandler
{
private:
  csEventHandlerFunc* evhdlr;

public:
  csAppEventHandler (csEventHandlerFunc* evhdlr)
  {
    SCF_CONSTRUCT_IBASE (NULL);
    csAppEventHandler::evhdlr = evhdlr;
  }
  SCF_DECLARE_IBASE;
  virtual bool HandleEvent (iEvent& ev)
  {
    return evhdlr (ev);
  }
};

SCF_IMPLEMENT_IBASE (csAppEventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END


bool csInitializer::SetupEventHandler (iObjectRegistry* object_reg,
	csEventHandlerFunc* evhdlr_func)
{
  csAppEventHandler* evhdlr = new csAppEventHandler (evhdlr_func);
  return SetupEventHandler (object_reg, evhdlr, CSMASK_Broadcast |
  	CSMASK_MouseUp | CSMASK_MouseDown | CSMASK_MouseMove |
	CSMASK_KeyDown | CSMASK_KeyUp | CSMASK_MouseClick |
	CSMASK_MouseDoubleClick | CSMASK_JoystickMove |
	CSMASK_JoystickDown | CSMASK_JoystickUp | CSMASK_Nothing);
}

bool csInitializer::OpenApplication (iObjectRegistry* object_reg)
{
  return global_sys->Open ();
}

void csInitializer::CloseApplication (iObjectRegistry* object_reg)
{
  global_sys->Close ();
}

void csInitializer::DestroyApplication (iObjectRegistry* object_reg)
{
  if (installed_event_handler)
  {
    iEventQueue* q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
    q->RemoveListener (installed_event_handler);
  }
  delete global_sys;
}

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

bool csInitializer::MainLoop (iObjectRegistry* /*object_reg*/)
{
  global_sys->Loop ();
  return true;
}


