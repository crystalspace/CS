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
#include "plugins/mesh/object/spr3d/spr3d.h"

IMPLEMENT_IBASE (csSkelLimb)
  IMPLEMENTS_INTERFACE (iSkeletonLimb)
IMPLEMENT_IBASE_END

csSkelLimb::csSkelLimb ()
  : next (NULL), vertices (NULL), num_vertices (0), children (NULL), name(NULL)
{
  CONSTRUCT_IBASE (NULL);
}

csSkelLimb::~csSkelLimb ()
{
  delete [] vertices;
  while (children)
  {
    csSkelLimb* n = children->GetNext ();
    delete children;
    children = n;
  }
  if (name) free (name);
}

void csSkelLimb::AddVertex (int v)
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

void csSkelLimb::AddChild (csSkelLimb* child)
{
  child->SetNext (children);
  children = child;
}

void csSkelLimbState::AddChild (csSkelLimbState* child)
{
  child->SetNext (children);
  children = child;
}

void csSkelLimb::UpdateState (csSkelLimbState* limb)
{
  if (name) limb->SetName (name);
  limb->vertices = vertices;
  limb->num_vertices = num_vertices;
  limb->tmpl = this;
  csSkelLimb* c = children;
  while (c)
  {
    limb->AddChild (c->CreateState ());
    c = c->GetNext ();
  }
}

void csSkelLimb::RemapVertices (int* mapping)
{
  if (num_vertices)
  {
    int i;
    for (i = 0 ; i < num_vertices ; i++)
      vertices[i] = mapping[vertices[i]];
  }

  csSkelLimb* c = children;
  while (c)
  {
    c->RemapVertices (mapping);
    c = c->GetNext ();
  }
}

void csSkelLimb::ComputeBoundingBox (csPoly3D* source)
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

  csSkelLimb* c = children;
  while (c)
  {
    c->ComputeBoundingBox (source);
    c = c->GetNext ();
  }
}

void csSkelLimb::SetName (const char *newname)
{
  if (name) free (name);
  name = strdup (newname);
}

iSkeletonConnection* csSkelLimb::CreateConnection ()
{
  csSkelConnection* con = new csSkelConnection ();
  AddChild (con);
  iSkeletonConnection* icon = QUERY_INTERFACE (con, iSkeletonConnection);
  icon->DecRef ();
  return icon;
}

csSkelLimbState* csSkelLimb::CreateState ()
{
  csSkelLimbState* limb = new csSkelLimbState ();
  UpdateState (limb);
  return limb;
}

csSkelLimbState* csSkelConnection::CreateState ()
{
  csSkelConnectionState* con = new csSkelConnectionState ();
  UpdateState ((csSkelLimbState*)con);
  con->SetTransformation (trans);
  return (csSkelLimbState*)con;
}

csSkelLimbState* csSkel::CreateState ()
{
  csSkelState* skel = new csSkelState ();
  UpdateState ((csSkelLimbState*)skel);
  return (csSkelLimbState*)skel;
}

//---------------------------------------------------------------------------

IMPLEMENT_IBASE_EXT (csSkelConnection)
  IMPLEMENTS_EMBEDDED_INTERFACE (iSkeletonConnection)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csSkelConnection::SkeletonConnection)
  IMPLEMENTS_INTERFACE (iSkeletonConnection)
IMPLEMENT_EMBEDDED_IBASE_END

csSkelConnection::csSkelConnection ()
{
  CONSTRUCT_EMBEDDED_IBASE (scfiSkeletonConnection);
}

IMPLEMENT_IBASE_EXT (csSkel)
  IMPLEMENTS_EMBEDDED_INTERFACE (iSkeleton)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csSkel::Skeleton)
  IMPLEMENTS_INTERFACE (iSkeleton)
IMPLEMENT_EMBEDDED_IBASE_END

csSkel::csSkel ()
{
CONSTRUCT_EMBEDDED_IBASE (scfiSkeleton);
}

//---------------------------------------------------------------------------

IMPLEMENT_IBASE (csSkelLimbState)
  IMPLEMENTS_INTERFACE (iSkeletonLimbState)
IMPLEMENT_IBASE_END

csSkelLimbState::csSkelLimbState (): 
  next (NULL), vertices (NULL), num_vertices (0), children (NULL),
  data (NULL)
{
  CONSTRUCT_IBASE (NULL);
}

csSkelLimbState::~csSkelLimbState ()
{
  while (children)
  {
    csSkelLimbState* n = children->GetNext ();
    delete children;
    children = n;
  }
}

void csSkelLimbState::Transform (const csTransform& tr, csVector3* source,
  csVector3* dest)
{
  csSkelLimbState* c = children;
  while (c)
  {
    c->Transform (tr, source, dest);
    c = c->GetNext ();
  }

  int i;
  for (i = 0 ; i < num_vertices ; i++)
    dest [vertices [i]] = tr * source[vertices [i]];
}

void csSkelLimbState::SetName (const char *newname)
{
  if (name) free (name);
  name = strdup (newname);
}

void csSkelConnectionState::Transform (const csTransform& tr,
  csVector3* source, csVector3* dest)
{
  csTransform tr_new = tr * trans;
  csSkelLimbState::Transform (tr_new, source, dest);
}

void csSkelLimbState::ComputeBoundingBox (const csTransform& tr,
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

  csSkelLimbState* c = children;
  while (c)
  {
    c->ComputeBoundingBox (tr, box);
    c = c->GetNext ();
  }
}

IMPLEMENT_IBASE_EXT (csSkelConnectionState)
  IMPLEMENTS_EMBEDDED_INTERFACE (iSkeletonConnectionState)
  IMPLEMENTS_EMBEDDED_INTERFACE (iSkeletonBone)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csSkelConnectionState::SkeletonConnectionState)
  IMPLEMENTS_INTERFACE (iSkeletonConnectionState)
IMPLEMENT_EMBEDDED_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csSkelConnectionState::SkeletonBone)
  IMPLEMENTS_INTERFACE (iSkeletonBone)
IMPLEMENT_EMBEDDED_IBASE_END

csSkelConnectionState::csSkelConnectionState ()
{
  CONSTRUCT_EMBEDDED_IBASE (scfiSkeletonBone);
  CONSTRUCT_EMBEDDED_IBASE (scfiSkeletonConnectionState);
}

void csSkelConnectionState::ComputeBoundingBox (const csTransform& tr,
	csBox3& box)
{
  csTransform tr_new = tr * trans;
  csSkelLimbState::ComputeBoundingBox (tr_new, box);
}

IMPLEMENT_IBASE_EXT (csSkelState)
  IMPLEMENTS_EMBEDDED_INTERFACE (iSkeletonState)
  IMPLEMENTS_EMBEDDED_INTERFACE (iSkeletonBone)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csSkelState::SkeletonState)
  IMPLEMENTS_INTERFACE (iSkeletonState)
IMPLEMENT_EMBEDDED_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csSkelState::SkeletonBone)
  IMPLEMENTS_INTERFACE (iSkeletonBone)
IMPLEMENT_EMBEDDED_IBASE_END

csSkelState::csSkelState ()
{
  CONSTRUCT_EMBEDDED_IBASE (scfiSkeletonState);
  CONSTRUCT_EMBEDDED_IBASE (scfiSkeletonBone);
}

void csSkelState::ComputeBoundingBox (const csTransform& tr,
	csBox3& box)
{
  box.StartBoundingBox ();
  csSkelLimbState::ComputeBoundingBox (tr, box);
}
