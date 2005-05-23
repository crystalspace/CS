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
  if (!CS_IS_KEYBOARD_EVENT (*event)) return false;

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

int csMouseEventHelper::GetNumber (const iEvent *event)
{
  int res = 0;
  event->Retrieve("mNumber", res);
  return res;
}

int csMouseEventHelper::GetAxis (const iEvent *event, int axis)
{
  const void *_xs; size_t _xs_sz;
  int axs;
  if (event->Retrieve("mAxes", _xs, _xs_sz) != csEventErrNone)
    return 0;
  if (event->Retrieve("mNumAxes", axs) != csEventErrNone)
    return 0;
  const int *axdata = (int *) _xs;
  if ((axis > 0) && (axis <= axs))
    return axdata[axis - 1];
  else
    return 0;
}

int csMouseEventHelper::GetButton (const iEvent *event)
{
  int res = 0;
  event->Retrieve("mButton", res);
  return res;
}

bool csMouseEventHelper::GetEventData (const iEvent* event, 
					csMouseEventData& data)
{
  if (!CS_IS_MOUSE_EVENT (*event)) return false;
  
  csEventError ok = csEventErrNone;
  ok = event->Retrieve("mX", data.x);
  CS_ASSERT(ok == csEventErrNone);
  ok = event->Retrieve("mY", data.y);
  CS_ASSERT(ok == csEventErrNone);
  ok = event->Retrieve("mButton", data.Button);
  CS_ASSERT(ok == csEventErrNone);
  ok = event->Retrieve("keyModifiers", data.Modifiers);
  CS_ASSERT(ok == csEventErrNone);
  return true;
}

int csJoystickEventHelper::GetNumber(const iEvent *event)
{
  int res = 0;
  event->Retrieve("jsNumber", res);
  return res;
}

int csJoystickEventHelper::GetAxis(const iEvent *event, int axis)
{
  const void *_xs; size_t _xs_sz;
  int axs;
  if (event->Retrieve("jsAxes", _xs, _xs_sz) != csEventErrNone)
    return 0;
  if (event->Retrieve("jsNumAxes", axs) != csEventErrNone)
    return 0;
  const int *axdata = (int *) _xs;
  if ((axis > 0) && (axis <= axs))
    return axdata[axis - 1];
  else
    return 0;
}

int csJoystickEventHelper::GetButton(const iEvent *event)
{
  int res = 0;
  event->Retrieve("jsButton", res);
  return res;
}

uint8 csJoystickEventHelper::GetNumAxes(const iEvent *event)
{
  uint8 res = 0;
  event->Retrieve("jsNumAxes", res);
  return res;
}

bool csJoystickEventHelper::GetEventData (const iEvent* event, 
					   csJoystickEventData& data)
{
  if (!CS_IS_JOYSTICK_EVENT (*event)) return false;
  
  data.number = GetNumber(event);
  const void *_ax = 0; size_t _ax_sz = 0;
  csEventError ok = csEventErrNone;
  ok = event->Retrieve("jsAxes", _ax, _ax_sz);
  CS_ASSERT(ok == csEventErrNone);
  ok = event->Retrieve("jsNumAxes", data.numAxes);
  CS_ASSERT(ok == csEventErrNone);
  for (int iter=0 ; iter<CS_MAX_JOYSTICK_AXES ; iter++) {
    if (iter<data.numAxes)
      data.axes[iter] = ((int *)_ax)[iter];
    else
      data.axes[iter] = 0;
  }
  ok = event->Retrieve("jsButton", data.Button);
  CS_ASSERT(ok == csEventErrNone);
  ok = event->Retrieve("keyModifiers", data.Modifiers);
  CS_ASSERT(ok == csEventErrNone);
  return true;
}

uint csCommandEventHelper::GetCode(const iEvent* event)
{
  uint res = 0;
  event->Retrieve("cmdCode", res);
  return res;
}

intptr_t csCommandEventHelper::GetInfo(const iEvent* event)
{
  int64 res = 0;
  event->Retrieve("cmdInfo", res);
  return res;
}

bool csCommandEventHelper::GetEventData(const iEvent* event, 
					 csCommandEventData& data)
{
  if (!CS_IS_COMMAND_EVENT (*event)) return false;

  csEventError ok = csEventErrNone;
  ok = event->Retrieve("cmdCode", data.Code);
  CS_ASSERT(ok == csEventErrNone);
  int64 ipt;
  ok = event->Retrieve("cmdInfo", ipt);
  data.Info = ipt;
  CS_ASSERT(ok == csEventErrNone);
  return true;
}
