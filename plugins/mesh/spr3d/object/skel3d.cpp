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
#include "spr3d.h"
#include "qsqrt.h"

SCF_IMPLEMENT_IBASE (csSkelLimb)
  SCF_IMPLEMENTS_INTERFACE (iSkeletonLimb)
SCF_IMPLEMENT_IBASE_END

csSkelLimb::csSkelLimb ()
  : next (NULL), vertices (NULL), num_vertices (0), children (NULL), name(NULL)
{
  SCF_CONSTRUCT_IBASE (NULL);
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
  if (name)
    free (name);
}

void csSkelLimb::AddVertex (int v)
{
  // We allocate 16 vertices at a time.
  int max_vertices = (num_vertices+15) & ~0xf;
  if (num_vertices >= max_vertices)
  {
    int* new_vertices;
    new_vertices = new int [max_vertices+16];
    if (vertices)
      memcpy (new_vertices, vertices, num_vertices*sizeof (int));
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
  if (name)
    limb->SetName (name);
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
    for (int i = 0 ; i < num_vertices ; i++)
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
    csVector3 max_sq_radius (0);
    box.StartBoundingBox ((*source)[vertices[0]]);
    for (int i = 1 ; i < num_vertices ; i++)
    {
      const csVector3& v = (*source)[vertices[i]];
      box.AddBoundingVertexSmart (v);
      csVector3 sq_radius (v.x*v.x, v.y*v.y, v.z*v.z);
      if (sq_radius.x > max_sq_radius.x) max_sq_radius.x = sq_radius.x;
      if (sq_radius.y > max_sq_radius.y) max_sq_radius.y = sq_radius.y;
      if (sq_radius.z > max_sq_radius.z) max_sq_radius.z = sq_radius.z;
    }
    radius.Set (
    	qsqrt (max_sq_radius.x),
	qsqrt (max_sq_radius.y),
	qsqrt (max_sq_radius.z));
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
  if (name)
    free (name);
  name = strdup (newname);
}

iSkeletonConnection* csSkelLimb::CreateConnection ()
{
  csSkelConnection* con = new csSkelConnection ();
  AddChild (con);
  iSkeletonConnection* icon = SCF_QUERY_INTERFACE (con, iSkeletonConnection);
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

SCF_IMPLEMENT_IBASE_EXT (csSkelConnection)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iSkeletonConnection)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSkelConnection::SkeletonConnection)
  SCF_IMPLEMENTS_INTERFACE (iSkeletonConnection)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csSkelConnection::csSkelConnection ()
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiSkeletonConnection);
}

SCF_IMPLEMENT_IBASE_EXT (csSkel)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iSkeleton)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSkel::Skeleton)
  SCF_IMPLEMENTS_INTERFACE (iSkeleton)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csSkel::csSkel ()
{
SCF_CONSTRUCT_EMBEDDED_IBASE (scfiSkeleton);
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csSkelLimbState)
  SCF_IMPLEMENTS_INTERFACE (iSkeletonLimbState)
SCF_IMPLEMENT_IBASE_END

csSkelLimbState::csSkelLimbState (): 
  next (NULL), tmpl (NULL), vertices (NULL), num_vertices (0),
  children (NULL), name (NULL), data (NULL)
{
  SCF_CONSTRUCT_IBASE (NULL);
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

  for (int i = 0 ; i < num_vertices ; i++)
    dest [vertices [i]] = tr * source[vertices [i]];
}

void csSkelLimbState::SetName (const char *newname)
{
  if (name)
    free (name);
  name = strdup (newname);
}

void csSkelConnectionState::Transform (const csTransform& tr,
  csVector3* source, csVector3* dest)
{
  csTransform tr_new = tr * trans;
  csSkelLimbState::Transform (tr_new, source, dest);
}

void csSkelLimbState::ComputeBoundingBox (const csTransform& tr,
	csBox3& box, csPoly3D* source)
{
  if (num_vertices)
  {
    if (num_vertices < 10)
    {
      // This is an optimization. If a limb has less than 8 vertices
      // then it is more optimal to compute the transformed bounding
      // box based on vertices than on the precalculated limb bounding
      // box. Not even is it more efficient, it is also more accurate
      // (i.e. bounding box will be closer to reality). For this gain
      // of accuracy we will also use this technique if a limb
      // has slightly more than 8 vertices.
      int i;
      for (i = 0 ; i < num_vertices ; i++)
      {
        const csVector3& v = (*source)[vertices[i]];
        box.AddBoundingVertex (tr * v);
      }
    }
    else
    {
      // The number of vertices in this limb is big enough so that
      // it is better (though less accurate) to just use the bounding
      // box.
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
  }

  csSkelLimbState* c = children;
  while (c)
  {
    c->ComputeBoundingBox (tr, box, source);
    c = c->GetNext ();
  }
}

void csSkelLimbState::ComputeSqRadius (const csTransform& tr,
	csVector3& max_sq_radius, csPoly3D* source)
{
  if (num_vertices)
  {
    // See the comment with ComputeBoundingBox above. The same
    // optimization applies here.
    if (num_vertices < 10)
    {
      int i;
      for (i = 0 ; i < num_vertices ; i++)
      {
        csVector3 v = tr * (*source)[vertices[i]];
	csVector3 sq_radius (v.x*v.x, v.y*v.y, v.z*v.z);
	if (sq_radius.x > max_sq_radius.x) max_sq_radius.x = sq_radius.x;
	if (sq_radius.y > max_sq_radius.y) max_sq_radius.y = sq_radius.y;
	if (sq_radius.z > max_sq_radius.z) max_sq_radius.z = sq_radius.z;
      }
    }
    else
    {
      csVector3 trans_origin = tr * csVector3 (0);
      csVector3 rad;
      tmpl->GetRadius (rad);
      csVector3 sq_radius = trans_origin - rad;
      sq_radius.x = sq_radius.x*sq_radius.x;
      sq_radius.y = sq_radius.y*sq_radius.y;
      sq_radius.z = sq_radius.z*sq_radius.z;
      if (sq_radius.x > max_sq_radius.x) max_sq_radius.x = sq_radius.x;
      if (sq_radius.y > max_sq_radius.y) max_sq_radius.y = sq_radius.y;
      if (sq_radius.z > max_sq_radius.z) max_sq_radius.z = sq_radius.z;
      sq_radius = trans_origin + csVector3 (-rad.x, rad.y, rad.z);
      sq_radius.x = sq_radius.x*sq_radius.x;
      sq_radius.y = sq_radius.y*sq_radius.y;
      sq_radius.z = sq_radius.z*sq_radius.z;
      if (sq_radius.x > max_sq_radius.x) max_sq_radius.x = sq_radius.x;
      if (sq_radius.y > max_sq_radius.y) max_sq_radius.y = sq_radius.y;
      if (sq_radius.z > max_sq_radius.z) max_sq_radius.z = sq_radius.z;
      sq_radius = trans_origin + csVector3 (rad.x, -rad.y, rad.z);
      sq_radius.x = sq_radius.x*sq_radius.x;
      sq_radius.y = sq_radius.y*sq_radius.y;
      sq_radius.z = sq_radius.z*sq_radius.z;
      if (sq_radius.x > max_sq_radius.x) max_sq_radius.x = sq_radius.x;
      if (sq_radius.y > max_sq_radius.y) max_sq_radius.y = sq_radius.y;
      if (sq_radius.z > max_sq_radius.z) max_sq_radius.z = sq_radius.z;
      sq_radius = trans_origin + csVector3 (rad.x, rad.y, -rad.z);
      sq_radius.x = sq_radius.x*sq_radius.x;
      sq_radius.y = sq_radius.y*sq_radius.y;
      sq_radius.z = sq_radius.z*sq_radius.z;
      if (sq_radius.x > max_sq_radius.x) max_sq_radius.x = sq_radius.x;
      if (sq_radius.y > max_sq_radius.y) max_sq_radius.y = sq_radius.y;
      if (sq_radius.z > max_sq_radius.z) max_sq_radius.z = sq_radius.z;
      sq_radius = trans_origin + csVector3 (-rad.x, rad.y, -rad.z);
      sq_radius.x = sq_radius.x*sq_radius.x;
      sq_radius.y = sq_radius.y*sq_radius.y;
      sq_radius.z = sq_radius.z*sq_radius.z;
      if (sq_radius.x > max_sq_radius.x) max_sq_radius.x = sq_radius.x;
      if (sq_radius.y > max_sq_radius.y) max_sq_radius.y = sq_radius.y;
      if (sq_radius.z > max_sq_radius.z) max_sq_radius.z = sq_radius.z;
      sq_radius = trans_origin + csVector3 (-rad.x, -rad.y, rad.z);
      sq_radius.x = sq_radius.x*sq_radius.x;
      sq_radius.y = sq_radius.y*sq_radius.y;
      sq_radius.z = sq_radius.z*sq_radius.z;
      if (sq_radius.x > max_sq_radius.x) max_sq_radius.x = sq_radius.x;
      if (sq_radius.y > max_sq_radius.y) max_sq_radius.y = sq_radius.y;
      if (sq_radius.z > max_sq_radius.z) max_sq_radius.z = sq_radius.z;
      sq_radius = trans_origin + csVector3 (rad.x, -rad.y, -rad.z);
      sq_radius.x = sq_radius.x*sq_radius.x;
      sq_radius.y = sq_radius.y*sq_radius.y;
      sq_radius.z = sq_radius.z*sq_radius.z;
      if (sq_radius.x > max_sq_radius.x) max_sq_radius.x = sq_radius.x;
      if (sq_radius.y > max_sq_radius.y) max_sq_radius.y = sq_radius.y;
      if (sq_radius.z > max_sq_radius.z) max_sq_radius.z = sq_radius.z;
      sq_radius = trans_origin - rad;
      sq_radius.x = sq_radius.x*sq_radius.x;
      sq_radius.y = sq_radius.y*sq_radius.y;
      sq_radius.z = sq_radius.z*sq_radius.z;
      if (sq_radius.x > max_sq_radius.x) max_sq_radius.x = sq_radius.x;
      if (sq_radius.y > max_sq_radius.y) max_sq_radius.y = sq_radius.y;
      if (sq_radius.z > max_sq_radius.z) max_sq_radius.z = sq_radius.z;
    }
  }

  csSkelLimbState* c = children;
  while (c)
  {
    c->ComputeSqRadius (tr, max_sq_radius, source);
    c = c->GetNext ();
  }
}

SCF_IMPLEMENT_IBASE_EXT (csSkelConnectionState)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iSkeletonConnectionState)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iSkeletonBone)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSkelConnectionState::SkeletonConnectionState)
  SCF_IMPLEMENTS_INTERFACE (iSkeletonConnectionState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSkelConnectionState::SkeletonBone)
  SCF_IMPLEMENTS_INTERFACE (iSkeletonBone)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csSkelConnectionState::csSkelConnectionState ()
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiSkeletonBone);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiSkeletonConnectionState);
}

void csSkelConnectionState::ComputeBoundingBox (
  const csTransform& tr, csBox3& box, csPoly3D* source)
{
  csTransform tr_new = tr * trans;
  csSkelLimbState::ComputeBoundingBox (tr_new, box, source);
}

void csSkelConnectionState::ComputeSqRadius (const csTransform& tr,
	csVector3& max_sq_radius, csPoly3D* source)
{
  csTransform tr_new = tr * trans;
  csSkelLimbState::ComputeSqRadius (tr_new, max_sq_radius, source);
}

SCF_IMPLEMENT_IBASE_EXT (csSkelState)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iSkeletonState)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iSkeletonBone)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSkelState::SkeletonState)
  SCF_IMPLEMENTS_INTERFACE (iSkeletonState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSkelState::SkeletonBone)
  SCF_IMPLEMENTS_INTERFACE (iSkeletonBone)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csSkelState::csSkelState ()
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiSkeletonState);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiSkeletonBone);
}

void csSkelState::ComputeBoundingBox (const csTransform& tr,
	csBox3& box, csPoly3D* source)
{
  box.StartBoundingBox ();
  csSkelLimbState::ComputeBoundingBox (tr, box, source);
}

void csSkelState::ComputeSqRadius (const csTransform& tr,
	csVector3& max_sq_radius, csPoly3D* source)
{
  max_sq_radius.Set (0, 0, 0);
  csSkelLimbState::ComputeSqRadius (tr, max_sq_radius, source);
}

