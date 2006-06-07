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

#include "iutil/array.h"

#include "csutil/array.h"
#include "csutil/scf_implementation.h"

template<typename IF, typename T, typename Backend = csArray<T> >
class scfArray : 
  public scfImplementation1<scfArray<IF, T, Backend>, IF>
{
public:
  Backend storage;

  scfArray () : scfImplementationType (this) {}
  scfArray (const Backend& storage) : scfImplementationType (this), 
    storage (storage) {}

  /**\name iArrayReadOnly<T> implementation
   * @{ */
  virtual size_t GetSize () const
  { return storage.GetSize(); }
  virtual T const& Get (size_t n) const
  { return storage.Get (n); }
  virtual T const& Top () const
  { return storage.Top(); }
  virtual size_t Find (T const& which) const
  { return storage.Find (which); }
  virtual size_t GetIndex (const T* which) const
  { return storage.GetIndex (which); }
  virtual bool IsEmpty() const
  { return storage.IsEmpty(); }
  virtual void GetAll (T* dest) const
  {
    for (size_t i = 0; i < storage.GetSize(); i++)
      dest[i] = storage[i];
  }
  /** @} */
  
  /**\name iArrayChangeElements<T> implementation
   * @{ */
  virtual T& Get (size_t n)
  { return storage.Get (n); }
  virtual T& Top ()
  { return storage.Top(); }
  /** @} */

  /**\name iArrayChangeAll<T> implementation
   * @{ */
  virtual void SetSize (size_t n, T const& what)
  { storage.SetSize (n, what); }
  virtual void SetSize (size_t n)
  { storage.SetSize (n); }
  virtual T& GetExtend (size_t n)
  { return storage.GetExtend (n); }
  virtual void Put (size_t n, T const& what)
  { storage.Put (n, what); }
  virtual size_t Push (T const& what)
  { return storage.Push (what); }
  virtual size_t PushSmart (T const& what)
  { return storage.PushSmart (what); }
  virtual T Pop ()
  { return storage.Pop (); }
  virtual bool Insert (size_t n, T const& item)
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
  virtual bool Delete (T const& item)
  { return storage.Delete (item); }
  virtual bool DeleteFast (T const& item)
  { return storage.DeleteFast (item); }
  /** @} */
};

template<typename IF, typename T, typename Backend = csArray<T> >
class scfArrayWrap : 
  public scfImplementation1<scfArrayWrap<IF, T, Backend>, IF>
{
public:
  Backend& storage;

  scfArrayWrap (Backend& storage) : scfImplementationType (this), 
    storage (storage) {}

  /**\name iArrayReadOnly<T> implementation
   * @{ */
  virtual size_t GetSize () const
  { return storage.GetSize(); }
  virtual T const& Get (size_t n) const
  { return storage.Get (n); }
  virtual T const& Top () const
  { return storage.Top(); }
  virtual size_t Find (T const& which) const
  { return storage.Find (which); }
  virtual size_t GetIndex (const T* which) const
  { return storage.GetIndex (which); }
  virtual bool IsEmpty() const
  { return storage.IsEmpty(); }
  virtual void GetAll (T* dest) const
  {
    for (size_t i = 0; i < storage.GetSize(); i++)
      dest[i] = storage[i];
  }
  /** @} */
  
  /**\name iArrayChangeElements<T> implementation
   * @{ */
  virtual T& Get (size_t n)
  { return storage.Get (n); }
  virtual T& Top ()
  { return storage.Top(); }
  /** @} */

  /**\name iArrayChangeAll<T> implementation
   * @{ */
  virtual void SetSize (size_t n, T const& what)
  { storage.SetSize (n, what); }
  virtual void SetSize (size_t n)
  { storage.SetSize (n); }
  virtual T& GetExtend (size_t n)
  { return storage.GetExtend (n); }
  virtual void Put (size_t n, T const& what)
  { storage.Put (n, what); }
  virtual size_t Push (T const& what)
  { return storage.Push (what); }
  virtual size_t PushSmart (T const& what)
  { return storage.PushSmart (what); }
  virtual T Pop ()
  { return storage.Pop (); }
  virtual bool Insert (size_t n, T const& item)
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
  virtual bool Delete (T const& item)
  { return storage.Delete (item); }
  virtual bool DeleteFast (T const& item)
  { return storage.DeleteFast (item); }
  /** @} */
};

template<typename IF, typename T, typename Backend = csArray<T> >
class scfArrayWrapConst : 
  public scfImplementation1<scfArrayWrapConst<IF, T, Backend>, IF>
{
public:
  const Backend& storage;

  scfArrayWrapConst (const Backend& storage) : scfImplementationType (this), 
    storage (storage) {}

  /**\name iArrayReadOnly<T> implementation
   * @{ */
  virtual size_t GetSize () const
  { return storage.GetSize(); }
  virtual T const& Get (size_t n) const
  { return storage.Get (n); }
  virtual T const& Top () const
  { return storage.Top(); }
  virtual size_t Find (T const& which) const
  { return storage.Find (which); }
  virtual size_t GetIndex (const T* which) const
  { return storage.GetIndex (which); }
  virtual bool IsEmpty() const
  { return storage.IsEmpty(); }
  virtual void GetAll (T* dest) const
  {
    for (size_t i = 0; i < storage.GetSize(); i++)
      dest[i] = storage[i];
  }
  /** @} */
};

#endif // __CS_CSUTIL_SCFARRAY_H__
