/*
    Dynamics/Kinematics modeling and simulation library.
    Copyright (C) 1999 by Michael Alexander Ewert

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

#ifndef CT_SPATIALVECTOR
#define CT_SPATIALVECTOR

#include "csphyzik/ctvector.h"
#include "qsqrt.h"

class ctSpatialVector6;
class ctSpatialMatrix6;

#define ctSpatialVector ctSpatialVector6

class ctVectorTranspose6
{
public:
  ctVectorTranspose6 ()
  {
    elements[0] = elements[1] = elements[2] = 0.0;
    elements[3] = elements[4] = elements[5] = 0.0;
  }

  ctVectorTranspose6 ( real pfirst, real psecond, real pthird,
		       real p2first, real p2second, real p2third )
  {
    elements[0] = pfirst;
    elements[1] = psecond;
    elements[2] = pthird;
    elements[3] = p2first;
    elements[4] = p2second;
    elements[5] = p2third;
  }

  void set ( real pfirst, real psecond, real pthird,
	     real p2first, real p2second, real p2third )
  {
    elements[0] = pfirst;
    elements[1] = psecond;
    elements[2] = pthird;
    elements[3] = p2first;
    elements[4] = p2second;
    elements[5] = p2third;

  }

  void set ( int pnum, real *pele )
  {
	int idx;
    for(idx = 0; idx < pnum; idx++ )
    {
      elements[idx] = *pele;
      pele++;
    }
  }

  void set ( real *pele )
  {
	int idx;
    for(idx = 0; idx < 6; idx++ )
    {
      elements[idx] = *pele;
      pele++;
    }
  }

  real operator[] (const int index) const
  { return elements[index]; }

  real& operator[] (const int index)
  { return elements[index]; }

  ctVectorTranspose6 operator* ( const real pk )
  {
    ctVectorTranspose6 scaled;
	int idx;
    for(idx = 0; idx < 6; idx++)
      scaled.elements[idx] = elements[idx] * pk;
    return scaled;
  }

  void operator*= (const real p)
  { int idx; for (idx=0; idx<6; ++idx) elements[idx] *= p; }

  void operator/= (const real p)
  { int idx; for (idx=0; idx<6; ++idx) elements[idx] /= p; }

  real operator* ( const ctSpatialVector6 &bs );

protected:
  real elements[ 6 ];
};

/// wacky vectors that have certain mathematical properties that allow for an O(n)
/// forward dynamics articulated body solver.
class ctSpatialVector6
{
public:

  ctSpatialVector6 ( const ctVector3 &pa, const ctVector3 &pb )
  {
    elements[0] = pa[0];
    elements[1] = pa[1];
    elements[2] = pa[2];
    elements[3] = pb[0];
    elements[4] = pb[1];
    elements[5] = pb[2];
  }

  ctSpatialVector6 ()
  {
    elements[0] = elements[1] = elements[2] = 0.0;
    elements[3] = elements[4] = elements[5] = 0.0;
  }

  ctSpatialVector6 ( real pone, real ptwo, real pthree,
		     real p2one, real p2two, real p2three )
  {
    elements[0] = pone;
    elements[1] = ptwo;
    elements[2] = pthree;
    elements[3] = p2one;
    elements[4] = p2two;
    elements[5] = p2three;
  }

  real operator[] (const int index) const
  { return elements[index]; }

  real& operator[](const int index)
  { return elements[index]; }

  /// spatial transpose is a special operation.
  ctVectorTranspose6 transpose ()
  {
    ctVectorTranspose6 trans;
    trans[0] = elements[3];
    trans[1] = elements[4];
    trans[2] = elements[5];
    trans[3] = elements[0];
    trans[4] = elements[1];
    trans[5] = elements[2];
    return trans;
  }

  /// do a spatial dot and return scalar
  real spatial_dot( ctSpatialVector6 &pb )
  {
    return ( elements[3]*pb[0] + elements[4]*pb[1] + elements[5]*pb[2] +
	     elements[0]*pb[3] + elements[1]*pb[4] + elements[2]*pb[5] );
  }

  ctVectorTranspose6 operator!()
  { return transpose(); }

  /// angular component.  *Danger* notational collision.
  /// a is NOT linear acceleration.
  void set_a ( const ctVector3 &pa )
  {
    elements[0] = pa[0];
    elements[1] = pa[1];
    elements[2] = pa[2];
  }

  /// linear component
  void set_b ( const ctVector3 &pb )
  {
    elements[3] = pb[0];
    elements[4] = pb[1];
    elements[5] = pb[2];
  }

  /// angular component.
  /// *Danger* notational collision.  a is NOT linear acceleration.
  ctVector3 get_a ()
  { return ctVector3 (elements[0], elements[1], elements[2]); }

  /// linear component
  ctVector3 get_b()
  { return ctVector3 ( elements[3], elements[4], elements[5]); }

  void operator= ( const ctSpatialVector6 &pm )
  {
	int idx;
    for (idx = 0; idx < 6; idx++ )
      elements[idx] = pm[idx];
  }

  /// return length of this vector
  real length ();

  /// return a vector of unit length in same direction as this vector
  ctSpatialVector6 unit ();
  void normalize ();

  /// set all elements to zero
  void zero ()
  { int idx; for(idx = 0; idx < 6; idx++ ) elements[idx] = 0.0; }

  // this = this + x
  void add ( const ctSpatialVector6 & px )
  {
    elements[0] += px.elements[0];
    elements[1] += px.elements[1];
    elements[2] += px.elements[2];
    elements[3] += px.elements[3];
    elements[4] += px.elements[4];
    elements[5] += px.elements[5];
  }

  // this = x + y
  void add2 (const ctSpatialVector6 & px, const ctSpatialVector6 & py)
  {
    elements[0] = px.elements[0] + py.elements[0];
    elements[1] = px.elements[1] + py.elements[1];
    elements[2] = px.elements[2] + py.elements[2];
    elements[3] = px.elements[3] + py.elements[3];
    elements[4] = px.elements[4] + py.elements[4];
    elements[5] = px.elements[5] + py.elements[5];
  }

  // dest = x + y
  void add3 ( ctSpatialVector6 & pdest,
	      const ctSpatialVector6 & px, const ctSpatialVector6 & py )
  {
    pdest.elements[0] = px.elements[0] + py.elements[0];
    pdest.elements[1] = px.elements[1] + py.elements[1];
    pdest.elements[2] = px.elements[2] + py.elements[2];
    pdest.elements[3] = px.elements[3] + py.elements[3];
    pdest.elements[4] = px.elements[4] + py.elements[4];
    pdest.elements[5] = px.elements[5] + py.elements[5];

  }

  void add_scaled ( ctSpatialVector6 & padme, real pk )
  {
    elements[0] += pk*padme.elements[0];
    elements[1] += pk*padme.elements[1];
    elements[2] += pk*padme.elements[2];
    elements[3] += pk*padme.elements[3];
    elements[4] += pk*padme.elements[4];
    elements[5] += pk*padme.elements[5];
  }

  void add_scaled ( real pk, ctSpatialVector6 & padme )
  {
    elements[0] += pk*padme.elements[0];
    elements[1] += pk*padme.elements[1];
    elements[2] += pk*padme.elements[2];
    elements[3] += pk*padme.elements[3];
    elements[4] += pk*padme.elements[4];
    elements[5] += pk*padme.elements[5];
  }

  void operator += (const ctSpatialVector6 & p)
  { int idx; for(idx = 0; idx < 6; idx++ ) elements[idx] += p.elements[idx];  }

  ctSpatialVector6 operator + ( const ctSpatialVector6 & p) const
  {
    ctSpatialVector6 sum;
	int idx;
    for (idx = 0; idx < 6; idx++)
      sum.elements[idx] = elements[idx] + p.elements[idx];
    return sum;
  }

  /// this = this + x
  void subtract ( const ctSpatialVector6 & px )
  { int idx; for(idx = 0; idx < 6; idx++ )  elements[idx] -= px.elements[idx]; }

  /// this = x + y
  void subtract2 ( const ctSpatialVector6 & px, const ctSpatialVector6 & py )
  {
	int idx;
    for(idx = 0; idx < 6; idx++)
      elements[idx] = px.elements[idx] - py.elements[idx];
  }

  /// dest = x + y
  void subtract3 (ctSpatialVector6 & pdest,
		  const ctSpatialVector6 & px, const ctSpatialVector6 & py)
  {
	int idx;
    for (idx = 0; idx < 6; idx++)
      pdest.elements[idx] = px.elements[idx] - py.elements[idx];
  }

  void operator -= (const ctSpatialVector6 & p)
  {
	int idx;
    for (idx = 0; idx < 6; idx++ )
      elements[idx] -= p.elements[idx];
  }

  ctSpatialVector6 operator - (const ctSpatialVector6 & p)
  {
    ctSpatialVector6 sum;
	int idx;
    for (idx = 0; idx < 6; idx++)
      sum.elements[idx] = elements[idx] - p.elements[idx];
    return sum;
  }

  ctSpatialVector6 operator - (const ctSpatialVector6 & p) const
  {
    ctSpatialVector6 sum;
	int idx;
    for (idx = 0; idx < 6; idx++)
      sum.elements[idx] = elements[idx] - p.elements[idx];
    return sum;
  }

  real operator * ( const ctSpatialVector6 & p )
  {
    real dotp = 0.0;
	int idx;
    for (idx = 0; idx < 6; idx++ ) dotp += elements[idx] * p.elements[idx];
    return dotp;
  }

  real operator * ( const ctSpatialVector6 & p ) const
  {
    real dotp = 0.0;
	int idx;
    for (idx = 0; idx < 6; idx++ ) dotp += elements[idx] * p.elements[idx];
    return dotp;
  }

  ctSpatialVector6 operator * ( const real pk )
  {
    ctSpatialVector6 scaled;
	int idx;
    for (idx = 0; idx < 6; idx++)
      scaled.elements[idx] = elements[idx] * pk;
    return scaled;
  }

  ctSpatialVector6 operator * ( const real pk ) const
  {
    ctSpatialVector6 scaled;
	int idx;
    for (idx = 0; idx < 6; idx++)
      scaled.elements[idx] = elements[idx] * pk;
    return scaled;
  }

  ctSpatialVector6 operator / ( const real pk )
  {
    ctSpatialVector6 scaled;
	int idx;
    for(idx = 0; idx < 6; idx++)
      scaled.elements[idx] = elements[idx] / pk;
    return scaled;
  }

  void operator *= (const real p)
  { int idx; for (idx=0; idx<6; ++idx) elements[idx] *= p;}

  void operator /= (const real p)
  { int idx; for (idx=0; idx<6; ++idx) elements[idx] /= p;}

  ctSpatialMatrix6 operator * ( const ctVectorTranspose6 &pvt );

  int get_dimension ()
  { return 6; }

  real *get_elements ()
  { return elements; }

protected:
  real elements[ 6 ];

};

inline real ctSpatialVector6::length ()
{
  return qsqrt (   elements[0] * elements[0]
		+ elements[1] * elements[1]
		+ elements[2] * elements[2]
		+ elements[3] * elements[3]
		+ elements[4] * elements[4]
		+ elements[5] * elements[5] );
}

inline ctSpatialVector6 ctSpatialVector6::unit ()
{
  return ((*this)/this->length() );
}

inline void ctSpatialVector6::normalize()
{
  real len;
  len = this->length ();
  if ( len > MIN_REAL )
    *this /= len;
}

inline real ctVectorTranspose6::operator* ( const ctSpatialVector6 &pv )
{
  real dotp = 0.0;
  int idx;
  for(idx = 0; idx < 6; idx++) dotp += elements[idx] * pv[idx];
  return dotp;
}

#endif
