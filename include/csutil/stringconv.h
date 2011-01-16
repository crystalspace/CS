/*
    Copyright (C) 2008 by Frank Richter
    
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

#ifndef __CS_CSUTIL_STRINGCONV_H__
#define __CS_CSUTIL_STRINGCONV_H__

#include "csutil/csstring.h"

/**\file
 * Routines to convert data to and from strings.
 */
 
namespace CS
{
  namespace Utility
  {
    /**
     * Convert a string to float value independent of the locale.
     * \param str The string to convert.
     * \param end If not 0 receives pointer to the first character
     *   after the converted value.
     * \remarks Wraps the C runtime library's strtod() or strtof()
     *   functions and has thus the same behaviour with respect to
     *   invalid inputs and under-/overflow.
     */
    float CS_CRYSTALSPACE_EXPORT strtof (const char* str,
      const char** end = 0);
      
    /**
     * Convert a float to a string, and attempting
     * to keep as much precision as possible.
     * \param f The float value to convert.
     * \return String representation of the float value.
     * \remarks Practically, the float value will be converted to a decimal
     *   number. The decimal point will be a '.', independent of locale.
     */
    csString CS_CRYSTALSPACE_EXPORT ftostr (float f);
  } // namespace Utility
} // namespace CS

#endif // __CS_CSUTIL_STRINGCONV_H__
