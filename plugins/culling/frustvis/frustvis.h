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

#ifndef __CS_FRUSTVIS_H__
#define __CS_FRUSTVIS_H__

#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/dbghelp.h"
#include "csutil/csvector.h"
#include "csutil/scf.h"
#include "csgeom/plane3.h"
#include "iengine/viscull.h"

class csSimpleKDTree;
class csSimpleKDTreeChild;
class csFrustumVis;
struct iPolygonMesh;
struct iMovable;
struct iMeshWrapper;
struct iThingState;

struct FrustTest_Front2BackData;

/**
 * This object is a wrapper for an iVisibilityObject from the engine.
 */
class csFrustVisObjectWrapper : public iObjectModelListener, iMovableListener
{
public:
  csFrustumVis* frustvis;
  iVisibilityObject* visobj;
  csSimpleKDTreeChild* child;
  long update_number;	// Last used update_number from movable.
  long shape_number;	// Last used shape_number from model.

  // Optional data for shadows. Both fields can be NULL.
  csRef<iMeshWrapper> mesh;
  csRef<iShadowCaster> caster;
  csRef<iShadowReceiver> receiver;
  csRef<iThingState> thing_state;

  // The 'updating' flag is true if the object is being updated. This flag
  // is to prevent us from updating it again (if the callback is fired
  // again).
  bool updating;

  csFrustVisObjectWrapper (csFrustumVis* frustvis)
  {
    SCF_CONSTRUCT_IBASE (NULL);
    csFrustVisObjectWrapper::frustvis = frustvis;
    updating = false;
  }
  virtual ~csFrustVisObjectWrapper ()
  {
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
 * A simple frustum based visisibility culling system.
 */
class csFrustumVis : public iVisibilityCuller
{
public:
  // List of objects to iterate over (after VisTest()).
  csVector vistest_objects;
  bool vistest_objects_inuse;	// If true the vector is in use.

private:
  iObjectRegistry *object_reg;
  csSimpleKDTree* kdtree;
  csVector visobj_vector;
  int scr_width, scr_height;	// Screen dimensions.
  uint32 current_visnr;

  // Fill the bounding box with the current object status.
  void CalculateVisObjBBox (iVisibilityObject* visobj, csBox3& bbox);

public:
  SCF_DECLARE_IBASE;

  csFrustumVis (iBase *iParent);
  virtual ~csFrustumVis ();
  virtual bool Initialize (iObjectRegistry *object_reg);

  // Test visibility for the given node. Returns true if visible.
  // This function will also modify the frustum_mask in 'data'. So
  // take care to restore this later if you recurse down.
  bool TestNodeVisibility (csSimpleKDTree* treenode,
  	FrustTest_Front2BackData* data);

  // Test visibility for the given object. Returns true if visible.
  bool TestObjectVisibility (csFrustVisObjectWrapper* obj,
  	FrustTest_Front2BackData* data);

  // Update one object in FrustVis. This is called whenever the movable
  // or object model changes.
  void UpdateObject (csFrustVisObjectWrapper* visobj_wrap);

  virtual void Setup (const char* name);
  virtual void RegisterVisObject (iVisibilityObject* visobj);
  virtual void UnregisterVisObject (iVisibilityObject* visobj);
  virtual bool VisTest (iRenderView* rview);
  virtual csPtr<iVisibilityObjectIterator> VisTest (const csBox3& box);
  virtual csPtr<iVisibilityObjectIterator> VisTest (const csSphere& sphere);
  virtual bool IntersectSegment (const csVector3& start,
    const csVector3& end, csVector3& isect, float* pr = NULL,
    iMeshWrapper** p_mesh = NULL, iPolygon3D** poly = NULL);
  virtual void CastShadows (iFrustumView* fview);
  virtual uint32 GetCurrentVisibilityNumber () const { return current_visnr; }

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csFrustumVis);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
};

#endif // __CS_FRUSTVIS_H__

