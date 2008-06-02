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
#include "cssysdef.h"
#include "csgeom/polyidx.h"

csPolyIndexed::csPolyIndexed (int start_size)
{
  max_vertices = start_size;
  vertices_idx = new int[max_vertices];
  MakeEmpty ();
}

csPolyIndexed::csPolyIndexed (csPolyIndexed &copy)
{
  max_vertices = copy.max_vertices;
  vertices_idx = new int[max_vertices];
  num_vertices = copy.num_vertices;
  memcpy (vertices_idx, copy.vertices_idx, sizeof (int) * num_vertices);
}

csPolyIndexed& csPolyIndexed::operator = (const csPolyIndexed& other)
{
  if (&other == this) return *this;
  delete[] vertices_idx;
  max_vertices = other.max_vertices;
  num_vertices = other.num_vertices;
  vertices_idx = new int[max_vertices];
  memcpy (vertices_idx, other.vertices_idx, sizeof (int) * num_vertices);
  return *this;
}

csPolyIndexed::~csPolyIndexed ()
{
  delete[] vertices_idx;
}

void csPolyIndexed::MakeEmpty ()
{
  num_vertices = 0;
}

void csPolyIndexed::MakeRoom (int new_max)
{
  if (new_max <= max_vertices) return ;

  int *new_vertices_idx = new int[new_max];
  memcpy (new_vertices_idx, vertices_idx, num_vertices * sizeof (int));
  delete[] vertices_idx;
  vertices_idx = new_vertices_idx;
  max_vertices = new_max;
}

int csPolyIndexed::AddVertex (int i)
{
  if (num_vertices >= max_vertices) MakeRoom (max_vertices + 5);
  vertices_idx[num_vertices] = i;
  num_vertices++;
  return num_vertices - 1;
}

//---------------------------------------------------------------------------
