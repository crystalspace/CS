/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#include "sysdef.h"
#include "csengine/sysitf.h"
#include "csengine/skeleton.h"
#include "csengine/cssprite.h"

csSkeletonLimb::~csSkeletonLimb ()
{
  CHK (delete [] vertices);
  while (children)
  {
    csSkeletonLimb* n = children->GetNext ();
    CHK (delete children);
    children = n;
  }
}

void csSkeletonLimb::AddVertex (int v)
{
  // We allocate 16 vertices at a time.
  int max_vertices = (num_vertices+15) & ~0xf;
  if (num_vertices >= max_vertices)
  {
    int* new_vertices;
    CHK (new_vertices = new int [max_vertices+16]);
    if (vertices) memcpy (new_vertices, vertices, num_vertices*sizeof (int));
    CHK (delete [] vertices);
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
  limb->vertices = vertices;
  limb->num_vertices = num_vertices;
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

csSkeletonLimbState* csSkeletonLimb::CreateState ()
{
  CHK (csSkeletonLimbState* limb = new csSkeletonLimbState ());
  UpdateState (limb);
  return limb;
}

csSkeletonLimbState* csSkeletonConnection::CreateState ()
{
  CHK (csSkeletonConnectionState* con = new csSkeletonConnectionState ());
  UpdateState ((csSkeletonLimbState*)con);
  con->SetTransformation (trans);
  return (csSkeletonLimbState*)con;
}

csSkeletonLimbState* csSkeleton::CreateState ()
{
  CHK (csSkeletonState* skel = new csSkeletonState ());
  UpdateState ((csSkeletonLimbState*)skel);
  return (csSkeletonLimbState*)skel;
}

CSOBJTYPE_IMPL(csSkeletonLimbState,csObject);
CSOBJTYPE_IMPL(csSkeletonConnectionState,csSkeletonLimbState);
CSOBJTYPE_IMPL(csSkeletonState,csSkeletonLimbState);

csSkeletonLimbState::~csSkeletonLimbState ()
{
  while (children)
  {
    csSkeletonLimbState* n = children->GetNext ();
    CHK (delete children);
    children = n;
  }
}

void csSkeletonLimbState::Transform (const csTransform& tr, csFrame* source, csVector3* dest)
{
  csSkeletonLimbState* c = children;
  while (c)
  {
    c->Transform (tr, source, dest);
    c = c->GetNext ();
  }
  csVector3* src_verts = source->GetVertices ();
  int i;
  for (i = 0 ; i < num_vertices ; i++)
  {
    dest[vertices[i]] = tr * src_verts[vertices[i]];
  }
}

void csSkeletonConnectionState::Transform (const csTransform& tr, csFrame* source, csVector3* dest)
{
  csTransform tr_new = tr * trans;
  csSkeletonLimbState::Transform (tr_new, source, dest);
}

