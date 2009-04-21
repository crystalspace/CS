/* 
    Copyright (C) 2003-2007 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter
          (C) Hristo Hristov, Boyan Hristov

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

#ifndef __CS_STENCIL2_TRIMESH_H__
#define __CS_STENCIL2_TRIMESH_H__

#include "csgeom/trimesh.h"
#include "csgeom/trimeshtools.h"

class csStencil2TriangleMesh : public scfImplementation1<
			       csStencil2TriangleMesh, iTriangleMesh>
{
private:
  csDirtyAccessArray<csVector3> verts;
  csDirtyAccessArray<csVector3> face_normals;
  csDirtyAccessArray<csTriangle> tris;
  csFlags flags;
  uint32 changenr;

  void CalcFaceNormals()
  {
    face_normals.SetSize (0);
    face_normals.SetMinimalCapacity (tris.GetSize ());
    size_t i;
    for (i = 0 ; i < tris.GetSize () ; i ++) 
    { 
      csVector3 face_normal = (verts[tris[i].b] - verts[tris[i].a]) %
      			      (verts[tris[i].c] - verts[tris[i].a]);
      face_normal.Normalize();
      face_normals.Push(face_normal);
    }
  }

public:
  csStencil2TriangleMesh () : scfImplementationType (this), changenr (~0)
  {
  }

  virtual ~csStencil2TriangleMesh ()
  {
  }

  void ShapeChanged ()
  {
    changenr++;
  }

  void AddTris (csTriangle* newtris, size_t size)
  {
    tris.SetCapacity (tris.GetSize () + size);
    size_t i;
    for (i = 0 ; i < size ; i++)
    {
      tris.Push (newtris[i]);
    }
    CalcFaceNormals();
    ShapeChanged ();
  }

  void AddTris (const csArray<csTriangle>& newtris)
  {
    tris.SetCapacity (tris.GetSize () + newtris.GetSize ());
    size_t i;
    for (i = 0 ; i < newtris.GetSize () ; i++)
    {
      tris.Push (newtris[i]);
    }
    CalcFaceNormals();
    ShapeChanged ();
  }

  void CopyFrom (iTriangleMesh* triMesh)
  {
    size_t i;
    csVector3* vt = triMesh->GetVertices ();
    for (i = 0 ; i < triMesh->GetVertexCount () ; i++)
      verts.Push (vt[i]);
    AddTris (triMesh->GetTriangles (), triMesh->GetTriangleCount ());
  }

  virtual size_t GetVertexCount () 
  { 
    return verts.GetSize (); 
  }
  virtual csVector3* GetVertices () 
  { 
    return verts.GetArray (); 
  }
  virtual size_t GetTriangleCount () 
  { 
    return tris.GetSize (); 
  }
  virtual csTriangle* GetTriangles ()
  {
    return tris.GetArray ();
  }

  virtual void Lock () { }
  virtual void Unlock () { }
  virtual csFlags& GetFlags () { return flags; }
  virtual uint32 GetChangeNumber () const  { return changenr; }

  csVector3* GetFaceNormals()
  {
    return face_normals.GetArray();
  }
};

#endif // __CS_STENCIL2_TRIMESH_H__
