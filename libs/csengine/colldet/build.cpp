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
#include "csgeom/math3d.h"
#include "csengine/collider.h"

/**
 * Located in eigen.cpp, this function returns the eigenvectors of M,
 * Sorted so that the vector corresponding to the largest eigenvalue
 * is first.
 */
int SortedEigen (csMatrix3& M, csMatrix3& evecs);

CD_model::CD_model (int n_triangles)
{
  b = 0;
  num_boxes_alloced = 0;

  CHK (tris = new CDTriangle [n_triangles]);
  num_tris = 0;
  num_tris_alloced = tris ? n_triangles : 0;
}

int CD_model::AddTriangle (int id, const csVector3 &p1, const csVector3 &p2,
  const csVector3 &p3)
{
  // first make sure that we haven't filled up our allocation.
  if (num_tris >= num_tris_alloced)
    return false;

  // now copy the new tri into the array
  tris [num_tris].id = id;
  tris [num_tris].p1 = p1;
  tris [num_tris].p2 = p2;
  tris [num_tris].p3 = p3;

  // update the counter
  num_tris++;

  return true;
}

static CDTriangle *CD_tri = 0;
static BBox *CD_boxes = 0;
static int CD_boxes_inited = 0;

/*
  There are <n> CDTriangle structures in an array starting at <t>.
  
  We are told that the mean point is <mp> and the orientation
  for the parent box will be <or>.  The split axis is to be the 
  vector given by <ax>.

  <or>, <ax>, and <mp> are model space coordinates.
*/
int CD_model::build_hierarchy ()
{
  // allocate the boxes and set the box list globals

  num_boxes_alloced = num_tris * 2;
  CHK (b = new BBox[num_boxes_alloced]);
  if (b == 0) return true;
  CD_boxes = b;
  CD_boxes_inited = 1;   // we are in process of initializing b[0].
  
  // Determine initial orientation, mean point, and splitting axis.

  int i; 
  Accum _M;
  
  CHK (Moment::stack = new Moment[num_tris]);

  if (Moment::stack == 0)
  {
    CHK (delete [] b);
    return true;
  }

  // first collect all the moments, and obtain the area of the 
  // smallest nonzero area triangle.
  float Amin = 0.0;
  int zero = 0;
  int nonzero = 0;
  for (i = 0; i < num_tris; i++)
  {
    Moment::stack [i].compute (tris [i].p1, tris [i].p2, tris [i].p3);
 
    if (Moment::stack[i].A == 0.0)
      zero = 1;
    else
    {
      nonzero = 1;
      if (Amin == 0.0)
        Amin = Moment::stack [i].A;
      else if (Moment::stack [i].A < Amin)
        Amin = Moment::stack [i].A;
    }
  }

  if (zero)
  {
    // if there are any zero area triangles, go back and set their area
    // if ALL the triangles have zero area, then set the area thingy
    // to some arbitrary value. Should never happen.
    if (Amin == 0.0)
      Amin = 1.0;

    for (i = 0; i < num_tris; i++)
      if (Moment::stack [i].A == 0.0)
        Moment::stack [i].A = Amin;
  }

  _M.clear ();

  for (i = 0; i < num_tris; i++)
    _M.moment (Moment::stack [i]);

  // csVector3 _pT;
  csMatrix3 _C;
  _M.mean (&(b [0].pT));

  _M.covariance (&_C);

  SortedEigen(_C, b[0].pR);

  // create the index list
  CHK (int *t = new int [num_tris]);
  if (t == 0)
  {
    CHK (delete [] b);
    CHK (delete [] Moment::stack);
    return true;
  }
  for (i = 0; i < num_tris; i++)
    t [i] = i;

  // set the tri pointer
  CD_tri = tris;
  
  // do the build
  int rc = b [0].split_recurse (t, num_tris);
  if (rc != false)
  {
    CHK (delete [] b);
    CHK (delete [] t);
    return true;
  }
  
  // free the moment list
  CHK (delete [] Moment::stack);
  Moment::stack = 0;

  // null the tri pointer
  CD_tri = 0;
  
  // free the index list
  CHK (delete [] t);

  return false;
}

int BBox::split_recurse (int *t, int n)
{
  // The orientation for the parent box is already assigned to this->pR.
  // The axis along which to split will be column 0 of this->pR.
  // The mean point is passed in on this->pT.

  // When this routine completes, the position and orientation in model
  // space will be established, as well as its dimensions.  Child boxes
  // will be constructed and placed in the parent's CS.

  if (n == 1)
    return split_recurse(t);
  
  // walk along the tris for the box, and do the following:
  //   1. collect the max and min of the vertices along the axes of <or>.
  //   2. decide which group the triangle goes in, performing appropriate swap.
  //   3. accumulate the mean point and covariance data for that triangle.

  Accum _M1, _M2;
  csMatrix3 C;

  int rc;   // for return code on procedure calls.
  int in;
  CDTriangle *ptr;
  int i;
  float axdmp;
  int n1 = 0;  // The number of tris in group 1.  
  // Group 2 will have n - n1 tris.

  // project approximate mean point onto splitting axis, and get coord.
  axdmp = (pR.m11 * pT.x + pR.m21 * pT.y + pR.m31 * pT.z);

  _M1.clear ();
  _M2.clear ();

  csVector3 c = pR.GetTranspose () * CD_tri[t[0]].p1;
  csVector3 minval = c, maxval = c;

  for (i=0 ; i<n ; i++)
  {
    in = t[i];
    ptr = CD_tri + in;

    c = pR.GetTranspose () * ptr->p1;
    csMath3::SetMinMax (c, minval, maxval); 

    c = pR.GetTranspose () * ptr->p2;
    csMath3::SetMinMax (c, minval, maxval); 

    c = pR.GetTranspose () * ptr->p3;
    csMath3::SetMinMax (c, minval, maxval); 

    // grab the mean point of the in'th triangle, project
    // it onto the splitting axis (1st column of pR) and
    // see where it lies with respect to axdmp.
     
    Moment::stack[in].mean (&c);

    if ((( pR.m11 * c.x + pR.m21 * c.y + pR.m31 * c.z) < axdmp)
	  && ((n!=2)) || ((n==2) && (i==0)))    
    {
      // accumulate first and second order moments for group 1
      _M1.moment (Moment::stack[in]);
      // put it in group 1 by swapping t[i] with t[n1]
      int temp = t[i];
      t[i] = t[n1];
      t[n1] = temp;
      n1++;
    }
    else
    {
      // accumulate first and second order moments for group 2
     _M2.moment (Moment::stack[in]);
      // leave it in group 2
      // do nothing...it happens by default
    }
  }

  // done using this->pT as a mean point.

  // error check!
  if ((n1 == 0) || (n1 == n))
  {
    // our partitioning has failed: all the triangles fell into just
    // one of the groups.  So, we arbitrarily partition them into
    // equal parts, and proceed.

    n1 = n/2;
      
    // now recompute accumulated stuff
    _M1.clear ();
    _M2.clear ();
    _M1.moments (t,n1);
    _M2.moments (t+n1,n-n1);
  }

  // With the max and min data, determine the center point and dimensions
  // of the parent box.

  c = (minval + maxval) * 0.5; 

  pT.x = c.x * pR.m11 + c.y * pR.m12 + c.z * pR.m13;
  pT.y = c.x * pR.m21 + c.y * pR.m22 + c.z * pR.m23;
  pT.z = c.x * pR.m31 + c.y * pR.m32 + c.z * pR.m33;

  // delta.
  d = (maxval - minval ) * 0.5;

  // allocate new boxes
  P = CD_boxes + CD_boxes_inited++;
  N = CD_boxes + CD_boxes_inited++;

  // Compute the orientations for the child boxes (eigenvectors of
  // covariance matrix).  Select the direction of maximum spread to be
  // the split axis for each child.
  csMatrix3 tR;
  if (n1 > 1)
  {
    _M1.mean (&P->pT);
    _M1.covariance (&C);

    int nn = SortedEigen(C, tR);
    if ( nn > 30 || nn == -1)
    {
      // unable to find an orientation.  We'll just pick identity.
      tR.Identity ();
    }

    P->pR = tR;
    if ((rc = P->split_recurse (t, n1)) != false)
      return rc;
  }
  else
  {
    if ((rc = P->split_recurse(t)) != false)
      return rc;
  }

  C = P->pR;
  P->pR = pR.GetTranspose () * C;
  c = P->pT - pT;
  P->pT = pR.GetTranspose () * c;

  if ((n-n1) > 1)
  {      
    _M2.mean (&N->pT);
    _M2.covariance (&C);
    int nn = SortedEigen(C, tR);

    if (nn > 30 || nn == -1)
    {
      // unable to find an orientation.  We'll just pick identity.
      tR.Identity ();
    }
      
    N->pR = tR;
    if ((rc = N->split_recurse(t + n1, n - n1)) != false)
      return rc;
  }
  else
  {
    if ((rc = N->split_recurse(t+n1)) != false)
      return rc;
  }

  C = N->pR;
  N->pR = pR.GetTranspose () * C;
 
  c = N->pT - pT;

  N->pT = pR.GetTranspose () * c;

  return false;
}

int BBox::split_recurse (int *t)
{
  // For a single triangle, orientation is easily determined.
  // The major axis is parallel to the longest edge.
  // The minor axis is normal to the triangle.
  // The in-between axis is determine by these two.

  // this->pR, this->d, and this->pT are set herein.

  P = N = 0;
  CDTriangle *ptr = CD_tri + t[0];

  // Find the major axis: parallel to the longest edge.
  // First compute the squared-lengths of each edge

  csVector3 u12 = ptr->p1 - ptr->p2;
  float d12 = u12 * u12;
 
  csVector3 u23 = ptr->p2 - ptr->p3;
  float d23 = u23 * u23;

  csVector3 u31 = ptr->p3 - ptr->p1;
  float d31 = u31 * u31;

  // Find the edge of longest squared-length, normalize it to
  // unit length, and put result into a0.
  csVector3 a0;
  float sv; // Return value of the squaroot.

  if (d12 > d23)
  {
    if (d12 > d31)
    {
      a0 = u12;
      sv = d12;
    }
    else 
    {
      a0 = u31;
      sv = d31;
    }
  }
  else 
  {
    if (d23 > d23)
    {
      a0 = u23;
      sv = d23;
    }
    else
    {
      a0 = u31;
      sv = d31;
    }
  }

  sv = sqrt (sv);
  a0 = a0 / (sv > SMALL_EPSILON ? sv : SMALL_EPSILON);
  // Now compute unit normal to triangle, and put into a2.
  csVector3 a2 = u12 % u23;
  if (a2.Norm () != 0) a2 = csVector3::Unit (a2);

  // a1 is a2 cross a0.
  csVector3 a1 = a2 % a0;
  // Now make the columns of this->pR the vectors a0, a1, and a2.
  pR.m11 = a0.x; pR.m12 = a1.x; pR.m13 = a2.x;
  pR.m21 = a0.y; pR.m22 = a1.y; pR.m23 = a2.y;
  pR.m31 = a0.z; pR.m32 = a1.z; pR.m33 = a2.z;

  // Now compute the maximum and minimum extents of each vertex 
  // along each of the box axes.  From this we will compute the 
  // box center and box dimensions.
  csVector3 c = pR.GetTranspose () * ptr->p1;
  csVector3 minval = c, maxval = c;

  c = pR.GetTranspose () * ptr->p2;
  csMath3::SetMinMax (c, minval, maxval);

  c = pR.GetTranspose () * ptr->p3;
  csMath3::SetMinMax (c, minval, maxval);
 
  // With the max and min data, determine the center point and dimensions
  // of the box
  c = (minval + maxval) * 0.5;

  pT.x = c.x * pR.m11 + c.y * pR.m12 + c.z * pR.m13;
  pT.y = c.x * pR.m21 + c.y * pR.m22 + c.z * pR.m23;
  pT.z = c.x * pR.m31 + c.y * pR.m32 + c.z * pR.m33;


  d = (maxval - minval) * 0.5;

  // Assign the one triangle to this box
  trp = ptr;

  return false;
}
