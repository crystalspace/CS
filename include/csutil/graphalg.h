/*
  Copyright (C) 2008 by Marten Svanfeldt

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

#ifndef __CS_CSUTIL_GRAPHALG_H__
#define __CS_CSUTIL_GRAPHALG_H__

#include "csutil/array.h"

namespace CS
{
namespace Utility
{

  /**
   * Represent an edge in a graph
   */
  struct CS_CRYSTALSPACE_EXPORT GraphEdge 
  {
    /// Construct from start and end vertex
    GraphEdge (size_t from = 0, size_t to = 0)
      : from (from), to (to)
    {}

    /// Starting vertex
    size_t from;

    /// Ending vertex
    size_t to;
  };


  /**
   * Do a topological sort on a graph defined by its edges.
   * Returns the vertices in a topological order.
   *
   * \note If the graph is in fact cyclic the returned array will not
   * contain all nodes.
   * \param inputGraph the edges defining the graph
   */
  CS_CRYSTALSPACE_EXPORT
  csArray<size_t> TopologicalSort (csArray<GraphEdge>& inputGraph);

}
}


#endif
