/*
    Copyright (C) 1999-2000 by Eric Sunshine <sunshine@sunshineco.com>
  
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

#ifndef __CS_CSBE_H__
#define __CS_CSBE_H__

#include "csutil/scf.h"
#include "csutil/csinput.h"
#include "cssys/system.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "ivideo/graph2d.h" // csMouseCursorID
#include <Handler.h>
#include <MessageQueue.h>
#include "cssys/be/behelp.h"


SCF_VERSION (iBeAssistant, 0, 0, 1);

struct iBeAssistant : public iBase {
  /**
   * A user action (probably sent from the subthread), such as keypress
   * or mouse action, in the form of a BMessage.  The BMessage is
   * placed in a thread-safe message queue and later processed by the
   * Crystal Space event-loop running in the main thread.
   */
  virtual bool UserAction (BMessage*) = 0;

  /**
   * Set the mouse pointer to one of the pre-defined Crystal Space
   * mouse shapes.
   */
  virtual bool SetCursor(csMouseCursorID) = 0;

  /**
   * Spawn a thread in which to run the BApplication and invokes its
   * Run() method.  If the application is already running in the
   * subthread, then this method does nothing.  This extension is
   * requested by 2D driver modules when they are about to place a
   * window on-screen, at which point the BeOS event-loop should be
   * running (independently of whether or not the Crystal Space
   * event-loop is running) so that they can respond to user actions.
   */
  //virtual bool BeginUI() = 0;

  /** 
   * Ask the Crystal Space event-loop to terminate.
   */
  virtual bool Quit() = 0;

  /**
   * Notify Crystal Space that a 2D graphics context is closing.
   */
  virtual bool ContextClose(iGraphics2D*) = 0;
};

class BeAssistant : public iBeAssistant, public BHandler {
//typedef iBeAssistant superclass
protected:
  enum { CSBE_MOUSE_BUTTON_COUNT = 3 };

  bool running;
  BMessageQueue message_queue;
  iEventOutlet* event_outlet;
  bool shift_down;
  bool alt_down;
  bool ctrl_down;
  bool button_state[CSBE_MOUSE_BUTTON_COUNT];
  bool real_mouse;
  bool mouse_moved;
  BPoint mouse_point;
  iObjectRegistry *object_reg;
  iEventQueue *event_queue;

  static int32 ThreadEntry(void*);
  void HideMouse();
  void ShowMouse();
  void DispatchMessage(BMessage*);
  void CheckMouseMoved();
  void DoContextClose(BMessage*);
  void DoMouseMotion(BMessage*);
  void DoMouseAction(BMessage*);
  void DoKeyAction(BMessage*);
  void QueueMouseEvent(int button, bool down, BPoint);
  void CheckButton(int button, int32 buttons, int32 mask, BPoint);
  void CheckButtons(BMessage*);
  void CheckModifiers(BMessage*);
  void CheckModifier(long flags, long mask, int tag, bool& state) const;
  void ClassifyFunctionKey(BMessage*, int& cs_key, int& cs_char) const;
  void ClassifyNormalKey(int, BMessage*, int& cs_key, int& cs_char) const;
  int ClassifyUnicodeKey(BMessage*) const;
  virtual void MessageReceived(BMessage*); // BHandler override.

public:
  BeAssistant(iObjectRegistry*);
  virtual ~BeAssistant();

  bool HandleEvent(iEvent& e);
  virtual bool UserAction (BMessage*);
  virtual bool SetCursor(csMouseCursorID);
  virtual bool Quit();
  virtual bool ContextClose(iGraphics2D*);

  bool SetMouse(csMouseCursorID shape);
  bool RunBeApp();
  bool QueueMessage(BMessage*);
  bool QueueMessage(uint32, void const* = 0);

  struct eiEventPlug : public iEventPlug
  {
    SCF_DECLARE_EMBEDDED_IBASE(BeAssistant);
    virtual uint GetPotentiallyConflictingEvents();
    virtual uint QueryEventPriority(uint type);
  } scfiEventPlug;

  struct eiEventHandler : public iEventHandler
  {
    SCF_DECLARE_EMBEDDED_IBASE (BeAssistant);
    virtual bool HandleEvent (iEvent& e) { return scfParent->HandleEvent(e); }
  } scfiEventHandler;

  SCF_DECLARE_IBASE;
};

class SysSystemDriver : public csSystemDriver
{
public:
  SysSystemDriver(iObjectRegistry* r) : csSystemDriver(r) {}
};

#endif // __CS_CSBE_H__
