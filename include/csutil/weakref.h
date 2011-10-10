/*
  Crystal Space Weak Reference
  Copyright (C) 2003 by Jorrit Tyberghein and Matthias Braun

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

#ifndef __CS_WEAKREF_H__
#define __CS_WEAKREF_H__

/**\file
 * Weak Reference
 */

#include "csextern.h"
#include "csutil/ref.h"
#include "csutil/threading/mutex.h"

struct iBase;

/* Define this macro if you want to get a warning when a weak reference is
 * used in a way that is unsafe with multiple threads. */
#ifdef CS_WEAKREF_WARN_MULTITHREADING_UNSAFE
#define CS_WEAKREF_METHOD_UNSAFE	\
  CS_DEPRECATED_METHOD_MSG("UNSAFE in multithreading scenarios - use Get<csRef<> >()")
#else
#define CS_WEAKREF_METHOD_UNSAFE
#endif

/**
 * A weak reference. This is a reference to a reference counted object
 * but in itself it doesn't increment the ref counter. As soon as the
 * object is destroyed (i.e. the last REAL reference to it is removed)
 * all weak references pointing to that object are cleared. This kind of
 * reference is useful if you want to maintain some kind of cached objects
 * that can safely be removed as soon as the last reference to it is gone.
 * 
 * Note: this class assumes that the T type implements at least the following
 * functions:
 * - AddRefOwner()
 * - RemoveRefOwner()
 *
 * \remarks An extended explanation on smart pointers - how they work and what
 *   type to use in what scenario - is contained in the User's manual, 
 *   section "Correctly Using Smart Pointers".
 * 
 * Although csWeakRef<> implements the necessary operators to transparently use
 * it like an actual pointer, this usage is <em>not</em> thread-safe: when
 * using multiple threads, it can happen that a weak pointer appears valid in
 * one thread, but is being destroyed (or has been destroyed) in another thread.
 * For <b>thread-safe usage</b>, convert the weak pointer to a csRef<> first
 * by using a Get() method: This ensures that a valid returned pointer stays
 * valid and that an invalid pointer is returned if the object is being
 * destroyed.
 */
template <class T>
class csWeakRef
{
private:
  union
  {
    T* obj;
    void* obj_void;
  };
#if defined(CS_DEBUG)
  void* this_saved;
#endif
  mutable CS::Threading::Mutex obj_mutex;

  /**
   * Unlink the object pointed to by this weak reference so that
   * this weak references will not automatically be set to 0 after
   * the object is destroyed.
   */
  void Unlink ()
  {
    if (obj) obj->RemoveRefOwner (&obj_void);
  }

  /**
   * Link the object again.
   */
  void Link ()
  {
    if (obj) obj->AddRefOwner (&obj_void, &obj_mutex);
  }

public:
  /**
   * Construct an empty weak reference.
   */
  csWeakRef () : obj (0) 
  {
#if defined(CS_DEBUG)
    this_saved = this;
#endif
  }

  /**
   * Construct a weak reference from a normal pointer.
   */
  csWeakRef (T* newobj)
  {
#if defined(CS_DEBUG)
    this_saved = this;
#endif
    obj = newobj;
    Link ();
  }

  /**
   * Construct a weak reference from a csRef<>.
   */
  csWeakRef (csRef<T> const& newobj)
  {
#if defined(CS_DEBUG)
    this_saved = this;
#endif
    obj = newobj;
    Link ();
  }

  /**
   * Weak pointer copy constructor.
   */
  csWeakRef (csWeakRef const& other)
  {
#if defined(CS_DEBUG)
    this_saved = this;
#endif
    // Keep object from 'other' alive until we linked
    typename T::WeakReferencedKeepAlive other_obj_ref;
    {
      CS::Threading::ScopedLock<CS::Threading::Mutex> lock_other_mutex (other.obj_mutex);
      obj = other.obj;
      other_obj_ref = obj;
      (void)other_obj_ref;
    }
    Link ();
  }

  /**
   * Construct a weak reference from a csPtr. This will put the object
   * in the weak reference and then it will release the reference.
   */
  csWeakRef (const csPtr<T>& newobj)
  {
#if defined(CS_DEBUG)
    this_saved = this;
#endif
    csRef<T> r = newobj;
    obj = r;
    Link ();
  }

  /**
   * Weak pointer destructor.
   */
  ~csWeakRef ()
  {
#if defined(CS_DEBUG)
    CS_ASSERT_MSG ("A csWeakRef<> was memcpy()ed, which is not allowed",
      this_saved == this);
#endif
    /* We need to keep the old object alive for unlinking,
     * but we can't keep the obj_mutex locked (possible deadlock
     * when the object is in DecRef()) */
    typename T::WeakReferencedKeepAlive obj_ref (Get<typename T::WeakReferencedKeepAlive> ());
    (void)obj_ref;
    Unlink ();
  }

  /**
   * Assign a raw object reference to this weak reference.
   */
  csWeakRef& operator = (T* newobj)
  {
    CS::Threading::ScopedLock<CS::Threading::Mutex> lock_mutex (obj_mutex);
    if (obj != newobj)
    {
      {
	/* We need to keep the old object alive for unlinking,
	 * but we can't keep the obj_mutex locked (possible deadlock
	 * when the object is in DecRef()) */
	typename T::WeakReferencedKeepAlive old_obj_ref (obj);
	(void)old_obj_ref;
	obj_mutex.Unlock();
	Unlink ();
      }
      obj_mutex.Lock();
      obj = newobj;
      Link ();
    }
    return *this;
  }

  /**
   * Assign a csRef<> to this weak reference.
   */
  csWeakRef& operator = (csRef<T> const& newobj)
  {
    return (*this = (T*)newobj);
  }

  /**
   * Assign a csPtr reference to this weak reference. This
   * will cause a DecRef() on the pointer.
   */
  csWeakRef& operator = (csPtr<T> newobj)
  {
    csRef<T> r (newobj);
    return (*this = r);
  }

  /**
   * Assign another object to this weak reference.
   */
  csWeakRef& operator = (csWeakRef const& other)
  {
    this->operator=(other.obj);
    return *this;
  }

  /// Test if the two references point to same object.
  inline friend bool operator == (const csWeakRef& r1, const csWeakRef& r2)
  {
    CS::Threading::ScopedLock<CS::Threading::Mutex> lock_mutex1 (r1.obj_mutex);
    CS::Threading::ScopedLock<CS::Threading::Mutex> lock_mutex2 (r2.obj_mutex);
    return r1.obj == r2.obj;
  }
  /// Test if the two references point to different object.
  inline friend bool operator != (const csWeakRef& r1, const csWeakRef& r2)
  {
    CS::Threading::ScopedLock<CS::Threading::Mutex> lock_mutex1 (r1.obj_mutex);
    CS::Threading::ScopedLock<CS::Threading::Mutex> lock_mutex2 (r2.obj_mutex);
    return r1.obj != r2.obj;
  }
  /// Test if object pointed to by reference is same as obj.
  inline friend bool operator == (const csWeakRef& r1, T* obj)
  {
    CS::Threading::ScopedLock<CS::Threading::Mutex> lock_mutex1 (r1.obj_mutex);
    return r1.obj == obj;
  }
  /// Test if object pointed to by reference is different from obj.
  inline friend bool operator != (const csWeakRef& r1, T* obj)
  {
    CS::Threading::ScopedLock<CS::Threading::Mutex> lock_mutex1 (r1.obj_mutex);
    return r1.obj != obj;
  }
  /// Test if object pointed to by reference is same as obj.
  inline friend bool operator == (T* obj, const csWeakRef& r1)
  {
    CS::Threading::ScopedLock<CS::Threading::Mutex> lock_mutex1 (r1.obj_mutex);
    return r1.obj == obj;
  }
  /// Test if object pointed to by reference is different from obj.
  inline friend bool operator != (T* obj, const csWeakRef& r1)
  {
    CS::Threading::ScopedLock<CS::Threading::Mutex> lock_mutex1 (r1.obj_mutex);
    return r1.obj != obj;
  }

  inline friend bool operator < (const csWeakRef& r1, const csWeakRef& r2)
  {
    CS::Threading::ScopedLock<CS::Threading::Mutex> lock_mutex1 (r1.obj_mutex);
    CS::Threading::ScopedLock<CS::Threading::Mutex> lock_mutex2 (r2.obj_mutex);
    return r1.obj < r2.obj;
  }

  /// Dereference underlying object.
  CS_WEAKREF_METHOD_UNSAFE
  T* operator -> () const
  { return obj; }
  
  /// Cast weak reference to a pointer to the underlying object.
  CS_WEAKREF_METHOD_UNSAFE
  operator T* () const
  { return obj; }
  
  /// Dereference underlying object.
  CS_WEAKREF_METHOD_UNSAFE
  T& operator* () const
  { return *obj; }

  //@{
  /**
   * Safely get the weakly referenced object.
   * \tparam U Type as which the object pointer should be returned. Must be
   *   constructible or assignable from \c T*. It's recommended to make it a
   *   csRef<T> in order to keep the object alive as long as it's needed.
   */
  template<typename U>
  U Get() const
  {
    CS::Threading::ScopedLock<CS::Threading::Mutex> lock_mutex (obj_mutex);
    return U (obj);
  }
  template<typename U>
  void Get(U& ref) const
  {
    CS::Threading::ScopedLock<CS::Threading::Mutex> lock_mutex (obj_mutex);
    ref = obj;
  }
  //@}


  /**
   * Weak pointer validity check.  Returns true if weak reference is pointing
   * at an actual object, otherwise returns false.
   */
  CS_WEAKREF_METHOD_UNSAFE
  bool IsValid () const
  { return (obj != 0); }

  /// Return a hash value for this smart pointer.
  uint GetHash() const
  {
    CS::Threading::ScopedLock<CS::Threading::Mutex> lock_mutex (obj_mutex);
    return (uintptr_t)obj;    
  }
};

#endif // __CS_WEAKREF_H__

