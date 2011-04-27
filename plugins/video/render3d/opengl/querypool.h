/*
  Copyright (C) 2010 by Frank Richter

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

#ifndef __QUERYPOOL_H__
#define __QUERYPOOL_H__

#include "csutil/dirtyaccessarray.h"

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{
  /**
   * Helper to keep glGenQueries calls down
   */
  class QueryPool
  {
    enum
    {
      /// How many queries to generate at once
      queryGenerateNum = 128,
      /// Discard queries when more than that number are allocated
      queryDiscardThreshold = queryGenerateNum*2,
      /// How many queries to discard
      queryDiscardNum = queryGenerateNum
    };
    
    csDirtyAccessArray<GLuint,
		       csArrayElementHandler<GLuint>,
		       CS::Container::ArrayAllocDefault,
		       csArrayCapacityFixedGrow<queryGenerateNum> > queries;
  public:
    GLuint AllocQuery();
    void RecycleQuery (GLuint query);
    
    void DiscardAllQueries ();
  };
}
CS_PLUGIN_NAMESPACE_END(gl3d)

#endif // __QUERYPOOL_H__

