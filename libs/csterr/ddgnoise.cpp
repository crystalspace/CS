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

#include "csterr/ddgnoise.h"
#include "csterr/ddgutil.h"
// ----------------------------------------------------------------------

ddgNoise		noise;	// Ensure noise gets initialized.

#define MAXOCT	10
double ddgNoise::bias(double a, double b)
{
	return pow(a, log(b) / log(0.5));
}

double ddgNoise::gain(double a, double b)
{
	double p = log(1.0 - b) / log(0.5);

	if (a < 0.001)
		return 0.0;
	else if (a > 0.999)
		return 1.0;
	if (a < 0.5)
		return pow(2 * a, p) / 2.0;
	else
		return 1.0 - pow(2.0 * (1.0 - a), p) / 2.0;
}

double ddgNoise::noise(double vec[], int len)
{
	switch (len) {
	case 0:
		return 0.;
	case 1:
		return noise1(vec[0]);
	case 2:
		return noise2(vec);
	default:
		return noise3(vec);
	}
}

double ddgNoise::turbulence(double *v, double freq)
{
	double t, vec[3];

	for (t = 0.0 ; freq >= 1.0 ; freq /= 2.0) {
		vec[0] = freq * v[0];
		vec[1] = freq * v[1];
		vec[2] = freq * v[2];
		t += fabs(noise3(vec)) / freq;
	}
	return t;
}

// Returns a value from -1.0 to 1.0
double ddgNoise::onoise(double *v, double freq)
{
	double t, vec[3];

	for (t = 0.0 ; freq >= 1.0 ; freq /= 2.0) {
		vec[0] = freq * v[0];
		vec[1] = freq * v[1];
		vec[2] = freq * v[2];
		t += noise3(vec) / freq;
	}
	return ddgUtil::clamp(t,-1.0,1.0);
}

// cloud texture
// v     - x,y location from 0.0 to 1.0.
// freq  - number of octaves.
// cc    - cloud cover from 0 (total) to 1.0 (empty).
// cs    - cloud sharpness from 0.0 to 1.0.
double ddgNoise::cloud( double *v, double freq, int cc, double cs )
{
    double vv = onoise(v,freq) - cc;
    if (vv < 0.0)
		vv = 0.0;
 
    return 1.0 - pow(cs, vv);
}

/* noise functions over 1, 2, and 3 dimensions */

#define B 0x100
#define BM 0xff

#define N 0x1000
#define NP 12   /* 2^N */
#define NM 0xfff

static p[B + B + 2];
static double g3[B + B + 2][3];
static double g2[B + B + 2][2];
static double g1[B + B + 2];
static bool start = true;
static double  exponent_array[MAXOCT];

#define s_curve(t) ( t * t * (3.0 - 2.0 * t) )

#define lerp(t, a, b) ( a + t * (b - a) )

#define setup(i,b0,b1,r0,r1)\
	t = vec[i] + N;\
	b0 = ((int)t) & BM;\
	b1 = (b0+1) & BM;\
	r0 = t - (int)t;\
	r1 = r0 - 1.0;

double ddgNoise::noise1(double arg)
{
	int bx0, bx1;
	double rx0, rx1, sx, t, u, v, vec[1];

	vec[0] = arg;
	if (start) {
		init();
	}

	setup(0, bx0,bx1, rx0,rx1);

	sx = s_curve(rx0);

	u = rx0 * g1[ p[ bx0 ] ];
	v = rx1 * g1[ p[ bx1 ] ];

	return lerp(sx, u, v);
}

double ddgNoise::noise2(double *vec
						)
{
	int bx0, bx1, by0, by1, b00, b10, b01, b11;
	double rx0, rx1, ry0, ry1, *q, sx, sy, a, b, t, u, v;
	register i, j;

	if (start) {
		init();
	}

	setup(0, bx0,bx1, rx0,rx1);
	setup(1, by0,by1, ry0,ry1);

	i = p[ bx0 ];
	j = p[ bx1 ];

	b00 = p[ i + by0 ];
	b10 = p[ j + by0 ];
	b01 = p[ i + by1 ];
	b11 = p[ j + by1 ];

	sx = s_curve(rx0);
	sy = s_curve(ry0);

#define at2(rx,ry) ( rx * q[0] + ry * q[1] )

	q = g2[ b00 ] ; u = at2(rx0,ry0);
	q = g2[ b10 ] ; v = at2(rx1,ry0);
	a = lerp(sx, u, v);

	q = g2[ b01 ] ; u = at2(rx0,ry1);
	q = g2[ b11 ] ; v = at2(rx1,ry1);
	b = lerp(sx, u, v);

	return lerp(sy, a, b);
}

double ddgNoise::noise3(double *vec)
{
	int bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
	double rx0, rx1, ry0, ry1, rz0, rz1, *q, sy, sz, a, b, c, d, t, u, v;
	register i, j;

	if (start) {
		init();
	}

	setup(0, bx0,bx1, rx0,rx1);
	setup(1, by0,by1, ry0,ry1);
	setup(2, bz0,bz1, rz0,rz1);

	i = p[ bx0 ];
	j = p[ bx1 ];

	b00 = p[ i + by0 ];
	b10 = p[ j + by0 ];
	b01 = p[ i + by1 ];
	b11 = p[ j + by1 ];

	t  = s_curve(rx0);
	sy = s_curve(ry0);
	sz = s_curve(rz0);

#define at3(rx,ry,rz) ( rx * q[0] + ry * q[1] + rz * q[2] )

	q = g3[ b00 + bz0 ] ; u = at3(rx0,ry0,rz0);
	q = g3[ b10 + bz0 ] ; v = at3(rx1,ry0,rz0);
	a = lerp(t, u, v);

	q = g3[ b01 + bz0 ] ; u = at3(rx0,ry1,rz0);
	q = g3[ b11 + bz0 ] ; v = at3(rx1,ry1,rz0);
	b = lerp(t, u, v);

	c = lerp(sy, a, b);

	q = g3[ b00 + bz1 ] ; u = at3(rx0,ry0,rz1);
	q = g3[ b10 + bz1 ] ; v = at3(rx1,ry0,rz1);
	a = lerp(t, u, v);

	q = g3[ b01 + bz1 ] ; u = at3(rx0,ry1,rz1);
	q = g3[ b11 + bz1 ] ; v = at3(rx1,ry1,rz1);
	b = lerp(t, u, v);

	d = lerp(sy, a, b);

	return lerp(sz, c, d);
}

void ddgNoise::normalize2(double v[2])
{
	double s;

	s = 1.0 / sqrt(v[0] * v[0] + v[1] * v[1]);
	v[0] = v[0] * s;
	v[1] = v[1] * s;
}

void ddgNoise::normalize3(double v[3])
{
	double s;

	s = 1.0 / sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] = v[0] * s;
	v[1] = v[1] * s;
	v[2] = v[2] * s;
}

void ddgNoise::init(void)
{
	int i, j, k;

	for (i = 0 ; i < B ; i++) {
		p[i] = i;

		g1[i] = (double)((rand() % (B + B)) - B) / B;

		for (j = 0 ; j < 2 ; j++)
			g2[i][j] = (double)((rand() % (B + B)) - B) / B;
		normalize2(g2[i]);

		for (j = 0 ; j < 3 ; j++)
			g3[i][j] = (double)((rand() % (B + B)) - B) / B;
		normalize3(g3[i]);
	}

	while (--i) {
		k = p[i];
		p[i] = p[j = rand() % B];
		p[j] = k;
	}

	for (i = 0 ; i < B + 2 ; i++) {
		p[B + i] = p[i];
		g1[B + i] = g1[i];
		for (j = 0 ; j < 2 ; j++)
			g2[B + i][j] = g2[i][j];
		for (j = 0 ; j < 3 ; j++)
			g3[B + i][j] = g3[i][j];
	}

	start = false;
}


double ddgNoise::fBm( double *point, double H, double lacunarity, double octaves )
{
	double value = 0.0, frequency = 1.0, remainder = octaves - (int)octaves;
	int     i;
	double p[3];
	static double oldH = 0.0, oldL = 0.0;
	if (H != oldH || lacunarity != oldL) {
		/* precompute and store special weights */
		for (i=0; i <= octaves; i ++)
		  {
		  // compute weight for each frequency.
		  exponent_array[i] = pow( frequency, -H );
		  frequency *= lacunarity;
		  }
		oldH = H;
		oldL = lacunarity;
	}

	p[0] = point[0];
	p[1] = point[1];
	p[2] = point[2];
	// Inner loop of spectral construction.
	for (i=0; i < octaves; i++)
	{
		value += noise3(p ) * exponent_array[i];
		p[0] *= lacunarity;
		p[1] *= lacunarity;
		p[2] *= lacunarity;
	}

	if (remainder)
	{
		value += remainder *noise3(p) * exponent_array[i];
	}
	return(value);
}
// H range fractal dimension 1.0 - 0.0 0.0 is total chaos.
// lacunarity gap between successive frequencies, 
// Octaves is the number of frequencies in the fBm.
// offset is the zero offset, which determines multifractality.
double ddgNoise::multifractal( double* point, double H, double lacunarity, double octaves, double offset )
{
	double value = 1.0, frequency = 1.0, remainder = octaves - (int)octaves;
	int     i;
	double p[3];
	p[0] = point[0];
	p[1] = point[1];
	p[2] = point[2];

	static double oldH = 0.0, oldL = 0.0;
	if (H != oldH || lacunarity != oldL) {
		/* precompute and store special weights */
		for (i=0; i <= octaves; i ++)
		  {
		  // compute weight for each frequency.
		  exponent_array[i] = pow( frequency, -H );
		  frequency *= lacunarity;
		  }
		oldH = H;
		oldL = lacunarity;
	}
  
	// Inner loop of spectral construction.
	for (i=0; i < octaves; i++)
	{
		value *= offset * noise3(p )* exponent_array[i];
		p[0] *= lacunarity;
		p[1] *= lacunarity;
		p[2] *= lacunarity;
	}

	if (remainder)
	{
		value += remainder *noise3(p) * exponent_array[i];
	}
	return(value);
}

// H range fractal dimension 1.0 - 0.0 0.0 is total chaos.
// lacunarity gap between successive frequencies, 
// Octaves is the number of frequencies in the fBm.
// offset is the zero offset, which determines multifractality.
double ddgNoise::hybridmultifractal( double* point, double H, double lacunarity, double octaves, double offset )
{
	double value = 1.0,
		frequency = 1.0,
		remainder = octaves - (int)octaves,
		signal,
		weight;
	int     i;
	double p[3];
	p[0] = point[0];
	p[1] = point[1];
	p[2] = point[2];

	static double oldH = 0.0, oldL = 0.0;
	if (H != oldH || lacunarity != oldL) {
		/* precompute and store special weights */
		for (i=0; i <= octaves; i ++)
		  {
		  // compute weight for each frequency.
		  exponent_array[i] = pow( frequency, -H );
		  frequency *= lacunarity;
		  }
		oldH = H;
		oldL = lacunarity;
	}
  
	/* Get 1st octave of function */
	value = (noise3(p)+offset)* exponent_array[0];
	weight = value;
	p[0] *= lacunarity;
	p[1] *= lacunarity;
	p[2] *= lacunarity;
	// Inner loop of spectral construction.
	for (i=1; i < octaves; i++)
	{
		// Prevent divergence.
		if (weight > 1.0) weight = 1.0;

		signal = (noise3(p) + offset) *exponent_array[i];
		value += weight *signal;
		weight *= signal;
		p[0] *= lacunarity;
		p[1] *= lacunarity;
		p[2] *= lacunarity;
	}

	if (remainder)
	{
		value += remainder *noise3(p) * exponent_array[i];
	}
	return(value);
}

double ddgNoise::four_noise(double* , double )
{
  return 0;
}