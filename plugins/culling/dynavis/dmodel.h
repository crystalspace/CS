/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __CS_DMODEL_H__
#define __CS_DMODEL_H__

#include "csgeom/obb.h"
#include "csutil/hash.h"
#include "csutil/scf.h"

// @@@ Hack(s) to avoid problems with static linking
#ifdef DYNAVIS_DEBUG
#define csOutlineInfo		csOutlineInfo_DEBUG
#define csDynavisObjectModel	csDynavisObjectModel_DEBUG
#define csObjectModelManager	csObjectModelManager_DEBUG
#endif

class csDynavisObjectModel;
class csObjectModelManager;
struct csPolygonMeshEdge;
struct iMeshWrapper;
struct iObjectModel;

/**
 * Outline information.
 */
class csOutlineInfo
{
  friend class csObjectModelManager;
  friend class csDynavisObjectModel;

private:
  csOutlineInfo ()
  {
    outline_edges = 0;
    outline_verts = 0;
  }
  ~csOutlineInfo ()
  {
    delete[] outline_edges;
    delete[] outline_verts;
  }

  void Clear ()
  {
    delete[] outline_edges; outline_edges = 0;
    delete[] outline_verts; outline_verts = 0;
  }

public:
  int num_outline_edges;
  int* outline_edges;
  bool* outline_verts;
  float valid_radius;
  csVector3 outline_pos;
};

/**
 * This object represents a model in this visibility system. Several
 * objects can share the same model. Every instance of this class corresponds
 * to a different iObjectModel from the engine.
 */
class csDynavisObjectModel
{
  friend class csObjectModelManager;

private:
  iObjectModel* imodel;
  long shape_number;	// Last used shape_number from model.
  int ref_cnt;		// Number of objects in vis system using this model.
  int num_planes;
  csPlane3* planes;	// Planes for this model.

  bool dirty_obb;	// If true obb is dirty and requires calculation.
  bool has_obb;		// If false then this model doesn't have an obb.
  csOBB obb;		// OBB for this model.

  // If true then we can use the outline filler for this model.
  // Otherwise this object has bad edges (edges with only one polygon
  // attached) in which case we use the polygon based culler.
  bool use_outline_filler;

  // If true then object is empty and we can't do coverage culling.
  bool empty_object;

  // If true then object is single polygon.
  bool single_polygon;

  csPolygonMeshEdge* edges;
  int num_edges;

  // Outline information (may in future move to a separate class).
  csOutlineInfo outline_info;

  csDynavisObjectModel ();
  ~csDynavisObjectModel ();

public:
  /// Get the iObjectModel for this model.
  iObjectModel* GetModel () const { return imodel; }
  /// Get the shape number.
  long GetShapeNumber () const { return shape_number; }
  /// Get the planes.
  const csPlane3* GetPlanes () const { return planes; }

  /// Update outline from the given position. Possibly reuse old outline.
  void UpdateOutline (const csVector3& pos);

  /// Get outline information.
  const csOutlineInfo& GetOutlineInfo () const { return outline_info; }

  /// Return true if this model has an OBB.
  bool HasOBB ();

  /// Return true if this model can use outline filler.
  bool CanUseOutlineFiller () const { return use_outline_filler; }

  /// Return true if model is empty.
  bool IsEmptyObject () const { return empty_object; }

  /// Return true if model is single polygon.
  bool IsSinglePolygon () const { return single_polygon; }
  
  /// Get the OBB for this model.
  const csOBB& GetOBB ();
};

/**
 * An object model manager. This class will manage instances of
 * csDynavisObjectModel for every iObjectModel that is received by the
 * visibility system.
 */
class csObjectModelManager
{
private:
  typedef csHash<csDynavisObjectModel*, csPtrKey<iObjectModel> > ModelHash;
  ModelHash models;

public:
  csObjectModelManager ();
  ~csObjectModelManager ();

  /**
   * Find the model for this iObjectModel. If it already exists, increase
   * the ref count and return it. Otherwise create a new one.
   */
  csDynavisObjectModel* CreateObjectModel (iObjectModel* model);

  /**
   * Release the model for this iObjectModel. This will possibly delete
   * the given model if this was the last reference.
   */
  void ReleaseObjectModel (csDynavisObjectModel* model);

  /**
   * Check if the object model needs recalculation (if the shape
   * of the object it represents has changed). Returns true if the
   * object has been changed.
   */
  bool CheckObjectModel (csDynavisObjectModel* model, iMeshWrapper* mesh);
};

#endif // __CS_DMODEL_H__
