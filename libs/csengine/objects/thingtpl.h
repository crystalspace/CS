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

#ifndef THINGTPL_H
#define THINGTPL_H

#include "csgeom/math3d.h"
#include "csgeom/math2d.h"  // texel coords
#include "csobject/csobj.h"
#include "csengine/basic/fog.h"
#include "csengine/cscolor.h"

class csPolygonTemplate;
class csTextureHandle;
class csCurveTemplate;

/**
 * This class represents a template for a Thing.
 * Using this template one can construct a Thing later.
 */
class csThingTemplate : public csObject
{
private:
  /// Vertices of this template.
  csVector3* vertices;
  /// Number of vertices.
  int num_vertices;
  /// Maximum number of vertices.
  int max_vertices;

  /**
   * List of polygontemplates.
   */
  csPolygonTemplate** polygon;
  /// Number of polygons.
  int num_polygon;
  /// Maximum number of polygons.
  int max_polygon;

  /**
   * List of curve templates.
   */
  csCurveTemplate** curves;
  /// Number of curves.
  int num_curves;
  /// Maximum number of polygons.
  int max_curves;


  /// Vertices to be used by curves
  csVector3* curve_vertices;
  /// Texture coords of curve vertices
  csVector2* curve_texels;

  /// Number of vertices.
  int num_curve_vertices;
  /// Maximum number of vertices.
  int max_curve_vertices;

  /// Fog structure.
  csFog fog;

 public:
  /**
   * Global thing parameters used for determining tesselation resolution
   */
  /// Center of thing to determine distance from
  csVector3 curves_center;
  /// scale param (the larger this param it, the more 
  /// the curves are tesselated)
  float   curves_scale;  

public:
  /// Add a vertex to this template.
  void AddVertex (float x, float y, float z);
  /// Add a vertex to this template.
  void AddVertex (csVector3& v) { AddVertex (v.x, v.y, v.z); }
  /// Add a polygon template to this thing template.
  void AddPolygon (csPolygonTemplate* p);

  void AddCurve (csCurveTemplate* p);
  int GetNumCurves () { return num_curves; }
  csCurveTemplate* GetCurve (int i) { return curves[i]; }

  ///
  int GetNumCurveVertices () { return num_curve_vertices; }
  ///
  csVector3& CurveVertex (int i) { return curve_vertices[i]; }
  ///
  csVector2& CurveTexel (int i) { return curve_texels[i]; }
  ///
  void AddCurveVertex (csVector3& v, csVector2& t);
  ///

public:
  ///
  csThingTemplate ();
  ///
  virtual ~csThingTemplate ();

  ///
  int GetNumVertices () { return num_vertices; }
  ///
  csVector3& Vtex (int i) { return vertices[i]; }
  ///
  int GetNumPolygon () { return num_polygon; }
  ///
  csPolygonTemplate* GetPolygon (int i) { return polygon[i]; }

  /// Return true if this has fog.
  bool HasFog () { return fog.enabled; }

  /// Return fog structure.
  csFog& GetFog () { return fog; }

  CSOBJTYPE;
};

/**
 * A csPolygonTemplate is used by a ThingTemplate to describe a
 * template for a polygon.
 */
class csPolygonTemplate
{
private:
  /// A table of indices into the vertices of the parent Polygon
  int* vertices_idx;
  /// Number of vertices for this polygon.
  int num_vertices;
  /// Maximum number of vertices.
  int max_vertices;
  ///
  char name[30];

  /// Should mipmapping be used for this polygon?
  int no_mipmap;
  /// Should this polygon be lighted?
  int no_lighting;
  /// The texture used for this polygon.
  csTextureHandle* texture;
  /// The transformation from object space to texture space.
  csMatrix3 m_obj2tex;
  /// The transformation from object space to texture space.
  csVector3 v_obj2tex;

  ///
  csThingTemplate* parent;

  /// Flat color to use instead of texture map.
  csColor flat_color;

  /// True if flat_color should be used.
  bool use_flat_color;

  /// True if gouraud shading used.
  bool use_gouraud;

public:
  ///
  csPolygonTemplate (csThingTemplate* parent, char* name, csTextureHandle* texture = NULL);
  ///
  virtual ~csPolygonTemplate ();

  ///
  int GetMaxVertices () { return max_vertices; }
  ///
  int GetNumVertices () { return num_vertices; }
  ///
  int* GetVerticesIdx () { return vertices_idx; }
  ///
  void AddVertex (int v);
  ///
  char* GetName () { return name; }

  /// Return true if flat color is used (instead of texture).
  bool UseFlatColor () { return use_flat_color; }

  /// Get the flat color for this polygon.
  csColor& GetFlatColor () { return flat_color; }

  /// Enable gouraud shading.
  void SetGouraud () { use_gouraud = true; }

  /// Is gouraud shading used?
  bool UseGouraud () { return use_gouraud; }

  /// Set the flat color for this polygon.
  void SetFlatColor (float r, float g, float b)
  {
    flat_color.red = r;
    flat_color.green = g;
    flat_color.blue = b;
    use_flat_color = true;
  }

  /// Set the flat color for this polygon.
  void SetFlatColor (csColor& fc) { flat_color = fc; use_flat_color = true; }

  /// Reset flat color (i.e. use texturing again).
  void ResetFlatColor () { use_flat_color = false; }

  /// Compute the plane normal of this polygon.
  void PlaneNormal (float* A, float* B, float* C);

  ///
  void SetTexture (csTextureHandle* texture) { csPolygonTemplate::texture = texture; }
  /// Set the transformation from object to texture space.
  void SetTextureSpace (csMatrix3& tx_matrix, csVector3& tx_vector);
  /// Transform the texture tranformation using the given matrix/vector.
  void Transform (csMatrix3& m, csVector3& v);
  ///
  csTextureHandle* GetTexture () { return texture; }
  ///
  csMatrix3& GetTextureMatrix () { return m_obj2tex; }
  ///
  csVector3& GetTextureVector () { return v_obj2tex; }

  ///
  void SetLighting (int yes) { no_lighting = !yes; }
  ///
  void SetMipmapping (int yes) { no_mipmap = !yes; }
  ///
  int IsLighted () { return !no_lighting; }
  ///
  int IsMipmapped () { return !no_mipmap; }
};

#endif /*THINGTPL_H*/
