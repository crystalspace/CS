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

class SysSystemDriver;
struct iObjectRegistry;
struct iEvent;

/**
 * Implementation class for iBeHelper.
 */
class BeHelper : public iBeHelper
{
private:
  SysSystemDriver* sys;

public:
  BeHelper (SysSystemDriver* sys);
  virtual ~BeHelper ();

  SCF_DECLARE_IBASE;
  virtual bool UserAction (BMessage* msg);
  virtual bool SetCursor (csMouseCursorID id);
  virtual bool BeginUI ();
  virtual bool Quit ();
  virtual bool ContextClose (iGraphics2D* g2d);
};

class SysSystemDriver :
  public csSystemDriver, public iEventPlug, public BHandler
{
  typedef csSystemDriver superclass;

public:
  // Public functions for BeHelper.
  bool SetMouse(csMouseCursorID shape);
  bool RunBeApp();
  bool QueueMessage(BMessage*);
  bool QueueMessage(uint32, void const* = 0);

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
  SCF_DECLARE_IBASE_EXT(csSystemDriver);

  SysSystemDriver(iObjectRegistry* object_reg);
  ~SysSystemDriver();

  // Implementation of the system.
  virtual bool Initialize();
  bool HandleEvent(iEvent& ev);

  // Implementation of iEventPlug.
  virtual unsigned int GetPotentiallyConflictingEvents();
  virtual unsigned int QueryEventPriority(unsigned int);

  struct eiEventHandler : public iEventHandler
  {
    SCF_DECLARE_EMBEDDED_IBASE (SysSystemDriver);
    virtual bool HandleEvent (iEvent& e) { return scfParent->HandleEvent (e); }
  } scfiEventHandler;
};

#endif // __CS_CSBE_H__
