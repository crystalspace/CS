/*
  Crystal Space Windowing System: vector class interface
  Copyright (C) 1998 by Jorrit Tyberghein
  Written by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CSVECTOR_H__
#define __CSVECTOR_H__

#include "csutil/csbase.h"
#include "types.h"	// for bool

typedef void *csSome;
typedef const void *csConstSome;

/**
 * csVector is an abstract class which can hold an unlimited array
 * of unspecified (void *) data. Since this is a basic object, it does
 * not presume anything about its elements, so FreeItem () is
 * effectively a NOP. If you want vector elements to free automatically
 * occupied memory upon vector destruction you should create a derived
 * class which should provide its own FreeItem () method (see csStrVector
 * for a example).
 */
class csVector : public csBase
{
protected:
  int count,limit,threshold;
  csSome *root;

public:
  /**
   * Initialize object to hold initially 'ilimit' elements, and increase
   * storage by 'ithreshold' each time the upper bound is exceeded.
   */
  csVector (int ilimit = 8, int ithreshold = 16);
  /// Destroy the vector object
  virtual ~csVector ();

  /// Get a reference to n-th element
  inline csSome& operator [] (int n);
  /// Same but doesn't call SetLength () in the event n is out of bounds
  inline csSome& Get (int n) const;
  inline csSome& operator [] (int n) const { return Get(n); }
  /// Set vector length to n
  void SetLength (int n);
  /// Query vector length
  inline int Length () const;
  /// Find a element in array and return its index (or -1 if not found)
  int Find (csSome which) const;
  /// Find a element by key (using Equal method)
  int FindKey (csConstSome value) const;
  /// Push a element on 'top' of vector
  inline void Push (csSome what);
  /// Pop a element from vector 'top'
  inline csSome Pop ();
  /// Delete element number 'n' from vector
  bool Delete (int n);
  /// Delete all elements
  void DeleteAll ();
  /// Insert element 'Item' before element 'n'
  bool Insert (int n, csSome Item);
  /// Virtual function which frees a vector element; returns success status
  virtual bool FreeItem (csSome Item);
  /// Compare a element with given key for equality
  virtual bool Equal (csSome Item, csConstSome Key) const;
};

inline csSome& csVector::operator [] (int n)
{
  if (n >= count)
    SetLength (n + 1);
  return (root[n]);
}

inline csSome& csVector::Get (int n) const
{
  return (root[n]);
}

inline int csVector::Length () const
{
  return (count);
}

inline void csVector::Push (csSome what)
{
  SetLength (count + 1);
  root[count - 1] = what;
}

inline csSome csVector::Pop ()
{
  csSome ret = root[count - 1];
  SetLength (count - 1);
  return (ret);
}

/* This macro will create a typesafe wrapper for csVector
 * This class will only manage pointers to Types, but this is still better than
 * nothing.
 *
 * For example, if you want to use a csVector, that contains pointers to integers,
 * you could just add the following line to your header:
 * DECLARE_TYPED_VECTOR(int)
 * this wil declare a new class called csVector_int for you, which can be used just
 * like csVector, just without that nasty casting.
 */
#define DECLARE_TYPED_VECTOR(TYPE) \
class csVector_##TYPE : protected csVector \
{ \
public: \
  csVector_##TYPE(int ilimit = 8, int ithreshold = 16) : csVector(ilimit, ithreshold) {} \
  inline TYPE*& operator [] (int n) {return (TYPE*&) csVector::operator[](n);} \
  inline TYPE*& Get (int n) const   {return (TYPE*&) csVector::Get(n);} \
  inline TYPE*& operator [] (int n) const { return Get(n); } \
  void SetLength (int n) {csVector::SetLength(n);} \
  inline int Length () const {return csVector::Length();} \
  int Find (TYPE* which) const {return csVector::Find(which);} \
  int FindKey (const TYPE* value) const {return csVector::FindKey(value);}\
  inline void Push (TYPE* what) {csVector::Push(what);}\
  inline TYPE* Pop () {return (TYPE*)csVector::Pop();}\
  bool Delete (int n) {return csVector::Delete(n);}\
  void DeleteAll () {csVector::DeleteAll();}\
  bool Insert (int n, TYPE* Item) {return Insert(n, Item);}\
  virtual bool FreeItem (TYPE* Item) {return FreeItem(Item);}\
  virtual bool Equal (TYPE* Item, const TYPE* Key) const {return Equal(Item, Key);}\
};

#endif // __CSVECTOR_H__
