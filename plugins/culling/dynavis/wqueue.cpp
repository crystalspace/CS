/*
    Copyright (C) 2002 by Jorrit Tyberghein

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
#include "cssysdef.h"
#include "csutil/sysfunc.h"
#include "csutil/scfstr.h"
#include "iutil/string.h"
#include "qint.h"
#include "qsqrt.h"
#include "csgeom/box.h"
#include "csgeom/math3d.h"
#include "wqueue.h"

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csWriteQueue)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iDebugHelper)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csWriteQueue::DebugHelper)
  SCF_IMPLEMENTS_INTERFACE(iDebugHelper)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csWriteQueue::csWriteQueue ()
{
  SCF_CONSTRUCT_IBASE (0);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiDebugHelper);

  free_elements = 0;
  queue_min = 0;
  queue_max = 0;
}

csWriteQueue::~csWriteQueue ()
{
  // First call Initialize() so that all items are moved to
  // 'free_elements'.
  Initialize ();
  while (free_elements)
  {
    csWriteQueueElement* el = free_elements->next;
    delete free_elements;
    free_elements = el;
  }
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiDebugHelper);
  SCF_DESTRUCT_IBASE ();
}

void csWriteQueue::Initialize ()
{
  if (queue_min)
  {
    CS_ASSERT (queue_max != 0);
    queue_max->next = free_elements;
    free_elements = queue_min;
    queue_min = queue_max = 0;
  }
}

void csWriteQueue::Append (const csBox2& box, float depth, void* obj)
{
  CS_ASSERT ((queue_min != 0) == (queue_max != 0));

  //-----
  // Find a free element to use or allocate one.
  //-----
  csWriteQueueElement* el;
  if (free_elements)
  {
    el = free_elements;
    free_elements = free_elements->next;
  }
  else
  {
    el = new csWriteQueueElement ();
  }

  //-----
  // Fill the element with sensible values.
  //-----
  el->box = box;
  el->depth = depth;
  el->obj = obj;

  //-----
  // If the queue is empty the situation is easy.
  //-----
  if (queue_min == 0)
  {
    queue_min = queue_max = el;
    el->next = el->prev = 0;
    return;
  }

  //-----
  // Since occluders are traversed roughly from front to back
  // it makes sense to try to include new objects near the end since
  // depth will be increasing mostly.
  //-----
  csWriteQueueElement* search_el = queue_max;
  while (search_el && depth < search_el->depth)
  {
    search_el = search_el->prev;
  }
  if (!search_el)
  {
    // Insert new element at the start.
    el->next = queue_min;
    el->prev = 0;
    queue_min->prev = el;
    queue_min = el;
  }
  else
  {
    // Insert new element right after search_el.
    el->prev = search_el;
    el->next = search_el->next;
    search_el->next = el;
    if (el->next)
      el->next->prev = el;
    else
      queue_max = el;
  }
}

bool csWriteQueue::IsPointAffected (const csVector2& p, float depth)
{
  csWriteQueueElement* el = queue_min;
  while (el)
  {
    if (el->depth > depth) return false;	// No occluder found.
    if (el->box.In (p)) return true;
    el = el->next;
  }
  return false;
}

void* csWriteQueue::Fetch (const csBox2& box, float depth, float& out_depth)
{
  CS_ASSERT ((queue_min != 0) == (queue_max != 0));

  csWriteQueueElement* el = queue_min;
  while (el)
  {
    if (el->depth > depth) return 0;	// No useful occluder found.
    if (el->relevant)
    {
      // The boxes intersect.
      out_depth = el->depth;
      void* obj = el->obj;

      // Remove object from queue.
      if (el->prev)
        el->prev->next = el->next;
      else
        queue_min = el->next;
      if (el->next)
        el->next->prev = el->prev;
      else
        queue_max = el->prev;
      // Put on the free list.
      el->next = free_elements;
      free_elements = el;

      return obj;
    }
    el = el->next;
  }
  return 0;
}

csPtr<iString> csWriteQueue::Debug_UnitTest ()
{
  scfString* rc = new scfString ();

  rc->DecRef ();
  return 0;
}

