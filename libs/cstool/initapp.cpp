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
#include "csgfx/shadervar.h"
#include "cstool/initapp.h"
#include "csutil/ansicommand.h"
#include "csutil/cfgacc.h"
#include "csutil/cfgfile.h"
#include "csutil/cfgmgr.h"
#include "csutil/cmdline.h"
#include "csutil/eventnames.h"
#include "csutil/cseventq.h"
#include "csutil/csinput.h"
#include "csutil/csshlib.h"
#include "csutil/csstring.h"
#include "csutil/memdebug.h"
#include "csutil/objreg.h"
#include "csutil/plugldr.h"
#include "csutil/plugmgr.h"
#include "csutil/scf_implementation.h"
#include "csutil/scfstrset.h"
#include "csutil/systemopenmanager.h"
#include "csutil/threadmanager.h"
#include "csutil/verbosity.h"
#include "csutil/virtclk.h"


#ifdef CS_DEBUG
#define CS_LOAD_LIB_VERBOSE true
#else
#define CS_LOAD_LIB_VERBOSE false
#endif

static bool config_done = false;
static iEventHandler* installed_event_handler = 0;

CS_IMPLEMENT_STATIC_VAR(GetDefaultAppID, csString, ())

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
  int argc, char const* const argv[], bool scanDefaultPluginPaths)
{
  CS_INITIALIZE_PLATFORM_APPLICATION;

  if (argc > 0)
  {
    csString appName (argv[0]);
    size_t slashPos = appName.FindLast (CS_PATH_SEPARATOR);
    if (slashPos != (size_t)-1) appName.DeleteAt (0, slashPos + 1);
#ifdef CS_PLATFORM_WIN32
    // Strip .EXE or .DLL extension
    if (appName.Length() >= 4)
    {
      const size_t extPos = appName.Length() - 4;
      const char* ext = appName.GetData() + extPos;
      if ((strcasecmp (ext, ".exe") == 0) || (strcasecmp (ext, ".dll") == 0))
	appName.DeleteAt (extPos, 4);
    }
#endif
    if (!appName.IsEmpty())
    {
      ::GetDefaultAppID()->Replace ("CrystalApp.");
      ::GetDefaultAppID()->Append (appName);
    }
  }
  
  iObjectRegistry* reg = 0;
  if (InitializeSCF (argc, argv, scanDefaultPluginPaths))
  {
    iObjectRegistry* r = CreateObjectRegistry();
    if (r != 0)
    {
      if (CreateCommandLineParser(r, argc, argv) &&
          CreateVerbosityManager(r) &&
          CreatePluginManager(r) &&
          CreateEventQueue(r) &&
          CreateVirtualClock(r) &&
          CreateConfigManager(r) &&
          CreateThreadManager(r) &&
          CreateInputDrivers(r) &&
	        CreateStringSet (r) &&
	  CreateSystemOpenManager (r) &&
          csPlatformStartup(r))
        reg = r;
      else
        r->DecRef();
#ifdef CS_MEMORY_TRACKER
    CS::Debug::MemTracker::RegisterModule ("app");
#endif

    }
  }
  return reg;
}

bool csInitializer::InitializeSCF (int argc, const char* const argv[],
                                   bool scanDefaultPluginPaths)
{
  scfInitialize (argc, argv, scanDefaultPluginPaths);

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

iThreadManager* csInitializer::CreateThreadManager (iObjectRegistry* r)
{
  csRef<iThreadManager> threadmgr = csPtr<iThreadManager> (new csThreadManager (r));
  r->Register (threadmgr, "iThreadManager");
  return threadmgr;
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
  strings.AttachNew (new csScfStringSet);
  r->Register (strings, "crystalspace.shared.stringset");
  csRef<iShaderVarStringSet> svStrings;
  svStrings.AttachNew (new CS::ScfStringSet<iShaderVarStringSet> );
  r->Register (svStrings, "crystalspace.shader.variablenameset");
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
  csRef<iCommandLineParser> cmdline (
    csQueryRegistry<iCommandLineParser> (r));
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

iSystemOpenManager* csInitializer::CreateSystemOpenManager (iObjectRegistry* r)
{
  csRef<iSystemOpenManager> mgr;
  mgr.AttachNew (new CS::Base::SystemOpenManager (r));
  r->Register (mgr, "iSystemOpenManager");
  return mgr;
}

static void SetupPluginLoadErrVerbosity(iObjectRegistry* r)
{
  csRef<iVerbosityManager> verbosemgr (
    csQueryRegistry<iVerbosityManager> (r));
  bool verbose = CS_LOAD_LIB_VERBOSE;
  if (verbosemgr.IsValid())
    verbose = verbosemgr->Enabled("loadlib");
  csSetLoadLibraryVerbose (verbose);
}

iVFS* csInitializer::SetupVFS(iObjectRegistry* r, const char* pluginID)
{
  csRef<iVFS> VFS (csQueryRegistry<iVFS> (r));
  if (!VFS)
  {
    csRef<iPluginManager> plugin_mgr (csQueryRegistry<iPluginManager> (r));
    csRef<iComponent> b = plugin_mgr->QueryPluginInstance (
      "iVFS", scfInterfaceTraits<iVFS>::GetVersion());
    VFS = scfQueryInterfaceSafe<iVFS> (b);
  }
  if (!VFS)
  {
    csRef<iPluginManager> plugin_mgr (csQueryRegistry<iPluginManager> (r));
    VFS = csLoadPlugin<iVFS> (plugin_mgr, pluginID);
    if (!VFS)
    {
      /* NB: loading the plugin should have already resulted in a message 
       * having been printed. */
      static const char highlight[] = 
	" " CS_ANSI_TEXT_BOLD_ON "%s" CS_ANSI_RST " ";
      csFPrintf (stderr, highlight, 
	"* This likely means that the plugins could not be found.");
      csFPrintf (stderr, "\n");
      csFPrintf (stderr, highlight, "If you're a user:");
      csFPrintf (stderr, "Check the working directory the application starts "
	"from -\n");
#if defined(CS_PLATFORM_MACOSX)
      csFPrintf (stderr, "  on MaxOS/X, it should be the directory containing the\n"
	"  %s subdirectory, *not* %s.\n",
	CS::Quote::Single ("<application>.app"),
	CS::Quote::Single ("<application>.app/ContentsMacOS"));
#else
      csFPrintf (stderr, "  usually, it is the same as the directory with the "
	"executable.\n");
#endif
      csFPrintf (stderr, "  If in doubt, contact the vendor.\n");
      csFPrintf (stderr, highlight, "If you're a developer:");
      csFPrintf (stderr, "Check if the CRYSTAL environment var points to the\n");
      csFPrintf (stderr, "  correct location - usually the directory CS was "
	"built in.\n");
      csFPrintf (stderr, "  You can also use the %s command line switch "
	"to troubleshoot\n", CS::Quote::Single ("--verbose"));
      csFPrintf (stderr, "  where CS looks for plugins.\n");
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
  
  if (AppID == 0)
    AppID = GetDefaultAppID();

  // @@@ Is this ugly? Do we need a better, more generalized way of doing this?
  // The reason that the VFS plugin is required
  // this early is that both the application configuration file and the
  // configuration file for other plugins may (and almost always do) reside on
  // a VFS volume.

  csRef<iVFS> VFS = SetupVFS(r);

  csRef<iConfigManager> Config (csQueryRegistry<iConfigManager> (r));
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

  // Handle command line config settings
  {
    csRef<iCommandLineParser> cmdline = 
      csQueryRegistry<iCommandLineParser> (r);
    if (cmdline.IsValid())
    {
      csRef<csConfigFile> cmdlineConfig;
      cmdlineConfig.AttachNew (new csConfigFile);
      cmdlineConfig->ParseCommandLine (cmdline, VFS);
      Config->AddDomain (cmdlineConfig, iConfigManager::ConfigPriorityCmdLine);
    }
  }

  // Init the threadmanager using this config.
  {
    csRef<iThreadManager> tman = csQueryRegistry<iThreadManager> (r);
    tman->Init(Config);
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

  csArray<csPluginRequest>::ConstIterator i(a.GetIterator());
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
  
  csRef<iConfigManager> Config (csQueryRegistry<iConfigManager> (r));
  plugldr->AddConfigurationPlugins (Config, "System.Plugins.");

  csRef<iCommandLineParser> CommandLine (
  	csQueryRegistry<iCommandLineParser> (r));
  CS_ASSERT (CommandLine != 0);
  plugldr->AddCommandLinePlugins (CommandLine);

  bool rc = plugldr->LoadPlugins ();
  delete plugldr;
  
  // flush all removed config files
  Config->FlushRemoved();

  return rc;
}

bool csInitializer::SetupEventHandler (
  iObjectRegistry* r, 
  iEventHandler* evhdlr, 
  const csEventID events[])
{
  CS_ASSERT(installed_event_handler == 0);
  csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (r));
  if (q) {
    return (q->RegisterListener (evhdlr, events) != CS_HANDLER_INVALID);
  }
  return false;
}

class csAppEventHandler : public 
  scfImplementation1<csAppEventHandler, iEventHandler>
{
private:
  csEventHandlerFunc evhdlr;
public:
  csAppEventHandler (csEventHandlerFunc h) 
    : scfImplementationType (this), evhdlr(h)
  { }
  virtual ~csAppEventHandler()
  { }
  virtual bool HandleEvent (iEvent& e) { return evhdlr (e); }
  CS_EVENTHANDLER_NAMES("application")
  CS_EVENTHANDLER_NIL_CONSTRAINTS
};


bool csInitializer::SetupEventHandler (
  iObjectRegistry* r, 
  csEventHandlerFunc evhdlr_func, 
  const csEventID events[])
{
  csRef<csAppEventHandler> evhdlr;
  evhdlr.AttachNew (new csAppEventHandler (evhdlr_func));
  return SetupEventHandler (r, evhdlr, events);
}

bool csInitializer::SetupEventHandler (iObjectRegistry* r, csEventHandlerFunc evhdlr_func)
{
  static const csEventID rootev[2] = { csevAllEvents (r), CS_EVENTLIST_END };
  return SetupEventHandler(r, evhdlr_func, rootev);
}

bool csInitializer::OpenApplication (iObjectRegistry* r)
{
  SetupConfigManager (r, 0);

  // Pass the open event to all interested listeners.
  csRef<iEventQueue> EventQueue (csQueryRegistry<iEventQueue> (r));
  CS_ASSERT (EventQueue != 0);
  csEventID ename = csevSystemOpen(r);
  csRef<iEvent> e = EventQueue->CreateBroadcastEvent(ename);
  EventQueue->Dispatch(*e);

  return true;
}

void csInitializer::CloseApplication (iObjectRegistry* r)
{
  // Notify all interested listeners that the system is going down
  csRef<iEventQueue> EventQueue (csQueryRegistry<iEventQueue> (r));
  if (EventQueue)
  {
    csRef<iEvent> e = EventQueue->CreateBroadcastEvent(csevSystemClose(r));
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
    csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (r));
    if (q)
      q->RemoveAllListeners ();
  }

  /* Process all pending thread events: these events may refer to objects that
     are to destroyed in the following cleanups, so process them as long as
     they may be possibly valid. */
  {
    csRef<iThreadManager> threadMgr (csQueryRegistry<iThreadManager> (r));
    if (threadMgr)
      threadMgr->ProcessAll ();
  }

  // Explicitly unload all plugins from the plugin manager because
  // some plugins hold references to the plugin manager so the plugin
  // manager will never get destructed if there are still plugins in memory.
  {
    csRef<iPluginManager> plugin_mgr (csQueryRegistry<iPluginManager> (r));
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

const char* csInitializer::GetDefaultAppID()
{
  csString* defAppID = ::GetDefaultAppID();
  return defAppID ? defAppID->GetData() : "CrystalApp.Noname";
}
