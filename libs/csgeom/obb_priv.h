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

#ifndef __CS_OBB_PRIV_H__
#define __CS_OBB_PRIV_H__

#include "csgeom/box.h"
#include "csgeom/matrix3.h"
#include "csgeom/vector3.h"

class csOBBLine3
{
public:
  csVector3 mA, mB;
  float mLength;
  csVector3 mDir;

public:
  csOBBLine3 () { mLength = 0; }
  csOBBLine3 (const csVector3 &a, const csVector3 &b);
  csOBBLine3 (const csVector3 &a, const csVector3 &b, const csVector3 &dir);
  csOBBLine3 (const csOBBLine3 &p)
		: mA (p.mA), mB (p.mB), mLength (p.mLength), mDir (p.mDir) {}
  inline csOBBLine3 &operator=(const csOBBLine3 &p)
  {
    mA = p.mA; mB = p.mB; mLength = p.mLength; mDir = p.mDir;
    return *this;
  }
  float Length () const { return mLength; }
  const csVector3& Direction () const { return mDir; }
};

class csOBBTreeNode
{
private:
  csBox3 mBox;
  csOBBTreeNode *mLeft, *mRight;
  csVector3 **mLeftPoint, **mRightPoint;

public:
  csOBBTreeNode (csVector3 **left, csVector3 **right);
  ~csOBBTreeNode ();

  const csBox3& GetBox () { return mBox; }
  const csVector3 &GetLeftPoint () { return **mLeftPoint; }
  csVector3 **GetLeftPointRef () { return mLeftPoint; }
  const csVector3 &GetRightPoint () { return **mRightPoint; }
  csVector3 **GetRightPointRef () { return mRightPoint; }
  int Size () { return mRightPoint - mLeftPoint + 1; }
  csOBBTreeNode *Left () { return mLeft; }
  csOBBTreeNode *Right() { return mRight; }
  bool Split ();
};

class csOBBTreePairHeap;

class csOBBTreePair
{
private:
  csOBBTreePairHeap &mHeap;
  csOBBTreeNode *mA, *mB;
  csOBBLine3 mPointPair;
  float mDiameter;

public:
  csOBBTreePair (csOBBTreePairHeap &heap, csOBBTreeNode *a, csOBBTreeNode *b);
  ~csOBBTreePair ();
  csOBBTreeNode *A() { return mA; }
  csOBBTreeNode *B() { return mB; }
  float Diameter () { return mDiameter; }

  void MakePair (csOBBTreeNode *, csOBBTreeNode *, float);
  void Split (float);

  const csOBBLine3 &GetPointPair () { return mPointPair; }
};

class csOBBTreePairHeap
{
private:
  void Resize ();

  csOBBTreePair **mArray;
  int mCount, mSize;

public:
  csOBBTreePairHeap ();
  ~csOBBTreePairHeap ();

  bool IsEmpty () { return mCount == 0; }
  int Size () { return mCount; }
  void Push (csOBBTreePair *p);
  csOBBTreePair *Pop ();
};

class csOBBTree
{
private:
  csVector3 **mArray;
  csOBBTreeNode *mRoot;
  csOBBTreePairHeap mHeap;

public:
  csOBBTree(const csVector3 *vertex_table, int num);
  ~csOBBTree ();

  void Compute (csOBBLine3& line, csOBBTreePair *p, float eps);
  void Diameter (csOBBLine3& line, float eps);
};

#endif // __CS_OBB_PRIV_H__

