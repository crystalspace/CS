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
   * run-time error, however it is safe to invoke IsValid() and Assign().
   */
  csRef () : obj (0) {}

  /**
   * Construct a smart pointer from a raw object reference.  Takes ownership of
   * the referenced object by assuming that its IncRef() method has already
   * been called, and invokes its DecRef() method upon destruction.
   */
  explicit csRef (T* newobj) : obj (newobj) { }
  
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
   * Assign another object to this smart pointer.
   */
  void Assign (csRef const& other)
  {
    if (obj != other.obj)
    {
      if (obj) obj->DecRef();
      obj = other.obj;
      if (obj) obj->IncRef();
    }
  }

  /**
   * Assign a raw object reference to this smart pointer.  If
   * transfer_ownership is true, then it assumes that object's IncRef() method
   * has already been called on behalf of the smart pointer.  Otherwise a new
   * reference is created via IncRef().
   */
  void Assign (T* newobj, bool transfer_ownership = true)
  {
    T* oldobj = obj;
    obj = newobj;
    if (obj && !transfer_ownership)
      obj->IncRef();
    if (oldobj)
      oldobj->DecRef();
  }

  /**
   * Assign another object to this smart pointer via the assignment operator.
   */
  csRef& operator = (csRef const& other)
  {
    Assign (other);
    return *this;
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
   * Boolean cast.  Returns true if smart pointer is pointing at an actual
   * object, otherwise returns false.
   */
  operator bool () const
  { return (obj != 0); }

  /**
   * Smart pointer validity check.  Returns true if smart pointer is pointing
   * at an actual object, otherwise returns false.
   */
  bool IsValid () const
  { return (obj != 0); }
};

#endif // __CSREF_H__

