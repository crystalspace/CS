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

#ifndef __CSUTIL_CSINPUTS_H__
#define __CSUTIL_CSINPUTS_H__

/**
 * Use in `int button' for csevXXXMove events
 * with the backward compatible funcs.
 */
#define CSAXIS_X -1
#define CSAXIS_Y -2

struct iEvent;
class csEvent;

/**
 * Convert a free-format string into an input event as understood by
 * the csinput library ("Ctrl+a", "alt+shift+mouse1" and so on).
 * Handy for supporting user-defined hot-keys, keyboard accelerators and so on.
 */
extern bool csParseInputDef (const char *name, iEvent *ev,
  bool use_shift = true);

extern bool csParseInputDef (const char *name, csEvent &ev,
  bool use_shift = true);

extern bool csParseKeyDef (const char *name, int &key, int &shift,
  bool use_shift = true);

extern bool csParseMouseDef (const char *name, int &button, int &shift,
  bool use_shift = true);

extern bool csParseJoystickDef (const char *name, int &button, int &shift,
  bool use_shift = true);

/**
 * Performs the reverse conversion; given an event object
 * the routine will copy a string describing the input combination in
 * human-understandable format.
 */
extern bool csGetInputDesc (iEvent *ev, char *buf,
  bool use_shift = true);

extern bool csGetInputDesc (csEvent &ev, char *buf,
  bool use_shift = true);

extern bool csGetKeyDesc (int key, int shift, char *buf,
  bool use_shift = true);

extern bool csGetMouseDesc (int button, int shift, char *buf,
  bool use_shift = true);

extern bool csGetJoyDesc (int button, int shift, char *buf,
  bool use_shift = true);

#endif // __CSUTIL_CSINPUTS_H__
