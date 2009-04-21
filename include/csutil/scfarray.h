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

#ifndef __CS_CSUTIL_SCFARRAY_H__
#define __CS_CSUTIL_SCFARRAY_H__

/**\file
 * Implementation for iArrayReadOnly<>-, iArrayChangeElements<>- and
 * iArrayChangeAll<>-derived interfaces.
 */

#include "iutil/array.h"

#include "csutil/array.h"
#include "csutil/scf_implementation.h"

/**\addtogroup util_containers
 * @{ */

/**
 * Implementation for iArrayReadOnly<>-, iArrayChangeElements<>- and
 * iArrayChangeAll<>-derived interfaces, backed by a per-instance array.
 * The \p IF template parameter denotes the array interface to be implemented, 
 * the optional \p Backend remplate parameter the array type internally used
 * for storage.
 *
 * Usage example:
 * \code
 * struct iSomeArray : public iArrayChangeElements<...> { ... };
 *
 * void MyClass::MyMethod()
 * {
 *   csRef<iSomeArray> array;
 *   array.AttachNew (new scfArray<iSomeArray>);
 *   ...
 * }
 * \endcode
 */
template<typename IF, 
  typename Backend = csArray<typename IF::ContainedType> >
class scfArray : 
  public scfImplementation1<scfArray<IF, Backend>, IF>
{
  typedef scfImplementation1<scfArray<IF, Backend>, IF> scfImplementationType;
  typedef typename IF::ContainedType ContainedType;
public:
  /// The array storage
  Backend storage;

  //@{
  /// Construct with empty storage.
  scfArray () : scfImplementationType (this) {}
  scfArray (iBase* scfParent) : scfImplementationType (this, scfParent) {}
  //@}
  //@{
  /// Construct and copy to storage contents from given array.
  scfArray (const Backend& storage) : scfImplementationType (this), 
    storage (storage) {}
  scfArray (const Backend& storage, iBase* scfParent) : 
    scfImplementationType (this, scfParent), storage (storage) {}
  //@}

  /**\name iArrayReadOnly<> implementation
   * @{ */
  virtual size_t GetSize () const
  { return storage.GetSize(); }
  virtual ContainedType const& Get (size_t n) const
  { return storage.Get (n); }
  virtual ContainedType const& Top () const
  { return storage.Top(); }
  virtual size_t Find (ContainedType const& which) const
  { return storage.Find (which); }
  virtual size_t GetIndex (const ContainedType* which) const
  { return storage.GetIndex (which); }
  virtual bool IsEmpty() const
  { return storage.IsEmpty(); }
  virtual void GetAll (ContainedType* dest) const
  {
    for (size_t i = 0; i < storage.GetSize(); i++)
      dest[i] = storage[i];
  }
  /** @} */
  
  /**\name iArrayChangeElements<> implementation
   * @{ */
  virtual ContainedType& Get (size_t n)
  { return storage.Get (n); }
  virtual ContainedType& Top ()
  { return storage.Top(); }
  /** @} */

  /**\name iArrayChangeAll<> implementation
   * @{ */
  virtual void SetSize (size_t n, ContainedType const& what)
  { storage.SetSize (n, what); }
  virtual void SetSize (size_t n)
  { storage.SetSize (n); }
  virtual ContainedType& GetExtend (size_t n)
  { return storage.GetExtend (n); }
  virtual void Put (size_t n, ContainedType const& what)
  { storage.Put (n, what); }
  virtual size_t Push (ContainedType const& what)
  { return storage.Push (what); }
  virtual size_t PushSmart (ContainedType const& what)
  { return storage.PushSmart (what); }
  virtual ContainedType Pop ()
  { return storage.Pop (); }
  virtual bool Insert (size_t n, ContainedType const& item)
  { return storage.Insert (n, item); }
  virtual void DeleteAll ()
  { storage.DeleteAll(); }
  virtual void Truncate (size_t n)
  { storage.Truncate(n); }
  virtual void Empty ()
  { storage.Empty(); }
  virtual bool DeleteIndex (size_t n)
  { return storage.DeleteIndex  (n); }
  virtual bool DeleteIndexFast (size_t n)
  { return storage.DeleteIndexFast  (n); }
  virtual bool Delete (ContainedType const& item)
  { return storage.Delete (item); }
  /** @} */
};

/**
 * Implementation for iArrayReadOnly<>-, iArrayChangeElements<>- and
 * iArrayChangeAll<>-derived interfaces, backed by a reference to another
 * array.
 * The \p IF template parameter denotes the array interface to be implemented, 
 * the \p Backend template parameter the array type used for storage.
 * \warning It must be ensured that the referenced array used for storage is
 *  alive as long as any instance of scfArrayWrap exists!
 */
template<typename IF, typename Backend>
class scfArrayWrap : 
  public scfImplementation1<scfArrayWrap<IF, Backend>, IF>
{
  typedef scfImplementation1<scfArrayWrap<IF, Backend>, IF> 
    scfImplementationType;
  typedef typename IF::ContainedType ContainedType;
public:
  /// Reference to the array storage.
  Backend& storage;

  //@{
  /// Initialize with a reference to the given storage.
  scfArrayWrap (Backend& storage) : scfImplementationType (this), 
    storage (storage) {}
  scfArrayWrap (Backend& storage, iBase* scfParent) : 
    scfImplementationType (this, scfParent), storage (storage) {}
  //@}

  /**\name iArrayReadOnly<> implementation
   * @{ */
  virtual size_t GetSize () const
  { return storage.GetSize(); }
  virtual ContainedType const& Get (size_t n) const
  { return storage.Get (n); }
  virtual ContainedType const& Top () const
  { return storage.Top(); }
  virtual size_t Find (ContainedType const& which) const
  { return storage.Find (which); }
  virtual size_t GetIndex (const ContainedType* which) const
  { return storage.GetIndex (which); }
  virtual bool IsEmpty() const
  { return storage.IsEmpty(); }
  virtual void GetAll (ContainedType* dest) const
  {
    for (size_t i = 0; i < storage.GetSize(); i++)
      dest[i] = storage[i];
  }
  /** @} */
  
  /**\name iArrayChangeElements<> implementation
   * @{ */
  virtual ContainedType& Get (size_t n)
  { return storage.Get (n); }
  virtual ContainedType& Top ()
  { return storage.Top(); }
  /** @} */

  /**\name iArrayChangeAll<> implementation
   * @{ */
  virtual void SetSize (size_t n, ContainedType const& what)
  { storage.SetSize (n, what); }
  virtual void SetSize (size_t n)
  { storage.SetSize (n); }
  virtual ContainedType& GetExtend (size_t n)
  { return storage.GetExtend (n); }
  virtual void Put (size_t n, ContainedType const& what)
  { storage.Put (n, what); }
  virtual size_t Push (ContainedType const& what)
  { return storage.Push (what); }
  virtual size_t PushSmart (ContainedType const& what)
  { return storage.PushSmart (what); }
  virtual ContainedType Pop ()
  { return storage.Pop (); }
  virtual bool Insert (size_t n, ContainedType const& item)
  { return storage.Insert (n, item); }
  virtual void DeleteAll ()
  { storage.DeleteAll(); }
  virtual void Truncate (size_t n)
  { storage.Truncate(n); }
  virtual void Empty ()
  { storage.Empty(); }
  virtual bool DeleteIndex (size_t n)
  { return storage.DeleteIndex  (n); }
  virtual bool DeleteIndexFast (size_t n)
  { return storage.DeleteIndexFast  (n); }
  virtual bool Delete (ContainedType const& item)
  { return storage.Delete (item); }
  /** @} */
};

/**
 * Implementation for iArrayReadOnly<>-derived interfaces, backed by a 
 * reference to another array.
 * The \p IF template parameter denotes the array interface to be implemented, 
 * the \p Backend template parameter the array type used for storage.
 * \warning It must be ensured that the referenced array used for storage is
 *  alive as long as any instance of scfArrayWrap exists!
 */
template<typename IF, typename Backend>
class scfArrayWrapConst : 
  public scfImplementation1<scfArrayWrapConst<IF, Backend>, IF>
{
  typedef scfImplementation1<scfArrayWrapConst<IF, Backend>, IF> 
    scfImplementationType;
  typedef typename IF::ContainedType ContainedType;
public:
  /// Reference to the array storage.
  const Backend& storage;

  //@{
  /// Initialize with a reference to the given storage.
  scfArrayWrapConst (const Backend& storage) : scfImplementationType (this), 
    storage (storage) {}
  scfArrayWrapConst (const Backend& storage, iBase* scfParent) : 
    scfImplementationType (this, scfParent), storage (storage) {}
  //@}

  /**\name iArrayReadOnly<> implementation
   * @{ */
  virtual size_t GetSize () const
  { return storage.GetSize(); }
  virtual ContainedType const& Get (size_t n) const
  { return storage.Get (n); }
  virtual ContainedType const& Top () const
  { return storage.Top(); }
  virtual size_t Find (ContainedType const& which) const
  { return storage.Find (which); }
  virtual size_t GetIndex (const ContainedType* which) const
  { return storage.GetIndex (which); }
  virtual bool IsEmpty() const
  { return storage.IsEmpty(); }
  virtual void GetAll (ContainedType* dest) const
  {
    for (size_t i = 0; i < storage.GetSize(); i++)
      dest[i] = storage[i];
  }
  /** @} */
};

/** @} */

#endif // __CS_CSUTIL_SCFARRAY_H__
