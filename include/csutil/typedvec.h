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
 * </ul><p>
 *
 * IMPORTANT: When you create a subclass of a vector class and override
 * FreeItem(), you must also override the destructor and call DeleteAll()
 * (or delete the items manually) in it. Don't rely on the superclass to do
 * this. Due to the way destruction works in C++, this would cause the
 * *non-derived* FreeItem() to be called from the destructor.
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
 * The vector calls DecRef on the items in FreeItem ().
 */

#define CS_DECLARE_TYPED_VECTOR_DECREF(NAME, TYPE) \
 CS_PRIVATE_DECLARE_TYPED_VECTOR_DECREF(NAME, TYPE)

/**
 * Construct a typed vector class called 'NAME', storing (TYPE*) objects.
 * The contained objects must be subclasses of iBase (this is not
 * checked by the compiler!!!). The objects are IncRef'ed when added and
 * DecRef'ed when removed. There is only one exeption: The Pop() function
 * does not DecRef the object, so you should do that yourself. The reason
 * is that at the time you call Pop(), you do usually not have a pointer
 * to the object, so you can not IncRef() it before. <p>
 */
#define CS_DECLARE_TYPED_IBASE_VECTOR(NAME,TYPE)			\
  CS_PRIVATE_DECLARE_TYPED_IBASE_VECTOR (NAME, TYPE)

/**
 * Construct a restricted-access vector class. Elements may not be assigned
 * directly (use Replace() instead). The class contains functions called
 * PrepareItem() and FreeItem(), each of which takes a void* as its parameter.
 * These functions are called when items are added or removed.
 */
#define CS_DECLARE_TYPED_RESTRICTED_ACCESS_VECTOR(NAME,TYPE)		\
  CS_PRIVATE_DECLARE_TYPED_RESTRICTED_ACCESS_VECTOR (NAME, TYPE)

//----------------------------------------------------------------------------
//--- implementation of the above macros follows -----------------------------
//----------------------------------------------------------------------------

/**
 * Subclass of csVector that restricts access to the contained objects. The
 * contents of the vector may not be assigned directly anymore. For every
 * added element, the PrepareItem() function is called. For every
 * removed element, the FreeItem() function is called. <p>
 *
 * All three functions may return false to abort the push/pop/remove operation.
 */
class csRestrictedAccessVector : public csVector
{
public:
  virtual bool PrepareItem (csSome )
  { return true; }
  virtual bool FreeItem (csSome )
  { return true; }

  inline csRestrictedAccessVector (int lim, int thr) : csVector (lim, thr) {}
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
    if (!PrepareItem (what)) return -1;
    return csVector::Push(what); 
  }
  inline int PushSmart (csSome what)
  {
    int n = Find (what);
    if (n != -1) return n;

    if (!PrepareItem (what)) return -1;
    return csVector::Push(what); 
  }
  inline bool Insert (int n, csSome Item)
  {
    if (!PrepareItem (Item)) return false;
    return csVector::Insert (n, Item);
  }
  inline int InsertSorted (csSome Item, int *oEqual = NULL, int Mode = 0)
  {
    if (!PrepareItem (Item)) return -1;
    return csVector::InsertSorted (Item, oEqual, Mode); 
  }
  inline bool Replace (int n, csSome what)
  {
    if (!PrepareItem (what)) return false;
    return csVector::Replace (n, what, true);
  }
  inline csSome Pop ()
  {
    if (FreeItem (Top ()))
      return csVector::Pop ();
    else
      return NULL;
  }
};

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
 * iBase. However, at the time CS_DECLARE_IBASE_VECTOR is used, this
 * inheritance may be unknown to the compiler because the class definition of
 * the contained class did not yet appear. <p>
 *
 * iBase vectors handle the Pop() method specially in the way that they do not
 * DecRef items removed with Pop().
 */
class csIBaseVector : public csRestrictedAccessVector
{
public:
  inline csIBaseVector (int lim, int thr) : csRestrictedAccessVector (lim, thr) {}

  virtual bool PrepareItem (csSome Item)
  {
    ((iBase*)Item)->IncRef ();
    return true;
  }
  virtual bool FreeItem (csSome Item)
  {
    ((iBase*)Item)->DecRef ();
    return true;
  }
  inline csSome Pop ()
  {
    // Items that are removed with Pop() should not be DecRef'ed. To keep
    // the code simple, we just IncRef them before.
    csSome item = Top ();
    ((iBase*)item)->IncRef ();

    if (FreeItem (item)) {
      // We also have to bypass csRestrictedAccessVector::Pop ().
      return csVector::Pop ();
    } else {
      // Removal failed, so we have to release our reference again.
      ((iBase*)item)->DecRef ();
      return NULL;
    }
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
    inline TYPE *Top ()	const						\
    { return (TYPE *)superclass::Top(); }				\
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

/**
 * This macro can be used to define restricted-access vectors. The vector
 * class inherits from 'sclass'. See csRestrictedAccessVector for
 * details.
 */
#define CS_PRIVATE_DECLARE_TYPED_RESTR_ACC_VECTOR(NAME,TYPE,sclass)	\
  class NAME : private sclass						\
  {									\
  protected:								\
    typedef sclass superclass;						\
    virtual bool PrepareItem (csSome item)				\
    { return superclass::PrepareItem (item); }				\
    virtual bool FreeItem (csSome item)					\
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

#define CS_PRIVATE_DECLARE_TYPED_RESTRICTED_ACCESS_VECTOR(NAME,TYPE)	\
  CS_PRIVATE_DECLARE_TYPED_RESTR_ACC_VECTOR (NAME, TYPE, csRestrictedAccessVector)

#define CS_PRIVATE_DECLARE_TYPED_IBASE_VECTOR(NAME,TYPE)		\
  CS_PRIVATE_DECLARE_TYPED_RESTR_ACC_VECTOR (NAME, TYPE, csIBaseVector)
  
/*
 * This is a special version of typed vectors that contain SCF objects. The
 * vector will correctly IncRef all added objects and DecRef all removed
 * objects. There is only one exeption: The Pop() function does not DecRef
 * the object, so you should do that yourself. The reason is that at the time
 * you call Pop(), you do usually not have a pointer to the object, so you can
 * not IncRef() it before. <p>
 *
 * Usage:
 *   CS_PRIVATE_DECLARE_TYPED_RESTR_ACC_VECTOR (superclass, NAME, TYPE)
 *
 * Parameters:
 *   superclass - The parent class. May be either csIBaseVector or
 *                csRestrictedAccessVector.
 *   NAME - Name of the new vector class.
 *   TYPE - Data type to which this vector refer.
 *          The TYPE should be possible to cast to (void *) and back.
 */

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
