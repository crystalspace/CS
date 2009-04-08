/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __CS_SHADOW_H__
#define __CS_SHADOW_H__

#include "csgeom/box.h"
#include "csutil/flags.h"
#include "csutil/scf_implementation.h"
#include "imesh/objmodel.h"
#include "imesh/object.h"
#include "ivideo/graph3d.h"

struct iEngine;
struct iMeshWrapper;
struct iCamera;

/**
 * BugPlug is the hiding place for many dark creatures. While Spider only
 * places a curse on the country and any unsuspecting wanderer that might
 * visit the country, Shadow takes possession of some creature and clings
 * to it with all its power. Unless Shadow willingly releases the creature
 * there is nothing that can be done to unbind Shadow.
 * The Shadow slowly devours the poor creature from all its knowledge.
 *
 * For the more technically minded:
 * - Shadow: mesh object
 * - Creature: another mesh object
 *
 * This mesh object follows another mesh object and it will render a bounding
 * box for that object.
 */
class csShadow :
  public scfImplementation2<csShadow, iMeshObject, iObjectModel>
{
private:
  iMeshWrapper* logparent;
  iMeshWrapper* wrap;
  bool do_bbox;	// Show bounding box.
  bool do_rad;	// Show bounding sphere.
  bool do_normals; // Show normals
  bool do_skeleton; // Show skeleton
  csFlags flags;
  csRef<iRenderView> keep_view;
  csBox3 bbox;

public:

  csShadow ();
  virtual ~csShadow ();

  /**
   * Get renderview found when rendering the shadow mesh.
   */
  iRenderView* GetView () const { return keep_view; }

  /**
   * Clear renderview.
   */
  void ClearView () { keep_view = 0; }

  /**
   * Add Shadow to the engine.
   */
  bool AddToEngine (iEngine* engine);

  /**
   * Remove the shadow from the engine.
   */
  void RemoveFromEngine (iEngine* engine);

  /**
   * Set what we are showing.
   */
  void SetShowOptions (bool bbox, bool rad, bool norm, bool skel)
  {
    do_bbox = bbox;
    do_rad = rad;
    do_normals = norm;
    do_skeleton = skel;
  }

  /**
   * Get what we are showing.
   */
  void GetShowOptions (bool& bbox, bool& rad, bool& norm, bool& skel) const
  {
    bbox = do_bbox;
    rad = do_rad;
    norm = do_normals;
    skel = do_skeleton;
  }

  const csBox3& GetObjectBoundingBox ()
  {
    bbox.Set (-100000, -100000, -100000, 100000, 100000, 100000);
    return bbox;
  }
  void SetObjectBoundingBox (const csBox3&) { }
  void GetRadius (float& rad, csVector3& cent)
  {
    rad = 200000;
    cent.Set (0,0,0);
  }

  virtual iMeshObjectFactory* GetFactory () const { return 0; }
  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> Clone () { return 0; }
  virtual CS::Graphics::RenderMesh** GetRenderMeshes (int& n, iRenderView* rview, 
    iMovable* movable, uint32);
  virtual void SetVisibleCallback (iMeshObjectDrawCallback*) { }
  virtual iMeshObjectDrawCallback* GetVisibleCallback () const { return 0; }
  virtual void NextFrame (csTicks, const csVector3& /*pos*/,
    uint /*currentFrame*/) { }
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
  virtual bool HitBeamOutline (const csVector3&, const csVector3&,
        csVector3&, float*)
  { return false; }
  virtual bool HitBeamObject (const csVector3&, const csVector3&,
  	csVector3&, float*, int* = 0, iMaterialWrapper** = 0,
    csArray<iMaterialWrapper*>* materials = 0) { return false; }
  virtual void SetMeshWrapper (iMeshWrapper* lp) { logparent = lp; }
  virtual iMeshWrapper* GetMeshWrapper () const { return logparent; }

  //------------------------- iObjectModel implementation ----------------
  virtual long GetShapeNumber () const { return 1; }
  virtual iTriangleMesh* GetTriangleData (csStringID) { return 0; }
  virtual csPtr<iTriangleMeshIterator> GetTriangleDataIterator ()
  { return 0; }
  virtual void SetTriangleData (csStringID, iTriangleMesh*) { }
  virtual bool IsTriangleDataSet (csStringID) { return false; }
  virtual void ResetTriangleData (csStringID) { }
  virtual iTerraFormer* GetTerraFormerColldet () { return 0; }
  virtual iTerrainSystem* GetTerrainColldet () { return 0; }
  virtual void AddListener (iObjectModelListener*) { }
  virtual void RemoveListener (iObjectModelListener*) { }

  virtual iObjectModel* GetObjectModel () { return this; }
  virtual bool SetColor (const csColor&) { return false; }
  virtual bool GetColor (csColor&) const { return false; }
  virtual bool SetMaterialWrapper (iMaterialWrapper*) { return false; }
  virtual iMaterialWrapper* GetMaterialWrapper () const { return 0; }
  virtual void SetMixMode (uint) { }
  virtual uint GetMixMode () const { return CS_FX_COPY; }

  /**
   * see imesh/object.h for specification. The default implementation
   * does nothing.
   */
  virtual void PositionChild (iMeshObject* /*child*/, csTicks /*current_time*/) { }

  virtual void BuildDecal(const csVector3* pos, float decalRadius,
	iDecalBuilder* decalBuilder)
  {
  }

};

#endif // __CS_SHADOW_H__
