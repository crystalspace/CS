/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein

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
#include "imesh/object.h"
#include "imesh/ball.h"
#include "ivideo/graph3d.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "ivideo/vbufmgr.h"

struct iMaterialWrapper;
struct iObjectRegistry;
class csBallMeshObjectFactory;
class csColor;
class G3DFogInfo;

#define ALL_FEATURES (CS_OBJECT_FEATURE_LIGHTING)

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
  iMeshObjectDrawCallback* vis_cb;
  bool reversed;
  bool toponly;
  bool cyl_mapping;
  bool do_lighting;
  csColor color;
  float current_lod;
  uint32 current_features;

  int verts_circle;
  iVertexBuffer* vbuf;
  iVertexBufferManager* vbufmgr;

  G3DTriangleMesh top_mesh;
  csVector3* ball_vertices;
  csVector2* ball_texels;
  csColor* ball_colors;
  int num_ball_vertices;

  csVector3* top_normals;
  bool initialized;
  csBox3 object_bbox;
  long shapenr;

  /**
   * Camera space bounding box is cached here.
   * GetCameraBoundingBox() will check the current camera number from the
   * camera to see if it needs to recalculate this.
   */
  csBox3 camera_bbox;
  csBox3 world_bbox;

  /// Current camera number.
  long cur_cameranr;
  /// Current movable number.
  long cur_movablenr;

  /**
   * Setup this object. This function will check if setup is needed.
   */
  void SetupObject ();

  /// Generate a mesh with a sphere.
  void GenerateSphere (int num_circle, G3DTriangleMesh& mesh,
      	csVector3*& normals);

  /// retrieve a vertexbuffer from the manager if not done already
  void SetupVertexBuffer ();

  /// interface to receive state of vertexbuffermanager
  struct eiVertexBufferManagerClient : public iVertexBufferManagerClient
  {
    SCF_DECLARE_EMBEDDED_IBASE (csBallMeshObject);
    virtual void ManagerClosing ();
  }scfiVertexBufferManagerClient;
  friend struct eiVertexBufferManagerClient;

public:
  /// Constructor.
  csBallMeshObject (iMeshObjectFactory* factory);

  /// Destructor.
  virtual ~csBallMeshObject ();

  /// Get the bounding box in transformed space.
  void GetTransformedBoundingBox (long cameranr, long movablenr,
      const csReversibleTransform& trans, csBox3& cbox);
  /**
   * Get the coordinates of the ball in screen coordinates.
   * Fills in the boundingBox with the X and Y locations of the ball.
   * Returns the max Z location of the ball, or -1 if not
   * on-screen. If the ball is not on-screen, the X and Y values are not
   * valid.
   */
  float GetScreenBoundingBox (long cameranr, long movablenr, float fov,
  	float sx, float sy,
	const csReversibleTransform& trans, csBox2& sbox, csBox3& cbox);

  /// Get the material for this ball.
  iMaterialWrapper* GetMaterialWrapper () const { return material; }
  /// Get mixmode.
  UInt GetMixMode () const { return MixMode; }
  void SetRadius (float radiusx, float radiusy, float radiusz);
  void GetRadius (float& radx, float& rady, float& radz) const
  { radx=radiusx; rady=radiusy; radz=radiusz; }
  void SetShift (float shiftx, float shifty, float shiftz)
  {
    initialized = false;
    shapenr++;
    shift.Set (shiftx, shifty, shiftz);
  }
  const csVector3& GetShift () const {return shift;}
  void SetRimVertices (int num)
  {
    initialized = false;
    verts_circle = num;
    if (verts_circle <= 1) verts_circle = 2;
    else if (verts_circle >= 60) verts_circle = 59;
  }
  int GetRimVertices () const {return verts_circle;}
  /// Set reversed mode (i.e. sphere visible from inside out).
  void SetReversed (bool r)
  {
    reversed = r;
    initialized = false;
    shapenr++;
  }
  /// Get reversed mode.
  bool IsReversed () const
  {
    return reversed;
  }
  /// Only show top half.
  void SetTopOnly (bool t)
  {
    toponly = t;
    initialized = false;
    shapenr++;
  }
  /// Only top half.
  bool IsTopOnly () const
  {
    return toponly;
  }
  /// Set lighting.
  void SetLighting (bool l) { do_lighting = l; }
  /// Is lighting enabled.
  bool IsLighting () const { return do_lighting; }
  /// Set the color to use. Will be added to the lighting values.
  void SetColor (const csColor& col) { color = col; }
  /// Get the color.
  csColor GetColor () const { return color; }
  /// Use cylindrical texture mapping.
  void SetCylindricalMapping (bool m)
  {
    cyl_mapping = m;
    initialized = false;
  }
  /// Test if cylindrical texture mapping is used.
  bool IsCylindricalMapping () const
  {
    return cyl_mapping;
  }

  /// apply vertical gradient
  void ApplyVertGradient(float horizon_height, float zenith_height,
    float** gradient);
  /// apply light spot
  void ApplyLightSpot(const csVector3& position, float size, float** gradient);

  //----------------------- iMeshObject implementation ------------------------
  SCF_DECLARE_IBASE;

  virtual iMeshObjectFactory* GetFactory () const { return factory; }
  virtual bool DrawTest (iRenderView* rview, iMovable* movable);
  virtual void UpdateLighting (iLight** lights, int num_lights,
      	iMovable* movable);
  virtual bool Draw (iRenderView* rview, iMovable* movable, csZBufMode mode);
  virtual void SetVisibleCallback (iMeshObjectDrawCallback* cb)
  {
    if (cb) cb->IncRef ();
    if (vis_cb) vis_cb->DecRef ();
    vis_cb = cb;
  }
  virtual iMeshObjectDrawCallback* GetVisibleCallback () const
  {
    return vis_cb;
  }
  virtual void GetObjectBoundingBox (csBox3& bbox, int type = CS_BBOX_NORMAL);
  virtual void GetRadius (csVector3& rad, csVector3& cent) 
	{ rad = max_radius; cent.Set(shift); }
  virtual void NextFrame (csTicks /*current_time*/) { }
  virtual bool WantToDie () const { return false; }
  virtual void HardTransform (const csReversibleTransform& t);
  virtual bool SupportsHardTransform () const { return true; }
  virtual bool HitBeamOutline (const csVector3& start, const csVector3& end, 
    csVector3& isect, float *pr);
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr);
  virtual long GetShapeNumber () const { return shapenr; }
  virtual uint32 GetLODFeatures () const { return current_features; }
  virtual void SetLODFeatures (uint32 mask, uint32 value)
  {
    mask &= ALL_FEATURES;
    current_features = (current_features & ~mask) | (value & mask);
  }
  virtual void SetLOD (float lod) { current_lod = lod; }
  virtual float GetLOD () const { return current_lod; }
  virtual int GetLODPolygonCount (float /*lod*/) const
  {
    return 0;	// @@@ Implement me please!
  }

  //------------------------- iBallState implementation ----------------
  class BallState : public iBallState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csBallMeshObject);
    virtual void SetRadius (float radiusx, float radiusy, float radiusz)
    {
      scfParent->SetRadius (radiusx, radiusy, radiusz);
    }
    virtual void GetRadius (float& radx, float& rady, float& radz) const
    {
      scfParent->GetRadius (radx, rady, radz);
    }
    virtual void SetShift (float shiftx, float shifty, float shiftz)
    {
      scfParent->SetShift (shiftx, shifty, shiftz);
    }
    virtual const csVector3& GetShift () const
    {
      return scfParent->GetShift ();
    }
    virtual void SetRimVertices (int num)
    {
      scfParent->SetRimVertices (num);
    }
    virtual int GetRimVertices () const
    {
      return scfParent->GetRimVertices ();
    }
    virtual void SetMaterialWrapper (iMaterialWrapper* material)
    {
      scfParent->material = material;
    }
    virtual iMaterialWrapper* GetMaterialWrapper () const
    { return scfParent->material; }
    virtual void SetMixMode (UInt mode) { scfParent->MixMode = mode; }
    virtual UInt GetMixMode () const { return scfParent->MixMode; }
    virtual void SetReversed (bool r) { scfParent->SetReversed (r); }
    virtual bool IsReversed () const { return scfParent->IsReversed (); }
    virtual void SetTopOnly (bool t) { scfParent->SetTopOnly (t); }
    virtual bool IsTopOnly () const { return scfParent->IsTopOnly (); }
    virtual void SetLighting (bool l) { scfParent->SetLighting (l); }
    virtual bool IsLighting () const { return scfParent->IsLighting (); }
    virtual void SetColor (const csColor& col) { scfParent->SetColor (col); }
    virtual csColor GetColor () const { return scfParent->GetColor (); }
    virtual void SetCylindricalMapping (bool m)
    {
      scfParent->SetCylindricalMapping (m);
    }
    virtual bool IsCylindricalMapping () const
    {
      return scfParent->IsCylindricalMapping ();
    }
    virtual void ApplyVertGradient(float horizon_height, float zenith_height,
      float** gradient)
    {
      scfParent->ApplyVertGradient(horizon_height, zenith_height, gradient);
    }
    virtual void ApplyLightSpot(const csVector3& position, float size,
      float** gradient)
    {
      scfParent->ApplyLightSpot(position, size, gradient);
    }
  } scfiBallState;
  friend class BallState;
};

/**
 * Factory for balls.
 */
class csBallMeshObjectFactory : public iMeshObjectFactory
{
public:
  iObjectRegistry* object_reg;

  /// Constructor.
  csBallMeshObjectFactory (iBase *pParent, iObjectRegistry* object_reg);

  /// Destructor.
  virtual ~csBallMeshObjectFactory ();

  //------------------------ iMeshObjectFactory implementation --------------
  SCF_DECLARE_IBASE;

  virtual iMeshObject* NewInstance ();
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
};

/**
 * Ball type. This is the plugin you have to use to create instances
 * of csBallMeshObjectFactory.
 */
class csBallMeshObjectType : public iMeshObjectType
{
public:
  iObjectRegistry* object_reg;

  SCF_DECLARE_IBASE;

  /// Constructor.
  csBallMeshObjectType (iBase*);
  /// Destructor.
  virtual ~csBallMeshObjectType ();
  /// Draw.
  virtual iMeshObjectFactory* NewFactory ();
  /// Get features.
  virtual uint32 GetFeatures () const
  {
    return ALL_FEATURES;
  }
  /// Initialize.
  bool Initialize (iObjectRegistry* object_reg)
  {
    csBallMeshObjectType::object_reg = object_reg;
    return true;
  }

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csBallMeshObjectType);
    virtual bool Initialize (iObjectRegistry* object_reg)
    {
      return scfParent->Initialize (object_reg);
    }
  } scfiComponent;
};

#endif // _BALL_H_
