/*
    Crystal Space utility library: type-safe extension of data vectors
    Copyright (C) 2000 by Martin Geisse (mgeisse@gmx.net)

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

#ifndef __TYPEDVEC_H__
#define __TYPEDVEC_H__

#include "csutil/scf.h"
#include "csutil/csvector.h"

/**
 * Helper class for vectors that contain 'iBase' objects. It assumes that
 * the contained objects may be cast to 'iBase'. Note that it does not
 * take parameters of type 'iBase'. This way it overrides the methods of
 * csVector and makes them unaccessible. Using csVector's methods
 * directly is unsafe. <p>
 * 
 * Also, this means that CS_DECLARE_IBASE_VECTOR only has to cast from and to
 * (void*), which is always possible. Theoretically, casting from and to iBase
 * is also always possible because the contained objects *must* be
 * derived from iBase. Hwever, at the time CS_DECLARE_IBASE_VECTOR is used, this
 * inheritance may be unknown to the compiler because the class definition
 * of the contained class did not yet appear.
 */
class csIBaseVector : public csVector
{
protected:
  void TestInheritance () {}
public:
  inline csIBaseVector (int lim, int thr) : csVector (lim, thr) {}
  inline bool Delete (int n)
  {
    return csVector::Delete (n, true); 
  }
  inline void DeleteAll ()
  {
    csVector::DeleteAll (true);
  }
  inline int Push (csSome what)
  {
    ((iBase*)what)->IncRef ();
    return csVector::Push((csSome)what); 
  }
  inline bool Insert (int n, csSome Item)
  {
    ((iBase*)Item)->IncRef ();
    return csVector::Insert (n, (csSome)Item);
  }
  inline int InsertSorted (csSome Item, int *oEqual = NULL, int Mode = 0)
  {
    ((iBase*)Item)->IncRef ();
    return csVector::InsertSorted ((csSome)Item, oEqual, Mode); 
  }
  inline bool Replace (int n, csSome what)
  {
    ((iBase*)what)->IncRef ();
    return csVector::Replace(n, (csSome)what, true);
  }
  virtual bool FreeItem (csSome Item)
  {
    ((iBase *)Item)->DecRef ();
    return true;
  }
};

/**
 * This is a helper macro for typed vectors. It defines all methods that are
 * valid for usual typed vectors and typed SCF vectors. This basically
 * excludes all methods that have a 'DeleteIt' parameter for
 * usual typed vectors and methods that return 'insecure' references. <p>
 *
 * This macro assumes that the type 'superclass' is defined to the superclass
 * of the typed vector.
 */
#define CS_DECLARE_TYPED_VECTOR_HELPER(NAME,TYPE)			\
    inline NAME (int ilimit = 16, int ithreshold = 16) :		\
      superclass (ilimit, ithreshold) {}				\
    virtual ~NAME ()							\
    { DeleteAll (); }							\
    inline void SetLength (int n)					\
    { superclass::SetLength(n); }					\
    inline int Length () const						\
    { return count; }							\
    inline int Limit () const						\
    { return limit; }							\
    inline void Exchange (int n1, int n2)				\
    { superclass::Exchange (n1, n2); }					\
    inline void QuickSort (int Left, int Right, int Mode = 0)		\
    { superclass::QuickSort (Left, Right, Mode); }			\
    inline void QuickSort (int Mode = 0)				\
    { superclass::QuickSort (Mode); }					\
    inline int Find (TYPE *which) const					\
    { return superclass::Find ((csSome)which); }			\
    inline int FindKey (csConstSome Key, int Mode = 0) const		\
    { return superclass::FindKey (Key, Mode); }				\
    inline int FindSortedKey (csConstSome Key, int Mode = 0) const	\
    { return superclass::FindSortedKey (Key, Mode); }			\
    inline int Push (TYPE *obj)						\
    { return superclass::Push ((csSome)obj); }				\
    inline TYPE *Pop ()							\
    { return (TYPE *)superclass::Pop(); }				\
    inline bool Insert (int n, TYPE *Item)				\
    { return superclass::Insert (n, (csSome)Item); }			\
    inline int InsertSorted (TYPE *Item, int *oEqual = NULL, int Mode = 0) \
    { return superclass::InsertSorted ((csSome)Item, oEqual, Mode); }

/**
 * Declares a new vector type NAME as a subclass of a given superclass.
 * Elements of this vector are of type TYPE. The elements are
 * automatically given to FreeTypedItem() on either Delete() or
 * DeleteAll() or upon vector destruction. However, you must define
 * FreeTypedItem() yourself. <p>
 *
 * Be careful with user-defined methods in typed vectors. Though the
 * vectors are type-safe to the outside, it is still possible to access
 * csVector members (type-unsafe!) from the inside, i.e. from your own methods.
 *
 * Usage (all features):
 *   CS_BEGIN_TYPED_VECTOR_WITH_SUPERCLASS (NAME, TYPE, INHERIT, SUPERCLASS)
 *     user-defined FreeTypedItem() here
 *     any other user-defined methods
 *   CS_FINISH_TYPED_VECTOR (TYPE)
 *
 * or (no user-defined superclass):
 *   CS_BEGIN_TYPED_VECTOR (NAME, TYPE)
 *     user-defined FreeTypedItem() here
 *     any other user-defined methods
 *   CS_FINISH_TYPED_VECTOR (TYPE)
 *
 * or (no user-defined members, contained objects are correctly deleted):
 *   CS_DECLARE_TYPED_VECTOR (NAME, TYPE)
 *
 * or (no user-defined members, contained objects are not deleted):
 *   CS_DECLARE_TYPED_VECTOR_NODELETE (NAME, TYPE)
 *
 * Parameters:
 *   NAME - Name of the new vector class.
 *   TYPE - Data type to which this vector refer.
 *          The TYPE should be possible to cast to (void *) and back.
 *   INHERIT - type of inheritance, usually 'private'.
 *   SUPERCLASS - The superclass of the vector. Usually csVector (in
 *          which case you can use CS_BEGIN_TYPED_VECTOR for convenience).
 */
#define CS_BEGIN_TYPED_VECTOR_WITH_SUPERCLASS(NAME,TYPE,INHERIT,SCLASS)	\
  class NAME : INHERIT SCLASS						\
  {									\
    typedef SCLASS superclass;						\
  public:								\
    inline bool Delete (int n, bool FreeIt = true)			\
    { return superclass::Delete (n, FreeIt); }				\
    inline void DeleteAll (bool FreeThem = true)			\
    { superclass::DeleteAll (FreeThem); }				\
    CS_DECLARE_TYPED_VECTOR_HELPER (NAME, TYPE)				\
    inline TYPE *& operator [] (int n)					\
    { return (TYPE *&)superclass::operator [] (n); }			\
    inline TYPE *& operator [] (int n) const				\
    { return (TYPE *&)superclass::operator [] (n); }			\
    inline TYPE *& Get (int n) const					\
    { return (TYPE *&)superclass::Get(n); }				\
    inline TYPE **GetArray ()						\
    { return (TYPE**)root; }						\
    inline bool Replace (int n, TYPE *what, bool FreePrevious = true)	\
    { return superclass::Replace(n, (csSome)what, FreePrevious); }

/// Begin the class definition of a typed vector
#define CS_BEGIN_TYPED_VECTOR(NAME,TYPE)				\
  CS_BEGIN_TYPED_VECTOR_WITH_SUPERCLASS (NAME, TYPE, private, csVector)

/// Finish the class definition of a typed vector
#define CS_FINISH_TYPED_VECTOR(TYPE)					\
    virtual bool FreeItem (csSome Item)					\
    { return FreeTypedItem ((TYPE *)Item); }				\
  }

/**
 * Declares a new vector type NAME as a subclass of csVector. Elements
 * of this vector are of type TYPE. The elements are automatically deleted
 * on either Delete() or DeleteAll() or upon vector destruction.
 */
#define CS_DECLARE_TYPED_VECTOR(NAME,TYPE)				\
  CS_BEGIN_TYPED_VECTOR (NAME,TYPE)					\
    inline bool FreeTypedItem (TYPE* obj)				\
    { delete obj; return true; }					\
  CS_FINISH_TYPED_VECTOR (TYPE)

/**
 * Declares a new vector type NAME as a subclass of csVector. Elements of
 * this vector are of type TYPE. The elements are not deleted by this vector.
 */
#define CS_DECLARE_TYPED_VECTOR_NODELETE(NAME,TYPE)			\
  CS_BEGIN_TYPED_VECTOR (NAME,TYPE)					\
    inline bool FreeTypedItem (TYPE* obj)				\
    { return true; }							\
  CS_FINISH_TYPED_VECTOR (TYPE)

/**
 * This is a special version of typed vectors that contain SCF objects. The
 * vector will correctly IncRef all added objects and DecRef all removed
 * objects. There is only one exeption: The Pop() function does not DecRef
 * the object, so you should do that yourself. The reason is that at the time
 * you call Pop(), you do usually not have a pointer to the object, so you can
 * not IncRef() it before. <p>
 *
 * Note that through some way the typed vector must be derived from
 * csIBaseVector, i.e. the given superclass must be csIBaseVector itself
 * or a descendant of it. This is required for correct reference counting. <p>
 *
 * Be careful with user-defined methods in typed vectors. Though the
 * vectors are type-safe to the outside, it is still possible to access
 * csVector members (type-unsafe!) from the inside, i.e. from your own methods.
 *
 * Usage (all features):
 *   CS_BEGIN_TYPED_IBASE_VECTOR_WITH_SUPERCLASS (NAME, TYPE, INHERIT, SUPERCLASS)
 *     any user-defined methods
 *   CS_FINISH_TYPED_IBASE_VECTOR
 *
 * or (no user-defined superclass):
 *   CS_BEGIN_TYPED_IBASE_VECTOR (NAME, TYPE)
 *     any other user-defined methods
 *   CS_FINISH_TYPED_IBASE_VECTOR
 *
 * or (no user-defined members):
 *   CS_DECLARE_TYPED_IBASE_VECTOR (NAME, TYPE)
 *
 * Parameters:
 *   NAME - Name of the new vector class.
 *   TYPE - Data type to which this vector refer.
 *          The TYPE should be possible to cast to (void *) and back.
 *   INHERIT - type of inheritance, usually 'private'.
 *   SUPERCLASS - The superclass of the vector. Usually csIBaseVector (in
 *          which case you can use CS_BEGIN_TYPED_IBASE_VECTOR for
 *          convenience). Note that if you use another superclass, then
 *          it must be a descendant of csIBaseVector.
 */
#define CS_BEGIN_TYPED_IBASE_VECTOR_WITH_SUPERCLASS(NAME,TYPE,INHERIT,SCLASS)	\
  class NAME : INHERIT SCLASS						\
  {									\
    typedef SCLASS superclass;						\
    /* This function has no other purpose than to assure inheritance */	\
    /* from csIBaseVector.                                           */	\
    inline void TestIBaseVectorInhertiance ()				\
    { csIBaseVector::TestInheritance (); }				\
  public:								\
    inline bool Delete (int n)						\
    { return superclass::Delete (n); }					\
    inline void DeleteAll ()						\
    { superclass::DeleteAll (); }					\
    CS_DECLARE_TYPED_VECTOR_HELPER (NAME, TYPE)				\
    inline TYPE *operator [] (int n) const				\
    { return (TYPE *)superclass::operator [] (n); }			\
    inline TYPE *Get (int n) const					\
    { return (TYPE *)superclass::Get(n); }				\
    inline bool Replace (int n, TYPE *what)				\
    { return superclass::Replace(n, (csSome)what); }

/// Begin the class definition of a typed vector for SCF objects
#define CS_BEGIN_TYPED_IBASE_VECTOR(NAME,TYPE)				\
  CS_BEGIN_TYPED_IBASE_VECTOR_WITH_SUPERCLASS(NAME,TYPE,private,csIBaseVector)

/// Finish the class definition of a typed vector for SCF objects
#define CS_FINISH_TYPED_IBASE_VECTOR					\
  }

/// Define a vector for SCF objects
#define CS_DECLARE_TYPED_IBASE_VECTOR(NAME,TYPE)			\
  CS_BEGIN_TYPED_IBASE_VECTOR (NAME, TYPE)				\
  CS_FINISH_TYPED_IBASE_VECTOR

#endif // __TYPEDVEC_H__
