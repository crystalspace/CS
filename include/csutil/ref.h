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

#ifndef __CSREF_H__
#define __CSREF_H__

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
class csPtr
{
private:
  T* obj;

public:
  csPtr (T* p) : obj (p) { }
  operator T* () const { return obj; }
};

/**
 * A smart pointer.  Maintains and correctly manages a reference to a
 * reference-counted object.  This template requires only that the object type
 * T implement the methods IncRef() and DecRef().  No other requirements are
 * placed upon T.
 */
template <class T>
class csRef
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
    obj = newobj;
  }

  /**
   * Construct a smart pointer from a raw object reference. Calls IncRef()
   * on the object.
   */
  csRef (T* newobj) : obj (newobj)
  {
    if (obj)
      obj->IncRef ();
  }
  
  /**
   * Smart pointer copy constructor.
   */
  csRef (csRef const& other) : obj (other.obj)
  {
    if (obj)
      obj->IncRef ();
  }

  /**
   * Smart pointer destructor.  Invokes DecRef() upon the underlying object.
   */
  ~csRef ()
  {
    if (obj)
      obj->DecRef ();
  }

  /**
   * Assign a csPtr to a smart pointer. Doesn't call IncRef() on
   * the object since it is assumed that the object in csPtr is already
   * IncRef()'ed.
   */
  csRef& operator = (const csPtr<T>& newobj)
  {
    T* oldobj = obj;
    // First assign and then DecRef() of old object!
    obj = newobj;
    if (oldobj)
      oldobj->DecRef ();
    return *this;
  }

  /**
   * Assign a raw object reference to this smart pointer.  This function
   * calls the object's IncRef() method.
   */
  csRef& operator = (T* newobj)
  {
    if (obj != newobj)
    {
      T* oldobj = obj;
      // It is very important to first assign the new value to
      // 'obj' BEFORE calling DecRef() on the old object. Otherwise
      // it is easy to get in infinite loops with objects being
      // destructed forever (when ref=NULL is used for example).
      obj = newobj;
      if (oldobj)
	oldobj->DecRef ();
      if (newobj)
	newobj->IncRef ();
    }
    return *this;
  }

  /**
   * Assign another object to this smart pointer.
   */
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
};

#endif // __CSREF_H__
