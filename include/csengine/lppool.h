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

#ifndef LPPOOL_H
#define LPPOOL_H

#include "csengine/light.h"

/**
 * This is an object pool which holds objects of type
 * csLightPatch. You can ask new instances from this pool.
 * If needed it will allocate one for you but ideally it can
 * give you one which was allocated earlier.
 */
class csLightPatchPool
{
private:
  struct PoolObj
  {
    PoolObj* next;
    csLightPatch* lp;
  };
  /// List of allocated lightpatches.
  PoolObj* alloced;
  /// List of previously allocated, but now unused lightpatches.
  PoolObj* freed;

public:
  /// Create an empty pool.
  csLightPatchPool () : alloced (NULL), freed (NULL) { }

  /// Destroy pool and all objects in the pool.
  ~csLightPatchPool ()
  {
    while (alloced)
    {
      PoolObj* n = alloced->next;
      //delete alloced->lp; @@@ This free is not valid!
      // We should use a ref count on the pool itself so that we
      // now when all objects in the pool are freed and the
      // 'alloced' list will be empty.
      delete alloced;
      alloced = n;
    }
    while (freed)
    {
      PoolObj* n = freed->next;
      delete freed->lp;
      delete freed;
      freed = n;
    }
  }

  /// Allocate a new object in the pool.
  csLightPatch* Alloc ()
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
      pnew->lp = new csLightPatch ();
    }
    pnew->next = alloced;
    alloced = pnew;
    return pnew->lp;
  }

  /**
   * Free an object and put it back in the pool.
   * Note that it is only legal to free objects which were allocated
   * from the pool.
   */
  void Free (csLightPatch* lp)
  {
    lp->RemovePatch ();
    if (alloced)
    {
      PoolObj* po = alloced;
      alloced = alloced->next;
      po->lp = lp;
      po->next = freed;
      freed = po;
    }
    else
    {
      // Cannot happen!
      CS_ASSERT (false);
    }
  }

  /// Dump some information about this pool.
  void Dump ()
  {
    int cnt;
    cnt = 0;
    PoolObj* po = alloced;
    while (po) { cnt++; po = po->next; }
    printf ("LightPatch pool: %d allocated, ", cnt);
    cnt = 0;
    po = freed;
    while (po) { cnt++; po = po->next; }
    printf ("%d freed.\n", cnt);
  }
};

#endif /*LPPOOL_H*/
