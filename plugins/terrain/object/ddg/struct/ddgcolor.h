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
#ifndef _ddgddgColor_Class
#define _ddgddgColor_Class

#include "util/ddgstr.h"
#include "math/ddgvec.h"

/** 
 * A convenience object to support colors.
 * This a 3 component color, with R,G,B values from 0 to 1.
 */
class WEXP ddgColor3 {
  public:
	/// ddgColor data.
	unsigned char v[3];
    /// Construct a color from a color.
	ddgColor3( const ddgColor3 *c);
	/// Construct a color from R,G,B values.
	ddgColor3( unsigned char r, unsigned char g, unsigned char b);
    /// Create a default white color.
    ddgColor3( void );
	/// return the value of the vector as a unsigned char array.
	operator const unsigned char* () { return v; }
	/// return a pointer to a vector from a vector.
	operator ddgColor3* () { return this; }
	/// Get value of one dimension of the vector.
	unsigned char operator[](int n) const  { return v[n]; }
	/// Set color into color object.
	void set(const ddgColor3 *c)
	{	v[0] = c->v[0]; v[1] = c->v[1]; v[2] = c->v[2]; }
	/// Set color into color object.
	void set(unsigned char r, unsigned char g,unsigned char  b)
	{	v[0] = r; v[1] = g; v[2] = b; }
	/// Linearly Interpolate a new color from two other colors.
	void linterp(const ddgColor3 *a, const ddgColor3 *b, float t)
	{
		set((unsigned char)ddgUtil::linterp(a->v[0],b->v[0],t),
		(unsigned char)ddgUtil::linterp(a->v[1],b->v[1],t),
		(unsigned char)ddgUtil::linterp(a->v[2],b->v[2],t));
	}
	/// Exponentially Interpolate a new color from two other colors.
	void einterp(const ddgColor3 *a, const ddgColor3 *b, float t)
	{
        float x = ddgUtil::einterp(0,1,t);
		linterp(a,b,x);
	}
	/// Cosine Interpolate a new color from two other colors.
	void cinterp(const ddgColor3 *a, const ddgColor3 *b, float t)
	{
        float x = ddgUtil::cinterp(0,1,t);
		linterp(a,b,x);
	}
	/// Bilinear Interpolate a new color from two other colors.
	void binterp(const ddgColor3 *a, const ddgColor3 *b, float t)
	{
        float x = ddgUtil::binterp(0,1,t);
		linterp(a,b,x);
	}
	/// Return red component.
	unsigned char r(void) { return v[0]; }
	/// Return green component.
	unsigned char g(void) { return v[1]; }
	/// Return blue component.
	unsigned char b(void) { return v[2]; }
};

/**
 *  A convenience object to support colors.
 *  This a 4 component color, with R,G,B and Alpha values from 0 to 1.
 */
class WEXP ddgColor4 {
  public:
	/// ddgColor data.
	unsigned char v[4];

	/// Construct a color from R,G,B values.
	ddgColor4( unsigned char r, unsigned char g, unsigned char b, unsigned char a);
    /// Create a default white color.
    ddgColor4( void );

	/// return the value of the vector as a unsigned char array.
	operator const unsigned char* () { return v; }
	/// return a pointer to a vector from a vector.
	operator ddgColor4* () { return this; }
	/// Get value of one dimension of the vector.
	unsigned char operator[](int n) const  { return v[n]; }
	/// Set color into color object.
	void set(const ddgColor4 *c)
	{	v[0] = c->v[0]; v[1] = c->v[1]; v[2] = c->v[2]; v[3] = c->v[3]; }
	/// Set color into color object.
	void set(unsigned char r, unsigned char g,unsigned char b, unsigned char a)
	{	v[0] = r; v[1] = g; v[2] = b; v[3] = a; }
	/// Interpolate a new color from two other colors.
	void linterp(const ddgColor4 *a, const ddgColor4 *b, float t)
	{
	   set((unsigned char)ddgUtil::linterp(a->v[0],b->v[0],t),
		(unsigned char)ddgUtil::linterp(a->v[1],b->v[1],t),
		(unsigned char)ddgUtil::linterp(a->v[2],b->v[2],t),
		(unsigned char)ddgUtil::linterp(a->v[3],b->v[3],t));
	}
	/// Exponentially Interpolate a new color from two other colors.
	void einterp(const ddgColor4 *a, const ddgColor4 *b, float t)
	{
        float x = ddgUtil::einterp(0,1,t);
		linterp(a,b,x);
	}
	/// Cosine Interpolate a new color from two other colors.
	void cinterp(const ddgColor4 *a, const ddgColor4 *b, float t)
	{
        float x = ddgUtil::cinterp(0,1,t);
		linterp(a,b,x);
	}
	/// Bilinear Interpolate a new color from two other colors.
	void binterp(const ddgColor4 *a, const ddgColor4 *b, float t)
	{
        float x = ddgUtil::binterp(0,1,t);
		linterp(a,b,x);
	}
	/// Test
	static void test( ddgColor4 *cs, ddgColor4 *ce );
	/// Return red component.
	unsigned char r(void) { return v[0]; }
	/// Return green component.
	unsigned char g(void) { return v[1]; }
	/// Return blue component.
	unsigned char b(void) { return v[2]; }
	/// Return alpha component.
	unsigned char a(void) { return v[3]; }
};

#ifdef DDGSTREAM
/// Stream operator for outputting of color.
WEXP ostream& WFEXP operator << ( ostream&s, ddgColor3 v );
/// Stream operator for outputting of color.
WEXP ostream& WFEXP operator << ( ostream&s, ddgColor3* v );
/// Stream operator for inputting of color.
WEXP istream& WFEXP operator >> ( istream& s, ddgColor3& v);

/// Stream operator for outputting of color.
WEXP ostream& WFEXP operator << ( ostream&s, ddgColor4 v );
/// Stream operator for outputting of color.
WEXP ostream& WFEXP operator << ( ostream&s, ddgColor4* v );
/// Stream operator for inputting of color.
WEXP istream& WFEXP operator >> ( istream& s, ddgColor4& v);
#endif // DDGSTREAM

/// A ddgColor3 object which can be added to a ddgColorSet.
class WEXP ddgColorNode : public ddgListNode {
    /// Optional key value.
    float	       _key;
    /// Optional color name.
    ddgStr            _name;
	/// Is it a 3 or 4 color node.
	bool			_is4;
public:
    /// The color data of the node.
	union
	{
		ddgColor3  *color3;
		ddgColor4	*color4;
	};
    /// Construct a named color from a 3 color vector.
    ddgColorNode( ddgColor3 *c, float k = 0.0, char* name = 0 )
		: color3(c), _key(k), _is4(false)
	{ _name.assign(name); }
    /// Construct a named color from a 4 color vector.
    ddgColorNode( ddgColor4 *c, float k = 0.0, char* name = 0 )
		: color4(c), _key(k), _is4(true)
	{ _name.assign(name); }
    /// Return the color's index value.
    float key(void)  { return _key; }
    /// Return the color's name.
    char* name(void)  { return _name; }
    /// Set the color's name.
    void name(char *name)  { _name.assign(name); }
	/// Return if the color is 4 component colour.
	bool is4(void) { return _is4; }
};


#ifdef DDGSTREAM
/// Stream operator for outputting of colornodes.
WEXP ostream& WFEXP operator << ( ostream&s, ddgColorNode v );
/// Stream operator for outputting of color.
WEXP ostream& WFEXP operator << ( ostream&s, ddgColorNode* v );
/*
/// Input stream for color nodes.
WEXP istream& WFEXP operator >> ( istream& s, ddgColorNode& v);
*/
#endif // DDGSTREAM
/**
 *  Maintain a set of colors which can be indexed.
 *  A ddgColorSet can maintains a set of ddgColor3 or ddgColor4 objects.
 *  However it only returns ddgColor3 objects (ie ddgColor4 objects are
 *  treated as if they were color3s.)
 */
class WEXP ddgColorSet : public ddgList {
    typedef ddgList super;
    public:
    /// Find a colour by gradient index.
    ddgColor3* find(float key)
    {
    	ddgColorNode *c = findnode(key);
		return c ? c->color3 : (ddgColor3*) NULL;
    }
    /// Find a colour node by gradient index.
    ddgColorNode* findnode(float key)
    {
    	ddgColorNode *c = (ddgColorNode*) head();
    	while (c && c->key() != key)
    		c = (ddgColorNode *) c->next();
		return c;
    }
    /// Find a colornode by name.
    ddgColorNode *findnode(char* name)
    {
    	ddgColorNode *c = (ddgColorNode*) head();
    	while (c && !ddgStr::equal(name,c->name()))
    		c = (ddgColorNode *) c->next();
		return c;
    }
    /// Find a color by name.
    ddgColor3 *find(char* name)
    {
    	ddgColorNode *c = findnode(name);
		return c ? c->color3 : (ddgColor3*) NULL;
    }
    /// Find a colour node by gradient index.
    ddgColorNode* findNode(float key)
    {
    	ddgColorNode *c = (ddgColorNode*) head();
    	while (c && c->key() != key)
    		c = (ddgColorNode *) c->next();
		return c;
    }
    /// Create a spread of colors starting from st to en, with n entries.
    void spread(ddgColor3 *st, ddgColor3 *en, float stk, float endk, unsigned int n );
};

#endif
