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

#ifndef __CS_OCCCULL_H__
#define __CS_OCCCULL_H__

#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/dbghelp.h"
#include "csplugincommon/rendermanager/rendertree.h"
#include "csplugincommon/rendermanager/render.h"
#include "csutil/array.h"
#include "csutil/list.h"
#include "csutil/parray.h"
#include "csutil/scf_implementation.h"
#include "csutil/hash.h"
#include "csutil/set.h"
#include "csutil/weakref.h"
#include "csgeom/plane3.h"
#include "csgeom/kdtree.h"
#include "imesh/objmodel.h"
#include "iengine/viscull.h"
#include "iengine/movable.h"
#include "iengine/mesh.h"
#include "aabbtree.h"
#include <map>
#include <queue>

enum NodeVisibility
{
  NODE_INVISIBLE,
  NODE_VISIBLE,
  NODE_INSIDE
};

class csKDTree;
class csKDTreeChild;
class csFrustumVis;
struct iMovable;
struct iMeshWrapper;

struct FrustTest_Front2BackData
{
  csVector3 pos;
  iRenderView* rview;
  csPlane3* frustum;
  csBox3 global_bbox;
  uint32 current_timestamp;
  // this is the callback to call when we discover a visible node
  iVisibilityCullerListener* viscallback;
};

//----------------------------------------------------------------------

/**
 * This object is a wrapper for an iVisibilityObject from the engine.
 */
class csFrustVisObjectWrapper :
  public scfImplementation2<csFrustVisObjectWrapper,
    iObjectModelListener, iMovableListener>
{
public:
  csFrustVisObjectWrapper (csFrustumVis* frustvis) :
    scfImplementationType(this), frustvis(frustvis) 
  {
  }

  csFrustumVis* frustvis;
  csRef<iVisibilityObject> visobj;
  csKDTreeChild* child;
  long update_number;	// Last used update_number from movable.
  long shape_number;	// Last used shape_number from model.

  // Optional data for shadows. Both fields can be 0.
  csRef<iMeshWrapper> mesh;

  /// The object model has changed.
  virtual void ObjectModelChanged (iObjectModel* model);
  /// The movable has changed.
  virtual void MovableChanged (iMovable* movable);
  /// The movable is about to be destroyed.
  virtual void MovableDestroyed (iMovable*) { }

  virtual ~csFrustVisObjectWrapper () { }
};

class NodeLeafData
{
public:
  NodeLeafData() : g3d(0), mesh(0), OCQueryLeafID(0)
  {
  }
  NodeLeafData(iGraphics3D* g3d,iMeshWrapper* mesh,const csBox3 &bbox)
  {
    this->g3d=g3d;
    this->mesh=mesh;
    this->bbox=bbox;
    if(g3d)
    {
      g3d->OQInitQueries(&OCQueryLeafID,1);
    }
  }

  unsigned int OCQueryLeafID; 
  iGraphics3D* g3d;
  iMeshWrapper* mesh;
  csBox3 bbox;

  const csBox3& GetBBox() const { return bbox; }
  unsigned int GetQueryLeafID() const
  {
     return OCQueryLeafID;
  }

  bool LeafQueryFinished() const
  {
    return g3d->OQueryFinished(OCQueryLeafID);
  }

  ~NodeLeafData()
  {
    g3d=0;
    mesh=0;
    if(g3d && OCQueryLeafID)
    {
      g3d->OQDelQueries(&OCQueryLeafID,1);
    }
  }
};

class NodeData
{
  csVector3 vertices[25];
  csVector4 colors[25];
public:
  NodeData()
  {
    srmSimpRendMesh.vertices=vertices;
    srmSimpRendMesh.colors=colors;
    OCQueryID=0;
    g3d=0;
  }
  
  iGraphics3D* g3d;
  unsigned int OCQueryID;
  csSimpleRenderMesh srmSimpRendMesh;
  std::map<iCamera*,uint32> mapCameraTimestamp;
  std::map<iCamera*,bool> mapCameraVisibility;
  //std::map<iCamera*,bool[2]> mapCameraChildrenVisibility;

  uint32 GetCameraTimestamp(iCamera* cam) const
  {
    return mapCameraTimestamp.find(cam)->second;
  }

  bool GetVisibilityForCamera(iCamera* cam) const
  {
    return mapCameraVisibility.find(cam)->second;
  }

  unsigned int GetQueryID() const
  {
     return OCQueryID;
  }

  bool InnerQueryFinished() const
  {
    return g3d->OQueryFinished(OCQueryID);
  }

  iGraphics3D* GetGraphics3D() const
  {
    return g3d;
  }

  void SetCameraTimestamp(iCamera* cam, const uint32 timestamp)
  {
    mapCameraTimestamp[cam]=timestamp;
  }

  void SetVisibilityForCamera(iCamera* cam, bool bVisibility)
  {
    mapCameraVisibility[cam]=bVisibility;
  }

  void SetQueryID(unsigned int ocq)
  {
    OCQueryID=ocq;
  }

  void LeafAddObject (NodeLeafData* data)
  {
    if(!g3d)
    {
      g3d=data->g3d;
      g3d->OQInitQueries(&OCQueryID,1);
    }
    CreateBBoxMesh(data->bbox);
  }

  void LeafUpdateObjects (NodeLeafData**, uint)
  {
  }

  void NodeUpdate (const NodeData& child1, 
                   const NodeData& child2,
                   const csBox3& box)
  {
    CreateBBoxMesh(box);
  }

  void CreateBBoxMesh(const csBox3& box)
  {
    vertices[0]=csVector3(box.MinX(),box.MinY(),box.MaxZ());
    vertices[1]=csVector3(box.MaxX(),box.MinY(),box.MaxZ());
    vertices[2]=csVector3(box.MaxX(),box.MaxY(),box.MaxZ());
    vertices[3]=csVector3(box.MinX(),box.MaxY(),box.MaxZ());

    vertices[4]=csVector3(box.MaxX(),box.MinY(),box.MinZ());
    vertices[5]=csVector3(box.MinX(),box.MinY(),box.MinZ());
    vertices[6]=csVector3(box.MinX(),box.MaxY(),box.MinZ());
    vertices[7]=csVector3(box.MaxX(),box.MaxY(),box.MinZ());

    vertices[8]=csVector3(box.MinX(),box.MinY(),box.MinZ());
    vertices[9]=csVector3(box.MinX(),box.MinY(),box.MaxZ());
    vertices[10]=csVector3(box.MinX(),box.MaxY(),box.MaxZ());
    vertices[11]=csVector3(box.MinX(),box.MaxY(),box.MinZ());

    vertices[12]=csVector3(box.MaxX(),box.MinY(),box.MaxZ());
    vertices[13]=csVector3(box.MaxX(),box.MinY(),box.MinZ());
    vertices[14]=csVector3(box.MaxX(),box.MaxY(),box.MinZ());
    vertices[15]=csVector3(box.MaxX(),box.MaxY(),box.MaxZ());

    vertices[16]=csVector3(box.MinX(),box.MaxY(),box.MinZ());
    vertices[17]=csVector3(box.MinX(),box.MaxY(),box.MaxZ());
    vertices[18]=csVector3(box.MaxX(),box.MaxY(),box.MaxZ());
    vertices[19]=csVector3(box.MaxX(),box.MaxY(),box.MinZ());

    vertices[20]=csVector3(box.MaxX(),box.MinY(),box.MinZ());
    vertices[21]=csVector3(box.MaxX(),box.MinY(),box.MaxZ());
    vertices[22]=csVector3(box.MinX(),box.MinY(),box.MaxZ());
    vertices[23]=csVector3(box.MinX(),box.MinY(),box.MinZ());

    colors[0]=csVector4(1.0f,1.0f,1.0f);
    for(int i=1;i<24; ++i)
    {
      colors[i]=colors[0];
    }

    srmSimpRendMesh.vertices=vertices;
    srmSimpRendMesh.colors=colors;
    srmSimpRendMesh.vertexCount=24;

    srmSimpRendMesh.meshtype = CS_MESHTYPE_QUADS;
    csAlphaMode alf;
    alf.alphaType = alf.alphaSmooth;
    alf.autoAlphaMode = false;
    srmSimpRendMesh.alphaType = alf;
    srmSimpRendMesh.z_buf_mode=CS_ZBUF_TEST;
  }

  ~NodeData()
  {
    if(g3d && OCQueryID)
    {
      g3d->OQDelQueries(&OCQueryID,1);
    }
  }
};

typedef AABBTree<NodeLeafData,1,NodeData>::Node* NodePtr;

/**
 * A simple frustum based visibility culling system.
 */
class csFrustumVis :
  public scfImplementation1<csFrustumVis, iVisibilityCuller>
{
public:
  // List of objects to iterate over (after VisTest()).
  typedef csArray<iVisibilityObject*, csArrayElementHandler<iVisibilityObject*>,
    CS::Container::ArrayAllocDefault, csArrayCapacityFixedGrow<256> >
    VistestObjectsArray;
  VistestObjectsArray vistest_objects;
  bool vistest_objects_inuse;	// If true the vector is in use.

private:
  iObjectRegistry *object_reg;
  csEventID CanvasResize;
  csRef<iEventHandler> weakEventHandler;
  csKDTree* kdtree;
  // Ever growing box of all objects that were ever in the tree.
  // This puts an upper limit of all boxes in the kdtree itself because
  // those go off to infinity.
  csBox3 kdtree_box;
  csRefArray<csFrustVisObjectWrapper, CS::Container::ArrayAllocDefault, 
    csArrayCapacityFixedGrow<256> > visobj_vector;
  std::map<iMeshWrapper*,NodeLeafData*> mapNodeLeafData;

  int scr_width, scr_height;	// Screen dimensions.
  uint32 current_vistest_nr;

  // This hash set holds references to csFrustVisObjectWrapper instances
  // that require updating in the culler.
  csSet<csPtrKey<csFrustVisObjectWrapper> > update_queue;
  // The 'updating' flag is true if the objects are being updated. This flag
  // is to prevent us from updating it again (if the callback is fired
  // again).
  bool updating;

  // Update all objects in the update queue.
  void UpdateObjects ();

  // Fill the bounding box with the current object status.
  void CalculateVisObjBBox (iVisibilityObject* visobj, csBox3& bbox);

  csRef<iGraphics3D> g3d;

  std::queue<const NodePtr> Queries;
  AABBTree<NodeLeafData,1,NodeData > aabbTree;

  // Frustum data (front to back)
  FrustTest_Front2BackData f2bData;

public:
  csFrustumVis ();
  virtual ~csFrustumVis ();
  virtual bool Initialize (iObjectRegistry *object_reg);

  // Test visibility for the given node. Returns 2 if camera is inside node,
  // 1 if visible normally, or 0 if not visible.
  // This function will also modify the frustum_mask in 'data'. So
  // take care to restore this later if you recurse down.
  int TestNodeVisibility (csKDTree* treenode,
  	FrustTest_Front2BackData* data, uint32& frustum_mask);

  // Test visibility for the given object. Returns true if visible.
  bool TestObjectVisibility (csFrustVisObjectWrapper* obj,
  	FrustTest_Front2BackData* data, uint32 frustum_mask);

  // Add an object to the update queue. That way it will be updated
  // in the kdtree later when needed.
  void AddObjectToUpdateQueue (csFrustVisObjectWrapper* visobj_wrap);

  virtual iGraphics3D* GetGraphics3D() const
  {
    return g3d;
  }

  // Update one object in FrustVis. This is called whenever the movable
  //   or object model changes.
  void UpdateObject (csFrustVisObjectWrapper* visobj_wrap);

  virtual void Setup (const char* name);
  virtual void RegisterVisObject (iVisibilityObject* visobj);
  virtual void UnregisterVisObject (iVisibilityObject* visobj);
  virtual bool VisTest (iRenderView* rview, 
    iVisibilityCullerListener* viscallback, int w = 0, int h = 0);
  virtual void PrecacheCulling () { VisTest ((iRenderView*)0, 0); }
  virtual csPtr<iVisibilityObjectIterator> VisTest (const csBox3& box);
  virtual csPtr<iVisibilityObjectIterator> VisTest (const csSphere& sphere);
  virtual void VisTest (const csSphere& sphere, 
    iVisibilityCullerListener* viscallback);
  virtual csPtr<iVisibilityObjectIterator> VisTest (csPlane3* planes,
  	int num_planes);
  virtual void VisTest (csPlane3* planes,
  	int num_planes, iVisibilityCullerListener* viscallback);
  virtual csPtr<iVisibilityObjectIterator> IntersectSegmentSloppy (
    const csVector3& start, const csVector3& end);
  virtual csPtr<iVisibilityObjectIterator> IntersectSegment (
    const csVector3& start, const csVector3& end, bool accurate = false);
  virtual bool IntersectSegment (const csVector3& start,
    const csVector3& end, csVector3& isect, float* pr = 0,
    iMeshWrapper** p_mesh = 0, int* poly_idx = 0,
    bool accurate = true);
  virtual const char* ParseCullerParameters (iDocumentNode*) { return 0; }
};

/*----------------------------------------------------------------------------*/

class csFrustVisObjIt :
  public scfImplementation1<csFrustVisObjIt, iVisibilityObjectIterator>
{
private:
  csFrustumVis::VistestObjectsArray* vector;
  size_t position;
  bool* vistest_objects_inuse;

public:
  csFrustVisObjIt (csFrustumVis::VistestObjectsArray* vector,
    bool* vistest_objects_inuse) :
    scfImplementationType(this)
  {
    csFrustVisObjIt::vector = vector;
    csFrustVisObjIt::vistest_objects_inuse = vistest_objects_inuse;
    if (vistest_objects_inuse) *vistest_objects_inuse = true;
    Reset ();
  }
  virtual ~csFrustVisObjIt ()
  {
    // If the vistest_objects_inuse pointer is not 0 we set the
    // bool to false to indicate we're no longer using the base
    // vector. Otherwise we delete the vector.
    if (vistest_objects_inuse)
      *vistest_objects_inuse = false;
    else
      delete vector;
  }

  virtual iVisibilityObject* Next()
  {
    if (position == (size_t)-1) return 0;
    iVisibilityObject* vo = vector->Get (position);
    position++;
    if (position == vector->GetSize ())
      position = (size_t)-1;
    return vo;
  }

  virtual void Reset()
  {
    if (vector == 0 || vector->GetSize () < 1)
      position = (size_t)-1;
    else
      position = 0;
  }

  virtual bool HasNext () const
  {
    return ((position != (size_t)-1) && position <= vector->GetSize ());
  }
};

class csFrustVisObjectDescriptor : public scfImplementation1<
	csFrustVisObjectDescriptor, iKDTreeObjectDescriptor>
{
public:
  csFrustVisObjectDescriptor () : scfImplementationType (this) { }
  ~csFrustVisObjectDescriptor () { }
  virtual csPtr<iString> DescribeObject (csKDTreeChild* child)
  {
    scfString* str = new scfString ();
    const csBox3& b = child->GetBBox ();
    csFrustVisObjectWrapper* obj = (csFrustVisObjectWrapper*)(
    	child->GetObject ());
    str->Format ("'%s' (%g,%g,%g)-(%g,%g,%g)",
    	obj->mesh->QueryObject ()->GetName (),
	b.MinX (), b.MinY (), b.MinZ (),
	b.MaxX (), b.MaxY (), b.MaxZ ());
    return str;
  }
};

#endif // __CS_FRUSTVIS_H__

