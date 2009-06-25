/*
    Copyright (C) 2006 by Frank Richter

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

#ifndef __CS_IUTIL_ARRAY_H__
#define __CS_IUTIL_ARRAY_H__

/**\file
 * Templates for array interfaces.
 */

#include "csutil/scf.h"

/**\addtogroup util_containers
 * @{ */

/**
 * Array interface template, read-only.
 * The functions provided allow to enumerate and read all elements of an
 * array, but no modifications are permitted.
 *
 * This template can't be used as-is in another interface; rather, it must
 * be derived and specialized before it can be used. Example:
 * \code
 * struct csFoo { ... };
 * struct iFooArray : public iArrayReadOnly<csFoo>
 * {
 *   SCF_IARRAYREADONLY_INTERFACE(iFooArray);
 * };
 * \endcode
 *
 * Standard implementations for this interface are scfArray, scfArrayWrap and 
 * scfArrayWrapConst.
 */
template<typename T>
struct iArrayReadOnly : public virtual iBase
{
  typedef T ContainedType;

  /// Return the number of elements in the array.
  virtual size_t GetSize () const = 0;
  /// Get an element (const).
  virtual T const& Get (size_t n) const = 0;
  /// Return the top element but do not remove it (const).
  virtual T const& Top () const = 0;
  /**
   * Find an element in array.
   * \return csArrayItemNotFound if not found, else the item index.
   * \warning Performs a slow linear search. 
   */
  virtual size_t Find (T const& which) const = 0;
  /**
   * Given a pointer to an element in the array this function will return
   * the index. Note that this function does not check if the returned
   * index is actually valid. The caller of this function is responsible
   * for testing if the returned index is within the bounds of the array.
   */
  virtual size_t GetIndex (const T* which) const = 0;
  /**
   * Return true if the array is empty.
   * \remarks Rigidly equivalent to <tt>return GetSize() == 0</tt>, but more
   *   idiomatic.
   */
  virtual bool IsEmpty() const = 0;
  /**
   * Copy all elements to \p dest.
   * \p dest should be large enough to take all elements contained in the 
   * array.
   */
  virtual void GetAll (T* dest) const = 0;
};

/**
 * Macro to define interface version of an iArrayReadOnly-derived interface.
 * Use this instead of #SCF_INTERFACE to automatically get version number
 * bumps for your array interface in case the base iArrayReadOnly gets
 * extended.
 */
#define SCF_IARRAYREADONLY_INTERFACE(Name)				\
  SCF_INTERFACE(Name, 0, 0, 1)

/**
 * Array interface template, elements are changeable.
 * Extents iArrayReadOnly to also support modifying the existing elements;
 * however, new elements can not be added or existing elements removed or
 * reordered.
 *
 * This template can't be used as-is in another interface; rather, it must
 * be derived and specialized before it can be used. Example:
 * \code
 * struct csBar { ... };
 * struct iBarArray : public iArrayChangeElements<csBar>
 * {
 *   SCF_IARRAYCHANGEELEMENTS_INTERFACE(iBarArray);
 * };
 * \endcode
 *
 * Standard implementations for this interface are scfArray and scfArrayWrap.
 */
template<typename T>
struct iArrayChangeElements : public virtual iArrayReadOnly<T>
{
  /// Get an element (non-const).
  using iArrayReadOnly<T>::Get;
  virtual T& Get (size_t n) = 0;
  /// Return the top element but do not remove it (non-const).
  using iArrayReadOnly<T>::Top;
  virtual T& Top () = 0;
};

/**
 * Macro to define interface version of an iArrayChangeElements-derived 
 * interface.
 * Use this instead of #SCF_INTERFACE to automatically get version number
 * bumps for your array interface in case the base iArrayChangeElements gets
 * extended.
 */
#define SCF_IARRAYCHANGEELEMENTS_INTERFACE(Name)			\
  SCF_INTERFACE(Name, 0, 0, 1)

/**
 * Array interface template, completely changeable.
 * Extents iArrayChangeElements to also support adding new elements and
 * removing existing elements.
 *
 * This template can't be used as-is in another interface; rather, it must
 * be derived and specialized before it can be used. Example:
 * \code
 * struct csBaz { ... };
 * struct iBazArray : public iArrayChangeAll<csBaz>
 * {
 *   SCF_IARRAYCHANGEALL_INTERFACE(iBarArray);
 * };
 * \endcode
 *
 * Standard implementations for this interface are scfArray and scfArrayWrap.
 */
template<typename T>
struct iArrayChangeAll : public virtual iArrayChangeElements<T>
{
  /**
   * Set the actual number of items in this array. This can be used to shrink
   * an array (like Truncate()) or to enlarge an array, in which case it will
   * properly construct all new items based on the given item.
   * \param n New array length.
   * \param what Object used as template to construct each newly added object
   *   using the object's copy constructor when the array size is increased by
   *   this method.
   */
  virtual void SetSize (size_t n, T const& what) = 0;
  /**
   * Set the actual number of items in this array. This can be used to shrink
   * an array (like Truncate()) or to enlarge an array, in which case it will
   * properly construct all new items using their default (zero-argument)
   * constructor.
   * \param n New array length.
   */
  virtual void SetSize (size_t n) = 0;
  /**
   * Get an item from the array. If the number of elements in this array is too
   * small the array will be automatically extended, and the newly added
   * objects will be created using their default (no-argument) constructor.
   */
  virtual T& GetExtend (size_t n) = 0;
  /// Insert a copy of element at the indicated position.
  virtual void Put (size_t n, T const& what) = 0;
  /**
   * Push a copy of an element onto the tail end of the array.
   * \return Index of newly added element.
   */
  virtual size_t Push (T const& what) = 0;
  /**
   * Push a element onto the tail end of the array if not already present.
   * \return Index of newly pushed element or index of already present element.
   */
  virtual size_t PushSmart (T const& what) = 0;
  /// Pop an element from tail end of array.
  virtual T Pop () = 0;
  /// Insert element \c item before element \c n.
  virtual bool Insert (size_t n, T const& item) = 0;
  /// Clear entire array, releasing all allocated memory.
  virtual void DeleteAll () = 0;
  /**
   * Truncate array to specified number of elements. The new number of
   * elements cannot exceed the current number of elements.
   * \remarks Does not reclaim memory used by the array itself, though the
   *   removed objects are destroyed. To reclaim the array's memory invoke
   *   ShrinkBestFit(), or DeleteAll() if you want to release all allocated
   *   resources.
   *
   * \remarks The more general-purpose SetSize() method can also enlarge the
   *   array.
   */
  virtual void Truncate (size_t n) = 0;
  /**
   * Remove all elements.  Similar to DeleteAll(), but does not release memory
   * used by the array itself, thus making it more efficient for cases when the
   * number of contained elements will fluctuate.
   */
  virtual void Empty () = 0;
  /**
   * Delete an element from the array.
   * return True if the indicated item index was valid, else false.
   * \remarks Deletion speed is proportional to the size of the array and the
   *   location of the element being deleted. If the order of the elements in
   *   the array is not important, then you can instead use DeleteIndexFast()
   *   for constant-time deletion.
  */
  virtual bool DeleteIndex (size_t n) = 0;
  /**
   * Delete an element from the array in constant-time, regardless of the
   * array's size.
   * return True if the indicated item index was valid, else false.
   * \remarks This is a special version of DeleteIndex() which does not
   *   preserve the order of the remaining elements. This characteristic allows
   *   deletions to be performed in constant-time, regardless of the size of
   *   the array.
   */
  virtual bool DeleteIndexFast (size_t n) = 0;
  /**
   * Delete the given element from the array.
   * \remarks Performs a linear search of the array to locate \c item, thus it
   *   may be slow for large arrays.
   */
  virtual bool Delete (T const& item) = 0;
};

/**
 * Macro to define interface version of an iArrayChangeAll-derived 
 * interface.
 * Use this instead of #SCF_INTERFACE to automatically get version number
 * bumps for your array interface in case the base iArrayChangeAll gets
 * extended.
 */
#define SCF_IARRAYCHANGEALL_INTERFACE(Name)				\
  SCF_INTERFACE(Name, 0, 1, 0)

/** @} */

#endif // __CS_IUTIL_ARRAY_H__
