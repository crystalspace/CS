/*
    Crystal Space utility library: vector class interface
    Copyright (C) 1998,1999,2000 by Andrew Zabolotny <bit@eltech.ru>

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

#include "cstypes.h"

/**
 * This is a lightweight base class for containers.  It contains no virtual
 * functions and, by default, does not preallocate any memory.  It contains all
 * the basic functionality of the derived csVector class, but none of the
 * gimmicks.
 */
class csBasicVector
{
protected:
  int count,limit,threshold;
  csSome *root;

public:
  /**
   * Initialize object to hold initially 'ilimit' elements, and increase
   * storage by 'ithreshold' each time the upper bound is exceeded.
   */
  csBasicVector (int ilimit = 0, int ithreshold = 0);
  
  /// Destroy the container but none of the objects to which it points.
  virtual ~csBasicVector();

  /// Get a reference to n-th element
  inline csSome& operator [] (int n);
  /// Same but doesn't call SetLength () in the event n is out of bounds
  inline csSome& operator [] (int n) const;
  /// Same but in function form
  inline csSome& Get (int n) const;

  /// Set vector length to n
  void SetLength (int n);

  /// Query vector length
  inline int Length () const;
  /// Query vector limit
  inline int Limit () const;

  /// Delete element number 'n' from vector (attention: non virtual!)
  bool Delete (int n);

  /// Exchange two elements in array
  inline void Exchange (int n1, int n2);
  /// Find a element in array and return its index (or -1 if not found)
  int Find (csSome which) const;

  /// Push a element on 'top' of vector
  inline int Push (csSome what);
  /// Push an elemen on top of the vector if it is not yet contained
  inline int PushSmart (csSome what);
  /// Pop a element from vector 'top'
  inline csSome Pop ();

  /// Insert element 'Item' before element 'n'
  bool Insert (int n, csSome Item);
};

/**
 * csVector is an abstract class which can hold an unlimited array
 * of unspecified (void*) data. Since this is a basic object, it does
 * not presume anything about its elements, so FreeItem() is
 * effectively a no-op. If you want vector elements to free automatically
 * occupied memory upon vector destruction you should create a derived
 * class which should provide its own FreeItem() method (see csStrVector
 * for a example).<p>
 * Note that FreeItem() returns a boolean value which is the success
 * status. This is used in Delete() and DeleteAll() to decide whenever
 * an element can be really deleted - if the element has a good reason to
 * stay 'sticky' it can return false from FreeItem().
 */
class csVector : public csBasicVector
{
public:
  /**
   * Initialize object to hold initially 'ilimit' elements, and increase
   * storage by 'ithreshold' each time the upper bound is exceeded.
   */
  csVector (int ilimit = 8, int ithreshold = 16) 
    : csBasicVector(ilimit, ithreshold) {}

  /// Destroy the vector object.
  virtual ~csVector () {}

  /// Find a element by key (using CompareKey method)
  int FindKey (csConstSome Key, int Mode = 0) const;
  /// Find a element in a SORTED array by key (using CompareKey method)
  int FindSortedKey (csConstSome Key, int Mode = 0) const;
  /// Partially sort the array
  void QuickSort (int Left, int Right, int Mode = 0);
  /// Same but for all elements
  void QuickSort (int Mode = 0);

  /// Delete element number 'n' from vector
  bool Delete (int n, bool FreeIt = true);
  /// Replace n-th item with another (free previous value)
  bool Replace (int n, csSome what, bool FreePrevious = true);
  /// Delete all elements
  void DeleteAll (bool FreeThem = true);

  /// Insert element 'Item' so that array remains sorted (assumes its already)
  int InsertSorted (csSome Item, int *oEqual = NULL, int Mode = 0);
  /// Virtual function which frees a vector element; returns success status
  virtual bool FreeItem (csSome Item);
  /// Compare two array elements in given Mode
  virtual int Compare (csSome Item1, csSome Item2, int Mode) const;
  /// Compare entry with a key; for csVector just compare (int)Key vs (int)Item
  virtual int CompareKey (csSome Item, csConstSome Key, int Mode) const;
};

inline csSome& csBasicVector::operator [] (int n)
{
  CS_ASSERT (n >= 0);
  if (n >= count)
    SetLength (n + 1);
  return (root [n]);
}

inline csSome& csBasicVector::operator [] (int n) const
{
  CS_ASSERT (n >= 0);
  CS_ASSERT (n < count);
  return (root [n]);
}

inline csSome& csBasicVector::Get (int n) const
{
  CS_ASSERT (n >= 0);
  CS_ASSERT (n < count);
  return (root [n]);
}

inline int csBasicVector::Limit () const
{
  return (limit);
}

inline int csBasicVector::Length () const
{
  return (count);
}

inline int csBasicVector::Push (csSome what)
{
  SetLength (count + 1);
  root [count - 1] = what;
  return (count - 1);
}

inline int csBasicVector::PushSmart (csSome what)
{
  int n = Find (what);
  return (n == -1) ? Push (what) : n;
}

inline csSome csBasicVector::Pop ()
{
  csSome ret = root [count - 1];
  SetLength (count - 1);
  return (ret);
}

inline void csBasicVector::Exchange (int n1, int n2)
{
  csSome tmp = root [n1];
  root [n1] = root [n2];
  root [n2] = tmp;
}

inline void csVector::QuickSort (int Mode)
{
  if (count > 0)
    QuickSort (0, count - 1, Mode);
}

#endif // __CSVECTOR_H__
