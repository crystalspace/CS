//=============================================================================
//
//	Copyright (C)1999-2001 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
//-----------------------------------------------------------------------------
// OSXAssistant.cpp
//
//	Implementation of the iOSXAssistant interface.
//
//	This object owns the OSXDelegate, thus OSXDelegate only gets
//	destroyed when the last reference to this object is removed.
//
//-----------------------------------------------------------------------------
#include "cssysdef.h"
#include "OSXAssistant.h"
#include "OSXDelegate.h"
#include "csutil/cfgacc.h"
#include "csutil/sysfunc.h"
#include "iutil/cmdline.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/virtclk.h"
#include "csver.h"

typedef void* OSXAssistantHandle;
#define NSD_PROTO(RET,FUNC) extern "C" RET OSXAssistant_##FUNC
#define NSD_ASSIST(HANDLE) ((iOSXAssistant*)(HANDLE))

SCF_IMPLEMENT_IBASE(OSXAssistant)
  SCF_IMPLEMENTS_INTERFACE(iOSXAssistant)
  SCF_IMPLEMENTS_INTERFACE(iOSXAssistantLocal)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iEventPlug)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iEventHandler)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE(OSXAssistant::eiEventPlug)
  SCF_IMPLEMENTS_INTERFACE(iEventPlug)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE(OSXAssistant::eiEventHandler)
  SCF_IMPLEMENTS_INTERFACE(iEventHandler)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
OSXAssistant::OSXAssistant(iObjectRegistry* r) : registry(r),
  event_queue(0), event_outlet(0), virtual_clock(0), should_shutdown(false)
{
  SCF_CONSTRUCT_IBASE(0);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiEventPlug);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiEventHandler);

  controller = OSXDelegate_startup(this);

  run_always = false;

  csRef<iEventQueue> q = get_event_queue();
  if (q.IsValid())
  {
    event_outlet = q->CreateEventOutlet(&scfiEventPlug);
    q->RegisterListener(&scfiEventHandler, CSMASK_Broadcast);
  }
}


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
OSXAssistant::~OSXAssistant()
{
  OSXDelegate_shutdown(controller);

  if (event_queue.IsValid())
    event_queue->RemoveListener(&scfiEventHandler);

  SCF_DESTRUCT_EMBEDDED_IBASE(scfiEventHandler);
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiEventPlug);
  SCF_DESTRUCT_IBASE();
}


//-----------------------------------------------------------------------------
// get_event_queue
//-----------------------------------------------------------------------------
csRef<iEventQueue> OSXAssistant::get_event_queue()
{
  if (!event_queue.IsValid() && registry != 0)
    event_queue = CS_QUERY_REGISTRY(registry, iEventQueue);
  return event_queue;
}


//-----------------------------------------------------------------------------
// get_virtual_clock
//-----------------------------------------------------------------------------
csRef<iVirtualClock> OSXAssistant::get_virtual_clock()
{
  if (!virtual_clock.IsValid() && registry != 0)
    virtual_clock = CS_QUERY_REGISTRY(registry, iVirtualClock);
  return virtual_clock;
}


//-----------------------------------------------------------------------------
// init_menu
//	Generate application menu based upon platform configuration.
//-----------------------------------------------------------------------------
void OSXAssistant::init_menu(iConfigFile* macosx_config)
{
  char const* style =
    macosx_config->GetStr("OSX.Global.menu", 0);
  if (style != 0)
    OSXDelegate_init_app_menu(controller, macosx_config, style);
}


//-----------------------------------------------------------------------------
// init_runmode
//	Initialize the value of run_always based on config files
// 	and command-line
//-----------------------------------------------------------------------------
void OSXAssistant::init_runmode()
{
  csRef<iCommandLineParser> parser = 
    CS_QUERY_REGISTRY(registry, iCommandLineParser);
  char const* s = parser->GetOption("alwaysruns");
  if (s != 0)
    run_always = 1;
  else
  {
    // Query whether to pause on loss of focus.
    csRef<iConfigManager> cfg = CS_QUERY_REGISTRY(registry, iConfigManager);
    if (cfg.IsValid())
      run_always = cfg->GetBool("System.RunWhenNotFocused");
  }
}


//-----------------------------------------------------------------------------
// start_event_loop
//	This method returns only after a csevBroadcast even has been posted to
//	the Crystal Space event queue with command code cscmdQuit.
//-----------------------------------------------------------------------------
void OSXAssistant::start_event_loop()
{
  csConfigAccess macosx_config(registry, "/config/macosx.cfg", true,
    iConfigManager::PriorityMin);
  init_menu(macosx_config);
  init_runmode();
  OSXDelegate_start_event_loop(controller);
}


//=============================================================================
// iOSXAssistant Implementation
//=============================================================================
//-----------------------------------------------------------------------------
// C++ iOSXAssistant Interface
//-----------------------------------------------------------------------------
void OSXAssistant::request_shutdown()
{
  event_outlet->ImmediateBroadcast(cscmdQuit, 0);
  OSXDelegate_stop_event_loop(controller);
}

void OSXAssistant::advance_state()
{
  csRef<iVirtualClock> c = get_virtual_clock();
  if (c.IsValid())
    c->Advance();
  csRef <iEventQueue> q = get_event_queue();
  if (q.IsValid())
    q->Process();
  if (!continue_running())
    OSXDelegate_stop_event_loop(controller);
}

bool OSXAssistant::always_runs() { return run_always; }

bool OSXAssistant::continue_running() { return !should_shutdown; }

void OSXAssistant::application_activated()
{
  if (!run_always)
  {
    csRef<iVirtualClock> c = get_virtual_clock();
    if (c != 0)
      c->Resume();
  }
  event_outlet->ImmediateBroadcast(cscmdFocusChanged,true);
}

void OSXAssistant::application_deactivated()
{
  if (!run_always)
  {
    csRef<iVirtualClock> c = get_virtual_clock();
    if (c != 0)
      c->Suspend();
  }
  event_outlet->ImmediateBroadcast(cscmdFocusChanged, false);
}

void OSXAssistant::application_hidden()
{
  event_outlet->ImmediateBroadcast(cscmdCanvasHidden, false);
}

void OSXAssistant::application_unhidden()
{
  event_outlet->ImmediateBroadcast(cscmdCanvasExposed, false);
}

void OSXAssistant::flush_graphics_context()
{ OSXDelegate_flush_graphics_context(controller); }

void OSXAssistant::hide_mouse_pointer()
{ OSXDelegate_hide_mouse_pointer(controller); }

void OSXAssistant::show_mouse_pointer()
{ OSXDelegate_show_mouse_pointer(controller); }

void OSXAssistant::dispatch_event(OSXEvent e, OSXView v)
{ OSXDelegate_dispatch_event(controller, e, v); }

void OSXAssistant::key_down(unsigned int raw, unsigned int cooked)
{ event_outlet->Key(raw, cooked, true); }

void OSXAssistant::key_up(unsigned int raw, unsigned int cooked)
{ event_outlet->Key(raw, cooked, false); }

void OSXAssistant::mouse_down(int b, int x, int y)
{ event_outlet->Mouse(b, true, x, y); }

void OSXAssistant::mouse_up(int b, int x, int y)
{ event_outlet->Mouse(b, false, x, y); }

void OSXAssistant::mouse_moved(int x, int y)
{ event_outlet->Mouse(0, false, x, y); }


//-----------------------------------------------------------------------------
// Pure-C iOSXAssistant Interface (for use from Objective-C world)
//-----------------------------------------------------------------------------
NSD_PROTO(void,request_shutdown)(OSXAssistantHandle h)
    { NSD_ASSIST(h)->request_shutdown(); }
NSD_PROTO(void,advance_state)(OSXAssistantHandle h)
    { NSD_ASSIST(h)->advance_state(); }
NSD_PROTO(int,always_runs)(OSXAssistantHandle h)
    { return NSD_ASSIST(h)->always_runs(); }
NSD_PROTO(int,continue_running)(OSXAssistantHandle h)
    { return NSD_ASSIST(h)->continue_running(); }
NSD_PROTO(void,application_activated)(OSXAssistantHandle h)
    { NSD_ASSIST(h)->application_activated(); }
NSD_PROTO(void,application_deactivated)(OSXAssistantHandle h)
    { NSD_ASSIST(h)->application_deactivated(); }
NSD_PROTO(void,flush_graphics_context)(OSXAssistantHandle h)
    { NSD_ASSIST(h)->flush_graphics_context(); }
NSD_PROTO(void,hide_mouse_pointer)(OSXAssistantHandle h)
    { NSD_ASSIST(h)->hide_mouse_pointer(); }
NSD_PROTO(void,show_mouse_pointer)(OSXAssistantHandle h)
    { NSD_ASSIST(h)->show_mouse_pointer(); }
NSD_PROTO(void,dispatch_event)(OSXAssistantHandle h, OSXEvent e, OSXView v)
    { NSD_ASSIST(h)->dispatch_event(e, v); }
NSD_PROTO(void,key_down)(OSXAssistantHandle h,
    unsigned int raw, unsigned int cooked)
    { NSD_ASSIST(h)->key_down(raw, cooked); }
NSD_PROTO(void,key_up)(OSXAssistantHandle h,
    unsigned int raw, unsigned int cooked)
    { NSD_ASSIST(h)->key_up(raw, cooked); }
NSD_PROTO(void,mouse_down)(OSXAssistantHandle h, int button, int x, int y)
    { NSD_ASSIST(h)->mouse_down(button, x, y); }
NSD_PROTO(void,mouse_up)(OSXAssistantHandle h, int button, int x, int y)
    { NSD_ASSIST(h)->mouse_up(button, x, y); }
NSD_PROTO(void,mouse_moved)(OSXAssistantHandle h, int x, int y)
    { NSD_ASSIST(h)->mouse_moved(x, y); }
NSD_PROTO(void,application_hidden)(OSXAssistantHandle h)
    { NSD_ASSIST(h)->application_hidden(); }
NSD_PROTO(void,application_unhidden)(OSXAssistantHandle h)
    { NSD_ASSIST(h)->application_unhidden(); }


//=============================================================================
// iEventPlug Implementation
//=============================================================================
uint OSXAssistant::eiEventPlug::GetPotentiallyConflictingEvents()
  { return (CSEVTYPE_Keyboard | CSEVTYPE_Mouse); }
uint OSXAssistant::eiEventPlug::QueryEventPriority(uint)
  { return 150; }


//=============================================================================
// iEventHandler Implementation
//=============================================================================
bool OSXAssistant::eiEventHandler::HandleEvent(iEvent& e)
{
  if (e.Type == csevBroadcast && e.Command.Code == cscmdQuit)
    scfParent->should_shutdown = true;
  return false;
}


//=============================================================================
// Implementation of platform-specific application support functions.
//=============================================================================
//-----------------------------------------------------------------------------
// csDefaultRunLoop
//	Implementation of an application run-loop for applications which do
//	not implement their own.
//-----------------------------------------------------------------------------
bool csDefaultRunLoop(iObjectRegistry* r)
{
  bool ok = false;
  csRef<iOSXAssistant> a = CS_QUERY_REGISTRY(r, iOSXAssistant);
  if (a.IsValid())
  {
    csRef<iOSXAssistantLocal> al =
      SCF_QUERY_INTERFACE(a, iOSXAssistantLocal);
    if (al.IsValid())
    {
      al->start_event_loop();
      ok = true;
    }
  }
  return ok;
}


//-----------------------------------------------------------------------------
// csPlatformStartup
//	Platform-specific startup.
//-----------------------------------------------------------------------------
bool csPlatformStartup(iObjectRegistry* r)
{
  printf("Crystal Space for " CS_PLATFORM_NAME " " CS_VERSION "\nPorted to "
    CS_PLATFORM_NAME " by Eric Sunshine <sunshine@sunshineco.com>\n\n");
  csRef<iOSXAssistant> a = csPtr<iOSXAssistant>(new OSXAssistant(r));
  r->Register(a, "iOSXAssistant");
  return true;
}


//-----------------------------------------------------------------------------
// csPlatformShutdown
//	Platform-specific shutdown.
//-----------------------------------------------------------------------------
bool csPlatformShutdown(iObjectRegistry* r)
{
  csRef<iOSXAssistant> a = CS_QUERY_REGISTRY(r, iOSXAssistant);
  if (a.IsValid())
    r->Unregister(a, "OSXAssistant"); // DecRefs() assistant as a side-effect.
  return true;
}
