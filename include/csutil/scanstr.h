/*
    Copyright (C) 1998 by Jorrit Tyberghein and Steve Israelson

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

#ifndef __CS_SCANSTR_H__
#define __CS_SCANSTR_H__

#include "csextern.h"

/**\file
 * String scanning (sscanf() flavor).
 */

/**
 * Own version of sscanf that is a bit more relaxed towards spaces
 * and also accepts quoted strings (the quotes will not be included into
 * the result string).
 *
 * It supports the following format commands:
 *
 * <ul>
 * <li>%%d -- integer number</li>
 * <li>%%f -- floating point</li>
 * <li>%%b -- boolean (0, 1, true, false, yes, no, on, off)</li>
 * <li>%%s -- string (with or without single quotes)</li>
 * <li>%%S -- string (delimited with double quotes)<br>
 *              \\n will be converted to a newline<br>
 *              \\t will be converted to a tab<br>
 *              \\\\ produces a \\<br>
 *              \\" produces a "<br>
 *              all other conbinations of \\ are copied.</li>
 * <li>%%D -- list of integers, first argument should be a
 *		pointer to an array of integers, second argument
 *		a pointer to an integer which will contain the
 *		number of elements inserted in the list.</li>
 * <li>%%F -- similarly, a list of floats.</li>
 * <li>%%n -- this returns the amount of the input string
 *              thats been consumed, in characters. Does NOT
 *              increment the return count and does not read
 *              from the input string.</li>
 * </ul>
 *
 * Returns the number of successfully scanned arguments or -1 if there
 * was a mismatch.
 */
CS_CRYSTALSPACE_EXPORT int csScanStr (const char* in, const char* format, ...);

#endif // __CS_SCANSTR_H__
