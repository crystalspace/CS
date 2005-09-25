/*
    Copyright (C) 2004 by Jorrit Tyberghein
	      (C) 2004 by Frank Richter

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

#ifndef __CS_IUTIL_REFTRACK_H__
#define __CS_IUTIL_REFTRACK_H__

/**\file
 * Reference tracker interface
 */

SCF_VERSION (iRefTracker, 0, 3, 0); 

/**
 * The reference tracker interface. Exposed by iSCF::SCF if enabled at
 * compile-time.
 */
struct iRefTracker : public iBase
{
  /// Called by an object if it has been IncRef()ed.
  virtual void TrackIncRef (void* object, int refCount) = 0;
  /// Called by an object if it has been DecRef()ed.
  virtual void TrackDecRef (void* object, int refCount) = 0;
  /// Called by an object if it constructed.
  virtual void TrackConstruction (void* object) = 0;
  /// Called by an object if it destructed.
  virtual void TrackDestruction (void* object, int refCount) = 0;

  /**
   * Match the most recent IncRef() to a 'tag' so it can be tracked what
   * IncRef()ed a ref. csRef<>s employ this mechanism and tag IncRef()s
   * with 'this'.
   */
  virtual void MatchIncRef (void* object, int refCount, void* tag) = 0;
  /**
   * Match the most recent DecRef() to a 'tag' so it can be tracked what
   * DecRef()ed a ref. csRef<>s employ this mechanism and tag DecRef()s
   * with 'this'.
   */
  virtual void MatchDecRef (void* object, int refCount, void* tag) = 0;

  /**
   * Add an alias. Basically says "mapTO is the same as obj." Used for
   * embedded interfaces.
   */
  virtual void AddAlias (void* obj, void* mapTo) = 0;
  /**
   * Remove an alias.
   */
  virtual void RemoveAlias (void* obj, void* mapTo) = 0;

  virtual void SetDescription (void* obj, const char* description) = 0;
};

#endif // __CS_IUTIL_REFTRACK_H__
