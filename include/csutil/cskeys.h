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

#ifndef __CSKEYS_H__
#define __CSKEYS_H__

/**
 * Convert a free-format string into a key definition as understood by
 * the csinput library ("Ctrl+a", "alt+shift+backspace" and so on).
 * On return oKey will contain the key code (alphanumeric character or
 * one of CSKEY_XXX constants) and oShiftMask will be a combination of
 * CSMASK_XXX constants. Handy for supporting user-defined hot-keys,
 * keyboard accelerators and so on.
 */
extern bool csParseKeyDef (const char *iKeyDef, int &oKey, int &oShiftMask);

/**
 * Performs the reverse conversion; given a key code and a shift mask
 * the routine will output a string describing the key combination in
 * human-understandable format. The output of csGetKeyDesc, if feed to
 * csParseKeyDef, will produce exactly the same key code/shift mask.
 * Note that there is no output buffer length parameter; that is because
 * we know in advance that a 30-character buffer is enough and will
 * never overflow.
 */
extern void csGetKeyDesc (int iKey, int iShiftMask, char *oKeyName);

#endif // __CSKEYS_H__
