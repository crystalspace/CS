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
#include "iutil/event.h"
#include "csutil/csevent.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

static struct csKeyCodeDef
{
  const char *key;
  int code;
} KeyDefs [] =
{
  { "Esc",	CSKEY_ESC	},
  { "Enter",	CSKEY_ENTER	},
  { "Tab",	CSKEY_TAB	},
  { "Back",	CSKEY_BACKSPACE },
  { "BackSpace",CSKEY_BACKSPACE },
  { "Up",	CSKEY_UP	},
  { "Down",	CSKEY_DOWN	},
  { "Left",	CSKEY_LEFT	},
  { "Right",	CSKEY_RIGHT	},
  { "PgUp",	CSKEY_PGUP	},
  { "PageUp",	CSKEY_PGUP	},
  { "PgDn",	CSKEY_PGDN	},
  { "PageDown",	CSKEY_PGDN	},
  { "Home",	CSKEY_HOME	},
  { "End",	CSKEY_END	},
  { "Ins",	CSKEY_INS	},
  { "Insert",	CSKEY_INS	},
  { "Del",	CSKEY_DEL	},
  { "Delete",	CSKEY_DEL	},
  { "Ctrl",	CSKEY_CTRL	},
  { "Control",	CSKEY_CTRL	},
  { "Alt",	CSKEY_ALT	},
  { "Shift",	CSKEY_SHIFT	},
  { "Center",	CSKEY_CENTER	},
  { "F1",	CSKEY_F1	},
  { "F2",	CSKEY_F2	},
  { "F3",	CSKEY_F3	},
  { "F4",	CSKEY_F4	},
  { "F5",	CSKEY_F5	},
  { "F6",	CSKEY_F6	},
  { "F7",	CSKEY_F7	},
  { "F8",	CSKEY_F8	},
  { "F9",	CSKEY_F9	},
  { "F10",	CSKEY_F10	},
  { "F11",	CSKEY_F11	},
  { "F12",	CSKEY_F12	},
  { "PAD+",	CSKEY_PADPLUS	},
  { "PAD-",	CSKEY_PADMINUS	},
  { "PAD*",	CSKEY_PADMULT	},
  { "PAD/",	CSKEY_PADDIV	},
  { NULL,	0		}
};

static struct csKeyMaskDef
{
  const char *key;
  int mask;
} KeyMasks [] =
{
  { "Ctrl+",	CSMASK_CTRL	},
  { "Alt+",	CSMASK_ALT	},
  { "Shift+",	CSMASK_SHIFT	},
  { NULL,	0		}
};

#ifdef COMP_VC
int strncmpi (const char* first, const char* last, unsigned int count)
{
  int f, l;
  int result = 0;
  
  if (count)
  {
    do
    {
      f = tolower (*first);
      l = tolower (*last);
      first++;
      last++;
    } while (--count && f && l && f == l);
		
    int result = f - l;
  }

  return result;
}
#endif

iEvent* csParseInputDef (const char *name, iEvent* ev, bool use_modifiers = true)
{
  int mod = 0;
  bool ismask;
  do
  {
    ismask = false;
    for (csKeyMaskDef *m = KeyMasks; m->key; m++)
      if (! strncmpi (m->key, name, strlen (m->key)))
      {
        if (use_modifiers) mod |= m->mask;
        name += strlen (m->key);
        ismask = true;
      }
  } while (ismask);

  if (! strncmpi (name, "Mouse", 5))
  {
    name += 5;
    if (*name == 'X' || *name == 'x')
      *ev = csEvent (0, csevMouseMove, 1, 0, 0, 0);
    else if (*name == 'Y' || *name == 'y')
      *ev = csEvent (0, csevMouseMove, 0, 1, 0, 0);
    else *ev = csEvent (0, csevMouseDown, 0, 0, atoi (name), mod);
  }
  else if (! strncmpi (name, "Joystick", 8))
  {
    name += 8;
    if (*name == 'X' || *name == 'x')
      *ev = csEvent (0, csevJoystickMove, 1, 1, 0, 0, 0);
    else if (*name == 'Y' || *name == 'y')
      *ev = csEvent (0, csevJoystickMove, 1, 0, 1, 0, 0);
    else *ev = csEvent (0, csevJoystickDown, 1, 0, 0, atoi (name), mod);
  }
  else
  {
    int code = 0;
    for (csKeyCodeDef *c = KeyDefs; c->key; c++)
      if (! strcmpi (c->key, name)) code = c->code;
    if (code) *ev = csEvent (0, csevKeyDown, code, 0, mod);
    else *ev = csEvent (0, csevKeyDown, 0, (int)*name, mod);
  }

  return ev;
}

csEvent& csParseInputDef (const char* name, csEvent& ev, bool use_modifiers = true)
{
  csParseInputDef (name, &ev, use_modifiers);
  return ev;
}

char* csGetInputDesc (iEvent *ev, char *buf, bool use_modifiers = true)
{
  char *ret = buf;
  int mod = 0;
  switch (ev->Type)
  {
    case csevKeyUp:
    case csevKeyDown:
      mod = ev->Key.Modifiers;
      break;

    case csevMouseUp:
    case csevMouseDown:
      mod = ev->Mouse.Modifiers;
      break;

    case csevJoystickUp:
    case csevJoystickDown:
      mod = ev->Joystick.Modifiers;
      break;

    default:
      break;
  }
  for (csKeyMaskDef *mask = KeyMasks; mask->key; mask++)
  {
    if (mod & mask->mask)
    {
      strcpy (buf, mask->key);
      buf = strchr (buf, 0);
    }
  }

  switch (ev->Type)
  {
    case csevKeyUp:
    case csevKeyDown:
      if (ev->Key.Code >= CSKEY_FIRST)
      {
        const char *key = NULL;
        for (csKeyCodeDef *k = KeyDefs; k->key; k++)
          if (k->code == ev->Key.Code) key = k->key;
        if (key)
        {
          strcpy (buf, key);
          return ret;
        }
      }
      if (ev->Key.Char < 255)
      {
        *buf = (char)ev->Key.Char;
        *++buf = 0;
        return ret;
      }
      break;

    case csevMouseUp:
    case csevMouseDown:
      strcpy (buf, "Mouse");
      sprintf (strchr (buf, 0), "%i", ev->Mouse.Button);
      return ret;

    case csevJoystickUp:
    case csevJoystickDown:
      strcpy (buf, "Joystick");
      sprintf (strchr (buf, 0), "%i", ev->Joystick.Button);
      return ret;

    case csevMouseMove:
      strcpy (buf, "Mouse");
      buf = strchr (buf, 0);
      if (ev->Mouse.x > ev->Mouse.y) *buf++ = 'X';
      else if (ev->Mouse.x < ev->Mouse.y) *buf++ = 'Y';
      *buf = 0;
      return ret;

    case csevJoystickMove:
      strcpy (buf, "Joystick");
      buf = strchr (buf, 0);
      if (ev->Joystick.x > ev->Joystick.y) *buf++ = 'X';
      else if (ev->Joystick.x < ev->Joystick.y) *buf++ = 'Y';
      *buf = 0;
      return ret;

    default:
      break;
  }
  return NULL;
}

char* csGetInputDesc (csEvent &ev, char *buf, bool use_modifiers = true)
{
  return csGetInputDesc (&ev, buf, use_modifiers);
}

