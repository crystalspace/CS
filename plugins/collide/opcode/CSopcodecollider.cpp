/*
    Copyright (C) 1998,2000 by Written by Alex Pfaffe

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

// The following classes are utility classes for the RAPID collision detection
// algorithm. The code is based on the UNC implementation of the RAPID
// algorithm:

/*************************************************************************\

  Copyright 1995 The University of North Carolina at Chapel Hill.
  All Rights Reserved.

  Permission to use, copy, modify and distribute this software and its
  documentation for educational, research and non-profit purposes, without
  fee, and without a written agreement is hereby granted, provided that the
  above copyright notice and the following three paragraphs appear in all
  copies.

  IN NO EVENT SHALL THE UNIVERSITY OF NORTH CAROLINA AT CHAPEL HILL BE
  LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
  CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE
  USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY
  OF NORTH CAROLINA HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH
  DAMAGES.

  THE UNIVERSITY OF NORTH CAROLINA SPECIFICALLY DISCLAIM ANY
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
  PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
  NORTH CAROLINA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
  UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

  The authors may be contacted via:

  US Mail:             S. Gottschalk
                       Department of Computer Science
                       Sitterson Hall, CB #3175
                       University of N. Carolina
                       Chapel Hill, NC 27599-3175

  Phone:               (919)962-1749

  EMail:              {gottscha}@cs.unc.edu


\**************************************************************************/

#include "cssysdef.h"
#include "qsqrt.h"
#include "qint.h"
#include "csutil/garray.h"
#include "csgeom/transfrm.h"
#include "CSopcodecollider.h"
//#include "prapid.h"
#include "igeom/polymesh.h"
#include "ivaria/collider.h"

using namespace Opcode;
//#define CD_MAX_COLLISION    1000

// This array contains the colliding pairs

//CS_TYPEDEF_GROWING_ARRAY_REF (prapid_CD_contact, csCollisionPair);
//CS_IMPLEMENT_STATIC_VAR (GetCD_contact, prapid_CD_contact,())

//static prapid_CD_contact *CD_contact = NULL;

//static int hits = 0;
// Array of hits.
//static csRapidCollider* hitv[CD_MAX_COLLISION][2];
//static int currHit;

///
//CS_IMPLEMENT_STATIC_CLASSVAR (csRapidCollider, mR, GetMR, csMatrix3,())
//CS_IMPLEMENT_STATIC_CLASSVAR (csRapidCollider, mT, GetMT, csVector3, (0))

///
///

SCF_IMPLEMENT_IBASE (csOPCODECollider)
  SCF_IMPLEMENTS_INTERFACE (iCollider)
SCF_IMPLEMENT_IBASE_END


csOPCODECollider::csOPCODECollider (iPolygonMesh* mesh)
{
  SCF_CONSTRUCT_IBASE (NULL);
  transform.m[0][3]=0;
  transform.m[1][3]=0;
  transform.m[2][3]=0;
  transform.m[3][3]=1;  
  GeometryInitialize (mesh);
 }

inline float min3 (float a, float b, float c)
{ return (a < b ? (a < c ? a : (c < b ? c : b)) : (b < c ? b : c)); }
inline float max3(float a, float b, float c)
{ return (a > b ? (a > c ? a : (c > b ? c : b)) : (b > c ? b : c)); }

void csOPCODECollider::GeometryInitialize (iPolygonMesh* mesh)
{
  OPCODECREATE OPCC;

  int i, v;
  int tri_count = 0;
  // first, count the number of triangles polyset contains
  csVector3* vertices = mesh->GetVertices ();
  int vertcount = mesh->GetVertexCount();
  csMeshedPolygon* polygons = mesh->GetPolygons ();
  int polycnt = mesh->GetPolygonCount ();
  for (i = 0; i < polycnt ; i++)
  {
    csMeshedPolygon& p = polygons[i];
    tri_count += p.num_vertices - 2;
  }

  if (tri_count>1)
  {
    m_pCollisionModel = new OPCODE_Model;
    if (!m_pCollisionModel)
      return;

   vertholder=(Point*)malloc( sizeof(Point)*vertcount );
   indexholder=(unsigned int*)malloc( sizeof(unsigned int)*3*tri_count );

   for (i = 0; i<vertcount ; i++) {
    vertholder[i].Set( vertices[i].x , vertices[i].y , vertices[i].z );
   };
   int* vidx;
   int index=0;
   int tri;
   for (i = 0; i<polycnt ; i ++) {
   csMeshedPolygon& p = polygons[i];
      vidx = p.vertices;
      tri=0;
      for (v = 2; v < p.num_vertices; v++ , tri+=3  ) //triangulation
      {
       indexholder[ index + tri ]=vidx[0];
       indexholder[ index + tri + 1 ]=vidx[v-1];
       indexholder[ index + tri + 2 ]=vidx[v];
         };
	 index+=3*(p.num_vertices - 2);
   };
      // Mesh data
       OPCC.NbTris=tri_count;
       OPCC.NbVerts=vertcount;
       OPCC.Tris=indexholder;
       OPCC.Verts=vertholder;
       OPCC.Rules = SPLIT_COMPLETE | SPLIT_SPLATTERPOINTS | SPLIT_BESTAXIS ;
       OPCC.NoLeaf = true;
       OPCC.Quantized = true;
        };

	bool status = m_pCollisionModel->Build(OPCC);  // this should create the OPCODE model
	if (!status) {	return;  };
	};
  

csOPCODECollider::~csOPCODECollider ()
{
  if (m_pCollisionModel)
  {
    delete m_pCollisionModel;
    m_pCollisionModel = NULL;
  }

  if (indexholder)  delete[](indexholder);

  if (vertholder)   delete[](vertholder);
};


