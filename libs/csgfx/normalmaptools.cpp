/*
  Copyright (C) 2004 by Frank Richter

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

#include "csgeom/tri.h"
#include "csgeom/vector2.h"
#include "csgeom/vector3.h"
#include "csutil/dirtyaccessarray.h"

#include "csgfx/normalmaptools.h"

void csNormalMappingTools::CalculateTangents (size_t numTriangles, 
					      const csTriangle* triangles, 
					      size_t numVertices, 
					      const csVector3* vertices,
					      const csVector3* normals,
					      const csVector2* texcoords, 
					      csVector3* outTangents, 
					      csVector3* outBitangents)
{
  /*
    Calculate tangents & bitangents for a triangle mesh.

    Based upon code from Eric Lengyel. See
    http://www.terathon.com/code/tangent.html and
    http://thread.gmane.org/gmane.games.devel.algorithms/2116
   */
  memset (outTangents, 0, sizeof(csVector3) * numVertices);
  memset (outBitangents, 0, sizeof(csVector3) * numVertices);

  for (size_t a = 0; a < numTriangles; a++)
  {
    const csTriangle& triangle = triangles[a];
    const int i1 (triangle.a);
    const int i2 (triangle.b);
    const int i3 (triangle.c);
    
    const csVector3& v1 = vertices[i1];
    const csVector3& v2 = vertices[i2];
    const csVector3& v3 = vertices[i3];
    
    const csVector2& w1 = texcoords[i1];
    const csVector2& w2 = texcoords[i2];
    const csVector2& w3 = texcoords[i3];
    
    float x1 = v2.x - v1.x;
    float x2 = v3.x - v1.x;
    float y1 = v2.y - v1.y;
    float y2 = v3.y - v1.y;
    float z1 = v2.z - v1.z;
    float z2 = v3.z - v1.z;
    
    float s1 = w2.x - w1.x;
    float s2 = w3.x - w1.x;
    float t1 = w2.y - w1.y;
    float t2 = w3.y - w1.y;
    
    float r = 1.0f / (s1 * t2 - s2 * t1);
    csVector3 sdir ((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, 
      (t2 * z1 - t1 * z2) * r);
    csVector3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, 
      (s1 * z2 - s2 * z1) * r);
    
    outTangents[i1] += sdir;
    outTangents[i2] += sdir;
    outTangents[i3] += sdir;
    
    outBitangents[i1] += tdir;
    outBitangents[i2] += tdir;
    outBitangents[i3] += tdir;
  }

  for (size_t v = 0; v < numVertices; v++)
  {
    /*csVector3 n = normals[v];
    csVector3 t = outTangents[v];

    outTangents[v] = (t - n * (n * t));
    outTangents[v].Normalize();
    outBitangents[v] = t % n;
    outBitangents[v].Normalize();*/

    outTangents[v] = (outTangents[v] - normals[v] * (normals[v] * outTangents[v]));

    outTangents[v].Normalize();
    outBitangents[v].Normalize();

    //float a = (outTangents[v] % outBitangents[v]) * normals[v];
    //printf ("%g\n", a);
    //CS_ASSERT ((fabs(a) - 1.0f) < SMALL_EPSILON);
    /*
      Don't orthonormalize. The idea is to support sheared texture mappings.
     */
  }
}
