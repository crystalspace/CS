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

#ifndef POLY3D_H
#define POLY3D_H

#include "csgeom/math3d.h"

class Dumper;

/**
 * The following class represents a general 3D polygon.
 */
class csPoly3D
{
  friend class Dumper;

protected:
  /// The 3D vertices.
  csVector3* vertices;
  ///
  int num_vertices;
  ///
  int max_vertices;

public:
  /**
   * Make a new empty polygon.
   */
  csPoly3D (int start_size = 10);

  /// Copy constructor.
  csPoly3D (csPoly3D& copy);

  /// Destructor.
  virtual ~csPoly3D ();

  /**
   * Initialize the polygon to empty.
   */
  void MakeEmpty ();

  /**
   * Get the number of vertices.
   */
  int GetNumVertices () { return num_vertices; }

  /**
   * Get the array with all vertices.
   */
  csVector3* GetVertices () { return vertices; }

  /**
   * Get the specified vertex.
   */
  csVector3* GetVertex (int i) 
  {
    if (i<0 || i>=num_vertices) return NULL;
    return &vertices[i];
  }

  /**
   * Get the specified vertex.
   */
  csVector3& operator[] (int i)
  {
    return vertices[i];
  }

  /**
   * Get the first vertex.
   */
  csVector3* GetFirst ()
  { if (num_vertices<=0) return NULL;  else return vertices; }

  /**
   * Get the last vertex.
   */
  csVector3* GetLast ()
  { if (num_vertices<=0) return NULL;  else return &vertices[num_vertices-1]; }

  /**
   * Make room for at least the specified number of vertices.
   */
  void MakeRoom (int new_max);

  /**
   * Set the number of vertices.
   */
  void SetNumVertices (int n) { MakeRoom (n); num_vertices = n; }

  /**
   * Add a vertex (3D) to the polygon.
   * Return index of added vertex.
   */
  int AddVertex (const csVector3& v) { return AddVertex (v.x, v.y, v.z); }

  /**
   * Add a vertex (3D) to the polygon.
   * Return index of added vertex.
   */
  int AddVertex (float x, float y, float z);

  /**
   * Set all polygon vertices at once.
   */
  void SetVertices (csVector3 *v, int num)
  { memcpy (vertices, v, (num_vertices = num) * sizeof (csVector3)); }
};

/**
 * This is actually the same class as csPoly3D. But it has been
 * renamed to make it clear that it is for other uses. It also
 * adds some functionality specific to that use. In particular
 * this class is more used to hold an unordered collection of 3D vectors.
 */
class csVector3Array : public csPoly3D
{
public:
  /**
   * Add a vertex but first check if it isn't already present
   * in the array. Return the index that the vertex was added too.
   */
  int AddVertexSmart (const csVector3& v)
  { return AddVertexSmart (v.x, v.y, v.z); }

  /**
   * Add a vertex but first check if it isn't already present
   * in the array. Return the index that the vertex was added too.
   */
  int AddVertexSmart (float x, float y, float z);
};

#endif /*POLY3D_H*/
