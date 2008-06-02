/*
  Copyright (C) 2006 by Marten Svanfeldt

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
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

#ifndef __CS_CSUTIL_NONCOPYABLE_H__
#define __CS_CSUTIL_NONCOPYABLE_H__

namespace CS
{
namespace Implementation
{
  /**
   * Base-class which ensures that derived classes are not copy-constructible
   * or assignable
   */
  class NonCopyable_
  {
  protected:
    NonCopyable_ () {}
    ~NonCopyable_ () {}
  private:
    //Really make them hidden
    NonCopyable_ (const NonCopyable_&);
    const NonCopyable_& operator= (const NonCopyable_&);
  };
} // namespace Implementation

// Avoid Argument Dependent Lookup
typedef Implementation::NonCopyable_ NonCopyable;

} // namespace CS



#endif // __CS_CSUTIL_NONCOPYABLE_H__
