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

#ifndef __VTPOOL_H__
#define __VTPOOL_H__

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
   * Free an array of n vertices.
   */
  virtual void FreeVertexArray (csVector3* ar, int n) = 0;
};

/**
 * This is a default implementation of csVertexArrayPool.
 * This implementation is rather trival. It will just do 'new' and
 * 'delete' to get the vertex arrays.
 */
class csDefaultVertexArrayPool
{
public:
  /**
   * Fetch a new array of n vertices.
   * Return NULL on failure.
   */
  virtual csVector3* GetVertexArray (int n)
  {
    return new csVector3[n];
  }

  /**
   * Free an array of n vertices.
   */
  virtual void FreeVertexArray (csVector3* ar, int)
  {
    delete[] ar;
  }
};

/**
 * This is another implementation of csVertexArrayPool. This
 * one takes vertices from a big pool. Note that 'FreeVertexArray'
 * does not work here. The unused vertices are lost.
 */
class csPooledVertexArrayPool
{
private:
  csVector3* pool;
  int lastn, maxn;

public:
  /// Allocate a vertex array pool with max maxn vertices.
  csPooledVertexArrayPool (int maxn)
  {
    csPooledVertexArrayPool::maxn = maxn;
    lastn = 0;
    pool = new csVector3[maxn];
  }

  /// Destroy pool and all vertex arrays in it.
  virtual ~csPooledVertexArrayPool ()
  {
    delete[] pool;
  }

  /**
   * Fetch a new array of n vertices.
   * Return NULL on failure.
   */
  virtual csVector3* GetVertexArray (int n)
  {
    if (lastn+n > maxn) return NULL;
    lastn += n;
    return pool+lastn-n;
  }

  /**
   * Free an array of n vertices.
   */
  virtual void FreeVertexArray (csVector3*, int)
  {
  }

  /**
   * Reinitialize the pool.
   */
  void Clear ()
  {
    lastn = 0;
  }
};

#endif // __VTPOOL_H__

