/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

#ifndef _BALL_H_
#define _BALL_H_

#include "csgeom/vector3.h"
#include "csgeom/transfrm.h"
#include "csutil/cscolor.h"
#include "imeshobj.h"
#include "imball.h"
#include "igraph3d.h"
#include "itranman.h"

struct iMaterialWrapper;
class csBallMeshObjectFactory;

/**
 * Ball version of mesh object.
 */
class csBallMeshObject : public iMeshObject
{
private:
  iMeshObjectFactory* factory;
  float radiusx, radiusy, radiusz;
  csVector3 max_radius;
  csVector3 shift;
  iMaterialWrapper* material;
  UInt MixMode;
  csMeshCallback* vis_cb;
  void* vis_cbData;

  int verts_circle;
  G3DTriangleMesh top_mesh;
  csVector3* top_normals;
  bool initialized;
  csBox3 object_bbox;

  /**
   * Camera space bounding box is cached here.
   * GetCameraBoundingBox() will check the current cookie from the
   * transformation manager to see if it needs to recalculate this.
   */
  csBox3 camera_bbox;

  /// Current cookie for camera_bbox.
  csTranCookie camera_cookie;

  /**
   * Setup this object. This function will check if setup is needed.
   */
  void SetupObject ();

  /// Generate a mesh with a sphere.
  void GenerateSphere (int num_circle, G3DTriangleMesh& mesh,
      	csVector3*& normals);

public:
  /// Constructor.
  csBallMeshObject (iMeshObjectFactory* factory);

  /// Destructor.
  virtual ~csBallMeshObject ();

  /// Get the bounding box in transformed space.
  void GetTransformedBoundingBox (iTransformationManager* tranman,
      const csReversibleTransform& trans, csBox3& cbox);
  /**
   * Get the coordinates of the ball in screen coordinates.
   * Fills in the boundingBox with the X and Y locations of the ball.
   * Returns the max Z location of the ball, or -1 if not
   * on-screen. If the ball is not on-screen, the X and Y values are not
   * valid.
   */
  float GetScreenBoundingBox (iTransformationManager* tranman, float fov,
      float sx, float sy,
      const csReversibleTransform& trans, csBox2& sbox, csBox3& cbox);

  /// Get the material for this ball.
  iMaterialWrapper* GetMaterialWrapper () { return material; }
  /// Get mixmode.
  UInt GetMixMode () { return MixMode; }
  void SetRadius (float radiusx, float radiusy, float radiusz);
  void SetShift (float shiftx, float shifty, float shiftz)
  {
    initialized = false;
    shift.Set (shiftx, shifty, shiftz);
  }
  void SetRimVertices (int num)
  {
    initialized = false;
    verts_circle = num;
    if (verts_circle <= 1) verts_circle = 2;
    else if (verts_circle >= 60) verts_circle = 59;
  }

  ///------------------------ iMeshObject implementation ------------------------
  DECLARE_IBASE;

  virtual iMeshObjectFactory* GetFactory () { return factory; }
  virtual bool DrawTest (iRenderView* rview, iMovable* movable);
  virtual void UpdateLighting (iLight** lights, int num_lights,
      	iMovable* movable);
  virtual bool Draw (iRenderView* rview, iMovable* movable);
  virtual void SetVisibleCallback (csMeshCallback* cb, void* cbData)
  {
    vis_cb = cb;
    vis_cbData = cbData;
  }
  virtual csMeshCallback* GetVisibleCallback ()
  {
    return vis_cb;
  }
  virtual void GetObjectBoundingBox (csBox3& bbox, bool accurate = false);
  virtual csVector3 GetRadius () { return max_radius; }
  virtual void NextFrame (cs_time /*current_time*/) { }
  virtual bool WantToDie () { return false; }
  virtual void HardTransform (const csReversibleTransform& t);
  virtual bool SupportsHardTransform () { return true; }
  virtual bool HitBeamObject (const csVector3&, const csVector3&,
  	csVector3&, float*) { return false; }

  //------------------------- iBallState implementation ----------------
  class BallState : public iBallState
  {
    DECLARE_EMBEDDED_IBASE (csBallMeshObject);
    virtual void SetRadius (float radiusx, float radiusy, float radiusz)
    {
      scfParent->SetRadius (radiusx, radiusy, radiusz);
    }
    virtual void SetShift (float shiftx, float shifty, float shiftz)
    {
      scfParent->SetShift (shiftx, shifty, shiftz);
    }
    virtual void SetRimVertices (int num)
    {
      scfParent->SetRimVertices (num);
    }
    virtual void SetMaterialWrapper (iMaterialWrapper* material)
    {
      scfParent->material = material;
    }
    virtual iMaterialWrapper* GetMaterialWrapper () { return scfParent->material; }
    virtual void SetMixMode (UInt mode) { scfParent->MixMode = mode; }
    virtual UInt GetMixMode () { return scfParent->MixMode; }
  } scfiBallState;
  friend class BallState;
};

/**
 * Factory for balls.
 */
class csBallMeshObjectFactory : public iMeshObjectFactory
{
public:
  /// Constructor.
  csBallMeshObjectFactory ();

  /// Destructor.
  virtual ~csBallMeshObjectFactory ();

  //------------------------ iMeshObjectFactory implementation --------------
  DECLARE_IBASE;

  virtual iMeshObject* NewInstance ();
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () { return false; }
};

/**
 * Ball type. This is the plugin you have to use to create instances
 * of csBallMeshObjectFactory.
 */
class csBallMeshObjectType : public iMeshObjectType
{
public:
  /// Constructor.
  csBallMeshObjectType (iBase*);

  /// Destructor.
  virtual ~csBallMeshObjectType ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

  //------------------------ iMeshObjectType implementation --------------
  DECLARE_IBASE;

  /// Draw.
  virtual iMeshObjectFactory* NewFactory ();
};

#endif // _BALL_H_

