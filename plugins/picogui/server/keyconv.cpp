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
#include "keyconv.h"

extern "C"
{
  #include <picogui/pgkeys.h>
}

int csPGKeyConverter::Chars [256];
int csPGKeyConverter::Codes [CSKEY_LAST - CSKEY_FIRST + 1];

void csPGKeyConverter::Construct ()
{
  #define CHAR(CS, PG) Chars[CS] = (PG);
  #define CODE(CS, PG) Codes[(CS) - CSKEY_FIRST] = (PG);
    #include "../keyconv.i"
  #undef CHAR
  #undef CODE
};

int csPGKeyConverter::CS2PG (iEvent &ev)
{
  CS_ASSERT (CS_IS_KEYBOARD_EVENT (ev));
  if (ev.Key.Char < 256 && ev.Key.Char > 0)
    return Chars [ev.Key.Code];
  else if (ev.Key.Code >= CSKEY_FIRST && ev.Key.Code <= CSKEY_LAST)
    return Codes [ev.Key.Code - CSKEY_FIRST];
  else
    CS_ASSERT (0);
  return 0;
}

int csPGKeyConverter::CS2PGMod (iEvent &ev)
{
  CS_ASSERT (CS_IS_KEYBOARD_EVENT (ev));
  int mod = 0;
  if (ev.Key.Modifiers & CSMASK_SHIFT)	mod |= PGMOD_SHIFT;
  if (ev.Key.Modifiers & CSMASK_CTRL)	mod |= PGMOD_CTRL;
  if (ev.Key.Modifiers & CSMASK_ALT)	mod |= PGMOD_ALT;
  return mod;
}
