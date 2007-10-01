/*
    Copyright (C) 2004 by Frank Richter

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

#ifndef __CS_UTIL_REGEX_WRAPPER_H__
#define __CS_UTIL_REGEX_WRAPPER_H__

#include "csplatform.h"

#ifdef CS_HAVE_REGEX
  #include <regex.h>
#else
  #if (defined(CS_COMPILER_MSVC) || defined(CS_COMPILER_BCC)) && \
      !defined(__STDC__)
    #define __STDC__  1
    #define __STDC__DEFINED
  #endif
  #include "generic/regex.h"
  #ifdef __STDC__DEFINED
    #undef __STDC__
  #endif
#endif

#endif // __CS_UTIL_REGEX_WRAPPER_H__
