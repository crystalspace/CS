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

#include "cssysdef.h"

#include "csutil/graphalg.h"
#include "csutil/hash.h"
#include "csutil/array.h"
#include "csutil/fifo.h"

namespace CS
{
namespace Utility
{

  csArray<size_t> TopologicalSort (csArray<GraphEdge>& inputGraph)
  {    
    csArray<size_t> result;

    csHash<csArray<size_t>, size_t> adjacency; // Adjacency list
    csHash<size_t, size_t> predCount; // Predecessor counts
    csFIFO<size_t> noPred; // Nodes without predecessors

    // Build adjacency list
    for (size_t i = 0; i < inputGraph.GetSize (); ++i)
    {
      const GraphEdge& edge = inputGraph[i];

      csArray<size_t>& adjList = adjacency.GetOrCreate (edge.from);
      adjList.Push (edge.to);

      // Record adjacency
      size_t& count = predCount.GetOrCreate (edge.to, 0);
      count++;

      // Setup the targets
      adjacency.GetOrCreate (edge.to);

      // Make sure there's a count for the from
      predCount.GetOrCreate (edge.from, 0);
    }

    // Find any non-preceded nodes
    {
      csHash<size_t, size_t>::GlobalIterator it = predCount.GetIterator ();
      while (it.HasNext ())
      {
        size_t count, node;
        count = it.Next (node);

        if (!count)
        {
          noPred.Push (node);
        }
      }
    }

    // Go through the lists and remove any elements with no predecessors
    while (noPred.GetSize () != 0)
    {
      size_t from = noPred.PopTop ();

      csArray<size_t>& adjList = adjacency.GetOrCreate (from);

      for (size_t i = 0; i < adjList.GetSize (); ++i)
      {
        size_t to = adjList.Get (i);
        size_t& toCount = predCount.GetOrCreate (to, 0);
        toCount--;
        if (!toCount)
        {
          noPred.Push (to);
        }
      }

      result.Push (from);
    }

    // If there are any nodes left with predCount then there are cycles.. problem!
    
    return result;
  }

}
}
