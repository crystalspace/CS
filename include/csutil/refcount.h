/*
    Crystal Space Reference Counting Interface
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

#ifndef __CS_REFCOUNT_H__
#define __CS_REFCOUNT_H__

/**\file
 * Reference Counting Interface
 */

#include "csextern.h"
#include "csutil/threading/atomicops.h"
#include "csutil/reftrackeraccess.h"

/**
 * This is a class which provides basic reference-counting semantics.  It can
 * be used in conjunction with the smart pointer template class csRef (see
 * <ref.h>).  This class itself provides no functionality beyond
 * reference counting.  It is intended that you should subclass csRefCount and 
 * add needed functionality.
 */
class csRefCount
{
protected:
  int ref_count;

  // To avoid a problem with MSVC and multiple DLLs (with separate memory
  // space) we have to use a virtual destructor.
  // @@@ Another alternative is to force this function to be non-inline, as
  // doing so will most likely achieve the same result.
  virtual void Delete ()
  {
    delete this;
  }

  virtual ~csRefCount () 
  {
    csRefTrackerAccess::TrackDestruction (this, ref_count);
  }

public:
  //@{
  /// Initialize object and set reference to 1.
  csRefCount () : ref_count (1) 
  {
    csRefTrackerAccess::TrackConstruction (this);
  }
  csRefCount (const csRefCount& other) : ref_count (1) 
  {
    csRefTrackerAccess::TrackConstruction (this);
  }
  //@}

  /// Increase the number of references to this object.
  void IncRef () 
  { 
    csRefTrackerAccess::TrackIncRef (this, ref_count); 
    ref_count++; 
  }
  /// Decrease the number of references to this object.
  void DecRef ()
  {
    csRefTrackerAccess::TrackDecRef (this, ref_count);
    ref_count--;
    if (ref_count <= 0)
      Delete ();
  }
  /// Get the reference count (only for debugging).
  int GetRefCount () const { return ref_count; }
};

namespace CS
{
  namespace Utility
  {
    /**
     * This is a class which provides basic reference-counting semantics.  It can
     * be used in conjunction with the smart pointer template class csRef (see
     * <ref.h>).  This class itself provides no functionality beyond
     * reference counting.  It is intended that you should subclass FastRefCount
     * and add needed functionality.
     *
     * This class has slightly less overhead than csRefCount, but is also less
     * generally useable: 
     *  - Pointers can *not* be passed across plugin boundaries.
     *  - Deriving from a class C derived from \a ActualClass without using a
     *    virtual destructor in C can lead to the wrong destructor being
     *    called.
     *  - Using virtual methods eliminates the overhead advantage.
     */
    template<typename ActualClass>
    class FastRefCount
    {
    protected:
      int ref_count;

      ~FastRefCount () 
      {
        csRefTrackerAccess::TrackDestruction (this, ref_count);
      }

    public:
      //@{
      /// Initialize object and set reference to 1.
      FastRefCount () : ref_count (1) 
      {
        csRefTrackerAccess::TrackConstruction (this);
      }
      FastRefCount (const FastRefCount& other) : ref_count (1) 
      {
        csRefTrackerAccess::TrackConstruction (this);
      }
      //@}

      /// Increase the number of references to this object.
      void IncRef () 
      { 
        csRefTrackerAccess::TrackIncRef (this, ref_count); 
        ref_count++; 
      }
      /// Decrease the number of references to this object.
      void DecRef ()
      {
        csRefTrackerAccess::TrackDecRef (this, ref_count);
        ref_count--;
        if (ref_count <= 0)
          delete static_cast<ActualClass*> (this);
      }
      /// Get the reference count (only for debugging).
      int GetRefCount () const { return ref_count; }
    };

    /**
     * This class is used to hold a reference count seperate to the normal one.
     * It allows the tracking of 'internal' references (such as those held by a
     * collection), and the execution of some action when the ref count decrements
     * to 0.
     */
    class InternalRefCount
    {
    protected:
      int internal_ref_count;

      // To be implemented as needed.
      virtual void InternalRemove() { return; }

      virtual ~InternalRefCount ()
      {
        csRefTrackerAccess::TrackDestruction (this, internal_ref_count);
      }

    public:
      /// Initialize object and set internal references to 0.
      InternalRefCount () : internal_ref_count (0)
      {
        csRefTrackerAccess::TrackConstruction (this);
      }

      /// Increase the number of internal references to this object.
      void InternalIncRef()
      {
        csRefTrackerAccess::TrackIncRef (this, internal_ref_count);
        internal_ref_count++;
      }

      /// Decrease the number of internal references to this object.
      void InternalDecRef ()
      {
        csRefTrackerAccess::TrackDecRef (this, internal_ref_count);
        internal_ref_count--;
        if (internal_ref_count <= 0)
        {
          InternalRemove();
        }
      }

      /// Get the reference count (only for debugging).
      int GetInternalRefCount () const { return internal_ref_count; }
    };

    /**
     * This is a class which provides basic atomic reference-counting semantics.
     * It behaves like csRefCount, with the difference that the reference count
     * is increased/decreased atomically, making this class suitable for using
     * reference-counted objects across threads.
     */
    class AtomicRefCount
    {
    protected:
      int32 ref_count;

      // To avoid a problem with MSVC and multiple DLLs (with separate memory
      // space) we have to use a virtual destructor.
      // @@@ Another alternative is to force this function to be non-inline, as
      // doing so will most likely achieve the same result.
      virtual void Delete ()
      {
        delete this;
      }

      virtual ~AtomicRefCount () 
      {
        csRefTrackerAccess::TrackDestruction (this, ref_count);
      }

    public:
      //@{
      /// Initialize object and set reference to 1.
      AtomicRefCount () : ref_count (1) 
      {
        csRefTrackerAccess::TrackConstruction (this);
      }
      AtomicRefCount (const AtomicRefCount& other) : ref_count (1) 
      {
        csRefTrackerAccess::TrackConstruction (this);
      }
      //@}

      /// Increase the number of references to this object.
      void IncRef () 
      { 
        csRefTrackerAccess::TrackIncRef (this, ref_count); 
        CS::Threading::AtomicOperations::Increment (&ref_count);
      }
      /// Decrease the number of references to this object.
      void DecRef ()
      {
        csRefTrackerAccess::TrackDecRef (this, ref_count);
        if (CS::Threading::AtomicOperations::Decrement (&ref_count) == 0)
          Delete ();
      }
      /// Get the reference count (only for debugging).
      int GetRefCount () const
      { return CS::Threading::AtomicOperations::Read (&ref_count); }
    };

  } // namespace Utility
} // namespace CS

#endif // __CS_REFCOUNT_H__

