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

#ifndef __CT_MATRIX__
#define __CT_MATRIX__

#include <stdlib.h>

#include "csphyzik/phyztype.h"
#include "csphyzik/ctvector.h"
#include "csphyzik/mtrxutil.h"
#include "csphyzik/debug.h"

class ctMatrix
{
public:
  int get_dim() { return dimen; }

protected:
  int dimen;


};

// NxN matrix
class ctMatrixN
{
protected:
  ctMatrixN ()
  { rows = NULL; dimen = 0; }

  real **rows;
  int dimen;

public:
  ctMatrixN ( long pdim, real scl = 1.0 )
  {
	int i, j;
    dimen = pdim;
    rows = new real *[pdim];
    for(i = 0; i < pdim; i++ )
    {
      rows[i] = new real[pdim];
      for(j = 0; j < pdim; j++ )
        rows[i][j] = 0.0;
      rows[i][i] = scl;
    }
  }
  
  ctMatrixN ( const ctMatrixN &pM )
  {
    int i,j;
    dimen = pM.dimen;
    rows = new real *[dimen];
    for( i = 0; i < dimen; i++ )
    {
      rows[i] = new real[dimen];
      for( j = 0; j < dimen; j++ )
        rows[i][j] = 0.0;
      rows[i][i] = 1.0;
    }

    for( i = 0; i < dimen; i++ )
      for( j = 0; j < dimen; j++ )
        rows[i][j] = pM.rows[i][j];
  }

  void operator = ( const ctMatrixN &pM )
  {
    int i,j;
    int lowdim;
    if ( pM.dimen < dimen)
      lowdim = pM.dimen;
    else
      lowdim = dimen;

    for( i = 0; i < lowdim; i++ )
      for( j = 0; j < lowdim; j++ )
        rows[i][j] = pM.rows[i][j];
  }

  virtual ~ctMatrixN ()
  {
    for( int i = 0; i < dimen; i++ )
      delete [] (rows[i]);
    delete [] rows;
  }

  real **access_elements ()
  { return rows; }

  void identity ()
  { 
	int i, j;
    for(i = 0; i < dimen; i++ )
    {
      for(j = 0; j < dimen; j++ )
        rows[i][j] = 0.0;
      rows[i][i] = 1.0;
    }
  }

  real *operator[] ( const int index )
  { return rows[index]; }

  real *operator[] ( const int index ) const 
  { return rows[index]; }

  ctMatrixN get_transpose () const 
  {
    ctMatrixN Mret;
	int idx, idy;
    for(idx = 0; idx < dimen; idx++)
      for(idy = 0; idy < dimen; idy++)
        Mret[idx][idy] = rows[idy][idx];
    return Mret;
  }

  void orthonormalize();

  // better be same size vector....
  void mult_v ( real *pdest, const real *pv )
  {
	int idx, idy;
    for(idx = 0; idx < dimen; idx++)
    {
      pdest[idx] = 0;
      for(idy = 0; idy < dimen; idy++)
        pdest[idx] += rows[idx][idy]*pv[idy];
    }
  }

  ctMatrixN operator* ( const ctMatrixN &MM ) const 
  {
    ctMatrixN Mret;
	int idr, idc, adder;
    for(idr = 0; idr < dimen; idr++)
      for(idc = 0; idc < dimen; idc++)
      {
        Mret[idr][idc] = 0.0;
        for(adder = 0; adder < dimen; adder++)
          Mret[idr][idc] += rows[idr][adder]*(MM[adder][idc]);
      }

    return Mret;
  }

  ctMatrixN operator* ( const real pk ) const 
  {
    ctMatrixN Mret;
	int idr, idc;
    for(idr = 0; idr < dimen; idr++)
      for(idc = 0; idc < dimen; idc++)
        Mret[idr][idc] = rows[idr][idc]*pk;
    return Mret;
  }

  void operator*= ( const real pm )
  {
	int idx, idy;
    for(idx = 0; idx < dimen; idx++)
      for(idy = 0; idy < dimen; idy++)
	rows[idx][idy] *= pm;
  }

  // addition
  void add ( const ctMatrixN &pm )
  {
	int idx, idy;
    for(idx = 0; idx < dimen; idx++)
      for(idy = 0; idy < dimen; idy++)
        rows[idx][idy] += pm.rows[idx][idy];
  }

  void add2 ( const ctMatrixN &pm1, const ctMatrixN &pm2 )
  {
	int idx, idy;
    for(idx = 0; idx < dimen; idx++)
      for(idy = 0; idy < dimen; idy++)
        rows[idx][idy] = pm1.rows[idx][idy] + pm2.rows[idx][idy];
  }

  void add3 ( ctMatrixN &pmdest, const ctMatrixN &pm1, const ctMatrixN &pm2 )
  {
    for( int idx = 0; idx < dimen; idx++ )
      for( int idy = 0; idy < dimen; idy++ )
        pmdest.rows[idx][idy] = pm1.rows[idx][idy] + pm2.rows[idx][idy];
  }

  void operator+=( const ctMatrixN &pm )
  {
	int idx, idy;
    for(idx = 0; idx < dimen; idx++)
      for(idy = 0; idy < dimen; idy++)
        rows[idx][idy] += pm.rows[idx][idy];
  }

  ctMatrixN operator+( const ctMatrixN &pm )
  {
    ctMatrixN Mret;

	int idx, idy;
    for(idx = 0; idx < dimen; idx++)
      for(idy = 0; idy < dimen; idy++)
        Mret.rows[idx][idy] = rows[idx][idy] + pm.rows[idx][idy];

    return Mret;
  }

  // subtraction
  void subtract ( const ctMatrixN &pm )
  {
	int idx, idy;
    for(idx = 0; idx < dimen; idx++)
      for(idy = 0; idy < dimen; idy++)
        rows[idx][idy] -= pm.rows[idx][idy];
  }

  void subtract2 ( const ctMatrixN &pm1, const ctMatrixN &pm2 )
  {
	int idx, idy;
    for(idx = 0; idx < dimen; idx++)
      for(idy = 0; idy < dimen; idy++)
        rows[idx][idy] = pm1.rows[idx][idy] - pm2.rows[idx][idy];
  }

  void subtract3 ( ctMatrixN &pmdest, const ctMatrixN &pm1, const ctMatrixN &pm2 )
  {
	int idx, idy;
    for(idx = 0; idx < dimen; idx++)
      for(idy = 0; idy < dimen; idy++)
        pmdest.rows[idx][idy] = pm1.rows[idx][idy] - pm2.rows[idx][idy];
  }

  void operator-= ( const ctMatrixN &pm )
  {
	int idx, idy;
    for(idx = 0; idx < dimen; idx++)
      for(idy = 0; idy < dimen; idy++)
        rows[idx][idy] -= pm.rows[idx][idy];
  }

  ctMatrixN operator- ( ctMatrixN &pm )
  {
    ctMatrixN Mret;

	int idx, idy;
    for(idx = 0; idx < dimen; idx++)
      for(idy = 0; idy < dimen; idy++)
        Mret.rows[idx][idy] = rows[idx][idy] - pm.rows[idx][idy];

    return Mret;
  }

  /**
   * Solve the linear system Ax = b where x is an unknown vector
   * b is a known vector and A is this matrix
   * solved x will be returned in px
   */
  void solve( real *px, const real *pb )
  {
    real *x;
    real *b;
    int idx;  
    b = (real *)malloc( sizeof( real )*dimen );
    x = px;

    for( idx = 0; idx < dimen; idx++ )
      b[idx] = pb[idx];

    // solve this sucker
    linear_solve ( rows, dimen, x, b );
    free(b);
  }

  void debug_print()
  {
	int i, j;
    for(i = 0; i < dimen; i++)
    {
      for(j = 0; j < dimen; j++)
      {
        logf( "%f :: ", rows[i][j] ); 
      }
      logf( "\n" );
    }
  }
};


class ctMatrix3 : public ctMatrix
{
protected:
  real rows[3][3];

  real cofactor(int i, int j) 
  {
    real sign = ((i + j) % 2) ? -1 : 1;

    // Set which rows/columns the cofactor will use
    int r1, r2, c1, c2;
    r1 = (i == 0) ? 1 : 0;
    r2 = (i == 2) ? 1 : 2;
    c1 = (j == 0) ? 1 : 0;
    c2 = (j == 2) ? 1 : 2;

    real C = rows[r1][c1] * rows[r2][c2] - rows[r2][c1] * rows[r1][c2];
    return sign * C;
  }

  real determinant() 
  {
    return rows[0][0]*rows[1][1]*rows[2][2] + rows[0][1]*rows[1][2]*rows[2][0]
      + rows[0][2]*rows[1][0]*rows[2][1] - rows[0][0]*rows[1][2]*rows[2][1]
      - rows[0][1]*rows[1][0]*rows[2][2] - rows[0][2]*rows[1][1]*rows[2][0];
  }

public:

  ctMatrix3( real scl = 1.0 )
  {
    dimen = 3;
    rows[0][0] = rows[1][1] = rows[2][2] = scl;
    rows[0][1] = rows[0][2] = rows[1][0] = 0.0;
    rows[1][2] = rows[2][0] = rows[2][1] = 0.0;
  }

  ctMatrix3( real p00, real p01, real p02,
    real p10, real p11, real p12,
    real p20, real p21, real p22 )
  {
    rows[0][0] = p00; 
    rows[0][1] = p01; 
    rows[0][2] = p02;
    rows[1][0] = p10; 
    rows[1][1] = p11; 
    rows[1][2] = p12;
    rows[2][0] = p20; 
    rows[2][1] = p21; 
    rows[2][2] = p22;
  }

  void set( real p00, real p01, real p02,
	    real p10, real p11, real p12,
	    real p20, real p21, real p22 );

  void identity()
  { 
    rows[0][0] = rows[1][1] = rows[2][2] = 1.0;
    rows[0][1] = rows[0][2] = rows[1][0] = 0.0;
    rows[1][2] = rows[2][0] = rows[2][1] = 0.0;
  }

  real *operator[]( const int index )
  { return (real *)(rows[index]); }

  real *operator[]( const int index ) const 
  { return (real *)(rows[index]); }

  ctMatrix3 get_transpose () const 
  {
    ctMatrix3 Mret;
	int idx, idy;
    for (idx = 0; idx < 3; idx++)
      for (idy = 0; idy < 3; idy++)
        Mret[idx][idy] = (*this)[idy][idx];
    return Mret;
  }

  void put_transpose( ctMatrix3 &Mret ) const 
  {
	int idx, idy;
    for (idx = 0; idx < 3; idx++)
      for (idy = 0; idy < 3; idy++)
        Mret[idx][idy] = (*this)[idy][idx];
  }

  /// I think that's what this is called... M * A * M_transpose
  void similarity_transform( ctMatrix3 &Mret, const ctMatrix3 &pA ) const 
  {
	int idr, idc, adder;
    for (idr = 0; idr < 3; idr++)
      for (idc = 0; idc < 3; idc++)
      {
        Mret[idr][idc] = 0.0;
        for (adder = 0; adder < 3; adder++)
          Mret[idr][idc] += ( rows[idr][0]*pA[0][adder] + 
                              rows[idr][1]*pA[1][adder] + 
                              rows[idr][2]*pA[2][adder] ) *
                              rows[idc][adder];
      }
  }

  void orthonormalize ();

  void mult_v ( ctVector3 &pdest, const ctVector3 &pv )
  {
	int idx, idy;
    for (idx = 0; idx < 3; idx++)
    {
      pdest[idx] = 0;
      for (idy = 0; idy < 3; idy++)
        pdest[idx] += rows[idx][idy]*pv[idy];
    }
  }

  ctVector3 operator* ( const ctVector3 &pv ) 
  {
    ctVector3 rv(0,0,0);
    int idx, idx2;
    for ( idx = 0; idx < 3; idx++ )
      for ( idx2 = 0; idx2 < 3; idx2++ )
        rv[idx] += rows[idx][idx2]*pv[idx2];
    return rv;
  }

  ctVector3 operator* ( const ctVector3 &pv ) const 
  {
    ctVector3 rv( 0,0,0);
    int idx, idx2;
    for ( idx = 0; idx < 3; idx++ )
      for ( idx2 = 0; idx2 < 3; idx2++ )
        rv[idx] += rows[idx][idx2]*pv[idx2];
    return rv;
  }

  ctMatrix3 operator* ( const ctMatrix3 &MM ) const 
  {
    ctMatrix3 Mret;
	int idr, idc, adder;
    for (idr = 0; idr < 3; idr++)
      for (idc = 0; idc < 3; idc++ )
      {
        Mret[idr][idc] = 0.0;
        for (adder = 0; adder < 3; adder++)
          Mret[idr][idc] += (*this)[idr][adder]*MM[adder][idc];
      }

    return Mret;
  }

  ctMatrix3 operator* ( const real pk ) const 
  {
    ctMatrix3 Mret;
	int idr, idc;
    for (idr = 0; idr < 3; idr++)
      for ( idc = 0; idc < 3; idc++)
        Mret[idr][idc] = (*this)[idr][idc]*pk;

    return Mret;
  }

  void operator*= ( const real pm )
  {
	int idx, idy;
    for (idx = 0; idx < 3; idx++)
      for (idy = 0; idy < 3; idy++)
	rows[idx][idy] *= pm;
  }

  // addition
  void add ( const ctMatrix3 &pm )
  {
	int idx, idy;
    for (idx = 0; idx < 3; idx++)
      for (idy = 0; idy < 3; idy++)
        (*this)[idx][idy] += pm[idx][idy];
  }

  void add2 ( const ctMatrix3 &pm1, const ctMatrix3 &pm2 )
  {
	int idx, idy;
    for (idx = 0; idx < 3; idx++)
      for (idy = 0; idy < 3; idy++)
        (*this)[idx][idy] = pm1[idx][idy] + pm2[idx][idy];
  }

  void add3 ( ctMatrix3 &pmdest, const ctMatrix3 &pm1, const ctMatrix3 &pm2 )
  {
	int idx, idy;
    for (idx = 0; idx < 3; idx++)
      for (idy = 0; idy < 3; idy++)
        pmdest[idx][idy] = pm1[idx][idy] + pm2[idx][idy];
  }

  void operator+= ( const ctMatrix3 &pm )
  {
	int idx, idy;
    for (idx = 0; idx < 3; idx++)
      for (idy = 0; idy < 3; idy++)
        (*this)[idx][idy] += pm[idx][idy];
  }

  ctMatrix3 operator+ ( const ctMatrix3 &pm )
  {
    ctMatrix3 Mret;

	int idx, idy;
    for (idx = 0; idx < 3; idx++)
      for (idy = 0; idy < 3; idy++)
        Mret[idx][idy] = (*this)[idx][idy] + pm[idx][idy];

    return Mret;
  }

  // subtraction
  void subtract ( const ctMatrix3 &pm )
  {
	int idx, idy;
    for (idx = 0; idx < 3; idx++ )
      for (idy = 0; idy < 3; idy++ )
        (*this)[idx][idy] -= pm[idx][idy];
  }

  void subtract2 ( const ctMatrix3 &pm1, const ctMatrix3 &pm2 )
  {
	int idx, idy;
    for (idx = 0; idx < 3; idx++)
      for (idy = 0; idy < 3; idy++)
        (*this)[idx][idy] = pm1[idx][idy] - pm2[idx][idy];
  }

  void subtract3 ( ctMatrix3 &pmdest, const ctMatrix3 &pm1, const ctMatrix3 &pm2 )
  {
	int idx, idy;
    for (idx = 0; idx < 3; idx++)
      for (idy = 0; idy < 3; idy++)
        pmdest[idx][idy] = pm1[idx][idy] - pm2[idx][idy];
  }

  void operator-=( const ctMatrix3 &pm )
  {
	int idx, idy;
    for (idx = 0; idx < 3; idx++)
      for (idy = 0; idy < 3; idy++)
        (*this)[idx][idy] -= pm[idx][idy];
  }

  ctMatrix3 operator-( ctMatrix3 &pm )
  {
    ctMatrix3 Mret;

	int idx, idy;
    for (idx = 0; idx < 3; idx++)
      for (idy = 0; idy < 3; idy++)
        Mret[idx][idy] = (*this)[idx][idy] - pm[idx][idy];

    return Mret;
  }

  // solve the linear system Ax = b where x is an unknown vector
  // b is a known vector and A is this matrix
  // solved x will be returned in px
  void solve ( ctVector3 &px, const ctVector3 &pb )
  {
    real **A;
    real *b;
    real *x;
    int idx, idy;
    
    b = (real *)malloc( sizeof( real )*3 );
    A = (real **)malloc( sizeof( real * )*3 );
//    x = px.get_elements();
    x = (real *)malloc( sizeof( real )*3 );
    x[0] = px[0]; x[1] = px[1]; x[2] = px[2];

    for(idx = 0; idx < 3; idx++)
    {
      b[idx] = pb[idx];
      A[idx] = (real *)malloc( sizeof( real )*3 );
      for(idy = 0; idy < 3; idy++)
        A[idx][idy] = (*this)[idx][idy];
    }

    // solve this sucker
    linear_solve ( A, 3, x, b );

    px[0] = x[0]; px[1] = x[1]; px[2] = x[2];
    free( x );
    free( b );
    free( A );
  }

  ctMatrix3 inverse () 
  {
    ctMatrix3 inv;
    real det = determinant();

    int i,j;
    for(i = 0; i < 3; i++) 
      for(j = 0; j < 3; j++)
	inv[i][j] = cofactor (j,i) / det;

    return inv;
  }

  void debug_print ()
  {
	int i, j;
    for (i = 0; i < dimen; i++)
    {
      for (j = 0; j < dimen; j++)
        DEBUGLOGF( "%f :: ", rows[i][j] ); 
      DEBUGLOG( "\n" );
    }
  }
};


inline void ctMatrix3::set ( real p00, real p01, real p02,
			     real p10, real p11, real p12,
			     real p20, real p21, real p22 )
{
  rows[0][0] = p00; 
  rows[0][1] = p01; 
  rows[0][2] = p02;
  rows[1][0] = p10; 
  rows[1][1] = p11; 
  rows[1][2] = p12;
  rows[2][0] = p20; 
  rows[2][1] = p21; 
  rows[2][2] = p22;
  
}

/*
inline void ctMatrix3::orthonormalize()
{
  rows[0].Normalize();
  rows[1] -= rows[0]*(rows[1] * rows[0]);
  rows[2] = rows[0] % rows[1];
}
*/
inline void ctMatrix3::orthonormalize ()
{
  real len = sqrt (  rows[0][0] * rows[0][0]  
		   + rows[0][1] * rows[0][1]  
		   + rows[0][2] * rows[0][2] ); 
  
  rows[0][0] /= len;   rows[0][1] /= len;   rows[0][2] /= len;
  
  real abdot =   rows[1][0] * rows[0][0] 
               + rows[1][1] * rows[0][1] 
               + rows[1][2] * rows[0][2]; 

  rows[1][0] -= rows[0][0] * abdot;
  rows[1][1] -= rows[0][1] * abdot;
  rows[1][2] -= rows[0][2] * abdot;

  rows[2][0] = rows[0][1] * rows[1][2] - rows[0][2] * rows[1][1];
  rows[2][1] = rows[0][2] * rows[1][0] - rows[0][0] * rows[1][2];
  rows[2][2] = rows[0][0] * rows[1][1] - rows[0][1] * rows[1][0];
}

#endif // __CT_MATRIX__
