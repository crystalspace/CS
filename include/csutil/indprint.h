/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __CS_INDPRINT_H__
#define __CS_INDPRINT_H__

#include <stdarg.h>
#include "csextern.h"

/**
 * A debugging class to help print out indented messages.
 */
class CS_CRYSTALSPACE_EXPORT csIndPrint
{
private:
  int cur_indent_level;

public:
  csIndPrint () : cur_indent_level (0) { }

  /// Go up in indentation.
  void Up (int amount = 1) { cur_indent_level -= amount; }
  /// Go down in indentation.
  void Down (int amount = 1) { cur_indent_level += amount; }
  /// Reset indentation.
  void Reset (int amount = 0) { cur_indent_level = amount; }

  /// Print with current indentation level.
  void Print (char* msg, ...)
  {
    int i = cur_indent_level;
    while (i >= 20) { printf ("                    "); i -= 20; }
    while (i >= 4) { printf ("    "); i -= 4; }
    while (i >= 0) { printf (" "); i--; }
    va_list arg;
    va_start (arg, msg);
    vprintf (msg, arg);
    va_end (arg);
    fflush (stdout);
  }
};

/**
 * This class is a small helper class to indent a level down and
 * automatically indent up again when this class is destructed.
 * So you can use it to keep track of indentation without the risk
 * of miscounting Up() and Down() calls.
 */
class CS_CRYSTALSPACE_EXPORT csIndPrintDown
{
private:
  csIndPrint& indprint;
  int amount;

public:
  /// Remember current state.
  csIndPrintDown (csIndPrint& ip, int amount = 1) : indprint (ip)
  {
    csIndPrintDown::amount = amount;
    ip.Down (amount);
  }
  ~csIndPrintDown ()
  {
    indprint.Up (amount);
  }
};

#endif // __CS_INDPRINT_H__

