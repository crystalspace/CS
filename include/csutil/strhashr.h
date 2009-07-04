/*
    Copyright (C) 2008 by Jorrit Tyberghein and Michael Gist

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


#ifndef __CS_UTIL_STRHASHR_H__
#define __CS_UTIL_STRHASHR_H__

#include "csutil/strhash.h"

/**
 * A csStringHash providing a reverse hash for fast string lookups when given an ID.
 */

class CS_CRYSTALSPACE_EXPORT csStringHashReversible : public csStringHash
{
private:
  csHash<const char*, csStringID> reverse;
  void Copy(csStringHashReversible const& h);

public:
    /// Constructor.
  csStringHashReversible (size_t size = 23);
  /// Copy constructor.
  csStringHashReversible (csStringHashReversible const& h) { Copy(h); }
  /// Destructor.
  ~csStringHashReversible ();
  /// Assignment operator.
  csStringHashReversible& operator=(csStringHashReversible const& h) { Copy(h); return *this; }

    /**
   * Register a string with an ID.
   * \param s The string with which to associate the ID.
   * \param id A numeric value with which to identify this string.
   * \return A pointer to the copy of the string in this hash.
   * \remarks If the string is already registered with a different ID, the old
   *   ID will be replaced with the one specified here. If you would like the
   *   convenience of having the ID assigned automatically, then consider using
   *   csStringSet, instead.
   * <p>
   * \remarks If you do not care about the ID, but instead simply want to use
   *   the hash as a string \e set which merely records if a string is present,
   *   then you can omit \c id. To find out if a string is contained in the
   *   set, invoke Contains(). The same functionality can be accomplished via
   *   csStringSet, however csStringSet is more heavyweight because it also
   *   maintains a reverse-mapping from ID to string. Omitting the \c id makes
   *   for a good alternative to csStringSet when you do not require its extra
   *   bulk.
   */
  const char* Register (const char* s, csStringID id = 0);

  /**
   * Request the ID for the given string.
   * \return The string's ID or csInvalidStringID if the string has not yet
   *   been registered.
   */
  csStringID Request (const char* s) const { return csStringHash::Request(s); }

  /**
   * Request the string for a given ID.
   * \return The string associated with the given ID, or the null pointer if
   *   the string has not yet been registered. If more than one string is
   *   associated with the ID, then one is returned (but specifically which one
   *   is unspecified).
   */
  const char* Request (csStringID id) const;

  /**
   * Request all strings for a given ID.
   * \return Array of all the elements, or empty if there are none.
   */
  csArray<const char*> RequestAll (csStringID id) const
  {
    return reverse.GetAll(id);
  }

  /**
   * Remove specified string.
   * \return True if a matching string was in thet set; else false.
   */
  bool Delete(char const* s);

  /**
   * Remove a string with the specified ID.
   * \return True if a matching string was in thet set; else false.
   * \remarks If more than one string is associated with the ID, then one is
   *   removed (but specifically which one is unspecified).
   */
  bool Delete(csStringID id);

  /**
   * Remove all stored strings.
   */
  void Empty ();
};

#endif // __CS_UTIL_STRHASHR_H__
