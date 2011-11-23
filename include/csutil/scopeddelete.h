/*
  Copyright (C) 2007 by Frank Richter

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
#ifndef __CSUTIL_SCOPEDDELETE_H__
#define __CSUTIL_SCOPEDDELETE_H__

/**\file
 * delete a pointer when exiting a scope
 */

#include "csutil/scopedpointer.h"

namespace CS
{
namespace Utility
{

  /**
   * Helper class to <tt>delete</tt> a pointer when exiting a scope.
   * \a T is the type pointed to.
   */
  template<class T>
  class ScopedDelete : public ScopedPointer<T>
  {
  public:
    /// Construct from given pointer
    ScopedDelete (T* ptr) : ScopedPointer<T> (ptr) {}
  };

} // namespace Utility
} // namespace CS

#endif // __CSUTIL_SCOPEDDELETE_H__
