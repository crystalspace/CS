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

#include "csutil/scf.h"
#include "csutil/hashmap.h"
#include "csgeom/obb.h"

struct iObjectModel;
class csObjectModel;
class csObjectModelManager;
struct csPolygonMeshEdge;

/**
 * Outline information.
 */
class csOutlineInfo
{
  friend class csObjectModelManager;
  friend class csObjectModel;

private:
  csOutlineInfo ()
  {
    outline_edges = NULL;
    outline_verts = NULL;
  }
  ~csOutlineInfo ()
  {
    delete[] outline_edges;
    delete[] outline_verts;
  }

  void Clear ()
  {
    delete[] outline_edges; outline_edges = NULL;
    delete[] outline_verts; outline_verts = NULL;
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
class csObjectModel
{
  friend class csObjectModelManager;

private:
  iObjectModel* imodel;
  long shape_number;	// Last used shape_number from model.
  int ref_cnt;		// Number of objects in vis system using this model.
  int num_planes;
  csPlane3* planes;	// Planes for this model.

  bool dirty_obb;	// If true obb is dirty and requires calculation.
  csOBB obb;		// OBB for this model.

  csPolygonMeshEdge* edges;
  int num_edges;

  // Outline information (may in future move to a seperate class).
  csOutlineInfo outline_info;

  csObjectModel ();
  ~csObjectModel ();

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

  /// Get the OBB for this model.
  const csOBB& GetOBB ();
};

/**
 * An object model manager. This class will manage instances of
 * csObjectModel for every iObjectModel that is received by the visibility
 * system.
 */
class csObjectModelManager
{
private:
  csHashMap models;

public:
  csObjectModelManager ();
  ~csObjectModelManager ();

  /**
   * Find the model for this iObjectModel. If it already exists, increase
   * the ref count and return it. Otherwise create a new one.
   */
  csObjectModel* CreateObjectModel (iObjectModel* model);

  /**
   * Release the model for this iObjectModel. This will possibly delete
   * the given model if this was the last reference.
   */
  void ReleaseObjectModel (csObjectModel* model);

  /**
   * Check if the object model needs recalculation (if the shape
   * of the object it represents has changed). Returns true if the
   * object has been changed.
   */
  bool CheckObjectModel (csObjectModel* model);
};

#endif // __CS_DMODEL_H__

