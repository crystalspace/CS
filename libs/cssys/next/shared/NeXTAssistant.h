#ifndef __NeXT_NeXTAssistant_h
#define __NeXT_NeXTAssistant_h
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
// NeXTAssistant.h
//
//	Implementation of the iNeXTAssistant interface.
//
//	This object owns the NeXTDelegate, thus NeXTDelegate only gets
//	destroyed when the last reference to this object is removed.
//
//-----------------------------------------------------------------------------
#if defined(__cplusplus)

#include "cssys/next/NeXTAssistant.h"
#include "cssys/next/NeXTSystemDriver.h"
#include "iutil/event.h"
#include "isys/system.h"
typedef void* NeXTDelegateHandle;

class NeXTAssistant : public iNeXTAssistant
{
private:
  NeXTSystemDriver* system;
  NeXTDelegateHandle controller;	// Application & window delegate.
  iEventOutlet* event_outlet;		// Shared event outlet.
  bool should_shutdown;			// cscmdQuit was received.
  void init_menu(iConfigFile*);

public:
  NeXTAssistant(NeXTSystemDriver*);
  virtual ~NeXTAssistant();
  void orphan();
  void start_event_loop();

  virtual void request_shutdown();
  virtual void advance_state();
  virtual bool continue_running();
  virtual void application_activated();
  virtual void application_deactivated();
  virtual void flush_graphics_context();
  virtual void hide_mouse_pointer();
  virtual void show_mouse_pointer();
  virtual void dispatch_event(NeXTEvent, NeXTView);
  virtual void key_down(int raw, int cooked);
  virtual void key_up(int raw, int cooked);
  virtual void mouse_down(int button, int x, int y);
  virtual void mouse_up(int button, int x, int y);
  virtual void mouse_moved(int x, int y);

  struct eiEventPlug : public iEventPlug
  {
    SCF_DECLARE_EMBEDDED_IBASE(NeXTAssistant);
    virtual uint GetPotentiallyConflictingEvents();
    virtual uint QueryEventPriority(uint type);
  } scfiEventPlug;

  struct eiEventHandler : public iEventHandler
  {
    SCF_DECLARE_EMBEDDED_IBASE(NeXTAssistant);
    virtual bool HandleEvent(iEvent&);
  } scfiEventHandler;
  friend struct eiEventHandler;

  SCF_DECLARE_IBASE;
};

#else // __cplusplus

#define NSD_PROTO(RET,FUNC) extern RET NeXTAssistant_##FUNC

typedef void* NeXTAssistant;
typedef void* NeXTEvent;
typedef void* NeXTView;

NSD_PROTO(void,request_shutdown)(NeXTAssistant);
NSD_PROTO(void,advance_state)(NeXTAssistant);
NSD_PROTO(int, continue_running)(NeXTAssistant);
NSD_PROTO(void,application_activated)(NeXTAssistant);
NSD_PROTO(void,application_deactivated)(NeXTAssistant);
NSD_PROTO(void,flush_graphics_context)(NeXTAssistant);
NSD_PROTO(void,hide_mouse_pointer)(NeXTAssistant);
NSD_PROTO(void,show_mouse_pointer)(NeXTAssistant);
NSD_PROTO(void,dispatch_event)(NeXTAssistant, NeXTEvent, NeXTView);
NSD_PROTO(void,key_down)(NeXTAssistant, int raw, int cooked);
NSD_PROTO(void,key_up)(NeXTAssistant, int raw, int cooked);
NSD_PROTO(void,mouse_down)(NeXTAssistant, int button, int x, int y);
NSD_PROTO(void,mouse_up)(NeXTAssistant, int button, int x, int y);
NSD_PROTO(void,mouse_moved)(NeXTAssistant, int x, int y);

#undef NSD_PROTO

#endif // __cplusplus

#endif // __NeXT_NeXTAssistant_h
