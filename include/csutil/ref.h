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

#ifndef CS_SMARTPOINTER_H__
#define CS_SMARTPOINTER_H__

/**
 * Smart pointer like class. Keeps a reference to something that
 * implements IncRef() and DecRef() (either iBase or csRefCount at this
 * moment).
 */
template <class T>
class csRef
{
private:
  T* obj;

public:
  /**
   * Default constructor for a smart pointer. This will initialize
   * the object to NULL.
   */
  csRef () : obj (NULL) { }

  /**
   * Construct a new smart pointer from a previously IncRef'ed pointer.
   * This should ONLY be used to contain return values from older functions
   * that are not yet aware of smart pointers.
   */
  csRef (T* newobj) : obj (newobj) { }
  
  /**
   * Copy constructor for a smart pointer. This will increase the
   * ref count of the object from the other smart pointer.
   */
  csRef (const csRef& other) : obj (other.obj)
  {
    if (obj) obj->IncRef ();
  }

  /**
   * Clean up the reference.
   */
  ~csRef ()
  {
    if (obj) obj->DecRef ();
  }

  /**
   * Replace the object pointed to in this smart pointer with another one.
   * This will increase the ref count of the new object and decrease the
   * ref count of the old one.
   */
  void Set (T* newobj)
  {
    T* oldobj = obj;
    obj = newobj;
    if (obj) obj->IncRef ();
    if (oldobj) oldobj->DecRef ();
  }

  /**
   * Replace the object pointed to in this smart pointer with another one.
   * This function assume the new object has already be increffed before.
   */
  void Assign (T* newobj)
  {
    T* oldobj = obj;
    obj = newobj;
    if (oldobj) oldobj->DecRef ();
  }

  T* operator -> () const
  { return obj; }
  
  operator T* () const
  { return obj; }
  
  T& operator* () const
  { return *obj; }

  operator bool () const
  { return obj; }

  /**
   * Assign another smart pointer to this one.
   * Keeps correct track of references.
   */
  csRef& operator = (const csRef& other)
  {
    Set (other.obj);
    return *this;
  }
};

#endif

