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
#include "igeom/objmodel.h"
#include "imesh/object.h"

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
 * <p>
 * For the more technically minded:
 * <ul>
 * <li>Shadow: mesh object
 * <li>Creature: another mesh object
 * </ul>
 * This mesh object follows another mesh object and it will render a bounding
 * box for that object.
 */
class csShadow : public iMeshObject
{
private:
  iBase* logparent;
  iMeshWrapper* wrap;
  bool do_bbox;	// Show bounding box.
  bool do_rad;	// Show bounding sphere.
  csFlags flags;
  iCamera* keep_camera;

public:

  csShadow ();
  virtual ~csShadow ();

  /**
   * Get renderview found when rendering the shadow mesh.
   */
  iCamera* GetCamera () const { return keep_camera; }

  /**
   * Clear renderview.
   */
  void ClearCamera () { keep_camera = 0; }

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
  void SetShowOptions (bool bbox, bool rad)
  {
    do_bbox = bbox;
    do_rad = rad;
  }

  /**
   * Get what we are showing.
   */
  void GetShowOptions (bool& bbox, bool& rad) const
  {
    bbox = do_bbox;
    rad = do_rad;
  }

  void GetObjectBoundingBox (csBox3& bbox)
  {
    bbox.Set (-100000, -100000, -100000, 100000, 100000, 100000);
    return;
  }
  void SetObjectBoundingBox (const csBox3&) { }
  void GetRadius (csVector3& rad, csVector3& cent)
  {
     rad.Set (200000, 200000, 200000);
     cent.Set (0,0,0);
  }

  SCF_DECLARE_IBASE;

  virtual iMeshObjectFactory* GetFactory () const { return 0; }
  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> Clone () { return 0; }
  virtual csRenderMesh** GetRenderMeshes (int& n, iRenderView* rview, 
    iMovable* movable, uint32);
  virtual void SetVisibleCallback (iMeshObjectDrawCallback*) { }
  virtual iMeshObjectDrawCallback* GetVisibleCallback () const { return 0; }
  virtual void NextFrame (csTicks, const csVector3& /*pos*/) { }
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
  virtual bool HitBeamOutline (const csVector3&, const csVector3&,
        csVector3&, float*)
  { return false; }
  virtual bool HitBeamObject (const csVector3&, const csVector3&,
  	csVector3&, float*, int* = 0) { return false; }
  virtual void SetLogicalParent (iBase* lp) { logparent = lp; }
  virtual iBase* GetLogicalParent () const { return logparent; }

  //------------------------- iObjectModel implementation ----------------
  class ObjectModel : public iObjectModel
  {
    SCF_DECLARE_EMBEDDED_IBASE (csShadow);
    virtual long GetShapeNumber () const { return 1; }
    virtual iPolygonMesh* GetPolygonMeshBase () { return 0; }
    virtual iPolygonMesh* GetPolygonMeshColldet () { return 0; }
    virtual void SetPolygonMeshColldet (iPolygonMesh*) { }
    virtual iPolygonMesh* GetPolygonMeshViscull () { return 0; }
    virtual void SetPolygonMeshViscull (iPolygonMesh*) { }
    virtual iPolygonMesh* GetPolygonMeshShadows () { return 0; }
    virtual void SetPolygonMeshShadows (iPolygonMesh*) { }
    virtual csPtr<iPolygonMesh> CreateLowerDetailPolygonMesh (float)
    { return 0; }
    virtual void GetObjectBoundingBox (csBox3& bbox)
    {
      scfParent->GetObjectBoundingBox (bbox);
    }
    virtual void SetObjectBoundingBox (const csBox3& bbox)
    {
      scfParent->SetObjectBoundingBox (bbox);
    }
    virtual void GetRadius (csVector3& rad, csVector3& cent)
    {
      scfParent->GetRadius (rad, cent);
    }
    virtual void AddListener (iObjectModelListener*)
    {
    }
    virtual void RemoveListener (iObjectModelListener*)
    {
    }
  } scfiObjectModel;
  friend class ObjectModel;

  virtual iObjectModel* GetObjectModel () { return &scfiObjectModel; }
  virtual bool SetColor (const csColor&) { return false; }
  virtual bool GetColor (csColor&) const { return false; }
  virtual bool SetMaterialWrapper (iMaterialWrapper*) { return false; }
  virtual iMaterialWrapper* GetMaterialWrapper () const { return 0; }
  virtual void InvalidateMaterialHandles () { }
  /**
   * see imesh/object.h for specification. The default implementation
   * does nothing.
   */
  virtual void PositionChild (iMeshObject* child,csTicks current_time) { }
};

#endif // __CS_SHADOW_H__
