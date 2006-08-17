/*
    Copyright (C) 2006 by Benjamin Stover

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
/*#include "iutil/object.h"
#include "igeom/polymesh.h"
#include "iengine/mesh.h"
#include "imesh/object.h"
#include "imesh/objmodel.h"
#include "csutil/array.h"
#include "csutil/csstring.h"
#include "csgeom/vector3.h"
#include "csgeom/box.h" */
#include "pvscomp.h"

extern "C"
{
#include "csplugincommon/cdd/setoper.h"
#include "csplugincommon/cdd/cddmp.h"
#include "csplugincommon/cdd/cddtypes.h"
#include "csplugincommon/cdd/cdd.h"
};

static dd_PolyhedraPtr MakeDDPoly(const csArray<Plucker>& planes);

static dd_PolyhedraPtr MakeDDPoly(const csArray<Plucker>& planes)
{
  // A row of the matrix will look like 0 H1 H2 H3 H4 H5 H6.
  // cdd needs matrix to look like b | -A where Ax >= b, but our hyperplanes
  // are defined such that Ax <= 0, if each row of A were a hyperplane.
  // Therefore, we set b to 0 and negate -A, which is A.

  dd_MatrixType* matrix = dd_CreateMatrix (planes.Length (), 7);

  for (int i = 0; i < matrix->rowsize; i++)
  {
    const Plucker& plane = planes.Get (i);
    matrix->matrix[i][0][0] = 0;
    matrix->matrix[i][1][0] = plane[0];
    matrix->matrix[i][2][0] = plane[1];
    matrix->matrix[i][3][0] = plane[2];
    matrix->matrix[i][4][0] = plane[3];
    matrix->matrix[i][5][0] = plane[4];
    matrix->matrix[i][6][0] = plane[5];
  }

  dd_ErrorType err;
  dd_PolyhedraPtr poly = dd_DDMatrix2Poly (matrix, &err);
  dd_FreeMatrix (matrix);

  return poly;
}

void ExtremalPluckerPoints (const Polygon* source, const Polygon* occluder,
    csArray<Plucker>& fill)
{
  for (int i = 0; i < source->numVertices; i++)
  {
    for (int j = 0; j < occluder->numVertices; j++)
    {
      const csVector3& vertex1 = source->vertices[source->index[i]];
      const csVector3& vertex2 = occluder->vertices[occluder->index[j]];
      fill.Push (Plucker (vertex1, vertex2));
    }
  }
}

void PluckerPlanes (const Polygon* source, const Polygon* occluder,
    csArray<Plucker>& fill)
{
  for (int i = 0; i < source->numVertices; i++)
  {
    const csVector3& vertex1 = source->vertices[source->index[i]];
    const csVector3& vertex2 = 
      source->vertices[source->index[(i + 1) % source->numVertices]];
    fill.Push (Plucker (vertex1, vertex2).Dual ());
  }
}

void VertexRepresentation (const csArray<Plucker>& planes,
    csArray<Plucker>& fill)
{
  dd_PolyhedraPtr cddrep = MakeDDPoly (planes);
  dd_MatrixPtr matrix = dd_CopyGenerators (cddrep);
  dd_FreePolyhedra (cddrep);
  for (int i = 0; i < matrix->rowsize; i++)
  {
    if (matrix->matrix[i][0][0] == 1)
    {
      // This is a vertex
      fill.Push (Plucker(matrix->matrix[i][1][0], matrix->matrix[i][2][0], 
            matrix->matrix[i][3][0], matrix->matrix[i][4][0], 
            matrix->matrix[i][5][0], matrix->matrix[i][6][0]));
    }
  }
  dd_FreeMatrix (matrix);
}

void CapPlanes (const csArray<Plucker>& vertices, csArray<Plucker>& fill)
{
}
