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
#include "cssysdef.h"
#include "csengine/polyint.h"
#include "csengine/poledges.h"
#include "csengine/polygon.h"

//---------------------------------------------------------------------------
csPolEdgeIterator::csPolEdgeIterator (csHashMap &edges, int i1, int i2)
{
  if (i1 > i2)
  {
    int swap = i1;
    i1 = i2;
    i2 = swap;
  }

  csPolEdgeIterator::i1 = i1;
  csPolEdgeIterator::i2 = i2;
  iterator = new csHashIterator (&edges, (i1 + 1) * (i2 + 1));
  if (iterator->HasNext ())
  {
    current = (csPolEdge *) (iterator->Next ());
    while (current && (current->i1 != i1 || current->i2 != i2))
    {
      current = (csPolEdge *) (iterator->Next ());
    }
  }
  else
    current = NULL;
}

csPolEdgeIterator::~csPolEdgeIterator ()
{
  delete iterator;
}

csPolygon3D *csPolEdgeIterator::Next ()
{
  if (!current) return NULL;

  csPolygon3D *rc_p = current->p;

  // Prepare to go to next polygon.
  current = (csPolEdge *) (iterator->Next ());
  while (current && (current->i1 != i1 || current->i2 != i2))
  {
    current = (csPolEdge *) (iterator->Next ());
  }

  return rc_p;
}

//---------------------------------------------------------------------------
csEdgeIterator::csEdgeIterator (csHashMap &edges)
{
  iterator = new csHashIterator (&edges);
  if (iterator->HasNext ())
    current = (csPolEdge *) (iterator->Next ());
  else
    current = NULL;
}

csEdgeIterator::~csEdgeIterator ()
{
  delete iterator;
}

csPolygon3D *csEdgeIterator::Next (int &e1, int &e2)
{
  if (!current) return NULL;
  e1 = current->i1;
  e2 = current->i2;

  // Prepare to go to next polygon.
  csPolygon3D *rc_p = current->p;
  current = (csPolEdge *) (iterator->Next ());
  return rc_p;
}

//---------------------------------------------------------------------------
csPolygonEdges::csPolygonEdges (csPolygonInt **polygons, int num_polygons) :
  edges(25247)  // Some prime number
{
  int i, j, j1;
  for (i = 0; i < num_polygons; i++)
  {
    csPolygon3D *p = (csPolygon3D *)polygons[i];
    const csPolyIndexed &pi = p->GetVertices ();
    j1 = pi.GetVertexCount () - 1;
    for (j = 0; j < pi.GetVertexCount (); j++)
    {
      int i1 = pi[j];
      int i2 = pi[j1];
      if (i1 > i2)
      {
        int swap = i1;
        i1 = i2;
        i2 = swap;
      }

      int key = (i1 + 1) * (i2 + 1);
      csPolEdge *pol_edge = new csPolEdge ();
      pol_edge->p = p;
      pol_edge->i1 = i1;
      pol_edge->i2 = i2;
      edges.Put (key, (void *)pol_edge);
      j1 = j;
    }
  }
}

csPolygonEdges::~csPolygonEdges ()
{
  csHashIterator iterator (&edges);
  while (iterator.HasNext ())
  {
    csPolEdge *pol_edge = (csPolEdge *) (iterator.Next ());
    delete pol_edge;
  }
}

//---------------------------------------------------------------------------
