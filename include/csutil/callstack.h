/*
  Call stack creation helper
  Copyright (C) 2004 by Jorrit Tyberghein
	    (C) 2004 by Frank Richter

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
#ifndef __CS_UTIL_CALLSTACK_H__
#define __CS_UTIL_CALLSTACK_H__

/**\file
 * Call stack creation helper
 */

#include "csextern.h"
#include "csutil/csstring.h"

/// Call stack.
class CS_CRYSTALSPACE_EXPORT csCallStack
{
protected:
  virtual ~csCallStack() {}
public:
  /**
   * Release the memory for this call stack.
   */
  virtual void Free() = 0;
  
  /// Get number of entries in the stack.
  virtual size_t GetEntryCount () = 0;
  /**
   * Get the function for an entry.
   * Contains usually raw address, function name and module name.
   * Returns false if an error occured or a name is not available.
   */
  virtual bool GetFunctionName (size_t num, csString& str) = 0;
  /**
   * Get file and line number for an entry.
   * Returns false if an error occured or a line number is not
   * available.
   */
  virtual bool GetLineNumber (size_t num, csString& str) = 0;
  /**
   * Get function parameter names and values.
   * Returns false if an error occured or if parameters are not available.
   */
  virtual bool GetParameters (size_t num, csString& str) = 0;
  /**
   * Print the complete stack.
   * \param f File handle to print to.
   * \param brief Brief output - line number and parameters are omitted.
   */
  void Print (FILE* f = stdout, bool brief = false)
  {
    for (size_t i = 0; i < GetEntryCount(); i++)
    {
      csString s;
      bool hasFunc = GetFunctionName (i, s);
      fprintf (f, "%s", hasFunc ? (const char*)s : "<unknown>");
      if (!brief && (GetLineNumber (i, s)))
	fprintf (f, " @%s", (const char*)s);
      if (!brief && (GetParameters (i, s)))
	fprintf (f, " (%s)", (const char*)s);
      fprintf (f, "\n");
    }
    fflush (f);
  }
  /**
   * Obtain complete text for an entry.
   * \param i Index of the entry.
   * \param brief Brief - line number and parameters are omitted.
   */
  csString GetEntryAll (size_t i, bool brief = false)
  {
    csString line;
    csString s;
    bool hasFunc = GetFunctionName (i, s);
    line << (hasFunc ? (const char*)s : "<unknown>");
    if (!brief && GetLineNumber (i, s))
      line << " @" << s;
    if (!brief && GetParameters (i, s))
      line << " (" << s << ")";
    return line;
  }
};

/// Helper to create a call stack.
class CS_CRYSTALSPACE_EXPORT csCallStackHelper
{
public:
  /**
   * Create a call stack.
   * \param skip The number of calls on the top of the stack to remove from
   *  the returned call stack. This can be used if e.g. the call stack is
   *  created from some helper function and the helper function itself should
   *  not appear in the stack.
   * \param fast Flag whether a fast call stack creation should be preferred 
   *  (usually at the expense of retrieved information).
   * \return A call stack object.
   * \remarks Free the returned object with its Free() method.
   */
  static csCallStack* CreateCallStack (int skip = 0, bool fast = false);
};

#endif // __CS_UTIL_CALLSTACK_H__
