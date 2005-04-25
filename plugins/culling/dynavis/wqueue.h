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

#ifndef __CS_WRITEQUEUE_H__
#define __CS_WRITEQUEUE_H__

#include "csgeom/vector2.h"
#include "csgeom/math2d.h"
#include "iutil/dbghelp.h"

// @@@ Hack(s) to avoid problems with static linking
#ifdef DYNAVIS_DEBUG
#define csWriteQueue	csWriteQueue_DEBUG
#endif

class csBox2;

struct csWriteQueueElement
{
  csWriteQueueElement* next, * prev;
  csBox2 box;
  float depth;
  void* obj;
  bool relevant;	// Can be used externally to mark items relevant.
};

/**
 * A Write Queue for delaying execution of writes in the coverage
 * buffer.
 */
class csWriteQueue : public iBase
{
private:
  // A single linked list of free elements in the queue.
  // The 'prev' pointer is not used in this list.
  csWriteQueueElement* free_elements;

  // A double linked list representing the current queue.
  // The 'queue_min' pointer points to the item with the lowest
  // depth value. The 'queue_max' pointer points to the item
  // with the highest depth values. The 'next' pointer will point
  // to higher depth values.
  csWriteQueueElement* queue_min;
  csWriteQueueElement* queue_max;

public:
  /// Create a new write queue.
  csWriteQueue ();
  /// Destroy the write queue.
  virtual ~csWriteQueue ();

  SCF_DECLARE_IBASE;

  /// Initialize the write queue to empty.
  void Initialize ();

  /// Append something to the queue.
  void Append (const csBox2& box, float depth, void* obj);

  /**
   * Fetch an object from the queue that can potentially help
   * occlusion for the given box and depth (box must intersect
   * with object in queue and depth > depth of object in queue).
   * If such an object is found it is also removed from the queue.
   * Returns the object if something was found or 0 otherwise.
   * When an object is found 'out_depth' will be set to the
   * depth of that object.
   * <p>
   * Note that the 'relevant' field in csWriteQueueElement will be
   * used by this function so make sure you initialize this correctly.
   */
  void* Fetch (const csBox2& box, float depth, float& out_depth);

  /**
   * Get the first object in the write queue. This is the object
   * with the lowest depth.
   */
  csWriteQueueElement* GetFirst () const { return queue_min; }

  /**
   * Test if there is an object in the queue that might affect
   * the given point.
   */
  bool IsPointAffected (const csVector2& p, float depth);

  // Debugging functions.
  csPtr<iString> Debug_UnitTest ();

  struct DebugHelper : public iDebugHelper
  {
    SCF_DECLARE_EMBEDDED_IBASE (csWriteQueue);
    virtual int GetSupportedTests () const
    {
      return CS_DBGHELP_UNITTEST;
    }
    virtual csPtr<iString> UnitTest ()
    {
      return scfParent->Debug_UnitTest ();
    }
    virtual csPtr<iString> StateTest ()
    {
      return 0;
    }
    virtual csTicks Benchmark (int)
    {
      return 0;
    }
    virtual csPtr<iString> Dump ()
    {
      return 0;
    }
    virtual void Dump (iGraphics3D*)
    {
    }
    virtual bool DebugCommand (const char*)
    {
      return false;
    }
  } scfiDebugHelper;
};

#endif // __CS_WRITEQUEUE_H__

