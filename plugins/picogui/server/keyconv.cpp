/*
    Crystal Space Key Code Converter for PicoGUI
    (C) 2003 Mat Sutcliffe <oktal@gmx.co.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
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
#include "iutil/evdefs.h"
#include "csutil/event.h"
#include "keyconv.h"

extern "C"
{
  #include <picogui/pgkeys.h>
}

int csPGKeyConverter::Chars [256];
int csPGKeyConverter::Codes [CSKEY_SPECIAL_LAST - CSKEY_SPECIAL_FIRST + 1];

void csPGKeyConverter::Construct ()
{
  #ifdef CHAR
  #undef CHAR
  #endif
  #ifdef CODE
  #undef CODE
  #endif
  #define CHAR(CS, PG) Chars[(int) CS] = (PG);
  #define CODE(CS, PG) Codes[(CS) - CSKEY_SPECIAL_FIRST] = (PG);
    #include "../keyconv.i"
  #undef CHAR
  #undef CODE
};

int csPGKeyConverter::CS2PG (iEvent &ev)
{
  CS_ASSERT (CS_IS_KEYBOARD_EVENT (ev));
  if (csKeyEventHelper::GetRawCode (&ev) < 256 && csKeyEventHelper::GetRawCode (&ev) > 0)
    return Chars [csKeyEventHelper::GetCookedCode (&ev)];
  else if (CSKEY_IS_SPECIAL (csKeyEventHelper::GetRawCode (&ev)) )
    return Codes [CSKEY_SPECIAL_NUM (csKeyEventHelper::GetRawCode (&ev)) ];
  else
    CS_ASSERT (0);
  return 0;
}

int csPGKeyConverter::CS2PGMod (iEvent &ev)
{
  CS_ASSERT (CS_IS_KEYBOARD_EVENT (ev));
  int mod = 0;
  csKeyModifiers modifiers;
  csKeyEventHelper::GetModifiers (&ev, modifiers);
  if (modifiers.modifiers[csKeyModifierTypeShift] !=0)	mod |= PGMOD_SHIFT;
  if (modifiers.modifiers[csKeyModifierTypeCtrl] !=0)	mod |= PGMOD_CTRL;
  if (modifiers.modifiers[csKeyModifierTypeAlt] !=0)	mod |= PGMOD_ALT;
  return mod;
}
