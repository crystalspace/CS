/*
    Crystal Space input library
    Copyright (C) 2000 by Andrew Zabolotny <bit@eltech.ru>
    Copyright (C) 2002, 04 by Mathew Sutcliffe <oktal@gmx.co.uk>

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
#include "csutil/inputdef.h"
#include "csutil/csuctransform.h"
#include "iutil/event.h"
#include "iutil/evdefs.h"
#include "csutil/csevent.h"
#include "csutil/event.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

struct csKeyModDef
{
  const char *key;
  int type;
  int num;
};
static const csKeyModDef KeyModifiers [] =
{
  { "LCtrl",	csKeyModifierTypeCtrl,		csKeyModifierNumLeft },
  { "RCtrl",	csKeyModifierTypeCtrl,		csKeyModifierNumRight },
  { "Ctrl",	csKeyModifierTypeCtrl,		csKeyModifierNumAny },
  { "LAlt",	csKeyModifierTypeAlt,		csKeyModifierNumLeft },
  { "RAlt",	csKeyModifierTypeAlt,		csKeyModifierNumRight },
  { "Alt",	csKeyModifierTypeAlt,		csKeyModifierNumAny },
  { "LShift",	csKeyModifierTypeShift,		csKeyModifierNumLeft },
  { "RShift",	csKeyModifierTypeShift,		csKeyModifierNumRight },
  { "Shift",	csKeyModifierTypeShift,		csKeyModifierNumAny },
  { "Num",	csKeyModifierTypeNumLock,	csKeyModifierNumAny },
  { "Scroll", 	csKeyModifierTypeScrollLock,	csKeyModifierNumAny },
  { "Caps",	csKeyModifierTypeCapsLock,	csKeyModifierNumAny },
  { 0, 0, 0 }
};

struct csKeyCodeDef
{
  const char *key;
  utf32_char codeRaw;
  utf32_char codeCooked;
};
static const csKeyCodeDef KeyDefs [] =
{
  { "Esc",	CSKEY_ESC,	   CSKEY_ESC },
  { "Enter",	CSKEY_ENTER,	   CSKEY_ENTER },
  { "Return",	CSKEY_ENTER,	   CSKEY_ENTER },
  { "Tab",	CSKEY_TAB,	   CSKEY_TAB },
  { "Back",	CSKEY_BACKSPACE,   CSKEY_BACKSPACE },
  { "BackSpace",CSKEY_BACKSPACE,   CSKEY_BACKSPACE },
  { "Space",	CSKEY_SPACE,	   CSKEY_SPACE },
  { "Up",	CSKEY_UP,	   CSKEY_UP },
  { "Down",	CSKEY_DOWN,	   CSKEY_DOWN },
  { "Left",	CSKEY_LEFT,	   CSKEY_LEFT },
  { "Right",	CSKEY_RIGHT,	   CSKEY_RIGHT },
  { "PgUp",	CSKEY_PGUP,	   CSKEY_PGUP },
  { "PageUp",	CSKEY_PGUP,	   CSKEY_PGUP },
  { "PgDn",	CSKEY_PGDN,	   CSKEY_PGDN },
  { "PageDown",	CSKEY_PGDN,	   CSKEY_PGDN },
  { "Home",	CSKEY_HOME,	   CSKEY_HOME },
  { "End",	CSKEY_END,	   CSKEY_END },
  { "Ins",	CSKEY_INS,	   CSKEY_INS },
  { "Insert",	CSKEY_INS,	   CSKEY_INS },
  { "Del",	CSKEY_DEL,	   CSKEY_DEL },
  { "Delete",	CSKEY_DEL,	   CSKEY_DEL },
  { "F1",	CSKEY_F1,	   CSKEY_F1 },
  { "F2",	CSKEY_F2,	   CSKEY_F2 },
  { "F3",	CSKEY_F3,	   CSKEY_F3 },
  { "F4",	CSKEY_F4,	   CSKEY_F4 },
  { "F5",	CSKEY_F5,	   CSKEY_F5 },
  { "F6",	CSKEY_F6,	   CSKEY_F6 },
  { "F7",	CSKEY_F7,	   CSKEY_F7 },
  { "F8",	CSKEY_F8,	   CSKEY_F8 },
  { "F9",	CSKEY_F9,	   CSKEY_F9 },
  { "F10",	CSKEY_F10,	   CSKEY_F10 },
  { "F11",	CSKEY_F11,	   CSKEY_F11 },
  { "F12",	CSKEY_F12,	   CSKEY_F12 },
  { "Print",	CSKEY_PRINTSCREEN, CSKEY_PRINTSCREEN },
  { "PrntScrn", CSKEY_PRINTSCREEN, CSKEY_PRINTSCREEN },
  { "Pause",    CSKEY_PAUSE,	   CSKEY_PAUSE },
  { "PadPlus",	CSKEY_PADPLUS,	   '+' },
  { "PadMinus",	CSKEY_PADMINUS,	   '-' },
  { "PadMult",	CSKEY_PADMULT,	   '*' },
  { "PadDiv",	CSKEY_PADDIV,	   '/' },
  { "Pad0",	CSKEY_PAD0,	   '0' },
  { "Pad1",	CSKEY_PAD1,	   '1' },
  { "Pad2",	CSKEY_PAD2,	   '2' },
  { "Pad3",	CSKEY_PAD3,	   '3' },
  { "Pad4",	CSKEY_PAD4,	   '4' },
  { "Pad5",	CSKEY_PAD5,	   '5' },
  { "Center",	CSKEY_CENTER,	   '5' },
  { "Pad6",	CSKEY_PAD6,	   '6' },
  { "Pad7",	CSKEY_PAD7,	   '7' },
  { "Pad8",	CSKEY_PAD8,	   '8' },
  { "Pad9",	CSKEY_PAD9,	   '9' },
  { "PadDecimal",CSKEY_PADDECIMAL, '.' }, // @@@ Not always '.' on all layouts.
  { "PadEnter",	CSKEY_PADENTER,	   CSKEY_ENTER },
  { "Shift",	CSKEY_SHIFT,	   CSKEY_SHIFT },
  { "LShift",	CSKEY_SHIFT_LEFT,  CSKEY_SHIFT_LEFT },
  { "RShift",	CSKEY_SHIFT_RIGHT, CSKEY_SHIFT_RIGHT },
  { "Ctrl",	CSKEY_CTRL,	   CSKEY_CTRL },
  { "LCtrl",	CSKEY_CTRL_LEFT,   CSKEY_CTRL_LEFT },
  { "RCtrl",	CSKEY_CTRL_RIGHT,  CSKEY_CTRL_RIGHT },
  { "Alt",	CSKEY_ALT,	   CSKEY_ALT },
  { "LAlt",	CSKEY_ALT_LEFT,	   CSKEY_ALT_LEFT },
  { "RAlt",	CSKEY_ALT_RIGHT,   CSKEY_ALT_RIGHT },
  { "Num",	CSKEY_PADNUM,	   CSKEY_PADNUM },
  { "Caps",	CSKEY_CAPSLOCK,	   CSKEY_CAPSLOCK },
  { "Scroll",	CSKEY_SCROLLLOCK,  CSKEY_SCROLLLOCK },

  { "Plus",	'+',		   '+'}, // Avoids confusion with the +/- that
  { "Minus",	'-',		   '-'}, //  come between modifiers and key.

  { 0, 0, 0 }
};

static const char* ModToName (csKeyModifierType type, csKeyModifierNumType num)
{
  for (const csKeyModDef *c = KeyModifiers; c->key; c++)
    if (c->type == (int) type && c->num == (int) num) return c->key;
  return 0;
}

static bool NameToMod (const char *name,
		       csKeyModifierType &type, csKeyModifierNumType &num)
{
  for (const csKeyModDef *c = KeyModifiers; c->key; c++)
    if (strcasecmp (name, c->key) == 0)
    {
      type = (csKeyModifierType) c->type;
      num = (csKeyModifierNumType) c->num;
      return true;
    }
  return false;
}

static const char* RawToName (utf32_char raw)
{
  for (const csKeyCodeDef *c = KeyDefs; c->key; c++)
    if (c->codeRaw == raw) return c->key;
  return 0;
}

static utf32_char NameToRaw (const char *name)
{
  for (const csKeyCodeDef *c = KeyDefs; c->key; c++)
    if (strcasecmp (name, c->key) == 0) return c->codeRaw;
  return 0;
}

static utf32_char NameToCooked (const char *name)
{
  for (const csKeyCodeDef *c = KeyDefs; c->key; c++)
    if (strcasecmp (name, c->key) == 0) return c->codeCooked;
  return 0;
}

static utf32_char RawToCooked (utf32_char raw)
{
  for (const csKeyCodeDef *c = KeyDefs; c->key; c++)
    if (c->codeRaw == raw) return c->codeCooked;
  return 0;
}

csInputDefinition::csInputDefinition (iEventNameRegistry* r,
	uint32 honorModifiers, bool useCookedCode) : name_reg(r)
{
  Initialize (honorModifiers, useCookedCode);
}

void csInputDefinition::Initialize (uint32 honorModifiers, bool useCookedCode)
{
  containedName = CS_EVENT_INVALID;
  modifiersHonored = honorModifiers;
  memset (&modifiers, 0, sizeof (modifiers));
  deviceNumber = 0;
  memset (&keyboard, 0, sizeof (keyboard));
  keyboard.isCooked = useCookedCode;
}

csInputDefinition::csInputDefinition (const csInputDefinition &o)
  : name_reg(o.name_reg),
    containedName (o.containedName),
    modifiersHonored (o.modifiersHonored), modifiers (o.modifiers),
    deviceNumber (o.deviceNumber)
{
  memcpy (&keyboard, &o.keyboard, sizeof (keyboard));
}

csInputDefinition::csInputDefinition (iEventNameRegistry* r,
	iEvent* ev, uint32 mods, bool cook) : name_reg(r)
{
  Initialize (mods, cook);
  InitializeFromEvent (ev);
}

csInputDefinition::csInputDefinition (iEventNameRegistry* r,
	iEvent* ev, uint8 axis) : name_reg(r)
{
  Initialize (0, false);
  mouseAxis = axis;
  InitializeFromEvent (ev);
}

void csInputDefinition::InitializeFromEvent (iEvent *ev)
{
  deviceNumber = 0;
  if (CS_IS_KEYBOARD_EVENT(name_reg, *ev))
  {
    containedName = csevKeyboardEvent(name_reg);
    keyboard.code = keyboard.isCooked ? csKeyEventHelper::GetCookedCode (ev)
				      : csKeyEventHelper::GetRawCode (ev);
    csKeyEventHelper::GetModifiers (ev, modifiers);
  }
  else if (CS_IS_MOUSE_EVENT(name_reg, *ev))
  {
    deviceNumber = csMouseEventHelper::GetNumber(ev);
    if (CS_IS_MOUSE_BUTTON_EVENT(name_reg, *ev, deviceNumber))
    {
      containedName = csevMouseButton(name_reg, deviceNumber);
      mouseButton = csMouseEventHelper::GetButton(ev);
      csKeyEventHelper::GetModifiers (ev, modifiers);
    }
    else if (CS_IS_MOUSE_MOVE_EVENT(name_reg, *ev, deviceNumber))
    {
      containedName = csevMouseMove(name_reg, deviceNumber);
      csMouseEventData data;
      csMouseEventHelper::GetEventData (ev, data);
      CS_ALLOC_STACK_ARRAY(bool, axesState, data.numAxes);
      uint32 axesChanged;
      // (vk) shouldn't that value be provided in the csMouseEventData struct ?
      ev->Retrieve ("mAxesChanged", axesChanged);
      uint currentAxis;
      for (currentAxis = 0; currentAxis < data.numAxes; currentAxis++)
      {
        axesState[currentAxis] = (axesChanged & (1 << currentAxis)) != 0;
        if (axesState[currentAxis])
        {
          mouseAxis = currentAxis;
          // @@@ (vk) only consider one axis for now...
          continue;
        }
      }
    }
  }
  else if (CS_IS_JOYSTICK_EVENT(name_reg, *ev))
  {
    deviceNumber = csJoystickEventHelper::GetNumber(ev);
    if (CS_IS_JOYSTICK_BUTTON_EVENT(name_reg, *ev, deviceNumber))
    {
      containedName = csevJoystickButton(name_reg, deviceNumber);
      joystickButton = csJoystickEventHelper::GetButton(ev);
      csKeyEventHelper::GetModifiers (ev, modifiers);
    }
    else if (CS_IS_JOYSTICK_MOVE_EVENT(name_reg, *ev, deviceNumber))
    {
      containedName = csevJoystickMove(name_reg, deviceNumber);
      csJoystickEventData data;
      csJoystickEventHelper::GetEventData (ev, data);
      CS_ALLOC_STACK_ARRAY(bool, axesState, data.numAxes);
      uint currentAxis;
      for (currentAxis = 0; currentAxis < data.numAxes; currentAxis++)
      {
        axesState[currentAxis] = (data.axesChanged & (1 << currentAxis)) != 0;
        if (axesState[currentAxis])
        {
          joystickAxis = currentAxis;
          // @@@ (vk) only consider one axis for now...
          continue;
        }
      }
    }
  }
}

csInputDefinition::csInputDefinition (iEventNameRegistry* r,
	const char *_s, uint32 mods, bool cook) : name_reg(r)
{
  Initialize (mods, cook);

  csString str (_s);
  size_t pos = 0, end;
  char *endp;

  // modifiers (alt, ctrl, shift, etc)
  while ((end = str.FindFirst ("+-", pos)) != (size_t)-1)
  {
    csString mod (str.Slice (pos, end - pos));
    if (mod.IsEmpty()) break;

    csKeyModifierType type;
    csKeyModifierNumType num;
    if (NameToMod (mod, type, num)) modifiers.modifiers[type] |= (1 << num);

    pos = end + 1;
  }

  str.DeleteAt (0, pos);

  // -1 serves as a flag that no device number was parsed from the string
  deviceNumber = (uint) -1;

  // device number
  if (strspn(str.GetData (), "0123456789") > 0)
  {
    deviceNumber = (uint)strtoul (str.GetData (), & endp, 10);
    str.DeleteAt (0, endp - str.GetData ());
  }

  // device name
  if (str.StartsWith ("Mouse", true))
  {
    str.DeleteAt (0, 5);
    if (deviceNumber == (uint)-1) deviceNumber = 0;

    // Mouse sub-device names
    if (str.CompareNoCase ("X"))
    {
      mouseAxis = 0;
      containedName = csevMouseMove(name_reg, deviceNumber);
    }
    else if (str.CompareNoCase ("Y"))
    {
      mouseAxis = 1;
      containedName = csevMouseMove(name_reg, deviceNumber);
    }
    else if (str.StartsWith ("Axis", true))
    {
      str.DeleteAt (0, 4);
      mouseAxis = (int) strtoul (str.GetData (), &endp, 10);
      if (endp != str.GetData ()) containedName = csevMouseMove(
      	name_reg, deviceNumber);
    }
    else
    {
      if (str.StartsWith ("Button", true))
	str.DeleteAt (0, 6);
      bool valid = true;
      if (str.CompareNoCase ("Left"))
	mouseButton = csmbLeft;
      else if (str.CompareNoCase ("Right"))
	mouseButton = csmbRight;
      else if (str.CompareNoCase ("Middle"))
	mouseButton = csmbMiddle;
      else if (str.CompareNoCase ("Extra1"))
	mouseButton = csmbExtra1;
      else if (str.CompareNoCase ("Extra2"))
	mouseButton = csmbExtra2;
      else if (str.CompareNoCase ("WheelUp"))
	mouseButton = csmbWheelUp;
      else if (str.CompareNoCase ("WheelDown"))
	mouseButton = csmbWheelDown;
      else if (str.CompareNoCase ("HWheelLeft"))
	mouseButton = csmbHWheelLeft;
      else if (str.CompareNoCase ("HWheelRight"))
	mouseButton = csmbHWheelRight;
      else
      {
	mouseButton = (int) strtoul (str.GetData (), & endp, 10);
	valid = endp != str.GetData ();
      }
      if (valid)
	containedName = csevMouseButton (name_reg, deviceNumber);
    }
  }
  else if (str.StartsWith ("Joystick", true))
  {
    str.DeleteAt (0, 8);
    if (deviceNumber == (uint)-1) deviceNumber = 0;

    // Joystick sub-device names
    if (str.CompareNoCase ("X"))
    {
      joystickAxis = 0;
      containedName = csevJoystickMove(name_reg, deviceNumber);
    }
    else if (str.CompareNoCase ("Y"))
    {
      joystickAxis = 1;
      containedName = csevJoystickMove(name_reg, deviceNumber);
    }
    else if (str.StartsWith ("Axis", true))
    {
      str.DeleteAt (0, 4);
      joystickAxis = (int) strtoul (str.GetData (), &endp, 10);
      if (endp != str.GetData ()) containedName = csevJoystickMove(
      	name_reg, deviceNumber);
    }
    else if (str.StartsWith ("Button", true))
    {
      str.DeleteAt (0, 6);
      joystickButton = (int) strtoul (str.GetData (), &endp, 10);
      if (endp != str.GetData ()) containedName = csevJoystickButton(
      	name_reg, deviceNumber);
    }
    else
    {
      joystickButton = (int) strtoul (str.GetData (), & endp, 10);
      if (endp != str.GetData ()) containedName = csevJoystickButton(
      	name_reg, deviceNumber);
    }
  }
  else
  {
    containedName = csevKeyboardEvent (name_reg);

    if (deviceNumber != (uint) -1)
    {
      /* this was actually a key, not a device number */
      csString str2 ("");
      str2.Append (deviceNumber);
      str = str2 + str;
    }
    deviceNumber = 0; /* only one logical keyboard, #0 */

    size_t skip = (size_t) csUnicodeTransform::UTF8Skip
      ((utf8_char *) str.GetData (), str.Length ());
    if (skip == str.Length ())
    {
      bool valid;
      csUnicodeTransform::UTF8Decode
        ((utf8_char *) str.GetData (), str.Length (), keyboard.code, &valid);
      if (! valid) keyboard.code = 0;
    }
    else
      keyboard.code = cook ? NameToCooked (str.GetData ())
			   : NameToRaw (str.GetData ());
  }
}

bool csInputDefinition::IsValid () const
{
  if (containedName == csevKeyboardEvent(name_reg))
    return keyboard.code != 0;
  else if (containedName == CS_EVENT_INVALID)
    return false;
  else
    return (csEventNameRegistry::IsKindOf(name_reg, containedName,
    	csevInput(name_reg)));
}

static void AppendMouseButton (csString& str, int mouseButton)
{
  switch (mouseButton)
  {
  case csmbLeft:		str.Append ("Left");		break;
  case csmbRight:		str.Append ("Right");		break;
  case csmbMiddle:		str.Append ("Middle");		break;
  case csmbExtra1:		str.Append ("Extra1");		break;
  case csmbExtra2:		str.Append ("Extra2");		break;
  case csmbWheelUp:		str.Append ("WheelUp");		break;
  case csmbWheelDown:		str.Append ("WheelDown");	break;
  case csmbHWheelLeft:	str.Append ("HWheelLeft");	break;
  case csmbHWheelRight:	str.Append ("HWheelRight");	break;
  default:
    str.Append (mouseButton);
  }
}

csString csInputDefinition::ToString (bool distinguishMods) const
{
  csString str;

  for (int type = 0; type < csKeyModifierTypeLast; type++)
  {
    if ((! distinguishMods && modifiers.modifiers[type] != 0)
     || modifiers.modifiers[type] & (1 << csKeyModifierNumAny))
    {
      str.Append (ModToName ((csKeyModifierType) type, csKeyModifierNumAny));
      str.Append ("+");
    }
    else
      for (int num = 0; num < csKeyModifierNumAny; num++)
        if (modifiers.modifiers[type] & (1 << num))
        {
          const char *name = ModToName ((csKeyModifierType) type,
					(csKeyModifierNumType) num);
          if (name)
          {
            str.Append (name);
            str.Append ("+");
          }
        }
  }

  if (deviceNumber != 0)
    str.Append(deviceNumber);

  CS_ASSERT_MSG(name_reg->GetString (containedName),
	    containedName == csevKeyboardEvent(name_reg) ||
	    containedName == csevMouseButton(name_reg, deviceNumber) ||
	    containedName == csevMouseMove(name_reg, deviceNumber) ||
	    containedName == csevMouseDown(name_reg, deviceNumber) ||
	    containedName == csevMouseUp(name_reg, deviceNumber) ||
	    containedName == csevMouseClick(name_reg, deviceNumber) ||
	    containedName == csevMouseDoubleClick(name_reg, deviceNumber) ||
	    containedName == csevJoystickButton(name_reg, deviceNumber) ||
	    containedName == csevJoystickMove(name_reg, deviceNumber) ||
	    containedName == csevJoystickDown(name_reg, deviceNumber) ||
	    containedName == csevJoystickUp(name_reg, deviceNumber));

  if (containedName == csevKeyboardEvent(name_reg))
  {
    if (CSKEY_IS_SPECIAL (keyboard.code) || (keyboard.code <= 0x20))
	str.Append (RawToName (keyboard.code));
    else
    {
      char buf[CS_UC_MAX_UTF8_ENCODED];
      size_t size = csUnicodeTransform::EncodeUTF8
	  (keyboard.code, (utf8_char *) buf, sizeof (buf));
      str.Append (buf, size);
    }
  }
  else if (containedName == csevMouseDown(name_reg, deviceNumber))
  {
    str.Append ("MouseButton");
    AppendMouseButton (str, mouseButton);
  }
  else if (containedName == csevMouseUp(name_reg, deviceNumber))
  {
    str.Append ("MouseButton");
    AppendMouseButton (str, mouseButton);
  }
  else if (containedName == csevMouseClick(name_reg, deviceNumber))
  {
    str.Append ("MouseButton");
    AppendMouseButton (str, mouseButton);
  }
  else if (containedName == csevMouseDoubleClick(name_reg, deviceNumber))
  {
    str.Append ("MouseButton");
    AppendMouseButton (str, mouseButton);
  }
  else if (containedName == csevMouseButton(name_reg, deviceNumber))
  {
    str.Append ("MouseButton");
    AppendMouseButton (str, mouseButton);
  }
  else if (containedName == csevMouseMove(name_reg, deviceNumber))
  {
    str.Append ("Mouse");
    switch (mouseAxis)
    {
    case 0: str.Append ("X"); break;
    case 1: str.Append ("Y"); break;
    default:
      str.Append ("Axis");
      str.Append (mouseAxis);
    }
  }
  else if (containedName == csevJoystickButton(name_reg, deviceNumber))
  {
    // @@@ (vk) this is never triggered ?
    str.Append ("JoystickButton");
    str.Append (joystickButton);
  }
  else if (containedName == csevJoystickDown(name_reg, deviceNumber))
  {
    str.Append ("JoystickButton");
    str.Append (joystickButton);
  }
  else if (containedName == csevJoystickUp(name_reg, deviceNumber))
  {
    str.Append ("JoystickButton");
    str.Append (joystickButton);
  }
  else if (containedName == csevJoystickMove(name_reg, deviceNumber))
  {
    str.Append ("JoystickAxis");
    str.Append (joystickAxis);
  }

  return str;
}

uint32 csInputDefinition::ComputeHash () const
{
  uint32 hash = csHashComputer<csEventID>::ComputeHash(containedName);
  if (containedName == csevKeyboardEvent(name_reg))
    return (hash<<2) ^ keyboard.code;
  else if (csEventNameRegistry::IsKindOf(name_reg, containedName,
  	csevInput(name_reg)))
    return (hash<<2) ^ mouseButton; /* union - all fields are stored here */
  else /* should never happen... ? */
    return hash;
}

int csInputDefinition::Compare (const csInputDefinition &other) const
{
  if (modifiersHonored || other.modifiersHonored)
    for (int type = 0; type < csKeyModifierTypeLast; type++)
      if (!CSKEY_MODIFIER_COMPARE_MASK (modifiers.modifiers[type],
				        other.modifiers.modifiers[type]))
      {
        return (int)modifiers.modifiers[type] 
          - (int)other.modifiers.modifiers[type];
      }

  if (containedName != other.containedName)
  {
    return (int) (containedName - other.containedName);
  }

  if (deviceNumber != other.deviceNumber)
  {
    return (int)deviceNumber - (int)other.deviceNumber;
  }

  if (containedName == csevKeyboardEvent(name_reg))
  {
    if (keyboard.isCooked)
    {
	// our keyboard code is cooked, so convert 'other'
	// code to cooked if necessary

      if (other.keyboard.isCooked)
        return (int)keyboard.code - (int)other.keyboard.code;
      else
        return (int)keyboard.code - (int)RawToCooked (other.keyboard.code);
    }
    else
    {
	// our keyboard code is not cooked, so convert this
	// code to cooked if necessary

      if (other.keyboard.isCooked)
        return (int)RawToCooked (keyboard.code) - (int)other.keyboard.code;
      else
        return (int)keyboard.code - (int)other.keyboard.code;
    }
  }
  else
    return (int)mouseButton - (int)other.mouseButton;
}

bool csInputDefinition::ParseKey (iEventNameRegistry* reg, const char *str, 
				  utf32_char *raw, utf32_char *cooked, 
				  csKeyModifiers *mods)
{
  csInputDefinition def (reg, str, CSMASK_ALLMODIFIERS, false);
  if (! def.IsValid () || def.GetName () != csevKeyboardEvent (reg))
    return false;
  if (raw) *raw = def.keyboard.code;
  if (cooked)
  {
    if (CSKEY_IS_SPECIAL (def.keyboard.code))
      *cooked = RawToCooked (def.keyboard.code);
    else
      *cooked = def.keyboard.code;
  }
  if (mods) *mods = def.modifiers;
  return true;
}

bool csInputDefinition::ParseOther (iEventNameRegistry* reg,
				    const char *str, csEventID *name, 
				    uint *device, int *num,
				    csKeyModifiers *mods)
{
  csInputDefinition def (reg, str, CSMASK_ALLMODIFIERS);
  if (! def.IsValid ()) return false;
  if (name) *name = def.containedName;
  if (device) *device = def.deviceNumber;
  if (num) *num = def.mouseButton;
  if (mods) *mods = def.modifiers;
  return true;
}

csString csInputDefinition::GetKeyString (iEventNameRegistry* reg,
	utf32_char code, const csKeyModifiers *mods, bool distinguishModifiers)
{
  csInputDefinition def (reg, CSMASK_ALLMODIFIERS);
  def.containedName = csevKeyboardEvent(reg);
  def.keyboard.code = code;
  if (mods) def.modifiers = *mods;
  return def.ToString (distinguishModifiers);
}

csString csInputDefinition::GetOtherString (iEventNameRegistry* reg,
					    csEventID name,
					    uint device, int num,
  const csKeyModifiers *mods, bool distinguishModifiers)
{
  csInputDefinition def (reg, CSMASK_ALLMODIFIERS);
  def.containedName = name;
  def.deviceNumber = device;
  // (vk) for now we don't know if it's mouse or joystick button, so init both
  //  @@@ maybe it would be better to have only one 'Button' variable ?
  def.mouseButton = def.joystickButton = num;
  if (mods) def.modifiers = *mods;
  return def.ToString (distinguishModifiers);
}
