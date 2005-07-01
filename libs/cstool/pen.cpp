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

#include "cssysdef.h"
#include "cstool/pen.h"

#include <math.h>
#include <float.h>

#ifndef M_PI
#define M_PI   3.14159265358979323846f
#endif

csPen::csPen(iGraphics3D *_g3d):g3d(_g3d)
{

}

csPen::~csPen()
{

}

void csPen::start()
{
  va.MakeEmpty();
  ia.SetLength(0);  
  colors.SetLength(0);
}

void csPen::vert(float x, float y)
{
  ia.Push((uint)va.AddVertexSmart(x,y,0));
  colors.Push(color);
}

void csPen::setupMesh()
{
  mesh.vertices =    va.GetVertices();
  mesh.vertexCount = static_cast<uint>(va.GetVertexCount());

  mesh.indices = ia.GetArray();
  mesh.indexCount = static_cast<uint>(ia.Length());

  mesh.colors = colors.GetArray();
  //mesh.colorCount = static_cast<uint>(colors.Length());  
}

void csPen::drawMesh(csRenderMeshType mesh_type)
{
  mesh.meshtype = mesh_type;
  g3d->DrawSimpleMesh(mesh, csSimpleMeshScreenspace);
}

void csPen::setColor(int32 r, int32 g, int32 b, int32 a)
{
  color.Set(r,g,b,a);
}

/** Draws a single line. */
void csPen::line(int32 x1, int32 y1, int32 x2, int32 y2)
{
  start();
  vert(x1,y1);
  vert(x2,y2);

  setupMesh();
  drawMesh(CS_MESHTYPE_LINES);
}

/** Draws a single point. */
void csPen::point(int32 x1, int32 y1)
{
  start();
  vert(x1,y1);
  
  setupMesh(); 
  drawMesh(CS_MESHTYPE_POINTS);
}

/** Draws a rectangle. */
void csPen::rect(int32 x1, int32 y1, int32 x2, int32 y2, bool fill)
{
  start();
  vert(x1, y1);
  vert(x1, y2);
  vert(x2, y2);
  vert(x1, y2);

  setupMesh();
  drawMesh(fill ? CS_MESHTYPE_QUADS : CS_MESHTYPE_LINESTRIP);
}

/** Draws a mitered rectangle. The miter value should be between 0.0 and 1.0, and determines how
* much of the corner is mitered off and beveled. */    
void csPen::mitered_rect(int32 x1, int32 y1, int32 x2, int32 y2, float miter,  bool fill)
{
  if (miter==0) { rect(x1,y1,x2,y2,fill); return; }
			
  float width=x2-x1;
  float height=y2-y1;

  float y_miter = (height*miter)*0.5;
  float x_miter = (width*miter)*0.5;
  		
  start();

  vert(x1, y2-y_miter);
  vert(x1, y1+y_miter);
  vert(x1+x_miter, y1);
  vert(x2-x_miter, y1);
  vert(x2, y1+y_miter);
  vert(x2, y2-y_miter);
  vert(x2-x_miter, y2);
  vert(x1+x_miter, y2);

  setupMesh();
  drawMesh(fill ? CS_MESHTYPE_POLYGON : CS_MESHTYPE_LINESTRIP);
}

/** Draws a rounded rectangle. The roundness value should be between 0.0 and 1.0, and determines how
  * much of the corner is rounded off. */
void csPen::rounded_rect(int32 x1, int32 y1, int32 x2, int32 y2, float roundness,  bool fill)
{
  if (roundness==0) { rect(x1,y1,x2,y2,fill); return; }
			
  float width=x2-x1;
  float height=y2-y1;

  float y_round = (height*roundness)*0.5;
  float x_round = (width*roundness)*0.5;
  float   steps = roundness * 12;
  float   delta = (M_PI/4.0)/steps;

  start();
  			
  for(float angle=(M_PI/2.0)*3.0; angle>M_PI; angle-=delta)
	  vert(x1+x_round+(cos(angle)*x_round), y2-y_round-(sin(angle)*y_round));
	  
  vert(x1, y2-y_round);
  vert(x1, y1+y_round);
  
  for(float angle=M_PI; angle>M_PI/2.0; angle-=delta)
	  vert(x1+x_round+(cos(angle)*x_round), y1+y_round-(sin(angle)*y_round));				
  
  vert(x1+x_round, y1);
  vert(x2-x_round, y1);
  
  for(float angle=M_PI/2.0; angle>0; angle-=delta)
	  vert(x2-x_round+(cos(angle)*x_round), y1+y_round-(sin(angle)*y_round));
  
  vert(x2, y1+y_round);
  vert(x2, y2-y_round);
  
  for(float angle=M_PI*2.0; angle>(M_PI/2.0)*3.0; angle-=delta)
	  vert(x2-x_round+(cos(angle)*x_round), y2-y_round-(sin(angle)*y_round));
  
  vert(x2-x_round, y2);
  vert(x1+x_round, y2);				

  setupMesh();
  drawMesh(fill ? CS_MESHTYPE_POLYGON : CS_MESHTYPE_LINESTRIP);
}
