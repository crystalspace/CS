/*
  Crystal Space Smart Pointers
  Copyright (C) 2002 by Jorrit Tyberghein and Matthias Braun

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

#ifndef __CS_REF_H__
#define __CS_REF_H__

/**\file
 * Smart Pointers
 */

#include "csextern.h"

#define CS_VOIDED_PTR 0xffffffff

template <class T> class csRef;

#if defined(CS_DEBUG)
#  define CS_TEST_VOIDPTRUSAGE
#else
#  undef CS_TEST_VOIDPTRUSAGE
#endif

#ifdef CS_REF_TRACKER
 #include <typeinfo>
 #include "csutil/reftrackeraccess.h"

 #define CSREF_TRACK(x, cmd, refCount, obj, tag)    \
  {						    \
    const int rc = obj ? refCount : -1;		    \
    if (obj) cmd;				    \
    if (obj)					    \
    {						    \
      csRefTrackerAccess::SetDescription (obj,	    \
	typeid(T).name());			    \
      csRefTrackerAccess::Match ## x (obj, rc, tag);\
    }						    \
  }
 #define CSREF_TRACK_INCREF(obj,tag)	\
  CSREF_TRACK(IncRef, obj->IncRef(), obj->GetRefCount(), obj, tag);
 #define CSREF_TRACK_DECREF(obj,tag)	\
  CSREF_TRACK(DecRef, obj->DecRef(), obj->GetRefCount(), obj, tag);
 #define CSREF_TRACK_ASSIGN(obj,tag)	\
  CSREF_TRACK(IncRef, (0), obj->GetRefCount() - 1, obj, tag);
#else
 #define CSREF_TRACK_INCREF(obj,tag) \
  if (obj) obj->IncRef();
 #define CSREF_TRACK_DECREF(obj,tag) \
  if (obj) obj->DecRef();
 #define CSREF_TRACK_ASSIGN(obj,tag)
#endif

/**
 * A normal pointer. This class should ONLY be used for functions
 * returning pointers that are already IncRef()'ed for the caller.
 * This class behaves like a pointer encapsulator. Please do NOT
 * use this for anything else (for example, don't use this class
 * to remember pointers to anything. Use csRef for that).
 * It only stores the pointer. Nothing else. When it is assigned to
 * a csRef, the csRef smart pointer will 'inherit' the reference
 * (so no IncRef() happens).
 */
template <class T>
class  csPtr
{
private:
  friend class csRef<T>;
  T* obj;

public:
  csPtr (T* p) : obj (p) { CSREF_TRACK_ASSIGN(obj, this); }

  template <class T2>
  explicit csPtr (csRef<T2> const& r) : obj((T2*)r) 
  { 
    CSREF_TRACK_INCREF (obj, this);
  }

#ifdef CS_TEST_VOIDPTRUSAGE
  ~csPtr ()
  {
    // If not assigned to a csRef we have a problem (leak).
    // So if this assert fires for you, then you are calling
    // a function that returns a csPtr and not using the result
    // (or at least not assigning it to a csRef). This is a memory
    // leak and you should fix that.
    CS_ASSERT_MSG ("csPtr<> was not assigned to a csRef<> prior destruction", 
      obj == (T*)CS_VOIDED_PTR);
  }
#endif

  csPtr (const csPtr<T>& copy)
  {
    obj = copy.obj;
#ifdef CS_TEST_VOIDPTRUSAGE
    ((csPtr<T>&)copy).obj = (T*)CS_VOIDED_PTR;
#endif
  }
};

/**
 * A smart pointer.  Maintains and correctly manages a reference to a
 * reference-counted object.  This template requires only that the object type
 * T implement the methods IncRef() and DecRef().  No other requirements are
 * placed upon T.
 */
template <class T>
class  csRef
{
private:
  T* obj;

public:
  /**
   * Construct an invalid smart pointer (that is, one pointing at nothing).
   * Dereferencing or attempting to use the invalid pointer will result in a
   * run-time error, however it is safe to invoke IsValid().
   */
  csRef () : obj (0) {}

  /**
   * Construct a smart pointer from a csPtr. Doesn't call IncRef() on
   * the object since it is assumed that the object in csPtr is already
   * IncRef()'ed.
   */
  csRef (const csPtr<T>& newobj)
  {
    obj = newobj.obj;
#   ifdef CS_TEST_VOIDPTRUSAGE
    CS_ASSERT_MSG ("csPtr<> was already assigned to a csRef<>",
      newobj.obj != (T*)CS_VOIDED_PTR);
#   endif
    // The following line is outside the ifdef to make sure
    // we have binary compatibility.
    ((csPtr<T>&)newobj).obj = (T*)CS_VOIDED_PTR;
  }

  /**
   * Construct a smart pointer from a raw object reference. Calls IncRef()
   * on the object.
   */
  csRef (T* newobj) : obj (newobj)
  {
    CSREF_TRACK_INCREF (obj, this);
  }
  
  /**
   * Smart pointer copy constructor from assignment-compatible csRef<T2>.
   */
  template <class T2>
  csRef (csRef<T2> const& other) : obj ((T2*)other)
  {
    CSREF_TRACK_INCREF (obj, this);
  }

  /**
   * Smart pointer copy constructor.
   */
  csRef (csRef const& other) : obj (other.obj)
  {
    CSREF_TRACK_INCREF (obj, this);
  }

  /**
   * Smart pointer destructor.  Invokes DecRef() upon the underlying object.
   */
  ~csRef ()
  {
    CSREF_TRACK_DECREF (obj, this);
  }

  /**
   * Assign a csPtr to a smart pointer. Doesn't call IncRef() on
   * the object since it is assumed that the object in csPtr is already
   * IncRef()'ed.
   * \remarks
   * After this assignment, the csPtr<T> object is invalidated and cannot
   * be used. You should not (and in fact cannot) decref the csPtr<T> after
   * this assignment has been made.
   */
  csRef& operator = (const csPtr<T>& newobj)
  {
    T* oldobj = obj;
    // First assign and then DecRef() of old object!
    obj = newobj.obj;
#   ifdef CS_TEST_VOIDPTRUSAGE
    CS_ASSERT_MSG ("csPtr<> was already assigned to a csRef<>",
      newobj.obj != (T*)CS_VOIDED_PTR);
#   endif
    // The following line is outside the ifdef to make sure
    // we have binary compatibility.
    ((csPtr<T>&)newobj).obj = (T*)CS_VOIDED_PTR;
    CSREF_TRACK_DECREF (oldobj, this);
    return *this;
  }

  /**
   * Assign a raw object reference to this smart pointer.
   * \remarks
   * This function calls the object's IncRef() method. Because of this you
   * should not assign a reference created with the new operator to a csRef
   * object driectly. The following code will produce a memory leak:
   * \code
   * csRef<iEvent> event = new csEvent;
   * \endcode
   * If you are assigning a new object to a csRef, use AttachNew(T* newObj)
   * instead.
   */
  csRef& operator = (T* newobj)
  {
    if (obj != newobj)
    {
      T* oldobj = obj;
      // It is very important to first assign the new value to
      // 'obj' BEFORE calling DecRef() on the old object. Otherwise
      // it is easy to get in infinite loops with objects being
      // destructed forever (when ref=0 is used for example).
      obj = newobj;
      CSREF_TRACK_INCREF (newobj, this);
      CSREF_TRACK_DECREF (oldobj, this);
    }
    return *this;
  }

  /**
   * Assign an object reference created with the new operator to this smart
   * pointer.
   * \remarks
   * This function allows you to assign an object pointer created with the
   * \c new operator to the csRef object. Proper usage would be:
   * \code
   * csRef<iEvent> event;
   * event.AttachNew (new csEvent);
   * \endcode
   * While not recommended, you can also use this function to assign a csPtr
   * object or csRef object to the csRef. In both of these cases, using
   * AttachNew is equivalent to performing a simple assignment using the
   * \c = operator.
   * \note
   * Calling this function is equivalent to casting an object to a csPtr<T>
   * and then assigning the csPtr<T> to the csRef, as follows:
   * \code
   * // Same effect as above code.
   * csRef<iEvent> event = csPtr<iEvent> (new csEvent);
   * \endcode
   */
  void AttachNew (csPtr<T> newObj)
  {
    // Note: The parameter usage of csPtr<T> instead of csPtr<T>& is
    // deliberate and not to be considered a bug.

    // Just Re-use csPtr assignment logic
    *this = newObj;
  }

  /// Assign another assignment-compatible csRef<T2> to this one.
  template <class T2>
  csRef& operator = (csRef<T2> const& other)
  {
    T* p = (T2*)other;
    this->operator=(p);
    return *this;
  }

  /// Assign another csRef<> of the same type to this one.
  csRef& operator = (csRef const& other)
  {
    this->operator=(other.obj);
    return *this;
  }

  /// Test if the two references point to same object.
  inline friend bool operator == (const csRef& r1, const csRef& r2)
  {
    return r1.obj == r2.obj;
  }
  /// Test if the two references point to different object.
  inline friend bool operator != (const csRef& r1, const csRef& r2)
  {
    return r1.obj != r2.obj;
  }
  /// Test if object pointed to by reference is same as obj.
  inline friend bool operator == (const csRef& r1, T* obj)
  {
    return r1.obj == obj;
  }
  /// Test if object pointed to by reference is different from obj.
  inline friend bool operator != (const csRef& r1, T* obj)
  {
    return r1.obj != obj;
  }
  /// Test if object pointed to by reference is same as obj.
  inline friend bool operator == (T* obj, const csRef& r1)
  {
    return r1.obj == obj;
  }
  /// Test if object pointed to by reference is different from obj.
  inline friend bool operator != (T* obj, const csRef& r1)
  {
    return r1.obj != obj;
  }
  /**
   * Test the relationship of the addresses of two objects.
   * \remarks Mainly useful when csRef<> is used as the subject of
   *   csComparator<>, which employs operator< for comparisons.
   */
  inline friend bool operator < (const csRef& r1, const csRef& r2)
  {
    return r1.obj < r2.obj;
  }


  /// Dereference underlying object.
  T* operator -> () const
  { return obj; }
  
  /// Cast smart pointer to a pointer to the underlying object.
  operator T* () const
  { return obj; }
  
  /// Dereference underlying object.
  T& operator* () const
  { return *obj; }

  /**
   * Smart pointer validity check.  Returns true if smart pointer is pointing
   * at an actual object, otherwise returns false.
   */
  bool IsValid () const
  { return (obj != 0); }

  /// Invalidate the smart pointer by setting it to null.
  void Invalidate()
  { *this = (T*)0; }

};

#undef CSREF_TRACK_INCREF
#undef CSREF_TRACK_DECREF
#undef CSREF_TRACK_ASSIGN

#endif // __CS_REF_H__
