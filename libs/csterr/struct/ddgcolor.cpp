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
#include <stdio.h>

#include "util/ddgerror.h"
#include "struct/ddgcolor.h"
// ----------------------------------------------------------------------

ddgColor3::ddgColor3( void )
{
	v[0] = 255;
	v[1] = 255;
	v[2] = 255;
}

ddgColor3::ddgColor3( unsigned char r, unsigned char g, unsigned char b)
{
	v[0] = r;
	v[1] = g;
	v[2] = b;
}

ddgColor3::ddgColor3( const ddgColor3 *c)
{
	v[0] = c->v[0];
	v[1] = c->v[1];
	v[2] = c->v[2];
}

#ifdef DDGSTREAM
// Define an input and output stream operator for ddgColor3.
// Output format is "<xxx,yyy,zzz>"
ostream& operator << ( ostream&s, ddgColor3 v )
     { return s <<'<'<<(unsigned int)v[0]<<','<<(unsigned int)v[1]<<','<<(unsigned int)v[2]<<'>'; }
ostream& operator << ( ostream&s, ddgColor3* v )
     { return s <<'<'<<(unsigned int)v->v[0]<<','<<(unsigned int)v->v[1]<<','<<(unsigned int)v->v[2]<<'>'; }
// Input format is "<xxx,yyy,zzz>"
istream& operator >> ( istream& s, ddgColor3& v)
     { char c; s >> c >> v.v[0] >> c >> v.v[1] >> c >> v.v[2]>> c; return s; }
#endif

ddgColor4::ddgColor4( void )
{
	v[0] = 255;
	v[1] = 255;
	v[2] = 255;
	v[3] = 255;
}

ddgColor4::ddgColor4( unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	v[0] = r;
	v[1] = g;
	v[2] = b;
	v[3] = a;
}

void ddgColor4::test(ddgColor4 *cs, ddgColor4 *ce )
{
	(void)cs; (void)ce;
#ifdef DDGSTREAM
	ddgColor4 cn,cx,cc,cb;
	float f;
	cerr << "Linear Exponential Cosine Binlinear" << endl;
	for (f = 0; f < 1.04; f = f + 0.1)
	{
		cn.linterp(cs,ce,f);
		cx.einterp(cs,ce,f);
		cc.cinterp(cs,ce,f);
		cb.binterp(cs,ce,f);
		cerr << f << " "
			<< cn << " "
			<< cx << " "
			<< cc << " "
			<< cb << endl;
	}
#endif
}

#ifdef DDGSTREAM
// Define an input and output stream operator for ddgColor4.
// Output format is "<xxx,yyy,zzz,ttt>"
ostream& operator << ( ostream&s, ddgColor4 v )
{
	return s <<'<'<<(unsigned int)v[0]<<','<<(unsigned int)v[1]<<','<<(unsigned int)v[2]<<','<<(unsigned int)v[3]<<'>';
}
ostream& operator << ( ostream&s, ddgColor4* v )
{
	return s <<'<'<<(unsigned int)v->v[0]<<','<<(unsigned int)v->v[1]<<','<<(unsigned int)v->v[2]<<','<<(unsigned int)v->v[3]<<'>';
}
// Input format is "<xxx,yyy,zzz,ttt>"
istream& operator >> ( istream& s, ddgColor4& v)
     { char c; s >> c >>v.v[0] >> c >> v.v[1] >> c >> v.v[2]>> c >> v.v[3] >> c; return s; }


// Define an output stream operator for ddgColorNode.
// Output format is "name [key] {color}"

ostream& operator << ( ostream&s, ddgColorNode c )
{
	if (c.is4())
		return s << ' ' << (c.name()?c.name():"(null)") << '[' << c.key() << ']' << c.color4;
	else
		return s << ' ' << (c.name()?c.name():"(null)") << '[' << c.key() << ']' << c.color3;
}

ostream& operator << ( ostream&s, ddgColorNode* c )
{
	if (c->is4())
		return s << ' ' << (c->name()?c->name():"(null)") << '[' << c->key() << ']' << c->color4;
	else
		return s << ' ' << (c->name()?c->name():"(null)") << '[' << c->key() << ']' << c->color3;
}
#endif


void ddgColorSet::spread(ddgColor3* st, ddgColor3 *en, float stk, float endk,
					  unsigned int no )
{
	ddgColor3 delta(en);
	float   curk = endk, d = (endk - stk) / (no-1);
	delta.v[0] -= st->v[0];
	delta.v[1] -= st->v[1];
	delta.v[2] -= st->v[2];
	delta.v[0] /= no-1;
	delta.v[1] /= no-1;
	delta.v[2] /= no-1;
	ddgColor3 cur(en);
	while(no)
	{
		add(new ddgColorNode(ddgColor3(cur),curk));
		cur.v[0] -=delta[0];
		cur.v[1] -=delta[1];
		cur.v[2] -=delta[2];
		curk -= d;
		no--;
	}
}
