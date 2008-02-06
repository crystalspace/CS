/*
    Copyright (C) 2002 by Jorrit Tyberghein
    Copyright (C) 2002 by Daniel Duhprey

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

#include <cssysdef.h>
#include "csgeom/obb.h"
#include "obb_priv.h"
#include "csgeom/transfrm.h"
#include "csqsqrt.h"

//============================================================================

void csOBBFrozen::Copy (const csOBB& obb, const csReversibleTransform& trans)
{
  csReversibleTransform newtrans;
  csMatrix3 mat_transp = obb.GetMatrix ().GetTranspose ();
  newtrans.SetO2T (trans.GetO2T () * mat_transp);
  newtrans.SetO2TTranslation (obb.GetMatrix () * trans.GetO2TTranslation ());
  for (int i = 0 ; i < 8 ; i++)
  {
    corners[i] = newtrans.Other2This (((csBox3)obb).GetCorner (i));
  }
}

// Version to cope with z <= 0. This is wrong but it in the places where
// it is used below the result is acceptable because it generates a
// conservative result (i.e. a box or outline that is bigger then reality).
static void PerspectiveWrong (const csVector3& v, csVector2& p, float fov,
    	float sx, float sy)
{
  float iz = fov * 10;
  p.x = v.x * iz + sx;
  p.y = v.y * iz + sy;
}

static void Perspective (const csVector3& v, csVector2& p, float fov,
    	float sx, float sy)
{
  float iz = fov / v.z;
  p.x = v.x * iz + sx;
  p.y = v.y * iz + sy;
}

bool csOBBFrozen::ProjectOBB (
	float fov, float sx, float sy, csBox2& sbox,
	float& min_z, float& max_z)
{
  csVector2 v;
  csVector3 p;
  p = GetCorner (0);
  min_z = p.z;
  max_z = p.z;
  if (p.z < .1)
    PerspectiveWrong (p, v, fov, sx, sy);
  else
    Perspective (p, v, fov, sx, sy);
  sbox.StartBoundingBox (v);
  int i;
  for (i = 1; i < 8; i++)
  {
    p = GetCorner (i);
    float z = p.z;
    if (z < min_z) min_z = z;
    else if (z > max_z) max_z = z;
    if (z < .1)
      PerspectiveWrong (p, v, fov, sx, sy);
    else
      Perspective (p, v, fov, sx, sy);
    sbox.AddBoundingVertexSmart (v);
  }

  return max_z >= .01;
}


//=============================================================================

csOBB::csOBB (const csVector3 &dir1, const csVector3 &dir2,
  const csVector3 &dir3)
{
  mMat.m11 = dir1.x; mMat.m12 = dir1.y; mMat.m13 = dir1.z;
  mMat.m21 = dir2.x; mMat.m22 = dir2.y; mMat.m23 = dir2.z;
  mMat.m31 = dir3.x; mMat.m32 = dir3.y; mMat.m33 = dir3.z;
}

void csOBB::AddBoundingVertex (const csVector3 &v)
{
  csBox3::AddBoundingVertex (mMat * v);
}

csVector3 csOBB::GetCorner (int corner) const
{
  return (mMat.GetTranspose() * csBox3::GetCorner (corner));
}

float csOBB::Volume ()
{
  float vol = csBox3::MaxX () - csBox3::MinX ();
  vol *= csBox3::MaxY () - csBox3::MinY ();
  vol *= csBox3::MaxZ () - csBox3::MinZ ();
  return vol;
}

csOBBLine3::csOBBLine3 (const csVector3 &a, const csVector3 &b)
{
  mA = a; mB = b;
  mDir = mA - mB;
  mLength = mDir.SquaredNorm();
  if (ABS (mLength) < 0.0001)
    mDir.Set (1, 0, 0);
  else
    mDir /= csQsqrt (mLength);
}

csOBBTreeNode::csOBBTreeNode (csVector3 **left, csVector3 **right)
{
  CS_ASSERT (left && right);
  mBox.StartBoundingBox ();
  csVector3 **c;
  for (c = left; c <= right; c ++)
  {
    CS_ASSERT (c);
    mBox.AddBoundingVertex (**c);
  }
  mLeftPoint = left; mRightPoint = right;
  mLeft = mRight = 0;
}

csOBBTreeNode::~csOBBTreeNode ()
{
  delete mLeft;
  delete mRight;
}

bool csOBBTreeNode::Split ()
{
  if (mLeftPoint == mRightPoint) { return false; }
  if (mLeft != 0 || mRight != 0) { return true; }
  int dim = 0;
  float max = mBox.MaxX () - mBox.MinX ();
  float d = mBox.MaxY () - mBox.MinY ();
  if (d > max) { max = d; dim = 1; }
  d = mBox.MaxZ () - mBox.MinZ ();
  if (d > max) { max = d; dim = 2; }

  max = (mBox.Max(dim) + mBox.Min(dim)) / 2;
  csVector3 **lr = mLeftPoint, **rl = mRightPoint;
  CS_ASSERT (rl > lr);
  while (lr < rl)
  {
    CS_ASSERT (lr && *lr && rl && *rl);
    if ((**lr)[dim] <= max)
    {
      lr ++;
    }
    else if ((**rl)[dim] > max)
    {
      rl --;
    }
    else
    {
      csVector3 *tmp = *rl;
      *rl = *lr;
      *lr = tmp;
    }
  }
#ifdef CS_DEBUG
  int size = lr - mLeftPoint;
  CS_ASSERT (size > 0 && size <= (mRightPoint - mLeftPoint));
#endif
  lr --;
  mLeft = new csOBBTreeNode (mLeftPoint, lr);
  mRight = new csOBBTreeNode (rl, mRightPoint);
  return true;
}

csOBBTreePair::csOBBTreePair (csOBBTreePairHeap &heap,
	csOBBTreeNode *a, csOBBTreeNode *b)
	: mHeap(heap)
{
  CS_ASSERT (a && b);
  mA = a; mB = b;
  csBox3 box = a->GetBox() + b->GetBox();

  float max = box.MaxX () - box.MinX ();
  mDiameter = max * max;
  int dim = 0;
  float d = box.MaxY () - box.MinY ();
  mDiameter += d * d;
  if (d > max) { max = d; dim = 1; }
  d = box.MaxZ () - box.MinZ ();
  mDiameter += d * d;
  if (d > max) { max = d; dim = 2; }

  csVector3 vmax = a->GetLeftPoint(), vmin = b->GetRightPoint();
  csVector3 **c;
  for (c = a->GetLeftPointRef(); c <= a->GetRightPointRef(); c ++)
  {
    if ((**c)[dim] > vmax[dim])
    {
      vmax = **c;
    }
    if ((**c)[dim] < vmin[dim])
    {
      vmin = **c;
    }
  }
  for (c = b->GetLeftPointRef(); c <= b->GetRightPointRef(); c ++)
  {
    if ((**c)[dim] > vmax[dim])
    {
      vmax = **c;
    }
    if ((**c)[dim] < vmin[dim])
    {
      vmin = **c;
    }
  }
  mPointPair = csOBBLine3 (vmin, vmax);
}

csOBBTreePair::~csOBBTreePair ()
{
}

void csOBBTreePair::MakePair (csOBBTreeNode *l, csOBBTreeNode *r,
	float dist_bound)
{
  CS_ASSERT (l && r);
  csOBBTreePair *n;
  n = new csOBBTreePair (mHeap, l, r);
  if (n->Diameter () > dist_bound)
  {
    mHeap.Push (n);
  }
  else
  {
    delete n;
  }
}

void csOBBTreePair::Split (float dist_bound)
{
  bool leftsplit = mA->Split ();
  bool rightsplit = mB->Split ();
  if (leftsplit && rightsplit)
  {
    MakePair (mA->Left(), mB->Left(), dist_bound);
    MakePair (mA->Right(), mB->Right(), dist_bound);
    MakePair (mA->Left(), mB->Right(), dist_bound);
    MakePair (mA->Right(), mB->Left(), dist_bound);
  }
  else if (leftsplit)
  {
    MakePair (mA->Left(), mB, dist_bound);
    MakePair (mA->Right(), mB, dist_bound);
  }
  else if (rightsplit)
  {
    MakePair (mA, mB->Left(), dist_bound);
    MakePair (mA, mB->Right(), dist_bound);
  }
}

csOBBTreePairHeap::csOBBTreePairHeap ()
{
  mArray = 0;
  mCount = mSize = 0;
}

csOBBTreePairHeap::~csOBBTreePairHeap ()
{
  delete [] mArray;
}

void csOBBTreePairHeap::Push (csOBBTreePair *pair)
{
  CS_ASSERT (pair);
  if (mCount == mSize) { Resize (); }
  CS_ASSERT (mCount < mSize);
  mArray[mCount] = pair;
  int i = mCount;
  int p = (i-1)>>1;
  while (i > 0 && mArray[i]->Diameter() > mArray[p]->Diameter())
  {
    CS_ASSERT (i >= 0 && i < mSize);
    CS_ASSERT (p >= 0 && p < mSize);
    CS_ASSERT (mArray[p] && mArray[i]);
    csOBBTreePair *tmp = mArray[p];
    mArray[p] = mArray[i];
    mArray[i] = tmp;
    i = p;
    p = (i-1)>>1;
  }
  mCount ++;
}

csOBBTreePair *csOBBTreePairHeap::Pop ()
{
  CS_ASSERT (mCount > 0);
  mCount --;
  CS_ASSERT (mCount < mSize);
  CS_ASSERT (mArray[0] && mArray[mCount]);
  csOBBTreePair *res = mArray[0];
  mArray[0] = mArray[mCount];
  mArray[mCount] = 0;
  if (mCount <= 2) { return res; }
  int i = 0, l = 1, r = 2;
  int m = (mArray[l]->Diameter() > mArray[r]->Diameter()) ?  l : r;
  while (i < mCount && m < mCount)
  {
    CS_ASSERT (mArray[i] && mArray[m]);
    if (mArray[i]->Diameter() > mArray[m]->Diameter()) { break; }
    csOBBTreePair *tmp = mArray[m];
    mArray[m] = mArray[i];
    mArray[i] = tmp;
    i = m; l = (i<<1) + 1; r = (i<<1) + 2;
    if (l >= mCount || r >= mCount) { break; }
    CS_ASSERT (mArray[l] && mArray[i]);
    m = (mArray[l]->Diameter() > mArray[r]->Diameter()) ?  l : r;
  }
  return res;
}

void csOBBTreePairHeap::Resize ()
{
  if (mSize == 0)
  {
    mSize ++;
    mArray = new csOBBTreePair*[mSize];
  }
  else
  {
    csOBBTreePair **tmp = mArray;
    mSize <<= 1;
    mArray = new csOBBTreePair*[mSize];
    memcpy (mArray, tmp, (mSize >> 1) * sizeof (csOBBTreePair *));
    delete[] tmp;
  }
}

csOBBTree::csOBBTree (const csVector3 *array, int num)
{
  CS_ASSERT (array != 0 && num > 0);
  mArray = new csVector3 *[num];
  for (int i = 0; i < num; i ++)
  {
    mArray[i] = (csVector3 *)&array[i];
  }
  mRoot = new csOBBTreeNode (&mArray[0], &mArray[num-1]);
}

csOBBTree::~csOBBTree ()
{
  CS_ASSERT (mRoot);
  delete mRoot;
  CS_ASSERT (mArray);
  delete [] mArray;
}

void csOBBTree::Compute (csOBBLine3& res,
	csOBBTreePair *p, float eps)
{
  CS_ASSERT (p);
  res = p->GetPointPair();
  p->Split ((1.0 + eps) * res.Length());
  while (!mHeap.IsEmpty ())
  {
    p = mHeap.Pop ();
    CS_ASSERT (p);
    if (p->GetPointPair().Length() > res.Length())
    {
      res = p->GetPointPair();
    }
    p->Split ((1.0 + eps) * res.Length());
    delete p;
  }
}

void csOBBTree::Diameter (csOBBLine3& line, float eps)
{
  CS_ASSERT (eps >= 0);
  csOBBTreePair *p = new csOBBTreePair (mHeap, mRoot, mRoot);
  Compute (line, p, eps);
  delete p;
}

void csOBB::FindOBB (const csVector3 *vertex_table, int num, float eps)
{
  CS_ASSERT (vertex_table);
  int i;

  csOBBTree *tree = new csOBBTree (vertex_table, num);
  csOBBLine3 l1;
  tree->Diameter (l1, eps);
  csVector3 dir1 = l1.Direction();
  delete tree;

  csVector3 *proj_vt = new csVector3[num];
  for (i = 0; i < num; i ++)
  {
    proj_vt[i] = vertex_table[i] - ((dir1 * vertex_table[i])*dir1);
  }
  tree = new csOBBTree (proj_vt, num);
  csOBBLine3 l2;
  tree->Diameter (l2, eps);
  csVector3 dir2;
  if (ABS (l2.Length()) < 0.0001)
  {
    dir2 = l2.Direction();
    dir2 = dir2 - dir1 * (dir1 * dir2);
    dir2.Normalize();
  }
  else
  {
    dir2 = l2.Direction();
  }
  delete tree;
  delete[] proj_vt;

  csVector3 dir3 = dir1 % dir2;

  csOBB mvbb(dir1, dir2, dir3);
  csOBB aabb;
  mvbb.StartBoundingBox ();
  aabb.StartBoundingBox ();
  for (i = 0; i < num; i ++)
  {
    mvbb.AddBoundingVertex (vertex_table[i]);
    aabb.AddBoundingVertex (vertex_table[i]);
  }

  if (aabb.Volume() < mvbb.Volume())
    *this = aabb;
  else
    *this = mvbb;
}

void csOBB::FindOBBAccurate (const csVector3 *vertex_table, int num)
{
  CS_ASSERT (vertex_table);
  int i, j;

  csVector3 dir1 = vertex_table[num-1] - vertex_table[0];
  float len1 = dir1.Norm();
  for (i = 0; i < num; i ++)
  {
    for (j = i; j < num; j ++)
    {
      csVector3 n = vertex_table[j] - vertex_table[i];
      float nlen = n.Norm();
      if (nlen > len1)
      {
        dir1 = n;
        len1 = nlen;
      }
    }
  }
  dir1.Normalize ();
  csVector3 dir2 =
    (vertex_table[num-1] - ((dir1 * vertex_table[num-1]) * dir1)) -
    (vertex_table[0] - ((dir1 * vertex_table[0]) * dir1));

  float len2 = dir2.Norm();
  for (i = 0; i < num; i ++)
  {
    for (j = i; j < num; j ++)
    {
      csVector3 vi = vertex_table[i];
      vi -= (vi * dir1) * dir1;
      csVector3 vj = vertex_table[j];
      vj -= (vj * dir1) * dir1;
      csVector3 n = vj - vi;
      float nlen = n.Norm();
      if (nlen > len2 + SMALL_EPSILON)
      {
        dir2 = n;
        len2 = nlen;
      }
    }
  }
  dir2.Normalize ();

  csVector3 dir3 = dir1 % dir2;

  csOBB mvbb (dir1, dir2, dir3);
  for (i = 0; i < num; i ++)
    mvbb.AddBoundingVertex (vertex_table[i]);

  *this = mvbb;
}
