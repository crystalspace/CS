/*
    Copyright (C) 2009 by Mike Gist and Jorrit Tyberghein

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

#ifndef __CS_OCCLUVIS_H__
#define __CS_OCCLUVIS_H__

#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/dbghelp.h"
#include "csplugincommon/rendermanager/rendertree.h"
#include "csutil/array.h"
#include "csutil/list.h"
#include "csutil/parray.h"
#include "csutil/scf_implementation.h"
#include "csutil/hash.h"
#include "csutil/set.h"
#include "csutil/weakref.h"
#include "csgeom/plane3.h"
#include "imesh/objmodel.h"
#include "iengine/viscull.h"
#include "iengine/movable.h"
#include "iengine/mesh.h"

class csKDTree;
class csKDTreeChild;
class csOccluVis;
struct iMovable;
struct iMeshWrapper;

struct FrustTest_Front2BackData;

/**
 * This object is a wrapper for an iVisibilityObject from the engine.
 */
class csOccluVisObjectWrapper :
  public scfImplementation2<csOccluVisObjectWrapper,
    iObjectModelListener, iMovableListener>
{
public:
  csOccluVis* occluvis;
  csRef<iVisibilityObject> visobj;
  csKDTreeChild* child;
  long update_number;	// Last used update_number from movable.
  long shape_number;	// Last used shape_number from model.
  bool wasVisible; // Was this object visible last time?

  // Optional data for shadows. Both fields can be 0.
  csRef<iMeshWrapper> mesh;

  csOccluVisObjectWrapper (csOccluVis* occluvis) :
    scfImplementationType(this), occluvis(occluvis), wasVisible(false) { }
  virtual ~csOccluVisObjectWrapper () { }

  /// The object model has changed.
  virtual void ObjectModelChanged (iObjectModel* model);
  /// The movable has changed.
  virtual void MovableChanged (iMovable* movable);
  /// The movable is about to be destroyed.
  virtual void MovableDestroyed (iMovable*) { }
};

typedef CS::RenderManager::RenderTree<
CS::RenderManager::RenderTreeStandardTraits> RenderTreeType;

/**
 * A occlusion query based visibility culler.
 * Written with the CHC++ algorithm in mind.
 */
class csOccluVis :
  public scfImplementation3<csOccluVis,
    iVisibilityCuller, iEventHandler, iComponent>
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
  csRef<iGraphics3D> g3d;
  csRef<iShaderManager> shaderManager;
  csEventID CanvasResize;
  csRef<iEventHandler> weakEventHandler;
  csKDTree* kdtree;
  // Ever growing box of all objects that were ever in the tree.
  // This puts an upper limit of all boxes in the kdtree itself because
  // those go off to infinity.
  csBox3 kdtree_box;
  csRefArray<csOccluVisObjectWrapper, CS::Container::ArrayAllocDefault, 
    csArrayCapacityFixedGrow<256> > visobj_vector;
  int scr_width, scr_height;	// Screen dimensions.
  uint32 current_vistest_nr;

  // This hash set holds references to csOccluVisObjectWrapper instances
  // that require updating in the culler.
  csSet<csPtrKey<csOccluVisObjectWrapper> > update_queue;
  // The 'updating' flag is true if the objects are being updated. This flag
  // is to prevent us from updating it again (if the callback is fired
  // again).
  bool updating;

  // Update all objects in the update queue.
  void UpdateObjects ();

  // Fill the bounding box with the current object status.
  void CalculateVisObjBBox (iVisibilityObject* visobj, csBox3& bbox);

  // Traverse the kdtree for frustum culling.
  void FrustTest_Traverse (csKDTree* treenode,
	FrustTest_Front2BackData* data,
	uint32 cur_timestamp, uint32 frustum_mask);

  class csVisibilityObjectHistory :
    public scfImplementation1<csVisibilityObjectHistory, iKDTreeUserData>
  {
  public:
    bool wasVisible;

    csVisibilityObjectHistory () :
    scfImplementationType (this), wasVisible(false)
    {
    }

    virtual ~csVisibilityObjectHistory()
    {
    }
  };

  enum NodeVisibility
  {
    INVISIBLE,
    VISIBLE,
    INSIDE
  };

  struct OcclusionQueries
  {
    // Query data.
    size_t numQueries;
    unsigned int* query;
    csRenderMesh** rm;
    CS::Graphics::RenderPriority* rp;
    RenderTreeType::MeshNode::SingleMesh* sm;

    // State data.
    size_t currentQuery;

    OcclusionQueries () : numQueries (0),
      query (0), rm (0), rp (0), sm (0),
      currentQuery (0)
    {
    }
  };

  struct TransversalData
  {
    csKDTree* parent;
    csKDTree* treenode;
    csKDTreeChild* treeleaf;
    OcclusionQueries queries;
    bool parentTotallyVisible;
    uint32 frustum_mask;
  };

  csList<TransversalData> TransversalQueue;

  csList<TransversalData> PushedQueue;
  csList<TransversalData> QueryQueue;
  csList<TransversalData> DelayedQueryQueue;

  bool WasVisible(TransversalData& data);
  void TransverseNode(TransversalData& tdata, uint32 cur_timestamp, bool parentTotallyVisible);

  RenderTreeType::PersistentData treePersistent;
  void RenderZMeshQuery(iRenderView* rview);
  void PushVisibleRenderMeshes(OcclusionQueries& query, iMeshWrapper *imesh,
    uint32 frustum_mask, iRenderView* rview);

  // Render context.
  RenderTreeType::ContextNode* OQContext;

  // Render tree.
  RenderTreeType* renderTree;

  size_t OQContextRMeshes;

public:
  csOccluVis (iBase *iParent);
  virtual ~csOccluVis ();
  virtual bool Initialize (iObjectRegistry *object_reg);

  // Test visibility for the given node. Returns 2 if camera is inside node,
  // 1 if visible normally, or 0 if not visible.
  // This function will also modify the frustum_mask in 'data'. So
  // take care to restore this later if you recurse down.
  NodeVisibility TestNodeVisibility (csKDTree* treenode,
  	FrustTest_Front2BackData* data, uint32& frustum_mask);

  // Test visibility for the given object. Returns true if visible.
  bool TestObjectVisibility (csOccluVisObjectWrapper* obj,
  	FrustTest_Front2BackData* data, uint32 frustum_mask);

  // Add an object to the update queue. That way it will be updated
  // in the kdtree later when needed.
  void AddObjectToUpdateQueue (csOccluVisObjectWrapper* visobj_wrap);

  // Update one object in Occluvis. This is called whenever the movable
  //   or object model changes.
  void UpdateObject (csOccluVisObjectWrapper* visobj_wrap);

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

  bool HandleEvent (iEvent& ev);

  CS_EVENTHANDLER_NAMES("crystalspace.occluvis")
  CS_EVENTHANDLER_NIL_CONSTRAINTS
};

#endif // __CS_OCCLUVIS_H__

