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
#include "csutil/csvector.h"
#include "csutil/scf.h"
#include "csgeom/plane3.h"
#include "iengine/viscull.h"

class csKDTree;
class csKDTreeChild;
class csCoverageBuffer;

enum csVisReason
{
  INVISIBLE_PARENT = 0,	// Invisible because some parent node is invisible.
  VISIBLE,		// Just visible.
  LAST_REASON
};

/**
 * This object is a wrapper for an iVisibilityObject from the engine.
 */
class csVisibilityObjectWrapper
{
public:
  iVisibilityObject* visobj;
  csKDTreeChild* child;
  long update_number;	// Last used update_number from movable.
  long shape_number;	// Last used shape_number from visobj.
  csVisReason reason;	// Reason object is visible/invisible.
};

/**
 * A dynamic visisibility culling system.
 */
class csDynaVis : public iVisibilityCuller
{
private:
  iObjectRegistry *object_reg;
  csKDTree* kdtree;
  csCoverageBuffer* covbuf;
  csVector visobj_vector;

  // For Debug_Dump(g3d): keep the last original camera.
  iCamera* debug_camera;

  // During VisTest() we keep the current frustum as planes.
  csPlane3 planeL, planeR, planeU, planeD;

  // Scan all objects, mark them as invisible and check if they
  // have moved since last frame (and update them in the kdtree then).
  void UpdateObjects ();
  // Fill the bounding box with the current object status.
  void CalculateVisObjBBox (iVisibilityObject* visobj, csBox3& bbox);
  // Calculate a screen bounding box for the given world space bbox.
  void ProjectBBox (iCamera* camera, const csBox3& bbox, csBox2& sbox);

public:
  SCF_DECLARE_IBASE;

  csDynaVis (iBase *iParent);
  virtual ~csDynaVis ();
  virtual bool Initialize (iObjectRegistry *object_reg);

  // Test visibility for the given node. Returns true if visible.
  bool TestNodeVisibility (csKDTree* treenode, iRenderView* rview,
  	const csVector3& pos);

  virtual void Setup (const char* name);
  virtual void RegisterVisObject (iVisibilityObject* visobj);
  virtual void UnregisterVisObject (iVisibilityObject* visobj);
  virtual bool VisTest (iRenderView* rview);
  virtual iPolygon3D* IntersectSegment (const csVector3& start,
    const csVector3& end, csVector3& isect, float* pr = NULL,
    iMeshWrapper** p_mesh = NULL) { return NULL; }
  virtual bool SupportsShadowCasting () { return false; }
  virtual void CastShadows (iFrustumView* /*fview*/) { }

  virtual void RegisterShadowReceiver (iShadowReceiver* /*receiver*/) { }
  virtual void UnregisterShadowReceiver (iShadowReceiver* /*receiver*/) { }

  // Debugging functions.
  iString* Debug_UnitTest ();
  iString* Debug_StateTest ();
  iString* Debug_Dump ();
  void Debug_Dump (iGraphics3D* g3d);
  csTicks Debug_Benchmark (int num_iterations);

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
    virtual iString* UnitTest ()
    {
      return scfParent->Debug_UnitTest ();
    }
    virtual iString* StateTest ()
    {
      return scfParent->Debug_StateTest ();
    }
    virtual csTicks Benchmark (int num_iterations)
    {
      return scfParent->Debug_Benchmark (num_iterations);
    }
    virtual iString* Dump ()
    {
      return scfParent->Debug_Dump ();
    }
    virtual void Dump (iGraphics3D* g3d)
    {
      scfParent->Debug_Dump (g3d);
    }
    virtual bool DebugCommand (const char*)
    {
      return false;
    }
  } scfiDebugHelper;
};

#endif // __CS_DYNAVIS_H__

