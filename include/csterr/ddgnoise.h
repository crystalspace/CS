/*
    Copyright (C) 1997, 1998, 1999 by Alex Pfaffe
	(Digital Dawn Graphics Inc)
  
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
#ifndef _ddgNoise_Class_
#define _ddgNoise_Class_

#include "csterr/ddg.h"
/**
 * A noise generation class based on Perlin noise.
 */
class WEXP ddgNoise
{
    ///
    static double bias(double a, double b);
    ///
    static double gain(double a, double b);
    ///
    static void normalize2(double *a );
    ///
    static void normalize3(double *a );
    ///
    static void init(void );
  public:
    /**
	 *  Generate noise value for a 1D location.
	 *  Returns values between -1.0 and 1.0.
	 */
    static double noise1(double point);
    /**
	 *  Generate noise value for a 2D location.
	 *  Returns values between -1.0 and 1.0.
	 */
    static double noise2(double *point);
    /**
	 *  Generate noise value for a 3D location.
	 *  Returns values between -1.0 and 1.0.
	 */
    static double noise3(double *point);
    /**
	 *  General noise function calls the previous 3 depending on len.
	 *  Input range from 0 to 3 works well.
	 *  Returns values between -1.0 and 1.0.
	 */
    static double noise(double *point, int len);
    /**
	 *  Generate turbulent noise across a number of octaves.
	 *  Returns values between -1.0 and 1.0.
	 */
    static double turbulence(double *point, double freq);
    /**
	 *  Noise across a number of octaves.
	 *  Returns values between -1.0 and 1.0.
	 */
    static double onoise(double *point, double freq);
    /**
	 *  Cloud noise with cloud cover and cloudsharpness.
     *  cc    - cloud cover from 0.0 (total) to 1.0 (empty).
     *  cs    - cloud sharpness from 0.0 to 1.0.
	 *  Returns values between 0.0 and 1.0.
	 */
    static double cloud( double *point, double freq, int cc, double cs );
	/**
	 * Fractal brownian motion.
	 * H range fractal dimension 1.0 - 0.0 where 0.0 is total chaos.
	 * lacunarity gap between successive frequencies,  use 2.0
	 * Octaves is the number of frequencies in the fBm. 3-7
	 * Returns values between -1.0 and 1.0.
	 */
    static double fBm( double* point, double H, double lacunarity, double octaves);
	/**
	 * Multifractal function
	 * H range fractal dimension 1.0 - 0.0 where 0.0 is total chaos.
	 * lacunarity gap between successive frequencies,  use 2.0
	 * Octaves is the number of frequencies in the fBm. 3-7
	 * offset is the zero offset, which determines multifractality. eg. 0.3
	 * Returns values between -n and n.
	 */
	static double multifractal( double* point, double H, double lacunarity, double octaves, double offset );
	/**
	 * Multifractal function
	 * H range fractal dimension 1.0 - 0.0 where 0.0 is total chaos. Eg 0.25
	 * lacunarity gap between successive frequencies,  use 2.0
	 * Octaves is the number of frequencies in the fBm. 3-7
	 * offset is the zero offset, which determines multifractality. eg. 0.7
	 * Returns values between -n and n.
	 */
	static double hybridmultifractal( double* point, double H, double lacunarity, double octaves, double offset );
	/**
	 * 4D Noise.
	 * Input values between 0 to 5.  Time between 0 and 10.
	 * Returns values between 0.0 and 1.0
	 */
	static double four_noise(double* point, double time);
};

#endif
