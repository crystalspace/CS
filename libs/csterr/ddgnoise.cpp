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
                 unsigned long mult, add;
               } pivot;

static pivot access[] = {
{{2.500000, 2.500000, 2.500000, 1.500000}, 3279704869ul, 1079814685ul},
{{3.000000, 3.000000, 3.000000, 2.000000}, 3233871397ul, 82344413ul},
{{3.500000, 3.500000, 3.500000, 2.500000}, 2832898341ul, 2979695005ul},
{{2.500000, 2.500000, 3.500000, 1.500000}, 749437669ul, 1654676173ul},
{{3.000000, 3.000000, 4.000000, 2.000000}, 3048629733ul, 2742483597ul},
{{3.500000, 3.500000, 4.500000, 2.500000}, 3713419493ul, 3525565005ul},
{{2.500000, 2.500000, 4.500000, 1.500000}, 2452456101ul, 4058468221ul},
{{3.000000, 3.000000, 5.000000, 2.000000}, 3836650917ul, 4133797693ul},
{{2.500000, 3.500000, 2.500000, 1.500000}, 3797183061ul, 1980195673ul},
{{3.000000, 4.000000, 3.000000, 2.000000}, 746675541ul, 3003340569ul},
{{3.500000, 4.500000, 3.500000, 2.500000}, 1706250325ul, 1973521113ul},
{{2.000000, 3.000000, 3.000000, 1.000000}, 1189317397ul, 1106144585ul},
{{2.500000, 3.500000, 3.500000, 1.500000}, 2894970389ul, 609087753ul},
{{3.000000, 4.000000, 4.000000, 2.000000}, 2516381973ul, 3611276489ul},
{{3.500000, 4.500000, 4.500000, 2.500000}, 103883797ul, 579057801ul},
{{2.000000, 3.000000, 4.000000, 1.000000}, 543715029ul, 3794808569ul},
{{2.500000, 3.500000, 4.500000, 1.500000}, 3359957461ul, 661750457ul},
{{3.000000, 4.000000, 5.000000, 2.000000}, 463885525ul, 1140135545ul},
{{2.500000, 4.500000, 2.500000, 1.500000}, 3415455365ul, 3793107669ul},
{{3.000000, 5.000000, 3.000000, 2.000000}, 3585866117ul, 4171109013ul},
{{2.000000, 4.000000, 3.000000, 1.000000}, 2736876357ul, 3902074309ul},
{{2.500000, 4.500000, 3.500000, 1.500000}, 4182961733ul, 2705282437ul},
{{3.000000, 5.000000, 4.000000, 2.000000}, 276394309ul, 2604137797ul},
{{2.000000, 4.000000, 4.000000, 1.000000}, 711668485ul, 727448693ul},
{{2.500000, 4.500000, 4.500000, 1.500000}, 3381327365ul, 4288885813ul},
{{3.000000, 5.000000, 5.000000, 2.000000}, 4116691205ul, 2565385205ul},
{{3.500000, 2.500000, 2.500000, 1.500000}, 1285747369ul, 2871563314ul},
{{4.000000, 3.000000, 3.000000, 2.000000}, 4085035433ul, 3923220210ul},
{{4.500000, 3.500000, 3.500000, 2.500000}, 879718569ul, 57792946ul},
{{3.000000, 2.000000, 3.000000, 1.000000}, 697882217ul, 2502137314ul},
{{3.500000, 2.500000, 3.500000, 1.500000}, 1235539305ul, 4017383586ul},
{{4.000000, 3.000000, 4.000000, 2.000000}, 730453097ul, 1801485154ul},
{{4.500000, 3.500000, 4.500000, 2.500000}, 3729249129ul, 799526434ul},
{{3.000000, 2.000000, 4.000000, 1.000000}, 197429545ul, 1886613074ul},
{{3.500000, 2.500000, 4.500000, 1.500000}, 1703659561ul, 3969732882ul},
{{4.000000, 3.000000, 5.000000, 2.000000}, 2280392489ul, 3151131602ul},
{{3.000000, 3.000000, 2.000000, 1.000000}, 2999759001ul, 1399883486ul},
{{3.500000, 3.500000, 2.500000, 1.500000}, 3497394073ul, 3358571934ul},
{{4.000000, 4.000000, 3.000000, 2.000000}, 1193823897ul, 935736414ul},
{{4.500000, 4.500000, 3.500000, 2.500000}, 1977851289ul, 3975408414ul},
{{2.500000, 2.500000, 2.500000, 0.500000}, 902379609ul, 1779434894ul},
{{3.000000, 3.000000, 3.000000, 1.000000}, 1470154585ul, 3548122190ul},
{{3.500000, 3.500000, 3.500000, 1.500000}, 1581340249ul, 728716046ul},
{{4.000000, 4.000000, 4.000000, 2.000000}, 3903513945ul, 2359941582ul},
{{4.500000, 4.500000, 4.500000, 2.500000}, 2514318425ul, 1374396558ul},
{{2.500000, 2.500000, 3.500000, 0.500000}, 1683103513ul, 816926462ul},
{{3.000000, 3.000000, 4.000000, 1.000000}, 2597383705ul, 372073918ul},
{{3.500000, 3.500000, 4.500000, 1.500000}, 2966994201ul, 2090908798ul},
{{4.000000, 4.000000, 5.000000, 2.000000}, 2238286873ul, 2395689790ul},
{{2.500000, 2.500000, 4.500000, 0.500000}, 2954040793ul, 2925227118ul},
{{3.000000, 3.000000, 5.000000, 1.000000}, 2779325657ul, 1345557294ul},
{{3.500000, 3.500000, 5.500000, 1.500000}, 2240295897ul, 2574449134ul},
{{3.000000, 4.000000, 2.000000, 1.000000}, 1688348297ul, 3521298314ul},
{{3.500000, 4.500000, 2.500000, 1.500000}, 304285065ul, 3646696010ul},
{{4.000000, 5.000000, 3.000000, 2.000000}, 261940361ul, 3784889610ul},
{{2.500000, 3.500000, 2.500000, 0.500000}, 1816613449ul, 189348666ul},
{{3.000000, 4.000000, 3.000000, 1.000000}, 1458729289ul, 570062330ul},
{{3.500000, 4.500000, 3.500000, 1.500000}, 2169933897ul, 1511977146ul},
{{4.000000, 5.000000, 4.000000, 2.000000}, 443789129ul, 3262557050ul},
{{2.500000, 3.500000, 3.500000, 0.500000}, 2388094217ul, 2506651050ul},
{{3.000000, 4.000000, 4.000000, 1.000000}, 3465923593ul, 1239466090ul},
{{3.500000, 4.500000, 4.500000, 1.500000}, 1028467465ul, 2470202154ul},
{{4.000000, 5.000000, 5.000000, 2.000000}, 1232964105ul, 2419791338ul},
{{2.500000, 3.500000, 4.500000, 0.500000}, 685152201ul, 4161356826ul},
{{3.000000, 4.000000, 5.000000, 1.000000}, 3390584521ul, 1876205274ul},
{{3.500000, 4.500000, 5.500000, 1.500000}, 2559759817ul, 1140013466ul},
{{2.500000, 4.500000, 2.500000, 0.500000}, 2267288889ul, 87805990ul},
{{3.000000, 5.000000, 3.000000, 1.000000}, 2288108601ul, 3107258086ul},
{{3.500000, 5.500000, 3.500000, 1.500000}, 1028158265ul, 2899144102ul},
{{2.500000, 4.500000, 3.500000, 0.500000}, 1604199417ul, 4226640790ul},
{{3.000000, 5.000000, 4.000000, 1.000000}, 43717369ul, 1819585110ul},
{{3.500000, 5.500000, 4.500000, 1.500000}, 4227924473ul, 3775605014ul},
{{2.500000, 4.500000, 4.500000, 0.500000}, 2966307513ul, 3115145990ul},
{{3.000000, 5.000000, 5.000000, 1.000000}, 3522851257ul, 4253985222ul},
{{4.500000, 2.500000, 2.500000, 1.500000}, 2166106365ul, 1448674955ul},
{{5.000000, 3.000000, 3.000000, 2.000000}, 1092924413ul, 2226493515ul},
{{4.000000, 2.000000, 3.000000, 1.000000}, 3333509053ul, 2338627579ul},
{{4.500000, 2.500000, 3.500000, 1.500000}, 4179696317ul, 134300091ul},
{{5.000000, 3.000000, 4.000000, 2.000000}, 2038293949ul, 3233948539ul},
{{4.000000, 2.000000, 4.000000, 1.000000}, 2787004797ul, 2970726187ul},
{{4.500000, 2.500000, 4.500000, 1.500000}, 2471648381ul, 1549705451ul},
{{5.000000, 3.000000, 5.000000, 2.000000}, 875784061ul, 3221213867ul},
{{4.000000, 3.000000, 2.000000, 1.000000}, 2628238253ul, 2359186471ul},
{{4.500000, 3.500000, 2.500000, 1.500000}, 2959677101ul, 2932189671ul},
{{5.000000, 4.000000, 3.000000, 2.000000}, 2668065197ul, 3598532519ul},
{{3.500000, 2.500000, 2.500000, 0.500000}, 2437731949ul, 3046362775ul},
{{4.000000, 3.000000, 3.000000, 1.000000}, 2913038701ul, 2571252823ul},
{{4.500000, 3.500000, 3.500000, 1.500000}, 2475887725ul, 904976919ul},
{{5.000000, 4.000000, 4.000000, 2.000000}, 773957485ul, 358596567ul},
{{3.500000, 2.500000, 3.500000, 0.500000}, 2658507821ul, 1684619975ul},
{{4.000000, 3.000000, 4.000000, 1.000000}, 3630855981ul, 3089823879ul},
{{4.500000, 3.500000, 4.500000, 1.500000}, 3317453357ul, 1444736583ul},
{{5.000000, 4.000000, 5.000000, 2.000000}, 2439720237ul, 402596871ul},
{{3.500000, 2.500000, 4.500000, 0.500000}, 1492759021ul, 1644337911ul},
{{4.000000, 3.000000, 5.000000, 1.000000}, 968805613ul, 93561015ul},
{{4.500000, 3.500000, 5.500000, 1.500000}, 127985645ul, 679798391ul},
{{4.000000, 4.000000, 2.000000, 1.000000}, 1868588637ul, 351018947ul},
{{4.500000, 4.500000, 2.500000, 1.500000}, 1703055709ul, 1376795011ul},
{{5.000000, 5.000000, 3.000000, 2.000000}, 2519841885ul, 3735065411ul},
{{3.500000, 3.500000, 2.500000, 0.500000}, 3265230109ul, 3304061747ul},
{{4.000000, 4.000000, 3.000000, 1.000000}, 1568727069ul, 3328320755ul},
{{4.500000, 4.500000, 3.500000, 1.500000}, 3585035037ul, 464555699ul},
{{5.000000, 5.000000, 4.000000, 2.000000}, 3861558813ul, 3533387891ul},
{{3.000000, 3.000000, 3.000000, 0.000000}, 2816824285ul, 805837475ul},
{{3.500000, 3.500000, 3.500000, 0.500000}, 2436234973ul, 3559150691ul},
{{4.000000, 4.000000, 4.000000, 1.000000}, 3398674909ul, 3565997603ul},
{{4.500000, 4.500000, 4.500000, 1.500000}, 1325290717ul, 1325500387ul},
{{5.000000, 5.000000, 5.000000, 2.000000}, 427163613ul, 2705490339ul},
{{3.000000, 3.000000, 4.000000, 0.000000}, 3679317405ul, 171821011ul},
{{3.500000, 3.500000, 4.500000, 0.500000}, 2206914717ul, 3374076307ul},
{{4.000000, 4.000000, 5.000000, 1.000000}, 4187276189ul, 511122259ul},
{{4.500000, 4.500000, 5.500000, 1.500000}, 2020322973ul, 2014192915ul},
{{3.500000, 4.500000, 2.500000, 0.500000}, 2876707021ul, 3810779151ul},
{{4.000000, 5.000000, 3.000000, 1.000000}, 3513797581ul, 3555429839ul},
{{4.500000, 5.500000, 3.500000, 1.500000}, 359040717ul, 1739291535ul},
{{3.000000, 4.000000, 3.000000, 0.000000}, 2694357901ul, 3534327935ul},
{{3.500000, 4.500000, 3.500000, 0.500000}, 896343693ul, 1228033599ul},
{{4.000000, 5.000000, 4.000000, 1.000000}, 3472791949ul, 3961736191ul},
{{4.500000, 5.500000, 4.500000, 1.500000}, 944575629ul, 490506687ul},
{{3.000000, 4.000000, 4.000000, 0.000000}, 469221709ul, 3748270767ul},
{{3.500000, 4.500000, 4.500000, 0.500000}, 1171553357ul, 2435326063ul},
{{4.000000, 5.000000, 5.000000, 1.000000}, 908998477ul, 780037679ul},
{{4.500000, 2.500000, 2.500000, 0.500000}, 2935324817ul, 2236143988ul},
{{5.000000, 3.000000, 3.000000, 1.000000}, 2210240401ul, 3576228404ul},
{{5.500000, 3.500000, 3.500000, 1.500000}, 1913040529ul, 1964391156ul},
{{4.500000, 2.500000, 3.500000, 0.500000}, 1370190161ul, 3102484324ul},
{{5.000000, 3.000000, 4.000000, 1.000000}, 3416311889ul, 613140516ul},
{{5.500000, 3.500000, 4.500000, 1.500000}, 3285655377ul, 791193828ul},
{{4.500000, 2.500000, 4.500000, 0.500000}, 537457169ul, 1775085908ul},
{{5.000000, 3.000000, 5.000000, 1.000000}, 3860564241ul, 320395796ul},
{{4.500000, 3.500000, 2.500000, 0.500000}, 3296762625ul, 91579840ul},
{{5.000000, 4.000000, 3.000000, 1.000000}, 1714405889ul, 2886535808ul},
{{5.500000, 4.500000, 3.500000, 1.500000}, 1959782657ul, 37613376ul},
{{4.000000, 3.000000, 3.000000, 0.000000}, 2187901121ul, 1391273968ul},
{{4.500000, 3.500000, 3.500000, 0.500000}, 2552242113ul, 2675580080ul},
{{5.000000, 4.000000, 4.000000, 1.000000}, 3699934913ul, 2597343600ul},
{{5.500000, 4.500000, 4.500000, 1.500000}, 1185017281ul, 1840236080ul},
{{4.000000, 3.000000, 4.000000, 0.000000}, 239439233ul, 69747424ul},
{{4.500000, 3.500000, 4.500000, 0.500000}, 3185980545ul, 4138715040ul},
{{5.000000, 4.000000, 5.000000, 1.000000}, 2767707009ul, 3824192608ul},
{{4.500000, 4.500000, 2.500000, 0.500000}, 342890097ul, 3035502412ul},
{{5.000000, 5.000000, 3.000000, 1.000000}, 2681032049ul, 3932819468ul},
{{4.000000, 4.000000, 3.000000, 0.000000}, 1732540465ul, 2413878908ul},
{{4.500000, 4.500000, 3.500000, 0.500000}, 755363633ul, 2199201596ul},
{{5.000000, 5.000000, 4.000000, 1.000000}, 3387450929ul, 2329849852ul},
{{4.000000, 4.000000, 4.000000, 0.000000}, 1733992689ul, 970330732ul},
{{4.500000, 4.500000, 4.500000, 0.500000}, 1792104433ul, 4006865708ul},
{{5.000000, 5.000000, 5.000000, 1.000000}, 1646857969ul, 2371290092ul}
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
