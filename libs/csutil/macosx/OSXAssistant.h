#ifndef __MACOSX_OSXAssistant_h
#define __MACOSX_OSXAssistant_h
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
// OSXAssistant.h
//
//	Implementation of the iOSXAssistant interface.
//
//	This object owns the OSXDelegate, thus OSXDelegate only gets
//	destroyed when the last reference to this object is removed.
//
//-----------------------------------------------------------------------------
#if defined(__cplusplus)

#include "csutil/macosx/OSXAssistant.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
struct iEventQueue;
struct iObjectRegistry;
struct iVirtualClock;
struct iConfigFile;
typedef void* OSXDelegateHandle;

SCF_VERSION (iOSXAssistantLocal, 0, 0, 2);
struct iOSXAssistantLocal : public iOSXAssistant
{
  virtual void start_event_loop() = 0;
};

class OSXAssistant : public iOSXAssistantLocal
{
private:
  OSXDelegateHandle controller;		// Application & window delegate.
  iObjectRegistry* registry;		// Global shared-object registry.
  csRef<iEventQueue> event_queue;	// Global event queue.
  csRef<iEventOutlet> event_outlet;	// Shared event outlet.
  csRef<iVirtualClock> virtual_clock;	// Global virtual clock.
  bool should_shutdown;			// cscmdQuit was received.
  bool run_always;		        // Does the run loop process events
                                        // when the app is not focused?
  csRef<iObjectRegistry> get_registry();
  csRef<iEventQueue> get_event_queue();
  csRef<iVirtualClock> get_virtual_clock();
  void init_menu(iConfigFile*);
  void init_runmode();

public:
  OSXAssistant(iObjectRegistry*);
  virtual ~OSXAssistant();
  virtual void start_event_loop();
  virtual void request_shutdown();
  virtual void advance_state();
  virtual bool always_runs();
  virtual bool continue_running();
  virtual void application_activated();
  virtual void application_deactivated();
  virtual void application_hidden();
  virtual void application_unhidden();
  virtual void flush_graphics_context();
  virtual void hide_mouse_pointer();
  virtual void show_mouse_pointer();
  virtual void dispatch_event(OSXEvent, OSXView);
  virtual void key_down(unsigned int raw, unsigned int cooked);
  virtual void key_up(unsigned int raw, unsigned int cooked);
  virtual void mouse_down(int button, int x, int y);
  virtual void mouse_up(int button, int x, int y);
  virtual void mouse_moved(int x, int y);

  struct eiEventPlug : public iEventPlug
  {
    SCF_DECLARE_EMBEDDED_IBASE(OSXAssistant);
    virtual uint GetPotentiallyConflictingEvents();
    virtual uint QueryEventPriority(uint type);
  } scfiEventPlug;

  struct eiEventHandler : public iEventHandler
  {
    SCF_DECLARE_EMBEDDED_IBASE(OSXAssistant);
    virtual bool HandleEvent(iEvent&);
  } scfiEventHandler;
  friend struct eiEventHandler;

  SCF_DECLARE_IBASE;
};

#else // __cplusplus

#define NSD_PROTO(RET,FUNC) extern RET OSXAssistant_##FUNC

typedef void* OSXAssistant;
typedef void* OSXEvent;
typedef void* OSXView;

NSD_PROTO(void,request_shutdown)(OSXAssistant);
NSD_PROTO(void,advance_state)(OSXAssistant);
NSD_PROTO(int, always_runs)(OSXAssistant);
NSD_PROTO(int, continue_running)(OSXAssistant);
NSD_PROTO(void,application_activated)(OSXAssistant);
NSD_PROTO(void,application_deactivated)(OSXAssistant);
NSD_PROTO(void,application_hidden)(OSXAssistant);
NSD_PROTO(void,application_unhidden)(OSXAssistant);
NSD_PROTO(void,flush_graphics_context)(OSXAssistant);
NSD_PROTO(void,hide_mouse_pointer)(OSXAssistant);
NSD_PROTO(void,show_mouse_pointer)(OSXAssistant);
NSD_PROTO(void,dispatch_event)(OSXAssistant, OSXEvent, OSXView);
NSD_PROTO(void,key_down)(OSXAssistant, unsigned int raw, unsigned int cooked);
NSD_PROTO(void,key_up)(OSXAssistant, unsigned int raw, unsigned int cooked);
NSD_PROTO(void,mouse_down)(OSXAssistant, int button, int x, int y);
NSD_PROTO(void,mouse_up)(OSXAssistant, int button, int x, int y);
NSD_PROTO(void,mouse_moved)(OSXAssistant, int x, int y);

#undef NSD_PROTO

#endif // __cplusplus

#endif // __MACOSX_OSXAssistant_h
