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

#include "csgeom/box.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/tri.h"
#include "csutil/csobject.h"
#include "csutil/scf.h"
#include "iengine/material.h"
#include "imesh/bezier.h"
#include "ivideo/graph3d.h"

#include "bezier2.h"
#include "clightmap.h"


class csBezierMesh;
class csBezierMeshObjectType;
class csCurveTemplate;
class csBezierLightPatch;

struct csCoverageMatrix;
struct csTriangle;

struct iCacheManager;
struct iCamera;
struct iFrustumView;
struct iMaterialHandle;
struct iMaterialWrapper;

/**
 * Tesselated curve. This is basically a list of triangles.
 */
class csCurveTesselated
{
private:
  // Object space coordinates.
  csVector3* ObjectCoords;
  // Texture coordinates.
  csVector2* TextureCoords;
  // Original control points.
  csVector2* ControlPoints;
  // Colors for the vertices.
  csColor* Colors;
  // Triangles.
  csTriangle* Triangles;

  // Number of vertices
  size_t NumVertices;
  // Number of triangles
  size_t NumTriangles;
  // A flag which indicates if the color table is filled in.
  bool ColorsValid;

public:
  /**
   * Allocate a new tesselated curve with the given
   * number of vertices and triangles.
   */
  csCurveTesselated (size_t NumVertices, size_t NumTriangles);
  /// destructor
  ~csCurveTesselated ();

  /// Return the number of vertices
  inline size_t GetVertexCount () const;
  /// Return the number of triangles
  inline size_t GetTriangleCount () const;
  /// Return the array of vertices
  inline csVector3* GetVertices ();
  /// Return the array of texture coordinates
  inline csVector2* GetTxtCoords ();
  /// Return the array of control points
  inline csVector2* GetControlPoints ();
  /// Return the array of vertex colors
  inline csColor* GetColors ();
  /// Return the array of triangles
  inline csTriangle* GetTriangles ();
  /// Return a single triangle @@@ why?
  inline csTriangle& GetTriangle (size_t i);
  /// Return true if the colors table is valid.
  inline bool AreColorsValid () const;

  /**
   * Update the 'colors' array in this tesselation given
   * a lightmap. This should be called whenever the lightmap
   * changes and the curve needs to be rendered.
   */
  void UpdateColors (csCurveLightMap* lightmap, const csColor& dynambient);
};

/**
 * This is an abstract class for all curves in Crystal Space.
 */
class csCurve : public csObject
{
private:
  csBezierMeshObjectType* thing_type;

  /// Material for this curve
  csRef<iMaterialWrapper> Material;

  /// list of light patches
  csBezierLightPatch* LightPatches;

  /**
   * Object to world transformation (Needed by CalculateLighting &
   * ShineDynLight).
   */
  csReversibleTransform* O2W;

  /*
   * Position Buffer: this is an array which coordinates u,v lightmap
   * pixel coordinates to the position on the curve in world space
   * coordinates.
   * i.e. in a 10x10 lightmap uv2World[5][5] is the world space coordinate
   *   of the lightmap texel 5,5 on the lightmap
   */
  csVector3* uv2World;

  /*
   * Normal Buffer: this is an array which coordinates u,v lightmap
   * pixel coordinates to the normal of the curve
   * i.e. in a 10x10 lightmap uv2Normal[5][5] is the normal vector which
   * corresponds to the lightmap texel 5,5 on the lightmap
   */
  csVector3* uv2Normal;


public:
  /// The polygon set parent.
  csBezierMesh* ParentThing;

  /// This is the lightmap to be placed on the curve.
  csCurveLightMap* LightMap;

  /// This flag indicates whether the lightmap is up-to-date
  bool LightmapUpToDate;

  /// Lighting version.
  uint32 light_version;

public:

  /// Constructor
  csCurve (csBezierMeshObjectType* thing_type);
  /// Destructor
  virtual ~csCurve ();

  /// Return the material handle for this curve
  inline iMaterialHandle* GetMaterialHandle () const;
  /// Return the material wrapper for this curve
  inline iMaterialWrapper* GetMaterial () const;
  /// Set the material wrapper for this curve
  void SetMaterial (iMaterialWrapper* h);

  /// @@@
  void DynamicLightDisconnect (iLight* dynlight);
  /// @@@
  void StaticLightDisconnect (iLight* statlight);
  /// Add a lightpatch to this curves list of light patches
  void AddLightPatch (csBezierLightPatch* lp);
  /// Remove a lightpatch from this curves list
  void UnlinkLightPatch (csBezierLightPatch* lp);
  /// update the real lightmap with all light info
  bool RecalculateDynamicLights ();
  /// update the real lightmap with info from the lightpatch
  void ShineDynLight (csBezierLightPatch* lp);

  /// Set the current object to world space transformation.
  void SetObject2World (const csReversibleTransform *o2w);
  /// Return the current object to world space transformation
  inline const csReversibleTransform *GetObject2World () const;

  /// Set the parent thing for this curve
  inline void SetParentThing (csBezierMesh* p);
  /// Return the parent thing for this curve
  inline csBezierMesh* GetParentThing () const;

  /// Get the lightmap.
  inline csCurveLightMap* GetLightMap () const;
  /// Calculate the lighting for this curve (static).
  void CalculateLightingStatic (iFrustumView* lview, bool vis);
  /// Calculate the lighting for this curve (dynamic).
  void CalculateLightingDynamic (iFrustumView* lview);
  /// Initialize default lighting.
  void InitializeDefaultLighting (bool clear);
  /// Read lighting from file.
  const char* ReadFromCache (iFile* file);
  /// Cache the curve lightmaps to file.
  bool WriteToCache (iFile* file);
  /// Prepare lighting.
  void PrepareLighting ();

  /**
   * Populate a coverage matrix which relates shadow information for this
   * curve's lightmap
   */
  void GetCoverageMatrix (iFrustumView* lview, csCoverageMatrix &cm) const;

  /// return an approximation of the area of this curve
  float GetArea();

  /// @@@
  void CalcUVBuffers();

  /**
   * Tesselate this curve with the given resolution.
   * This function will allocated and return a csCurveTesselated object.
   * the curve is responsible for clamping res to allowed values itself.
  */
  virtual csCurveTesselated* Tesselate (int res) = 0;

  /**
   * set control index for a control point (referring to the controls
   * in the parent csBezierMesh)
   */
  virtual void SetControlPoint (int index, int control_id) = 0;

  /// Return a bounding box in object space for this curve.
  virtual void GetObjectBoundingBox (csBox3& bbox) = 0;

  /**
   * Get a bounding box in camera space. This function
   * uses the object bounding box so it will exagerate the real
   * bounding box a little.
   */
  void GetCameraBoundingBox (const csTransform& obj2cam, csBox3& cbox);

  /**
   * Get a bounding box in screen space and camera space.
   * This function will use GetCameraBoundingBox().
   * It will fill in the boundingBox with the X and Y locations
   * of the curve.  Returns the max Z location of the curve,
   * or -1 if the curve is not on-screen.
   * If the curve is not on-screen, the X and Y values are
   * not valid.
   */
  float GetScreenBoundingBox (const csTransform& obj2cam,
  	iCamera* camera, csBox3& cameraBox, csBox2& boundingBox);

  /**
   * Lighting support. If IsLightable returns true, PosInSpace and Normal should
   * calculate accurate positions and normals for the beziers, regardless of the
   * approximating tesselation method. These are used for lighting.
   * Default behaviour is to allow for unlighted curves. Derive these if you do
   * want them lighted.
   */
  virtual bool IsLightable ();
  /// Helper function for lighting. Override for different types of curves.
  virtual void PosInSpace (csVector3& vec, double u, double v);
  /// Helper function for lighting. Override for different types of curves.
  virtual void Normal (csVector3& vec, double u, double v);

  /// Do a hard transform on this curve.
  virtual void HardTransform (const csReversibleTransform& trans);

  /// Set a vertex of the template.
  virtual void SetVertex (int index, int ver_ind) = 0;
  /// Return a vertex of the template.
  virtual int GetVertex (int index)  = 0;
  /// Return the number of vertices in the template.
  virtual int GetVertexCount () = 0;

  SCF_DECLARE_IBASE_EXT (csObject);

  //----------------------- iCurve interface implementation -----------------
  /// iCurve implementation.
  struct Curve : public iCurve
  {
    SCF_DECLARE_EMBEDDED_IBASE (csCurve);

    virtual csCurve* GetOriginalObject ()
    { return scfParent; }
    virtual iObject *QueryObject()
    { return scfParent; }
    virtual void SetMaterial (iMaterialWrapper* mat)
    { scfParent->SetMaterial (mat); }
    virtual iMaterialWrapper* GetMaterial ()
    { return scfParent->GetMaterial (); }
    virtual void SetControlPoint (int idx, int control_id)
    { scfParent->SetControlPoint (idx, control_id); }

    virtual int GetVertexCount () const
    { return scfParent->GetVertexCount (); }
    virtual int GetVertex (int idx) const
    { return scfParent->GetVertex (idx); }
    virtual void SetVertex (int idx, int vt)
    { scfParent->SetVertex (idx, vt); }
  } scfiCurve;
  friend struct Curve;
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

  int ver_id[9];

public:
  ///
  csBezierCurve (csBezierMeshObjectType* thing_type);
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

  /// Tesselate this curve.
  virtual void SetVertex (int index, int ver_ind);
  ///
  virtual int GetVertex (int index);
  virtual int GetVertexCount ();
};

/*
 * Implementation of inline functions
 */

inline size_t csCurveTesselated::GetVertexCount () const
{ return NumVertices; }
inline size_t csCurveTesselated::GetTriangleCount () const
{ return NumTriangles; }
inline csVector3* csCurveTesselated::GetVertices ()
{ return ObjectCoords; }
inline csVector2* csCurveTesselated::GetTxtCoords ()
{ return TextureCoords; }
inline csVector2* csCurveTesselated::GetControlPoints ()
{ return ControlPoints; }
inline csColor* csCurveTesselated::GetColors ()
{ return Colors; }
inline csTriangle* csCurveTesselated::GetTriangles ()
{ return Triangles; }
inline csTriangle& csCurveTesselated::GetTriangle (size_t i)
{ return Triangles[i]; }
inline bool csCurveTesselated::AreColorsValid () const
{ return ColorsValid; }

inline iMaterialWrapper* csCurve::GetMaterial () const
{ return Material; }
inline csCurveLightMap* csCurve::GetLightMap () const
{ return LightMap; }
inline void csCurve::SetParentThing (csBezierMesh* p)
{ ParentThing = p; }
inline csBezierMesh* csCurve::GetParentThing () const
{ return ParentThing; }
inline const csReversibleTransform *csCurve::GetObject2World () const
{ return O2W; }

#endif // __CS_CURVE_H__

