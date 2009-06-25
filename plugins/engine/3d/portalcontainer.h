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

#include "csgeom/trimeshtools.h"
#include "csgeom/vector3.h"
#include "cstool/meshobjtmpl.h"
#include "cstool/rendermeshholder.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/refarr.h"
#include "csutil/scf_implementation.h"
#include "iengine/portalcontainer.h"
#include "ivideo/rendermesh.h"
#include "plugins/engine/3d/portal.h"

class csMeshWrapper;
class csMovable;

CS_PLUGIN_NAMESPACE_BEGIN(Engine)
{

/**
 * A helper class for iTriangleMesh implementations used by csPortalContainer.
 */
class csPortalContainerTriMeshHelper : 
  public scfImplementation1<csPortalContainerTriMeshHelper,
                            iTriangleMesh>
{
public:

  /**
   * Make a triangle mesh helper which will accept polygons which match
   * with the given flag (one of CS_POLY_COLLDET or CS_POLY_VISCULL).
   */
  csPortalContainerTriMeshHelper (uint32 flag) 
    : scfImplementationType (this), vertices (0),
      poly_flag (flag), triangles (0)
  {
  }
  virtual ~csPortalContainerTriMeshHelper ()
  {
    Cleanup ();
  }

  void Setup ();
  void SetPortalContainer (csPortalContainer* pc);

  virtual size_t GetVertexCount ()
  {
    Setup ();
    return vertices->GetSize ();
  }
  virtual csVector3* GetVertices ()
  {
    Setup ();
    return vertices->GetArray ();
  }
  virtual size_t GetTriangleCount ()
  {
    Setup ();
    return num_tri;
  }
  virtual csTriangle* GetTriangles ()
  {
    Setup ();
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
  // Array of vertices from portal container.
  csDirtyAccessArray<csVector3>* vertices;
  size_t num_tri;		// Total number of triangles.
  uint32 poly_flag;		// Polygons must match with this flag.
  csFlags flags;
  csTriangle* triangles;
};

/**
 * This is a container class for portals.
 */
class csPortalContainer : public scfImplementationExt1<csPortalContainer,
                                                       csMeshObject, 
                                                       iPortalContainer>
{
private:
  csRefArray<csPortal> portals;
  bool prepared;
  // Number that is increased with every significant change.
  uint32 data_nr;

  // Object space data.
  csDirtyAccessArray<csVector3> vertices;
  csBox3 object_bbox;
  float object_radius;

  csRef<iShaderManager> shader_man;
  csRef<iShader> fog_shader;
  CS::ShaderVarStringID fogplane_name, fogdensity_name, fogcolor_name;

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
  bool ClipToPlane (int portal_idx, csPlane3 *portal_plane,
	const csVector3 &v_w2c, csVector3 * &pverts, int &num_verts);
  /*bool DoPerspective (csVector3 *source, int num_verts,
	csPoly2D *dest, bool mirror, int fov,
	float shift_x, float shift_y, const csPlane3& plane_cam);*/
  void DrawOnePortal (csPortal* po, const csPoly2D& poly,
	const csReversibleTransform& movtrans, iRenderView *rview,
	const csPlane3& camera_plane);

  csRenderMeshHolder rmHolder;

  csFlags flags;
  csMeshWrapper* meshwrapper;

  bool ExtraVisTest (iRenderView* rview, csReversibleTransform& tr_o2c,
  	csVector3& camera_origin);

  void GetBoundingSpheres (iRenderView* rview, csReversibleTransform* tr_o2c, 
    csVector3* camera_origin, csSphere& world_sphere, csSphere& cam_sphere);
protected:
  /**
   * Destructor.  This is private in order to force clients to use DecRef()
   * for object destruction.
   */
  virtual ~csPortalContainer ();

public:
  /// Constructor.
  csPortalContainer (iEngine* engine, iObjectRegistry *object_reg);
  using iMeshObject::SetMeshWrapper;
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
  bool Draw (iRenderView* rview, iMovable* movable, csZBufMode zbufMode);

  //-------------------For iPortalContainer ----------------------------//
  virtual iPortal* CreatePortal (csVector3* vertices, int num);
  virtual void RemovePortal (iPortal* portal);
  virtual int GetPortalCount () const { return (int)portals.GetSize () ; }
  virtual iPortal* GetPortal (int idx) const { return (iPortal*)portals[idx]; }
  virtual void Draw (iRenderView* rview)
  {
    Draw (rview, 0, CS_ZBUF_NONE);
  }

  //--------------------- For iMeshObject ------------------------------//
  virtual iMeshObjectFactory* GetFactory () const { return 0; }
  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> Clone () { return 0; }
  virtual void HardTransform (const csReversibleTransform& t);
  virtual bool SupportsHardTransform () const { return true; }
  virtual bool HitBeamOutline (const csVector3& start,
  	const csVector3& end, csVector3& isect, float* pr);
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr, int* polygon_idx = 0,
	iMaterialWrapper** material = 0);

  virtual csRenderMesh** GetRenderMeshes (int& num, iRenderView* rview, 
    iMovable* movable, uint32 frustum_mask);

  //--------------------- For csMeshObject ------------------------------//
  virtual const csBox3& GetObjectBoundingBox ()
  {
    Prepare ();
    return object_bbox;
  }
  virtual void SetObjectBoundingBox (const csBox3& bbox)
  {
    object_bbox = bbox;
    ShapeChanged ();
  }
  virtual void GetRadius (float& radius, csVector3& center);
  
  void ComputeScreenPolygons (iRenderView* rview,
    csVector2* verts2D, csVector3* verts3D, size_t vertsSize, size_t* numVerts,
    int viewWidth, int viewHeight);
  
  size_t GetTotalVertexCount () const;
};

}
CS_PLUGIN_NAMESPACE_END(Engine)

#endif // __CS_PORTALCONTAINER_H__
