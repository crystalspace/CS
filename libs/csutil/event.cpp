/*
    Event system related helpers
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter
              (C) 2005 by Adam D. Bradley <artdodge@cs.bu.edu>

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
#include "csutil/event.h"
#include "csutil/csevent.h"

utf32_char csKeyEventHelper::GetRawCode (const iEvent* event)
{
  uint32 code;
  if (event->Retrieve ("keyCodeRaw", code) != csEventErrNone)
    return 0;
  return code;
}

utf32_char csKeyEventHelper::GetCookedCode (const iEvent* event)
{
  uint32 code;
  if (event->Retrieve ("keyCodeCooked", code) != csEventErrNone)
    return 0;
  return code;
}

void csKeyEventHelper::GetModifiers (const iEvent* event, 
				     csKeyModifiers& modifiers)
{
  memset (&modifiers, 0, sizeof (modifiers));

  const void* mod;
  size_t modSize;
  if (event->Retrieve ("keyModifiers", mod, modSize) != csEventErrNone) 
    return;
  memcpy (&modifiers, mod, MIN (sizeof (modifiers), modSize));
}

void csKeyEventHelper::GetModifiers (uint32 mask, csKeyModifiers& modifiers)
{
  memset (&modifiers, 0, sizeof (modifiers));

  for (int type = 0; type < csKeyModifierTypeLast; type++)
    if (mask & (1 << type))
      modifiers.modifiers[type] = (1 << csKeyModifierNumAny);
}

csKeyEventType csKeyEventHelper::GetEventType (const iEvent* event)
{
  uint8 type;
  if (event->Retrieve ("keyEventType", type) != csEventErrNone)
    return (csKeyEventType)-1;
  return (csKeyEventType)type;
}

bool csKeyEventHelper::GetAutoRepeat (const iEvent* event)
{
  bool autoRep;
  if (event->Retrieve ("keyAutoRepeat", autoRep) != csEventErrNone) 
    return false;
  return autoRep;
}
				    
csKeyCharType csKeyEventHelper::GetCharacterType (const iEvent* event)
{
  uint8 type;
  if (event->Retrieve ("keyCharType", type) != csEventErrNone)
    return (csKeyCharType)-1;
  return (csKeyCharType)type;
}

bool csKeyEventHelper::GetEventData (const iEvent* event, 
				     csKeyEventData& data)
{
  // if (!CS_IS_KEYBOARD_EVENT (*event)) return false; // Need an iObjectRegistry* to do this...

  data.autoRepeat = GetAutoRepeat (event);
  data.charType = GetCharacterType (event);
  data.codeCooked = GetCookedCode (event);
  data.codeRaw = GetRawCode (event);
  data.eventType = GetEventType (event);
  GetModifiers (event, data.modifiers);

  return true;
}

uint32 csKeyEventHelper::GetModifiersBits (const iEvent* event)
{
  csKeyModifiers m;
  GetModifiers (event, m);
  return GetModifiersBits (m);
}

uint32 csKeyEventHelper::GetModifiersBits (const csKeyModifiers& m)
{
  uint32 res = 0;

  for (int n = 0; n < csKeyModifierTypeLast; n++)
  {
    if (m.modifiers[n] != 0)
      res |= (1 << n);
  }

  return res;
}

csEvent *csMouseEventHelper::NewEvent (csRef<iEventNameRegistry> &reg,
  csTicks iTime, csEventID name, csMouseEventType mType, 
  int mx, int my, uint32 axesChanged,
  uint mButton, bool mButtonState, 
  uint32 buttonMask, uint32 mModifiers)
{
  csEvent *ev = new csEvent(iTime, name, false);
  (void)reg; // reg is unused except for this assert so silence the warning
  CS_ASSERT(reg->IsKindOf(name, csevMouseEvent(reg)));
  ev->Add("mNumber", (uint8) 0);
  ev->Add("mEventType", (uint8) (mType+1));
  int32 axes[2] = { mx, my };
  ev->Add("mAxes", (void *) axes, 2 * sizeof(int32)); /* makes copy */
  ev->Add("mNumAxes", (uint8) 2);
  ev->Add("mAxesChanged", (uint32) axesChanged);
  ev->Add("mButton", (uint8) mButton);
  ev->Add("mButtonState", mButtonState);
  ev->Add("mButtonMask", buttonMask);
  ev->Add("keyModifiers", (uint32) mModifiers);
  return ev;
}

csEvent *csMouseEventHelper::NewEvent (csRef<iEventNameRegistry> &reg,
  csTicks iTime, csEventID name, uint8 n, csMouseEventType mType, 
  int x, int y, uint32 axesChanged, 
  uint button, bool buttonState, 
  uint32 buttonMask, uint32 modifiers)
{
  csEvent *ev = new csEvent(iTime, name, false);
  int32 axes[2] = { x, y };
  (void)reg; // reg is unused except for this assert so silence the warning
  CS_ASSERT(reg->IsKindOf(name, csevMouseEvent(reg)));
  ev->Add("mNumber", (uint8) n);
  ev->Add("mEventType", (uint8) (mType+1));
  ev->Add("mAxes", (void *) axes, 2 * sizeof(int)); /* makes copy */
  ev->Add("mNumAxes", (uint8) 2);
  ev->Add("mAxesChanged", (uint32) axesChanged);
  ev->Add("mButton", (uint8) button);
  ev->Add("mButtonState", buttonState);
  ev->Add("mButtonMask", buttonMask);
  ev->Add("keyModifiers", (uint32) modifiers);
  return ev;
}

csEvent *csMouseEventHelper::NewEvent (csRef<iEventNameRegistry> &reg,
  csTicks iTime, csEventID name, uint8 n, csMouseEventType mType,
  const int32 *axes, uint8 numAxes, uint32 axesChanged, 
  uint button, bool buttonState, uint32 buttonMask, uint32 modifiers)
{
  csEvent *ev = new csEvent (iTime, name, false);
  (void)reg; // reg is unused except for this assert so silence the warning
  CS_ASSERT(reg->IsKindOf(name, csevMouseEvent(reg)));
  ev->Add("mNumber", (uint8) n);
  ev->Add("mEventType", (uint8) (mType+1));
  ev->Add("mAxes", (void *) axes, numAxes * sizeof(int)); /* makes copy */
  ev->Add("mNumAxes", (uint8) numAxes);
  ev->Add("mAxesChanged", (uint32) axesChanged);
  ev->Add("mButton", (uint8) button);
  ev->Add("mButtonState", buttonState);
  ev->Add("mButtonMask", buttonMask);
  ev->Add("keyModifiers", (uint32) modifiers);
  return ev;
}

csMouseEventType csMouseEventHelper::GetEventType (const iEvent* event)
{
  uint8 type;
  if (event->Retrieve ("mEventType", type) != csEventErrNone)
    return (csMouseEventType)-1;
  return (csMouseEventType)(type-1);
}

uint csMouseEventHelper::GetNumber (const iEvent *event)
{
  uint8 res = 0;
  event->Retrieve("mNumber", res);
  return res;
}

int csMouseEventHelper::GetAxis (const iEvent *event, uint axis)
{
  const void *_xs; size_t _xs_sz;
  uint8 axs;
  if (event->Retrieve("mAxes", _xs, _xs_sz) != csEventErrNone)
    return 0;
  if (event->Retrieve("mNumAxes", axs) != csEventErrNone)
    return 0;
  const int32 *axdata = (int32 *) _xs;
  if (axis < axs)
    return axdata[axis];
  else
    return 0;
}

uint csMouseEventHelper::GetButton (const iEvent *event)
{
  uint8 res = 0;
  event->Retrieve("mButton", res);
  return res;
}

bool csMouseEventHelper::GetButtonState (const iEvent *event)
{
  bool res = false;
  event->Retrieve("mButtonState", res);
  return res;
}

uint32 csMouseEventHelper::GetButtonMask (const iEvent *event)
{
  uint32 res = 0;
  event->Retrieve("mButtonMask", res);
  return res;
}

uint csMouseEventHelper::GetNumAxes(const iEvent *event)
{
  uint8 res = 0;
  event->Retrieve("mNumAxes", res);
  return res;
}

bool csMouseEventHelper::GetEventData (const iEvent* event, 
				       csMouseEventData& data)
{
  // if (!CS_IS_MOUSE_EVENT (*event)) return false; // Need an iObjectRegistry* to do this...

  const void *_ax = 0; size_t _ax_sz = 0;
  uint8 ui8;
  csEventError ok = csEventErrNone;
  ok = event->Retrieve("mAxes", _ax, _ax_sz);
  CS_ASSERT(ok == csEventErrNone);
  ok = event->Retrieve("mNumAxes", ui8);
  CS_ASSERT(ok == csEventErrNone);
  data.numAxes = ui8;
  for (uint iter=0 ; iter<CS_MAX_MOUSE_AXES ; iter++)
  {
    if (iter<data.numAxes)
      data.axes[iter] = ((int32*)_ax)[iter];
    else
      data.axes[iter] = 0;
  }
  data.x = data.axes[0];
  data.y = data.axes[1];
  ok = event->Retrieve("mButton", ui8);
  CS_ASSERT(ok == csEventErrNone);
  data.Button = ui8;
  ok = event->Retrieve("keyModifiers", data.Modifiers);
  CS_ASSERT(ok == csEventErrNone);
  return true;
}

//---------------------------------------------------------------------------

csEvent *csJoystickEventHelper::NewEvent (csRef<iEventNameRegistry> &reg,
  csTicks iTime, csEventID name, int n, 
  int x, int y, uint32 axesChanged, 
  uint button, bool buttonState, 
  uint32 buttonMask, uint32 modifiers)
{
  csEvent *ev = new csEvent (iTime, name, false);
  int axes[2] = { x, y };
  (void)reg; // reg is unused except for this assert so silence the warning
  CS_ASSERT(reg->IsKindOf(name, csevJoystickEvent(reg)));
  ev->Add("jsNumber", (uint8) n);
  ev->Add("jsAxes", (void *) axes, 2 * sizeof(int)); /* makes copy */
  ev->Add("jsNumAxes", (uint8) 2);
  ev->Add("jsAxesChanged", (uint32) axesChanged);
  ev->Add("jsButton", (uint8) button);
  ev->Add("jsButtonState", buttonState);
  ev->Add("jsButtonMask", buttonMask);
  ev->Add("keyModifiers", (uint32) modifiers);
  return ev;
}

csEvent *csJoystickEventHelper::NewEvent (csRef<iEventNameRegistry> &reg,
  csTicks iTime, csEventID name, int n, const int32 *axes, 
  uint8 numAxes, uint32 axesChanged, 
  uint button, bool buttonState,
  uint32 buttonMask, uint32 modifiers)
{
  csEvent *ev = new csEvent (iTime, name, false);
  (void)reg; // reg is unused except for this assert so silence the warning
  CS_ASSERT(reg->IsKindOf(name, csevJoystickEvent(reg)));
  ev->Add("jsNumber", (uint8) n);
  ev->Add("jsAxes", (void *) axes, numAxes * sizeof(int)); /* makes copy */
  ev->Add("jsNumAxes", (uint8) numAxes);
  ev->Add("jsAxesChanged", (uint32) axesChanged);
  ev->Add("jsButton", (uint8) button);
  ev->Add("jsButtonState", buttonState);
  ev->Add("jsButtonMask", buttonMask);
  ev->Add("keyModifiers", (uint32) modifiers);
  return ev;
}

uint csJoystickEventHelper::GetNumber(const iEvent *event)
{
  uint8 res = 0;
  event->Retrieve("jsNumber", res);
  return res;
}

int csJoystickEventHelper::GetAxis(const iEvent *event, uint axis)
{
  const void *_xs; size_t _xs_sz;
  uint8 axs;
  if (event->Retrieve("jsAxes", _xs, _xs_sz) != csEventErrNone)
    return 0;
  if (event->Retrieve("jsNumAxes", axs) != csEventErrNone)
    return 0;
  const int *axdata = (int *) _xs;
  if (axis < axs)
    return axdata[axis];
  else
    return 0;
}

uint csJoystickEventHelper::GetButton (const iEvent *event)
{
  uint8 res = 0;
  event->Retrieve("jsButton", res);
  return res;
}

bool csJoystickEventHelper::GetButtonState(const iEvent *event)
{
  bool res = false;
  event->Retrieve("jsButtonState", res);
  return res;
}

uint32 csJoystickEventHelper::GetButtonMask (const iEvent *event)
{
  uint32 res = 0;
  event->Retrieve("jsButtonMask", res);
  return res;
}

uint csJoystickEventHelper::GetNumAxes (const iEvent *event)
{
  uint8 res = 0;
  event->Retrieve("jsNumAxes", res);
  return res;
}

bool csJoystickEventHelper::GetEventData (const iEvent* event, 
					  csJoystickEventData& data)
{
  // if (!CS_IS_JOYSTICK_EVENT (*event)) return false; // Need an iObjectRegistry* to do this...
  
  const void *_ax = 0; size_t _ax_sz = 0;
  uint8 ui8;
  csEventError ok = csEventErrNone;
  ok = event->Retrieve("jsNumber", ui8);
  data.number = ui8;
  CS_ASSERT(ok == csEventErrNone);
  ok = event->Retrieve("jsAxes", _ax, _ax_sz);
  CS_ASSERT(ok == csEventErrNone);
  ok = event->Retrieve("jsNumAxes", ui8);
  data.numAxes = ui8;
  CS_ASSERT(ok == csEventErrNone);
  for (uint iter=0 ; iter<CS_MAX_JOYSTICK_AXES ; iter++)
  {
    if (iter<data.numAxes)
      data.axes[iter] = ((int32 *)_ax)[iter];
    else
      data.axes[iter] = 0;
  }
  ok = event->Retrieve("jsAxesChanged", data.axesChanged);
  CS_ASSERT(ok == csEventErrNone);
  ok = event->Retrieve("jsButton", ui8);
  CS_ASSERT(ok == csEventErrNone);
  data.Button = ui8;
  ok = event->Retrieve("keyModifiers", data.Modifiers);
  CS_ASSERT(ok == csEventErrNone);
  return true;
}

uint csInputEventHelper::GetButton (iEventNameRegistry *reg,
	const iEvent* event)
{
  CS_ASSERT(CS_IS_INPUT_EVENT(reg, *event));

  if (CS_IS_MOUSE_EVENT(reg, *event))
  {
    return csMouseEventHelper::GetButton(event);
  }
  else if (CS_IS_JOYSTICK_EVENT(reg, *event))
  {
    return csJoystickEventHelper::GetButton(event);
  }
  else if (CS_IS_KEYBOARD_EVENT(reg, *event))
  {
    return 0;
  }
  else
  {
    return 0;
  }
}

bool csInputEventHelper::GetButtonState (iEventNameRegistry *reg,
	const iEvent* event)
{
  CS_ASSERT(CS_IS_INPUT_EVENT(reg, *event));

  if (CS_IS_MOUSE_EVENT(reg, *event))
  {
    return csMouseEventHelper::GetButtonState (event);
  }
  else if (CS_IS_JOYSTICK_EVENT(reg, *event))
  {
    return csJoystickEventHelper::GetButtonState (event);
  }
  else if (CS_IS_KEYBOARD_EVENT(reg, *event))
  {
    return ((csKeyEventHelper::GetEventType (event) == csKeyEventTypeDown)
	    ?true:false);
  }
  else
  {
    return false;
  }
}

//---------------------------------------------------------------------------

csEvent *csCommandEventHelper::NewEvent (csTicks iTime,
  csEventID name, bool broadcast, intptr_t cInfo)
{
  csEvent *ev = new csEvent(iTime, name, broadcast);
  ev->Add("cmdInfo", (int64)cInfo);
  return ev;
}

uint csCommandEventHelper::GetCode(const iEvent* event)
{
  uint32 res = 0;
  event->Retrieve("cmdCode", res);
  return res;
}

intptr_t csCommandEventHelper::GetInfo(const iEvent* event)
{
  int64 res = 0;
  event->Retrieve("cmdInfo", res);
  return res;
}

bool csCommandEventHelper::GetEventData (const iEvent* event, 
					 csCommandEventData& data)
{
  csEventError ok = csEventErrNone;
  uint32 ui32;
  ok = event->Retrieve("cmdCode", ui32);
  CS_ASSERT(ok == csEventErrNone);
  data.Code = ui32;
  int64 ipt;
  ok = event->Retrieve("cmdInfo", ipt);
  data.Info = ipt;
  CS_ASSERT(ok == csEventErrNone);
  return true;
}

//---------------------------------------------------------------------------

class csWeakEventHandler :
  public scfImplementation1<csWeakEventHandler, iEventHandler>
{
private:
  csWeakRef<iEventHandler> parent;

public:
  csWeakEventHandler (iEventHandler *parent) :
    scfImplementationType (this), parent(parent) { }

  bool HandleEvent (iEvent &e)
  { return parent->HandleEvent(e); }
  const char * GenericName() const
  { return parent->GenericName(); }
  csHandlerID GenericID (csRef<iEventHandlerRegistry> &r) const
  { return parent->GenericID(r); }
  const csHandlerID * GenericPrec (
    csRef<iEventHandlerRegistry> &hr, csRef<iEventNameRegistry> &nr,
    csEventID id) const
  { return parent->GenericPrec(hr, nr, id); }
  const csHandlerID * GenericSucc (
    csRef<iEventHandlerRegistry> &hr, csRef<iEventNameRegistry> &nr,
    csEventID id) const
  { return parent->GenericSucc(hr, nr, id); }
  const csHandlerID * InstancePrec (
    csRef<iEventHandlerRegistry> &hr, csRef<iEventNameRegistry> &nr,
    csEventID id) const
  { return parent->InstancePrec(hr, nr, id); }
  const csHandlerID * InstanceSucc (
    csRef<iEventHandlerRegistry> &hr, csRef<iEventNameRegistry> &nr,
    csEventID id) const
  { return parent->InstanceSucc(hr, nr, id); }
};

csHandlerID RegisterWeakListener (iEventQueue *q, iEventHandler *listener,
  csRef<iEventHandler> &handler)
{
  handler.AttachNew (new csWeakEventHandler (listener));
  return q->RegisterListener (handler);
}

csHandlerID RegisterWeakListener (iEventQueue *q, iEventHandler *listener,
  const csEventID &ename, csRef<iEventHandler> &handler)
{
  handler.AttachNew (new csWeakEventHandler (listener));
  return q->RegisterListener (handler, ename);
}

csHandlerID RegisterWeakListener (iEventQueue *q, iEventHandler *listener,
  const csEventID ename[], csRef<iEventHandler> &handler)
{
  handler.AttachNew (new csWeakEventHandler (listener));
  return q->RegisterListener (handler, ename);
}

void RemoveWeakListener (iEventQueue *q, csRef<iEventHandler> &handler)
{
  q->RemoveListener(handler);
}
