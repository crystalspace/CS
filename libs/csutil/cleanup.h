/*
  Crystal Space Windowing System: vector class interface
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

#ifndef __CSCLEANUP_H__
#define __CSCLEANUP_H__

#include "csutil/csvector.h"

class csCleanable;

/**
 * This general class holds a number of csCleanable
 * classes which it will clean up at destruction. This
 * system is useful for allocating stuff for data structures
 * for which there is no clear place to deallocate them. They
 * can be put in this class and they will be deallocated when
 * this class is.
 */
class csCleanup : public csVector
{
public:
  /// Construct.
  csCleanup () : csVector (8, 16) { }

  virtual bool FreeItem (csSome Item);
};

/**
 * A class which can be cleaned up by csCleanup.
 * Another class should subclass this class or use it
 * by delegation. The destructor should be overriden
 * so that the needed cleanups can be done.
 */
class csCleanable
{
public:
  virtual ~csCleanable () { }
};

#endif // __CSCLEANUP_H__
