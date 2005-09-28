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

struct iBase;

/**
 * A weak reference. This is a reference to a reference counted object
 * but in itself it doesn't increment the ref counter. As soon as the
 * object is destroyed (i.e. the last REAL reference to it is removed)
 * all weak references pointing to that object are cleared. This kind of
 * reference is useful if you want to maintain some kind of cached objects
 * that can safely be removed as soon as the last reference to it is gone.
 * <p>
 * Note: this class assumes that the T type implements at least the following
 * functions:
 * - AddRefOwner()
 * - RemoveRefOwner()
 */
template <class T>
class csWeakRef
{
private:
  T* obj;

  /**
   * Unlink the object pointed to by this weak reference so that
   * this weak references will not automatically be set to 0 after
   * the object is destroyed.
   */
  void Unlink ()
  {
    if (obj) obj->RemoveRefOwner ((iBase**)&obj);
  }

  /**
   * Link the object again.
   */
  void Link ()
  {
    if (obj) obj->AddRefOwner ((iBase**)&obj);
  }

public:
  /**
   * Construct an empty weak reference.
   */
  csWeakRef () : obj (0) {}

  /**
   * Construct a weak reference from a normal pointer.
   */
  csWeakRef (T* newobj)
  {
    obj = newobj;
    Link ();
  }

  /**
   * Construct a weak reference from a csRef<>.
   */
  csWeakRef (csRef<T> const& newobj)
  {
    obj = newobj;
    Link ();
  }

  /**
   * Weak pointer copy constructor.
   */
  csWeakRef (csWeakRef const& other) : obj (other.obj)
  {
    Link ();
  }

  /**
   * Construct a weak reference from a csPtr. This will put the object
   * in the weak reference and then it will release the reference.
   */
  csWeakRef (const csPtr<T>& newobj)
  {
    csRef<T> r = newobj;
    obj = r;
    Link ();
  }

  /**
   * Weak pointer destructor.
   */
  ~csWeakRef ()
  {
    Unlink ();
  }

  /**
   * Assign a raw object reference to this weak reference.
   */
  csWeakRef& operator = (T* newobj)
  {
    if (obj != newobj)
    {
      Unlink ();
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
    if (newobj != obj)
    {
      Unlink ();
      obj = newobj;
      Link ();
    }
    return *this;
  }

  /**
   * Assign a csPtr reference to this weak reference. This
   * will cause a DecRef() on the pointer.
   */
  csWeakRef& operator = (csPtr<T> newobj)
  {
    csRef<T> r = newobj;
    if (obj != r)
    {
      Unlink ();
      obj = r;
      Link ();
    }
    return *this;
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
    return r1.obj == r2.obj;
  }
  /// Test if the two references point to different object.
  inline friend bool operator != (const csWeakRef& r1, const csWeakRef& r2)
  {
    return r1.obj != r2.obj;
  }
  /// Test if object pointed to by reference is same as obj.
  inline friend bool operator == (const csWeakRef& r1, T* obj)
  {
    return r1.obj == obj;
  }
  /// Test if object pointed to by reference is different from obj.
  inline friend bool operator != (const csWeakRef& r1, T* obj)
  {
    return r1.obj != obj;
  }
  /// Test if object pointed to by reference is same as obj.
  inline friend bool operator == (T* obj, const csWeakRef& r1)
  {
    return r1.obj == obj;
  }
  /// Test if object pointed to by reference is different from obj.
  inline friend bool operator != (T* obj, const csWeakRef& r1)
  {
    return r1.obj != obj;
  }

  /// Dereference underlying object.
  T* operator -> () const
  { return obj; }
  
  /// Cast weak reference to a pointer to the underlying object.
  operator T* () const
  { return obj; }
  
  /// Dereference underlying object.
  T& operator* () const
  { return *obj; }

  /**
   * Weak pointer validity check.  Returns true if weak reference is pointing
   * at an actual object, otherwise returns false.
   */
  bool IsValid () const
  { return (obj != 0); }
};

#endif // __CS_WEAKREF_H__

