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
#include "csutil/csshlib.h"
#include "csutil/sysfunc.h"
#include "cstool/initapp.h"
#include "csutil/cfgacc.h"
#include "csutil/cfgfile.h"
#include "csutil/cfgmgr.h"
#include "csutil/cmdline.h"
#include "csutil/cseventq.h"
#include "csutil/csinput.h"
#include "csutil/objreg.h"
#include "csutil/physfile.h"
#include "csutil/plugldr.h"
#include "csutil/plugmgr.h"
#include "csutil/prfxcfg.h"
#include "csutil/scfstrset.h"
#include "csutil/verbosity.h"
#include "csutil/virtclk.h"
#include "csutil/xmltiny.h"
#include "iengine/engine.h"
#include "igraphic/imageio.h"
#include "imap/loader.h"
#include "imesh/crossbld.h"
#include "imesh/mdlconv.h"
#include "isound/loader.h"
#include "isound/renderer.h"
#include "iutil/cfgmgr.h"
#include "iutil/cmdline.h"
#include "iutil/comp.h"
#include "iutil/evdefs.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/strset.h"
#include "iutil/vfs.h"
#include "iutil/virtclk.h"
#include "ivaria/conin.h"
#include "ivaria/conout.h"
#include "ivaria/reporter.h"
#include "ivaria/stdrep.h"
#include "ivideo/fontserv.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"

#ifdef CS_DEBUG
#define CS_LOAD_LIB_VERBOSE true
#else
#define CS_LOAD_LIB_VERBOSE false
#endif

static bool config_done = false;
static iEventHandler* installed_event_handler = 0;

csPluginRequest::csPluginRequest(
  csString cname, csString iname, scfInterfaceID iid, int iver) :
  class_name(cname),
  interface_name(iname),
  interface_id(iid),
  interface_version(iver)
{
}

void csPluginRequest::set(csPluginRequest const& r)
{
  if (&r != this)
  {
    class_name = r.class_name;
    interface_name = r.interface_name;
    interface_id = r.interface_id;
    interface_version = r.interface_version;
  }
}

bool csPluginRequest::operator==(csPluginRequest const& r) const
{
  return (&r == this) ||
    (class_name == r.class_name &&
    interface_name == r.interface_name &&
    interface_id == r.interface_id &&
    interface_version == r.interface_version);
}

iObjectRegistry* csInitializer::CreateEnvironment (
  int argc, char const* const argv[])
{
  CS_INITIALIZE_PLATFORM_APPLICATION;

  iObjectRegistry* reg = 0;
  if (InitializeSCF (argc, argv))
  {
    iObjectRegistry* r = CreateObjectRegistry();
    if (r != 0)
    {
      if (CreatePluginManager(r) &&
          CreateEventQueue(r) &&
          CreateVirtualClock(r) &&
          CreateCommandLineParser(r, argc, argv) &&
          CreateVerbosityManager(r) &&
          CreateConfigManager(r) &&
          CreateInputDrivers(r) &&
	  CreateStringSet (r) &&
          csPlatformStartup(r))
        reg = r;
      else
        r->DecRef();
#ifdef CS_MEMORY_TRACKER
    extern void mtiRegisterModule (char* Class);
    mtiRegisterModule ("app");
#endif

    }
  }
  return reg;
}

bool csInitializer::InitializeSCF (int argc, const char* const argv[])
{
  scfInitialize (argc, argv);

  return true;
}

iObjectRegistry* csInitializer::CreateObjectRegistry ()
{
  return new csObjectRegistry ();
}

iPluginManager* csInitializer::CreatePluginManager (iObjectRegistry* r)
{
  csRef<csPluginManager> plugmgr = csPtr<csPluginManager> (
  new csPluginManager (r));
  r->Register (plugmgr, "iPluginManager");
  return plugmgr;
}

iEventQueue* csInitializer::CreateEventQueue (iObjectRegistry* r)
{
  // Register the shared event queue.
  csRef<iEventQueue> q = csPtr<iEventQueue> (new csEventQueue (r));
  r->Register (q, "iEventQueue");
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

bool csInitializer::CreateStringSet (iObjectRegistry* r)
{
  csRef<iStringSet> strings;
  strings.AttachNew (new csScfStringSet ());
  r->Register (strings, "crystalspace.shared.stringset");
  return true;
}

iVirtualClock* csInitializer::CreateVirtualClock (iObjectRegistry* r)
{
  csRef<csVirtualClock> vc = csPtr<csVirtualClock> (new csVirtualClock ());
  r->Register (vc, "iVirtualClock");
  return vc;
}

iCommandLineParser* csInitializer::CreateCommandLineParser(
  iObjectRegistry* r, int argc, char const* const argv[])
{
  csRef<iCommandLineParser> c = csPtr<iCommandLineParser> (
      new csCommandLineParser (argc, argv));
  r->Register (c, "iCommandLineParser");
  return c;
}

iVerbosityManager* csInitializer::CreateVerbosityManager (
  iObjectRegistry* r)
{
  csVerbosityManager* v = new csVerbosityManager;
  csRef<iCommandLineParser> cmdline (CS_QUERY_REGISTRY (
    r, iCommandLineParser));
  if (cmdline.IsValid())
  {
    for (size_t i = 0; ; i++)
    {
      char const* flags = cmdline->GetOption ("verbose", i);
      if (flags != 0)
	v->Parse(flags);
      else
	break;
    }
  }
  csRef<iVerbosityManager> vm;
  vm.AttachNew (v);
  r->Register (vm, "iVerbosityManager");
  return v;
}

iConfigManager* csInitializer::CreateConfigManager (iObjectRegistry* r)
{
  csRef<iConfigFile> cfg = csPtr<iConfigFile> (new csConfigFile ());
  csRef<iConfigManager> Config = csPtr<iConfigManager> (
    new csConfigManager (cfg, true));
  r->Register (Config, "iConfigManager");
  return Config;
}

static void SetupPluginLoadErrVerbosity(iObjectRegistry* r)
{
  csRef<iVerbosityManager> verbosemgr (
    CS_QUERY_REGISTRY(r, iVerbosityManager));
  bool verbose = CS_LOAD_LIB_VERBOSE;
  if (verbosemgr.IsValid())
    verbose = verbosemgr->Enabled("loadlib");
  csSetLoadLibraryVerbose (verbose);
}

iVFS* csInitializer::SetupVFS(iObjectRegistry* r, const char* pluginID)
{
  csRef<iVFS> VFS (CS_QUERY_REGISTRY (r, iVFS));
  if (!VFS)
  {
    csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (r, iPluginManager));
    VFS = csPtr<iVFS> ((iVFS*)(plugin_mgr->QueryPlugin (
      "iVFS", scfInterfaceTraits<iVFS>::GetVersion())));
  }
  if (!VFS)
  {
    csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (r, iPluginManager));
    VFS = CS_LOAD_PLUGIN (plugin_mgr, pluginID, iVFS);
    if (!VFS)
    {
      csFPrintf (stderr, "Couldn't load VFS plugin \"%s\"!\n", pluginID);
      return 0;
    }
    r->Register (VFS, "iVFS");
  }
  return VFS;
}

bool csInitializer::SetupConfigManager (
  iObjectRegistry* r, char const* configName, char const* AppID)
{
  SetupPluginLoadErrVerbosity(r);

  if (config_done) return true;

  // @@@ Is this ugly? Do we need a better, more generalized way of doing this?
  // The reason that the VFS plugin is required
  // this early is that both the application configuration file and the
  // configuration file for other plugins may (and almost always do) reside on
  // a VFS volume.

  csRef<iVFS> VFS = SetupVFS(r);

  csRef<iConfigManager> Config (CS_QUERY_REGISTRY (r, iConfigManager));
  csRef<iConfigFile> cfg = Config->GetDynamicDomain ();
  Config->SetDomainPriority (cfg, iConfigManager::ConfigPriorityApplication);

  // Initialize application configuration file
  if (configName)
    if (!cfg->Load (configName, VFS))
      return false;

  // Check if the user-specific config domain should be used.
  {
    csConfigAccess cfgacc (r, "/config/system.cfg");
    if (cfgacc->GetBool ("System.UserConfig", true))
    {
      // Open the user-specific, application-neutral config domain.
      cfg = csGetPlatformConfig ("CrystalSpace.Global");
      Config->AddDomain (cfg, iConfigManager::ConfigPriorityUserGlobal);

      // Open the user-and-application-specific config domain.
      const char* appid = cfgacc->GetStr ("System.ApplicationID", AppID);
      cfg = csGetPlatformConfig (appid);
      Config->AddDomain (cfg, iConfigManager::ConfigPriorityUserApp);
      Config->SetDynamicDomain (cfg);
    }
  }

  config_done = true;
  return true;
}

bool csInitializer::RequestPlugins (iObjectRegistry* r, ...)
{
  va_list args;
  va_start(args, r);
  bool const ok = RequestPluginsV(r, args);
  va_end(args);
  return ok;
}

bool csInitializer::RequestPluginsV (iObjectRegistry* r, va_list args)
{
  csArray<csPluginRequest> reqs;
  char const* plugName = va_arg (args, char*);
  while (plugName != 0)
  {
    char* intName = va_arg (args, char*);
    scfInterfaceID scfId = va_arg (args, scfInterfaceID);
    int version = va_arg (args, int);
    csPluginRequest req(plugName, intName, scfId, version);
    reqs.Push(req);
    plugName = va_arg (args, char*);
  }
  return RequestPlugins(r, reqs);
}

bool csInitializer::RequestPlugins (
  iObjectRegistry* r, csArray<csPluginRequest> const& a)
{
  SetupConfigManager (r, 0);
  SetupPluginLoadErrVerbosity(r);

  csPluginLoader* plugldr = new csPluginLoader (r);

  csArray<csPluginRequest>::Iterator i(a.GetIterator());
  while (i.HasNext())
  {
    csPluginRequest const req(i.Next());
    csString plugName = req.GetClassName();
    csString intName = req.GetInterfaceName();
    size_t const colon = plugName.FindFirst(':');
    if (colon != (size_t)-1)
    {
      // We have a special tag name.
      intName = plugName.Slice(colon + 1, plugName.Length() - colon);
      plugName.Truncate(colon);
    }
    plugldr->RequestPlugin (plugName, intName);
  }

  bool rc = plugldr->LoadPlugins ();
  delete plugldr;
  return rc;
}

bool csInitializer::SetupEventHandler (
  iObjectRegistry* r, iEventHandler* evhdlr, unsigned int eventmask)
{
  CS_ASSERT(installed_event_handler == 0);
  csRef<iEventQueue> q (CS_QUERY_REGISTRY (r, iEventQueue));
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
  { SCF_CONSTRUCT_IBASE (0); }
  virtual ~csAppEventHandler()
  { SCF_DESTRUCT_IBASE(); }
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
  SetupConfigManager (r, 0);

  // Pass the open event to all interested listeners.
  csRef<iEventQueue> EventQueue (CS_QUERY_REGISTRY (r, iEventQueue));
  CS_ASSERT (EventQueue != 0);
  csRef<iEvent> e(EventQueue->CreateEvent(csevBroadcast));
  e->Add("cmdCode", (uint32)cscmdSystemOpen);
  e->Add("cmdInfo", (uint32)0);
  EventQueue->Dispatch(*e);

  return true;
}

void csInitializer::CloseApplication (iObjectRegistry* r)
{
  // Notify all interested listeners that the system is going down
  csRef<iEventQueue> EventQueue (CS_QUERY_REGISTRY (r, iEventQueue));
  if (EventQueue)
  {
    csRef<iEvent> e(EventQueue->CreateEvent(csevBroadcast));
    e->Add("cmdCode", (uint32)cscmdSystemClose);
    e->Add("cmdInfo", (uint32)0);
    EventQueue->Dispatch(*e);
  }
}

void csInitializer::DestroyApplication (iObjectRegistry* r)
{
  CloseApplication (r);
  csPlatformShutdown (r);

  // This will remove the installed_event_handler (if used) as well as free
  // all other event handlers which are registered through plug-ins or
  // other sources.
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY (r, iEventQueue));
    if (q)
      q->RemoveAllListeners ();
  }

  // Explicitly unload all plugins from the plugin manager because
  // some plugins hold references to the plugin manager so the plugin
  // manager will never get destructed if there are still plugins in memory.
  {
    csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (r, iPluginManager));
    if (plugin_mgr)
      plugin_mgr->Clear ();
    // Force cleanup here.
  }

  // Explicitly clear the object registry before its destruction since some
  // objects being cleared from it may need to query it for other objects, and
  // such queries can fail (depending upon the compiler) if they are made while
  // the registry itself it being destroyed.  Furthermore, such objects may
  // make SCF queries as they are destroyed, so this must occur before SCF is
  // finalized (see below).
  r->Clear ();
  r->DecRef (); // @@@ Why is this being done? !!!

  // Destroy all static variables created by CS_IMPLEMENT_STATIC_VAR() or one
  // of its cousins.
  CS_STATIC_VARIABLE_CLEANUP

  iSCF::SCF->Finish();

  config_done = false;
  installed_event_handler = 0;
}
