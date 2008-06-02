/*
    Copyright (C) 2006 by Jorrit Tyberghein
	      (C) 2006 by Frank Richter

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

/**\file
 * Disable "deprecated" compiler warnings.
 * Compilers are occasionally overzealous with deprecation warnings; they're
 * even emitted when not desired, for example, when a deprecated method is 
 * overridden, which inevitably happens when implementing an interface method. 
 * To work around this, this file is intended to be included from headers 
 * before code that causes such "false deprecation" warnings, as this header 
 * will disable  it. After such code the file deprecated_warn_on.h should be 
 * included to re-enable the warning.
 */

#if defined(CS_COMPILER_MSVC)
#pragma warning(push)
#pragma warning(disable:4996)
#endif

#if defined(CS_COMPILER_GCC)
#  if (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 1))
#    pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#  endif
#endif
