/*
    Copyright (C) 1998 by Ayal Zwi Pinkus
  
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

#ifndef __BEZIER_H__
#define __BEZIER_H__

#define TDtDouble double
#define TDtFloat  float
#define TDtInt    int
#define TDtByte   unsigned char

// Must be called if bezier stuff is to be used!!!!
void BuildBezierLuts();
void BezierNormal(TDtDouble *aResult, TDtDouble** aControls, TDtInt iu, TDtInt iv,TDtInt resolution);
void BezierPoint(TDtDouble *aResult, TDtDouble** aControls, TDtInt iu, TDtInt iv, TDtInt resolution, 
                          TDtDouble *map );


TDtDouble bfact(TDtDouble u, TDtInt j, TDtDouble v, TDtInt k);
void BezierPoint(double *aResult, double** aControls, double u, double v,
                          double (*func)(double, int, double, int) );
void BezierNormal(double *aResult, double** aControls, double u, double v);


TDtDouble *BinomiumMap();


class csBezierCache
{
 public:
  csBezierCache();
};
inline csBezierCache::csBezierCache()
{
  BuildBezierLuts();
}


#endif
