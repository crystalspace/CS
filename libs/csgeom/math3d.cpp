/*
    Copyright (C) 1998,1999,2000 by Jorrit Tyberghein
    Largely rewritten by Ivan Avramovic <ivan@avramovic.com>
  
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

#include <math.h>
#include <float.h>
#include "sysdef.h"
#include "qint.h"
#include "csgeom/math3d.h"
#include "csgeom/math3d_d.h"
#include "csgeom/matrix3.h"

//---------------------------------------------------------------------------

bool csMath3::FindIntersection(const csVector3  tri1[3], 
                               const csVector3  tri2[3],
                               csVector3        line[2])
{
  int i,j;
  csVector3 v1[3],v2[3];

  for(i=0;i<3;i++)
  {
    j=(i+1)%3;
    v1[i]=tri1[j]-tri1[i];
    v2[i]=tri2[j]-tri2[i];
  }

  csVector3 n1=v1[0]%v1[1];
  csVector3 n2=v2[0]%v2[1];

  float d1=-n1*tri1[0],d2=-n2*tri2[0];

  csVector3 d=n1%n2;

  int index=0;
  float max=fabs(d.x);
  if(fabs(d.y)>max)
    max=fabs(d.y), index=1;
  if(fabs(d.z)>max)
    max=fabs(d.z), index=2;

  int m1=0,m2=0,n=0;
  float t1[3],t2[3];
  csVector3 p1[2],p2[2];
  p1[0].Set (0, 0, 0);
  p1[1].Set (0, 0, 0);
  p2[0].Set (0, 0, 0);
  p2[1].Set (0, 0, 0);
  float isect1[2],isect2[2],isect[4];
  csVector3 *idx[4];

  for(i=0;i<3;i++)
  {
    float div1=n2*v1[i],div2=n1*v2[i];
    float pr1=-(n2*tri1[i]+d2),pr2=-(n1*tri2[i]+d1);

    if(fabs(div1) < SMALL_EPSILON)
    {
      if(fabs(pr1) < SMALL_EPSILON)
      {
	// line is in the plane of intersection
	t1[i]=0;
      }
      else
      {
	// line is parallel to the plane of
	// intersection, so we don't need it ;)
	t1[i]=15.0;
      }
    }
    else
      t1[i]=pr1/div1;

    if(fabs(div2) < SMALL_EPSILON)
    {
      if(fabs(pr2) < SMALL_EPSILON)
      {
	// line is in the plane of intersection
	t2[i]=0;
      }
      else
      {
	// line is parallel to the plane of
	// intersection, so we don't need it ;)
	t2[i]=15.0;
      }
    }
    else
      t2[i]=pr2/div2;

    if(t1[i]>=0.0&&t1[i]<=1.0&&m1!=2)
    {
      p1[m1]=tri1[i]+v1[i]*t1[i];
      isect1[m1]=p1[m1][index];
      idx[n]=p1+m1;
      isect[n++]=isect1[m1++];
    }
    if(t2[i]>=0.0&&t2[i]<=1.0&&m2!=2)
    {
      p2[m2]=tri2[i]+v2[i]*t2[i];
      isect2[m2]=p2[m2][index];
      idx[n]=p2+m2;
      isect[n++]=isect2[m2++];
    }
  }

  if(n<4)
  {
    // triangles are not intersecting
    return 0;
  }

  for(i=0;i<4;i++)
  {
    for(j=i+1;j<4;j++)
    {
      if(isect[i]>isect[j])
      {
	csVector3 *p=idx[j];
	idx[j]=idx[i];
	idx[i]=p;

	float _=isect[i];
	isect[i]=isect[j];
	isect[j]=_;
      }
    }
  }

  line[0]=*idx[1];
  line[1]=*idx[2];

  return 1;
}

void csMath3::Between (const csVector3& v1, const csVector3& v2,
		       csVector3& v, float pct, float wid)
{
  if (pct != -1)
    pct /= 100.;
  else
  {
    float dist = sqrt((v1-v2)*(v1-v2));
    if (dist == 0) return;
    pct = wid/dist;
  }
  v = v1 + pct*(v2-v1);
}

bool csMath3::Visible (const csVector3& p, const csVector3& t1,
		       const csVector3& t2, const csVector3& t3)
{
   float x1 = t1.x-p.x;
   float y1 = t1.y-p.y;
   float z1 = t1.z-p.z;
   float x2 = t2.x-p.x;
   float y2 = t2.y-p.y;
   float z2 = t2.z-p.z;
   float x3 = t3.x-p.x;
   float y3 = t3.y-p.y;
   float z3 = t3.z-p.z;
   float c = x3*((z1*y2)-(y1*z2))+
             y3*((x1*z2)-(z1*x2))+
             z3*((y1*x2)-(x1*y2));
   return c > 0;
}

bool csMath3::PlanesClose (const csPlane3& p1, const csPlane3& p2)
{
  if (PlanesEqual (p1, p2)) return true;
  csPlane3 p1n = p1; p1n.Normalize ();
  csPlane3 p2n = p2; p2n.Normalize ();
  return PlanesEqual (p1n, p2n);
}

//---------------------------------------------------------------------------

float csSquaredDist::PointLine (const csVector3& p, 
                                const csVector3& l1, const csVector3& l2)
{
  csVector3 W = l1-p;
  csVector3 L = l2-l1;
  csVector3 p2l = W - L * (W*L)/(L*L);
  return p2l * p2l;
}

float csSquaredDist::PointPoly (const csVector3& p, csVector3 *V, int n, 
                                const csPlane3& plane, float sqdist)
{
  csVector3 W, L;
  bool lflag = true, lflag0 = true;
  for (int i=0; i<n-1; i++)
  {
    W = V[i] - p;
    if (i==0)
    {
      if ( !(W*(V[n-1]-V[0]) > 0) ) lflag0 = false;
      else if (W*(V[1]-V[0]) > 0) return W*W;
      else lflag = false;
    }
    else if ( !(W*(L = V[i-1]-V[i]) > 0) )
    {
      if ( !lflag && W*(plane.norm % L) > 0 )
      { 
        L = W - L * (W*L)/(L*L);  
        return L*L; 
      }
      lflag = (W*(V[i+1]-V[i]) > 0);
    }
    else if (W*(V[i+1]-V[i]) > 0) return W*W;
    else lflag = false;
  }

  W = V[n-1] - p;
  if (!lflag)
  {
    lflag = W * (L = V[n-2]-V[n-1]) <= 0;
    if ( lflag && (W*(plane.norm % L) > 0) )
    {
      L = W - L * (W*L)/(L*L);
      return L*L;
    }
  }
  if (!lflag0)
  {
    lflag0 = W * (L = V[0]-V[n-1]) <= 0;
    if ( lflag0 && (W*(plane.norm % L) < 0) )
    {
      L = W - L * (W*L)/(L*L);
      return L*L;
    }
  }

  if (!lflag && !lflag0) return W*W;
  if (sqdist >= 0) return sqdist;
  return csSquaredDist::PointPlane (p,plane);
}

//---------------------------------------------------------------------------

void csIntersect3::Plane(const csVector3& u, const csVector3& v,
                         const csVector3& normal, const csVector3& a,
                         csVector3& isect)
{
  float counter = normal * (u - a);
  float divider = normal * (v - u);
  float dist;
  if (divider == 0) { isect = v; return; }
  dist = counter / divider;
  isect = u + dist*(u - v);
}

bool csIntersect3::Plane(const csVector3& u, const csVector3& v,
                         float A, float B, float C, float D,
                         csVector3& isect, float& dist)
{
  float x,y,z, denom;

  x = v.x-u.x;  y = v.y-u.y;  z = v.z-u.z;
  denom = A*x + B*y + C*z;
  if (ABS (denom) < SMALL_EPSILON) return false; // they are parallel

  dist = -(A*u.x + B*u.y + C*u.z + D) / denom;
  if (dist < -SMALL_EPSILON || dist > 1+SMALL_EPSILON) return false;

  isect.x = u.x + dist*x;  isect.y = u.y + dist*y;  isect.z = u.z + dist*z;
  return true;
}

bool csIntersect3::Plane(const csVector3& u, const csVector3& v,
                         const csPlane3& p, csVector3& isect, float& dist)
{
  float x,y,z, denom;

  x = v.x-u.x;  y = v.y-u.y;  z = v.z-u.z;
  denom = p.norm.x*x + p.norm.y*y + p.norm.z*z;
  if (ABS (denom) < SMALL_EPSILON) return false; // they are parallel

  dist = -(p.norm*u + p.DD) / denom;
  if (dist < -SMALL_EPSILON || dist > 1+SMALL_EPSILON) return false;

  isect.x = u.x + dist*x;  isect.y = u.y + dist*y;  isect.z = u.z + dist*z;
  return true;
}

bool csIntersect3::Planes(const csPlane3& p1, const csPlane3& p2, 
                          const csPlane3& p3, csVector3& isect)
{
  //To find the one point that is on all three planes, we need to solve
  //the following equation system (we need to find the x, y and z which
  //are true for all equations):
  // A1*x+B1*y+C1*z+D1=0 //plane1
  // A2*x+B2*y+C2*z+D2=0 //plane2
  // A3*x+B3*y+C3*z+D3=0 //plane3
  //This can be solved according to Cramers rule by looking at the 
  //determinants of the equation system.
  csMatrix3 mdet(p1.A(), p1.B(), p1.C(),
                 p2.A(), p2.B(), p2.C(),
                 p3.A(), p3.B(), p3.C());
  float det = mdet.Determinant();
  if (det == 0) return false; //some planes are parallel.

  csMatrix3 mx(-p1.D(),  p1.B(),  p1.C(), 
               -p2.D(),  p2.B(),  p2.C(), 
               -p3.D(),  p3.B(),  p3.C());
  float xdet = mx.Determinant();

  csMatrix3 my( p1.A(), -p1.D(),  p1.C(), 
                p2.A(), -p2.D(),  p2.C(), 
                p3.A(), -p3.D(),  p3.C());
  float ydet = my.Determinant();

  csMatrix3 mz( p1.A(),  p1.B(), -p1.D(), 
                p2.A(),  p2.B(), -p2.D(), 
                p3.A(),  p3.B(), -p3.D());
  float zdet = mz.Determinant();

  isect.x = xdet/det;
  isect.y = ydet/det;
  isect.z = zdet/det;
  return true;
}


float csIntersect3::Z0Plane(
  const csVector3& u, const csVector3& v, csVector3& isect)
{
  float r = u.z / (u.z-v.z);
  isect.x = r * (v.x-u.x) + u.x;
  isect.y = r * (v.y-u.y) + u.y;
  isect.z = 0;
  return r;
} 

float csIntersect3::ZPlane(
  float zval, const csVector3& u, const csVector3& v, csVector3& isect)
{
  float r = (zval-u.z) / (v.z-u.z);
  isect.x = r * (v.x-u.x) + u.x;
  isect.y = r * (v.y-u.y) + u.y;
  isect.z = zval;
  return r;
}

float csIntersect3::XFrustrum(
  float A, const csVector3& u, const csVector3& v, csVector3& isect)
{
  float r = (A*u.x+u.z) / ( A*(u.x-v.x) + u.z-v.z );
  isect.x = r * (v.x-u.x) + u.x;
  isect.y = r * (v.y-u.y) + u.y;
  isect.z = r * (v.z-u.z) + u.z;
  return r;
}

float csIntersect3::YFrustrum(
  float B, const csVector3& u, const csVector3& v, csVector3& isect)
{
  float r = (B*u.y+u.z) / ( B*(u.y-v.y) + u.z-v.z );
  isect.x = r * (v.x-u.x) + u.x;
  isect.y = r * (v.y-u.y) + u.y;
  isect.z = r * (v.z-u.z) + u.z;
  return r;
}
