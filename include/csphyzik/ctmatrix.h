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

#ifndef CT_MATRIX
#define CT_MATRIX

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
public:
  ctMatrixN( long pdim, real scl = 1.0 ){
    dimen = pdim;
    rows = new real *[pdim];
    for( int i = 0; i < pdim; i++ ){
      rows[i] = new real[pdim];
      for( int j = 0; j < pdim; j++ )
        rows[i][j] = 0.0;
      rows[i][i] = scl;
    }
  }
  
  virtual ~ctMatrixN()
  {
    for( int i = 0; i < dimen; i++ ){
      delete [] rows[i];
    }
    delete [] rows;
  }

  real **access_elements(){ return rows; }

  void identity(){ 
    for( int i = 0; i < dimen; i++ ){
      for( int j = 0; j < dimen; j++ )
        rows[i][j] = 0.0;
      rows[i][i] = 1.0;
    }
  }

  real *operator[]( const int index ){ return rows[index]; }
  real *operator[]( const int index ) const { return rows[index]; }

  ctMatrixN get_transpose() const {
    ctMatrixN Mret;
    for( int idx = 0; idx < dimen; idx++ )
      for( int idy = 0; idy < dimen; idy++ )
        Mret[idx][idy] = rows[idy][idx];
    return Mret;
  }

  void orthonormalize();

  // better be same size vector....
  void mult_v( real *pdest, const real *pv ){
    for( int idx = 0; idx < dimen; idx++ ){
      pdest[idx] = 0;
      for( int idy = 0; idy < dimen; idy++ ){
        pdest[idx] += rows[idx][idy]*pv[idy];
      }
    }
  }

/*  ctVector3 operator* ( const ctVector3 &pv ) {
    ctVector3 rv;
    for( int idx = 0; idx < dimen; idx++ ){
      rv[idx] = 0;
      for( int idy = 0; idy < dimen; idy++ ){
        rv[idx] += rows[idx][idy]*pv[idy];
      }
    }
    return rv;
  }

  ctVector3 operator* ( const ctVector3 &pv ) const {
    ctVector3 rv;
    for( int idx = 0; idx < dimen; idx++ ){
      rv[idx] = 0;
      for( int idy = 0; idy < dimen; idy++ ){
        rv[idx] += rows[idx][idy]*pv[idy];
      }
    }
    return rv;
  }
*/
  ctMatrixN operator* ( const ctMatrixN &MM ) const {
    ctMatrixN Mret;
    for( int idr = 0; idr < dimen; idr++ )
      for( int idc = 0; idc < dimen; idc++ ){
        Mret[idr][idc] = 0.0;
        for( int adder = 0; adder < dimen; adder++ )
          Mret[idr][idc] += rows[idr][adder]*(MM[adder][idc]);
      }

    return Mret;
  }

  ctMatrixN operator* ( const real pk ) const {
    ctMatrixN Mret;
    for( int idr = 0; idr < dimen; idr++ )
      for( int idc = 0; idc < dimen; idc++ ){
        Mret[idr][idc] = rows[idr][idc]*pk;
      }

    return Mret;
  }

  void operator*=( const real pm ){
  for( int idx = 0; idx < dimen; idx++ )
    for( int idy = 0; idy < dimen; idy++ )
      rows[idx][idy] *= pm;
  }

  // addition
  void add( const ctMatrixN &pm ){
    for( int idx = 0; idx < dimen; idx++ )
      for( int idy = 0; idy < dimen; idy++ )
        rows[idx][idy] += pm.rows[idx][idy];
  }

  void add2( const ctMatrixN &pm1, const ctMatrixN &pm2 ){
    for( int idx = 0; idx < dimen; idx++ )
      for( int idy = 0; idy < dimen; idy++ )
        rows[idx][idy] = pm1.rows[idx][idy] + pm2.rows[idx][idy];
  }

  void add3( ctMatrixN &pmdest, const ctMatrixN &pm1, const ctMatrixN &pm2 ){
    for( int idx = 0; idx < dimen; idx++ )
      for( int idy = 0; idy < dimen; idy++ )
        pmdest.rows[idx][idy] = pm1.rows[idx][idy] + pm2.rows[idx][idy];
  }

  void operator+=( const ctMatrixN &pm ){
    for( int idx = 0; idx < dimen; idx++ )
      for( int idy = 0; idy < dimen; idy++ )
        rows[idx][idy] += pm.rows[idx][idy];
  }

  ctMatrixN operator+( const ctMatrixN &pm ){
    ctMatrixN Mret;

    for( int idx = 0; idx < dimen; idx++ )
      for( int idy = 0; idy < dimen; idy++ )
        Mret.rows[idx][idy] = rows[idx][idy] + pm.rows[idx][idy];

    return Mret;
  }

  // subtraction
  void subtract( const ctMatrixN &pm ){
    for( int idx = 0; idx < dimen; idx++ )
      for( int idy = 0; idy < dimen; idy++ )
        rows[idx][idy] -= pm.rows[idx][idy];
  }

  void subtract2( const ctMatrixN &pm1, const ctMatrixN &pm2 ){
    for( int idx = 0; idx < dimen; idx++ )
      for( int idy = 0; idy < dimen; idy++ )
        rows[idx][idy] = pm1.rows[idx][idy] - pm2.rows[idx][idy];
  }

  void subtract3( ctMatrixN &pmdest, const ctMatrixN &pm1, const ctMatrixN &pm2 ){
    for( int idx = 0; idx < dimen; idx++ )
      for( int idy = 0; idy < dimen; idy++ )
        pmdest.rows[idx][idy] = pm1.rows[idx][idy] - pm2.rows[idx][idy];
  }

  void operator-=( const ctMatrixN &pm ){
    for( int idx = 0; idx < dimen; idx++ )
      for( int idy = 0; idy < dimen; idy++ )
        rows[idx][idy] -= pm.rows[idx][idy];
  }

  ctMatrixN operator-( ctMatrixN &pm ){
    ctMatrixN Mret;

    for( int idx = 0; idx < dimen; idx++ )
      for( int idy = 0; idy < dimen; idy++ )
        Mret.rows[idx][idy] = rows[idx][idy] - pm.rows[idx][idy];

    return Mret;
  }

  // solve the linear system Ax = b where x is an unknown vector
  // b is a known vector and A is this matrix
  // solved x will be returned in px
  void solve( real *px, const real *pb ){
    real *x;
    real *b;
    int idx;  
    b = (real *)malloc( sizeof( real )*dimen );
    x = px;

    for( idx = 0; idx < dimen; idx++ ){
      b[idx] = pb[idx];
    }

    // solve this sucker
    linear_solve( rows, dimen, x, b );
    free(b);
  }

  void debug_print(){
    for( int i = 0; i < dimen; i++ ){
      for( int j = 0; j < dimen; j++ ){
#ifdef __CYSTALSPACE__
        Debug::logf( CT_DEBUG_LEVEL, "%lf :: ", rows[i][j] ); 
#endif
      }
#ifdef __CYSTALSPACE__
        Debug::logf( CT_DEBUG_LEVEL, "\n" );
#endif
    }
  }

protected:
  ctMatrixN(){ rows = NULL; dimen = 0; }

  real **rows;
  int dimen;
};


class ctMatrix3 : public ctMatrix
{
public:
  ctMatrix3( real scl = 1.0 ){
    dimen = 3;
    rows[0][0] = rows[1][1] = rows[2][2] = scl;
    rows[0][1] = rows[0][2] = rows[1][0] = 0.0;
    rows[1][2] = rows[2][0] = rows[2][1] = 0.0;
  }
  
  void set( real p00, real p01, real p02,
    real p10, real p11, real p12,
    real p20, real p21, real p22 );

  void identity(){ 
    rows[0][0] = rows[1][1] = rows[2][2] = 1.0;
    rows[0][1] = rows[0][2] = rows[1][0] = 0.0;
    rows[1][2] = rows[2][0] = rows[2][1] = 0.0;
  }

  ctVector3 &operator[]( const int index ){ return rows[index]; }
  ctVector3 operator[]( const int index ) const { return rows[index]; }

  ctMatrix3 get_transpose() const {
    ctMatrix3 Mret;
    for( int idx = 0; idx < 3; idx++ )
      for( int idy = 0; idy < 3; idy++ )
        Mret[idx][idy] = rows[idy][idx];
    return Mret;
  }

  void orthonormalize();

  void mult_v( ctVector3 &pdest, const ctVector3 pv ){
    for( int idx = 0; idx < 3; idx++ )
      pdest[idx] = rows[idx]*pv;
  }

  ctVector3 operator* ( const ctVector3 &pv ) {
    ctVector3 rv;
    for( int idx = 0; idx < 3; idx++ )
      rv[idx] = rows[idx]*pv;
    return rv;
  }

  ctVector3 operator* ( const ctVector3 &pv ) const {
    ctVector3 rv;
    for( int idx = 0; idx < 3; idx++ )
      rv[idx] = rows[idx]*pv;
    return rv;
  }

  ctMatrix3 operator* ( const ctMatrix3 &MM ) const {
    ctMatrix3 Mret;
    for( int idr = 0; idr < 3; idr++ )
      for( int idc = 0; idc < 3; idc++ ){
        Mret[idr][idc] = 0.0;
        for( int adder = 0; adder < 3; adder++ )
          Mret[idr][idc] += rows[idr][adder]*MM[adder][idc];
      }

    return Mret;
  }

  ctMatrix3 operator* ( const real pk ) const {
    ctMatrix3 Mret;
    for( int idr = 0; idr < 3; idr++ )
      for( int idc = 0; idc < 3; idc++ ){
        Mret[idr][idc] = rows[idr][idc]*pk;
      }

    return Mret;
  }

  void operator*=( const real pm ){
  for( int idx = 0; idx < 3; idx++ )
    for( int idy = 0; idy < 3; idy++ )
      rows[idx][idy] *= pm;
  }

  // addition
  void add( const ctMatrix3 &pm ){
    for( int idx = 0; idx < 3; idx++ )
      for( int idy = 0; idy < 3; idy++ )
        rows[idx][idy] += pm.rows[idx][idy];
  }

  void add2( const ctMatrix3 &pm1, const ctMatrix3 &pm2 ){
    for( int idx = 0; idx < 3; idx++ )
      for( int idy = 0; idy < 3; idy++ )
        rows[idx][idy] = pm1.rows[idx][idy] + pm2.rows[idx][idy];
  }

  void add3( ctMatrix3 &pmdest, const ctMatrix3 &pm1, const ctMatrix3 &pm2 ){
    for( int idx = 0; idx < 3; idx++ )
      for( int idy = 0; idy < 3; idy++ )
        pmdest.rows[idx][idy] = pm1.rows[idx][idy] + pm2.rows[idx][idy];
  }

  void operator+=( const ctMatrix3 &pm ){
    for( int idx = 0; idx < 3; idx++ )
      for( int idy = 0; idy < 3; idy++ )
        rows[idx][idy] += pm.rows[idx][idy];
  }

  ctMatrix3 operator+( const ctMatrix3 &pm ){
    ctMatrix3 Mret;

    for( int idx = 0; idx < 3; idx++ )
      for( int idy = 0; idy < 3; idy++ )
        Mret.rows[idx][idy] = rows[idx][idy] + pm.rows[idx][idy];

    return Mret;
  }

  // subtraction
  void subtract( const ctMatrix3 &pm ){
    for( int idx = 0; idx < 3; idx++ )
      for( int idy = 0; idy < 3; idy++ )
        rows[idx][idy] -= pm.rows[idx][idy];
  }

  void subtract2( const ctMatrix3 &pm1, const ctMatrix3 &pm2 ){
    for( int idx = 0; idx < 3; idx++ )
      for( int idy = 0; idy < 3; idy++ )
        rows[idx][idy] = pm1.rows[idx][idy] - pm2.rows[idx][idy];
  }

  void subtract3( ctMatrix3 &pmdest, const ctMatrix3 &pm1,
                  const ctMatrix3 &pm2 ){
    for( int idx = 0; idx < 3; idx++ )
      for( int idy = 0; idy < 3; idy++ )
        pmdest.rows[idx][idy] = pm1.rows[idx][idy] - pm2.rows[idx][idy];
  }

  void operator-=( const ctMatrix3 &pm ){
    for( int idx = 0; idx < 3; idx++ )
      for( int idy = 0; idy < 3; idy++ )
        rows[idx][idy] -= pm.rows[idx][idy];
  }

  ctMatrix3 operator-( ctMatrix3 &pm ){
    ctMatrix3 Mret;

    for( int idx = 0; idx < 3; idx++ )
      for( int idy = 0; idy < 3; idy++ )
        Mret.rows[idx][idy] = rows[idx][idy] - pm.rows[idx][idy];

    return Mret;
  }

  // solve the linear system Ax = b where x is an unknown vector
  // b is a known vector and A is this matrix
  // solved x will be returned in px
  void solve( ctVector3 &px, const ctVector3 &pb ){
    real **A;
    real *b;
    real *x;
    int idx;
    
    b = (real *)malloc( sizeof( real )*3 );
    A = (real **)malloc( sizeof( real * )*3 );
    x = px.get_elements();

    for( idx = 0; idx < 3; idx++ ){
      b[idx] = pb[idx];
      A[idx] = (real *)malloc( sizeof( real )*3 );
      for( int idy = 0; idy < 3; idy++ )
        A[idx][idy] = rows[idx][idy];
    }

    // solve this sucker
    linear_solve( A, 3, x, b );
    free( b );
    free( A );
  }

  ctMatrix3 inverse() {
    ctMatrix3 inv;
    real det = determinant();

    int i,j;
    for(i=0; i<3; i++) {
      for(j=0; j<3; j++) {
	inv[i][j] = cofactor(j,i) / det;
      }
    }

    return inv;
  }

  void debug_print(){
    for( int i = 0; i < dimen; i++ ){
      for( int j = 0; j < dimen; j++ ){
    //    Debug::logf( CT_DEBUG_LEVEL, "%lf :: ", rows[i][j] ); 
      }
    //    Debug::logf( CT_DEBUG_LEVEL, "\n" );
    }
  }

protected:
  ctVector3 rows[3];

  real cofactor(int i, int j) {
    real sign = ((i + j) % 2) ? -1 : 1;

    // Set which rows/columns the cofactor will use
    int r1, r2, c1, c2;
    r1 = (i == 0) ? 1 : 0;
    r2 = (i == 2) ? 1 : 2;
    c1 = (j == 0) ? 1 : 0;
    c2 = (j == 2) ? 1 : 2;

    real C = rows[r1][c1] * rows[r2][c2] - rows[r2][c1] * rows[r1][c2];
    return rows[i][j] * sign * C;
  }

  real determinant() {
    return rows[0][0]*rows[1][1]*rows[2][2] + rows[0][1]*rows[1][2]*rows[2][0]
      + rows[0][2]*rows[1][0]*rows[2][1] - rows[0][0]*rows[1][2]*rows[2][1]
      - rows[0][1]*rows[1][0]*rows[2][2] - rows[0][2]*rows[1][1]*rows[2][0];
  }

};

inline ctMatrix3 ctVector3::operator*( const ctVectorTranspose3 &pv )
{ 
ctMatrix3 Mret;
  for( int idr = 0; idr < 3; idr++ ){
    for( int idc = 0; idc < 3; idc++ ){
      Mret[idr][idc] = elements[idr]*pv[idc];
    } 
  }

  return Mret;
}


inline void ctMatrix3::set( real p00, real p01, real p02,
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

inline void ctMatrix3::orthonormalize()
{
  rows[0].normalize();
  rows[1] -= rows[0]*(rows[1] * rows[0]);
  rows[2] = rows[0] % rows[1];
}



//************   MATRIX6

class ctMatrix6 : public ctMatrix
{
public:
  ctMatrix6(){
    dimen = 6;
    for( int idx = 0; idx < 6; idx++ )
      for( int idy = 0; idy < 6; idy++ )
        rows[idx][idy] = ( idx == idy ) ? 1.0 : 0.0;
  }
  
  void identity(){ 
    for( int idx = 0; idx < 6; idx++ )
      for( int idy = 0; idy < 6; idy++ )
        if( idx == idy )
          rows[idx][idy] = 1.0;
        else
          rows[idx][idy] = 0.0;
  }

  ctMatrix6 get_transpose() const {
    ctMatrix6 Mret;
    for( int idx = 0; idx < 6; idx++ )
      for( int idy = 0; idy < 6; idy++ )
        Mret[idx][idy] = rows[idy][idx];
    return Mret;
  }

  ctVector6 &operator[]( const int index ){ return rows[index]; }
  ctVector6 operator[]( const int index ) const { return rows[index]; } //!me eh?

  void orthonormalize();

  void mult_v( ctVector6 &pdest, const ctVector6 pv ){
    for( int idx = 0; idx < 6; idx++ )
      pdest[idx] = rows[idx]*pv;
  }

  ctVector6 operator* ( const ctVector6 &pv ) {
    ctVector6 rv;
    for( int idx = 0; idx < 6; idx++ )
      rv[idx] = rows[idx]*pv;
    return rv;
  }

  ctVector6 operator* ( const ctVector6 &pv ) const {
    ctVector6 rv;
    for( int idx = 0; idx < 6; idx++ )
      rv[idx] = rows[idx]*pv;
    return rv;
  }

  ctMatrix6 operator* ( const ctMatrix6 &MM ) const {
    ctMatrix6 Mret;
    for( int idr = 0; idr < 6; idr++ )
      for( int idc = 0; idc < 6; idc++ ){
        Mret[idr][idc] = 0.0;
        for( int adder = 0; adder < 6; adder++ )
          Mret[idr][idc] += rows[idr][adder]*MM[adder][idc];
      }

    return Mret;
  }

  ctMatrix6 operator* ( const real pk ) const {
    ctMatrix6 Mret;
    for( int idr = 0; idr < 6; idr++ )
      for( int idc = 0; idc < 6; idc++ ){
        Mret[idr][idc] = rows[idr][idc]*pk;
      }

    return Mret;
  }

  void operator*=( const real pm ){
  for( int idx = 0; idx < 6; idx++ )
    for( int idy = 0; idy < 6; idy++ )
      rows[idx][idy] *= pm;
  }

  // addition
  void add( const ctMatrix6 &pm ){
    for( int idx = 0; idx < 6; idx++ )
      for( int idy = 0; idy < 6; idy++ )
        rows[idx][idy] += pm.rows[idx][idy];
  }

  void add2( const ctMatrix6 &pm1, const ctMatrix6 &pm2 ){
    for( int idx = 0; idx < 6; idx++ )
      for( int idy = 0; idy < 6; idy++ )
        rows[idx][idy] = pm1.rows[idx][idy] + pm2.rows[idx][idy];
  }

  void add3( ctMatrix6 &pmdest, const ctMatrix6 &pm1, const ctMatrix6 &pm2 ){
    for( int idx = 0; idx < 6; idx++ )
      for( int idy = 0; idy < 6; idy++ )
        pmdest.rows[idx][idy] = pm1.rows[idx][idy] + pm2.rows[idx][idy];
  }

  void operator+=( const ctMatrix6 &pm ){
    for( int idx = 0; idx < 6; idx++ )
      for( int idy = 0; idy < 6; idy++ )
        rows[idx][idy] += pm.rows[idx][idy];
  }

  ctMatrix6 operator+( const ctMatrix6 &pm ){
    ctMatrix6 Mret;

    for( int idx = 0; idx < 6; idx++ )
      for( int idy = 0; idy < 6; idy++ )
        Mret.rows[idx][idy] = rows[idx][idy] + pm.rows[idx][idy];

    return Mret;
  }

  // subtraction
  void subtract( const ctMatrix6 &pm ){
    for( int idx = 0; idx < 6; idx++ )
      for( int idy = 0; idy < 6; idy++ )
        rows[idx][idy] -= pm.rows[idx][idy];
  }

  void subtract2( const ctMatrix6 &pm1, const ctMatrix6 &pm2 ){
    for( int idx = 0; idx < 6; idx++ )
      for( int idy = 0; idy < 6; idy++ )
        rows[idx][idy] = pm1.rows[idx][idy] - pm2.rows[idx][idy];
  }

  void subtract3( ctMatrix6 &pmdest, const ctMatrix6 &pm1, const ctMatrix6 &pm2 ){
    for( int idx = 0; idx < 6; idx++ )
      for( int idy = 0; idy < 6; idy++ )
        pmdest.rows[idx][idy] = pm1.rows[idx][idy] - pm2.rows[idx][idy];
  }

  void operator-=( const ctMatrix6 &pm ){
    for( int idx = 0; idx < 6; idx++ )
      for( int idy = 0; idy < 6; idy++ )
        rows[idx][idy] -= pm.rows[idx][idy];
  }

  ctMatrix6 operator-( ctMatrix6 &pm ){
    ctMatrix6 Mret;

    for( int idx = 0; idx < 6; idx++ )
      for( int idy = 0; idy < 6; idy++ )
        Mret.rows[idx][idy] = rows[idx][idy] - pm.rows[idx][idy];

    return Mret;
  }

  //!me not working????
  // solve the linear system Ax = b where x is an unknown vector
  // b is a known vector and A is this matrix
  // solved x will be returned in px
  void solve( ctVector6 &px, const ctVector6 &pb ){
    real **A;
    real *b;
    real *x;
    int idx;
    
    b = (real *)malloc( sizeof( real )*6 );
    A = (real **)malloc( sizeof( real * )*6 );
    x = px.get_elements();

    for( idx = 0; idx < 6; idx++ ){
      b[idx] = pb[idx];
      A[idx] = (real *)malloc( sizeof( real )*6 );
      for( int idy = 0; idy < 6; idy++ )
        A[idx][idy] = rows[idx][idy];
    }

    // solve this sucker
    linear_solve( A, 6, x, b );

    free( b );
    free( A );
  }

  void debug_print(){
    for( int i = 0; i < dimen; i++ ){
      for( int j = 0; j < dimen; j++ ){
//        Debug::logf( CT_DEBUG_LEVEL, "%lf :: ", rows[i][j] ); 
      }
//        Debug::logf( CT_DEBUG_LEVEL, "\n" );
    }
  }

protected:
  ctVector6 rows[6];

};

inline ctMatrix6 ctVector6 ::operator*( const ctVectorTranspose6 &pv )
{ 
ctMatrix6 Mret;
  for( int idr = 0; idr < 6; idr++ ){
    for( int idc = 0; idc < 6; idc++ ){
      Mret[idr][idc] = elements[idr]*pv[idc];
    } 
  }

  return Mret;
}




#endif
