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

#ifndef __CSREF_H__
#define __CSREF_H__

/**
 * This is a basic ref-counting class.
 */
class csRefCount
{
private:
  int ref_count;

  // To avoid a problem with MSVC and multiple dll (with seperate memory
  // space) we have to use a virtual destructor. Perhaps not inlining
  // could help too.
  virtual void Delete ()
  {
    delete this;
  }

protected:
  virtual ~csRefCount () { }

public:
  /// Initialize object and set reference to 1.
  csRefCount () : ref_count (1) { }

  /// Increment the number of references to this object.
  void IncRef () { ref_count++; }
  /// Decrement the reference count.
  void DecRef ()
  {
    ref_count--;
    if (ref_count <= 0)
    {
      Delete ();
    }
  }
  /// Get the ref count (only for debugging).
  int GetRefCount () const { return ref_count; }
};


#endif // __CSREF_H__

