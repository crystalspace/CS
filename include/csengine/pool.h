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

#ifndef POOL_H
#define POOL_H

#include "cscom/com.h"
#include "csengine/pol2d.h"
#include "csengine/sysitf.h"

/**
 * This is an object pool which holds objects of type
 * csPolygon2D. You can ask new instance from this pool.
 * If needed it will allocate one for you but ideally it can
 * give you one which was allocated earlier.
 */
class csPolygon2DPool
{
private:
  struct PoolObj
  {
    PoolObj* next;
    csPolygon2D* pol2d;
  };
  PoolObj* alloced;
  PoolObj* freed;

public:
  /// Create an empty pool.
  csPolygon2DPool () : alloced (NULL), freed (NULL) { }

  /// Destroy pool and all objects in the pool.
  ~csPolygon2DPool ()
  {
    while (alloced)
    {
      PoolObj* n = alloced->next;
      CHK (delete alloced->pol2d);
      CHK (delete alloced);
      alloced = n;
    }
    while (freed)
    {
      PoolObj* n = freed->next;
      CHK (delete freed->pol2d);
      CHK (delete freed);
      freed = n;
    }
  }

  /// Allocate a new object in the pool.
  csPolygon2D* Alloc ()
  {
    PoolObj* pnew;
    if (freed)
    {
      pnew = freed;
      freed = freed->next;
    }
    else
    {
      CHK (pnew = new PoolObj ());
      CHK (pnew->pol2d = new csPolygon2D ());
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
  void Free (csPolygon2D* pol)
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
      CsPrintf (MSG_INTERNAL_ERROR, "Object was not allocated from csPolygon2D pool!\n");
    }
  }
};


#endif /*POOL_H*/
