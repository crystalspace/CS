/*
    Crystal Space input library
    Copyright (C) 2000 by Andrew Zabolotny <bit@eltech.ru>

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

#include <ctype.h>

extern char ShiftedKey [];

// And this one performs backward conversion
char const UnshiftedKey [128-32] =
{
' ', '1', '\'','3', '4', '5', '7', '\'','9', '0', '8', '=', ',', '-', '.', '/',
'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ';', ';', ',', '=', '.', '/',
'2', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '[', '\\',']', '6', '-',
'`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '[', '\\',']', '`', 127
};

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
  { "Space",	' '		},
  { NULL,	0		}
};

static struct csKeyMaskDef
{
  const char *key;
  int mask;
} KeyMasks [] =
{
  { "Ctrl",	CSMASK_CTRL	},
  { "Alt",	CSMASK_ALT	},
  { "Shift",	CSMASK_SHIFT	},
  { NULL,	0		}
};

static inline bool is_shifted (char c)
{
  for (char *p = ShiftedKey; *p != 127; p++)
    if (*p == c) return true;
  return false;
}

static inline char char_shift (char c)
{
  return ((signed char) (c) >= 32) ? ShiftedKey [c - 32] : c;
}

static inline char char_unshift (char c)
{
  return ((signed char) (c) >= 32) ? UnshiftedKey [c - 32] : c;
}

bool csParseKeyDef (const char *iKeyDef, int &oKey, int &oShiftMask)
{
  // Skip initial whitespaces
  const char *src = iKeyDef + strspn (iKeyDef, " \t");
  oShiftMask = 0;
  oKey = 0;
  while (*src)
  {
    // Now find where the string ends
    const char *end;
    // special case for "'+'"
    if (strncmp (src, "'+'", 3) == 0)
      end = strpbrk (src + 3, " \t+");
    // special case for "PAD+"
    else if (strncasecmp (src, "PAD+", 4) == 0)
      end = strpbrk (src + 4, " \t+");
    else
      end = strpbrk (src, " \t+");
    if (!end) end = strchr (src, 0);

    // Now get the key name
    char key [30];
    size_t len = end - src;
    if (len >= sizeof (key)) len = sizeof (key) - 1;
    memcpy (key, src, len);
    key [len] = 0;

    // Find the end of the space between this key and next and decide
    // whenever this is the last key or it is a control prefix
    src = end + strspn (end, " \t");
    src += strspn (src, " \t+");
    if (*src)
    {
      // control prefix
      for (csKeyMaskDef *cur = KeyMasks; cur->key; cur++)
        if (strcasecmp (cur->key, key) == 0)
        {
          oShiftMask |= cur->mask;
          goto ok;
        }
      return false; // unknown control prefix
    }
    else
    {
      // If key name looks like {?} replace with just ?
      if (key [0] == '\'' && key [2] == '\'' && key [3] == 0)
      {
        key [0] = key [1];
        key [1] = 0;
      }
      // Check if this is a regular alphanumeric key
      if (!key [1])
      {
        // Check if key is shifted, set Shift mask
        if (is_shifted (key [0]))
          oShiftMask |= CSMASK_SHIFT;

        key [0] = char_unshift (key [0]);
        oKey = key [0];
        goto ok;
      }
      // the actual key code
      for (csKeyCodeDef *cur = KeyDefs; cur->key; cur++)
        if (strcasecmp (cur->key, key) == 0)
        {
          oKey = cur->code;
          goto ok;
        }
      return false; // unknown key code
    }
ok:;
  }
  return true;
}

void csGetKeyDesc (int iKey, int iShiftMask, char *oKeyName)
{
  char *dst = oKeyName;

  // Remove Shift from most characters
  if ((iKey < 255) && (iShiftMask & CSMASK_SHIFT))
    if (isalpha (iKey))
    {
      iKey = char_shift (iKey);
      iShiftMask &= ~CSMASK_SHIFT;
    }
    else
      iKey = char_unshift (iKey);

  for (csKeyMaskDef *km = KeyMasks; km->key; km++)
    if (iShiftMask & km->mask)
    {
      strcpy (dst, km->key);
      dst = strchr (dst, 0);
      *dst++ = '+';
    }

  for (csKeyCodeDef *kd = KeyDefs; kd->key; kd++)
    if (iKey == kd->code)
    {
      strcpy (dst, kd->key);
      return;
    }

  if (!isalpha (iKey))
  {
    *dst++ = '\'';
    *dst++ = iKey;
    *dst++ = '\'';
  }
  else
    *dst++ = iKey;
  *dst = 0;
}
