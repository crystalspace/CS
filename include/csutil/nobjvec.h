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
 * to cast the contained objects from and to (iBase*), and that they implement
 * the iObject interface. <p>
 *
 * Object vectors also have the following special methods: <ul>
 * <li> GetObjectVector () : Return the vector as a csNamedObjectVector.
 *      The returned object is used to deal with the vector as a vector
 *      of iObject's. However, as they are not really iObject's, it is not
 *      possible to add objects to the vector, due to type-unsafety.
 * <li> GetIndexByName () : Return the index of an object by searching
 *      for its name.
 * <li> FindByName () : Return the pointer to an object by searching
 *      for its name.
 * <li> FindObjectByName () : Like FindByName(), but returns an (iObject*)
 *      pointer.
 * </ul>
 */

/**
 * This class is intended as a wrapper around an existing csVector. It assumes
 * that the vector contains iBase objects which implement the iObject
 * interface. This class allows access to the vector as if it was a vector
 * of iObject's. However, it is not possible to add objects to the vector
 * though this interface, because this interface does not know which type of
 * objects is actually contained.
 *
 * @@@ the current implementation uses SCF_QUERY_INTERFACE everywhere. This
 * is slow and also the refcount is not handled correctly.
 */
class csNamedObjectVector
{
private:
  // the wrapped vector
  csVector *Vector;

public:
  /// constructor
  inline csNamedObjectVector ();

  /// Return the index of the element with the given name, or -1.
  int GetIndexByName (const char *name) const;
  /// Find an object by name
  iObject *FindByName (const char *name) const;

  /// Set the length of this vector
  inline void SetLength (int n);
  /// Return the length of this vector
  inline int Length () const;
  /// Return the limit of this vector
  inline int Limit () const;
  /// Exchange two elements of the vector
  inline void Exchange (int n1, int n2);
  /// Quick-sort the elements, using the given mode
  inline void QuickSort (int Left, int Right, int Mode = 0);
  /// Quick-sort the elements, using the given mode
  inline void QuickSort (int Mode = 0);
  /// Find an element and return its index
  int Find (iObject *obj) const;
  /// Find an element using the given key
  inline int FindKey (csConstSome Key, int Mode = 0) const;
  /// Find an element using the given key. Expects the elements to be sorted.
  inline int FindSortedKey (csConstSome Key, int Mode = 0) const;
  /// Pop an element from the vector.
  inline iObject *Pop ();
  /// Delete an element from the vector
  inline bool Delete (int n);
  /// Delete an element from the vector
  bool Delete (iObject *obj);
  /// Delete all elements
  inline void DeleteAll ();
  /// Return the pointer to an element
  inline iObject *Get (int n) const;
  /// Return the pointer to an element
  inline iObject *operator[] (int n) const;

  // private method: Compare two objects by their names
  static int Compare (csSome Item1, csSome Item2, int Mode);
  // private method: Compare object's name with a string
  static int CompareKey (csSome Item, csConstSome Key, int Mode);
  // private method: set the wrapped vector
  inline void SetVector (void *vec);
};

/**
 * Declare an object vector class which leaves the reference count of the
 * contained objects untouched.
 */
#define CS_DECLARE_OBJECT_VECTOR_NOREF(NAME,TYPE)			\
  CS_PRIVATE_DECLARE_OBJECT_VECTOR_NOREF (NAME, TYPE)

/**
 * Declare an object vector class which contains overridable PrepareItem(),
 * FreeItem() and PopItem() functions. The vector still leaves the RefCount
 * of the contained objects untouched.
 */
#define CS_DECLARE_OBJECT_VECTOR(NAME,TYPE)				\
  CS_PRIVATE_DECLARE_OBJECT_VECTOR (NAME, TYPE)

/**
 * Declare an object vector class which handles the reference count of the
 * contained object correctly and also contains overridable PrepareItem(),
 * FreeItem() and PopItem() functions.
 */
#define CS_DECLARE_RESTRICED_ACCESS_OBJECT_VECTOR(NAME,TYPE)		\
  CS_PRIVATE_DECLARE_RESTRICED_ACCESS_OBJECT_VECTOR (NAME, TYPE)

//----------------------------------------------------------------------------
//--- implementation of the above macros follows -----------------------------
//----------------------------------------------------------------------------

#define CS_PRIVATE_DECLARE_OBJECT_VECTOR(NAME,TYPE)			\
  CS_PRIVATE_DECLARE_OBJECT_VECTOR_COMMON (				\
    CS_DECLARE_TYPED_IBASE_VECTOR, NAME, TYPE)

#define CS_PRIVATE_DECLARE_OBJECT_VECTOR_NOREF(NAME,TYPE)		\
  CS_PRIVATE_DECLARE_OBJECT_VECTOR_COMMON (				\
    CS_DECLARE_TYPED_VECTOR_NODELETE, NAME, TYPE)

#define CS_PRIVATE_DECLARE_RESTRICTED_ACCESS_OBJECT_VECTOR(NAME,TYPE)	\
  CS_PRIVATE_DECLARE_OBJECT_VECTOR_COMMON (				\
    CS_DECLARE_TYPED_RESTRICTED_ACCESS_VECTOR, NAME, TYPE)

#define CS_PRIVATE_DECLARE_OBJECT_VECTOR_COMMON(MACRO,NAME,TYPE)	\
  CS_BEGIN_TYPED_VECTOR (MACRO, NAME, TYPE)				\
  private:								\
    csNamedObjectVector ObjVec;						\
  public:								\
    NAME (int ilimit = 16, int ithreshold = 16) :			\
      NAME##_Helper (ilimit, ithreshold)				\
      {ObjVec.SetVector (this);}					\
    virtual ~NAME ()							\
      { DeleteAll (); }							\
    inline csNamedObjectVector *GetObjectVector ()			\
      { return &ObjVec; }						\
    inline const csNamedObjectVector *GetObjectVector () const		\
      { return &ObjVec; }						\
    int GetIndexByName (const char* iName) const			\
      { return ObjVec.GetIndexByName (iName); }				\
    TYPE *FindByName (const char* iName) const				\
      { int n = GetIndexByName (iName);					\
        return (n==-1) ? 0 : Get(n); }					\
    iObject *FindObjectByName (const char* iName) const			\
      { return ObjVec.FindByName (iName); }				\
    virtual int Compare (csSome Item1, csSome Item2, int Mode) const	\
      { return csNamedObjectVector::Compare (Item1, Item2, Mode); }	\
    virtual int CompareKey (csSome Item, csConstSome Key, int Mode) const \
      { return csNamedObjectVector::CompareKey (Item, Key, Mode); }	\
  CS_FINISH_TYPED_VECTOR

// implementation of inline functions follows
inline csNamedObjectVector::csNamedObjectVector ()
  { Vector = 0; }
inline void csNamedObjectVector::SetVector (void *v)
  { Vector = (csVector*)v; }
inline void csNamedObjectVector::SetLength (int n)
  { Vector->SetLength(n); }
inline int csNamedObjectVector::Length () const
  { return Vector->Length (); }
inline int csNamedObjectVector::Limit () const
  { return Vector->Limit (); }
inline void csNamedObjectVector::Exchange (int n1, int n2)
  { Vector->Exchange (n1, n2); }
inline void csNamedObjectVector::QuickSort (int Left, int Right, int Mode)
  { Vector->QuickSort (Left, Right, Mode); }
inline void csNamedObjectVector::QuickSort (int Mode)
  { Vector->QuickSort (Mode); }
inline int csNamedObjectVector::FindKey (csConstSome Key, int Mode) const
  { return Vector->FindKey (Key, Mode); }
inline int csNamedObjectVector::FindSortedKey (csConstSome Key, int Mode) const
  { return Vector->FindSortedKey (Key, Mode); }
inline iObject *csNamedObjectVector::Pop ()
  {
    iBase *objbase = (iBase*)Vector->Pop ();
    if (!objbase) return 0;
    iObject *obj = SCF_QUERY_INTERFACE_FAST (objbase, iObject);
    CS_ASSERT (obj);
    obj->DecRef ();
    return obj;
  }
inline bool csNamedObjectVector::Delete (int n)
  { return Vector->Delete (n); }
inline void csNamedObjectVector::DeleteAll ()
  { Vector->DeleteAll (); }
inline iObject *csNamedObjectVector::Get (int n) const
  { 
    iBase *objbase = (iBase*)Vector->Get (n);
    iObject *o = objbase ? SCF_QUERY_INTERFACE_FAST (objbase, iObject) : 0;
    if (o) o->DecRef ();
    return o;
  }
inline iObject *csNamedObjectVector::operator[] (int n) const
  { 
    iBase *objbase = (iBase*)Vector->Get (n);
    iObject *o = objbase ? SCF_QUERY_INTERFACE_FAST (objbase, iObject) : 0;
    if (o) o->DecRef ();
    return o;
  }

#endif // __NOBJVEC_H__
