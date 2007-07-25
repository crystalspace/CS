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

#ifndef __CS_BEZIER_H__
#define __CS_BEZIER_H__

//////////////////////////////////////////////
//////////////////////////////////////////////
// Cache interface
//////////////////////////////////////////////
//////////////////////////////////////////////
#define NR1 4
#define NR2 9
#define NR3 16
#define NR4 25
#define NR5 36
#define NR6 49
#define NR7 64
#define NR8 81
#define NR9 100

#define IND1  0
#define IND2  (IND1+NR1)
#define IND3  (IND2+NR2)
#define IND4  (IND3+NR3)
#define IND5  (IND4+NR4)
#define IND6  (IND5+NR5)
#define IND7  (IND6+NR6)
#define IND8  (IND7+NR7)
#define IND9  (IND8+NR8)
#define IND10 (IND9+NR9)

#define OFFSET_1  (IND1  * 9)
#define OFFSET_2  (IND2  * 9)
#define OFFSET_3  (IND3  * 9)
#define OFFSET_4  (IND4  * 9)
#define OFFSET_5  (IND5  * 9)
#define OFFSET_6  (IND6  * 9)
#define OFFSET_7  (IND7  * 9)
#define OFFSET_8  (IND8  * 9)
#define OFFSET_9  (IND9  * 9)
#define OFFSET_10 (IND10 * 9)

#define LUT_SIZE  OFFSET_10 // Doubles

class csVector3;
class csVector2;

/**
 * Bezier curve
 */
class csBezier2
{
private:
  // binary coefficients for a 2nd degree polynomial
  static double bincoeff[3];

  // This should be approx. less than 82K
  static double bernsteinMap[LUT_SIZE];
  static double bernsteinDuMap[LUT_SIZE];
  static double bernsteinDvMap[LUT_SIZE];
  static bool initialized;

public:
  /// Initialize.
  static void Initialize ();

  /// Evaluate the bernstien polynomial defined by the given j & k at u & v
  static double BernsteinAt(double u, int j, double v, int k);

  /**
   * Evaluate the derivite of the Berstein polynomial defined by j & k with
   * respect to u at coordinates u, v
   */
  static double BernsteinDuAt(double u, int j, double v, int k);

  /**
   * Evaluate the derivite of the Berstein polynomial defined by j & k with
   * respect to v at coordinates u, v
   */
  static double BernsteinDvAt(double u, int j, double v, int k);

  /**
   * Find the normal at u, v where u, v are integers representing an index
   * to a control point on the curve in the u and v directions respectively at
   * the given resolution
   * <pre>
   * Formula:                      /2\/2\
   * vtx = sum(j=0->2) sum(k=0->2) \j/\k/ u^j(1-u)^(2-j) v^k(1-v)^(2-k)*P_jk
   * </pre>
   */
  static csVector3 GetNormal(double** aControls, int u,
                             int v, int resolution);

  /**
   * Find the normal at u,v where u and v a the parametric coordinates on the
   * curve
   */
  static csVector3 GetNormal(double** aControls, double u, double v);

  /**
   * Find the point at u, v where u, v are integers representing an index
   * to a control point on the curve in the u and v directions respectively at
   * the given resolution
   */
  static csVector3 GetPoint(double** aControls, int u, int v, int resolution,
                            double *map = 0);

  /**
   * Find the texture coordinates at u, v where u, v are integers
   * representing an index to a control point on the curve in the u and v
   * directions respectively at the given resolution
   */
  static csVector2 GetTextureCoord(double** aControls, int u, int v,
                                   int resolution, double *map = 0);

  /**
   * Find the point at u,v where u and v a the parametric coordinates on the
   * curve
   */
  static csVector3 GetPoint(double** aControls, double u, double v,
                            double (*func)(double, int, double, int) = 0 );
};

#endif // __CS_BEZIER_H__
