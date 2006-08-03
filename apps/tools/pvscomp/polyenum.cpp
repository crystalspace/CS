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
#include "csutil/array.h"
#include "csutil/csstring.h"
#include "csgeom/vector3.h"
#include "csgeom/box.h"
#include "pvscomp.h"

extern "C"
{
#include "csplugincommon/cdd/setoper.h"
#include "csplugincommon/cdd/cddmp.h"
#include "csplugincommon/cdd/cddtypes.h"
#include "csplugincommon/cdd/cdd.h"
};

static Polygon* CreateFace (const csVector3& a, const csVector3& b, 
    const csVector3& c, const csVector3& d)
{
  Polygon* p = new Polygon ();
  p->numVertices = 4;

  p->vertices = new csVector3[4];
  p->vertices[0] = a;
  p->vertices[1] = b;
  p->vertices[2] = c;
  p->vertices[3] = d;

  p->index = new int[4];
  p->index[0] = 0;
  p->index[1] = 1;
  p->index[2] = 2;
  p->index[3] = 3;

  return p;
}

static void FreeFace (Polygon* p)
{
  if (p->freeData)
  {
    delete[] p->vertices;
    delete[] p->index;
  }
  delete p;
}

static void FillWithFaces (const csBox3& region, csArray<Polygon*> tofill)
{
  // TODO:  debug!!
  tofill.Push (CreateFace (region.GetCorner (0), region.GetCorner (4),
                           region.GetCorner (5), region.GetCorner (1)));
  tofill.Push (CreateFace (region.GetCorner (2), region.GetCorner (6),
                           region.GetCorner (7), region.GetCorner (3)));
  tofill.Push (CreateFace (region.GetCorner (0), region.GetCorner (1),
                           region.GetCorner (3), region.GetCorner (2)));
  tofill.Push (CreateFace (region.GetCorner (4), region.GetCorner (6),
                           region.GetCorner (7), region.GetCorner (5)));
  tofill.Push (CreateFace (region.GetCorner (1), region.GetCorner (5),
                           region.GetCorner (7), region.GetCorner (3)));
  tofill.Push (CreateFace (region.GetCorner (0), region.GetCorner (2),
                           region.GetCorner (6), region.GetCorner (4)));
}

static PVSArray& GetPVS (csStaticKDTree* node)
{
  return *((PVSArray*) node->GetNodeData ());
}

void Compiler::ConstructPVS(csStaticKDTree* node)
{
  if (node->IsLeafNode ())
  {
    ConstructPVSForRegion (node->GetNodeBBox (), GetPVS (node));
  }
  else
  {
    ConstructPVS (node->GetChild1 ());
    ConstructPVS (node->GetChild2 ());
  }
}

void Compiler::ConstructPVSForRegion(const csBox3& region, 
    PVSArray& pvs)
{
  csArray<Polygon*> regionFaces;
  FillWithFaces (region, regionFaces);

  // for every face of the region
  for (int i = 0; i < 6; i++)
    ConstructPVSForFace(regionFaces[i], pvs);
}

void Compiler::ConstructPVSForFace(const Polygon* p, PVSArray& pvs)
{
/*  PolyhedronTree* tree;
    // for every polygon, front-to-back
    Polygon* target;
    const char* targetName;
    PolyhedronTree* addtree = PolyhedronTree::Construct (sourceFace, target,
        targetName);
    tree->Union (addtree);
  tree->MakeSizeSet ();
  tree->CollectPVS (pvsoftree); */
}

void Compiler::PropogatePVS(csStaticKDTree* node)
{
}


dd_PolyhedraPtr MakeDDPoly(const csArray<PluckerPlane>& planes)
{
  // A row of the matrix will look like 0 H1 H2 H3 H4 H5 H6.
  // cdd needs matrix to look like b | -A where Ax >= b, but our hyperplanes
  // are defined such that Ax <= 0, if each row of A were a hyperplane.
  // Therefore, we set b to 0 and negate -A, which is A.

  dd_MatrixType* matrix = dd_CreateMatrix (planes.Length (), 7);

  for (int i = 0; i < matrix->rowsize; i++)
  {
    const PluckerPlane& plane = planes.Get (i);
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

/*class Hyperplane 
{
private:
  float coeff[6];

public:
  float operator[] (int index) const
  {
    return coeff[index];
  }
  float& operator[] (int index) 
  {
    return coeff[index];
  }
}; */


/*  dd_MatrixType* matrix = new dd_MatrixType ();
  matrix->rowsize = planes.Length ();
  matrix->colsize = 7;

  matrix->matrix = new mytype*[matrix->rowsize];
  for (int i = 0; i < matrix->rowsize; i++)
  {
    const Hyperplane& plane = planes.Get (i);
    matrix->matrix[i] = new mytype[7];
    matrix->matrix[i][0][0] = 0;
    matrix->matrix[i][1][0] = plane[0];
    matrix->matrix[i][2][0] = plane[1];
    matrix->matrix[i][3][0] = plane[2];
    matrix->matrix[i][4][0] = plane[3];
    matrix->matrix[i][5][0] = plane[4];
    matrix->matrix[i][6][0] = plane[5];
  }

  // Set the rest of the data
  set_initialize (& (matrix->linset), planes.Length ());
  matrix->representation = dd_Inequality;
  matrix->objective = dd_LPnone;
  matrix->numbtype = dd_Real;
  dd_InitializeArow (7, & (matrix->rowvec)); */


