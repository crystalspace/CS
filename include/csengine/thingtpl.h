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

#include "csutil/cscolor.h"
#include "csutil/garray.h"
#include "csutil/flags.h"
#include "csgeom/vector2.h"
#include "csgeom/matrix3.h"
#include "csobject/csobject.h"
#include "csengine/arrays.h"
#include "igraph3d.h"

class csPolygonTemplate;
class csMaterialWrapper;
class csCurveTemplate;

/**
 * This class represents a template for a Thing.
 * Using this template one can construct a Thing later.
 */
class csThingTemplate : public csObject
{
private:
  /// Vertices of this template.
  DECLARE_GROWING_ARRAY (vertices, csVector3);

  /// List of polygontemplates.
  csPolygonTemplateArray polygons;

  /// List of curve templates.
  csCurveTemplateArray curves;

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
  /**
   * Curves scale parameter (the larger this param it, the more 
   * the curves are tesselated).
   */
  float curves_scale;  

public:
  /// Add a vertex to this template.
  void AddVertex (float x, float y, float z);
  /// Add a vertex to this template.
  void AddVertex (const csVector3& v)
  { AddVertex (v.x, v.y, v.z); }
  /// Add a polygon template to this thing template.
  void AddPolygon (csPolygonTemplate* poly)
  { polygons.Push (poly); }

  /// Add a curve surface template
  void AddCurve (csCurveTemplate* curve)
  { curves.Push (curve); }
  /// Get number of curve surface templates
  int GetNumCurves ()
  { return curves.Length (); }
  /// Get Nth curve surface template
  csCurveTemplate* GetCurve (int i)
  { return curves.Get (i); }

  ///
  int GetNumCurveVertices ()
  { return num_curve_vertices; }
  ///
  csVector3& CurveVertex (int i)
  { return curve_vertices[i]; }
  ///
  csVector2& CurveTexel (int i)
  { return curve_texels[i]; }
  ///
  void AddCurveVertex (csVector3& v, csVector2& t);

public:
  ///
  csThingTemplate ();
  ///
  virtual ~csThingTemplate ();

  ///
  int GetNumVertices ()
  { return vertices.Length (); }
  ///
  csVector3& Vtex (int i)
  { return vertices[i]; }

  ///
  int GetNumPolygon ()
  { return polygons.Length (); }
  /// Get the Nth polygon
  csPolygonTemplate* GetPolygon (int i)
  { return polygons.Get (i); }

  /// Return true if this has fog.
  bool HasFog () { return fog.enabled; }

  /// Return fog structure.
  csFog& GetFog () { return fog; }

  CSOBJTYPE;
};

/**
 * Polygon template flags; "normal" polygon flags
 * such as CS_POLY_LIGHTING or CS_POLY_COLLDET
 * are applicable as well. WARNING: make sure these
 * flags don't use same bits as CS_POLY_XXX (as defined
 * in ipolygon.h and csengine/polygon.h)
 */
/// Collision detection state mask for this polygon
#define CS_POLYTPL_COLLDET		0x000c0000
/// Enable collision detection
#define CS_POLYTPL_COLLDET_ENABLE	0x00040000
/// Disable collision detection
#define CS_POLYTPL_COLLDET_DISABLE	0x00080000
/// Polygon texturing mode mask
#define CS_POLYTPL_TEXMODE		0x00030000
/// No texturing
#define CS_POLYTPL_TEXMODE_NONE		0x00000000
/// Flat lighted texture
#define CS_POLYTPL_TEXMODE_FLAT		0x00010000
/// Gouraud lighted texture
#define CS_POLYTPL_TEXMODE_GOURAUD	0x00020000
/// Lightmapped texture
#define CS_POLYTPL_TEXMODE_LIGHTMAP	0x00030000

/**
 * A csPolygonTemplate is used by a ThingTemplate to describe a
 * template for a polygon.
 */
class csPolygonTemplate
{
private:
  /// Polygon name
  char *name;
  /// A table of indices into the vertices of the parent thing template
  DECLARE_GROWING_ARRAY (vertices, int);

  /// The material used for this polygon.
  csMaterialWrapper* material;
  /// The transformation from object space to texture space.
  csMatrix3 m_obj2tex;
  /// The transformation from object space to texture space.
  csVector3 v_obj2tex;
  /// Optional uv coordinates.
  csVector2 *uv_coords;
  /// Optional vertex colors
  csColor *colors;

  /// Parent template object
  csThingTemplate* parent;

public:
  /// Polygon flags
  csFlags flags;

  ///
  csPolygonTemplate (csThingTemplate* iParent, char* iName, csMaterialWrapper* iMaterial = NULL);
  ///
  virtual ~csPolygonTemplate ();

  ///
  int GetNumVertices () { return vertices.Length (); }
  ///
  int* GetVerticesIdx () { return vertices.GetArray (); }
  ///
  void AddVertex (int v);
  ///
  char* GetName () { return name; }

  /// Set UV coordinate for specified vertex.
  void SetUV (int i, float u, float v);

  /// Reset UV coordinate array.
  void ResetUV ();

  /// Get the pointer to the vertex uv coordinates.
  csVector2* GetUVCoords () { return uv_coords; }

  /// Set color value for specified vertex.
  void SetColor (int i, const csColor &iCol);

  /// Same but given separate R,G,B values
  void SetColor (int i, float iR, float iG, float iB)
  { SetColor (i, csColor (iR, iG, iB)); }

  /// Get the pointer to the vertex uv coordinates.
  csColor* GetColors () { return colors; }

  /// Compute the plane normal of this polygon.
  void PlaneNormal (float* A, float* B, float* C);

  ///
  void SetMaterial (csMaterialWrapper* material) { csPolygonTemplate::material = material; }
  /// Set the transformation from object to texture space.
  void SetTextureSpace (csMatrix3& tx_matrix, csVector3& tx_vector);
  /// Transform the texture tranformation using the given matrix/vector.
  void Transform (csMatrix3& m, csVector3& v);
  ///
  csMaterialWrapper* GetMaterial () { return material; }
  ///
  csMatrix3& GetTextureMatrix () { return m_obj2tex; }
  ///
  csVector3& GetTextureVector () { return v_obj2tex; }
};

#endif /*THINGTPL_H*/
