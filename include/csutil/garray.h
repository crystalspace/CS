/*
    Crystal Space utility library: vector class interface
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __GARRAY_H__
#define __GARRAY_H__

/**
 * This is a macro that will declare a (possibly static) growable array
 * variable that is able to contain a number of elements of given type.
 * It has an reference count, so if you do an IncRef each time you make
 * use of it and an DecRef when you're done, the array will be automatically
 * freed when there are no more references to it.<p>
 * Methods:<p>
 * void SetLimit (int) - set max number of values the array can hold
 * int GetLimit () - query max number of values the array can hold
 * void IncRef ()/void DecRef () - Reference counter management
 * operator [] (int) - return a reference to Nth element of the array
 * <p>
 * Usage examples:
 * <p>
 * DECLARE_GROWING_ARRAY (static, LightArray, csLight*)
 * DECLARE_GROWING_ARRAY (, IntArray, int)
 */
#define DECLARE_GROWING_ARRAY(Static, Name, Type)			\
  Static class __##Name							\
  {									\
    Type *root;								\
    int Limit;								\
    int RefCount;							\
  public:								\
    __##Name ()								\
    { Limit = 0; RefCount = 0; root = NULL; }				\
    ~__##Name ()							\
    { SetLimit (0); }							\
    int GetLimit ()							\
    { return Limit; }							\
    void SetLimit (int iLimit)						\
    {									\
      if (!(Limit = iLimit))						\
      { if (root) { free (root); root = NULL; } }			\
      else								\
        root = (Type *)realloc (root, Limit * sizeof (Type));		\
    }									\
    void IncRef ()							\
    { RefCount++; }							\
    void DecRef ()							\
    {									\
      if (RefCount == 1) SetLimit (0);					\
      RefCount--;							\
    }									\
    Type &operator [] (int n)						\
    { return root [n]; }						\
    Type *GetArray ()							\
    { return root; }							\
  } Name;

#endif // __GARRAY_H__
