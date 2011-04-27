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

/*
-------------------------------------------------------------------------
*
*           OPCODE collision detection plugin for CrystalSpace
*
*           OPCODE library was written by Pierre Terdiman
*                  ported to CS by Charles Quarra
*
-------------------------------------------------------------------------
*/

#include "cssysdef.h"
#include "csqsqrt.h"
#include "csqint.h"
#include "csutil/dirtyaccessarray.h"
#include "csgeom/transfrm.h"
#include "csgeom/tri.h"
#include "CSopcodecollider.h"
#include "igeom/trimesh.h"
#include "ivaria/collider.h"

#include "OPC_TreeBuilders.h"

CS_PLUGIN_NAMESPACE_BEGIN(csOpcode)
{

using namespace Opcode;

csOPCODECollider::csOPCODECollider (iTriangleMesh* mesh) :
  scfImplementationType(this)
{
  m_pCollisionModel = 0;
  indexholder = 0;
  vertholder = 0;
  //transform.m[0][3] = 0;
  //transform.m[1][3] = 0;
  //transform.m[2][3] = 0;
  //transform.m[3][3] = 1;

  opcMeshInt.SetCallback (&MeshCallback, this);

  GeometryInitialize (mesh);
}

inline float min3 (float a, float b, float c)
{ return (a < b ? (a < c ? a : (c < b ? c : b)) : (b < c ? b : c)); }
inline float max3(float a, float b, float c)
{ return (a > b ? (a > c ? a : (c > b ? c : b)) : (b > c ? b : c)); }

void csOPCODECollider::GeometryInitialize (csVector3* vertices,
    size_t vertcount, csTriangle* triangles, size_t tri_count)
{
  OPCODECREATE OPCC;
  size_t i;

  if (tri_count>=1)
  {
    m_pCollisionModel = new Opcode::Model;
    if (!m_pCollisionModel)
      return;

    vertholder = new Point [vertcount];
    indexholder = new unsigned int[3*tri_count];

    csBox3 tmp;
    tmp.StartBoundingBox ();
    for (i = 0; i < vertcount; i++)
    {
      tmp.AddBoundingVertex (vertices[i]);
      vertholder[i].Set (vertices[i].x , vertices[i].y , vertices[i].z);
    }

    radius = max3 (tmp.MaxX ()- tmp.MinX (), tmp.MaxY ()- tmp.MinY (),
	tmp.MaxZ ()- tmp.MinZ ());

    int index = 0;
    for (i = 0 ; i < tri_count ; i++)
    {
      indexholder[index++] = triangles[i].a;
      indexholder[index++] = triangles[i].b;
      indexholder[index++] = triangles[i].c;
    }

    opcMeshInt.SetNbTriangles ((udword)tri_count);
    opcMeshInt.SetNbVertices ((udword)vertcount);

    // Mesh data
    OPCC.mIMesh = &opcMeshInt;
    OPCC.mSettings.mRules = SPLIT_SPLATTER_POINTS | SPLIT_GEOM_CENTER;
    OPCC.mNoLeaf = true;
    OPCC.mQuantized = true;
    OPCC.mKeepOriginal = false;
    OPCC.mCanRemap = false;
  }
  else
    return;

  // this should create the OPCODE model
  bool status = m_pCollisionModel->Build (OPCC);
  if (!status) { return; };
}

void csOPCODECollider::GeometryInitialize (iTriangleMesh* mesh)
{
  // first, count the number of triangles polyset contains
  csVector3* vertices = mesh->GetVertices ();
  size_t vertcount = mesh->GetVertexCount ();
  csTriangle* triangles = mesh->GetTriangles ();
  size_t tri_count = mesh->GetTriangleCount ();
  GeometryInitialize (vertices, vertcount, triangles, tri_count);
}

csOPCODECollider::~csOPCODECollider ()
{
  if (m_pCollisionModel)
  {
    delete m_pCollisionModel;
    m_pCollisionModel = 0;
  }

  delete[] indexholder;
  delete[] vertholder;
}

void csOPCODECollider::MeshCallback (udword triangle_index,
				     VertexPointers& triangle,
				     void* user_data)
{
  csOPCODECollider* collider = (csOPCODECollider*)user_data;

  udword *tri_array = collider->indexholder;
  Point *vertholder = collider->vertholder;
  int index = 3 * triangle_index;
  triangle.Vertex[0] = &vertholder [tri_array[index]] ;
  triangle.Vertex[1] = &vertholder [tri_array[index + 1]];
  triangle.Vertex[2] = &vertholder [tri_array[index + 2]];
}

}
CS_PLUGIN_NAMESPACE_END(csOpcode)
