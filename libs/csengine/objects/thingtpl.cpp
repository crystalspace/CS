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

#include "cssysdef.h"
#include "csengine/thingtpl.h"
#include "csengine/polyset.h"
#include "ipolygon.h"

//---------------------------------------------------------------------------

IMPLEMENT_CSOBJTYPE (csThingTemplate,csObject);

csThingTemplate::csThingTemplate () : csObject (),
  polygons (32, 32), curves (16, 16)
{
  num_curve_vertices = max_curve_vertices = 0;
  curves_center.x = curves_center.y = curves_center.z = 0.0;
  curves_scale = 40;
  curve_vertices = NULL;
  curve_texels = NULL;

  fog.enabled = false;
}

csThingTemplate::~csThingTemplate ()
{
  delete [] curve_vertices;
  delete [] curve_texels;
}

void csThingTemplate::AddVertex (float x, float y, float z)
{
  int num = vertices.Length ();
  vertices.SetLength (num + 1, 16);

  vertices [num].x = x;
  vertices [num].y = y;
  vertices [num].z = z;
}

void csThingTemplate::AddCurveVertex (csVector3& v, csVector2& t)
{
  if (!curve_vertices)
  {
    max_curve_vertices = 16;
    curve_vertices = new csVector3 [max_curve_vertices];
    curve_texels   = new csVector2 [max_curve_vertices];
  }
  while (num_curve_vertices >= max_curve_vertices)
  {
    max_curve_vertices += 16;
    csVector3* new_vertices = new csVector3 [max_curve_vertices];
    csVector2* new_texels = new csVector2 [max_curve_vertices];
    memcpy (new_vertices, curve_vertices, sizeof (csVector3)*num_curve_vertices);
    memcpy (new_texels,   curve_texels,   sizeof (csVector2)*num_curve_vertices);
    delete [] curve_vertices;
    delete [] curve_texels;
    curve_vertices = new_vertices;
    curve_texels   = new_texels;
  }

  curve_vertices[num_curve_vertices] = v;
  curve_texels[num_curve_vertices] = t;
  num_curve_vertices++;
}

//---------------------------------------------------------------------------

csPolygonTemplate::csPolygonTemplate (csThingTemplate* iParent, char* iName,
  csMaterialHandle* iMaterial)
  : flags (CS_POLY_LIGHTING | CS_POLYTPL_TEXMODE_LIGHTMAP | CS_POLYTPL_TEXMODE_LIGHTMAP)
{
  parent = iParent;
  name = strnew (iName);
  material = iMaterial;
  uv_coords = NULL;
  colors = NULL;
}

csPolygonTemplate::~csPolygonTemplate ()
{
  delete [] name;
  delete [] colors;
  delete [] uv_coords;
}

void csPolygonTemplate::AddVertex (int v)
{
  int num = vertices.Length ();
  vertices.SetLength (num + 1);
  vertices [num] = v;
}

void csPolygonTemplate::SetUV (int i, float u, float v)
{
  if (!uv_coords) uv_coords = new csVector2 [vertices.Length ()];
  uv_coords [i].x = u;
  uv_coords [i].y = v;
}

void csPolygonTemplate::ResetUV ()
{
  delete [] uv_coords;
  uv_coords = NULL;
}

void csPolygonTemplate::SetColor (int i, const csColor &iCol)
{
  if (!colors) colors = new csColor [vertices.Length ()];
  colors [i] = iCol;
}

void csPolygonTemplate::PlaneNormal (float* A, float* B, float* C)
{
  float ayz = 0;
  float azx = 0;
  float axy = 0;
  int i, i1;
  float x1, y1, z1, x, y, z;

  i1 = vertices.Length ()-1;
  for (i = 0 ; i < vertices.Length () ; i++)
  {
    x = parent->Vtex (vertices [i]).x;
    y = parent->Vtex (vertices [i]).y;
    z = parent->Vtex (vertices [i]).z;
    x1 = parent->Vtex (vertices [i1]).x;
    y1 = parent->Vtex (vertices [i1]).y;
    z1 = parent->Vtex (vertices [i1]).z;
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
