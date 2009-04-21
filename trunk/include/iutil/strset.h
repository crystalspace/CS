/*
    Copyright (C) 2003 by Anders Stenberg

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

#ifndef __CS_IUTIL_STRINGSET_H__
#define __CS_IUTIL_STRINGSET_H__

/**\file
 * Stringset interface
 */
/**\addtogroup util
 * @{ */
#include "csutil/scf_interface.h"

namespace CS
{
  /// Type for the actual value of a string ID
  typedef unsigned int StringIDValue;

  /**
   * An identifier for a string. This identifier is equivalent to the contents
   * of a string: If two strings have the same content, they have get the same
   * identifier. If they have different content, they get different identifiers.
   *
   * String IDs have a "tag" which identifies for what the string ID is used.
   * They also protect against mixing string IDs used for different purposes.
   */
  template<typename Tag>
  struct StringID
  {
  protected:
    StringIDValue id;
    
  public:
    StringID() {}
    StringID (const StringID& other) : id (other.id) {}
    StringID (StringIDValue id) : id (id) {}
    
    // Detect attempts to mix string IDs with different tags
    template<typename Tag2>
    CS_DEPRECATED_METHOD_MSG("Mixing string IDs with different tags may "
      "indicate an error. Explicit construct if really needed")
    StringID (const StringID<Tag2>& other) : id ((StringIDValue)other) {}
    
    StringID& operator=(const StringID& other)
    {
      id = other.id;
      return *this;
    }
    
    // Detect attempts to mix string IDs with different tags
    template<typename Tag2>
    CS_DEPRECATED_METHOD_MSG("Mixing string IDs with different tags may "
      "indicate an error. Explicit construct if really needed")
    StringID& operator=(const StringID<Tag2>& other)
    {
      id = (StringIDValue)other;
      return *this;
    }
    
    operator StringIDValue() const { return id; }
    
    unsigned int GetHash() const { return id; }
  };
  
  /// An \e invalid csStringID.
  template<typename Tag>
  struct InvalidStringID : public StringID<Tag>
  {
    InvalidStringID() { this->id = (StringIDValue)~0; }
  };
} // namespace CS


/**
 * The string set is a collection of unique strings. Each string has an ID
 * number. The most important operation is to request a string, which means to
 * return the ID for the string, adding it to the collection if not already
 * present.  This is useful when you need to work with strings but want the
 * performance characteristics of simple numeric comparisons.  Rather than
 * performing string comparisons, you instead compare the numeric string ID's.
 *
 * As a convenience, the csInitializer class creates a string set at
 * application initialization time and inserts it into the global object
 * registry (iObjectRegistry).  To obtain a reference this shared string set
 * (which may be desirable when modules need to share string ID's), use a code
 * snippet similar to the following:
 *
 * \code
 * iObjectRegistry* object_reg = ...;
 * csRef<iStringSet> strings = csQueryRegistryTagInterface<iStringSet> (
 *   object_reg, "crystalspace.shared.stringset");
 * csStringID myid = strings->Request ("crystalspace.mygame.mystring");
 * \endcode
 *
 * \sa csStringHash
 * \sa csStringSet
 */
template<typename Tag>
struct iStringSetBase : public virtual iBase
{
  typedef Tag TagType;

  //SCF_INTERFACE(iStringSet, 2,0,0);
  /**
   * Request the numeric ID for the given string.
   * \return The ID of the string.
   * \remarks Creates a new ID if the string is not yet present in the set,
   *   else returns the previously assigned ID.
   */
  virtual CS::StringID<Tag> Request(const char*) = 0;

  /**
   * Request the string corresponding to the given ID.
   * \return Null if the string * has not been requested (yet), else the string
   *   corresponding to the ID.
   */
  virtual const char* Request(CS::StringID<Tag>) const = 0;

  /**
   * Check if the set contains a particular string.
   */
  virtual bool Contains(char const*) const = 0;

  /**
   * Check if the set contains a string with a particular ID.
   * \remarks This is rigidly equivalent to
   *   <tt>return Request(id) != NULL</tt>, but more idiomatic.
   */
  virtual bool Contains(CS::StringID<Tag>) const = 0;

  /**
   * Remove specified string.
   * \return True if a matching string was in thet set; else false.
   */
  virtual bool Delete(char const*) = 0;

  /**
   * Remove a string with the specified ID.
   * \return True if a matching string was in the set; else false.
   */
  virtual bool Delete(CS::StringID<Tag>) = 0;

  /**
   * Remove all stored strings. When new strings are registered again, new
   * ID values will be used; the old ID's will not be re-used.
   */
  virtual void Empty() = 0;

  /**
   * Remove all stored strings.
   * \deprecated Deprecated in 1.3. Use Empty() instead.
   */
  CS_DEPRECATED_METHOD_MSG("Use Empty() instead.")
  virtual void Clear() = 0;

  /// Get the number of elements in the hash.
  virtual size_t GetSize () const = 0;

  /**
   * Return true if the hash is empty.
   * \remarks Rigidly equivalent to <tt>return GetSize() == 0</tt>, but more
   *   idiomatic.
   */
  virtual bool IsEmpty() const = 0;
};

#define CS_ISTRINGSSET_SCF_VERSION(Class)   SCF_INTERFACE(Class, 2, 0, 0)

namespace CS
{
  namespace StringSetTag
  {
    struct General;
  } // namespace StringSetTag
} // namespace CS

#if defined(SWIG)
// Templated classes must be declared before we actually use
// them, so we have to declare the following here.
%template(iGeneralStringSetBase) iStringSetBase<CS::StringSetTag::General>;
#endif

/// General string ID string set
struct iStringSet : public iStringSetBase<CS::StringSetTag::General>
{
  CS_ISTRINGSSET_SCF_VERSION(iStringSet);
};

/// General string ID
typedef CS::StringID<CS::StringSetTag::General> csStringID;
/// Invalid string ID
csStringID const csInvalidStringID = CS::InvalidStringID<CS::StringSetTag::General> ();

/** @} */

#endif // __CS_IUTIL_STRINGSET_H__
