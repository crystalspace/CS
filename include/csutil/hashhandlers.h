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

/**\file
 * Additional key handlers for csHash<>
 */

#include "csextern.h"
#include "hash.h"
#include "ref.h"
#include "util.h"

/**
 * A hash key handler for const char* strings.
 * It uses strcmp () to compare two strings, so using this key handler is safe
 * even if two strings have the same hash key.
 * \remark Be aware that this key handler does NOT allocate any copies of the
 * keys! If you want to make copies then you can use csStrKey as the type
 * of the key instead of const char*.
 */
class csConstCharHashKeyHandler
{
public:
  static uint32 ComputeHash (const char* const& key)
  {
    return (csHashCompute (key));
  }

  static bool CompareKeys (const char* const& key1, const char* const& key2)
  {
    return (strcmp (key1, key2) == 0);
  }
};

/**
 * This is a simple helper class to make a copy of a const char*.
 * This can be used together with csConstCharHashKeyHandler to have a hash that
 * makes copies of the keys.
 */
class csStrKey
{
private:
  char* str;

public:
  csStrKey (const char* s) { str = csStrNew (s); }
  csStrKey (const csStrKey& c) { str = csStrNew (c.str); }
  ~csStrKey () { delete[] str; }
  csStrKey& operator=(const csStrKey& o)
  {
    delete[] str; str = csStrNew (o.str);
    return *this;
  }
  operator const char* () const { return str; }
};

/**
 * A hash key handler for csRef<> keys.
 * \remark Every time the a ref is used as a key, the referenced object's 
 *  reference count is incremented. If this behaviour is not desired, use
 *  simple pointers as keys instead (e.g. csHash<int, iBase*>.)
 */
template <class T>
class csRefHashKeyHandler
{
public:
  static uint32 ComputeHash (const csRef<T>& key)
  {
    return (uint32)((T*)key);
  }

  static bool CompareKeys (const csRef<T>& key1, const csRef<T>& key2)
  {
    return ((T*)key1 == (T*)key2);
  }
};

/**
 * A hash key handler for struct type keys.
 */
template <class T>
class csStructKeyHandler
{
public:
  static uint32 ComputeHash (const csRef<T>& key)
  {
    return csHashCompute ((char*)&key, sizeof (T));
  }

  static bool CompareKeys (const csRef<T>& key1, const csRef<T>& key2)
  {
    return (memcmp (&key1, &key2, sizeof (T)) == 0);
  }
};

#endif // __CS_UTIL_HASHHANDLERS_H__
