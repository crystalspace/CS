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
// NeXTAssistant.cpp
//
//	Implementation of the iNeXTAssistant interface.
//
//	This object owns the NeXTDelegate, thus NeXTDelegate only gets
//	destroyed when the last reference to this object is removed.
//
//-----------------------------------------------------------------------------
#include "cssysdef.h"
#include "NeXTAssistant.h"
#include "NeXTDelegate.h"
#include "csutil/cfgacc.h"

typedef void* NeXTAssistantHandle;
#define NSD_PROTO(RET,FUNC) extern "C" RET NeXTAssistant_##FUNC
#define NSD_ASSIST(HANDLE) ((iNeXTAssistant*)(HANDLE))

SCF_IMPLEMENT_IBASE(NeXTAssistant)
  SCF_IMPLEMENTS_INTERFACE(iNeXTAssistant)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iEventPlug)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE(NeXTAssistant::eiEventPlug)
  SCF_IMPLEMENTS_INTERFACE(iEventPlug)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
NeXTAssistant::NeXTAssistant(NeXTSystemDriver* p) : system(p)
{
  SCF_CONSTRUCT_IBASE(0);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiEventPlug);
  controller = NeXTDelegate_startup(this);
  event_outlet = system->CreateEventOutlet(&scfiEventPlug);
}


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
NeXTAssistant::~NeXTAssistant()
{
  NeXTDelegate_shutdown(controller);
  event_outlet->DecRef();
}


//-----------------------------------------------------------------------------
// init_menu
//	Generate application menu based upon platform configuration.
//-----------------------------------------------------------------------------
void NeXTAssistant::init_menu(iConfigFile* next_config)
{
  char const* style =
    next_config->GetStr("NeXT.Platform." OS_NEXT_DESCRIPTION ".menu", 0);
  if (style != 0)
    NeXTDelegate_init_app_menu(controller, next_config, style);
}


//-----------------------------------------------------------------------------
// start_event_loop
//-----------------------------------------------------------------------------
void NeXTAssistant::start_event_loop()
{
  csConfigAccess next_config(system->GetObjectRegistry(), "/config/next.cfg",
    true, iConfigManager::PriorityMin);
  init_menu(next_config);
  NeXTDelegate_start_event_loop(controller);
}


//=============================================================================
// iNeXTAssistant Implementation
//=============================================================================
//-----------------------------------------------------------------------------
// C++ iNeXTAssistant Interface
//-----------------------------------------------------------------------------
void NeXTAssistant::request_shutdown()
{
  event_outlet->ImmediateBroadcast(cscmdQuit, 0);
  NeXTDelegate_stop_event_loop(controller);
}

void NeXTAssistant::advance_state()
{
  if (system != 0)
  {
    system->NextFrame();
    if (!continue_running())
      NeXTDelegate_stop_event_loop(controller);
  }
}

bool NeXTAssistant::continue_running()
{ return (system != 0 && !system->Shutdown); }

void NeXTAssistant::application_activated()
{
  if (system != 0)
  {
    system->ResumeVirtualTimeClock();
    event_outlet->ImmediateBroadcast(cscmdFocusChanged,(void*)true);
  }
}

void NeXTAssistant::application_deactivated()
{
  if (system != 0)
  {
    system->SuspendVirtualTimeClock();
    event_outlet->ImmediateBroadcast(cscmdFocusChanged, (void*)false);
  }
}

void NeXTAssistant::flush_graphics_context()
{ NeXTDelegate_flush_graphics_context(controller); }

void NeXTAssistant::hide_mouse_pointer()
{ NeXTDelegate_hide_mouse_pointer(controller); }

void NeXTAssistant::show_mouse_pointer()
{ NeXTDelegate_show_mouse_pointer(controller); }

void NeXTAssistant::dispatch_event(NeXTEvent e, NeXTView v)
{ NeXTDelegate_dispatch_event(controller, e, v); }

void NeXTAssistant::key_down(int raw, int cooked)
{ event_outlet->Key(raw, cooked, true); }

void NeXTAssistant::key_up(int raw, int cooked)
{ event_outlet->Key(raw, cooked, false); }

void NeXTAssistant::mouse_down(int b, int x, int y)
{ event_outlet->Mouse(b, true, x, y); }

void NeXTAssistant::mouse_up(int b, int x, int y)
{ event_outlet->Mouse(b, false, x, y); }

void NeXTAssistant::mouse_moved(int x, int y)
{ event_outlet->Mouse(0, false, x, y); }


//-----------------------------------------------------------------------------
// Pure-C iNeXTAssistant Interface (for use from Objective-C world)
//-----------------------------------------------------------------------------
NSD_PROTO(void,request_shutdown)(NeXTAssistantHandle h)
    { NSD_ASSIST(h)->request_shutdown(); }
NSD_PROTO(void,advance_state)(NeXTAssistantHandle h)
    { NSD_ASSIST(h)->advance_state(); }
NSD_PROTO(int,continue_running)(NeXTAssistantHandle h)
    { return NSD_ASSIST(h)->continue_running(); }
NSD_PROTO(void,application_activated)(NeXTAssistantHandle h)
    { NSD_ASSIST(h)->application_activated(); }
NSD_PROTO(void,application_deactivated)(NeXTAssistantHandle h)
    { NSD_ASSIST(h)->application_deactivated(); }
NSD_PROTO(void,flush_graphics_context)(NeXTAssistantHandle h)
    { NSD_ASSIST(h)->flush_graphics_context(); }
NSD_PROTO(void,hide_mouse_pointer)(NeXTAssistantHandle h)
    { NSD_ASSIST(h)->hide_mouse_pointer(); }
NSD_PROTO(void,show_mouse_pointer)(NeXTAssistantHandle h)
    { NSD_ASSIST(h)->show_mouse_pointer(); }
NSD_PROTO(void,dispatch_event)(NeXTAssistantHandle h, NeXTEvent e, NeXTView v)
    { NSD_ASSIST(h)->dispatch_event(e, v); }
NSD_PROTO(void,key_down)(NeXTAssistantHandle h, int raw, int cooked)
    { NSD_ASSIST(h)->key_down(raw, cooked); }
NSD_PROTO(void,key_up)(NeXTAssistantHandle h, int raw, int cooked)
    { NSD_ASSIST(h)->key_up(raw, cooked); }
NSD_PROTO(void,mouse_down)(NeXTAssistantHandle h, int button, int x, int y)
    { NSD_ASSIST(h)->mouse_down(button, x, y); }
NSD_PROTO(void,mouse_up)(NeXTAssistantHandle h, int button, int x, int y)
    { NSD_ASSIST(h)->mouse_up(button, x, y); }
NSD_PROTO(void,mouse_moved)(NeXTAssistantHandle h, int x, int y)
    { NSD_ASSIST(h)->mouse_moved(x, y); }


//=============================================================================
// iEventPlug Implementation
//=============================================================================
uint NeXTAssistant::eiEventPlug::GetPotentiallyConflictingEvents()
  { return (CSEVTYPE_Keyboard | CSEVTYPE_Mouse); }
uint NeXTAssistant::eiEventPlug::QueryEventPriority(uint type)
  { return 150; }
