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

#define CS_SYSDEF_PROVIDE_PATH
#include "cssysdef.h"
#include "cssys/sysdriv.h"
#include "cssys/system.h"
#include "cssys/sysfunc.h"
#include "cssys/csshlib.h"
#include "cstool/initapp.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "iutil/plugin.h"
#include "csutil/cseventq.h"
#include "csutil/cmdline.h"
#include "csutil/cfgfile.h"
#include "csutil/cfgmgr.h"
#include "csutil/cfgacc.h"
#include "csutil/prfxcfg.h"
#include "csutil/objreg.h"
#include "csutil/virtclk.h"
#include "csutil/csinput.h"
#include "csutil/plugmgr.h"
#include "csutil/plugldr.h"
#include "iutil/eventq.h"
#include "iutil/evdefs.h"
#include "iutil/virtclk.h"
#include "iutil/eventq.h"
#include "iutil/cmdline.h"
#include "iutil/cfgmgr.h"
#include "isound/renderer.h"
#include "isound/loader.h"
#include "inetwork/driver.h"
#include "igraphic/imageio.h"
#include "ivideo/fontserv.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "imap/services.h"
#include "imap/parser.h"
#include "imesh/crossbld.h"
#include "imesh/mdlconv.h"
#include "iengine/motion.h"
#include "iengine/engine.h"
#include "ivaria/iso.h"
#include "ivaria/conin.h"
#include "ivaria/reporter.h"
#include "ivaria/stdrep.h"
#include "ivaria/conout.h"
#include "ivaria/perfstat.h"

static SysSystemDriver* global_sys = NULL;
static bool config_done = false;
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
  if (!CreateInputDrivers (object_reg)) return NULL;
  return object_reg;
}

bool csInitializer::InitializeSCF ()
{
  // Initialize Shared Class Facility|
  char scfconfigpath [MAXPATHLEN + 1];

#ifndef CS_STATIC_LINKED
  // Add both installpath and installpath/lib dirs to search for plugins
  csGetInstallPath (scfconfigpath, sizeof (scfconfigpath));
  csAddLibraryPath (scfconfigpath);
  strcat (scfconfigpath, "lib");
  int scfconfiglen = strlen(scfconfigpath);
  scfconfigpath[scfconfiglen] = PATH_SEPARATOR;
  scfconfigpath[scfconfiglen+1] = 0;
  csAddLibraryPath (scfconfigpath);
#endif

  // Find scf.cfg and initialize SCF
  csGetInstallPath (scfconfigpath, sizeof (scfconfigpath));
  strcat (scfconfigpath, "scf.cfg");
  csConfigFile scfconfig (scfconfigpath);
  scfInitialize (&scfconfig);
  return true;
}

iObjectRegistry* csInitializer::CreateObjectRegistry ()
{
  csObjectRegistry* object_reg = new csObjectRegistry ();
  global_sys = new SysSystemDriver (object_reg);
  return object_reg;
}

iPluginManager* csInitializer::CreatePluginManager (
	iObjectRegistry* object_reg)
{
  csPluginManager* plugmgr = new csPluginManager (object_reg);
  object_reg->Register (plugmgr, NULL);
  plugmgr->DecRef ();
  return plugmgr;
}

iEventQueue* csInitializer::CreateEventQueue (iObjectRegistry* object_reg)
{
  // Register the shared event queue.
  iEventQueue* q = new csEventQueue (object_reg);
  object_reg->Register (q, NULL);
  q->DecRef();
  return q;
}

bool csInitializer::CreateInputDrivers (iObjectRegistry* object_reg)
{
  // Register some generic pseudo-plugins.  (Some day these should probably
  // become real plugins.)
  iKeyboardDriver* k = new csKeyboardDriver (object_reg);
  iMouseDriver*    m = new csMouseDriver    (object_reg);
  iJoystickDriver* j = new csJoystickDriver (object_reg);
  object_reg->Register (k, NULL);
  object_reg->Register (m, NULL);
  object_reg->Register (j, NULL);
  j->DecRef();
  m->DecRef();
  k->DecRef();
  return true;
}

iVirtualClock* csInitializer::CreateVirtualClock (iObjectRegistry* object_reg)
{
  csVirtualClock* vc = new csVirtualClock ();
  object_reg->Register (vc, NULL);
  vc->DecRef ();
  return vc;
}

iCommandLineParser* csInitializer::CreateCommandLineParser (
  	iObjectRegistry* object_reg)
{
  iCommandLineParser* cmdline = new csCommandLineParser ();
  object_reg->Register (cmdline, NULL);
  cmdline->DecRef ();
  return cmdline;
}

iConfigManager* csInitializer::CreateConfigManager (
	iObjectRegistry* object_reg)
{
  iConfigFile* cfg = new csConfigFile ();
  iConfigManager* Config = new csConfigManager (cfg, true);
  object_reg->Register (Config, NULL);
  Config->DecRef ();
  cfg->DecRef ();
  return Config;
}

bool csInitializer::SetupCommandLineParser (iObjectRegistry* object_reg,
  	int argc, const char* const argv[])
{
  iCommandLineParser* c = CS_QUERY_REGISTRY (object_reg, iCommandLineParser);
  CS_ASSERT (c != NULL);
  c->Initialize (argc, argv);
  return true;
}

bool csInitializer::SetupConfigManager (iObjectRegistry* object_reg,
	const char* configName)
{
  if (config_done) return true;

  // @@@ This is ugly.  We need a better, more generalized way of doing this.
  // Hard-coding the name of the VFS plugin (crytalspace.kernel.vfs) is bad.
  // Then later ensuring that we skip over this same plugin when requested
  // by the client is even uglier.  The reason that the VFS plugin is required
  // this early is that both the application configuration file and the
  // configuration file for other plugins may (and almost always do) reside on
  // a VFS volume.

  // we first create an empty application config file, so we can create the
  // config manager at all. Then we load the VFS. After that, all config files
  // can be loaded. At the end, we make the user-and-application-specific
  // config file the dynamic one.

  iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  iVFS* VFS = CS_QUERY_PLUGIN (plugin_mgr, iVFS);
  if (!VFS)
  {
    VFS = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.kernel.vfs",
  	  CS_FUNCID_VFS, iVFS);
    if (!VFS)
      return false;
    object_reg->Register (VFS);
  }

  iConfigManager* Config = CS_QUERY_REGISTRY (object_reg, iConfigManager);
  iConfigFile* cfg = Config->GetDynamicDomain ();
  Config->SetDomainPriority (cfg, iConfigManager::ConfigPriorityApplication);

  // Initialize application configuration file
  if (configName)
    if (!cfg->Load (configName, VFS))
    {
      VFS->DecRef ();
      return false;
    }
  VFS->DecRef ();

  // look if the user-specific config domain should be used
  {
    csConfigAccess cfgacc (object_reg, "/config/system.cfg");
    if (cfgacc->GetBool ("System.UserConfig", true))
    {
      // open the user-specific, application-neutral config domain
      cfg = new csPrefixConfig ("/config/user.cfg", VFS, "Global",
      	"User.Global");
      Config->AddDomain (cfg, iConfigManager::ConfigPriorityUserGlobal);
      cfg->DecRef ();

      // open the user-and-application-specific config domain
      cfg = new csPrefixConfig ("/config/user.cfg", VFS,
      	cfgacc->GetStr ("System.ApplicationID", "Noname"),
        "User.Application");
      Config->AddDomain (cfg, iConfigManager::ConfigPriorityUserApp);
      Config->SetDynamicDomain (cfg);
      cfg->DecRef ();
    }
  }

  config_done = true;
  return true;
}

bool csInitializer::RequestPlugins (iObjectRegistry* object_reg, ...)
{
  if (!config_done) SetupConfigManager (object_reg, NULL);

  csPluginLoader* plugldr = new csPluginLoader (object_reg);

  va_list arg;
  va_start (arg, object_reg);
  char* plugName = va_arg (arg, char*);
  while (plugName != NULL)
  {
    int scfId = va_arg (arg, scfInterfaceID);
    int version = va_arg (arg, int);
    // scfId and version are unused for now.
    (void)scfId; (void)version;
    plugldr->RequestPlugin (plugName);
    plugName = va_arg (arg, char*);
  }
  va_end (arg);

  bool rc = plugldr->LoadPlugins ();
  delete plugldr;
  return rc;
}

bool csInitializer::Initialize (iObjectRegistry* object_reg)
{
  if (!config_done) SetupConfigManager (object_reg, NULL);
  return csPlatformStartup (object_reg) && global_sys->Initialize ();
}

bool csInitializer::SetupEventHandler (iObjectRegistry* object_reg,
	iEventHandler* evhdlr, unsigned int eventmask)
{
  CS_ASSERT(installed_event_handler == 0);
  iEventQueue* q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
  if (q)
  {
    q->RegisterListener (evhdlr, eventmask);
    installed_event_handler = evhdlr;
    return true;
  }
  return false;
}

class csAppEventHandler : public iEventHandler
{
private:
  csEventHandlerFunc evhdlr;
public:
  SCF_DECLARE_IBASE;
  csAppEventHandler (csEventHandlerFunc h) : evhdlr(h)
  { SCF_CONSTRUCT_IBASE (NULL); }
  virtual bool HandleEvent (iEvent& e) { return evhdlr (e); }
};

SCF_IMPLEMENT_IBASE (csAppEventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

bool csInitializer::SetupEventHandler (iObjectRegistry* object_reg,
	csEventHandlerFunc evhdlr_func, unsigned int eventmask)
{
  csAppEventHandler* evhdlr = new csAppEventHandler (evhdlr_func);
  return SetupEventHandler (object_reg, evhdlr, eventmask);
}

bool csInitializer::OpenApplication (iObjectRegistry* object_reg)
{
  // Pass the open event to all interested listeners.
  csEvent Event (csGetTicks (), csevBroadcast, cscmdSystemOpen);
  iEventQueue* EventQueue = CS_QUERY_REGISTRY (object_reg, iEventQueue);
  CS_ASSERT (EventQueue != NULL);
  EventQueue->Dispatch (Event);
  return true;
}

void csInitializer::CloseApplication (iObjectRegistry* object_reg)
{
  // Warn all interested listeners the system is going down
  csEvent Event (csGetTicks (), csevBroadcast, cscmdSystemClose);
  iEventQueue* EventQueue = CS_QUERY_REGISTRY (object_reg, iEventQueue);
  CS_ASSERT (EventQueue != NULL);
  EventQueue->Dispatch (Event);
}

void csInitializer::DestroyApplication (iObjectRegistry* object_reg)
{
  CloseApplication (object_reg);
  csPlatformShutdown (object_reg);
  if (installed_event_handler)
  {
    iEventQueue* q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
    q->RemoveListener (installed_event_handler);
  }
  delete global_sys;

  // Explicitly clear the object registry before its destruction since some
  // objects being cleared from it may need to query it for other objects, and
  // such queries can fail (depending upon the compiler) if they are made while
  // the registry itself it being destroyed.  Furthermore, such objects may may
  // SCF queries as they are destroyed, so this must occur before SCF is
  // finalized (see below).
  object_reg->Clear ();
  object_reg->DecRef ();

  iSCF::SCF->Finish();
}
