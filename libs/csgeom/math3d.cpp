/*
    Copyright (C) 1998 by Jorrit Tyberghein
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
#include "sysdef.h"
#include "qint.h"
#include "csgeom/math3d.h"

//---------------------------------------------------------------------------

float csVector3::Norm () const
{ return sqrt (x*x + y*y + z*z); }

//---------------------------------------------------------------------------

csMatrix3::csMatrix3 () 
{
  m12 = m13 = 0;
  m21 = m23 = 0;
  m31 = m32 = 0;
  m11 = m22 = m33 = 1;
}

csMatrix3::csMatrix3 (float m11, float m12, float m13,
  	    	  float m21, float m22, float m23,
  	   	  float m31, float m32, float m33)
{
  csMatrix3::m11 = m11;
  csMatrix3::m12 = m12;
  csMatrix3::m13 = m13;
  csMatrix3::m21 = m21;
  csMatrix3::m22 = m22;
  csMatrix3::m23 = m23;
  csMatrix3::m31 = m31;
  csMatrix3::m32 = m32;
  csMatrix3::m33 = m33;
}

csMatrix3& csMatrix3::operator+= (const csMatrix3& m)
{
  m11 += m.m11; m12 += m.m12; m13 += m.m13;
  m21 += m.m21; m22 += m.m22; m23 += m.m23;
  m31 += m.m31; m32 += m.m32; m33 += m.m33;
  return *this;
}

csMatrix3& csMatrix3::operator-= (const csMatrix3& m)
{
  m11 -= m.m11; m12 -= m.m12; m13 -= m.m13;
  m21 -= m.m21; m22 -= m.m22; m23 -= m.m23;
  m31 -= m.m31; m32 -= m.m32; m33 -= m.m33;
  return *this;
}

csMatrix3& csMatrix3::operator*= (const csMatrix3& m)
{
  csMatrix3 r;
  r.m11 = m11*m.m11 + m12*m.m21 + m13*m.m31;
  r.m12 = m11*m.m12 + m12*m.m22 + m13*m.m32;
  r.m13 = m11*m.m13 + m12*m.m23 + m13*m.m33;
  r.m21 = m21*m.m11 + m22*m.m21 + m23*m.m31;
  r.m22 = m21*m.m12 + m22*m.m22 + m23*m.m32;
  r.m23 = m21*m.m13 + m22*m.m23 + m23*m.m33;
  r.m31 = m31*m.m11 + m32*m.m21 + m33*m.m31;
  r.m32 = m31*m.m12 + m32*m.m22 + m33*m.m32;
  r.m33 = m31*m.m13 + m32*m.m23 + m33*m.m33;
  *this = r;
  return *this;
}

csMatrix3& csMatrix3::operator*= (float s)
{
  m11 *= s; m12 *= s; m13 *= s;
  m21 *= s; m22 *= s; m23 *= s;
  m31 *= s; m32 *= s; m33 *= s;
  return *this;
}

void csMatrix3::Identity ()
{
  m12 = m13 = 0;
  m21 = m23 = 0;
  m31 = m32 = 0;
  m11 = m22 = m33 = 1;
}

void csMatrix3::Transpose ()
{
  float swap;
  swap = m12; m12 = m21; m21 = swap;
  swap = m13; m13 = m31; m31 = swap;
  swap = m23; m23 = m32; m32 = swap;
}

csMatrix3 csMatrix3::GetTranspose () const
{
  csMatrix3 t;
  t.m12 = m21; t.m21 = m12;
  t.m13 = m31; t.m31 = m13;
  t.m23 = m32; t.m32 = m23;
  t.m11 = m11; t.m22 = m22; t.m33 = m33;
  return t;
}

float csMatrix3::Determinant () const
{
  return
    m11 * (m22*m33 - m23*m32)
   -m12 * (m21*m33 - m23*m31)
   +m13 * (m21*m32 - m22*m31);
}


// this is the matrix for which we seek to find the eigen values.
// vout is the matrix of eigen vectors.
// dout is the vector of dominant eigen values.
// returns:  -1   - error failed to converge within 50 iterations.
//           0-50 - number of iterations.
int csMatrix3::Eigen (csMatrix3* vout, csVector3* dout)
{
#define rotate(a1,a2) g=a1; h=a2; a1=g-s*(h+g*tau); a2=h+s*(g-h*tau);
#define rfabs(x) ((x < 0) ? -x : x)
  int i;
  float tresh,theta,tau,t,sm,s,h,g,c;
  int nrot;

  csMatrix3 v;
  csVector3 z (0, 0, 0);

  // Load b and d with the diagonals of a.
  csVector3 b (m11, m22, m33), d (m11, m22, m33);

  nrot = 0;
  
  // Try up to 50 times.
  for(i=0; i<50; i++)
    {
      // See if bottom half of matrix a is non zero.
      sm=0.0; sm+=fabs(m12); sm+=fabs(m13); sm+=fabs(m23);
      // If it is 0 we are done.  Return the current vector v and d.
      if (sm == 0.0)
	{
	  *vout = v;
	  *dout = d;
	  return i;
	}
      
      if (i < 3) tresh=0.2*sm/(3*3); else tresh=0.0;
      
      // Try rotations in 1st dimension
      {
	g = 100.0*rfabs(m12);  
	// Does this make sense??
	// equiv to   if (i>3 && g == 0) 
	if (i>3 && rfabs(d.x)+g==rfabs(d.x) && rfabs(d.y)+g==rfabs(d.y))
	  m12 =0.0;
	else if (rfabs(m12)>tresh)
	  {
	    h = d.y-d.x;
	    if (rfabs(h)+g == rfabs(h)) t=(m12)/h;
	    else
	      {
		theta=0.5*h/(m12);
		t=1.0/(rfabs(theta)+sqrt(1.0+theta*theta));
		if (theta < 0.0) t = -t;
	      }
	    c=1.0/sqrt(1+t*t); s=t*c; tau=s/(1.0+c); h=t*m12;
	    z.x -= h; z.y += h; d.x -= h; d.y += h;
	    m12=0.0;
	    rotate(m13,m23); rotate(v.m11,v.m12);
            rotate(v.m21,v.m22); rotate(v.m31,v.m32); 
	    nrot++;
	  }
      }

      // Try rotations in the 2nd dimension.
      {
	g = 100.0*rfabs(m13);
	// See above, can be simplified.
	if (i>3 && rfabs(d.x)+g==rfabs(d.x) && rfabs(d.z)+g==rfabs(d.z))
	  m13=0.0;
	else if (rfabs(m13)>tresh)
	  {
	    h = d.z-d.x;
	    if (rfabs(h)+g == rfabs(h)) t=(m13)/h;
	    else
	      {
		theta=0.5*h/(m13);
		t=1.0/(rfabs(theta)+sqrt(1.0+theta*theta));
		if (theta < 0.0) t = -t;
	      }
	    c=1.0/sqrt(1+t*t); s=t*c; tau=s/(1.0+c); h=t*m13;
	    z.x -= h; z.z += h; d.x -= h; d.z += h;
	    m13=0.0;
	    rotate(m12,m23); rotate(v.m11,v.m13);
            rotate(v.m21,v.m23); rotate(v.m31,v.m33); 
	    nrot++;
	  }
      }


      // Try rotations in 3rd dimension.
      {
	g = 100.0*rfabs(m23);
	if (i>3 && rfabs(d.y)+g==rfabs(d.y) && rfabs(d.z)+g==rfabs(d.z))
	  m23=0.0;
	else if (rfabs(m23)>tresh)
	  {
	    h = d.z-d.y;
	    if (rfabs(h)+g == rfabs(h)) t=(m23)/h;
	    else
	      {
		theta=0.5*h/(m23);
		t=1.0/(rfabs(theta)+sqrt(1.0+theta*theta));
		if (theta < 0.0) t = -t;
	      }
	    c=1.0/sqrt(1+t*t); s=t*c; tau=s/(1.0+c); h=t*m23;
	    z.y -= h; z.z += h; d.y -= h; d.z += h;
	    m23=0.0;
	    rotate(m12,m13); rotate(v.m12,v.m13);
            rotate(v.m22,v.m23); rotate(v.m32,v.m33); 
	    nrot++;
	  }
      }

      b = b + z; d = b; z.Set( 0, 0, 0);      
    }

  return -1;
#undef rotate
#undef rfabs
}

// Find eigen vectors of matrix and sort them to have the largest
// eigen vector in the first column.
int csMatrix3::Eigens1 (csMatrix3 *evecs)
{
#define swap(a,b) { float t = a; a = b; b = t; }
  int n;
  csVector3 evals (0, 0, 0);

  n = Eigen (evecs, &evals);

  if (evals.z > evals.x)
    {
      if (evals.z > evals.y)
	{
	  // 3 is largest, swap with column 1
	  swap(evecs->m13,evecs->m11);
	  swap(evecs->m23,evecs->m21);
	  swap(evecs->m33,evecs->m31);
	}
      else
	{
	  // 2 is largest, swap with column 1
	  swap(evecs->m12,evecs->m11);
	  swap(evecs->m22,evecs->m21);
	  swap(evecs->m32,evecs->m31);
	}
    }
  else
    {
      if (evals.x > evals.y)
	{
	  // 1 is largest, do nothing
	}
      else
	{
  	  // 2 is largest
	  swap(evecs->m12,evecs->m11);
	  swap(evecs->m22,evecs->m21);
	  swap(evecs->m32,evecs->m31);
	}
    }
  // we are returning the number of iterations Meigen took.
  // too many iterations means our chosen orientation is bad.
  return n; 
#undef swap
}


csMatrix3 operator+ (const csMatrix3& m1, const csMatrix3& m2) 
{
  return csMatrix3 (m1.m11+m2.m11, m1.m12+m2.m12, m1.m13+m2.m13,
                  m1.m21+m2.m21, m1.m22+m2.m22, m1.m23+m2.m23,
                  m1.m31+m2.m31, m1.m32+m2.m32, m1.m33+m2.m33);
}
                  
csMatrix3 operator- (const csMatrix3& m1, const csMatrix3& m2)
{
  return csMatrix3 (m1.m11-m2.m11, m1.m12-m2.m12, m1.m13-m2.m13,
                  m1.m21-m2.m21, m1.m22-m2.m22, m1.m23-m2.m23,
                  m1.m31-m2.m31, m1.m32-m2.m32, m1.m33-m2.m33);
}
csMatrix3 operator* (const csMatrix3& m1, const csMatrix3& m2)
{
  return csMatrix3 (
   m1.m11*m2.m11 + m1.m12*m2.m21 + m1.m13*m2.m31,
   m1.m11*m2.m12 + m1.m12*m2.m22 + m1.m13*m2.m32,
   m1.m11*m2.m13 + m1.m12*m2.m23 + m1.m13*m2.m33,
   m1.m21*m2.m11 + m1.m22*m2.m21 + m1.m23*m2.m31,
   m1.m21*m2.m12 + m1.m22*m2.m22 + m1.m23*m2.m32,
   m1.m21*m2.m13 + m1.m22*m2.m23 + m1.m23*m2.m33,
   m1.m31*m2.m11 + m1.m32*m2.m21 + m1.m33*m2.m31,
   m1.m31*m2.m12 + m1.m32*m2.m22 + m1.m33*m2.m32,
   m1.m31*m2.m13 + m1.m32*m2.m23 + m1.m33*m2.m33 );
}

csMatrix3 operator* (const csMatrix3& m, float f)
{
  return csMatrix3 (m.m11*f, m.m12*f, m.m13*f,
                  m.m21*f, m.m22*f, m.m23*f,
                  m.m31*f, m.m32*f, m.m33*f);
}

csMatrix3 operator* (float f, const csMatrix3& m)
{
  return csMatrix3 (m.m11*f, m.m12*f, m.m13*f,
                  m.m21*f, m.m22*f, m.m23*f,
                  m.m31*f, m.m32*f, m.m33*f);
}

csMatrix3 operator/ (const csMatrix3& m, float f)
{
  return csMatrix3 (m.m11/f, m.m12/f, m.m13/f,
                  m.m21/f, m.m22/f, m.m23/f,
                  m.m31/f, m.m32/f, m.m33/f);
}

bool operator== (const csMatrix3& m1, const csMatrix3& m2)
{ 
  if (m1.m11 != m2.m11 || m1.m12 != m2.m12 || m1.m13 != m2.m13) return false;
  if (m1.m21 != m2.m21 || m1.m22 != m2.m22 || m1.m23 != m2.m23) return false;
  if (m1.m31 != m2.m31 || m1.m32 != m2.m32 || m1.m33 != m2.m33) return false;
  return true;
}

bool operator!= (const csMatrix3& m1, const csMatrix3& m2)
{
  if (m1.m11 != m2.m11 || m1.m12 != m2.m12 || m1.m13 != m2.m13) return true;
  if (m1.m21 != m2.m21 || m1.m22 != m2.m22 || m1.m23 != m2.m23) return true;
  if (m1.m31 != m2.m31 || m1.m32 != m2.m32 || m1.m33 != m2.m33) return true;
  return false;
}

bool operator< (const csMatrix3& m, float f)
{
  return ABS(m.m11)<f && ABS(m.m12)<f && ABS(m.m13)<f &&
         ABS(m.m21)<f && ABS(m.m22)<f && ABS(m.m23)<f &&
         ABS(m.m31)<f && ABS(m.m32)<f && ABS(m.m33)<f;
}

bool operator> (float f, const csMatrix3& m)
{
  return ABS(m.m11)<f && ABS(m.m12)<f && ABS(m.m13)<f &&
         ABS(m.m21)<f && ABS(m.m22)<f && ABS(m.m23)<f &&
         ABS(m.m31)<f && ABS(m.m32)<f && ABS(m.m33)<f;
}

//---------------------------------------------------------------------------

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

int csMath3::WhichSide3D (const csVector3& p,
			  const csVector3& v1, const csVector3& v2)
{
  float s = p * (v1%v2);
  if (s < 0) return 1;
  else if (s > 0) return -1;
  else return 0;
}

bool csMath3::PlanesClose (const csPlane& p1, const csPlane& p2)
{
  if (PlanesEqual (p1, p2)) return true;
  csPlane p1n = p1; p1n.Normalize ();
  csPlane p2n = p2; p2n.Normalize ();
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
                          const csPlane& plane, float sqdist)
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
                       const csPlane& p,
                       csVector3& isect, float& dist)
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

//---------------------------------------------------------------------------
