/*
    Copyright (C) 2000 by Norman Kraemer

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

#ifndef __CS_CRYSBALL_H__
#define __CS_CRYSBALL_H__

#include "csextern.h"

#include "csutil/tree.h"
#include "csutil/array.h"
#include "csgeom/vector3.h"
#include "csgeom/math3d.h"
#include "csgeom/transfrm.h"
#include "igeom/polymesh.h"

/**
 * This is an attempt to provide a  massdetection of backfaced polygons.
 * <p>
 * Observation:
 * Mark 3 distinct points on a sphere. Consider the spherical triangle they
 * form . If all 3 points z-coordinates sign equal then all other points
 * z-coordinate inside the triangle have the same sign.
 * <p>
 * It works this way:
 * Precompute the normals of a rigid set of polygons. These normals are unit
 * size and therefor mark a point on the unit sphere. If one rotates the
 * polygonset all points on the unit sphere are rotated by the same amount.
 * <p>
 * To start off, subdivide the sphere into 8 spherical triangles. For every
 * triangle make a list of  points (normals) in it. For every triangle then
 * subdivide further into 3 subtriangles and mark the points contained in it.
 * Repeat this process until every triangle contains only one point (normal).
 * <p>
 * Now to find the backfaced polygons rotate the triangle points on level 0.
 * If a triangles points have all equally signed z-coordinates all polygons
 * belonging to it are either all visible or invisible.
 * If the signs differ then recursively test the subtriangles.
 */
class CS_CRYSTALSPACE_EXPORT csCrystalBall
{
  class csCrystalBallVec : public csVector3
  {
  protected:
    int idx; // holds the index of the polygon
  public:
    csCrystalBallVec (int polyidx) { idx = polyidx; }
    inline int GetIndex () { return idx; }
  };

  class csTriNode : public csTreeNode
  {
  public:
    enum
    {
      INSIDE = 0,
      OUTSIDE = 1
    };

    size_t from, len; // contains <len> points in vPoints starting at <from>
    size_t divider; // index to point that divides this triangle
    csTriNode (csTriNode *theParent=0, size_t from=(size_t)-1, size_t len=0) : csTreeNode (theParent)
      {this->from = from; this->len = len;}

    // find a triangle for <normal> and return the index where its been inserted into vP
    int Add (csCrystalBallVec *normal, size_t tri1, size_t tri2, size_t tri3,
	      csArray<csCrystalBallVec*> *vP, csArray<csVector3*> *vTP);

    // adjust (from,len) pairs after a new point was inserted
    void Adjust (size_t nPos);

    // classify a point to lie inside or outside the spherical triangle
    int Classify (const csVector3 &n, size_t i1, size_t i2, size_t i3,
		  const csArray<csVector3*> *vTP) const;

    // are all 3 normals on the side <useSign>
    // 0 ... yes
    // 1 ... partly
    // 2 ... all on other side
    int SignMatches (const csVector3 *n1, const csVector3 *n2,
		     const csVector3 *td, int useSign);

    // is the normal tn on the <useSign> side ?
    bool SignMatches (const csVector3 *tn, int useSign);

    // rotate the unitsphere by matrix <m>. Add all polygon indices to <indexVector>
    // which normlals point to the <useSign> side
    void Transform (const csMatrix3 &m, csDirtyAccessArray<int> &indexVector,
		    int useSign, long cookie,
		    const csArray<csCrystalBallVec*> *vP,
		    const csArray<csVector3*> *vTP,
		    const csVector3 &v1, const csVector3 &v2,
		    const csVector3 &v3);
  };

 protected:
  // here we store the normals (pointers to csCrystalBallVec)
  csArray<csCrystalBallVec*> vPoints;
  // we divide a triangle into 3 sub triangles by inserting a divider point.
  // and <vTrianglePoints> is the place where we store those points (pointers to csVector3)
  csArray<csVector3*> vTrianglePoints;
  // our crystal ball is initially made of 8 spherical triangles (in the octants of a 3d cartesian coo system)
  csTriNode tri[8];

 public:
  csCrystalBall ();
  ~csCrystalBall ();

  // add all polygons in <polyset> to the crystal ball
  void Build (iPolygonMesh *polyset);

  // add a single polygon to the crystal ball
  void InsertPolygon (iPolygonMesh *polyset, int idx);

  // rotate the unitsphere by <t>. Add all polygon indices to <indexVector>
  // which normlals point to the <useSign> side
  void Transform (const csTransform &t, csDirtyAccessArray<int> &indexVector, int useSign, long cookie);
};

#endif // __CS_CRYSBALL_H__
