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

#ifndef CURVE_H
#define CURVE_H

#include "csutil/scf.h"
#include "csutil/csvector.h"
#include "csgeom/math3d.h"
#include "csgeom/math2d.h"
#include "csengine/bezier.h"
#include "csengine/texture.h"
#include "csengine/lghtmap.h"
#include "csengine/rview.h"
#include "csobject/csobject.h"

struct iTextureHandle;
class csBspContainer;

/**
 * Vertex resulting from a tesselated curve.
 */
class csCurveVertex
{
public:
  /// Object space coordinates.
  csVector3 object_coord;
  /// Texture coordinates.
  csVector2 txt_coord;
  /// Original control points.
  csVector2 control;
};

/**
 * Triangle resulting from a tesselated curve.
 */
class csCurveTriangle
{
public:
  /// Indices in triangle list.
  int i1, i2, i3;
};

/**
 * Tesselated curve. This is basicly a list of triangles.
 */
class csCurveTesselated
{
private:
  csCurveVertex* vertices;
  int num_vertices;
  csCurveTriangle* triangles;
  int num_triangles;

public:
  /**
   * Allocate a new tesselated curve with the given
   * number of vertices and triangles.
   */
  csCurveTesselated (int num_v, int num_t);
  ///
  ~csCurveTesselated ();

  ///
  int GetNumVertices () { return num_vertices; }
  ///
  int GetNumTriangles () { return num_triangles; }
  ///
  csCurveVertex& GetVertex (int i) { return vertices[i]; }
  ///
  csCurveTriangle& GetTriangle (int i) { return triangles[i]; }
};

class csPolygonSet;
class csThingTemplate;
class csCurveTemplate;

/**
 * This is an abstract class for all curves in Crystal Space.
 */
class csCurve : public csObject
{
private:
  csTextureHandle* cstxt;
  // Pointer to the parent template.
  csCurveTemplate* parent_template;

public:
  /// The polygon set parent.
  csPolygonSet* parent;

  /// This is the lightmap to be placed on the curve.
  csLightMap* lightmap;
  bool lightmap_up_to_date;

public:
  ///
  csCurve (csCurveTemplate* parent_tmpl) : csObject (), cstxt (NULL),
  	parent_template (parent_tmpl),
  	parent (NULL), lightmap (NULL), lightmap_up_to_date (false) {} 
  ///
  virtual ~csCurve ();

  ///
  void SetParent (csPolygonSet* p) { parent = p; }

  /// Get the parent template used for this curve.
  csCurveTemplate* GetParentTemplate () { return parent_template; }

  /**
   * Tesselate this curve with the given resolution.
   * This function will allocated and return a csCurveTesselated object.
   * the curve is responsible for clamping res to allowed values itself.
  */
  virtual csCurveTesselated* Tesselate (int res) = 0;

  /**
   * set control index for a control point (referring to the controls
   * in the parent csPolygonSet)
   */
  virtual void SetControlPoint (int index, int control_id) = 0;

  ///
  iTextureHandle* GetTextureHandle () { return cstxt ? cstxt->GetTextureHandle () : (iTextureHandle*)NULL; }
  ///
  void SetTextureHandle (csTextureHandle* h) { cstxt = h; }

  /**
   * Lighting support. If IsLightable returns true, PosInSpace and Normal should
   * calculate accurate positions and normals for the beziers, regardless of the
   * approximating tesselation method. These are used for lighting.
   * Default behaviour is to allow for unlighted curves. Derive these if you do
   * want them lighted.
   */
  virtual bool IsLightable ();
  ///
  virtual void PosInSpace (csVector3& vec, double u, double v);
  ///
  virtual void Normal (csVector3& vec, double u, double v);
  ///
  void InitLightMaps (csPolygonSet* owner, bool do_cache, int index);
  ///
  void CalculateLighting (csLightView& lview);
  ///
  void CacheLightMaps (csPolygonSet* owner, int index);
  
  /**
   * Add polygons to csBspContainer so that the following condition
   * is true: if curve is visible then any of the added polygons should
   * be visible.
   */
  virtual void AddBoundingPolygons (csBspContainer* container) = 0;

  CSOBJTYPE;
};

/**
 * A curve template.
 */
class csCurveTemplate : public csObject
{
protected:
  csThingTemplate *parent;

  csTextureHandle* cstxt;

public:
  ///
  csCurveTemplate() : csObject() {}

  ///
  virtual void SetParent (csThingTemplate *p) { parent=p; }

  ///
  virtual csCurve* MakeCurve () = 0;

  /// Tesselate this curve.
  virtual void SetVertex (int index, int ver_ind) = 0;
  ///
  virtual int GetVertex (int index)  = 0;
  ///
  virtual int NumVertices () = 0;
  ///
  csTextureHandle* GetTextureHandle () { return cstxt; }
  ///
  void SetTextureHandle (csTextureHandle* h) { cstxt = h; }

  CSOBJTYPE;
};

/**
 * A specific curve implementation for Bezier curve template.
 */
class csBezierTemplate : public csCurveTemplate
{
private:
  int ver_id[9];

public:
  csBezierTemplate();

  virtual csCurve* MakeCurve();

  /// Tesselate this curve.
  virtual void SetVertex (int index, int ver_ind);
  ///
  virtual int GetVertex (int index);
  virtual int NumVertices ();

  CSOBJTYPE;
};

/**
 * A specific curve implementation for Bezier curves.
 */
class csBezier : public csCurve
{
private:
  /// The control points of this curve.
  csVector3 points[3][3];
  /// The texture coordinates of every control point.
  csVector2 texture_coords[3][3];

  TDtDouble cpt[9][5];

  csCurveTesselated* previous_tesselation;
  int previous_resolution;

public:
  ///
  csBezier (csBezierTemplate* parent_tmpl);
  ///
  ~csBezier ();

  /// Tesselate this curve.
  virtual csCurveTesselated* Tesselate (int res);

  /// Load a curve from disk.
  void Load (char* buf);

  virtual void SetControlPoint (int index, int control_id);

  /// Get a curve point.
  inline csVector3& GetControlPoint (int i) { return points[i/3][i-(i/3)*3]; }

  /// Get the texture coordinate of a curve point.
  inline csVector2& GetTextureCoord (int i) { return texture_coords[i/3][i-(i/3)*3]; }

  virtual bool IsLightable ();
  virtual void PosInSpace (csVector3& vec, double u, double v);
  virtual void Normal (csVector3& vec, double u, double v);

  virtual void AddBoundingPolygons (csBspContainer* container);

  CSOBJTYPE;
};

#endif /*CURVE_H*/
