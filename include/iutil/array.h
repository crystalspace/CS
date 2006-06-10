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

#include "csutil/scf.h"

template<typename T>
struct iArrayReadOnly : public virtual iBase
{
  typedef T ContainedType;

  virtual size_t GetSize () const = 0;
  virtual T const& Get (size_t n) const = 0;
  virtual T const& Top () const = 0;
  virtual size_t Find (T const& which) const = 0;
  virtual size_t GetIndex (const T* which) const = 0;
  virtual bool IsEmpty() const = 0;
  virtual void GetAll (T* dest) const = 0;
};

#define SCF_IARRAYREADONLY_INTERFACE(Name)				\
  SCF_INTERFACE(Name, 0, 0, 1)

template<typename T>
struct iArrayChangeElements : public virtual iArrayReadOnly<T>
{
  virtual T& Get (size_t n) = 0;
  virtual T& Top () = 0;
};

#define SCF_IARRAYCHANGEELEMENTS_INTERFACE(Name)			\
  SCF_INTERFACE(Name, 0, 0, 1)

template<typename T>
struct iArrayChangeAll : public virtual iArrayChangeElements<T>
{
  virtual void SetSize (size_t n, T const& what) = 0;
  virtual void SetSize (size_t n) = 0;
  virtual T& GetExtend (size_t n) = 0;
  virtual void Put (size_t n, T const& what) = 0;
  virtual size_t Push (T const& what) = 0;
  virtual size_t PushSmart (T const& what) = 0;
  virtual T Pop () = 0;
  virtual bool Insert (size_t n, T const& item) = 0;
  virtual void DeleteAll () = 0;
  virtual void Truncate (size_t n) = 0;
  virtual void Empty () = 0;
  virtual bool DeleteIndex (size_t n) = 0;
  virtual bool DeleteIndexFast (size_t n) = 0;
  virtual bool Delete (T const& item) = 0;
  virtual bool DeleteFast (T const& item) = 0;
};

#define SCF_IARRAYCHANGEALL_INTERFACE(Name)				\
  SCF_INTERFACE(Name, 0, 0, 1)

#endif // __CS_IUTIL_ARRAY_H__
