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

static struct csKeyModDef
{
  const char *key;
  int type;
  int num;
} KeyModifiers [] =
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
  { "Scroll", 	csKeyModifierTypeCapsLock,	csKeyModifierNumAny },
  { "Caps",	csKeyModifierTypeScrollLock,	csKeyModifierNumAny },
  { 0, 0, 0 }
};

static struct csKeyCodeDef
{
  const char *key;
  utf32_char codeRaw;
  utf32_char codeCooked;
} KeyDefs [] =
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
  for (csKeyModDef *c = KeyModifiers; c->key; c++)
    if (c->type == (int) type && c->num == (int) num) return c->key;
  return 0;
}

static bool NameToMod (const char *name,
		       csKeyModifierType &type, csKeyModifierNumType &num)
{
  for (csKeyModDef *c = KeyModifiers; c->key; c++)
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
  for (csKeyCodeDef *c = KeyDefs; c->key; c++)
    if (c->codeRaw == raw) return c->key;
  return 0;
}

static utf32_char NameToRaw (const char *name)
{
  for (csKeyCodeDef *c = KeyDefs; c->key; c++)
    if (strcasecmp (name, c->key) == 0) return c->codeRaw;
  return 0;
}

static utf32_char NameToCooked (const char *name)
{
  for (csKeyCodeDef *c = KeyDefs; c->key; c++)
    if (strcasecmp (name, c->key) == 0) return c->codeCooked;
  return 0;
}

static utf32_char RawToCooked (utf32_char raw)
{
  for (csKeyCodeDef *c = KeyDefs; c->key; c++)
    if (c->codeRaw == raw) return c->codeCooked;
  return 0;
}

csInputDefinition::csInputDefinition (uint32 honorModifiers, bool useCookedCode)
{
  Initialize (honorModifiers, useCookedCode);
}

void csInputDefinition::Initialize (uint32 honorModifiers, bool useCookedCode)
{
  containedType = 0;
  modifiersHonored = honorModifiers;
  memset (&modifiers, 0, sizeof (modifiers));
  memset (&keyboard, 0, sizeof (keyboard));
  keyboard.isCooked = useCookedCode;
}

csInputDefinition::csInputDefinition (const csInputDefinition &o)
: containedType (o.containedType),
  modifiersHonored (o.modifiersHonored), modifiers (o.modifiers)
{
  memcpy (&keyboard, &o.keyboard, sizeof (keyboard));
}

csInputDefinition::csInputDefinition (iEvent* ev, uint32 mods, bool cook)
{
  Initialize (mods, cook);
  InitializeFromEvent (ev);
}

csInputDefinition::csInputDefinition (iEvent* ev, int axis)
{
  Initialize (0, false);
  mouseAxis = axis;
  InitializeFromEvent (ev);
}

void csInputDefinition::InitializeFromEvent (iEvent *ev)
{
  switch (ev->Type)
  {
    case csevKeyboard:
    containedType = csevKeyboard;
    keyboard.code = keyboard.isCooked ? csKeyEventHelper::GetCookedCode (ev)
				      : csKeyEventHelper::GetRawCode (ev);
    csKeyEventHelper::GetModifiers (ev, modifiers);
    break;

    case csevMouseUp:
    case csevMouseDown:
    containedType = csevMouseDown;
    mouseButton = ev->Mouse.Button;
    csKeyEventHelper::GetModifiers ((uint32) ev->Mouse.Modifiers, modifiers);
    break;

    case csevJoystickUp:
    case csevJoystickDown:
    containedType = csevJoystickDown;
    joystickButton = ev->Joystick.Button;
    csKeyEventHelper::GetModifiers ((uint32) ev->Joystick.Modifiers, modifiers);
    break;

    case csevMouseMove:
    containedType = csevMouseMove;
    break;

    case csevJoystickMove:
    containedType = csevJoystickMove;
    break;
  }
}

csInputDefinition::csInputDefinition (const char *_s, uint32 mods, bool cook)
{
  Initialize (mods, cook);

  csString str (_s);
  size_t pos = 0, end;

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

  if (str.StartsWith ("Mouse", true))
  {
    str.DeleteAt (0, 5);

    if (str.CompareNoCase ("X"))
    {
      mouseAxis = 0;
      containedType = csevMouseMove;
    }
    else if (str.CompareNoCase ("Y"))
    {
      mouseAxis = 1;
      containedType = csevMouseMove;
    }
    else
    {
      char *end;
      mouseButton = (int) strtoul (str.GetData (), & end, 10);
      if (end != str.GetData ()) containedType = csevMouseDown;
    }
  }
  else if (str.StartsWith ("Joystick", true))
  {
    str.DeleteAt (0, 8);

    if (str.CompareNoCase ("X"))
    {
      joystickAxis = 0;
      containedType = csevJoystickMove;
    }
    else if (str.CompareNoCase ("Y"))
    {
      joystickAxis = 1;
      containedType = csevJoystickMove;
    }
    else
    {
      char *end;
      joystickButton = (int) strtoul (str.GetData (), & end, 10);
      if (end != str.GetData ()) containedType = csevJoystickDown;
    }
  }
  else
  {
    containedType = csevKeyboard;

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
  if (containedType == csevKeyboard)
    return keyboard.code != 0;
  else
    return (1 << containedType) & CSMASK_Input;
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

  switch (containedType)
  {
    case csevKeyboard:
      if (CSKEY_IS_SPECIAL (keyboard.code) || (keyboard.code <= 0x20))
	str.Append (RawToName (keyboard.code));
      else
      {
	char buf[CS_UC_MAX_UTF8_ENCODED];
	size_t size = csUnicodeTransform::EncodeUTF8
	  (keyboard.code, (utf8_char *) buf, sizeof (buf));
	str.Append (buf, size);
      }
      break;

    case csevMouseDown:
      str.Append ("Mouse");
      str.Append (mouseButton);
      break;

    case csevMouseMove:
      str.Append ("Mouse");
      str.Append (mouseAxis ? "Y" : "X");
      break;

    case csevJoystickDown:
      str.Append ("Joystick");
      str.Append (joystickButton);
      break;

    case csevJoystickMove:
      str.Append ("Joystick");
      str.Append (joystickAxis ? "Y" : "X");
      break;
  }

  return str;
}

uint32 csInputDefinition::ComputeHash () const
{
  switch (containedType)
  {
    case csevKeyboard:	   return keyboard.code;
    case csevMouseMove:	   return CSMASK_MouseMove | mouseAxis;
    case csevMouseDown:    return CSMASK_MouseDown | mouseButton;
    case csevJoystickMove: return CSMASK_JoystickMove | joystickAxis;
    case csevJoystickDown: return CSMASK_JoystickDown | joystickButton;
    default: return 0;
  }
}

bool csInputDefinition::Compare (const csInputDefinition &other) const
{
  if (modifiersHonored || other.modifiersHonored)
    for (int type = 0; type < csKeyModifierTypeLast; type++)
      if (! CSKEY_MODIFIER_COMPARE (modifiers.modifiers[type],
				    other.modifiers.modifiers[type]))
        return false;

  if (containedType != other.containedType) return false;

  if (containedType == csevKeyboard)
  {
    if (keyboard.isCooked)
    {
      if (other.keyboard.isCooked)
        return keyboard.code == other.keyboard.code;
      else
        return keyboard.code == RawToCooked (other.keyboard.code);
    }
    else
    {
      if (other.keyboard.isCooked)
        return keyboard.code == other.keyboard.code;
      else
        return RawToCooked (keyboard.code) == other.keyboard.code;
    }
  }
  else
    return mouseButton == other.mouseButton;
}

bool csInputDefinition::ParseKey (const char *str, utf32_char *raw,
  utf32_char *cooked, csKeyModifiers *mods)
{
  csInputDefinition def (str, CSMASK_ALLMODIFIERS, false);
  if (! def.IsValid ()) return false;
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

bool csInputDefinition::ParseOther (const char *str, int *type, int *num,
  csKeyModifiers *mods)
{
  csInputDefinition def (str, CSMASK_ALLMODIFIERS);
  if (! def.IsValid ()) return false;
  if (type) *type = def.containedType;
  if (num) *num = def.mouseButton;
  if (mods) *mods = def.modifiers;
  return true;
}

csString csInputDefinition::GetKeyString (utf32_char code,
  const csKeyModifiers *mods, bool distinguishModifiers)
{
  csInputDefinition def (CSMASK_ALLMODIFIERS);
  def.containedType = csevKeyboard;
  def.keyboard.code = code;
  if (mods) def.modifiers = *mods;
  return def.ToString (distinguishModifiers);
}

csString csInputDefinition::GetOtherString (int type, int num,
  const csKeyModifiers *mods, bool distinguishModifiers)
{
  csInputDefinition def (CSMASK_ALLMODIFIERS);
  def.containedType = type;
  def.mouseButton = num;
  if (mods) def.modifiers = *mods;
  return def.ToString (distinguishModifiers);
}
