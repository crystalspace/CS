/*
    Crystal Space 3D engine
    Copyright (C) 2003 by Jorrit Tyberghein

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

#ifndef __CS_CSGEOM_POLYMESH_H__
#define __CS_CSGEOM_POLYMESH_H__

#include "igeom/polymesh.h"
#include "csgeom/vector3.h"

/**
 * \addtogroup geom_utils
 * @{ */
 
/**
 * A conveniance polygon mesh implementation that you can feed
 * with vertices and polygons from another source.
 * This mesh optionally supports deformable.
 */
class csPolygonMesh : public iPolygonMesh
{
private:
  bool deformable;
  uint32 change_nr;

  int vt_count;
  csVector3* vt;
  bool delete_vt;	// If true this class is responsible for cleanup.

  int po_count;
  csMeshedPolygon* po;
  bool delete_po;	// If true this class is responsible for cleanup.

public:
  /**
   * Construct a polygon mesh.
   */
  csPolygonMesh ()
  {
    SCF_CONSTRUCT_IBASE (NULL);
    deformable = false;
    change_nr = 0;
    vt = NULL;
    vt_count = 0;
    delete_vt = false;
    po = NULL;
    po_count = 0;
    delete_po = false;
  }

  virtual ~csPolygonMesh ()
  {
    if (delete_vt) delete[] vt;
    if (delete_po) delete[] po;
  }

  /**
   * Set the vertices to use for this polygon mesh.
   * If 'delete_vt' is true then this class will do the cleanup itself
   * at destruction. Otherwise you have to make sure that the pointer
   * to the vertices remains valid until this object is deleted.
   */
  void SetVertices (csVector3* vt, int vt_count, bool delete_vt)
  {
    csPolygonMesh::vt = vt;
    csPolygonMesh::vt_count = vt_count;
    csPolygonMesh::delete_vt = delete_vt;
    ShapeChanged ();
  }

  /**
   * Set the polygons to use for this polygon mesh.
   * If 'delete_po' is true then this class will do the cleanup itself
   * at destruction. Otherwise you have to make sure that the pointer
   * to the polygons remains valid until this object is deleted.
   */
  void SetPolygons (csMeshedPolygon* po, int po_count, bool delete_po)
  {
    csPolygonMesh::po = po;
    csPolygonMesh::po_count = po_count;
    csPolygonMesh::delete_po = delete_po;
    ShapeChanged ();
  }

  /**
   * Set vertex count. This will make room for the specified number
   * of vertices so that the user can update them. This class will delete
   * the vertices itself later.
   */
  void SetVertexCount (int vt_count)
  {
    vt = new csVector3[vt_count];
    csPolygonMesh::vt_count = vt_count;
    delete_vt = true;
    ShapeChanged ();
  }

  /**
   * Set polygon count. This will make room for the specified number
   * of polygons so that the user can update them. This class will delete
   * the polygons itself later.
   */
  void SetPolygonCount (int po_count)
  {
    po = new csMeshedPolygon[po_count];
    csPolygonMesh::po_count = po_count;
    delete_po = true;
    ShapeChanged ();
  }

  /**
   * Set deformable on or off.
   */
  void SetDeformable (bool deformable)
  {
    csPolygonMesh::deformable = deformable;
  }

  /**
   * Indicate that the shape has changed.
   */
  void ShapeChanged ()
  {
    change_nr++;
  }

  SCF_DECLARE_IBASE;

  virtual int GetVertexCount () { return vt_count; }
  virtual csVector3* GetVertices () { return vt; }
  virtual int GetPolygonCount () { return po_count; }
  virtual csMeshedPolygon* GetPolygons () { return po; }
  virtual void Cleanup () { }
  virtual bool IsDeformable () const { return deformable; }
  virtual uint32 GetChangeNumber () const { return change_nr; }
};

/** @} */

#endif // __CS_CSGEOM_POLYMESH_H__

