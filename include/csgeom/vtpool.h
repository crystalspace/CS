/*
  Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __CS_VTPOOL_H__
#define __CS_VTPOOL_H__

#include "csgeom/vector3.h"

/**
 * This is a generalized vertex array pool. Use this to obtain
 * vertex arrays of some length. This is an abstract class.
 */
class csVertexArrayPool
{
public:
  /// Destroy pool and all vertex arrays in it.
  virtual ~csVertexArrayPool () { }

  /**
   * Fetch a new array of n vertices.
   * Return NULL on failure.
   */
  virtual csVector3* GetVertexArray (int n) = 0;

  /**
   * Free an array of n vertices. Implementations of FreeVertexArray()
   * are guaranteed to check if 'ar' == NULL and do nothing in that case.
   */
  virtual void FreeVertexArray (csVector3* ar, int n) = 0;
};

/**
 * This is a default implementation of csVertexArrayPool.
 * This implementation is rather trival. It will just do 'new' and
 * 'delete' to get the vertex arrays.
 */
class csDefaultVertexArrayPool : public csVertexArrayPool
{
public:
  csDefaultVertexArrayPool ();

  /**
   * Fetch a new array of n vertices.
   * Return NULL on failure.
   */
  virtual csVector3* GetVertexArray (int n)
  {
    return new csVector3[n];
  }

  /// Free an array of n vertices.
  virtual void FreeVertexArray (csVector3* ar, int)
  {
    delete[] ar;
  }

  /// Fetch the singleton instance of this pool.
  CS_DECLARE_STATIC_CLASSVAR_REF(default_pool,GetDefaultPool,csDefaultVertexArrayPool)
};

/**
 * This is another implementation of csVertexArrayPool. This
 * one takes vertices from a big pool. Note that 'FreeVertexArray'
 * only works to delete the last allocated array. i.e. you can only
 * allocate and delete array in a stack fashion.
 */
class csStackedVertexArrayPool : public csVertexArrayPool
{
private:
  csVector3* pool;
  int lastn, maxn;

public:
  /// Allocate a vertex array pool with max maxn vertices.
  csStackedVertexArrayPool (int maxn)
  {
    csStackedVertexArrayPool::maxn = maxn;
    lastn = 0;
    pool = new csVector3[maxn];
  }

  /// Destroy pool and all vertex arrays in it.
  virtual ~csStackedVertexArrayPool ()
  {
    delete[] pool;
  }

   /// Fetch a new array of n vertices.  Return NULL on failure.
  virtual csVector3* GetVertexArray (int n)
  {
    if (lastn+n > maxn) return NULL;
    lastn += n;
    return pool+lastn-n;
  }

  /// Free an array of n vertices.
  virtual void FreeVertexArray (csVector3* ar, int n)
  {
    if (ar == pool+lastn-n) lastn -= n;
  }

  /// Reinitialize the pool.
  void Clear ()
  {
    lastn = 0;
  }
};

/**
 * This is another implementation of csVertexArrayPool. This
 * one takes vertices from a big pool. It allows random allocation
 * and free of arrays but it less efficient than csStackedVertexArrayPool.
 */
class csPooledVertexArrayPool : public csVertexArrayPool
{
private:
  struct PoolEl
  {
    PoolEl* next;
    int n;	// Number of vertices.
    csVector3 first_vertex;
  };
  // For all common number of vertices there are specific pools.
  PoolEl* pool[6];      // For sizes 3, 4, 5, 6, 7, and 8.
  PoolEl* miscpool;	// For all other sizes.

public:
  /// Allocate a vertex array pool.
  csPooledVertexArrayPool ();

  /// Destroy pool and all vertex arrays in it.
  virtual ~csPooledVertexArrayPool ();

  /// Fetch a new array of n vertices.  Return NULL on failure.
  virtual csVector3* GetVertexArray (int n);

  /// Free an array of n vertices.
  virtual void FreeVertexArray (csVector3* ar, int n);

  /// Fetch the singleton instance of this pool.
  CS_DECLARE_STATIC_CLASSVAR_REF(default_pool,GetDefaultPool,csPooledVertexArrayPool)
};

#endif // __CS_VTPOOL_H__
