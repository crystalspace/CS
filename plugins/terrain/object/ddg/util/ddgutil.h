/*
    Copyright (C) 1997, 1998, 1999, 2000 by Alex Pfaffe
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
#ifndef _ddgUtil_Class_
#define _ddgUtil_Class_

#include "util/ddg.h"
#ifdef CRYSTAL
#include "cstypes.h"
#endif

#if !defined(WIN32) || (__BORLANDC__) ||  (defined(OS_WIN32) && defined (COMP_GCC))
#ifdef DDG
#include <strstream.h> 
#endif
#include <stdlib.h>
#define sqrtf	sqrt
#define cosf	cos
#define sinf	sin
#define atan2f	atan2
#define fabsf	fabs
#define acosf	acos
#define logf	log
#define powf	pow
#endif

#ifndef MAXFLOAT
#define MAXFLOAT  99999999
#endif
#ifndef M_PI
#define M_PI      3.1415926535897932384626433832795028841971693993751
#endif

#define DDG_BGET(VALUE,BITMASK) ((VALUE) & (BITMASK))
#define DDG_BSET(VALUE,BITMASK) (VALUE) |= (BITMASK)
#define DDG_BCLEAR(VALUE,BITMASK) (VALUE) &= ~(BITMASK)

#define sq(a) ((a)*(a))
// Some convenient angle functions.
// Convert degrees to radians
#define PITCH	0
#define YAW     1
#define ROLL    2
///
unsigned int const ddgAngle_res = 16;

/// General purpose value of PI.
#define ddgPi	M_PI

/// General utility object to perform conversions for angles.
class WEXP ddgAngle
{
    /// Cosine lookup table from 0 to 180 degrees.
    static float _cosTable[180*ddgAngle_res+1];
	/// ArcCosine lookup table from 0 to 1.
    static float _acosTable[180*ddgAngle_res+1];
public:
    /// Initializes lookup tables.
    ddgAngle();
    /// Input is angle in degrees.
    inline static float degtorad(float d) { return (d * (float)M_PI / 180.0f); }
    /// Input is angle in radians.
    inline static float radtodeg(float r) { return (r * 180.0f / (float)M_PI); }
    /// Input is angle in 0 to 1.
    inline static float ttorad(float t) { return (t * 2.0f * (float)M_PI); }
    /// Input is angle in degrees.
    inline static float mod(float a) { return a >= 360.0f ? a - 360.0f : ( a < 0.0f ? a + 360.0f : a ); }
    /// Input is angle in degrees.
    inline static float cotf(float a) { return cosf(a)/sinf(a); }
    /// Input is angle in degrees.
    static float cos( float angle);
    /// Input is angle in degrees.
    inline static float sin( float angle) { return cos(angle-90.0f);} 
    /// Input is value from 0 to 1, Output is angle in degrees.
    static float acos( float value);
    /// Input is value from 0 to 1, Output is angle in degrees.
    static float asin( float value) { return acos(1.0-value);}
};

#undef min
#undef max
/**
 *  Very general utlility class with some interpolation and clamping
 *  functionality.  Uses Angle class.
 */
class WEXP ddgUtil
{
public:
    /// return the clamped value of v so that v lies between a and b.
    inline static float clamp(float v, float a,float b)
	{ return (v) < a ? a : ((v) > b ? b : (v)); }
    /// Linearly interpolate a value between a and b for x where v = 0->1
    inline static float linterp(float a, float b, float x)
	{ return a + (b-a)*x;}
    /// Smooth binlinear interpolation from a to b.
    inline static float binterp(float a, float b, float x)
    {
		if ( x < a ) return 0; if ( x >= b) return 1;
        x = (x - a ) / (b-a);
        return (x*x * (3 - 2*x));
    }
    /// Exponential interpolation from a to b.
    inline static float einterp(float a, float b, float x)
    {
        return a+(b-a)*(powf(2.0,8.0*x)/256.0);
    }
    /// Cosine interpolation from a to b. Goes from a->b->a.
    inline static float cinterp(float a, float b, float x)
    {
        float f = (1 - ddgAngle::cos(x * 360))*0.5;
        return a*(1-f) + b * f;
    }
    /// Wrap value v if outside of a or b.
	inline static float wrap( float v, float a, float b)
	{ return v < a ? v + b - a + 1: ( v > b ? v - b + a -1 : v ); }
	/// A difference between two float values.
	inline static float diff( float v1, float v2)
	{ return v1 < v2 ? (v2-v1):(v1-v2); }
	/// A difference between two integer values.
	inline static float idiff( float v1, float v2)
	{ return v1 < v2 ? (v2-v1):(v1-v2); }
	/// The max of two float values.
	inline static float max( float v1, float v2)
	{ return v1 < v2 ? v2:v1; }
	/// The min of two float values.
	inline static float min( float v1, float v2)
	{ return v1 > v2 ? v2:v1; }
	/// The min of two float values.
	inline static float abs( float v1)
	{ return v1 > 0 ? v1:-v1; }
	/// Detect if we have SIMD instructions.
	static bool DetectSIMD(void);
};

/**
 * This global class can is used to define a root directory from which all data
 * files are referenced.
 */
class WEXP ddgFileHelper
{
public:
	/// Define the data directory.  Path should include trailing '/'.
	static void setDataDirectory( const char *dataDir );
	/**
	 * Return filename concatenated with data directory.
	 * The caller doesnt have to free this string we reuse it each time.
	 */
	static char *getFullFileName( const char *filename );
	/// Free the memory which was allocated by this class.
	static void clear(void);
};

/**
 * A simple console class.
 * Useful for error or progress messages.
 */
class WEXP ddgConsole
{
	/// The last value that progress displayed.
	static int	_last;
public:
	/// Standard message to console
	static void progress( char *s, int p, int total);
	/// Send a newline to the console.
	static void end(void);
	/// Send a string to the console
	static void s( char *s );
	/// Send an integer to the console.
	static void i( int i );
	/// Send an float to the console.
	static void f( float f );

};
/// A generic node class which can be maintained by a List.
class WEXP ddgListNode
{
    /// Next node in the list.
    ddgListNode* _next;
public:
    /// Construct a node.
    inline ddgListNode() { _next = 0; }
    /// return the next node.
    inline ddgListNode* next( ddgListNode* t = 0 )
	{
		return (t? _next = t : _next); 
	}
};

/// A generic list class which maintains a list of ListNodes.
class WEXP ddgList
{
    /// Number of nodes in the list.
    unsigned int _no;
    /// The head of the list.
    ddgListNode*  _head;
public:
    /// Constructor for an empty list.
    ddgList() { _no = 0; _head = 0; }
    /// Add a listnode to the list.
    inline void add( ddgListNode* t) { t->next(_head); _head = t; _no++; }
    /// Remove a specified list node from the list.
    inline void remove( ddgListNode* t) {
		ddgListNode *c = _head, *p = 0;
		while(c)
		{
			if (t == c) 
			{
				_no--;
				if (p) p->next(t->next());
				if (p==_head) _head = t->next(); c=0;
			}
			else { p = c; c = c->next(); }
        }
    }
    /// Return the size of the list.
    inline unsigned int size() { return _no; }
    /// Return the head of the list.
    inline ddgListNode* head( ddgListNode *h = 0) { return (h? _head = h: _head); }
};

// Useful macros to parse commandline arguments.
// Assumes text array is in 'argv' and arg count is in 'argc'.
// Assumes current arguments index is in 'argi'.
// Assumes keywords are prefixed with '-'
#define ddgArgMatch(s)		 (argi < argc && ddgStr::equal(argv[argi],"-"##s))
#define ddgArgMatchValue(s) (ddgArgMatch(s) && (++argi) < argc)

#endif
