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
 * This header file contains a set of macros to construct 'typed vectors'.
 * They are the type-safe variant of csVector. You can perform almost any
 * operation as with csVector, but the contained objects are always
 * given as a (TYPE*) instead of a (csSome) which is actually a (void*). <p>
 *
 * There are basically two types of such vectors. The first type are used
 * to store normal C++ objects. These vectors work exactly like csVector.
 * The contained objects are given to FreeItem() when deleted and upon
 * vector destruction. Whether the items are really deleted in that function
 * depends on which macro you use. You can also define your own FreeItem
 * if you want. Also, every method that possibly calls FreeItem accepts
 * a parameter that determines whether that is actually done. <p>
 *
 * The second type are 'iBase vectors'. They are used to store SCF objects.
 * The contained objects are IncRef'ed when added and DecRef'ed when
 * removed (with the exception of the Pop() function, where it makes more
 * sense not to DecRef the contained object but instead let the user do that).
 * This has some implications on the interface: <ul>
 * <li> No parameter is accepted that determines whether removed items are
 *      actually deleted. They are always DecRef'ed.
 * <li> No reference access is allowed. csVector::Get() and operator[]
 *      return references to the contained objects which let you assign
 *      them directly. This is not possible for iBase vectors, as it
 *      circumvents proper reference-counting.
 * </ul>
 */

/**
 * Construct a typed vector class called 'NAME', storing (TYPE*) objects.
 * The vector deletes the items in FreeItem ().
 */
#define CS_DECLARE_TYPED_VECTOR(NAME,TYPE)				\
  CS_PRIVATE_DECLARE_TYPED_VECTOR (NAME, TYPE)

/**
 * Construct a typed vector class called 'NAME', storing (TYPE*) objects.
 * The vector does not do anything in FreeItem ().
 */
#define CS_DECLARE_TYPED_VECTOR_NODELETE(NAME,TYPE)			\
  CS_PRIVATE_DECLARE_TYPED_VECTOR_NODELETE (NAME, TYPE)

/**
 * Construct a typed vector class called 'NAME', storing (TYPE*) objects.
 * This macro lets you define how items are deleted. You have to define
 * the following function for this: <p>
 *
 * bool NAME::FreeTypedItem (TYPE *item);
 */
#define CS_DECLARE_TYPED_VECTOR_USERDELETE(NAME,TYPE)			\
  CS_PRIVATE_DECLARE_TYPED_VECTOR_USERDELETE (NAME, TYPE)

/**
 * 
 */

#define CS_DECLARE_TYPED_VECTOR_DECREF(NAME, TYPE) \
 CS_PRIVATE_DECLARE_TYPED_VECTOR_DECREF(NAME, TYPE)

/**
 * Implement Name::FreeTypedItem() for CS_DECLARE_TYPED_VECTOR_USERDELETE to
 * delete the item. This combination can be used instead of
 * CS_DECLARE_TYPED_VECTOR if the contained type is undefined at the time
 * you declare the vector class.
 */
#define CS_IMPLEMENT_TYPED_VECTOR_DELETE(NAME,TYPE)			\
  CS_PRIVATE_IMPLEMENT_TYPED_VECTOR_DELETE(NAME,TYPE)

/**
 * Construct a typed vector class called 'NAME', storing (TYPE*) objects.
 * The contained objects must be subclasses of iBase (this is not
 * checked by the compiler!!!)
 */
#define CS_DECLARE_TYPED_IBASE_VECTOR(NAME,TYPE)			\
  CS_PRIVATE_DECLARE_TYPED_IBASE_VECTOR (NAME, TYPE)

/**
 * Begin the class definition of a typed vector. The 'macro' parameter
 * determines which of the above macros is used, and by this the type
 * of vector to use. After this macro, add any number of user-defined
 * members and functions, then close the class definition with
 * CS_FINISH_TYPED_VECTOR. This actually creates an empty subclass of
 * a typed vector class called "NAME_Helper". This also means that by
 * default there is no constructor that takes any parameters. Note that
 * when using CS_DECLARE_TYPED_VECTOR_USERDELETE for MACRO, the function
 * to implement is called "NAME_Helper::FreeTypedItem", Not "NAME::..." <p>
 *
 * IMPORTANT: FreeTypedItem() is not virtual. If you define that function
 * between CS_BEGIN_TYPED_VECTOR() and CS_FINISH_TYPED_VECTOR(), it will
 * not be called! (more exactly: It will not override the FreeTypedItem
 * that is defined by 'MACRO')
 */
#define CS_BEGIN_TYPED_VECTOR(MACRO,NAME,TYPE)				\
  CS_PRIVATE_BEGIN_USER_VECTOR(MACRO,NAME,TYPE)

/**
 * Finish the class definition of a typed vector.
 */
#define CS_FINISH_TYPED_VECTOR						\
  CS_PRIVATE_FINISH_USER_VECTOR

/**
 * Define the default constructor in a typed vector (must be placed into
 * the class definition).
 */
#define CS_TYPED_VECTOR_CONSTRUCTOR(NAME)				\
  CS_PRIVATE_TYPED_VECTOR_CONSTRUCTOR (NAME)


//----------------------------------------------------------------------------
//--- implementation of the above macros follows -----------------------------
//----------------------------------------------------------------------------

/*
 * Helper class for vectors that contain 'iBase' objects. It assumes that
 * the contained objects may be cast to 'iBase'. Note that it does not
 * take parameters of type 'iBase'. This way it overrides the methods of
 * csVector and makes them unaccessible. Using csVector's methods
 * directly is unsafe. <p>
 * 
 * Also, this means that CS_DECLARE_IBASE_VECTOR only has to cast from and to
 * (void*), which is always possible.  Theoretically, casting from and to iBase
 * is also always possible because the contained objects *must* be derived from
 * iBase.  Hwever, at the time CS_DECLARE_IBASE_VECTOR is used, this
 * inheritance may be unknown to the compiler because the class definition of
 * the contained class did not yet appear.
 */
class csIBaseVector : public csVector
{
public:
  inline csIBaseVector (int lim, int thr) : csVector (lim, thr) {}
  inline bool Delete (int n)
  {
    return csVector::Delete (n, true); 
  }
  inline bool Delete (csSome what)
  {
    return csVector::Delete (what, true); 
  }
  inline void DeleteAll ()
  {
    csVector::DeleteAll (true);
  }
  inline int Push (csSome what)
  {
    if (what) ((iBase*)what)->IncRef ();
    return csVector::Push((csSome)what); 
  }
  inline int PushSmart (csSome what)
  {
    int n = Find (what);
    if (n != -1) return n;

    if (what) ((iBase*)what)->IncRef ();
    return csVector::Push((csSome)what); 
  }
  inline bool Insert (int n, csSome Item)
  {
    if (Item) ((iBase*)Item)->IncRef ();
    return csVector::Insert (n, (csSome)Item);
  }
  inline int InsertSorted (csSome Item, int *oEqual = NULL, int Mode = 0)
  {
    if (Item) ((iBase*)Item)->IncRef ();
    return csVector::InsertSorted ((csSome)Item, oEqual, Mode); 
  }
  inline bool Replace (int n, csSome what)
  {
    if (what) ((iBase*)what)->IncRef ();
    return csVector::Replace(n, (csSome)what, true);
  }
  virtual bool FreeItem (csSome Item)
  {
    if (Item) ((iBase *)Item)->DecRef ();
    return true;
  }
};

/*
 * This is a helper macro for typed vectors. It defines all methods that are
 * valid for usual typed vectors and typed SCF vectors. This basically
 * excludes all methods that have a 'DeleteIt' parameter for
 * usual typed vectors and methods that return 'insecure' references. <p>
 *
 * This macro assumes that the type 'superclass' is defined to the superclass
 * of the typed vector.
 */
#define CS_PRIVATE_DECLARE_TYPED_VECTOR_HELPER(NAME,TYPE)		\
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
    inline int PushSmart (TYPE *obj)					\
    { return superclass::PushSmart ((csSome)obj); }			\
    inline TYPE *Pop ()							\
    { return (TYPE *)superclass::Pop(); }				\
    inline bool Insert (int n, TYPE *Item)				\
    { return superclass::Insert (n, (csSome)Item); }			\
    inline int InsertSorted (TYPE *Item, int *oEqual = NULL, int Mode = 0) \
    { return superclass::InsertSorted ((csSome)Item, oEqual, Mode); }

/*
 * Declares a new vector type NAME as a subclass of csVector.
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
 *   CS_PRIVATE_BEGIN_TYPED_VECTOR (NAME, TYPE)
 *     user-defined FreeTypedItem() here
 *     any other user-defined methods
 *   CS_PRIVATE_FINISH_TYPED_VECTOR (TYPE)
 *
 * or (no user-defined members, contained objects are correctly deleted):
 *   CS_PRIVATE_DECLARE_TYPED_VECTOR (NAME, TYPE)
 *
 * or (no user-defined members, contained objects are not deleted):
 *   CS_PRIVATE_DECLARE_TYPED_VECTOR_NODELETE (NAME, TYPE)
 *
 * or (no user-defined members, user has to define FreeTypedItem):
 *   CS_PRIVATE_DECLARE_TYPED_VECTOR_NODELETE (NAME, TYPE)
 *
 * Parameters:
 *   NAME - Name of the new vector class.
 *   TYPE - Data type to which this vector refer.
 *          The TYPE should be possible to cast to (void *) and back.
 */
#define CS_PRIVATE_BEGIN_TYPED_VECTOR(NAME,TYPE)			\
  class NAME : private csVector						\
  {									\
    typedef csVector superclass;					\
  public:								\
    inline bool Delete (int n, bool FreeIt = true)			\
    { return superclass::Delete (n, FreeIt); }				\
    inline bool Delete (TYPE *Item, bool FreeIt = true)			\
    { return superclass::Delete ((csSome)Item, FreeIt); }		\
    inline void DeleteAll (bool FreeThem = true)			\
    { superclass::DeleteAll (FreeThem); }				\
    CS_PRIVATE_DECLARE_TYPED_VECTOR_HELPER (NAME, TYPE)			\
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

// Finish the class definition of a typed vector
#define CS_PRIVATE_FINISH_TYPED_VECTOR(TYPE)				\
    virtual bool FreeItem (csSome Item)					\
    { return FreeTypedItem ((TYPE *)Item); }				\
  }

/*
 * Declares a new vector type NAME as a subclass of csVector. Elements
 * of this vector are of type TYPE. The elements are automatically deleted
 * on either Delete() or DeleteAll() or upon vector destruction.
 */
#define CS_PRIVATE_DECLARE_TYPED_VECTOR(NAME,TYPE)			\
  CS_PRIVATE_BEGIN_TYPED_VECTOR (NAME,TYPE)				\
    inline bool FreeTypedItem (TYPE* obj)				\
    { delete obj; return true; }					\
  CS_PRIVATE_FINISH_TYPED_VECTOR (TYPE)

/**
 * Declares a new vector type NAME as a subclass of csVector. Elements of
 * this vector are of type TYPE. The elements are not deleted by this vector.
 */
#define CS_PRIVATE_DECLARE_TYPED_VECTOR_NODELETE(NAME,TYPE)		\
  CS_PRIVATE_BEGIN_TYPED_VECTOR (NAME,TYPE)				\
    inline bool FreeTypedItem (TYPE*)					\
    { return true; }							\
  CS_PRIVATE_FINISH_TYPED_VECTOR (TYPE)

/**
 * Declares a new vector type NAME as a subclass of csVector. Elements of
 * this vector are of type TYPE. The user has to define FreeTypedItem ().
 */
#define CS_PRIVATE_DECLARE_TYPED_VECTOR_USERDELETE(NAME,TYPE)		\
  CS_PRIVATE_BEGIN_TYPED_VECTOR (NAME,TYPE)				\
    bool FreeTypedItem (TYPE*);						\
  CS_PRIVATE_FINISH_TYPED_VECTOR (TYPE)

#define CS_PRIVATE_DECLARE_TYPED_VECTOR_DECREF(NAME,TYPE)  \
  CS_PRIVATE_BEGIN_TYPED_VECTOR (NAME,TYPE)                \
    inline bool FreeTypedItem (TYPE *Item)                 \
    { Item->DecRef(); Item = NULL; return true; }          \
  CS_PRIVATE_FINISH_TYPED_VECTOR (TYPE)
  
/*
 * This is a special version of typed vectors that contain SCF objects. The
 * vector will correctly IncRef all added objects and DecRef all removed
 * objects. There is only one exeption: The Pop() function does not DecRef
 * the object, so you should do that yourself. The reason is that at the time
 * you call Pop(), you do usually not have a pointer to the object, so you can
 * not IncRef() it before. <p>
 *
 * Be careful with user-defined methods in typed vectors. Though the
 * vectors are type-safe to the outside, it is still possible to access
 * csVector members (type-unsafe!) from the inside, i.e. from your own methods.
 *
 * Usage:
 *   CS_PRIVATE_DECLARE_TYPED_IBASE_VECTOR (NAME, TYPE)
 *
 * Parameters:
 *   NAME - Name of the new vector class.
 *   TYPE - Data type to which this vector refer.
 *          The TYPE should be possible to cast to (void *) and back.
 */
#define CS_PRIVATE_DECLARE_TYPED_IBASE_VECTOR(NAME,TYPE)		\
  class NAME : csIBaseVector						\
  {									\
    typedef csIBaseVector superclass;					\
  protected:								\
    inline bool FreeItem (csSome item)					\
    { return superclass::FreeItem (item); }				\
  public:								\
    inline bool Delete (int n)						\
    { return superclass::Delete (n); }					\
    inline bool Delete (TYPE *Item)					\
    { return superclass::Delete ((csSome)Item); }			\
    inline void DeleteAll ()						\
    { superclass::DeleteAll (); }					\
    CS_PRIVATE_DECLARE_TYPED_VECTOR_HELPER (NAME, TYPE)			\
    inline TYPE *operator [] (int n) const				\
    { return (TYPE *)superclass::operator [] (n); }			\
    inline TYPE *Get (int n) const					\
    { return (TYPE *)superclass::Get(n); }				\
    inline bool Replace (int n, TYPE *what)				\
    { return superclass::Replace(n, (csSome)what); }			\
  }

#define CS_PRIVATE_IMPLEMENT_TYPED_VECTOR_DELETE(NAME,TYPE)		\
  bool NAME::FreeTypedItem (TYPE *Item)					\
  { delete Item; return true; }

#define CS_PRIVATE_BEGIN_USER_VECTOR(MACRO,NAME,TYPE)			\
  MACRO (NAME##_Helper, TYPE);						\
  class NAME : public NAME##_Helper					\
  {									\
  public:

#define CS_PRIVATE_FINISH_USER_VECTOR					\
  }

#define CS_PRIVATE_TYPED_VECTOR_CONSTRUCTOR(NAME)			\
  NAME (int ilimit = 8, int ithreshold = 16) :				\
    NAME##_Helper (ilimit, ithreshold) {}

#endif // __TYPEDVEC_H__
