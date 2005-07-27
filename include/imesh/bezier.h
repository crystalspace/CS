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

#ifndef __CS_IMESH_BEZIER_H__
#define __CS_IMESH_BEZIER_H__

#include "csutil/scf.h"

class csCurve;
class csFlags;
class csMatrix3;
class csVector2;
class csVector3;

struct iMaterialWrapper;
struct iObject;

/**
 * This is the interface for a curve.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iBezierFactoryState::CreateCurve()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>iBezierFactoryState::GetCurve()
 *   </ul>
 */
struct iCurve : public iBase
{
  /// Get the original curve (@@@ UGLY).
  virtual csCurve* GetOriginalObject () = 0;
  /// Get the iObject for this curve.
  virtual iObject *QueryObject() = 0;
  /// Set the material wrapper.
  virtual void SetMaterial (iMaterialWrapper* mat) = 0;
  /// Get the material wrapper.
  virtual iMaterialWrapper* GetMaterial () = 0;
  /// Set a control point.
  virtual void SetControlPoint (int idx, int control_id) = 0;

  /// Get the number of vertices.
  virtual int GetVertexCount () const = 0;
  /// Get a vertex.
  virtual int GetVertex (int idx) const = 0;
  /// Set a vertex.
  virtual void SetVertex (int idx, int vt) = 0;
};

SCF_VERSION (iBezierFactoryState, 0, 0, 1);

/**
 * This is the state interface to access the internals of a bezier
 * mesh factory.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>Bezier mesh object plugin (crystalspace.mesh.object.bezier)
 *   <li>iMeshObjectType::NewFactory()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>SCF_QUERY_INTERFACE() on iMeshFactoryWrapper::GetMeshObjectFactory()
 *   <li>iBezierState::GetFactory()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>Bezier Factory Loader plugin (crystalspace.mesh.loader.factory.bezier)
 *   </ul>
 */
struct iBezierFactoryState : public iBase
{
  /**
   * Get the center of the curves.
   */
  virtual const csVector3& GetCurvesCenter () const = 0;
  /**
   * Set the center of the curves.
   */
  virtual void SetCurvesCenter (const csVector3& cen) = 0;

  /**
   * Get the scale of the curves.
   */
  virtual float GetCurvesScale () const = 0;
  /**
   * Set the scale of the curves.
   */
  virtual void SetCurvesScale (float scale) = 0;

  /// Add a curve vertex.
  virtual void AddCurveVertex (const csVector3& v, const csVector2& uv) = 0;
  /// Get the number of curves.
  virtual int GetCurveCount () const = 0;
  /// Get the curve.
  virtual iCurve* GetCurve (int idx) const = 0;
  /// Get the number of curve vertices.
  virtual int GetCurveVertexCount () const = 0;
  /// Get the specified curve vertex.
  virtual csVector3& GetCurveVertex (int i) const = 0;
  /// Get the curve vertices.
  virtual csVector3* GetCurveVertices () const = 0;
  /// Get the specified curve texture coordinate (texel).
  virtual csVector2& GetCurveTexel (int i) const = 0;
  /// Set a curve vertex.
  virtual void SetCurveVertex (int idx, const csVector3& vt) = 0;
  /// Set a curve texel.
  virtual void SetCurveTexel (int idx, const csVector2& vt) = 0;
  /// Clear all curve vertices (and texels too).
  virtual void ClearCurveVertices () = 0;

  /// Create a new curve for this thing.
  virtual iCurve* CreateCurve () = 0;
  /// Find the index for a curve. Returns -1 if curve cannot be found.
  virtual int FindCurveIndex (iCurve* curve) const = 0;
  /// Delete a curve given an index.
  virtual void RemoveCurve (int idx) = 0;
  /// Delete all curves.
  virtual void RemoveCurves () = 0;

  /**
   * Get cosinus factor.
   */
  virtual float GetCosinusFactor () const = 0;
  /**
   * Set cosinus factor. This cosinus factor controls how lighting affects
   * the polygons relative to the angle. If no value is set here then the
   * default is used.
   */
  virtual void SetCosinusFactor (float cosfact) = 0;

  /**
   * Add polygons and vertices from the specified thing (seen as template).
   */
  virtual void MergeTemplate (iBezierFactoryState* tpl,
  	iMaterialWrapper* default_material = 0,
	csVector3* shift = 0, csMatrix3* transform = 0) = 0;
};

SCF_VERSION (iBezierState, 0, 0, 1);

/**
 * This is the state interface to access the internals of a thing
 * mesh object.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>Bezier mesh object plugin (crystalspace.mesh.object.bezier)
 *   <li>iMeshObjectFactory::NewInstance()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>SCF_QUERY_INTERFACE() on iMeshWrapper::GetMeshObject()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>Bezier Loader plugin (crystalspace.mesh.loader.bezier)
 *   </ul>
 */
struct iBezierState : public iBase
{
  /// Get the factory.
  virtual iBezierFactoryState* GetFactory () = 0;
};

SCF_VERSION (iCurve, 0, 1, 0);

#endif // __CS_IMESH_BEZIER_H__

