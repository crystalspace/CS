/*
    Copyright (C) 2000 by Norman Kramer

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
#include "csutil/dirtyaccessarray.h"
#include "csgeom/crysball.h"
#include "csgeom/frustum.h"

int csCrystalBall::csTriNode::Add (
  csCrystalBallVec *normal,
  size_t tri1,
  size_t tri2,
  size_t tri3,
  csArray<csCrystalBallVec*> *vP,
  csArray<csVector3*> *vTP)
{
  int nPos = 0;
  if (IsLeaf ())
  {
    // we reached the last level
    // if this leaf is empty just add the polyidx
    // if the new normal to be added equals the one in this leaf we are done
    // otherwise we have to split this leaf

    if (len == 0) //leaf is empty
    {
      nPos = vP->Insert (from, normal);
    }
    else
    {
      csCrystalBallVec *n = vP->Get (from);

      if (*n == *normal)
      {
        // just append this index
        nPos = vP->Insert (from, normal);
      }
      else
      {
        // split
        // as the splitpoint we select the middle of old-new (this will
        // ensure, that the old and new point are separated in distinct
        // triangles)
        csVector3 *mc = new csVector3 (*n + (*normal -*n) / 2.0f);
        mc->Normalize ();     // now the point lies on the unit sphere
        divider = vTP->Push (mc);

        bool newStuffed = false, oldStuffed = false;
        size_t tripoint[4] = {tri1, tri2, tri3, tri1};

        // now decide which triangle the old and the new point(s) will go into
        for (int i=0; i<3; i++)
        {
          if (!oldStuffed && Classify (*n, tripoint[i], tripoint[i+1], divider, vTP) == INSIDE)
          {
            // contains the original points
            AddChild (new csTriNode (this, from, len));
            oldStuffed = true;
          }
          else
          {
            if (!newStuffed && Classify (*normal, tripoint[i], tripoint[i+1], divider, vTP) == INSIDE)
            {
              csTriNode *tnode = new csTriNode (this, from + len, 1);
              AddChild (tnode); // contains the new point
              nPos = tnode->Add (normal, tripoint[i], tripoint[i+1], divider, vP, vTP);
              newStuffed = true;
            }
            else
              AddChild (new csTriNode (this, from, 0)); // remains empty
          }
        }
      }
    }
  } // IsLeaf ()
  else
  {
    // ok, this is not a leaf, lets check which subtriangle we have to dive into
    if (Classify (*normal, tri1, tri2, divider, vTP) == INSIDE)
      nPos = ((csTriNode *)children.Get (0))->Add (
          normal,
          tri1,
          tri2,
          divider,
          vP,
          vTP);
    else if (Classify (*normal, tri2, tri3, divider, vTP) == INSIDE)
      nPos = ((csTriNode *)children.Get (1))->Add (
          normal,
          tri2,
          tri3,
          divider,
          vP,
          vTP);
    else
      nPos = ((csTriNode *)children.Get (2))->Add (
          normal,
          tri3,
          tri1,
          divider,
          vP,
          vTP);
  }

  return nPos;
}

void csCrystalBall::csTriNode::Adjust (size_t nPos)
{
  bool bDive = true;
  if (from > nPos)
    from++;
  else if ((from <= nPos && from + len > nPos) || (from == nPos && len == 0))
    len++;
  else
    bDive = false;

  if (bDive && !IsLeaf ())
  {
    ((csTriNode *)children.Get (0))->Adjust (nPos);
    ((csTriNode *)children.Get (1))->Adjust (nPos);
    ((csTriNode *)children.Get (2))->Adjust (nPos);
  }
}

int csCrystalBall::csTriNode::Classify (
  const csVector3 &n,
  size_t i1,
  size_t i2,
  size_t i3,
  const csArray<csVector3*> *vTP) const
{
  csVector3 origo (0, 0, 0);

  csFrustum frust (origo);
  frust.AddVertex (*vTP->Get (i1));
  frust.AddVertex (*vTP->Get (i2));
  frust.AddVertex (*vTP->Get (i3));

  return frust.Contains (
      n) ? csCrystalBall::csTriNode::INSIDE : csCrystalBall::csTriNode::OUTSIDE;
}

void csCrystalBall::csTriNode::Transform (
  const csMatrix3 &m,
  csDirtyAccessArray<int> &indexVector,
  int useSign,
  long cookie,
  const csArray<csCrystalBallVec*> *vP,
  const csArray<csVector3*> *vTP,
  const csVector3 &v1,
  const csVector3 &v2,
  const csVector3 &v3)
{
  // If the node has no children, we can decide based on the normal

  // assigned to this triangle.
  if (IsLeaf ())
  {
    if (len > 0)
    {
      // check the normal directly
      csCrystalBallVec *n = (csCrystalBallVec *)vP->Get (from);
      csVector3 tn = m * (*n);
      if (SignMatches (&tn, useSign))
      {
        // add all polygon indices
        size_t to = from + len;
        size_t i;
        for (i = from; i < to; i++)
          indexVector.Push (
              ((csCrystalBallVec *)vP->Get (i))->GetIndex ());
      }
    }
  }
  else
  {
    const csVector3 *p[4] = { &v1, &v2, &v3, &v1 };
    csVector3 td = m * (*vTP->Get (divider));
    int i;
    for (i = 0; i < 3; i++)
    {
      const csVector3 &n1 = *p[i];
      const csVector3 &n2 = *p[i + 1];
      int match = SignMatches (&n1, &n2, &td, useSign);

      if (match == 0)
      {
        // all in same half of sphere, sp add them all
        csTriNode *tri = (csTriNode *)children.Get (i);
        size_t to = tri->from + tri->len;
        size_t j;
        for (j = tri->from; j < to; j++)
          indexVector.Push (
              ((csCrystalBallVec *)vP->Get (j))->GetIndex ());
      }
      else if (match == 1)
      {
        // triangle is on both sides of sphere, so dive into subtriangles
        Transform (m, indexVector, useSign, cookie, vP, vTP, n1, n2, td);
      }
    }
  }
}

bool csCrystalBall::csTriNode::SignMatches (const csVector3 *tn, int useSign)
{
  bool match = false;
  if (useSign < 0)
  {
    if (tn->z <= 0.0)
      match = true;
    else if (tn->z >= 0.0)
      match = false;
  }
  else
  {
    if (tn->z >= 0.0)
      match = true;
    else if (tn->z <= 0.0)
      match = false;
  }

  return match;
}

int csCrystalBall::csTriNode::SignMatches (
  const csVector3 *n1,
  const csVector3 *n2,
  const csVector3 *td,
  int useSign)
{
  int match;
  if (useSign < 0)
  {
    if (n1->z <= 0.0 && n2->z <= 0.0 && td->z <= 0.0)
      match = 0;
    else if (n1->z >= 0.0 && n2->z >= 0.0 && td->z >= 0.0)
      match = 2;
    else
      match = 1;
  }
  else
  {
    if (n1->z >= 0.0 && n2->z >= 0.0 && td->z >= 0.0)
      match = 0;
    else if (n1->z <= 0.0 && n2->z <= 0.0 && td->z <= 0.0)
      match = 2;
    else
      match = 1;
  }

  return match;
}

csCrystalBall::csCrystalBall ()
{
  // standard axis points from which we'll create the initial 8 triangles
  vTrianglePoints.Push (new csVector3 (1, 0, 0));
  vTrianglePoints.Push (new csVector3 (0, 1, 0));
  vTrianglePoints.Push (new csVector3 (0, 0, 1));
  vTrianglePoints.Push (new csVector3 (-1, 0, 0));
  vTrianglePoints.Push (new csVector3 (0, -1, 0));
  vTrianglePoints.Push (new csVector3 (0, 0, -1));
}

csCrystalBall::~csCrystalBall ()
{
  size_t i;
  for (i = 0; i < vTrianglePoints.Length (); i++)
    delete (csVector3 *)vTrianglePoints.Get (i);
  for (i = 0; i < vPoints.Length (); i++)
    delete (csCrystalBallVec *)vPoints.Get (i);
}

void csCrystalBall::InsertPolygon (iPolygonMesh *polyset, int idx)
{
  csMeshedPolygon &mp = polyset->GetPolygons ()[idx];

  // calc the normal first
  csCrystalBallVec *n = new csCrystalBallVec (idx);
  csMath3::CalcNormal (
      *n,
      polyset->GetVertices ()[mp.vertices[0]],
      polyset->GetVertices ()[mp.vertices[1]],
      polyset->GetVertices ()[mp.vertices[2]]);

  // which of the 8 starting triangles we add this to ?
  int i1 = 0, i2 = 0, i3 = 0, i = 0;
  if (n->y < 0) i += 4; // select one of the octants behind (where y is negative) the xz plane
  if (n->z < 0) i += 2; // select one of the octants behind the xy plane
  if (n->x >= 0) i += 1;

  // determine the indices to the base vectors that make the found octant <i>
  switch (i)
  {
    case 0: i1 = 3; i2 = 1; i2 = 2; break;
    case 1: i1 = 0; i2 = 1; i2 = 2; break;
    case 2: i1 = 3; i2 = 1; i2 = 5; break;
    case 3: i1 = 0; i2 = 1; i2 = 5; break;
    case 4: i1 = 3; i2 = 4; i2 = 2; break;
    case 5: i1 = 0; i2 = 4; i2 = 2; break;
    case 6: i1 = 3; i2 = 4; i2 = 5; break;
    case 7: i1 = 0; i2 = 4; i2 = 5; break;
  }

  // stuff our new normal <n> into the triangle span by these vectors
  int nPos = tri[i].Add (n, i1, i2, i3, &vPoints, &vTrianglePoints);
  for (i = 0; i < 7; i++) tri[i].Adjust (nPos);
}

void csCrystalBall::Build (iPolygonMesh *polyset)
{
  // just cycle through all polygons and add them
  int i;
  for (i = 0; i < polyset->GetPolygonCount (); i++)
    InsertPolygon (polyset, i);
}

void csCrystalBall::Transform (
  const csTransform &o2c,
  csDirtyAccessArray<int> &indexVector,
  int useSign,
  long cookie)
{
  // fill indexVector with the indices to those polygons that have a
  // normal vector with sign(z-coordinate) == <useSign>
  // first check the base triangles
  // for this we simply check the base vectors - luckily the form a unit matrix
  // so we can simply check o2c.
  const csMatrix3 &m = o2c.GetO2T ();
  csVector3 e1 = m.Col1 ();
  csVector3 e2 = m.Col2 ();
  csVector3 e3 = m.Col3 ();
  csVector3 ne1 = -1 * e1;
  csVector3 ne2 = -1 * e2;
  csVector3 ne3 = -1 * e3;

  tri[0].Transform (
      m,
      indexVector,
      useSign,
      cookie,
      &vPoints,
      &vTrianglePoints,
      ne1,
      e2,
      e3);
  tri[1].Transform (
      m,
      indexVector,
      useSign,
      cookie,
      &vPoints,
      &vTrianglePoints,
      e1,
      e2,
      e3);
  tri[2].Transform (
      m,
      indexVector,
      useSign,
      cookie,
      &vPoints,
      &vTrianglePoints,
      ne1,
      e2,
      ne3);
  tri[3].Transform (
      m,
      indexVector,
      useSign,
      cookie,
      &vPoints,
      &vTrianglePoints,
      e1,
      e2,
      ne3);
  tri[4].Transform (
      m,
      indexVector,
      useSign,
      cookie,
      &vPoints,
      &vTrianglePoints,
      ne1,
      ne2,
      e3);
  tri[5].Transform (
      m,
      indexVector,
      useSign,
      cookie,
      &vPoints,
      &vTrianglePoints,
      e1,
      ne2,
      e3);
  tri[6].Transform (
      m,
      indexVector,
      useSign,
      cookie,
      &vPoints,
      &vTrianglePoints,
      ne1,
      ne2,
      ne3);
  tri[7].Transform (
      m,
      indexVector,
      useSign,
      cookie,
      &vPoints,
      &vTrianglePoints,
      e1,
      ne2,
      ne3);
}
