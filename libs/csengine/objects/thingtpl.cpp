/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#include <math.h>

#include "sysdef.h"
#include "csengine/thingtpl.h"
#include "csengine/polyset.h"

//---------------------------------------------------------------------------

CSOBJTYPE_IMPL(csThingTemplate,csObject);

csThingTemplate::csThingTemplate () : csObject ()
{
  max_vertices = num_vertices = 0;
  max_polygon = num_polygon = 0;

  num_curves = max_curves = 0;
  curves=NULL;

  curves_center.x = curves_center.y = curves_center.z = 0.0;
  curves_scale = 40;
  curve_vertices = NULL;
  curve_texels = NULL;
  num_curve_vertices = max_curve_vertices = 0;

  vertices = NULL;
  polygon = NULL;
  fog.enabled = false;
}

csThingTemplate::~csThingTemplate ()
{
  CHK (delete [] vertices);
  if (polygon)
  {
    for (int i = 0; i < num_polygon; i++)
      CHKB (delete polygon [i]);
    CHK (delete [] polygon);
  }
  CHK (delete curve_vertices);
  CHK (delete curve_texels);
}

void csThingTemplate::AddVertex (float x, float y, float z)
{
  if (!vertices)
  {
    max_vertices = 10;
    CHK (vertices = new csVector3 [max_vertices]);
  }
  while (num_vertices >= max_vertices)
  {
    max_vertices += 10;
    CHK (csVector3* new_vertices = new csVector3 [max_vertices]);
    memcpy (new_vertices, vertices, sizeof (csVector3)*num_vertices);
    CHK (delete [] vertices);
    vertices = new_vertices;
  }

  vertices[num_vertices].x = x;
  vertices[num_vertices].y = y;
  vertices[num_vertices].z = z;
  num_vertices++;
}

void csThingTemplate::AddCurveVertex (csVector3& v, csVector2& t)
{
  if (!curve_vertices)
  {
    max_curve_vertices = 10;
    CHK (curve_vertices = new csVector3 [max_curve_vertices]);
    CHK (curve_texels   = new csVector2 [max_curve_vertices]);
  }
  while (num_curve_vertices >= max_curve_vertices)
  {
    max_curve_vertices += 10;
    CHK (csVector3* new_vertices = new csVector3 [max_curve_vertices]);
    CHK (csVector2* new_texels   = new csVector2 [max_curve_vertices]);
    memcpy (new_vertices, curve_vertices, sizeof (csVector3)*num_curve_vertices);
    memcpy (new_texels,   curve_texels,   sizeof (csVector2)*num_curve_vertices);
    CHK (delete [] curve_vertices);
    CHK (delete [] curve_texels);
    curve_vertices = new_vertices;
    curve_texels   = new_texels;
  }

  curve_vertices[num_curve_vertices] = v;
  curve_texels[num_curve_vertices] = t;
  num_curve_vertices++;
}


void csThingTemplate::AddCurve (csCurveTemplate* poly)
{
  if (!curves)
  {
    max_curves = 6;
    CHK (curves = new csCurveTemplate* [max_curves]);
  }
  while (num_curves >= max_curves)
  {
    max_curves += 6;
    CHK (csCurveTemplate** new_curves = new csCurveTemplate* [max_curves]);
    memcpy (new_curves, curves, sizeof (csCurveTemplate*)*num_curves);
    CHK (delete [] curves);
    curves = new_curves;
  }

  // Here we could try to include a test for the right orientation
  // of the polygon.
  curves[num_curves++] = poly;
}


void csThingTemplate::AddPolygon (csPolygonTemplate* poly)
{
  if (!polygon)
  {
    max_polygon = 6;
    CHK (polygon = new csPolygonTemplate* [max_polygon]);
  }
  while (num_polygon >= max_polygon)
  {
    max_polygon += 6;
    CHK (csPolygonTemplate** new_polygon = new csPolygonTemplate* [max_polygon]);
    memcpy (new_polygon, polygon, sizeof (csPolygonTemplate*)*num_polygon);
    CHK (delete [] polygon);
    polygon = new_polygon;
  }

  // Here we could try to include a test for the right orientation
  // of the polygon.
  polygon[num_polygon++] = poly;
}

//---------------------------------------------------------------------------

csPolygonTemplate::csPolygonTemplate (csThingTemplate* parent, char* name,
	csTextureHandle* texture)
{
  vertices_idx = NULL;
  max_vertices = num_vertices = 0;

  strcpy (csPolygonTemplate::name, name);
  csPolygonTemplate::texture = texture;
  csPolygonTemplate::parent = parent;

  no_mipmap = false;
  no_lighting = false;

  use_flat_color = false;
  use_gouraud = false;
}

csPolygonTemplate::~csPolygonTemplate ()
{
  CHK (delete [] vertices_idx);
}

void csPolygonTemplate::AddVertex (int v)
{
  if (!vertices_idx)
  {
    max_vertices = 4;
    CHK (vertices_idx = new int [max_vertices]);
  }
  while (num_vertices >= max_vertices)
  {
    max_vertices += 2;
    CHK (int* new_vertices_idx = new int [max_vertices]);
    memcpy (new_vertices_idx, vertices_idx, sizeof (int)*num_vertices);
    CHK (delete [] vertices_idx);
    vertices_idx = new_vertices_idx;
  }

  vertices_idx[num_vertices++] = v;
}

void csPolygonTemplate::PlaneNormal (float* A, float* B, float* C)
{
  float ayz = 0;
  float azx = 0;
  float axy = 0;
  int i, i1;
  float x1, y1, z1, x, y, z;

  i1 = num_vertices-1;
  for (i = 0 ; i < num_vertices ; i++)
  {
    x = parent->Vtex (vertices_idx[i]).x;
    y = parent->Vtex (vertices_idx[i]).y;
    z = parent->Vtex (vertices_idx[i]).z;
    x1 = parent->Vtex (vertices_idx[i1]).x;
    y1 = parent->Vtex (vertices_idx[i1]).y;
    z1 = parent->Vtex (vertices_idx[i1]).z;
    ayz += (z1+z) * (y-y1);
    azx += (x1+x) * (z-z1);
    axy += (y1+y) * (x-x1);
    i1 = i;
  }

  float d = sqrt (ayz*ayz + azx*azx + axy*axy);

  if (d < SMALL_EPSILON) d = SMALL_EPSILON;

  *A = ayz / d;
  *B = azx / d;
  *C = axy / d;
}

void csPolygonTemplate::SetTextureSpace (csMatrix3& tx_matrix, csVector3& tx_vector)
{
  m_obj2tex = tx_matrix;
  v_obj2tex = tx_vector;
}

void csPolygonTemplate::Transform (csMatrix3& m, csVector3& v)
{
  (void)m; (void)v;
  // ??? @@@ What needs to be done here?
}

//---------------------------------------------------------------------------

