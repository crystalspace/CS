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
#include "ivideo/fontserv.h"
#include "ivideo/graph2d.h"


csPen::csPen (iGraphics2D *_g2d, iGraphics3D *_g3d) : g3d (_g3d), g2d(_g2d)
{
  mesh.object2world.Identity();
}

csPen::~csPen ()
{

}

void csPen::Start ()
{
  poly.MakeEmpty();
  poly_idx.MakeEmpty();
  colors.SetLength (0);  
}

void csPen::AddVertex (float x, float y)
{
  poly_idx.AddVertex((int)poly.AddVertex(x,y,0));
  colors.Push(color);
}

void csPen::SetupMesh ()
{
  mesh.vertices = poly.GetVertices ();
  mesh.vertexCount = (uint)poly.GetVertexCount ();

  mesh.indices = (uint *)poly_idx.GetVertexIndices ();
  mesh.indexCount = (uint)poly_idx.GetVertexCount ();

  mesh.colors = colors.GetArray ();
  //mesh.colorCount = static_cast<uint>(colors.Length());  

  mesh.mixmode = CS_FX_COPY | CS_FX_FLAT;  
}

void csPen::DrawMesh (csRenderMeshType mesh_type)
{
  mesh.meshtype = mesh_type;
  g3d->DrawSimpleMesh (mesh, csSimpleMeshScreenspace);
}

void csPen::SetColor (float r, float g, float b, float a)
{
  color.x=r;
  color.y=g;
  color.z=b;
  color.w=a;
}

void csPen::SetColor(const csColor4 &c)
{
  color.x=c.red;
  color.y=c.green;
  color.z=c.blue;
  color.w=c.alpha;
}

void csPen::SwapColors()
{
  csVector4 tmp;

  tmp=color;
  color=alt_color;
  alt_color=tmp;
}

void csPen::ClearTransform()
{
  mesh.object2world.Identity();
}

void csPen::PushTransform()
{
  transforms.Push(mesh.object2world);
}

void csPen::PopTransform()
{
  ClearTransform();

  mesh.object2world*=transforms.Top();
  transforms.Pop();
}

void csPen::SetOrigin(const csVector3 &o)
{
  mesh.object2world.SetOrigin(o);
}

void 
csPen::Translate(const csVector3 &t)
{
  csTransform tr;

  tr.Translate(t);
  mesh.object2world*=tr;

  tt+=t;
}

void 
csPen::Rotate(const float &a)
{
  csZRotMatrix3 rm(a);
  csTransform tr(rm, csVector3(0));
  mesh.object2world*=tr;
}


/** Draws a single line. */
void csPen::DrawLine (uint x1, uint y1, uint x2, uint y2)
{
  Start ();
  AddVertex (x1,y1);
  AddVertex (x2,y2);

  SetupMesh ();
  DrawMesh (CS_MESHTYPE_LINES);
}

/** Draws a single point. */
void csPen::DrawPoint (uint x1, uint y1)
{
  Start ();
  AddVertex (x1,y1);
  
  SetupMesh (); 
  DrawMesh (CS_MESHTYPE_POINTS);
}

/** Draws a rectangle. */
void csPen::DrawRect (uint x1, uint y1, uint x2, uint y2, bool swap_colors, bool fill)
{
  Start ();

  AddVertex (x1, y1);
  AddVertex (x2, y1);

  if (swap_colors) SwapColors();

  AddVertex (x2, y2);
  AddVertex (x1, y2);

  if (swap_colors) SwapColors();

  if (!fill) AddVertex (x1, y1);

  

  SetupMesh ();
  DrawMesh (fill ? CS_MESHTYPE_QUADS : CS_MESHTYPE_LINESTRIP);
}

/** Draws a mitered rectangle. The miter value should be between 0.0 and 1.0, and determines how
* much of the corner is mitered off and beveled. */    
void csPen::DrawMiteredRect (uint x1, uint y1, uint x2, uint y2, 
                             float miter, bool swap_colors, bool fill)
{
  if (miter == 0.0f) 
  { 
    DrawRect (x1,y1,x2,y2,fill); 
    return; 
  }
			
  float width = x2-x1;
  float height = y2-y1;

  float center_x = x1+(width/2);
  float center_y = y1+(height/2);

  float y_miter = (height*miter)*0.5;
  float x_miter = (width*miter)*0.5;
  		
  Start ();

  if (fill)AddVertex(center_x, center_y);

  AddVertex (x1, y2-y_miter);
  AddVertex (x1, y1+y_miter);  
  AddVertex (x1+x_miter, y1);
  AddVertex (x2-x_miter, y1);  
  AddVertex (x2, y1+y_miter);

  if (swap_colors) SwapColors();

  AddVertex (x2, y2-y_miter);
  AddVertex (x2-x_miter, y2);
  AddVertex (x1+x_miter, y2);
  AddVertex (x1, y2-y_miter);

  if (swap_colors) SwapColors();

  SetupMesh ();
  DrawMesh (fill ? CS_MESHTYPE_TRIANGLEFAN: CS_MESHTYPE_LINESTRIP);
}

/** Draws a rounded rectangle. The roundness value should be between 0.0 and 1.0, and determines how
  * much of the corner is rounded off. */
void csPen::DrawRoundedRect (uint x1, uint y1, uint x2, uint y2, 
                             float roundness, bool swap_colors, bool fill)
{
  if (roundness == 0.0f) 
  { 
    DrawRect (x1,y1,x2,y2,fill); 
    return; 
  }
			
  float width = x2-x1;
  float height = y2-y1;
  
  float center_x = x1+(width/2);
  float center_y = y1+(height/2);

  float y_round = (height*roundness)*0.5;
  float x_round = (width*roundness)*0.5;
  float   steps = roundness * 12;
  float   delta = (PI/4.0)/steps;

  Start();

  float angle;

  if (fill)AddVertex(center_x, center_y);
  			
  for(angle=(HALF_PI)*3.0f; angle>PI; angle-=delta)
  {
    AddVertex (x1+x_round+(cosf (angle)*x_round), y2-y_round-(sinf (angle)*y_round));
  }
	  
  AddVertex (x1, y2-y_round);
  AddVertex (x1, y1+y_round);
  
  for(angle=PI; angle>HALF_PI; angle-=delta)
  {
	  AddVertex (x1+x_round+(cosf (angle)*x_round), y1+y_round-(sinf (angle)*y_round));
  }
  
  AddVertex (x1+x_round, y1);
  AddVertex (x2-x_round, y1);

  if (swap_colors) SwapColors();
  
  for(angle=HALF_PI; angle>0; angle-=delta)
  {
    AddVertex (x2-x_round+(cosf (angle)*x_round), y1+y_round-(sinf (angle)*y_round));
  }
  
  AddVertex (x2, y1+y_round);
  AddVertex (x2, y2-y_round);
  
  for(angle=TWO_PI; angle>HALF_PI*3.0; angle-=delta)
  {
    AddVertex (x2-x_round+(cosf (angle)*x_round), y2-y_round-(sinf (angle)*y_round));
  }
  
  AddVertex (x2-x_round, y2);
  AddVertex (x1+x_round, y2);		

  if (swap_colors) SwapColors();

  SetupMesh ();
  DrawMesh (fill ? CS_MESHTYPE_TRIANGLEFAN : CS_MESHTYPE_LINESTRIP);
}

/** 
   * Draws an elliptical arc from start angle to end angle.  Angle must be specified in radians.
   * The arc will be made to fit in the given box.  If you want a circular arc, make sure the box is
   * a square.  If you want a full circle or ellipse, specify 0 as the start angle and 2*PI as the end
   * angle.
   */
void csPen::DrawArc(uint x1, uint y1, uint x2, uint y2, float start_angle, float end_angle, bool swap_colors, bool fill)
{
  float width = x2-x1;
  float height = y2-y1;

  if (width==0 || height==0) return;

  float x_radius = width/2;
  float y_radius = height/2;
  
  float center_x = x1+(x_radius);
  float center_y = y1+(y_radius);
  
  // This is a totally made-up metric.  The idea is to make the circle or arc smoother as it gets larger by increasing the number of steps to take.  
  float steps = (width*height) * 0.01;

  // Make sure that the number of steps we take is never below 1. (This will cause an inversion and the number of steps we take will grow vastly.)
  while(steps<1) steps*=10;

  float delta = (end_angle-start_angle) / steps;
  float angle;

  Start();
  
  if (fill) AddVertex(center_x, center_y);

  for(angle=start_angle; angle<=end_angle; angle+=delta)
  {
    AddVertex(center_x+(cos(angle)*x_radius), center_y+(sin(angle)*y_radius));
  }

  SetupMesh ();
  DrawMesh (fill ? CS_MESHTYPE_TRIANGLEFAN : CS_MESHTYPE_LINESTRIP);
}

void csPen::DrawTriangle(uint x1, uint y1, uint x2, uint y2, uint x3, uint y3, bool fill)
{
  Start();

  AddVertex(x1, y1);
  AddVertex(x2, y2);
  AddVertex(x3, y3);

  SetupMesh ();
  DrawMesh (fill ? CS_MESHTYPE_TRIANGLES : CS_MESHTYPE_LINESTRIP);
}

void 
csPen::Write(iFont *font, uint x1, uint y1, char *text)
{
  if (font==0) return;

  int the_color = g2d->FindRGB(static_cast<int>(color.x*255), 
		 	       static_cast<int>(color.y*255), 
			       static_cast<int>(color.z*255),
			       static_cast<int>(color.w*255));

  csVector3 pos(x1,y1,0);

  pos += tt;

  g2d->Write(font, (int)pos.x, (int)pos.y, the_color, -1, text);  
}

void 
csPen::WriteBoxed(iFont *font, uint x1, uint y1, uint x2, uint y2, uint h_align, uint v_align, char *text)
{
  if (font==0) return;

  uint x, y;
  int w, h;

  // Get the maximum dimensions of the text.
  font->GetDimensions(text, w, h);

  // Figure out the correct starting point in the box for horizontal alignment.
  switch(h_align)
  {
    case CS_PEN_TA_LEFT:
    default:
      x=x1;
    break;

    case CS_PEN_TA_RIGHT:
      x=x2-w;
    break;
  
    case CS_PEN_TA_CENTER:
      x=x1+((x2-x1)>>1) - (w>>1);
    break;
  }

  // Figure out the correct starting point in the box for vertical alignment.
  switch(v_align)
  {
    case CS_PEN_TA_TOP:
    default:
      y=y1;
    break;

    case CS_PEN_TA_BOT:
      y=y2-h;
    break;
  
    case CS_PEN_TA_CENTER:
      y=y1+((y2-y1)>>1) - (h>>1);
    break;
  }

  Write(font, x, y, text);
}
