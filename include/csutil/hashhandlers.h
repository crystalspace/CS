/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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

#ifndef __CS_UTIL_HASHHANDLERS_H__
#define __CS_UTIL_HASHHANDLERS_H__

#ifdef CS_COMPILER_GCC
  #warning The "hash key handlers" mechanism has been removed; to provide custom
  #warning comparisons for two keys, provide a suitable specialization of the
  #warning csComparator<> and csHashComputer<> templates.
  #warning Also note that for some commonly used key types (notably, const char*) 
  #warning specializations are already provide (consult API docs for a list of
  #warning specializations.)
#endif
#ifdef CS_COMPILER_MSVC
  #pragma message ("The \"hash key handlers\" mechanism has been removed; to provide custom")
  #pragma message ("comparisons for two keys, provide a suitable specialization of the")
  #pragma message ("csComparator<> and csHashComputer<> templates.")
  #pragma message ("Also note that for some commonly used key types (notably, const char*)")
  #pragma message ("specializations are already provide (consult API docs for a list of")
  #pragma message ("specializations.)")
#endif

#endif // __CS_UTIL_HASHHANDLERS_H__
