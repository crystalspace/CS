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

#ifndef CT_SPATIALMATRIX
#define CT_SPATIALMATRIX

#include "csphyzik/ctmatrix.h"
#include "csphyzik/ctvspat.h"

#define ctSpatialMatrix ctSpatialMatrix6


//************   MATRIX6

class ctSpatialMatrix6 : public ctMatrix
{
public:
  ctSpatialMatrix6 ()
  {
    dimen = 6;
	int idx, idy;
    for(idx = 0; idx < 6; idx++)
      for(idy = 0; idy < 6; idy++)
        rows[idx][idy] = ( idx == idy ) ? 1.0 : 0.0;
  }

  /**
   * Spatial transform based on rotation matrix pR and offset vector pr
   * bewteen two frames of reference.  Result will be a matrix that transforms
   * any spatial vector between those two frames
   */
  void form_spatial_transformation ( const ctMatrix3 &pR, const ctVector3 &pr );

  /**
   * Spatial inertia tensor constructed from a rigid bodies inertia tensor
   * and it's mass
   */
  void form_spatial_I ( const ctMatrix3 &pI, real pmass );

  real * operator[] ( const int index )
  { return (real *)(rows[index]); }

  real * operator[] ( const int index ) const
  { return (real *)(rows[index]); } //!me eh?

  void operator=( const ctSpatialMatrix6 &pm )
  {
    int idx, idy;
    for( idx = 0; idx < 6; idx++ )
      for( idy = 0; idy < 6; idy++ )
	rows[idx][idy] = pm[idx][idy];
  }

  void identity ()
  {
	int idx, idy;
    for (idx = 0; idx < 6; idx++)
      for (idy = 0; idy < 6; idy++)
      {
        if( idx == idy )
          rows[idx][idy] = 1.0;
        else
          rows[idx][idy] = 0.0;
      }
  }

  ctSpatialMatrix6 get_transpose () const
  {
    ctSpatialMatrix6 Mret;
	int idx, idy;
    for (idx = 0; idx < 6; idx++)
      for (idy = 0; idy < 6; idy++)
        Mret[idx][idy] = rows[idy][idx];
    return Mret;
  }

  void orthonormalize ();

  void mult_v ( ctSpatialVector6 &pdest, const ctSpatialVector6 &pv )
  {
	int idx, idy;
    for(idx = 0; idx < 6; idx++)
    {
      pdest[idx] = 0;
      for(idy = 0; idy < 6; idy++)
        pdest[idx] += rows[idx][idy]*pv[idy];
    }
  }

  ctSpatialVector6 operator* ( const ctSpatialVector6 &pv )
  {
    ctSpatialVector6 rv (0,0,0,0,0,0);
    int idx, idx2;
    for (idx = 0; idx < 6; idx++)
      for(idx2 = 0; idx2 < 6; idx2++)
        rv[idx] += rows[idx][idx2]*pv[idx2];

    return rv;
  }

  ctSpatialVector6 operator* ( const ctSpatialVector6 &pv ) const
  {
    ctSpatialVector6 rv( 0,0,0,0,0,0);
    int idx, idx2;
    for( idx = 0; idx < 6; idx++ )
      for( idx2 = 0; idx2 < 6; idx2++ )
        rv[idx] += rows[idx][idx2]*pv[idx2];
    return rv;
  }

  ctSpatialMatrix6 operator* ( const ctSpatialMatrix6 &MM ) const
  {
    ctSpatialMatrix6 Mret;
	int idr, idc, adder;
    for(idr = 0; idr < 6; idr++)
      for(idc = 0; idc < 6; idc++)
      {
        Mret[idr][idc] = 0.0;
        for(adder = 0; adder < 6; adder++)
          Mret[idr][idc] += rows[idr][adder]*MM[adder][idc];
      }

    return Mret;
  }

  void mult_M( ctSpatialMatrix6 &Mret, const ctSpatialMatrix6 &MM ) const
  {
	int idr, idc, adder;
    for (idr = 0; idr < 6; idr++)
      for (idc = 0; idc < 6; idc++)
      {
        Mret[idr][idc] = 0.0;
        for(adder = 0; adder < 6; adder++)
          Mret[idr][idc] += rows[idr][adder]*MM[adder][idc];
      }
  }

  void operator*=( const ctSpatialMatrix6 &MM )
  {
    ctSpatialMatrix6 Mret;
	int idr, idc, adder;
    for (idr = 0; idr < 6; idr++)
      for (idc = 0; idc < 6; idc++)
      {
        Mret[idr][idc] = 0.0;
        for (adder = 0; adder < 6; adder++)
          Mret[idr][idc] += rows[idr][adder]*MM[adder][idc];
      }
    *this = Mret;
  }

  ctSpatialMatrix6 operator* ( const real pk ) const
  {
    ctSpatialMatrix6 Mret;
	int idr, idc;
    for (idr = 0; idr < 6; idr++)
      for (idc = 0; idc < 6; idc++)
        Mret[idr][idc] = rows[idr][idc]*pk;
    return Mret;
  }

  void operator*= ( const real pm )
  {
	int idx, idy;
    for (idx = 0; idx < 6; idx++)
      for (idy = 0; idy < 6; idy++)
	rows[idx][idy] *= pm;
  }

  // addition
  void add ( const ctSpatialMatrix6 &pm )
  {
	int idx, idy;
    for (idx = 0; idx < 6; idx++)
      for (idy = 0; idy < 6; idy++)
        rows[idx][idy] += pm.rows[idx][idy];
  }

  void add2 ( const ctSpatialMatrix6 &pm1, const ctSpatialMatrix6 &pm2 )
  {
	int idx, idy;
    for (idx = 0; idx < 6; idx++)
      for (idy = 0; idy < 6; idy++)
        rows[idx][idy] = pm1.rows[idx][idy] + pm2.rows[idx][idy];
  }

  void add3 ( ctSpatialMatrix6 &pmdest,
	      const ctSpatialMatrix6 &pm1, const ctSpatialMatrix6 &pm2 )
  {
	int idx, idy;
    for (idx = 0; idx < 6; idx++)
      for (idy = 0; idy < 6; idy++)
        pmdest.rows[idx][idy] = pm1.rows[idx][idy] + pm2.rows[idx][idy];
  }

  void operator+= ( const ctSpatialMatrix6 &pm )
  {
	int idx, idy;
    for (idx = 0; idx < 6; idx++)
      for (idy = 0; idy < 6; idy++)
        rows[idx][idy] += pm.rows[idx][idy];
  }

  ctSpatialMatrix6 operator+ ( const ctSpatialMatrix6 &pm )
  {
    ctSpatialMatrix6 Mret;
	int idx, idy;
    for (idx = 0; idx < 6; idx++)
      for (idy = 0; idy < 6; idy++)
        Mret.rows[idx][idy] = rows[idx][idy] + pm.rows[idx][idy];

    return Mret;
  }

  // subtraction
  void subtract ( const ctSpatialMatrix6 &pm )
  {
	int idx, idy;
    for (idx = 0; idx < 6; idx++)
      for (idy = 0; idy < 6; idy++)
        rows[idx][idy] -= pm.rows[idx][idy];
  }

  void subtract2 ( const ctSpatialMatrix6 &pm1, const ctSpatialMatrix6 &pm2 )
  {
	int idx, idy;
    for (idx = 0; idx < 6; idx++)
      for (idy = 0; idy < 6; idy++)
        rows[idx][idy] = pm1.rows[idx][idy] - pm2.rows[idx][idy];
  }

  void subtract3 ( ctSpatialMatrix6 &pmdest,
		  const ctSpatialMatrix6 &pm1, const ctSpatialMatrix6 &pm2 )
  {
	int idx, idy;
    for (idx = 0; idx < 6; idx++)
      for (idy = 0; idy < 6; idy++)
        pmdest.rows[idx][idy] = pm1.rows[idx][idy] - pm2.rows[idx][idy];
  }

  void operator-= ( const ctSpatialMatrix6 &pm )
  {
	int idx, idy;
    for (idx = 0; idx < 6; idx++)
      for (idy = 0; idy < 6; idy++)
        rows[idx][idy] -= pm.rows[idx][idy];
  }

  ctSpatialMatrix6 operator- ( ctSpatialMatrix6 &pm )
  {
    ctSpatialMatrix6 Mret;
	int idx, idy;
    for (idx = 0; idx < 6; idx++)
      for (idy = 0; idy < 6; idy++)
        Mret.rows[idx][idy] = rows[idx][idy] - pm.rows[idx][idy];
    return Mret;
  }

  //!me not working????
  // solve the linear system Ax = b where x is an unknown vector
  // b is a known vector and A is this matrix
  // solved x will be returned in px
  void solve ( ctSpatialVector6 &px, const ctSpatialVector6 &pb )
  {
    real **A;
    real *b;
    real *x;
    int idx, idy;

    b = (real *)malloc ( sizeof( real )*6 );
    A = (real **)malloc ( sizeof( real * )*6 );
    x = px.get_elements();

    for(idx = 0; idx < 6; idx++)
    {
      b[idx] = pb[idx];
      A[idx] = (real *)malloc( sizeof( real )*6 );
      for(idy = 0; idy < 6; idy++)
        A[idx][idy] = rows[idx][idy];
    }

    // solve this sucker
    linear_solve( A, 6, x, b );

    free( b );
    free( A );
  }

  void debug_print ()
  {
	int i, j;
    for(i = 0; i < dimen; i++)
    {
      for(j = 0; j < dimen; j++)
      {
//        Debug::logf( CT_DEBUG_LEVEL, "%lf :: ", rows[i][j] );
//      Debug::logf( CT_DEBUG_LEVEL, "\n" );
      }
    }
  }

protected:
  real rows[6][6];

};

inline ctSpatialMatrix6 ctSpatialVector6::operator*
   ( const ctVectorTranspose6 &pv )
{
  ctSpatialMatrix6 Mret;
  int idr, idc;
  for(idr = 0; idr < 6; idr++)
    for(idc = 0; idc < 6; idc++)
      Mret[idr][idc] = elements[idr]*pv[idc];
  return Mret;
}

inline void ctSpatialMatrix::form_spatial_transformation
   ( const ctMatrix3 &pR, const ctVector3 &pr )
{
  // -~rR     ~r is the x-product matrix thing
  rows[3][0]/*M21.m11*/ = pr[2]*pR[1][0] - pr[1] * pR[2][0];
  rows[3][1]/*M21.m12*/ = pr[2]*pR[1][1] - pr[1] * pR[2][1];
  rows[3][2]/*M21.m13*/ = pr[2]*pR[1][2] - pr[1] * pR[2][2];

  rows[4][0]/*M21.m21*/ = - pr[2]*pR[0][0] + pr[0] * pR[2][0];
  rows[4][1]/*M21.m22*/ = - pr[2]*pR[0][1] + pr[0] * pR[2][1];
  rows[4][2]/*M21.m23*/ = - pr[2]*pR[0][2] + pr[0] * pR[2][2];

  rows[5][0]/*M21.m31*/ =  pr[1]*pR[0][0] - pr[0] * pR[1][0];
  rows[5][1]/*M21.m32*/ =  pr[1]*pR[0][1] - pr[0] * pR[1][1];
  rows[5][2]/*M21.m33*/ =  pr[1]*pR[0][2] - pr[0] * pR[1][2];

  int idr, idc;
  for (idr = 0; idr < 3; idr++)
  {
    for(idc = 0; idc < 3; idc++)
    {
      rows[idr][idc] = pR[idr][idc];
      rows[idr+3][idc+3] = pR[idr][idc];
      rows[idr][idc+3] = 0.0;
    }
  }
}

inline void ctSpatialMatrix::form_spatial_I( const ctMatrix3 &pI, real pmass )
{
/*	M11 *= 0;
	M12.Identity();
	M12 *= pmass;
	M21 = pI;
	M22 *= 0;
*/

  int idr, idc;
  for (idr = 0; idr < 3; idr++)
  {
    for (idc = 0; idc < 3; idc++)
    {
      rows[idr][idc] = 0;
      rows[idr][idc+3] = ( idc == idr ) ? pmass : 0.0;
      rows[idr+3][idc] = pI[idr][idc];
      rows[idr+3][idc+3] = 0.0;
    }
  }
}

#endif
