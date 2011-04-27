/*
    Copyright (C) 2010 by Frank Richter

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

#ifndef __CS_STRINGQUOTE_H__
#define __CS_STRINGQUOTE_H__

/**\file
 * Helper functions to decorate strings with nice-looking quote characters
 */

#include "csextern.h"
#include "csstring.h"

namespace CS
{
  /**
   * Helper functions to decorate strings with nice-looking quote characters.
   * \remarks This functions works upon UTF-8 strings. Non-UTF-8 input will
   *   likely be mangled.
   */
  struct CS_CRYSTALSPACE_EXPORT Quote
  {
    /**
     * Add single quotes (<tt>&lsquo;</tt>, <tt>&rsquo;</tt>) around a string.
     * \param out String to receive quoted input.
     * \param str String to quote.
     */
    static void Single (csStringBase& out, const char* str);
    /**
     * Add single quotes (<tt>&lsquo;</tt>, <tt>&rsquo;</tt>) around a string.
     * \param str String to quote.
     * \return Pointer to quoted input. The returned string will be discarded
     *   overwritten after a small, but indeterminate time. It is safe to
     *   assume it survives to be used as an argument to a function call, but
     *	 for anything longer than that the string should be stowed away
     *   manually somewhere.
     */
    static const char* Single (const char* str);
    
    /**
     * Add double quotes (<tt>&ldquo;</tt>, <tt>&rdquo;</tt>) around a string.
     * \param out String to receive quoted input.
     * \param str String to quote.
     */
    static void Double (csStringBase& out, const char* str);
    /**
     * Add double quotes (<tt>&ldquo;</tt>, <tt>&rdquo;</tt>) around a string.
     * \param str String to quote.
     * \return Pointer to quoted input. The returned string will be discarded
     *   overwritten after a small, but indeterminate time. It is safe to
     *   assume it survives to be used as an argument to a function call, but
     *	 for anything longer than that the string should be stowed away
     *   manually somewhere.
     */
    static const char* Double (const char* str);
  };
} // namespace CS

#endif // __CS_STRINGQUOTE_H__
