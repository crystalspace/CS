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

#ifndef CT_VECTOR
#define CT_VECTOR

// very annoying that I can't use templates.  
// Crystal Space people have some beef with templates.

class ctVector3;
class ctMatrix3;
class ctVector6;
class ctMatrix6;

/*
* written by: Michael Alexander Ewert
*/

// seems like I could abstract a parent class of ctVector3 and ctVectorTranspose3

#include <stdarg.h>
#include <math.h>
#include "csphyzik/phyztype.h"

class ctVectorTranspose3 
{
public:
  ctVectorTranspose3(){
    elements[0] = elements[1] = elements[2] = 0.0;
  }

  ctVectorTranspose3( real pfirst, real psecond, real pthird )
  {

    elements[0] = pfirst;
    elements[1] = psecond;
    elements[2] = pthird;

  }

  void set( real pfirst, real psecond, real pthird ){
    elements[0] = pfirst;
    elements[1] = psecond;
    elements[2] = pthird;

  }

  void set( int pnum, real *pele ){
    for( int idx = 0; idx < pnum; idx++ ){
      elements[idx] = *pele;
      pele++;
    }
  }

  void set( real *pele ){
    for( int idx = 0; idx < 3; idx++ ){
      elements[idx] = *pele;
      pele++;
    }
  }

  real operator[](const int index) const { return elements[index]; } 
  real& operator[](const int index) { return elements[index]; }

  ctVectorTranspose3 operator*( const real pk ) { 
    ctVectorTranspose3 scaled;
    for( int idx = 0; idx < 3; idx++ ) 
      scaled.elements[idx] = elements[idx] * pk;  
    return scaled;
  }
  
  void operator*=(const real p) {
    for (int idx=0; idx<3; ++idx) elements[idx] *= p;
  }
  void operator/=(const real p) {
    for (int idx=0; idx<3; ++idx) elements[idx] /= p;
  }

  real operator* ( const ctVector3 &bs );

protected:
  real elements[ 3 ];
};


class ctVector3
{
public:
  ctVector3(){
    elements[0] = elements[1] = elements[2] = 0.0;
  }

  ctVector3( real pone, real ptwo, real pthree ){
    elements[0] = pone;
    elements[1] = ptwo;
    elements[2] = pthree;
  }

  real operator[](const int index) const { return elements[index]; } 
  real& operator[](const int index) { return elements[index]; }

  ctVectorTranspose3 transpose(){
    ctVectorTranspose3 trans;
    trans.set( elements );
    return trans;
  }

  // return length of this vector
  real length();

  // return a vector of unit length in same direction as this vector
  ctVector3 unit();
  void normalize();

  // set all elements to zero
  void zero(){
    for( int idx = 0; idx < 3; idx++ ) elements[idx] = 0.0;
  }

  // this = this + x
  void add( const ctVector3 & px ){
    elements[0] += px.elements[0];
    elements[1] += px.elements[1];
    elements[2] += px.elements[2];
  }

  // this = x + y
  void add2(const ctVector3 & px, const ctVector3 & py){
    elements[0] = px.elements[0] + py.elements[0];  
    elements[1] = px.elements[1] + py.elements[1];  
    elements[2] = px.elements[2] + py.elements[2];  
  
  }

  // dest = x + y
  void add3(ctVector3 & pdest, const ctVector3 & px, const ctVector3 & py){
    pdest.elements[0] = px.elements[0] + py.elements[0];  
    pdest.elements[1] = px.elements[1] + py.elements[1];  
    pdest.elements[2] = px.elements[2] + py.elements[2];  
  }
  
  void add_scaled( ctVector3 & padme, real pk ){
    elements[0] += pk*padme.elements[0];    
    elements[1] += pk*padme.elements[1];    
    elements[2] += pk*padme.elements[2];    
  }

  void add_scaled( real pk, ctVector3 & padme ){
    elements[0] += pk*padme.elements[0];    
    elements[1] += pk*padme.elements[1];    
    elements[2] += pk*padme.elements[2];    
  }

  void operator+=(const ctVector3 & p){
    for( int idx = 0; idx < 3; idx++ ) elements[idx] += p.elements[idx];  }

  ctVector3 operator+( const ctVector3 & p) const {
    ctVector3 sum;
    for( int idx = 0; idx < 3; idx++ ) 
      sum.elements[idx] = elements[idx] + p.elements[idx];  
    return sum;
  }

  // this = this - x
  void subtract( const ctVector3 & px ){
    for( int idx = 0; idx < 3; idx++ )  elements[idx] -= px.elements[idx]; }

  // this = x - y
  void subtract2(const ctVector3 & px, const ctVector3 & py) {
    for( int idx = 0; idx < 3; idx++ ) {
      elements[idx] = px.elements[idx] - py.elements[idx];
    }
  }

  // dest = x - y
  void subtract3(ctVector3 & pdest, const ctVector3 & px,
		 const ctVector3 & py){
    for( int idx = 0; idx < 3; idx++ ) {
      pdest.elements[idx] = px.elements[idx] - py.elements[idx];
    }
  }

  void operator-=(const ctVector3 & p) {
    for( int idx = 0; idx < 3; idx++ ) elements[idx] -= p.elements[idx];
  }

  ctVector3 operator-(const ctVector3 & p){
    ctVector3 sum;
    for( int idx = 0; idx < 3; idx++ ) 
      sum.elements[idx] = elements[idx] - p.elements[idx];  
    return sum;
  }

  ctVector3 operator-(const ctVector3 & p) const {
    ctVector3 sum;
    for( int idx = 0; idx < 3; idx++ ) 
      sum.elements[idx] = elements[idx] - p.elements[idx];  
    return sum;
  }

  ctVector3 operator-(){
    ctVector3 sum;
    for( int idx = 0; idx < 3; idx++ ) 
      sum.elements[idx] = -elements[idx];
    return sum;
  }

  ctVector3 operator-() const {
    ctVector3 sum;
    for( int idx = 0; idx < 3; idx++ ) 
      sum.elements[idx] = -elements[idx];
    return sum;
  }

  real operator*( const ctVector3 & p ){
    real dotp = 0.0;
    for( int idx = 0; idx < 3; idx++ ) dotp += elements[idx] * p.elements[idx]; 
    return dotp;
  }

  real operator*( const ctVector3 & p ) const {
    real dotp = 0.0;
    for( int idx = 0; idx < 3; idx++ ) dotp += elements[idx] * p.elements[idx]; 
    return dotp;
  }

  ctVector3 operator*( const real pk ) { 
    ctVector3 scaled;
    for( int idx = 0; idx < 3; idx++ ) 
      scaled.elements[idx] = elements[idx] * pk;  
    return scaled;
  }

  ctVector3 operator*( const real pk ) const { 
    ctVector3 scaled;
    for( int idx = 0; idx < 3; idx++ ) 
      scaled.elements[idx] = elements[idx] * pk;  
    return scaled;
  }

  ctVector3 operator/( const real pk ) { 
    ctVector3 scaled;
    for( int idx = 0; idx < 3; idx++ ) 
      scaled.elements[idx] = elements[idx] / pk;  
    return scaled;
  }

  void operator*=(const real p) { for (int idx=0; idx<3; ++idx) elements[idx] *= p;} 
  void operator/=(const real p) { for (int idx=0; idx<3; ++idx) elements[idx] /= p;}
 
  void cross(const ctVector3 & px, const ctVector3 & py){
    elements[0] = px.elements[1]*py.elements[2] - px.elements[2]*py.elements[1];
    elements[1] = px.elements[2]*py.elements[0] - px.elements[0]*py.elements[2];
    elements[2] = px.elements[0]*py.elements[1] - px.elements[1]*py.elements[0];
  }

  ctVector3 operator%( const ctVector3 & py ) const {
    ctVector3 xross;
    xross.elements[0] = elements[1]*py.elements[2] - elements[2]*py.elements[1];
    xross.elements[1] = elements[2]*py.elements[0] - elements[0]*py.elements[2];
    xross.elements[2] = elements[0]*py.elements[1] - elements[1]*py.elements[0];
    return xross;
  }

  ctMatrix3 operator*( const ctVectorTranspose3 &pvt );

  int get_dimension(){ return 3; }

  real *get_elements(){ return elements; }

//  friend ctVectorTranspose3<D>;
protected:
  real elements[ 3 ];

};

inline real ctVector3::length() {
  return sqrt( elements[0]*elements[0] + elements[1]*elements[1] + elements[2]*elements[2]);
}

inline ctVector3 ctVector3::unit() {
  return ((*this)/this->length() );
}

inline void ctVector3::normalize() {
real len;
  len = this->length();
  if( len > MIN_REAL )
    *this /= len;
  
}

inline real ctVectorTranspose3::operator*( const ctVector3 &pv )
{ 
real dotp = 0.0;
  for( int idx = 0; idx < 3; idx++ ) dotp += elements[idx] * pv[idx]; 
  return dotp;
}


//*************** VECTOR6

class ctVectorTranspose6
{
public:
  ctVectorTranspose6(){
    elements[0] = elements[1] = elements[2] = 0.0;
    elements[3] = elements[4] = elements[5] = 0.0;
  }

  ctVectorTranspose6( real pfirst, real psecond, real pthird, real p2first, real p2second, real p2third )
  {

    elements[0] = pfirst;
    elements[1] = psecond;
    elements[2] = pthird;
    elements[3] = p2first;
    elements[4] = p2second;
    elements[5] = p2third;


  }

  void set( real pfirst, real psecond, real pthird, real p2first, real p2second, real p2third ){
    elements[0] = pfirst;
    elements[1] = psecond;
    elements[2] = pthird;
    elements[3] = p2first;
    elements[4] = p2second;
    elements[5] = p2third;

  }

  void set( int pnum, real *pele ){
    for( int idx = 0; idx < pnum; idx++ ){
      elements[idx] = *pele;
      pele++;
    }
  }

  void set( real *pele ){
    for( int idx = 0; idx < 6; idx++ ){
      elements[idx] = *pele;
      pele++;
    }
  }

  real operator[](const int index) const { return elements[index]; } 
  real& operator[](const int index) { return elements[index]; }

  ctVectorTranspose6 operator*( const real pk ) { 
    ctVectorTranspose6 scaled;
    for( int idx = 0; idx < 6; idx++ ) 
      scaled.elements[idx] = elements[idx] * pk;  
    return scaled;
  }
  
  void operator*=(const real p) { for (int idx=0; idx<6; ++idx) elements[idx] *= p;} 
  void operator/=(const real p) { for (int idx=0; idx<6; ++idx) elements[idx] /= p;} 

  real operator* ( const ctVector6 &bs );

protected:
  real elements[ 6 ];
};


class ctVector6
{
public:
  ctVector6(){
    elements[0] = elements[1] = elements[2] = 0.0;
    elements[3] = elements[4] = elements[5] = 0.0;
  }

  ctVector6( real pone, real ptwo, real pthree, real p2one, real p2two, real p2three ){
    elements[0] = pone;
    elements[1] = ptwo;
    elements[2] = pthree;
    elements[3] = p2one;
    elements[4] = p2two;
    elements[5] = p2three;
  }

  real operator[](const int index) const { return elements[index]; } 
  real& operator[](const int index) { return elements[index]; }

  ctVectorTranspose6 transpose(){
    ctVectorTranspose6 trans;
    trans.set( elements );
    return trans;
  }

  // return length of this vector
  real length();

  // return a vector of unit length in same direction as this vector
  ctVector6 unit();
  void normalize();

  // set all elements to zero
  void zero(){
    for( int idx = 0; idx < 6; idx++ ) elements[idx] = 0.0;
  }

  // this = this + x
  void add( const ctVector6 & px ){
    elements[0] += px.elements[0];
    elements[1] += px.elements[1];
    elements[2] += px.elements[2];
    elements[3] += px.elements[3];
    elements[4] += px.elements[4];
    elements[5] += px.elements[5];
  }

  // this = x + y
  void add2(const ctVector6 & px, const ctVector6 & py){
    elements[0] = px.elements[0] + py.elements[0];  
    elements[1] = px.elements[1] + py.elements[1];  
    elements[2] = px.elements[2] + py.elements[2];  
    elements[3] = px.elements[3] + py.elements[3];  
    elements[4] = px.elements[4] + py.elements[4];  
    elements[5] = px.elements[5] + py.elements[5];  
  }

  // dest = x + y
  void add3(ctVector6 & pdest, const ctVector6 & px, const ctVector6 & py){
    pdest.elements[0] = px.elements[0] + py.elements[0];  
    pdest.elements[1] = px.elements[1] + py.elements[1];  
    pdest.elements[2] = px.elements[2] + py.elements[2];  
    pdest.elements[3] = px.elements[3] + py.elements[3];  
    pdest.elements[4] = px.elements[4] + py.elements[4];  
    pdest.elements[5] = px.elements[5] + py.elements[5];  

  }
  
  void add_scaled( ctVector6 & padme, real pk ){
    elements[0] += pk*padme.elements[0];    
    elements[1] += pk*padme.elements[1];    
    elements[2] += pk*padme.elements[2];
    elements[3] += pk*padme.elements[3];    
    elements[4] += pk*padme.elements[4];    
    elements[5] += pk*padme.elements[5];
  }

  void add_scaled( real pk, ctVector6 & padme ){
    elements[0] += pk*padme.elements[0];    
    elements[1] += pk*padme.elements[1];    
    elements[2] += pk*padme.elements[2];
    elements[3] += pk*padme.elements[3];    
    elements[4] += pk*padme.elements[4];    
    elements[5] += pk*padme.elements[5];
  }

  void operator+=(const ctVector6 & p){
    for( int idx = 0; idx < 6; idx++ ) elements[idx] += p.elements[idx];  }

  ctVector6 operator+( const ctVector6 & p) const {
    ctVector6 sum;
    for( int idx = 0; idx < 6; idx++ ) 
      sum.elements[idx] = elements[idx] + p.elements[idx];  
    return sum;
  }

    // this = this + x
  void subtract( const ctVector6 & px ){
    for( int idx = 0; idx < 6; idx++ )  elements[idx] -= px.elements[idx]; }

  // this = x + y
  void subtract2(const ctVector6 & px, const ctVector6 & py){
    for( int idx = 0; idx < 6; idx++ ) elements[idx] = px.elements[idx] - py.elements[idx];}

  // dest = x + y
  void subtract3(ctVector6 & pdest, const ctVector6 & px, const ctVector6 & py){
    for( int idx = 0; idx < 6; idx++ )  pdest.elements[idx] = px.elements[idx] - py.elements[idx];  }
  
  void operator-=(const ctVector6 & p){
    for( int idx = 0; idx < 6; idx++ ) elements[idx] -= p.elements[idx];  }

  ctVector6 operator-(const ctVector6 & p){
    ctVector6 sum;
    for( int idx = 0; idx < 6; idx++ ) 
      sum.elements[idx] = elements[idx] - p.elements[idx];  
    return sum;
  }

  ctVector6 operator-(const ctVector6 & p) const {
    ctVector6 sum;
    for( int idx = 0; idx < 6; idx++ ) 
      sum.elements[idx] = elements[idx] - p.elements[idx];  
    return sum;
  }

  real operator*( const ctVector6 & p ){
    real dotp = 0.0;
    for( int idx = 0; idx < 6; idx++ ) dotp += elements[idx] * p.elements[idx]; 
    return dotp;
  }

  real operator*( const ctVector6 & p ) const {
    real dotp = 0.0;
    for( int idx = 0; idx < 6; idx++ ) dotp += elements[idx] * p.elements[idx]; 
    return dotp;
  }

  ctVector6 operator*( const real pk ) { 
    ctVector6 scaled;
    for( int idx = 0; idx < 6; idx++ ) 
      scaled.elements[idx] = elements[idx] * pk;  
    return scaled;
  }

  ctVector6 operator*( const real pk ) const { 
    ctVector6 scaled;
    for( int idx = 0; idx < 6; idx++ ) 
      scaled.elements[idx] = elements[idx] * pk;  
    return scaled;
  }

  ctVector6 operator/( const real pk ) { 
    ctVector6 scaled;
    for( int idx = 0; idx < 6; idx++ ) 
      scaled.elements[idx] = elements[idx] / pk;  
    return scaled;
  }

  void operator*=(const real p) { for (int idx=0; idx<6; ++idx) elements[idx] *= p;} 
  void operator/=(const real p) { for (int idx=0; idx<6; ++idx) elements[idx] /= p;}

  /*
  void cross(const ctVector6 & px, const ctVector6 & py){
    elements[0] = px.elements[1]*py.elements[2] - px.elements[2]*py.elements[1];
    elements[1] = px.elements[2]*py.elements[0] - px.elements[0]*py.elements[2];
    elements[2] = px.elements[0]*py.elements[1] - px.elements[1]*py.elements[0];
  }

  ctVector3 operator%( const ctVector3 & py ){
    ctVector3 xross;
    xross.elements[0] = elements[1]*py.elements[2] - elements[2]*py.elements[1];
    xross.elements[1] = elements[2]*py.elements[0] - elements[0]*py.elements[2];
    xross.elements[2] = elements[0]*py.elements[1] - elements[1]*py.elements[0];
    return xross;
  }
*/
  ctMatrix6 operator*( const ctVectorTranspose6 &pvt );

  int get_dimension(){ return 6; }

  real *get_elements(){ return elements; }

//  friend ctVectorTranspose3<D>;
protected:
  real elements[ 6 ];

};

inline real ctVector6::length() {
  return sqrt( elements[0]*elements[0] + elements[1]*elements[1] + elements[2]*elements[2] +
        elements[3]*elements[3] + elements[4]*elements[4] + elements[5]*elements[5]);
}

inline ctVector6 ctVector6::unit() {
  return ((*this)/this->length() );
}

inline void ctVector6::normalize() {
real len;
  len = this->length();
  if( len > MIN_REAL )
    *this /= len;
  
}

inline real ctVectorTranspose6::operator*( const ctVector6 &pv )
{ 
real dotp = 0.0;
  for( int idx = 0; idx < 6; idx++ ) dotp += elements[idx] * pv[idx]; 
  return dotp;
}




#endif
