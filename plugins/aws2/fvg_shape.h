/*
    Copyright (C) 2005 by Christopher Nelson

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

#ifndef __AWS_FVG_SHAPE_H__
#define __AWS_FVG_SHAPE_H__

#include "csgeom/triangulate.h"
#include "csutil/cscolor.h"
#include "cstool/pen.h"

/** file Flexible Vector Graphics shape definitions.
 */

namespace aws
{
  namespace fvg
  {

    /** The attributes of any given shape. If the shape is stroked AND filled, it will be filled first,
    * and stroked second. */
    struct shape_attr
    {
      /** The color to stroke the shape in. */
      csColor4 stroke_color;

      /** The color to fill the shape in. */
      csColor4 fill_color;

      /** The rotation vector. */
      csVector2 rotation;

      /** The translation vector. */
      csVector2 translation;

      /** The scaling vector. */
      csVector2 scale;

      /** True if this shape is stroked. */
      bool stroked;

      /** True if this shape is filled. */
      bool filled;   

      /** True is this shape is translated. */
      bool translated;

      /** True if this shape is rotated. */
      bool rotated;

      /** True is this shape is scaled. */
      bool scaled;
    };

    /** The base class for all fvg shapes. */
    class shape
    {
    protected:
      /** The attributes of this shape. */
      shape_attr attr;

    public:
      /** Set the attributes for this shape. */
      void SetAttr(shape_attr &_attr) { attr = _attr; }

      /** Read the attributes for this shape. */
      const shape_attr& GetAttr()	{ return attr;  } 

      /** Draw this shape. */
      virtual void Draw(iPen *pen)=0;
    };

    enum RECT_STYLE { RECT_NORMAL, RECT_ROUNDED, RECT_MITERED };

    /** A rectangular shape. */
    class rect : public shape
    {
      /** The smoothness factor for rounded/mitered rects. */
      float roundness;

      /** The top left and bottom right coords for the rect. */
      csVector2 tl, br;

      /** The style of the rectangle to draw. */
      RECT_STYLE rect_style;
      
      void draw_rect(iPen *pen, bool fill=false)
      {
	switch(rect_style)
	{
	  case RECT_NORMAL:
	    pen->DrawRect(tl.x, tl.y, br.x, br.y, false, fill);
	  break;

	  case RECT_ROUNDED:
	    pen->DrawRoundedRect(tl.x, tl.y, br.x, br.y, roundness, false, fill);
	  break;

	  case RECT_MITERED:
	    pen->DrawMiteredRect(tl.x, tl.y, br.x, br.y, roundness, false, fill);
	  break;
	}
      }

    public:
      rect(RECT_STYLE _rect_style, const csVector2 &_tl, const csVector2 &_br, float _roundness_or_miter=0.0):roundness(_roundness_or_miter),
											 tl(_tl), br(_br),
											 rect_style(_rect_style) {}

      virtual ~rect() {}

      /** Draws the rectangle according to the shape and style information present. */
      virtual void Draw(iPen *pen)
      {
        if (attr.filled)
	{
	  pen->SetColor(attr.fill_color);
	  draw_rect(pen, true);
	}

	if (attr.stroked)
	{
	   pen->SetColor(attr.stroke_color);
	   draw_rect(pen, false);
	}
      }
    };

    /** An elliptical shape, can also do arcs. */
    class ellipse : public shape
    {    
      /** The top left and bottom right coords for the ellipse. */
      csVector2 tl, br;

      /** The start angle. */
      float sa;

      /** The end angle. */
      float ea;      
    
    public:
      ellipse(csVector2 &_tl, csVector2 &_br, float _sa=0.0, float _ea=6.2831853):tl(_tl), br(_br), sa(_sa), ea(_ea) {}
									
      virtual ~ellipse() {}

      /** Draws the ellipse according to the shape and style information present. */
      virtual void Draw(iPen *pen)
      {
        if (attr.filled)
	{
	  pen->SetColor(attr.fill_color);
	  pen->DrawArc(tl.x, tl.y, br.x, br.y, sa, ea, false, true);
	}

	if (attr.stroked)
	{
	   pen->SetColor(attr.stroke_color);
	   pen->DrawArc(tl.x, tl.y, br.x, br.y, sa, ea, false, false);
	}
      }
    };

     /** A line shape. */
    class line : public shape
    {    
      /** The top left and bottom right coords for the line. */
      csVector2 tl, br;
    
    public:
      line(csVector2 &_tl, csVector2 &_br):tl(_tl), br(_br) {}
									
      virtual ~line() {}

      /** Draws the ellipse according to the shape and style information present. */
      virtual void Draw(iPen *pen)
      {
	if (attr.stroked)
	{
	   pen->SetColor(attr.stroke_color);
	   pen->DrawLine(tl.x, tl.y, br.x, br.y);
	}
      }
    };

    /** An arbitrary polygon.  If it is closed it may be filled.  Otherwise it
     * may only be stroked. */
    class polygon : public shape
    {
      /** The contour of the polygon. */
      csContour2 poly;

      /** The resulting vertices for the triangulated polygon. */
      csContour2 tri_verts;

      /** The triangle mesh that for the triangulated polygon. */
      csTriangleMesh tri_mesh;

      /** True if it is closed.  False otherwise. */
      bool closed;

    public:
      polygon():closed(false) {}
      virtual ~polygon() {}

      /** Adds a vertex to the polygon. */
      void AddVertex(float x, float y)
      {
	poly.Push(csVector2(x,y));
      }

      /** Adds a vertex to the polygon. */
      void AddVertex(const csVector2 &v)
      {
	poly.Push(v);
      }

      /** Closes the polygon.  It is assumed that the last vertex added will
       * connect to the first vertex added. */
      void Close() 
      {
	::csTriangulate2 t;

	if (closed) return;

	closed=true;

	t.Process(poly, tri_mesh, tri_verts);
      }
    
      /** Draws the ellipse according to the shape and style information present. */
      virtual void Draw(iPen *pen)
      {
        if (attr.filled && closed)
	{
	  pen->SetColor(attr.fill_color);

	  // Draw all the triangles generated from the polygon.          
	  for(size_t i=0; i<tri_mesh.GetTriangleCount(); ++i)
	  {
	    csTriangle &tri = tri_mesh.GetTriangle((int)i);
            
	    pen->DrawTriangle(tri_verts[tri.a].x, tri_verts[tri.a].y,
			      tri_verts[tri.b].x, tri_verts[tri.b].y,
			      tri_verts[tri.c].x, tri_verts[tri.c].y, true);
	  }
	}

	if (attr.stroked)
	{
	   pen->SetColor(attr.stroke_color);

	   for(size_t i=0; i<poly.Length()-1; ++i)
	   {
             pen->DrawLine(poly[i].x, poly[i].y, poly[i+1].x, poly[i+1].y);
	   }
	}
      }
    };

    
      

  }; // end fvg namespace  
}; // end aws namespace

#endif
