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
#define CS_SYSDEF_PROVIDE_DIR
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
static bool sys_init_done = false;
static iEventHandler* installed_event_handler = NULL;

CS_IMPLEMENT_STATIC_VARIABLE_CLEANUP

iObjectRegistry* csInitializer::CreateEnvironment (
  int argc, char const* const argv[])
{
  iObjectRegistry* reg = 0;
  if (InitializeSCF())
  {
    iObjectRegistry* r = CreateObjectRegistry();
    if (r != 0)
    {
      if (CreatePluginManager(r) &&
          CreateEventQueue(r) &&
          CreateVirtualClock(r) &&
          CreateCommandLineParser(r, argc, argv) &&
          CreateConfigManager(r) &&
          CreateInputDrivers(r) &&
	  csPlatformStartup(r))
        reg = r;
      else
        r->DecRef();
    }
  }
  return reg;
}

bool csInitializer::InitializeSCF ()
{
  // Initialize Shared Class Facility|
  char scfconfigpath [CS_MAXPATHLEN + 1];
  char scffilepath [CS_MAXPATHLEN + 1];
  struct dirent* de;
  DIR* dh;

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
  strcpy(scffilepath, scfconfigpath);
  strcat (scffilepath, "scf.cfg");
  csConfigFile scfconfig (scffilepath);
  scfInitialize (&scfconfig);

  dh = opendir(scfconfigpath[0] == '\0' ? "." : scfconfigpath);
  if (dh != 0)
  {
    while ((de = readdir(dh)) != 0)
    {
      if (!(isdir(scfconfigpath, de)))
      {
        int const n = strlen(de->d_name);
        if (n >= 4 && strcasecmp(de->d_name + n - 4, ".scf") == 0)
        {
	  strcpy(scffilepath, scfconfigpath);
	  strcat(scffilepath, de->d_name);
	  scfconfig.Clear();
	  scfconfig.Load(scffilepath);
	  int scfver = scfconfig.GetInt(".scfVersion", 0);
	  switch (scfver)
	  {
	    case 1:
	      iConfigIterator* it;
	      it = scfconfig.Enumerate();
	      while (it->Next())
	      {
		char const* key = it->GetKey();
		if (*key == '.')
		  scfconfig.DeleteKey(key);
	      }
	      it->DecRef();
	      scfInitialize (&scfconfig);
	      break;
	    default:
	      /* unrecognized version */;
	  }
	}
      }
    }
    closedir(dh);
  }
  return true;
}

iObjectRegistry* csInitializer::CreateObjectRegistry ()
{
  csObjectRegistry* r = new csObjectRegistry ();
  global_sys = new SysSystemDriver (r);
  return r;
}

iPluginManager* csInitializer::CreatePluginManager (iObjectRegistry* r)
{
  csPluginManager* plugmgr = new csPluginManager (r);
  r->Register (plugmgr, "iPluginManager");
  plugmgr->DecRef ();
  return plugmgr;
}

iEventQueue* csInitializer::CreateEventQueue (iObjectRegistry* r)
{
  // Register the shared event queue.
  iEventQueue* q = new csEventQueue (r);
  r->Register (q, "iEventQueue");
  q->DecRef();
  return q;
}

bool csInitializer::CreateInputDrivers (iObjectRegistry* r)
{
  // Register some generic pseudo-plugins.  (Some day these should probably
  // become real plugins.)
  iKeyboardDriver* k = new csKeyboardDriver (r);
  iMouseDriver*    m = new csMouseDriver    (r);
  iJoystickDriver* j = new csJoystickDriver (r);
  r->Register (k, "iKeyboardDriver");
  r->Register (m, "iMouseDriver");
  r->Register (j, "iJoystickDriver");
  j->DecRef();
  m->DecRef();
  k->DecRef();
  return true;
}

iVirtualClock* csInitializer::CreateVirtualClock (iObjectRegistry* r)
{
  csVirtualClock* vc = new csVirtualClock ();
  r->Register (vc, "iVirtualClock");
  vc->DecRef ();
  return vc;
}

iCommandLineParser* csInitializer::CreateCommandLineParser(
  iObjectRegistry* r, int argc, char const* const argv[])
{
  iCommandLineParser* c = new csCommandLineParser (argc, argv);
  r->Register (c, "iCommandLineParser");
  c->DecRef ();
  return c;
}

iConfigManager* csInitializer::CreateConfigManager (iObjectRegistry* r)
{
  iConfigFile* cfg = new csConfigFile ();
  iConfigManager* Config = new csConfigManager (cfg, true);
  r->Register (Config, "iConfigManager");
  Config->DecRef ();
  cfg->DecRef ();
  return Config;
}

bool csInitializer::SetupConfigManager (
  iObjectRegistry* r, const char* configName, const char *AppID)
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

  iVFS* VFS = CS_QUERY_REGISTRY (r, iVFS);
  if (!VFS)
  {
    iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (r, iPluginManager);
    VFS = (iVFS*)(plugin_mgr->QueryPlugin ("iVFS", VERSION_iVFS));
    plugin_mgr->DecRef ();
  }
  if (!VFS)
  {
    iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (r, iPluginManager);
    VFS = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.kernel.vfs", iVFS);
    plugin_mgr->DecRef ();
    if (!VFS)
      return false;
    r->Register (VFS, "iVFS");
  }

  iConfigManager* Config = CS_QUERY_REGISTRY (r, iConfigManager);
  iConfigFile* cfg = Config->GetDynamicDomain ();
  Config->SetDomainPriority (cfg, iConfigManager::ConfigPriorityApplication);

  // Initialize application configuration file
  if (configName)
    if (!cfg->Load (configName, VFS))
    {
      VFS->DecRef ();
      Config->DecRef ();
      return false;
    }
  VFS->DecRef ();

  // look if the user-specific config domain should be used
  {
    csConfigAccess cfgacc (r, "/config/system.cfg");
    if (cfgacc->GetBool ("System.UserConfig", true))
    {
      // open the user-specific, application-neutral config domain
      cfg = new csPrefixConfig ("/config/user.cfg", VFS, "Global",
      	"User.Global");
      Config->AddDomain (cfg, iConfigManager::ConfigPriorityUserGlobal);
      cfg->DecRef ();

      // open the user-and-application-specific config domain
      cfg = new csPrefixConfig ("/config/user.cfg", VFS,
      	cfgacc->GetStr ("System.ApplicationID", AppID),
        "User.Application");
      Config->AddDomain (cfg, iConfigManager::ConfigPriorityUserApp);
      Config->SetDynamicDomain (cfg);
      cfg->DecRef ();
    }
  }

  config_done = true;
  Config->DecRef ();
  return true;
}

bool csInitializer::RequestPlugins (iObjectRegistry* r, ...)
{
  if (!config_done) SetupConfigManager (r, NULL);

  csPluginLoader* plugldr = new csPluginLoader (r);

  va_list arg;
  va_start (arg, r);
  char* plugName = va_arg (arg, char*);
  while (plugName != NULL)
  {
    char* intName = va_arg (arg, char*);
    int scfId = va_arg (arg, scfInterfaceID);
    int version = va_arg (arg, int);
    // scfId and version are unused for now.
    (void)scfId; (void)version;
    char* colon = strchr (plugName, ':');
    if (colon)
    {
      // We have a special tag name.
      char newPlugName[512];
      strcpy (newPlugName, plugName);
      *strchr (newPlugName, ':') = 0;
      plugldr->RequestPlugin (newPlugName, colon+1);
    }
    else
    {
      plugldr->RequestPlugin (plugName, intName);
    }
    plugName = va_arg (arg, char*);
  }
  va_end (arg);

  bool rc = plugldr->LoadPlugins ();
  delete plugldr;
  return rc;
}

bool csInitializer::SetupEventHandler (
  iObjectRegistry* r, iEventHandler* evhdlr, unsigned int eventmask)
{
  CS_ASSERT(installed_event_handler == 0);
  iEventQueue* q = CS_QUERY_REGISTRY (r, iEventQueue);
  if (q)
  {
    q->RegisterListener (evhdlr, eventmask);
    q->DecRef ();
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

bool csInitializer::SetupEventHandler (
  iObjectRegistry* r, csEventHandlerFunc evhdlr_func, unsigned int eventmask)
{
  csAppEventHandler* evhdlr = new csAppEventHandler (evhdlr_func);
  bool rc = SetupEventHandler (r, evhdlr, eventmask);
  evhdlr->DecRef ();
  return rc;
}

bool csInitializer::OpenApplication (iObjectRegistry* r)
{
  if (!config_done) SetupConfigManager (r, NULL);
  // @@@ Temporary.
  if (!sys_init_done)
  {
    if (!global_sys->Initialize ()) return false;
    sys_init_done = true;
  }

  // Pass the open event to all interested listeners.
  csEvent Event (csGetTicks (), csevBroadcast, cscmdSystemOpen);
  iEventQueue* EventQueue = CS_QUERY_REGISTRY (r, iEventQueue);
  CS_ASSERT (EventQueue != NULL);
  EventQueue->Dispatch (Event);
  EventQueue->DecRef ();
  return true;
}

void csInitializer::CloseApplication (iObjectRegistry* r)
{
  // Notify all interested listeners that the system is going down
  csEvent Event (csGetTicks (), csevBroadcast, cscmdSystemClose);
  iEventQueue* EventQueue = CS_QUERY_REGISTRY (r, iEventQueue);
  CS_ASSERT (EventQueue != NULL);
  EventQueue->Dispatch (Event);
  EventQueue->DecRef ();
}

void csInitializer::DestroyApplication (iObjectRegistry* r)
{
  CloseApplication (r);
  csPlatformShutdown (r);
  if (installed_event_handler)
  {
    iEventQueue* q = CS_QUERY_REGISTRY (r, iEventQueue);
    CS_ASSERT (q != NULL);
    q->RemoveListener (installed_event_handler);
    q->DecRef ();
  }
  delete global_sys;

  // Explicitly unload all plugins from the plugin manager because
  // some plugins hold references to the plugin manager so the plugin
  // manager will never get destructed if there are still plugins in memory.
  iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (r, iPluginManager);
  if (plugin_mgr)
  {
    plugin_mgr->Clear ();
    plugin_mgr->DecRef ();
  }

  // Explicitly clear the object registry before its destruction since some
  // objects being cleared from it may need to query it for other objects, and
  // such queries can fail (depending upon the compiler) if they are made while
  // the registry itself it being destroyed.  Furthermore, such objects may
  // make SCF queries as they are destroyed, so this must occur before SCF is
  // finalized (see below).
  r->Clear ();
  r->DecRef ();

  // destruct all static variables that had been created during runtime
  CS_STATIC_VAR_DESTRUCTION

  iSCF::SCF->Finish();
}

