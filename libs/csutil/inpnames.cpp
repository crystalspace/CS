/*
    Crystal Space input library
    Copyright (C) 2000 by Andrew Zabolotny <bit@eltech.ru>
    Copyright (C) 2002 by Mathew Sutcliffe <oktal@gmx.co.uk>

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
#include "cssys/csuctransform.h"
#include "iutil/event.h"
#include "csutil/csevent.h"
#include "csutil/inpnames.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

static struct csKeyModDef
{
  const char *key;
  utf32_char code;
} KeyModifiers [] =
{
  {"ctrl",	CSKEY_CTRL},
  {"lctrl",	CSKEY_CTRL_LEFT},
  {"rctrl",	CSKEY_CTRL_RIGHT},
  {"alt",	CSKEY_ALT},
  {"lalt",	CSKEY_ALT_LEFT},
  {"ralt",	CSKEY_ALT_RIGHT},
  {"shift",	CSKEY_SHIFT},
  {"lshift",	CSKEY_SHIFT_LEFT},
  {"rshift",	CSKEY_SHIFT_RIGHT},
  {"num",	CSKEY_PADNUM},
  {"scroll", 	CSKEY_SCROLLLOCK},
  {"caps",	CSKEY_CAPSLOCK},
  { 0,	0}
};

static struct csKeyCodeDef
{
  const char *key;
  utf32_char codeRaw;
  utf32_char codeCooked;
} KeyDefs [] =
{
  { "Esc",	CSKEY_ESC,	   CSKEY_ESC},
  { "Enter",	CSKEY_ENTER,	   CSKEY_ENTER},
  { "Return",	CSKEY_ENTER,	   CSKEY_ENTER},
  { "Tab",	CSKEY_TAB,	   CSKEY_TAB},
  { "Back",	CSKEY_BACKSPACE,   CSKEY_BACKSPACE},
  { "BackSpace",CSKEY_BACKSPACE,   CSKEY_BACKSPACE},
  { "Up",	CSKEY_UP,	   CSKEY_UP},
  { "Down",	CSKEY_DOWN,	   CSKEY_DOWN},
  { "Left",	CSKEY_LEFT,	   CSKEY_LEFT},
  { "Right",	CSKEY_RIGHT,	   CSKEY_RIGHT},
  { "PgUp",	CSKEY_PGUP,	   CSKEY_PGUP},
  { "PageUp",	CSKEY_PGUP,	   CSKEY_PGUP},
  { "PgDn",	CSKEY_PGDN,	   CSKEY_PGDN},
  { "PageDown",	CSKEY_PGDN,	   CSKEY_PGDN},
  { "Home",	CSKEY_HOME,	   CSKEY_HOME},
  { "End",	CSKEY_END,	   CSKEY_END},
  { "Ins",	CSKEY_INS,	   CSKEY_INS},
  { "Insert",	CSKEY_INS,	   CSKEY_INS},
  { "Del",	CSKEY_DEL,	   CSKEY_DEL},
  { "Delete",	CSKEY_DEL,	   CSKEY_DEL},
  { "F1",	CSKEY_F1,	   CSKEY_F1},
  { "F2",	CSKEY_F2,	   CSKEY_F2},
  { "F3",	CSKEY_F3,	   CSKEY_F3},
  { "F4",	CSKEY_F4,	   CSKEY_F4},
  { "F5",	CSKEY_F5,	   CSKEY_F5},
  { "F6",	CSKEY_F6,	   CSKEY_F6},
  { "F7",	CSKEY_F7,	   CSKEY_F7},
  { "F8",	CSKEY_F8,	   CSKEY_F8},
  { "F9",	CSKEY_F9,	   CSKEY_F9},
  { "F10",	CSKEY_F10,	   CSKEY_F10},
  { "F11",	CSKEY_F11,	   CSKEY_F11},
  { "F12",	CSKEY_F12,	   CSKEY_F12},
  { "Print",	CSKEY_PRINTSCREEN, CSKEY_PRINTSCREEN},
  { "PrntScrn", CSKEY_PRINTSCREEN, CSKEY_PRINTSCREEN},
  { "Pause",    CSKEY_PAUSE,	   CSKEY_PAUSE},
  { "PAD+",	CSKEY_PADPLUS,	   '+'},
  { "PAD-",	CSKEY_PADMINUS,	   '-'},
  { "PAD*",	CSKEY_PADMULT,	   '*'},
  { "PAD/",	CSKEY_PADDIV,	   '/'},
  { "PAD0",	CSKEY_PAD0,	   '0'},
  { "PAD1",	CSKEY_PAD1,	   '1'},
  { "PAD2",	CSKEY_PAD2,	   '2'},
  { "PAD3",	CSKEY_PAD3,	   '3'},
  { "PAD4",	CSKEY_PAD4,	   '4'},
  { "Center",	CSKEY_CENTER,	   '5'},
  { "PAD6",	CSKEY_PAD6,	   '6'},
  { "PAD7",	CSKEY_PAD7,	   '7'},
  { "PAD8",	CSKEY_PAD8,	   '8'},
  { "PAD9",	CSKEY_PAD9,	   '9'},
  { "PADDECIMAL",CSKEY_PADDECIMAL, '.'}, // @@@ Not always '.' on all kbd layouts
  { 0,	0		}
};

csInputDefinition::csInputDefinition ()
{
  modifiersHonored = CSMASK_ALLSHIFTS;
  /*keyCode = 0;
  codeIsCooked = false;*/
  memset (&modifiers, 0, sizeof (modifiers));
  memset (&k, 0, MAX(sizeof (k), MAX(sizeof (m), sizeof (j))));
}

csInputDefinition::csInputDefinition (iEvent* event)
{
  csInputDefinition ();
  FromEvent (event);
}

void csInputDefinition::SetHonoredModifiers (uint32 honorModifiers)
{
  modifiersHonored = honorModifiers;
}

uint32 csInputDefinition::GetHonoredModifiers ()
{
  return modifiersHonored;
}

bool csInputDefinition::Parse (const char* string, bool useCooked)
{
  memset (&modifiers, 0, sizeof (modifiers));

  const char* name = string;
  bool ismask;
  do
  {
    ismask = false;
    char* sep = strchr (name, '+');
    if (!sep) break;
    // Hack: don't run "PAD+" through the modifier check
    if (*(sep + 1) == 0) break;

    size_t mln = sep - name;
    CS_ALLOC_STACK_ARRAY(char, modString, mln + 1);

    strncpy (modString, name, mln);
    modString[mln] = 0;

    for (csKeyModDef *m = KeyModifiers; m->key; m++)
    {
      if (CSKEY_MODIFIER_NUM(m->code) == csKeyModifierNumAny)
      {
	if (strncasecmp (m->key, modString, strlen (m->key)) == 0)
        {
	  char* numStr = modString + strlen (m->key);
	  if (*numStr == 0)
	  {
	    modifiers.modifiers[CSKEY_MODIFIER_TYPE(m->code)] |=
	      0xffffffff;
	  }
	  else
	  {
	    int num;
	    if ((sscanf (numStr, "%d", &num) > 0) && 
	      (num < csKeyModifierNumAny))
	    {
	      modifiers.modifiers[CSKEY_MODIFIER_TYPE(m->code)] |=
		(1 << num);
	    }
	  }
	  ismask = true;
	  break;
        }
      }
      else
      {
        if (strcasecmp (m->key, modString) == 0)
	{
	  modifiers.modifiers[CSKEY_MODIFIER_TYPE(m->code)] |=
	    1 << CSKEY_MODIFIER_NUM(m->code);
	  ismask = true;
	  break;
	}
      }
    }

    name = sep + 1;
  } while (ismask);

/*  int mod = 0;
  bool ismask;
  do
  {
    ismask = false;
    for (csKeyMaskDef *m = KeyMasks; m->key; m++)
      if (! strncasecmp (m->key, name, strlen (m->key)))
      {
        if (use_shift) mod |= m->mask;
        name += strlen (m->key);
        ismask = true;
      }
  } while (ismask);*/

  if (! strncasecmp (name, "Mouse", 5))
  {
    name += 5;
    memset (&m, 0, sizeof (m));
    if (*name == 'X' || *name == 'x')
    {
      //*ev = csEvent (0, csevMouseMove, 1, 0, 0, 0);
      m.x = 1;
      containedType = csevMouseMove;
    }
    else if (*name == 'Y' || *name == 'y')
    {
      //*ev = csEvent (0, csevMouseMove, 0, 1, 0, 0);
      m.y = 1;
      containedType = csevMouseMove;
    }
    else 
    {
      if (sscanf (name, "%d", &m.Button) <= 0) return false;
      //*ev = csEvent (0, csevMouseDown, 0, 0, atoi (name), mod);
      containedType = csevMouseDown;
    }
  }
  else if (! strncasecmp (name, "Joystick", 8))
  {
    name += 8;
    memset (&j, 0, sizeof (m));
    j.number = 1;
    if (*name == 'X' || *name == 'x')
    {
      j.x = 1;
      containedType = csevJoystickMove;
    }
    else if (*name == 'Y' || *name == 'y')
    {
      j.y = 1;
      containedType = csevJoystickMove;
    }
    else 
    {
      if (sscanf (name, "%d", &j.Button) <= 0) return false;
      containedType = csevJoystickDown;
    }
/*    if (*name == 'X' || *name == 'x')
      *ev = csEvent (0, csevJoystickMove, 1, 1, 0, 0, 0);
    else if (*name == 'Y' || *name == 'y')
      *ev = csEvent (0, csevJoystickMove, 1, 0, 1, 0, 0);
    else *ev = csEvent (0, csevJoystickDown, 1, 0, 0, atoi (name), mod);*/
  }
  else
  {
    utf32_char code = 0;
    
    for (csKeyCodeDef *c = KeyDefs; c->key; c++)
    {
      if (strcasecmp (c->key, name) == 0) 
      { 
	code = useCooked ? c->codeCooked : c->codeRaw; 
	break; 
      }
    }
    
    if (code != 0)
    {
      k.keyCode = code;
      k.codeIsCooked = useCooked;
    }
    else if (strlen (name) != 1)
      return false;
    else
    {
      if (useCooked)
      {
	k.keyCode = (utf32_char)*name;
	k.codeIsCooked = true;
      }
      else
      {
	utf32_char ch;
	bool valid;
	csUnicodeTransform::UTF8Decode ((utf8_char*)name,
	  strlen (name), ch, &valid);
	if (!valid) return false;
	// @@@ Use I18Ned tolower() (once it's there...)
	k.keyCode = (ch < 256) ? tolower (ch) : ch;
	k.codeIsCooked = false;
      }
    }
    containedType = csevKeyboard;
  }
  return true;
}

csString csInputDefinition::GetDescription ()
{
  // @@@ Implement me
  return "";
}

bool csInputDefinition::FromEvent (iEvent* event, bool useCookedKey)
{
  // @@@ Implement me
  return false;
}

uint32 csInputDefinition::ComputeHash ()
{
  // @@@ Implement me
  return 0;
}

uint32 csInputDefinition::ComputeEventHash (iEvent* event)
{
  // @@@ Implement me
  return 0;
}

static utf32_char RawToCooked (utf32_char raw)
{
  for (csKeyCodeDef *c = KeyDefs; c->key; c++)
  {
    if (c->codeRaw == raw)
      return c->codeCooked;
  }
  return 0;
}

int csInputDefinition::Compare (const csInputDefinition& def)
{
  int d = containedType - def.containedType;
  
  if (d == 0)
  {
    d = modifiersHonored - def.modifiersHonored;
    if (d == 0)
    {
      d = ((int)(csKeyEventHelper::GetModifiersBits (modifiers) & modifiersHonored)) -
	((int)(csKeyEventHelper::GetModifiersBits (def.modifiers) & modifiersHonored));
      if (d == 0)
      {
	switch (containedType)
	{
	  case csevKeyboard:
	    {
	      if (!(k.codeIsCooked ^ def.k.codeIsCooked))
	      {
		d = (int)k.keyCode - def.k.keyCode;
	      }
	      else
	      {
		utf32_char cooked1 = 
		  k.codeIsCooked ? k.keyCode : RawToCooked (k.keyCode);
		utf32_char cooked2 = 
		  def.k.codeIsCooked ? def.k.keyCode : RawToCooked (def.k.keyCode);
		d = (int)cooked1 - (int)cooked2;
	      }
	      if (d == 0)
	      {
		const csKeyModifiers& mod = def.modifiers;
		for (int n = 0; n < csKeyModifierTypeLast; n++)
		{
		  if (modifiersHonored & (1 << n))
		  {
		    /*
		      Modifier compare logic:
		      - If the parsed modifier was generic, allow any modifier of a type.
		      - If the parsed modifier(s) were specific, only allow those.
		    */
		    if (!(((modifiers.modifiers[n] == 0xffffffff) && (mod.modifiers[n] != 0)) || 
		      (((mod.modifiers[n] & modifiers.modifiers[n]) == modifiers.modifiers[n]) && 
		      ((mod.modifiers[n] & ~modifiers.modifiers[n]) == 0))))
		    {
		      d = n;
		    }
		  }
		  if (d != 0) break;
		}
	      }
	    }
	    break;
	  case csevMouseUp:
	  case csevMouseDown:
	    d = m.Button - def.m.Button;
	    break;
	  case csevMouseMove:
	    if (((m.x != 0) && (def.m.x != 0)) ||
	      ((m.y != 0) && (def.m.y != 0)))
	      d = 0;
	    else
	      d = abs (m.y - def.m.y) + abs (m.x - def.m.x);
	    break;
	  case csevJoystickUp:
	  case csevJoystickDown:
	    d = j.Button - def.j.Button;
	    if (d == 0)
	      d =j.number - def.j.number;
	    break;
	  case csevJoystickMove:
	    if (((j.x != 0) && (def.j.x != 0)) ||
	      ((j.y != 0) && (def.j.y != 0)))
	      d = 0;
	    else
	      d = abs (j.y - def.j.y) + abs (j.x - def.j.x);
	    break;
	}
      }
    }
  }

  return d;
}

int csInputDefinition::Compare (iEvent* event)
{
  /*
    @@@ Very similar to Compare (const csInputDefinition&)
    -> try to merge both
   */ 
  int d = containedType - event->Type;
  
  if (d == 0)
  {
    d = ((int)(csKeyEventHelper::GetModifiersBits (modifiers) & modifiersHonored)) -
      ((int)(csKeyEventHelper::GetModifiersBits (event) & modifiersHonored));
    if (d == 0)
    {
      switch (event->Type)
      {
	case csevKeyboard:
	  {
	    if (k.codeIsCooked)
	    {
	      d = (int)k.keyCode - 
		(int)csKeyEventHelper::GetCookedCode (event);
	    }
	    else
	    {
	      d = (int)k.keyCode - 
		(int)csKeyEventHelper::GetRawCode (event);
	    }
	    if (d == 0)
	    {
	      csKeyModifiers mod;
	      csKeyEventHelper::GetModifiers (event, mod);
	      for (int n = 0; n < csKeyModifierTypeLast; n++)
	      {
		if (modifiersHonored & (1 << n))
		{
		  /*
		    Modifier compare logic:
		    - If the parsed modifier was generic, allow any modifier of a type.
		    - If the parsed modifier(s) were specific, only allow those.
		   */
		  if (!(((modifiers.modifiers[n] == 0xffffffff) && (mod.modifiers[n] != 0)) || 
		    (((mod.modifiers[n] & modifiers.modifiers[n]) == modifiers.modifiers[n]) && 
		    ((mod.modifiers[n] & ~modifiers.modifiers[n]) == 0))))
		  {
		    d = n;
		  }
		}
		if (d != 0) break;
	      }
	    }
	  }
	  break;
	case csevMouseUp:
	case csevMouseDown:
	  d = m.Button - event->Mouse.Button;
	  break;
	case csevMouseMove:
	  if (((m.x != 0) && (event->Mouse.x != 0)) ||
	    ((m.y != 0) && (event->Mouse.y != 0)))
	    d = 0;
	  else
	    d = abs (m.y - event->Mouse.y) + abs (m.x - event->Mouse.x);
	  break;
	case csevJoystickUp:
	case csevJoystickDown:
	  d = j.Button - event->Joystick.Button;
	  if (d == 0)
	    d =j.number - event->Joystick.number;
	  break;
	case csevJoystickMove:
	  if (((j.x != 0) && (event->Joystick.x != 0)) ||
	    ((j.y != 0) && (event->Joystick.y != 0)))
	    d = 0;
	  else
	    d = abs (j.y - event->Joystick.y) + abs (j.x - event->Joystick.x);
	  break;
      }
    }
  }

  return d;
}

