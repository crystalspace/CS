/*
    PC keyboard scancode -> CS key conversion table
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

unsigned short ScanCodeToChar [128] =
{
  0,              CSKEY_ESC,      '1',            '2',
  '3',            '4',            '5',            '6',		// 00..07
  '7',            '8',            '9',            '0',
  '-',            '=',            CSKEY_BACKSPACE,CSKEY_TAB,	// 08..0F
  'q',            'w',            'e',            'r',
  't',            'y',            'u',            'i',		// 10..17
  'o',            'p',            '[',            ']',
  CSKEY_ENTER,    CSKEY_CTRL,     'a',            's',		// 18..1F
  'd',            'f',            'g',            'h',
  'j',            'k',            'l',            ';',		// 20..27
  39,             '`',            CSKEY_SHIFT,    '\\',
  'z',            'x',            'c',            'v',		// 28..2F
  'b',            'n',            'm',            ',',
  '.',            '/',            CSKEY_SHIFT,    CSKEY_PADMULT,// 30..37
  CSKEY_ALT,      ' ',            0,              CSKEY_F1,
  CSKEY_F2,       CSKEY_F3,       CSKEY_F4,       CSKEY_F5,	// 38..3F
  CSKEY_F6,       CSKEY_F7,       CSKEY_F8,       CSKEY_F9,
  CSKEY_F10,      0,              0,              CSKEY_HOME,	// 40..47
  CSKEY_UP,       CSKEY_PGUP,     CSKEY_PADMINUS, CSKEY_LEFT,
  CSKEY_CENTER,   CSKEY_RIGHT,    CSKEY_PADPLUS,  CSKEY_END,	// 48..4F
  CSKEY_DOWN,     CSKEY_PGDN,     CSKEY_INS,      CSKEY_DEL,
  0,              0,              0,              CSKEY_F11,	// 50..57
  CSKEY_F12,      0,              0,              0,
  0,              0,              0,              0,		// 58..5F
  0,              0,              0,              0,
  0,              0,              0,              0,		// 60..67
  0,              0,              0,              0,
  0,              0,              0,              CSKEY_PADDIV,	// 68..6F
  0,              0,              0,              0,
  0,              0,              0,              0,		// 70..77
  0,              0,              0,              0,
  0,              0,              0,              0		// 78..7F
};
