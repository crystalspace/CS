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
 * Disable MSVC deprecated warnings.
 * Unfortunately, MSVC is overzealous with deprecated warnings; it even emits 
 * one when a deprecated method is overridden, e.g. when implementing an 
 * interface method. To work around this, this file is intended to be
 * included from headers before code that causes such "false deprecation" 
 * warnings (C4996), as this header will disable it. After such code the
 * file msvc_deprecated_warn_on.h should be included to re-enable the
 * warning.
 */

#if defined(CS_COMPILER_MSVC)
#pragma warning(push)
#pragma warning(disable:4996)
#endif
