/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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
#include "csengine/skeleton.h"

IMPLEMENT_IBASE (csSkeletonLimb)
  IMPLEMENTS_INTERFACE (iSkeletonLimb)
IMPLEMENT_IBASE_END

csSkeletonLimb::csSkeletonLimb ()
  : next (NULL), vertices (NULL), num_vertices (0), children (NULL), name(NULL)
{
  CONSTRUCT_IBASE (NULL);
}

csSkeletonLimb::~csSkeletonLimb ()
{
  delete [] vertices;
  while (children)
  {
    csSkeletonLimb* n = children->GetNext ();
    delete children;
    children = n;
  }
  if (name) free (name);
}

void csSkeletonLimb::AddVertex (int v)
{
  // We allocate 16 vertices at a time.
  int max_vertices = (num_vertices+15) & ~0xf;
  if (num_vertices >= max_vertices)
  {
    int* new_vertices;
    new_vertices = new int [max_vertices+16];
    if (vertices) memcpy (new_vertices, vertices, num_vertices*sizeof (int));
    delete [] vertices;
    vertices = new_vertices;
  }
  vertices[num_vertices++] = v;
}

void csSkeletonLimb::AddChild (csSkeletonLimb* child)
{
  child->SetNext (children);
  children = child;
}

void csSkeletonLimbState::AddChild (csSkeletonLimbState* child)
{
  child->SetNext (children);
  children = child;
}

void csSkeletonLimb::UpdateState (csSkeletonLimbState* limb)
{
  if (name) limb->SetName (name);
  limb->vertices = vertices;
  limb->num_vertices = num_vertices;
  limb->tmpl = this;
  csSkeletonLimb* c = children;
  while (c)
  {
    limb->AddChild (c->CreateState ());
    c = c->GetNext ();
  }
}

void csSkeletonLimb::RemapVertices (int* mapping)
{
  if (num_vertices)
  {
    int i;
    for (i = 0 ; i < num_vertices ; i++)
      vertices[i] = mapping[vertices[i]];
  }

  csSkeletonLimb* c = children;
  while (c)
  {
    c->RemapVertices (mapping);
    c = c->GetNext ();
  }
}

void csSkeletonLimb::ComputeBoundingBox (csPoly3D* source)
{
  if (num_vertices)
  {
    box.StartBoundingBox ((*source)[vertices[0]]);
    int i;
    for (i = 1 ; i < num_vertices ; i++)
    {
      box.AddBoundingVertexSmart ((*source)[vertices[i]]);
    }
  }

  csSkeletonLimb* c = children;
  while (c)
  {
    c->ComputeBoundingBox (source);
    c = c->GetNext ();
  }
}

void csSkeletonLimb::SetName (const char *newname)
{
  if (name) free (name);
  name = strdup (newname);
}

iSkeletonConnection* csSkeletonLimb::CreateConnection ()
{
  csSkeletonConnection* con = new csSkeletonConnection ();
  AddChild (con);
  return QUERY_INTERFACE (con, iSkeletonConnection);
}

csSkeletonLimbState* csSkeletonLimb::CreateState ()
{
  csSkeletonLimbState* limb = new csSkeletonLimbState ();
  UpdateState (limb);
  return limb;
}

csSkeletonLimbState* csSkeletonConnection::CreateState ()
{
  csSkeletonConnectionState* con = new csSkeletonConnectionState ();
  UpdateState ((csSkeletonLimbState*)con);
  con->SetTransformation (trans);
  return (csSkeletonLimbState*)con;
}

csSkeletonLimbState* csSkeleton::CreateState ()
{
  csSkeletonState* skel = new csSkeletonState ();
  UpdateState ((csSkeletonLimbState*)skel);
  return (csSkeletonLimbState*)skel;
}

//---------------------------------------------------------------------------

IMPLEMENT_IBASE_EXT (csSkeletonConnection)
  IMPLEMENTS_EMBEDDED_INTERFACE (iSkeletonConnection)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csSkeletonConnection::SkeletonConnection)
  IMPLEMENTS_INTERFACE (iSkeletonConnection)
IMPLEMENT_EMBEDDED_IBASE_END

csSkeletonConnection::csSkeletonConnection ()
{
  CONSTRUCT_EMBEDDED_IBASE (scfiSkeletonConnection);
}

IMPLEMENT_IBASE_EXT (csSkeleton)
  IMPLEMENTS_EMBEDDED_INTERFACE (iSkeleton)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csSkeleton::Skeleton)
  IMPLEMENTS_INTERFACE (iSkeleton)
IMPLEMENT_EMBEDDED_IBASE_END

csSkeleton::csSkeleton ()
{
CONSTRUCT_EMBEDDED_IBASE (scfiSkeleton);
}

//---------------------------------------------------------------------------

IMPLEMENT_IBASE (csSkeletonLimbState)
  IMPLEMENTS_INTERFACE (iSkeletonLimbState)
IMPLEMENT_IBASE_END

csSkeletonLimbState::csSkeletonLimbState (): 
  next (NULL), vertices (NULL), num_vertices (0), children (NULL),
  data (NULL)
{
  CONSTRUCT_IBASE (NULL);
}

csSkeletonLimbState::~csSkeletonLimbState ()
{
  while (children)
  {
    csSkeletonLimbState* n = children->GetNext ();
    delete children;
    children = n;
  }
}

void csSkeletonLimbState::Transform (const csTransform& tr, csVector3* source,
  csVector3* dest)
{
  csSkeletonLimbState* c = children;
  while (c)
  {
    c->Transform (tr, source, dest);
    c = c->GetNext ();
  }

  int i;
  for (i = 0 ; i < num_vertices ; i++)
    dest [vertices [i]] = tr * source[vertices [i]];
}

void csSkeletonLimbState::SetName (const char *newname)
{
  if (name) free (name);
  name = strdup (newname);
}

void csSkeletonConnectionState::Transform (const csTransform& tr,
  csVector3* source, csVector3* dest)
{
  csTransform tr_new = tr * trans;
  csSkeletonLimbState::Transform (tr_new, source, dest);
}

void csSkeletonLimbState::ComputeBoundingBox (const csTransform& tr,
	csBox3& box)
{
  if (num_vertices)
  {
    csBox3 b;
    tmpl->GetBoundingBox (b);
    box.AddBoundingVertex (tr * b.GetCorner (0));
    box.AddBoundingVertexSmart (tr * b.GetCorner (1));
    box.AddBoundingVertexSmart (tr * b.GetCorner (2));
    box.AddBoundingVertexSmart (tr * b.GetCorner (3));
    box.AddBoundingVertexSmart (tr * b.GetCorner (4));
    box.AddBoundingVertexSmart (tr * b.GetCorner (5));
    box.AddBoundingVertexSmart (tr * b.GetCorner (6));
    box.AddBoundingVertexSmart (tr * b.GetCorner (7));
  }

  csSkeletonLimbState* c = children;
  while (c)
  {
    c->ComputeBoundingBox (tr, box);
    c = c->GetNext ();
  }
}

IMPLEMENT_IBASE_EXT (csSkeletonConnectionState)
  IMPLEMENTS_EMBEDDED_INTERFACE (iSkeletonConnectionState)
  IMPLEMENTS_EMBEDDED_INTERFACE (iSkeletonBone)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csSkeletonConnectionState::SkeletonConnectionState)
  IMPLEMENTS_INTERFACE (iSkeletonConnectionState)
IMPLEMENT_EMBEDDED_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csSkeletonConnectionState::SkeletonBone)
  IMPLEMENTS_INTERFACE (iSkeletonBone)
IMPLEMENT_EMBEDDED_IBASE_END

csSkeletonConnectionState::csSkeletonConnectionState ()
{
  CONSTRUCT_EMBEDDED_IBASE (scfiSkeletonBone);
  CONSTRUCT_EMBEDDED_IBASE (scfiSkeletonConnectionState);
}

void csSkeletonConnectionState::ComputeBoundingBox (const csTransform& tr,
	csBox3& box)
{
  csTransform tr_new = tr * trans;
  csSkeletonLimbState::ComputeBoundingBox (tr_new, box);
}

IMPLEMENT_IBASE_EXT (csSkeletonState)
  IMPLEMENTS_EMBEDDED_INTERFACE (iSkeletonState)
  IMPLEMENTS_EMBEDDED_INTERFACE (iSkeletonBone)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csSkeletonState::SkeletonState)
  IMPLEMENTS_INTERFACE (iSkeletonState)
IMPLEMENT_EMBEDDED_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csSkeletonState::SkeletonBone)
  IMPLEMENTS_INTERFACE (iSkeletonBone)
IMPLEMENT_EMBEDDED_IBASE_END

csSkeletonState::csSkeletonState ()
{
  CONSTRUCT_EMBEDDED_IBASE (scfiSkeletonState);
  CONSTRUCT_EMBEDDED_IBASE (scfiSkeletonBone);
}

void csSkeletonState::ComputeBoundingBox (const csTransform& tr,
	csBox3& box)
{
  box.StartBoundingBox ();
  csSkeletonLimbState::ComputeBoundingBox (tr, box);
}
