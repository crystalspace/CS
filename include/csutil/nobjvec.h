/*
    Crystal Space: Named Object Vector class
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __NOBJVEC_H__
#define __NOBJVEC_H__

#include "csutil/csobjvec.h"
#include "csutil/typedvec.h"
#include "iutil/object.h"

class csObject;
struct iObject;

SCF_DECLARE_FAST_INTERFACE (iObject);

/**
 * csNamedObjVector is a version of csObjVector that assumes all
 * its components are csObject's which can be examined by name etc.
 * All csVector methods should work with this class: Find, FindKey,
 * QuickSort, FindSortedKey and so on.
 */
class csNamedObjVector : public csObjVector
{
public:
  /// Constructor just passes control to csVector's
  csNamedObjVector (int ilimit = 8, int ithreshold = 16) :
    csObjVector (ilimit, ithreshold) {}

  /// Find an item in this vector by name and returns it (or NULL if not found)
  csObject *FindByName (const char* iName) const;

  /// Compare two objects by their names
  virtual int Compare (csSome Item1, csSome Item2, int Mode) const;

  /// Compare object's name with a string
  virtual int CompareKey (csSome Item, csConstSome Key, int Mode) const;

  /// Override Get() to avoid casting to csObject
  csObject *Get (int idx) const
  { return (csObject *)csVector::Get (idx); }
};

/**
 * Object Vectors: A subset of typed vectors. They assume that it is possible
 * to cast the contained objects from and to (iBase*). This pointer is then
 * used to query the iObject interface form the contained objects.
 */

/**
 * This class is intended as a superclass for typed object vectors. Instances
 * of this class itself should not be created; However it is valid to cast
 * a typed object vector to this class. It is not possible to add objects to
 * the vector though this interface, because this interface does not know
 * which type of objects is actually contained.
 */
class csNamedObjectVector : protected csIBaseVector
{
protected:
  /// Find an object by name
  csSome FindByName (const char* iName) const;
  /// Compare two objects by their names
  virtual int Compare (csSome Item1, csSome Item2, int Mode) const;
  /// Compare object's name with a string
  virtual int CompareKey (csSome Item, csConstSome Key, int Mode) const;

public:
  /// constructor
  csNamedObjectVector (int iLimit = 8, int iThreshold = 16) :
    csIBaseVector (iLimit, iThreshold) {}

  /// Set the length of this vector
  inline void SetLength (int n)
  { csIBaseVector::SetLength(n); }
  /// Return the length of this vector
  inline int Length () const
  { return count; }
  /// Return the limit of this vector
  inline int Limit () const
  { return limit; }
  /// Exchange two elements of the vector
  inline void Exchange (int n1, int n2)
  { csIBaseVector::Exchange (n1, n2); }
  /// Quick-sort the elements, using the given mode
  inline void QuickSort (int Left, int Right, int Mode = 0)
  { csIBaseVector::QuickSort (Left, Right, Mode); }
  /// Quick-sort the elements, using the given mode
  inline void QuickSort (int Mode = 0)
  { csIBaseVector::QuickSort (Mode); }
  /// Find an element using the given key
  inline int FindKey (csConstSome Key, int Mode = 0) const
  { return csIBaseVector::FindKey (Key, Mode); }
  /// Find an element using the given key. Expects the elements to be sorted.
  inline int FindSortedKey (csConstSome Key, int Mode = 0) const
  { return csIBaseVector::FindSortedKey (Key, Mode); }
  /// Pop an element from the vector.
  inline iObject *PopObject ()
  { return SCF_QUERY_INTERFACE_FAST (((iBase*)csIBaseVector::Pop()), iObject); }
  /// Delete an element from the vector
  inline bool Delete (int n)
  { return csIBaseVector::Delete (n); }
  /// Delete all elements
  inline void DeleteAll ()
  { csIBaseVector::DeleteAll (); }
  /// Return the pointer to an element
  inline iObject *GetObject (int n) const
  { return SCF_QUERY_INTERFACE_FAST (((iBase*)(csIBaseVector::Get(n))), iObject); }
};

/**
 * Begin the class definition of an object vector. Be careful with
 * user-defined methods in this class: Due to the 'protected' inheritance
 * they can access members and methods of csVector (not type-safe!). <p>
 *
 * Note that exploiting the inheritance from csNamedObjectVector is legal. 
 */
#define CS_BEGIN_OBJECT_VECTOR(NAME,TYPE)				\
  CS_BEGIN_TYPED_IBASE_VECTOR_WITH_SUPERCLASS (NAME, TYPE, public, csNamedObjectVector) \
    TYPE *FindByName (const char* iName) const				\
    { return (TYPE*)csNamedObjectVector::FindByName (iName); }		\

/// Finish the class definition of an object vector
#define CS_FINISH_OBJECT_VECTOR						\
  CS_FINISH_TYPED_IBASE_VECTOR

/// Declare an object vector class
#define CS_DECLARE_OBJECT_VECTOR(NAME,TYPE)				\
  CS_BEGIN_OBJECT_VECTOR (NAME, TYPE)					\
  CS_FINISH_OBJECT_VECTOR

#endif // __NOBJVEC_H__
