/*
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

#ifndef __CS_CSTOOL_IDENTSTRINGS_H__
#define __CS_CSTOOL_IDENTSTRINGS_H__

/**\file
 * Set of helper macros to obtain the names of identifiers.
 */

/**
 * Helper class that contains a list of identifiers and their names.
 */
class csIdentStrings
{
public:
  struct csIdentString
  {
    int ident;
    const char* str;
  };
protected:
  const csIdentString* strings;
  size_t stringCount;
  csString* scratch;
public:
  csIdentStrings (const csIdentString* str, csString* scratch, size_t cnt) :
    strings(str), stringCount(cnt), scratch(scratch) { }
  /**
   * Obtain the name of \a ident.
   * \remarks The returned string is only guaranteed to be valid until the
   *  next StringForIdent() call.
   */
  const char* StringForIdent (int ident)
  {
    size_t l = 0, r = stringCount;
    while (l < r)
    {
      size_t m = (l+r) / 2;
      if (strings[m].ident == ident)
      {
	return strings[m].str;
      }
      else if (strings[m].ident < ident)
      {
	l = m + 1;
      }
      else
      {
	r = m;
      }
    }
    scratch->Format ("%d", ident);
    return scratch->GetData();
  }
};

/**
 * Begin an identifier list. \p ListName is the identifier of the generated
 * csIdentStrings object.
 * <p>
 * Example:
 * \code
 * CS_IDENT_STRING_LIST(FooNames)
 *   CS_IDENT_STRING(FOO_BAR)
 *   CS_IDENT_STRING(FOO_BAZ)
 * CS_IDENT_STRING_LIST_END(FooNames)
 * \endcode
 * From code, use like:
 * \code
 *   csPrintf ("%s\n", FooNames.StringForIdent (foo));
 * \endcode
 */
#define CS_IDENT_STRING_LIST(ListName)					\
  static csIdentStrings::csIdentString ListName##_strings[] = {
/**
 * Entry in the identifier list.
 * \remark The identifiers <b>*must*</b> be sorted in increasing order.
 */
#define CS_IDENT_STRING(Ident)				{Ident, #Ident},
/**
 * Entry in the identifier list, but with an explicitly set name.
 * \remark The identifiers <b>*must*</b> be sorted in increasing order.
 */
#define CS_IDENT_STRING_EXPLICIT(Ident, Str)		{Ident, str},
/**
 * End an identifier list.
 */
#define CS_IDENT_STRING_LIST_END(ListName)				\
    {0, 0}								\
  };									\
  CS_IMPLEMENT_STATIC_VAR(Get##ListName##Scratch, csString, ());	\
  static csIdentStrings ListName (ListName##_strings, 			\
    Get##ListName##Scratch(),						\
    (sizeof (ListName##_strings) / sizeof (csIdentStrings::csIdentString)) - 1);

#endif // __CS_CSTOOL_IDENTSTRINGS_H__
