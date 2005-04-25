/*
    Copyright (C) 2003 by Jorrit Tyberghein
              (C) 2004 by Marten Svanfeldt

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

#ifndef __CS_PORTALCONTAINER_H__
#define __CS_PORTALCONTAINER_H__

#include "iengine/portalcontainer.h"
#include "csutil/refarr.h"
#include "csutil/dirtyaccessarray.h"
#include "csgeom/vector3.h"
#include "csgeom/pmtools.h"
#include "plugins/engine/3d/portal.h"
#include "cstool/meshobjtmpl.h"
#include "cstool/rendermeshholder.h"
#include "iengine/shadcast.h"
#include "ivideo/rendermesh.h"

class csMeshWrapper;
class csMovable;

/**
 * A helper class for iPolygonMesh implementations used by csPortalContainer.
 */
class csPortalContainerPolyMeshHelper : public iPolygonMesh
{
public:
  SCF_DECLARE_IBASE;

  /**
   * Make a polygon mesh helper which will accept polygons which match
   * with the given flag (one of CS_POLY_COLLDET or CS_POLY_VISCULL).
   */
  csPortalContainerPolyMeshHelper (uint32 flag) :
  	polygons (0), vertices (0), poly_flag (flag), triangles (0)
  {
    SCF_CONSTRUCT_IBASE (0);
  }
  virtual ~csPortalContainerPolyMeshHelper ()
  {
    Cleanup ();
    SCF_DESTRUCT_IBASE ();
  }

  void Setup ();
  void SetPortalContainer (csPortalContainer* pc);

  virtual int GetVertexCount ()
  {
    Setup ();
    return (int)vertices->Length ();
  }
  virtual csVector3* GetVertices ()
  {
    Setup ();
    return vertices->GetArray ();
  }
  virtual int GetPolygonCount ()
  {
    Setup ();
    return num_poly;
  }
  virtual csMeshedPolygon* GetPolygons ()
  {
    Setup ();
    return polygons;
  }
  virtual int GetTriangleCount ()
  {
    Triangulate ();
    return tri_count;
  }
  virtual csTriangle* GetTriangles ()
  {
    Triangulate ();
    return triangles;
  }
  virtual void Lock () { }
  virtual void Unlock () { }
 
  virtual csFlags& GetFlags () { return flags;  }
  virtual uint32 GetChangeNumber() const { return 0; }

  void Cleanup ();

private:
  csPortalContainer* parent;
  uint32 data_nr;		// To see if the portal container has changed.
  csMeshedPolygon* polygons;	// Array of polygons.
  // Array of vertices from portal container.
  csDirtyAccessArray<csVector3>* vertices;
  int num_poly;			// Total number of polygons.
  uint32 poly_flag;		// Polygons must match with this flag.
  csFlags flags;
  csTriangle* triangles;
  int tri_count;

  void Triangulate ()
  {
    if (triangles) return;
    csPolygonMeshTools::Triangulate (this, triangles, tri_count);
  }
};

/**
 * This is a container class for portals.
 */
class csPortalContainer : public csMeshObject, public iPortalContainer,
	public iShadowReceiver
{
private:
  csRefArray<csPortal> portals;
  bool prepared;
  // Number that is increased with every significant change.
  uint32 data_nr;

  // Object space data.
  csDirtyAccessArray<csVector3> vertices;
  csBox3 object_bbox;
  csVector3 object_radius;
  float max_object_radius;

  csRef<iShaderManager> shader_man;
  csRef<iShader> fog_shader;

  // World space data. movable_nr is used to detect if it needs to be
  // recalculated.
  long movable_nr;
  bool movable_identity;
  csDirtyAccessArray<csVector3> world_vertices;

  // Camera space data.
  csDirtyAccessArray<csVector3> camera_vertices;
  csArray<csPlane3> camera_planes;

  int clip_portal, clip_plane, clip_z_plane;

  // Drawing stuff...
  bool csPortalContainer::ClipToPlane (int portal_idx, csPlane3 *portal_plane,
	const csVector3 &v_w2c, csVector3 * &pverts, int &num_verts);
  bool DoPerspective (csVector3 *source, int num_verts,
	csPoly2D *dest, bool mirror, int fov,
	float shift_x, float shift_y, const csPlane3& plane_cam);
  void DrawOnePortal (csPortal* po, const csPoly2D& poly,
	const csReversibleTransform& movtrans, iRenderView *rview,
	const csPlane3& camera_plane);

  csRenderMeshHolder rmHolder;

  csFlags flags;
  csMeshWrapper* meshwrapper;

  bool ExtraVisTest (iRenderView* rview, csReversibleTransform& tr_o2c,
  	csVector3& camera_origin);

protected:
  /**
   * Destructor.  This is private in order to force clients to use DecRef()
   * for object destruction.
   */
  virtual ~csPortalContainer ();

public:
  /// Constructor.
  csPortalContainer (iEngine* engine, iObjectRegistry *object_reg);
  void SetMeshWrapper (csMeshWrapper* meshwrapper)
  {
    csPortalContainer::meshwrapper = meshwrapper;
  }

  uint32 GetDataNumber () const { return data_nr; }
  void Prepare ();
  csDirtyAccessArray<csVector3>* GetVertices () { return &vertices; }
  csDirtyAccessArray<csVector3>* GetWorldVertices () { return &world_vertices; }
  const csRefArray<csPortal>& GetPortals () const { return portals; }

  /// Check if the object to world needs updating.
  void CheckMovable ();
  /// Transform from object to world space.
  void ObjectToWorld (const csMovable& movable,
  	const csReversibleTransform& movtrans);
  /// Transform from world to camera space.
  void WorldToCamera (iCamera* camera, const csReversibleTransform& camtrans);

  SCF_DECLARE_IBASE_EXT (csMeshObject);

  //-------------------- iPolygonMesh interface implementation ---------------
  csPortalContainerPolyMeshHelper scfiPolygonMesh;
  //------------------- CD iPolygonMesh implementation ---------------
  csPortalContainerPolyMeshHelper scfiPolygonMeshCD;
  //------------------- LOD iPolygonMesh implementation ---------------
  csPortalContainerPolyMeshHelper scfiPolygonMeshLOD;

  //-------------------For iShadowReceiver ----------------------------//
  virtual void CastShadows (iMovable* movable, iFrustumView* fview);

  //-------------------For iPortalContainer ----------------------------//
  virtual iPortal* CreatePortal (csVector3* vertices, int num);
  virtual void RemovePortal (iPortal* portal);
  virtual int GetPortalCount () const { return (int)portals.Length () ; }
  virtual iPortal* GetPortal (int idx) const { return (iPortal*)portals[idx]; }
  virtual void Draw (iRenderView* rview)
  {
    Draw (rview, 0, CS_ZBUF_NONE);
  }

  //--------------------- For iMeshObject ------------------------------//
  virtual iMeshObjectFactory* GetFactory () const { return 0; }
  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> Clone () { return 0; }
  virtual bool DrawTest (iRenderView* rview, iMovable* movable,
  	uint32 frustum_mask);
  virtual bool Draw (iRenderView* rview, iMovable* movable,
  	csZBufMode zbufMode);
  virtual void HardTransform (const csReversibleTransform& t);
  virtual bool SupportsHardTransform () const { return true; }
  virtual bool HitBeamOutline (const csVector3& start,
  	const csVector3& end, csVector3& isect, float* pr);
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr, int* polygon_idx = 0);

  virtual csRenderMesh** GetRenderMeshes (int& num, iRenderView* rview, 
    iMovable* movable, uint32 frustum_mask);

  //--------------------- For csMeshObject ------------------------------//
  virtual void GetObjectBoundingBox (csBox3& bbox)
  {
    Prepare ();
    bbox = object_bbox;
  }
  virtual void SetObjectBoundingBox (const csBox3& bbox)
  {
    object_bbox = bbox;
    scfiObjectModel.ShapeChanged ();
  }
  virtual void GetRadius (csVector3& radius, csVector3& center);
};

#endif // __CS_PORTALCONTAINER_H__
