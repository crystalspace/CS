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
#ifdef DDG
#include "ddgnoise.h"
#include "ddgutil.h"
#else
#include "sysdef.h"
#include "csterr/ddgnoise.h"
#include "csterr/ddgutil.h"
#endif
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

static int p[B + B + 2];
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
	register int i, j;

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
	register int i, j;

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
/////////////////////////////////////////////////////////////////////
/* Tables for 4D noise generation
 * Created by Geoff Wyvill December 1998
 * These tables enable the calculation of Pseudo Random numbers on
 * the fly for points in a skewed 4D closely packed grid
 * The software for creating these tables is not yet public.
 * These tables work! The noise space is a 500 unit hypercube.
 */
/*
 * Four dimensional noise routines using compressed space.
 * Written by Geoff Wyvill, December 1998
 * Copyright 1998 Geoff Wyvill <geoff@cs.otago.ac.nz>
 * This source may be freely distributed for educational purposes
 * If you have a commercial application, contact the author.
 * The software that generates the tables is not currently available for
 * distribution. There is a known optimisation that will make this code
 * four to five times faster but the implementation is tedious.
 * The improved version will probably be available in October 1999.
 */

#define TABLESIZE 148
static unsigned int
  cubelength = 125000000, layerlength = 250000, rowlength = 500;
typedef double vecfour[4];

typedef struct { vecfour point;
                 int mult, add;
               } pivot;

static pivot access[] = {
{{2.500000, 2.500000, 2.500000, 1.500000}, 3279704869, 1079814685},
{{3.000000, 3.000000, 3.000000, 2.000000}, 3233871397, 82344413},
{{3.500000, 3.500000, 3.500000, 2.500000}, 2832898341, 2979695005},
{{2.500000, 2.500000, 3.500000, 1.500000}, 749437669, 1654676173},
{{3.000000, 3.000000, 4.000000, 2.000000}, 3048629733, 2742483597},
{{3.500000, 3.500000, 4.500000, 2.500000}, 3713419493, 3525565005},
{{2.500000, 2.500000, 4.500000, 1.500000}, 2452456101, 4058468221},
{{3.000000, 3.000000, 5.000000, 2.000000}, 3836650917, 4133797693},
{{2.500000, 3.500000, 2.500000, 1.500000}, 3797183061, 1980195673},
{{3.000000, 4.000000, 3.000000, 2.000000}, 746675541, 3003340569},
{{3.500000, 4.500000, 3.500000, 2.500000}, 1706250325, 1973521113},
{{2.000000, 3.000000, 3.000000, 1.000000}, 1189317397, 1106144585},
{{2.500000, 3.500000, 3.500000, 1.500000}, 2894970389, 609087753},
{{3.000000, 4.000000, 4.000000, 2.000000}, 2516381973, 3611276489},
{{3.500000, 4.500000, 4.500000, 2.500000}, 103883797, 579057801},
{{2.000000, 3.000000, 4.000000, 1.000000}, 543715029, 3794808569},
{{2.500000, 3.500000, 4.500000, 1.500000}, 3359957461, 661750457},
{{3.000000, 4.000000, 5.000000, 2.000000}, 463885525, 1140135545},
{{2.500000, 4.500000, 2.500000, 1.500000}, 3415455365, 3793107669},
{{3.000000, 5.000000, 3.000000, 2.000000}, 3585866117, 4171109013},
{{2.000000, 4.000000, 3.000000, 1.000000}, 2736876357, 3902074309},
{{2.500000, 4.500000, 3.500000, 1.500000}, 4182961733, 2705282437},
{{3.000000, 5.000000, 4.000000, 2.000000}, 276394309, 2604137797},
{{2.000000, 4.000000, 4.000000, 1.000000}, 711668485, 727448693},
{{2.500000, 4.500000, 4.500000, 1.500000}, 3381327365, 4288885813},
{{3.000000, 5.000000, 5.000000, 2.000000}, 4116691205, 2565385205},
{{3.500000, 2.500000, 2.500000, 1.500000}, 1285747369, 2871563314},
{{4.000000, 3.000000, 3.000000, 2.000000}, 4085035433, 3923220210},
{{4.500000, 3.500000, 3.500000, 2.500000}, 879718569, 57792946},
{{3.000000, 2.000000, 3.000000, 1.000000}, 697882217, 2502137314},
{{3.500000, 2.500000, 3.500000, 1.500000}, 1235539305, 4017383586},
{{4.000000, 3.000000, 4.000000, 2.000000}, 730453097, 1801485154},
{{4.500000, 3.500000, 4.500000, 2.500000}, 3729249129, 799526434},
{{3.000000, 2.000000, 4.000000, 1.000000}, 197429545, 1886613074},
{{3.500000, 2.500000, 4.500000, 1.500000}, 1703659561, 3969732882},
{{4.000000, 3.000000, 5.000000, 2.000000}, 2280392489, 3151131602},
{{3.000000, 3.000000, 2.000000, 1.000000}, 2999759001, 1399883486},
{{3.500000, 3.500000, 2.500000, 1.500000}, 3497394073, 3358571934},
{{4.000000, 4.000000, 3.000000, 2.000000}, 1193823897, 935736414},
{{4.500000, 4.500000, 3.500000, 2.500000}, 1977851289, 3975408414},
{{2.500000, 2.500000, 2.500000, 0.500000}, 902379609, 1779434894},
{{3.000000, 3.000000, 3.000000, 1.000000}, 1470154585, 3548122190},
{{3.500000, 3.500000, 3.500000, 1.500000}, 1581340249, 728716046},
{{4.000000, 4.000000, 4.000000, 2.000000}, 3903513945, 2359941582},
{{4.500000, 4.500000, 4.500000, 2.500000}, 2514318425, 1374396558},
{{2.500000, 2.500000, 3.500000, 0.500000}, 1683103513, 816926462},
{{3.000000, 3.000000, 4.000000, 1.000000}, 2597383705, 372073918},
{{3.500000, 3.500000, 4.500000, 1.500000}, 2966994201, 2090908798},
{{4.000000, 4.000000, 5.000000, 2.000000}, 2238286873, 2395689790},
{{2.500000, 2.500000, 4.500000, 0.500000}, 2954040793, 2925227118},
{{3.000000, 3.000000, 5.000000, 1.000000}, 2779325657, 1345557294},
{{3.500000, 3.500000, 5.500000, 1.500000}, 2240295897, 2574449134},
{{3.000000, 4.000000, 2.000000, 1.000000}, 1688348297, 3521298314},
{{3.500000, 4.500000, 2.500000, 1.500000}, 304285065, 3646696010},
{{4.000000, 5.000000, 3.000000, 2.000000}, 261940361, 3784889610},
{{2.500000, 3.500000, 2.500000, 0.500000}, 1816613449, 189348666},
{{3.000000, 4.000000, 3.000000, 1.000000}, 1458729289, 570062330},
{{3.500000, 4.500000, 3.500000, 1.500000}, 2169933897, 1511977146},
{{4.000000, 5.000000, 4.000000, 2.000000}, 443789129, 3262557050},
{{2.500000, 3.500000, 3.500000, 0.500000}, 2388094217, 2506651050},
{{3.000000, 4.000000, 4.000000, 1.000000}, 3465923593, 1239466090},
{{3.500000, 4.500000, 4.500000, 1.500000}, 1028467465, 2470202154},
{{4.000000, 5.000000, 5.000000, 2.000000}, 1232964105, 2419791338},
{{2.500000, 3.500000, 4.500000, 0.500000}, 685152201, 4161356826},
{{3.000000, 4.000000, 5.000000, 1.000000}, 3390584521, 1876205274},
{{3.500000, 4.500000, 5.500000, 1.500000}, 2559759817, 1140013466},
{{2.500000, 4.500000, 2.500000, 0.500000}, 2267288889, 87805990},
{{3.000000, 5.000000, 3.000000, 1.000000}, 2288108601, 3107258086},
{{3.500000, 5.500000, 3.500000, 1.500000}, 1028158265, 2899144102},
{{2.500000, 4.500000, 3.500000, 0.500000}, 1604199417, 4226640790},
{{3.000000, 5.000000, 4.000000, 1.000000}, 43717369, 1819585110},
{{3.500000, 5.500000, 4.500000, 1.500000}, 4227924473, 3775605014},
{{2.500000, 4.500000, 4.500000, 0.500000}, 2966307513, 3115145990},
{{3.000000, 5.000000, 5.000000, 1.000000}, 3522851257, 4253985222},
{{4.500000, 2.500000, 2.500000, 1.500000}, 2166106365, 1448674955},
{{5.000000, 3.000000, 3.000000, 2.000000}, 1092924413, 2226493515},
{{4.000000, 2.000000, 3.000000, 1.000000}, 3333509053, 2338627579},
{{4.500000, 2.500000, 3.500000, 1.500000}, 4179696317, 134300091},
{{5.000000, 3.000000, 4.000000, 2.000000}, 2038293949, 3233948539},
{{4.000000, 2.000000, 4.000000, 1.000000}, 2787004797, 2970726187},
{{4.500000, 2.500000, 4.500000, 1.500000}, 2471648381, 1549705451},
{{5.000000, 3.000000, 5.000000, 2.000000}, 875784061, 3221213867},
{{4.000000, 3.000000, 2.000000, 1.000000}, 2628238253, 2359186471},
{{4.500000, 3.500000, 2.500000, 1.500000}, 2959677101, 2932189671},
{{5.000000, 4.000000, 3.000000, 2.000000}, 2668065197, 3598532519},
{{3.500000, 2.500000, 2.500000, 0.500000}, 2437731949, 3046362775},
{{4.000000, 3.000000, 3.000000, 1.000000}, 2913038701, 2571252823},
{{4.500000, 3.500000, 3.500000, 1.500000}, 2475887725, 904976919},
{{5.000000, 4.000000, 4.000000, 2.000000}, 773957485, 358596567},
{{3.500000, 2.500000, 3.500000, 0.500000}, 2658507821, 1684619975},
{{4.000000, 3.000000, 4.000000, 1.000000}, 3630855981, 3089823879},
{{4.500000, 3.500000, 4.500000, 1.500000}, 3317453357, 1444736583},
{{5.000000, 4.000000, 5.000000, 2.000000}, 2439720237, 402596871},
{{3.500000, 2.500000, 4.500000, 0.500000}, 1492759021, 1644337911},
{{4.000000, 3.000000, 5.000000, 1.000000}, 968805613, 93561015},
{{4.500000, 3.500000, 5.500000, 1.500000}, 127985645, 679798391},
{{4.000000, 4.000000, 2.000000, 1.000000}, 1868588637, 351018947},
{{4.500000, 4.500000, 2.500000, 1.500000}, 1703055709, 1376795011},
{{5.000000, 5.000000, 3.000000, 2.000000}, 2519841885, 3735065411},
{{3.500000, 3.500000, 2.500000, 0.500000}, 3265230109, 3304061747},
{{4.000000, 4.000000, 3.000000, 1.000000}, 1568727069, 3328320755},
{{4.500000, 4.500000, 3.500000, 1.500000}, 3585035037, 464555699},
{{5.000000, 5.000000, 4.000000, 2.000000}, 3861558813, 3533387891},
{{3.000000, 3.000000, 3.000000, 0.000000}, 2816824285, 805837475},
{{3.500000, 3.500000, 3.500000, 0.500000}, 2436234973, 3559150691},
{{4.000000, 4.000000, 4.000000, 1.000000}, 3398674909, 3565997603},
{{4.500000, 4.500000, 4.500000, 1.500000}, 1325290717, 1325500387},
{{5.000000, 5.000000, 5.000000, 2.000000}, 427163613, 2705490339},
{{3.000000, 3.000000, 4.000000, 0.000000}, 3679317405, 171821011},
{{3.500000, 3.500000, 4.500000, 0.500000}, 2206914717, 3374076307},
{{4.000000, 4.000000, 5.000000, 1.000000}, 4187276189, 511122259},
{{4.500000, 4.500000, 5.500000, 1.500000}, 2020322973, 2014192915},
{{3.500000, 4.500000, 2.500000, 0.500000}, 2876707021, 3810779151},
{{4.000000, 5.000000, 3.000000, 1.000000}, 3513797581, 3555429839},
{{4.500000, 5.500000, 3.500000, 1.500000}, 359040717, 1739291535},
{{3.000000, 4.000000, 3.000000, 0.000000}, 2694357901, 3534327935},
{{3.500000, 4.500000, 3.500000, 0.500000}, 896343693, 1228033599},
{{4.000000, 5.000000, 4.000000, 1.000000}, 3472791949, 3961736191},
{{4.500000, 5.500000, 4.500000, 1.500000}, 944575629, 490506687},
{{3.000000, 4.000000, 4.000000, 0.000000}, 469221709, 3748270767},
{{3.500000, 4.500000, 4.500000, 0.500000}, 1171553357, 2435326063},
{{4.000000, 5.000000, 5.000000, 1.000000}, 908998477, 780037679},
{{4.500000, 2.500000, 2.500000, 0.500000}, 2935324817, 2236143988},
{{5.000000, 3.000000, 3.000000, 1.000000}, 2210240401, 3576228404},
{{5.500000, 3.500000, 3.500000, 1.500000}, 1913040529, 1964391156},
{{4.500000, 2.500000, 3.500000, 0.500000}, 1370190161, 3102484324},
{{5.000000, 3.000000, 4.000000, 1.000000}, 3416311889, 613140516},
{{5.500000, 3.500000, 4.500000, 1.500000}, 3285655377, 791193828},
{{4.500000, 2.500000, 4.500000, 0.500000}, 537457169, 1775085908},
{{5.000000, 3.000000, 5.000000, 1.000000}, 3860564241, 320395796},
{{4.500000, 3.500000, 2.500000, 0.500000}, 3296762625, 91579840},
{{5.000000, 4.000000, 3.000000, 1.000000}, 1714405889, 2886535808},
{{5.500000, 4.500000, 3.500000, 1.500000}, 1959782657, 37613376},
{{4.000000, 3.000000, 3.000000, 0.000000}, 2187901121, 1391273968},
{{4.500000, 3.500000, 3.500000, 0.500000}, 2552242113, 2675580080},
{{5.000000, 4.000000, 4.000000, 1.000000}, 3699934913, 2597343600},
{{5.500000, 4.500000, 4.500000, 1.500000}, 1185017281, 1840236080},
{{4.000000, 3.000000, 4.000000, 0.000000}, 239439233, 69747424},
{{4.500000, 3.500000, 4.500000, 0.500000}, 3185980545, 4138715040},
{{5.000000, 4.000000, 5.000000, 1.000000}, 2767707009, 3824192608},
{{4.500000, 4.500000, 2.500000, 0.500000}, 342890097, 3035502412},
{{5.000000, 5.000000, 3.000000, 1.000000}, 2681032049, 3932819468},
{{4.000000, 4.000000, 3.000000, 0.000000}, 1732540465, 2413878908},
{{4.500000, 4.500000, 3.500000, 0.500000}, 755363633, 2199201596},
{{5.000000, 5.000000, 4.000000, 1.000000}, 3387450929, 2329849852},
{{4.000000, 4.000000, 4.000000, 0.000000}, 1733992689, 970330732},
{{4.500000, 4.500000, 4.500000, 0.500000}, 1792104433, 4006865708},
{{5.000000, 5.000000, 5.000000, 1.000000}, 1646857969, 2371290092}
}; 

// Congruential sequence constants. Suggested by Neave and Knuth.
#define RANDOM_BEE 3125
#define RANDOM_CEE 49
#define WIDTH 1.4
#define RANGE 4294967296.0

/*
 *  Generated file contains the 4D matrix of data points with
 *  relative co-ordinates and random generation constants:
 *
 *   cubelength = 125000000, layerlength = 250000, rowlength = 500;
 *   static pivot access[] = { generated data };
 */

unsigned int any_random (unsigned int k)
{
unsigned int b = RANDOM_BEE;
unsigned int c = RANDOM_CEE;
unsigned int result = 1; /* have to start somewhere */

for (;k > 0;k>>=1)
	{
	if (k & 1) result = result * b + c;
	c += b * c;
	b *= b;
	}
return result;
}

double ddgNoise::four_noise(double* sample, double time)
{ unsigned int seed;
  int ix, iy, iz, iw, index;
  double C, sum = 0.0, weight = 0.0, x, y, z, w, rsquare;
// unskew
  w = 10.0*time;
  x = 10.0*sample[0]-w;
  y = 10.0*sample[1]-w;
  z = 10.0*sample[2]-w;
  w *= 2.0;
// find base
  ix = int(floor(x) - 2);
  iy = int(floor(y) - 2);
  iz = int(floor(z) - 2);
  iw = int(floor(w) - 2);
// find seed at base  
  seed = any_random(cubelength*iw + layerlength*iz + rowlength*iy + ix);
// unskewed offsets
  x = x - ix;
  y = y - iy;
  z = z - iz;
  w = w - iw;
//skew
  w *= 0.5;
  x += w;
  y += w;
  z += w;
  for (index = 0; index < TABLESIZE; index++)
  { rsquare = (access[index].point[0]-x)*(access[index].point[0]-x) +
              (access[index].point[1]-y)*(access[index].point[1]-y) +
              (access[index].point[2]-z)*(access[index].point[2]-z) +
              (access[index].point[3]-w)*(access[index].point[3]-w);
//printf("index %d rsquare %f, %d\n", index, rsquare, TABLESIZE);
//printf("at: %f %f %f %f\n", x, y, z, w);
    if (rsquare < WIDTH * WIDTH)
    { C = 1.0 - rsquare/(WIDTH * WIDTH);
      C *= C * C;
      sum += C * (seed * access[index].mult + access[index].add)/RANGE;
      weight += C;
//printf("sum %f, weight %f\n", sum, weight);
    }
  }
//printf("sum %f, weight %f\n", sum, weight);
  return sum/weight;
//printf("returning\n");
}
