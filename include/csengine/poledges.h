/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

#ifndef _CS_POLEDGES_H
#define _CS_POLEDGES_H

#include "csutil/hashmap.h"

class csPolygonInt;
class csPolygon3D;
class csPolygonEdges;

/**
 * A private structure inside the hashmap.
 */
struct csPolEdge
{
  csPolygon3D* p;
  int i1, i2;	// Edge; i1 < i2
};

/**
 * An iterator to iterate over all polygons sharing some
 * edge.
 */
class csPolEdgeIterator
{
  friend class csPolygonEdges;

private:
  /// Hash iterator to iterate over the key.
  csHashIterator* iterator;
  /// Indices of two vertices making the edge.
  int i1, i2;
  /// Current poledge structure.
  csPolEdge* current;

private:
  /**
   * Constructor is private. This object is only made
   * by the friend csPolygonEdges.
   */
  csPolEdgeIterator (csHashMap& edges, int i1, int i2);

public:
  /// Destructor.
  virtual ~csPolEdgeIterator ();

  /// Is there a next polygon in this iterator?
  bool HasNext ()
  {
    return current != NULL;
  }

  /// Get the next polygon.
  csPolygon3D* Next ();
};

/**
 * An iterator to iterate over all edges.
 */
class csEdgeIterator
{
  friend class csPolygonEdges;

private:
  /// Hash iterator.
  csHashIterator* iterator;
  /// Current poledge structure.
  csPolEdge* current;

private:
  /**
   * Constructor is private. This object is only made
   * by the friend csPolygonEdges.
   */
  csEdgeIterator (csHashMap& edges);

public:
  /// Destructor.
  virtual ~csEdgeIterator ();

  /// Is there a next edge in this iterator?
  bool HasNext ()
  {
    return current != NULL;
  }

  /// Get the next polygon/edge.
  csPolygon3D* Next (int& e1, int& e2);
};

/**
 * A class representing all edges in a set of polygons.
 */
class csPolygonEdges
{
private:
  /**
   * This hashmap contains entries for all edges (and
   * attached polygon). An edge is defined by the two
   * vertex indices increased with one and then multiplied
   * (i.e. (i1+1) * (i2+1)).
   */
  csHashMap edges;

public:
  /**
   * Construct this structure based on the given array
   * of csPolygonInt*. The csPolygonInt* are assumed to
   * be of type csPolygon3D.
   */
  csPolygonEdges (csPolygonInt** polygons, int num_polygons);

  /**
   * Destroy this set of edges
   */
  virtual ~csPolygonEdges ();

  /**
   * Return an iterator to iterate over all polygons sharing
   * some edge. 'delete' this iterator when ready.
   * i1 and i2 are the indices of the vertices making up the edge.
   * The order of i1 and i2 is irrelevant.
   */
  csPolEdgeIterator* GetPolygons (int i1, int i2)
  {
    return new csPolEdgeIterator (edges, i1, i2);
  }

  /**
   * Get an iterator to iterate over all edges and polygons
   * in this structure. 'delete' this iterator when ready.
   */
  csEdgeIterator* GetEdges ()
  {
    return new csEdgeIterator (edges);
  }
};

#endif // __CS_POLEDGES_H__

