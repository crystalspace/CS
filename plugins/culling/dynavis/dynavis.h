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

#ifndef __CS_DYNAVIS_H__
#define __CS_DYNAVIS_H__

#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/dbghelp.h"
#include "csutil/array.h"
#include "csutil/parray.h"
#include "csutil/hash.h"
#include "csutil/blockallocator.h"
#include "csutil/scf.h"
#include "csutil/set.h"
#include "csutil/leakguard.h"
#include "igeom/objmodel.h"
#include "iengine/movable.h"
#include "csgeom/plane3.h"
#include "csgeom/tcovbuf.h"
#include "iengine/viscull.h"
#include "dmodel.h"
#include "dhistmgr.h"
#include "wqueue.h"

// @@@ Hack(s) to avoid problems with static linking
#ifdef DYNAVIS_DEBUG
#define csVisibilityObjectWrapper	csVisibilityObjectWrapper_DEBUG
#define csDynaVis			csDynaVis_DEBUG
#endif

class csKDTree;
class csKDTreeChild;
class csCoverageBuffer;
class csTiledCoverageBuffer;
class csDynaVis;
struct csTestRectData;
struct iPolygonMesh;
struct iMovable;
struct iMeshWrapper;
struct iBugPlug;
struct iMeshWrapper;
struct iShadowCaster;
struct iShadowReceiver;
struct iThingState;

#define VIEWMODE_STATS 0
#define VIEWMODE_STATSOVERLAY 1
#define VIEWMODE_CLEARSTATSOVERLAY 2
#define VIEWMODE_OUTLINES 3

#define VIEWMODE_FIRST 0
#define VIEWMODE_LAST 3

#define COVERAGE_NONE 0
#define COVERAGE_POLYGON 1
#define COVERAGE_OUTLINE 2

struct VisTest_Front2BackData;

/**
 * This object is a wrapper for an iVisibilityObject from the engine.
 */
class csVisibilityObjectWrapper : public iObjectModelListener,
	public iMovableListener
{
public:
  CS_LEAKGUARD_DECLARE (csVisibilityObjectWrapper);
  
  csDynaVis* dynavis;
  iVisibilityObject* visobj;
  csKDTreeChild* child;
  long update_number;	// Last used update_number from movable.
  long shape_number;	// Last used shape_number from model.
  csDynavisObjectModel* model;

  bool hint_closed;
  bool hint_badoccluder;
  bool hint_goodoccluder;
  bool use_outline_filler;
  bool full_transform_identity;	// Cache for IsFullTransformIdentity().

  uint32 last_visible_vistestnr;

  csVisibilityObjectHistory* history;
  // Optional data for shadows. Both fields can be 0.
  csRef<iMeshWrapper> mesh;
  csRef<iShadowCaster> caster;

  csVisibilityObjectWrapper ()
  {
    SCF_CONSTRUCT_IBASE (0);
    history = new csVisibilityObjectHistory ();
    last_visible_vistestnr = 0;
    full_transform_identity = false;
  }
  virtual ~csVisibilityObjectWrapper ()
  {
    history->DecRef ();
    SCF_DESTRUCT_IBASE();
  }
  void SetDynavis (csDynaVis* dynavis)
  {
    csVisibilityObjectWrapper::dynavis = dynavis;
  }

  void MarkInvisible (csVisReason reason)
  {
    history->reason = reason;
    history->no_writequeue_vis_cnt = 0;
    history->has_vpt_point = false;
  }

  void MarkVisible (csVisReason reason, int cnt, int no_writequeue_cnt,
  	uint32 current_vistest_nr, uint32 history_frame_cnt)
  {
    last_visible_vistestnr = current_vistest_nr;
    history->reason = reason;
    history->vis_cnt = history_frame_cnt+cnt;
    if (no_writequeue_cnt == 0)
      history->no_writequeue_vis_cnt = 0;
    else
      history->no_writequeue_vis_cnt = history_frame_cnt+cnt+no_writequeue_cnt;
    history->history_frame_cnt = history_frame_cnt;
  }

  void MarkVisibleForHistory (uint32 current_vistest_nr,
		uint32 history_frame_cnt)
  {
    history->reason = VISIBLE_HISTORY;
    last_visible_vistestnr = current_vistest_nr;
    history->history_frame_cnt = history_frame_cnt;
  }

  SCF_DECLARE_IBASE;

  /// The object model has changed.
  virtual void ObjectModelChanged (iObjectModel* model);
  /// The movable has changed.
  virtual void MovableChanged (iMovable* movable);
  /// The movable is about to be destroyed.
  virtual void MovableDestroyed (iMovable*) { }
};

/**
 * A structure that is used during vis culling to store every occluder
 * that is being used.
 */
class csOccluderInfo
{
public:
  csVisibilityObjectWrapper* obj;

  // This is a structure that is used during visibility testing
  // to hold the 2D box on screen that this object covers when used as
  // an occluder.
  csBox2Int occluder_box;

  // Total count of occluded objects for all covered tiles (these
  // are the objects that are occluded BEFORE this occluder comes into play).
  int total_notoccluded;
};

/**
 * A dynamic visisibility culling system.
 */
class csDynaVis : public iVisibilityCuller
{
public:
  // List of objects to iterate over (after VisTest()).
  csArray<iVisibilityObject*> vistest_objects;
  bool vistest_objects_inuse;	// If true the vector is in use.

private:
  csBlockAllocator<csVisibilityObjectWrapper> visobj_wrappers;

  iObjectRegistry* object_reg;
  csRef<iBugPlug> bugplug;
  csKDTree* kdtree;
  // Ever growing box of all objects that were ever in the tree.
  // This puts an upper limit of all boxes in the kdtree itself because
  // those go off to infinity.
  csBox3 kdtree_box;
  csTiledCoverageBuffer* tcovbuf;
  csArray<csVisibilityObjectWrapper*> visobj_vector;
  csObjectModelManager* model_mgr;
  csWriteQueue* write_queue;

  // List of occluders that were used this frame.
  csArray<csOccluderInfo> occluder_info;

  int scr_width, scr_height;	// Screen dimensions.
  int reduce_buf;
  float fov, sx, sy;
  csReversibleTransform cam_trans;	// Cache of the camera transform.

  uint32 current_vistest_nr;

  // For Debug_Dump(g3d): keep the last original camera.
  iCamera* debug_camera;
  float debug_lx, debug_rx, debug_ty, debug_by;	// Frustum.

  // Count the number of objects marked as visible.
  int cnt_visible;
  // Count the number of nodes marked as visible.
  int cnt_node_visible;

  // Various flags to enable/disable parts of the culling algorithm.
  static bool do_cull_frustum;
  static int do_cull_coverage;
  static bool do_cull_history;
  static bool do_cull_writequeue;
  static bool do_cull_ignoresmall;
  static bool do_cull_clampoccluder;
  static bool do_cull_vpt;
  static bool do_cull_outline_splatting;
  static bool do_insert_inverted_clipper;
  static bool do_cull_ignore_bad_occluders;
  static int badoccluder_thresshold;
  static int badoccluder_maxsweepcount;

  // Compared to history_frame_cnt to see if we must retry all bad occluders
  // again.
  uint32 badoccluder_sweepcount;
  bool badoccluder_retry;

  bool do_freeze_vis;

  // This hash set holds references to csVisibilityObjectWrapper instances
  // that require updating in the culler.
  csSet<csPtrKey<csVisibilityObjectWrapper> > update_queue;
  // The 'updating' flag is true if the objects are being updated. This flag
  // is to prevent us from updating it again (if the callback is fired
  // again).
  bool updating;

  // Update all objects in the update queue.
  void UpdateObjects ();

  // For history culling: this is used by the main VisTest() routine
  // to keep track of how many every VisTest() call. We can use that
  // to see if some object was visible previous frame.
  uint32 history_frame_cnt;

  // View mode for debugging (one of VIEWMODE_... constants).
  int cfg_view_mode;

  // Depth we will use for showing the object debug view.
  // This is the depth of the origin on screen.
  float debug_origin_z;

  // If this flag is true we will do an extensive dump of the current
  // visibility culling proceedings during the next frame. This flag
  // will immediatelly be cleared after that dump.
  bool do_state_dump;

  // Fill the bounding box with the current object status.
  void CalculateVisObjBBox (iVisibilityObject* visobj, csBox3& bbox,
  	bool full_transform_identity);

  // Given an occluder, update it in the coverage buffer.
  void UpdateCoverageBuffer (csVisibilityObjectWrapper* obj);

  // Given an occluder, update it in the coverage buffer. Using the outline.
  void UpdateCoverageBufferOutline (csVisibilityObjectWrapper* obj);

  // Append an occluder to the write queue.
  void AppendWriteQueue (iVisibilityObject* visobj,
  	csDynavisObjectModel* model, csVisibilityObjectWrapper* obj,
	const csBox2& sbox, float min_depth, float maximum);

  // Test if using the write queue is relevant for the given rectangle.
  // If not return false. This function will also mark all write queue
  // elements with 'relevant' true or false.
  bool TestWriteQueueRelevance (float min_depth,
	const csTestRectData& testrect_data, const csBox2& sbox);

public:
  SCF_DECLARE_IBASE;

  csDynaVis (iBase *iParent);
  virtual ~csDynaVis ();
  virtual bool Initialize (iObjectRegistry *object_reg);

  // Test visibility for the given node. Returns true if visible.
  // This function will also modify the frustum_mask in 'data'. So
  // take care to restore this later if you recurse down.
  bool TestNodeVisibility (csKDTree* treenode, VisTest_Front2BackData* data,
  	uint32& frustum_mask);

  // Test visibility for the given single-polygon object.
  // Returns true if visible.
  void TestSinglePolygonVisibility (csVisibilityObjectWrapper* obj,
  	VisTest_Front2BackData* data, bool& vis,
	csBox2& sbox, float& min_depth, float& max_depth,
	uint32 frustum_mask);
  // Test visibility for the given object. Returns true if visible.
  bool TestObjectVisibility (csVisibilityObjectWrapper* obj,
  	VisTest_Front2BackData* data, uint32 frustum_mask);

  // Add an object to the update queue. That way it will be updated
  // in the kdtree later when needed.
  void AddObjectToUpdateQueue (csVisibilityObjectWrapper* visobj_wrap);

  // Update one object in Dynavis. This is called whenever the movable
  // or object model changes.
  void UpdateObject (csVisibilityObjectWrapper* visobj_wrap);

  virtual void Setup (const char* name);
  virtual void RegisterVisObject (iVisibilityObject* visobj);
  virtual void UnregisterVisObject (iVisibilityObject* visobj);
  virtual bool VisTest (iRenderView* rview, 
    iVisibilityCullerListener *viscallback);
  virtual void PrecacheCulling () { VisTest ((iRenderView*)0, 0); }
  virtual csPtr<iVisibilityObjectIterator> VisTest (const csBox3& box);
  virtual csPtr<iVisibilityObjectIterator> VisTest (const csSphere& sphere);
  virtual void VisTest (const csSphere& sphere,
    iVisibilityCullerListener *viscallback);
  virtual csPtr<iVisibilityObjectIterator> VisTest (csPlane3* planes,
  	int num_planes);
  virtual void VisTest (csPlane3* planes, int num_planes,
    iVisibilityCullerListener *viscallback);
  virtual csPtr<iVisibilityObjectIterator> IntersectSegmentSloppy (
    const csVector3& start, const csVector3& end);
  virtual csPtr<iVisibilityObjectIterator> IntersectSegment (
    const csVector3& start, const csVector3& end, bool accurate = false);
  virtual bool IntersectSegment (const csVector3& start,
    const csVector3& end, csVector3& isect, float* pr = 0,
    iMeshWrapper** p_mesh = 0, int* poly_idx = 0,
    bool accurate = true);
  virtual void CastShadows (iFrustumView* fview);
  virtual const char* ParseCullerParameters (iDocumentNode*) { return 0; }

  // Debugging functions.
  csPtr<iString> Debug_UnitTest ();
  csPtr<iString> Debug_StateTest ();
  csPtr<iString> Debug_Dump ();
  void Debug_Dump (iGraphics3D* g3d);
  csTicks Debug_Benchmark (int num_iterations);
  bool Debug_DebugCommand (const char* cmd);
  csKDTree* GetKDTree () { return kdtree; }

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csDynaVis);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;

  struct DebugHelper : public iDebugHelper
  {
    SCF_DECLARE_EMBEDDED_IBASE (csDynaVis);
    virtual int GetSupportedTests () const
    {
      return CS_DBGHELP_UNITTEST | CS_DBGHELP_TXTDUMP |
      	CS_DBGHELP_STATETEST | CS_DBGHELP_BENCHMARK |
	CS_DBGHELP_GFXDUMP;
    }
    virtual csPtr<iString> UnitTest ()
    {
      return scfParent->Debug_UnitTest ();
    }
    virtual csPtr<iString> StateTest ()
    {
      return scfParent->Debug_StateTest ();
    }
    virtual csTicks Benchmark (int num_iterations)
    {
      return scfParent->Debug_Benchmark (num_iterations);
    }
    virtual csPtr<iString> Dump ()
    {
      return scfParent->Debug_Dump ();
    }
    virtual void Dump (iGraphics3D* g3d)
    {
      scfParent->Debug_Dump (g3d);
    }
    virtual bool DebugCommand (const char* cmd)
    {
      return scfParent->Debug_DebugCommand (cmd);
    }
  } scfiDebugHelper;
};

#endif // __CS_DYNAVIS_H__

