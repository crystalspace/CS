/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#ifndef __CS_POLYPOOL_H__
#define __CS_POLYPOOL_H__

/**\file 
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "csextern.h"

#include "csgeom/poly2d.h"

/**
 * This is an object pool which holds objects of type
 * csPoly2D. You can ask new instances from this pool.
 * If needed it will allocate one for you but ideally it can
 * give you one which was allocated earlier.
 */
class csPoly2DPool
{
private:
  struct PoolObj
  {
    PoolObj* next;
    csPoly2D* pol2d;
  };
  /// List of allocated polygons.
  PoolObj* alloced;
  /// List of previously allocated, but now unused polygons.
  PoolObj* freed;

  // Factory for creating new polygons.
  csPoly2DFactory* factory;

public:
  /// Create an empty pool.
  csPoly2DPool (csPoly2DFactory* fact) : alloced (0), freed (0),
  	factory (fact) { }

  /// Destroy pool and all objects in the pool.
  ~csPoly2DPool ()
  {
    while (alloced)
    {
      PoolObj* n = alloced->next;
      //delete alloced->pol2d; @@@ This free is not valid!
      // We should use a ref count on the pool itself so that we
      // now when all objects in the pool are freed and the
      // 'alloced' list will be empty.
      delete alloced;
      alloced = n;
    }
    while (freed)
    {
      PoolObj* n = freed->next;
      delete freed->pol2d;
      delete freed;
      freed = n;
    }
  }

  /// Allocate a new object in the pool.
  csPoly2D* Alloc ()
  {
    PoolObj* pnew;
    if (freed)
    {
      pnew = freed;
      freed = freed->next;
    }
    else
    {
      pnew = new PoolObj ();
      pnew->pol2d = factory->Create ();
    }
    pnew->next = alloced;
    alloced = pnew;
    return pnew->pol2d;
  }

  /**
   * Free an object and put it back in the pool.
   * Note that it is only legal to free objects which were allocated
   * from the pool.
   */
  void Free (csPoly2D* pol)
  {
    if (alloced)
    {
      PoolObj* po = alloced;
      alloced = alloced->next;
      po->pol2d = pol;
      po->next = freed;
      freed = po;
    }
    else
    {
      // Cannot happen!
      CS_ASSERT (false);
    }
  }
};

/** @} */

#endif // __CS_POLYPOOL_H__
