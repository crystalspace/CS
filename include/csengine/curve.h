/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
  
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

#ifndef __CS_CURVE_H__
#define __CS_CURVE_H__

#include "csutil/scf.h"
#include "csutil/csvector.h"
#include "csgeom/math3d.h"
#include "csgeom/math2d.h"
#include "csgeom/box.h"
#include "csengine/bezier.h"
#include "csengine/texture.h"
#include "csengine/material.h"
#include "csengine/lghtmap.h"
#include "csengine/rview.h"
#include "csobject/csobject.h"
#include "ivideo/graph3d.h"
#include "iengine/curve.h"

struct iMaterialHandle;
class csBspContainer;
class csFrustumView;

/**
 * Tesselated curve. This is basicly a list of triangles.
 */
class csCurveTesselated
{
private:
  // Object space coordinates.
  csVector3* object_coords;
  // Texture coordinates.
  csVector2* txt_coords;
  // Original control points.
  csVector2* controls;
  // Colors for the vertices.
  csColor* colors;
  // Triangles.
  csTriangle* triangles;

  int num_vertices;
  int num_triangles;

  // A flag which indicates if the color table is filled in.
  bool colors_valid;

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
  csVector3* GetVertices () { return object_coords; }
  ///
  csVector2* GetTxtCoords () { return txt_coords; }
  ///
  csVector2* GetControlPoints () { return controls; }
  ///
  csColor* GetColors () { return colors; }
  ///
  csTriangle* GetTriangles () { return triangles; }
  ///
  csTriangle& GetTriangle (int i) { return triangles[i]; }

  /**
   * Update the 'colors' array in this tesselation given
   * a lightmap. This should be called whenever the lightmap
   * changes and the curve needs to be rendered.
   */
  void UpdateColors (csLightMap* lightmap);

  /// Return true if the colors table is valid.
  bool AreColorsValid () { return colors_valid; }
};

class csThing;
class csCurveTemplate;
class csLightPatch;
class csSector;
class csRadCurve;
class Dumper;
struct csCoverageMatrix;

/**
 * This is an abstract class for all curves in Crystal Space.
 */
class csCurve : public csObject
{
  // allow csRadCurve to use our UV Buffers
  friend class csRadCurve;
  friend class Dumper;

private:
  /// ID for this curve.
  unsigned long curve_id;
  /// Last used ID.
  static unsigned long last_curve_id;

  csMaterialWrapper* cstxt;
  // Pointer to the parent template.
  csCurveTemplate* parent_template;
  
  // list of light patches
  csLightPatch* lightpatches;

  // Object to world transformation (Needed by CalculateLighting & 
  // ShineDynLight)
  csReversibleTransform* _o2w;

  /*
   * Position Buffer: this is an array which coordinates u,v lightmap
   * pixel coordinates to the position on the curve in world space
   * coordinates.  
   * i.e. in a 10x10 lightmap uv2World[5][5] is the world space coordinate
   *   of the lightmap texel 5,5 on the lightmap
   */
  csVector3* _uv2World;

  /*
   * Normal Buffer: this is an array which coordinates u,v lightmap
   * pixel coordinates to the normal of the curve  
   * i.e. in a 10x10 lightmap uv2Normal[5][5] is the normal vector which
   * corresponds to the lightmap texel 5,5 on the lightmap
   */
  csVector3* _uv2Normal;

public:
  /// The polygon set parent.
  csThing* parent;

  /// This is the lightmap to be placed on the curve.
  csLightMap* lightmap;
  bool lightmap_up_to_date;

public:
  ///
  csCurve (csCurveTemplate* parent_tmpl);

  /// Destructor
  virtual ~csCurve ();

  /// Get the ID of this curve.
  unsigned long GetCurveID () { return curve_id; }

  /**
   * Populate a coverage matrix which relates shadow information for this 
   * curve's lightmap
   */
  void GetCoverageMatrix (csFrustumView& lview, csCoverageMatrix &cm) const;

  /// return an approximation of the area of this curve
  float GetArea();
  
  /// Set the current object to world space transformation
  void SetObject2World (csReversibleTransform* o2w);
  
  /// Sets the parent thing for this Curve
  void SetParent (csThing* p) { parent = p; }

  void MakeDirtyDynamicLights ();

  /// Get the parent template used for this curve.
  csCurveTemplate* GetParentTemplate () { return parent_template; }

  /// Get the lightmap.
  csLightMap* GetLightMap () { return lightmap; }

  void CalcUVBuffers();

  /**
   * Tesselate this curve with the given resolution.
   * This function will allocated and return a csCurveTesselated object.
   * the curve is responsible for clamping res to allowed values itself.
  */
  virtual csCurveTesselated* Tesselate (int res) = 0;

  /**
   * set control index for a control point (referring to the controls
   * in the parent csThing)
   */
  virtual void SetControlPoint (int index, int control_id) = 0;

  ///
  iMaterialHandle* GetMaterialHandle () { return cstxt ? cstxt->GetMaterialHandle () : (iMaterialHandle*)NULL; }
  ///
  csMaterialWrapper* GetMaterialWrapper () { return cstxt; }
  ///
  void SetMaterialWrapper (csMaterialWrapper* h) { cstxt = h; }

  /// Return a bounding box in object space for this curve.
  virtual void GetObjectBoundingBox (csBox3& bbox) = 0;

  /**
   * Get a bounding box in camera space. This function
   * uses the object bounding box so it will exagerate the real
   * bounding box a little.
   */
  void GetCameraBoundingBox (const csTransform& obj2cam, csBox3& cbox);

  /**
   * Get a bounding box in screen space.
   * This function will use GetCameraBoundingBox().
   * It will fill in the boundingBox with the X and Y locations
   * of the curve.  Returns the max Z location of the curve,
   * or -1 if the curve is not on-screen.
   * If the curve is not on-screen, the X and Y values are
   * not valid.
   */
  float GetScreenBoundingBox (const csTransform& obj2cam,
  	iCamera* camera, csBox2& boundingBox);

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
  void InitLightMaps (bool do_cache);

  /// Add a lightpatch to this curves list of light patches
  void AddLightPatch (csLightPatch* lp);

  /// Remove a lightpatch from this curves list
  void UnlinkLightPatch (csLightPatch* lp);

  /// update the real lightmap with all light info
  bool RecalculateDynamicLights ();

  /// update the real lightmap with info from the lightpatch
  void ShineDynLight (csLightPatch* lp);

  /// calculate the lighting for this curve
  void CalculateLighting (csFrustumView& lview);

  ///
  void CacheLightMaps ();

  /// Do a hard transform on this curve.
  virtual void HardTransform (const csReversibleTransform& trans);
  
  CSOBJTYPE;
  DECLARE_IBASE_EXT (csObject);

  //----------------------- iCurve interface implementation -----------------
  struct Curve : public iCurve
  {
    DECLARE_EMBEDDED_IBASE (csCurve);
    virtual csCurve* GetOriginalObject () { return (csCurve*)scfParent; }
    virtual iObject *QueryObject() {return scfParent;}
    virtual iCurveTemplate* GetParentTemplate ();
    virtual void SetMaterial (iMaterialWrapper* mat);
    virtual iMaterialWrapper* GetMaterial ();
    virtual void SetName (const char* name) { scfParent->SetName (name); }
    virtual const char* GetName () const { return scfParent->GetName (); }
    virtual void SetControlPoint (int idx, int control_id)
    {
      scfParent->SetControlPoint (idx, control_id);
    }
  } scfiCurve;
  friend struct Curve;
};

/**
 * A curve template.
 */
class csCurveTemplate : public csPObject
{
protected:
  csMaterialWrapper* cstxt;

public:
  ///
  csCurveTemplate();
  ///
  virtual ~csCurveTemplate () { }

  ///
  virtual csCurve* MakeCurve () = 0;

  /// Tesselate this curve.
  virtual void SetVertex (int index, int ver_ind) = 0;
  ///
  virtual int GetVertex (int index)  = 0;
  ///
  virtual int NumVertices () = 0;
  ///
  csMaterialWrapper* GetMaterialWrapper () { return cstxt; }
  ///
  void SetMaterialWrapper (csMaterialWrapper* h) { cstxt = h; }

  CSOBJTYPE;
  DECLARE_IBASE_EXT (csObject);

  //------------------ iCurveTemplate interface implementation --------------
  struct CurveTemplate : public iCurveTemplate
  {
    DECLARE_EMBEDDED_IBASE (csCurveTemplate);
    virtual iObject *QueryObject()
    {
      return scfParent;
    }
    virtual void SetMaterial (iMaterialWrapper* mat);
    virtual iMaterialWrapper* GetMaterial ();
    virtual iCurve* MakeCurve ();
    virtual int GetNumVertices () const { return scfParent->NumVertices (); }
    virtual int GetVertex (int idx) const
    {
      return scfParent->GetVertex (idx);
    }
    virtual void SetVertex (int idx, int vt)
    {
      scfParent->SetVertex (idx, vt);
    }
  } scfiCurveTemplate;
  friend struct CurveTemplate;
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
class csBezierCurve : public csCurve
{
private:
  /// The control points of this curve.
  csVector3 points[3][3];
  /// The texture coordinates of every control point.
  csVector2 texture_coords[3][3];

  double cpt[9][5];

  csCurveTesselated* previous_tesselation;
  int previous_resolution;

  /// Object space bounding box.
  csBox3 object_bbox;
  /// If true then the object bbox is valid.
  bool valid_bbox;

public:
  ///
  csBezierCurve (csBezierTemplate* parent_tmpl);
  ///
  ~csBezierCurve ();

  /// Tesselate this curve.
  virtual csCurveTesselated* Tesselate (int res);

  /// Return a bounding box in object space for this curve.
  virtual void GetObjectBoundingBox (csBox3& bbox);

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

  /// Do a hard transform on this curve.
  virtual void HardTransform (const csReversibleTransform& trans);

  CSOBJTYPE;
};

#endif // __CS_CURVE_H__
