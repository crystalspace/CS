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
#include "csutil/csuctransform.h"
#include "iutil/event.h"
#include "iutil/evdefs.h"
#include "csutil/csevent.h"
#include "csutil/event.h"
#include "csutil/inpnames.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

static struct csKeyModDef
{
  const char *key;
  utf32_char code;
} KeyModifiers [] =
{
  {"LCtrl",	CSKEY_CTRL_LEFT},
  {"RCtrl",	CSKEY_CTRL_RIGHT},
  {"Ctrl",	CSKEY_CTRL},
  {"LAlt",	CSKEY_ALT_LEFT},
  {"RAlt",	CSKEY_ALT_RIGHT},
  {"Alt",	CSKEY_ALT},
  {"LShift",	CSKEY_SHIFT_LEFT},
  {"RShift",	CSKEY_SHIFT_RIGHT},
  {"Shift",	CSKEY_SHIFT},
  {"Num",	CSKEY_PADNUM},
  {"Scroll", 	CSKEY_SCROLLLOCK},
  {"Caps",	CSKEY_CAPSLOCK},
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
  { "Space", CSKEY_SPACE, CSKEY_SPACE},
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
  { "PADDECIMAL",CSKEY_PADDECIMAL, '.'}, // @@@ Not always '.' on all layouts.
  { 0,	0 }
};

//---------------------------------------------------------------------------

static bool ParseModifier (const char* str, int& type, int& number)
{
  for (csKeyModDef *m = KeyModifiers; m->key; m++)
  {
    if (CSKEY_MODIFIER_NUM(m->code) == csKeyModifierNumAny)
    {
      if (strncasecmp (m->key, str, strlen (m->key)) == 0)
      {
	type = CSKEY_MODIFIER_TYPE(m->code);
	const char* numStr = str + strlen (m->key);
	if (*numStr == 0)
	{
	  number = csKeyModifierNumAny;
	}
	else
	{
	  int num;
	  if ((sscanf (numStr, "%d", &num) > 0) && 
	    (num >= 0) && (num < csKeyModifierNumAny))
	  {
	    number = num;
	  }
	}
	return true;
      }
    }
    else
    {
      if (strcasecmp (m->key, str) == 0)
      {
	number = CSKEY_MODIFIER_NUM(m->code);
	type = CSKEY_MODIFIER_TYPE(m->code);
	return true;
      }
    }
  }
  return false;
}

static const char* ParseModifiers (const char* str, csKeyModifiers& modifiers)
{
  memset (&modifiers, 0, sizeof (modifiers));

  const char* name = str;
  bool ismask;
  do
  {
    ismask = false;
    char const* sep = strpbrk (name, "+-");
    if (!sep) break;
    // Hack: don't run "PAD+"/"PAD-" through the modifier check
    if (*(sep + 1) == 0) break;

    size_t mln = sep - name;
    CS_ALLOC_STACK_ARRAY(char, modString, mln + 1);

    strncpy (modString, name, mln);
    modString[mln] = 0;

    //for (csKeyModDef *m = KeyModifiers; m->key; m++)
    //{
    int t, n;
    if (ParseModifier (modString, t, n))
    {
      if (n == csKeyModifierNumAny)
      {
	modifiers.modifiers[t] |= 0xffffffff;
      }
      else
      {
	modifiers.modifiers[t] |= (1 << n);
      }
      ismask = true;
    }

    name = sep + 1;
  } while (ismask);

  return name;
}

static bool ParseKey (const char* key, utf32_char* rawCode,
  utf32_char* cookedCode)
{
  for (csKeyCodeDef *c = KeyDefs; c->key; c++)
  {
    if (strcasecmp (c->key, key) == 0) 
    { 
      if (rawCode) *rawCode = c->codeRaw;
      if (cookedCode) *cookedCode = c->codeCooked;
      return true;
    }
  }

  int t, n;
  if (ParseModifier (key, t, n))
  {
    if (rawCode) *rawCode = CSKEY_MODIFIER (t, n);
    if (cookedCode) *cookedCode = CSKEY_MODIFIER (t, csKeyModifierNumAny);;
    return true;
  }

  size_t const x = strlen(key);
  if ((uint)csUnicodeTransform::UTF8Skip((utf8_char*)key, x) < x)
    return false; // It's more than 1 character, so must be misspelled keyname.

  utf32_char ch;
  bool valid;
  csUnicodeTransform::UTF8Decode ((utf8_char*)key,
    strlen (key), ch, &valid);
  if (!valid) return false;
  // @@@ Use I18Ned tolower() (once it's there...)
  if (rawCode) *rawCode = (ch < 256) ? tolower (ch) : ch;
  if (cookedCode) *cookedCode = 0; // @@@ Or ch? 

  return true;
}

bool csParseKeyDef (const char* str, utf32_char* rawCode,
  utf32_char* cookedCode, csKeyModifiers* modifiers)
{
  csKeyModifiers m;

  const char* name = ParseModifiers (str, m);
  if (!name) return false;

  if (!ParseKey (name, rawCode, cookedCode)) return false;
  if (modifiers) *modifiers = m;

  return true;
}

static const char* GetModifierStr (uint num, uint type)
{
  for (csKeyModDef* md = KeyModifiers; md->key != 0; md++)
  {
    if (md->code == CSKEY_MODIFIER (num, type))
    {
      return md->key;
    }
  }
  return 0;
}

csString csGetKeyDesc (utf32_char code, const csKeyModifiers* modifiers,
		       bool distinguishModifiers)
{
  csString ret;

  if (modifiers)
  {
    for (uint m = 0; m < csKeyModifierTypeLast; m++)
    {
      if (modifiers->modifiers[m] == 0) continue;

      if (distinguishModifiers && 
	((modifiers->modifiers[m] & 0x80000000) == 0))
      {
	for (uint t = 0; t < csKeyModifierNumAny; t++)
	{
	  if (modifiers->modifiers[m] & (1 << t))
	  {
	    const char* key = GetModifierStr (m, t);
	    if (key) 
	      ret << key << '+';
	    else
	    {
	      key = GetModifierStr (m, csKeyModifierNumAny);
	      if (key) ret << key << t << '+';
	    }
	  }
	}
      }
      else
      {
	const char* key = GetModifierStr (m, csKeyModifierNumAny);
	if (key) ret << key << '+';
      }
    }
  }

  if (CSKEY_IS_MODIFIER(code))
  {
    for (csKeyModDef *m = KeyModifiers; m->key; m++)
    {
      if (CSKEY_MODIFIER_TYPE(code) == CSKEY_MODIFIER_TYPE(m->code))
      {
	if (m->code == code)
	{
	  ret << m->key;
	  return ret;
	}
	else if (CSKEY_MODIFIER_NUM(m->code) == csKeyModifierNumAny)
	{
	  ret << m->key << CSKEY_MODIFIER_NUM(code);
	  return ret;
	}
      }
    }
    return "";
  }

  for (csKeyCodeDef *c = KeyDefs; c->key; c++)
  {
    if (c->codeRaw == code)
    {
      ret << c->key;
      return ret;
    }
  }

  // @@@ Use I18Ned toupper() (once it's there...)
  if ((code < 256) && (islower (code))) code = toupper (code);
  utf8_char keyStr[CS_UC_MAX_UTF8_ENCODED + 1];
  int keySize = csUnicodeTransform::EncodeUTF8 (code, keyStr, 
    sizeof (keyStr) / sizeof (utf8_char));

  if (keySize == 0) return "";
  keyStr[keySize] = 0;
  ret << (char*)keyStr;

  return ret;
}

bool csParseMoverDef(const char* prefix, int length, const char* str,
  int* x, int* y, int* button, csKeyModifiers* modifiers)
{
  if (x) *x = 0;
  if (y) *y = 0;
  if (button) *button = -1;
  if (modifiers) memset(modifiers->modifiers, 0, sizeof(modifiers->modifiers));
  csKeyModifiers m;
  const char* name = ParseModifiers (str, m);
  if (strncasecmp(prefix, name, length)) return false;
  name += length;
  if (modifiers) *modifiers = m;
  if (!strcasecmp ("x", name)) { if (x) *x = 1; return true; }
  if (!strcasecmp ("y", name)) { if (y) *y = 1; return true; }
  if (!isdigit(name[0] & 0x7f)) return false;
  if (button) *button = atoi(name);
  return true;
}

bool csParseMouseDef(const char* str, int* x, int* y, 
    int* button, csKeyModifiers* modifiers)
{
  return csParseMoverDef("mouse", 5, str, x, y, button, modifiers);
}

bool csParseJoystickDef(const char* str, int* x, int* y, 
    int* button, csKeyModifiers* modifiers)
{
  return csParseMoverDef("joystick", 8, str, x, y, button, modifiers);
}

static bool
csGetMoverDef(csString& ret, const char* prefix, int x, int y, int button,
	      const csKeyModifiers* modifiers, bool distinguishModifiers)
{
  ret = "";
  if (modifiers)
  {
    for (uint m = 0; m < csKeyModifierTypeLast; m++)
    {
      if (modifiers->modifiers[m] == 0) continue;

      if (distinguishModifiers && 
	((modifiers->modifiers[m] & 0x80000000) == 0))
      {
	for (uint t = 0; t < csKeyModifierNumAny; t++)
	{
	  if (modifiers->modifiers[m] & (1 << t))
	  {
	    const char* key = GetModifierStr (m, t);
	    if (key) 
	      ret << key << '+';
	    else
	    {
	      key = GetModifierStr (m, csKeyModifierNumAny);
	      if (key) ret << key << t << '+';
	    }
	  }
	}
      }
      else
      {
	const char* key = GetModifierStr (m, csKeyModifierNumAny);
	if (key) ret << key << '+';
      }
    }
  }

  ret << prefix;

  if (x)
    ret << 'X';
  else if (y)
    ret << 'Y';
  else if (button)
    ret << button;
  else
    return false;

  return true;
}

csString csGetMouseDesc (int x, int y, int button,
			  const csKeyModifiers* modifiers,
			  bool distinguishModifiers)
{
  
  csString ret;
  (void)csGetMoverDef(ret, "Mouse", x, y, button,
		      modifiers, distinguishModifiers);
  return ret;
    
}

csString csGetJoystickDesc (int x, int y, int button,
			    const csKeyModifiers* modifiers,
			    bool distinguishModifiers)
{
  
  csString ret;
  (void)csGetMoverDef(ret, "Joystick", x, y, button,
		      modifiers, distinguishModifiers);
  return ret;
    
}


//@@@TODO: optimize this function
int csTypeOfInputDef(const char* str)
{
  csKeyModifiers m;
  const char* name = ParseModifiers (str, m);
  if (csParseJoystickDef (name, 0, 0, 0, 0)) return CSEVTYPE_Joystick;
  else if (csParseMouseDef (name, 0, 0, 0, 0)) return CSEVTYPE_Mouse;
  else if (csParseKeyDef (name, 0, 0, 0)) return CSEVTYPE_Keyboard;
  else return 0;
}

//---------------------------------------------------------------------------

csInputDefinition::csInputDefinition ()
{
  modifiersHonored = CSMASK_ALLSHIFTS;
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

uint32 csInputDefinition::GetHonoredModifiers () const
{
  return modifiersHonored;
}

bool csInputDefinition::Parse (const char* string, bool useCooked)
{
  const char* name = ParseModifiers (string, modifiers);

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
  }
  else
  {
    utf32_char codeRaw = 0, codeCooked = 0;
    if (!ParseKey (name, &codeRaw, &codeCooked)) return false;

    utf32_char code = (useCooked && (codeCooked != 0)) ? codeCooked : codeRaw;

    if (code != 0)
    {
      k.keyCode = code;
      k.codeIsCooked = useCooked;
    }
    else 
      return false;
    containedType = csevKeyboard;
  }
  return true;
}

csString csInputDefinition::GetDescription () const
{
  // @@@ Implement me
  return "";
}

bool csInputDefinition::FromEvent (iEvent* event, bool useCookedKey)
{
  if (CS_IS_KEYBOARD_EVENT(*event))
  {
    csKeyEventData ked;
    if (csKeyEventHelper::GetEventData (event, ked))
    {
      k.keyCode = (useCookedKey && (ked.codeCooked != 0)) 
	? ked.codeCooked 
	: ked.codeRaw;
      containedType = csevKeyboard;
      modifiers = ked.modifiers;
      return true;
    }
  }
  else if (CS_IS_MOUSE_EVENT(*event))
  {
    m = event->Mouse;
    if (m.Button)
    {
      containedType = csevMouseDown;
      m.x = m.y = 0;
    }
    else if (m.y)
    {
      containedType = csevMouseMove;
      m.x = 0;
      m.y = 1;
      m.Button = 0;
    }
    else if (m.x)
    {
      containedType = csevMouseMove;
      m.x = 1;
      m.y = 0;
      m.Button = 0;
    }
    else
      return false;
    // Left/Right/any is lost in translation, we default to any
    for (int mod=0; mod < csKeyModifierTypeLast; mod++)
      if (m.Modifiers & (1<<mod))
	modifiers.modifiers[mod] = csKeyModifierNumAny;
    return true;
  }
  else if (CS_IS_JOYSTICK_EVENT(*event))
  {
    j = event->Joystick;
    if (j.Button)
    {
      containedType = csevJoystickDown;
      j.x = j.y = 0;
    }
    else if (j.y)
    {
      containedType = csevJoystickMove;
      j.x = 0;
      j.y = 1;
      j.Button = 0;
    }
    else if (j.x)
    {
      containedType = csevJoystickMove;
      j.x = 1;
      j.y = 0;
      j.Button = 0;
    }
    else
      return false;
    // Left/Right/any is lost in translation, we default to any
    for (int mod=0; mod < csKeyModifierTypeLast; mod++)
      if (m.Modifiers & (1<<mod))
	modifiers.modifiers[mod] = csKeyModifierNumAny;
    return true;
  }

  return false;
}

uint32 csInputDefinition::ComputeHash () const
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

int csInputDefinition::Compare (const csInputDefinition& def) const
{
  int d = containedType - def.containedType;
  
  if (d == 0)
  {
    d = modifiersHonored - def.modifiersHonored;
    if (d == 0)
    {
      d = ((int)(csKeyEventHelper::GetModifiersBits (modifiers) &
            modifiersHonored)) -
	  ((int)(csKeyEventHelper::GetModifiersBits (def.modifiers) &
	    modifiersHonored));
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
		utf32_char cooked2 = def.k.codeIsCooked ?
		  def.k.keyCode : RawToCooked (def.k.keyCode);
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
		      - If the parsed modifier was generic, allow any modifier
		        of a type.
		      - If the parsed modifier(s) were specific, only allow
		        those.
		    */
		    if (!(((modifiers.modifiers[n] == 0xffffffff) &&
		         (mod.modifiers[n] != 0)) || 
		      (((mod.modifiers[n] & modifiers.modifiers[n]) ==
		        modifiers.modifiers[n]) && 
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

int csInputDefinition::Compare (iEvent* event) const
{
  /*
    @@@ Very similar to Compare (const csInputDefinition&)
    -> try to merge both
   */ 
  int d = containedType - event->Type;
  
  if (d == 0)
  {
    d = ((int)(csKeyEventHelper::GetModifiersBits (modifiers) &
          modifiersHonored)) -
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
		    - If the parsed modifier was generic, allow any modifier of
		        a type.
		    - If the parsed modifier(s) were specific, only allow
		        those.
		   */
		  if (!(((modifiers.modifiers[n] == 0xffffffff) &&
		       (mod.modifiers[n] != 0)) || 
		    (((mod.modifiers[n] & modifiers.modifiers[n]) ==
		      modifiers.modifiers[n]) && 
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
