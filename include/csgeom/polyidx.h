/*
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

#ifndef __CS_POLYIDX_H__
#define __CS_POLYIDX_H__

/**\file 
 * General polygon (space-agnostic).
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "csextern.h"

/**
 * The following class represents a general polygon.
 * Vertices are indexed relative to some vertex array instead of directly
 * represented in the polygon. Note that this polygon has no
 * knowledge of the actual values of the vertices. It only keeps
 * the indices. So it can actually be used both for 3D and 2D polygons.
 */
class CS_CRYSTALSPACE_EXPORT csPolyIndexed
{
protected:
  /// The vertex indices.
  int* vertices_idx;
  ///
  int num_vertices;
  ///
  int max_vertices;

public:
  /**
   * Make a new empty polygon.
   */
  csPolyIndexed (int start_size = 10);

  /// Copy constructor.
  csPolyIndexed (csPolyIndexed& copy);

  /// Destructor.
  virtual ~csPolyIndexed ();

  /// Assignment operator.
  csPolyIndexed& operator = (const csPolyIndexed& other);

  /**
   * Initialize the polygon to empty.
   */
  void MakeEmpty ();

  /**
   * Get the number of vertices.
   */
  int GetVertexCount () const { return num_vertices; }

  /**
   * Get the array with all vertex indices.
   */
  int* GetVertexIndices () { return vertices_idx; }

  /**
   * Get the array with all vertex indices.
   */
  const int* GetVertexIndices () const { return vertices_idx; }

  /**
   * Get the specified vertex index.
   */
  int GetVertex (int i)  const
  {
    if (i<0 || i>=num_vertices) return -1;
    return vertices_idx[i];
  }

  /**
   * Get the specified vertex index.
   */
  int& operator[] (int i)
  {
    CS_ASSERT (i >= 0 && i < num_vertices);
    return vertices_idx[i];
  }

  /**
   * Get the specified vertex index.
   */
  int& operator[] (int i) const
  {
    CS_ASSERT (i >= 0 && i < num_vertices);
    return vertices_idx[i];
  }

  /**
   * Make room for at least the specified number of vertices.
   */
  void MakeRoom (int new_max);

  /**
   * Add a vertex index to the polygon.
   * Return index of added index.
   */
  int AddVertex (int i);
};

/** @} */

#endif // __CS_POLYIDX_H__
