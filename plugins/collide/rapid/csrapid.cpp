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
#include "csrapid.h"
#include "rapcol.h"
#include "igeom/polymesh.h"
#include "csgeom/transfrm.h"
#include "csgeom/tri.h"
#include "csutil/scfstr.h"
#include "iutil/string.h"

CS_IMPLEMENT_PLUGIN

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csRapidCollideSystem)
  SCF_IMPLEMENTS_INTERFACE (iCollideSystem)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iDebugHelper)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csRapidCollideSystem::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csRapidCollideSystem::DebugHelper)
  SCF_IMPLEMENTS_INTERFACE (iDebugHelper)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csRapidCollideSystem)


csRapidCollideSystem::csRapidCollideSystem (iBase *pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiDebugHelper);
}

csRapidCollideSystem::~csRapidCollideSystem ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiDebugHelper);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

csPtr<iCollider> csRapidCollideSystem::CreateCollider (iPolygonMesh* mesh)
{
  csRapidCollider* col = new csRapidCollider (mesh);
  return csPtr<iCollider> (col);
}

bool csRapidCollideSystem::Collide (
  iCollider* collider1, const csReversibleTransform* trans1,
  iCollider* collider2, const csReversibleTransform* trans2)
{
  csRapidCollider* col1 = (csRapidCollider*)collider1;
  csRapidCollider* col2 = (csRapidCollider*)collider2;
  return col1->Collide (*col2, trans1, trans2);
}

csCollisionPair* csRapidCollideSystem::GetCollisionPairs ()
{
  return csRapidCollider::GetCollisions ();
}

size_t csRapidCollideSystem::GetCollisionPairCount ()
{
  return csRapidCollider::numHits;
}

void csRapidCollideSystem::ResetCollisionPairs ()
{
  csRapidCollider::CollideReset ();
  csRapidCollider::numHits = 0;
}

//-------------------------------------------------------------------------

#define RAP_ASSERT(test,msg) \
  if (!(test)) \
  { \
    str.Format ("csRapid failure (%d,%s): %s\n", int(__LINE__), \
    	#msg, #test); \
    return csPtr<iString> (rc); \
  }

struct TriPolygonMesh : public iPolygonMesh
{
  csVector3 verts[3];
  csMeshedPolygon poly[1];
  csTriangle triangle;
  int vt_idx[3];
  csFlags flags;

  SCF_DECLARE_IBASE;

  TriPolygonMesh ()
  {
    SCF_CONSTRUCT_IBASE (0);
    vt_idx[0] = 0;
    vt_idx[1] = 1;
    vt_idx[2] = 2;
    poly[0].num_vertices = 3;
    poly[0].vertices = vt_idx;
    triangle.a = 0;
    triangle.b = 1;
    triangle.c = 2;
    flags.Set (CS_POLYMESH_TRIANGLEMESH);
  }
  virtual ~TriPolygonMesh ()
  {
    SCF_DESTRUCT_IBASE();
  }

  virtual int GetVertexCount () { return 3; }
  virtual csVector3* GetVertices () { return verts; }
  virtual int GetPolygonCount () { return 1; }
  virtual csMeshedPolygon* GetPolygons () { return poly; }
  virtual int GetTriangleCount () { return 1; }
  virtual csTriangle* GetTriangles () { return &triangle; }
  virtual void Lock () { }
  virtual void Unlock () { }
  virtual csFlags& GetFlags () { return flags; }
  virtual uint32 GetChangeNumber () const { return 0; }
};

SCF_IMPLEMENT_IBASE (TriPolygonMesh)
  SCF_IMPLEMENTS_INTERFACE (iPolygonMesh)
SCF_IMPLEMENT_IBASE_END

csPtr<iString> csRapidCollideSystem::Debug_UnitTest ()
{
  scfString* rc = new scfString ();
  csString& str = rc->GetCsString ();

  TriPolygonMesh* tri1 = new TriPolygonMesh ();
  tri1->verts[0].Set (csVector3 (0, 5, 0));
  tri1->verts[1].Set (csVector3 (0, 3, 1));
  tri1->verts[2].Set (csVector3 (0, 3, -1));
  csReversibleTransform trans1;
  trans1.Translate (csVector3 (0, 0, 4));
  trans1.RotateThis (csVector3 (0, 1, 0), -1);

  TriPolygonMesh* tri2 = new TriPolygonMesh ();
  tri2->verts[0].Set (csVector3 (4, 5, 0));
  tri2->verts[1].Set (csVector3 (4, 3, 1));
  tri2->verts[2].Set (csVector3 (4, 3, -1));
  csReversibleTransform trans2;
  trans2.Translate (csVector3 (0, 0, 8));
  trans2.RotateThis (csVector3 (0, 1, 0), 1.4f);

  csRef<iCollider> cd1 (CreateCollider (tri1));
  csRef<iCollider> cd2 (CreateCollider (tri2));

  RAP_ASSERT (Collide (cd1, &trans1, cd2, &trans2) == true, "cd1 -> cd2");
  RAP_ASSERT (Collide (cd2, &trans2, cd1, &trans1) == true, "cd2 -> cd1");

  tri1->DecRef ();
  tri2->DecRef ();

  rc->DecRef ();
  return 0;
}

//-------------------------------------------------------------------------

