/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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

#ifndef __OBJPOOL_H__
#define __OBJPOOL_H__

/**
 * This macro defines a 'pool' class for the given type. The pool class can
 * be used to create objects of the given type, but it will re-use older
 * objects if possible to save time. For this reason, unused objects of the
 * given type should not be deleted but given to the pool.
 */
#define DECLARE_OBJECT_POOL(name,type)					\
class name : private csObjectPool {					\
private:								\
  virtual void* CreateItem ()						\
  { return new type (); }						\
  virtual void FreeItem (void* o)					\
  { type *obj = (type*)o; delete obj; }					\
public:									\
  type *Allocate ()							\
  { return csObjectPool::Allocate (); }					\
  void Free (type *o)							\
  { csObjectPool::Free (o); }						\
};

/// This is a helper class for DECLARE_OBJECT_POOL
class csObjectPool
{
private:
  csSome *Objects;
  int Num, Max;

public:
  virtual void* CreateItem () = 0;
  virtual void FreeItem (void*) = 0;

  csObjectPool ()
  {
    Objects = new csSome [16];
    Num = 0;
    Max = 16;
  }
  ~csObjectPool ()
  {
	int i;
    for (i=0; i<Num; i++)
      FreeItem (Objects[Num]);
    delete[] Objects;
  }
  void *Allocate ()
  {
    if (Num > 0) {
      Num--;
      return Objects [Num];
    } else {
      return CreateItem ();
    }
  }
  void Free (void* o) {
    if (Num == Max) {
      csSome *old = Objects;
      Objects = new csSome [Max + 16];
      memcpy (Objects, old, sizeof (csSome) * Max);
      delete[] old;
      Max += 16;
    }
    Objects [Num] = o;
    Num++;
  }
};

#endif // __OBJPOOL_H__
