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

#ifndef __CS_BALL_H__
#define __CS_BALL_H__

#include "csgeom/objmodel.h"
#include "csgeom/transfrm.h"
#include "csgeom/vector3.h"
#include "csgfx/shadervarcontext.h"
#include "cstool/rendermeshholder.h"
#include "csutil/cscolor.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/flags.h"
#include "csutil/refarr.h"
#include "iengine/lightmgr.h"
#include "igeom/polymesh.h"
#include "imesh/ball.h"
#include "imesh/object.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "ivideo/graph3d.h"
#include "ivideo/rendermesh.h"
#include "ivideo/rndbuf.h"

struct iMaterialWrapper;
struct iObjectRegistry;
struct iEngine;
class csBallMeshObjectFactory;
class csColor;

/**
 * Ball version of mesh object.
 */
class csBallMeshObject : public iMeshObject
{
private:
  iMeshObjectFactory* factory;
  iMeshWrapper* logparent;
  float radiusx, radiusy, radiusz;
  csVector3 max_radius;
  csVector3 shift;
  csRef<iMaterialWrapper> material;
  uint MixMode;
  iMeshObjectDrawCallback* vis_cb;
  bool reversed;
  bool toponly;
  bool cyl_mapping;
  bool do_lighting;
  bool generated_colors;
  csColor color;
  float current_lod;
  uint32 current_features;

  int verts_circle;

  csVector3* ball_vertices;
  csVector2* ball_texels;
  csColor* ball_colors;
  int num_ball_vertices;
  csVector3* top_normals;
  bool initialized;
  csBox3 object_bbox;
  csFlags flags;

  unsigned int* ball_indices;
  int ball_triangles;
  
  csRenderMeshHolder rmHolder;

  csRef<iRenderBuffer> vertex_buffer;
  csRef<iRenderBuffer> texel_buffer;
  csRef<iRenderBuffer> normal_buffer;
  csRef<iRenderBuffer> color_buffer;
  csRef<iRenderBuffer> index_buffer;

  csRef<iGraphics3D> g3d;

  bool ball_vertices_dirty_flag;
  bool ball_texels_dirty_flag;
  bool ball_normals_dirty_flag;
  bool ball_colors_dirty_flag;
  bool ball_triangle_dirty_flag;
  csRef<csRenderBufferHolder> bufferHolder;
  
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

  csMeshedPolygon* polygons;

  /**
   * Setup this object. This function will check if setup is needed.
   */
  void SetupObject ();


  /// Generate a mesh with a sphere.
  void GenerateSphere (int num_circle);

public:
  /// Constructor.
  csBallMeshObject (iMeshObjectFactory* factory);

  /// Destructor.
  virtual ~csBallMeshObject ();

  void UpdateLighting (const csArray<iLight*>& lights, iMovable* movable);

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

  /// Get mixmode.
  uint GetMixMode () const { return MixMode; }
  void SetRadius (float radiusx, float radiusy, float radiusz);
  void GetRadius (float& radx, float& rady, float& radz) const
  { radx=radiusx; rady=radiusy; radz=radiusz; }
  void SetShift (float shiftx, float shifty, float shiftz)
  {
    initialized = false;
    shift.Set (shiftx, shifty, shiftz);
    scfiObjectModel.ShapeChanged ();
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
    scfiObjectModel.ShapeChanged ();
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
    scfiObjectModel.ShapeChanged ();
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
  /// paint a sky
  void PaintSky(float time, float **dayvert, float **nightvert,
    float **topsun, float **sunset);
  void GetObjectBoundingBox (csBox3& bbox);
  void SetObjectBoundingBox (const csBox3& bbox);

  /**
   * Calculate polygons for iPolygonMesh.
   */
  csMeshedPolygon* GetPolygons ();

  int GetVertexCount () { SetupObject(); return num_ball_vertices; }
  csVector3* GetVertices () { SetupObject (); return ball_vertices; }
  virtual csRenderMesh **GetRenderMeshes (int &num, iRenderView* rview, 
    iMovable* movable, uint32 frustum_mask);


  int GetTriangleCount () { SetupObject(); return ball_triangles; }
  csTriangle* GetTriangles ()
  {
    SetupObject ();
    return (csTriangle*)ball_indices;
  }

  void UpdateBuffers();


  //----------------------- iMeshObject implementation ------------------------
  SCF_DECLARE_IBASE;

  virtual iMeshObjectFactory* GetFactory () const { return factory; }
  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> Clone () { return 0; }
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
  virtual void NextFrame(csTicks /*current_time*/, const csVector3& /*pos*/) {}
  virtual void HardTransform (const csReversibleTransform& t);
  virtual bool SupportsHardTransform () const { return true; }
  virtual bool HitBeamOutline (const csVector3& start, const csVector3& end,
    csVector3& isect, float *pr);
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr, int* polygon_idx = 0);
  virtual void SetMeshWrapper (iMeshWrapper* lp) { logparent = lp; }
  virtual iMeshWrapper* GetMeshWrapper () const { return logparent; }

  //------------------------- iObjectModel implementation ----------------
  class ObjectModel : public csObjectModel
  {
  public:
    SCF_DECLARE_EMBEDDED_IBASE (csBallMeshObject);
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
      rad = scfParent->max_radius;
      cent.Set (scfParent->shift);
    }
  } scfiObjectModel;
  friend class ObjectModel;

  virtual iObjectModel* GetObjectModel () { return &scfiObjectModel; }
  virtual bool SetColor (const csColor& col) { color = col; return true; }
  virtual bool GetColor (csColor& col) const { col = color; return true; }
  virtual bool SetMaterialWrapper (iMaterialWrapper* mat)
  {
    material = mat;
    return true;
  }
  virtual iMaterialWrapper* GetMaterialWrapper () const { return material; }
  virtual void InvalidateMaterialHandles () { }
  /**
   * see imesh/object.h for specification. The default implementation
   * does nothing.
   */
  virtual void PositionChild (iMeshObject* child,csTicks current_time) { }


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
    virtual void SetMixMode (uint mode) { scfParent->MixMode = mode; }
    virtual uint GetMixMode () const { return scfParent->MixMode; }
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
    virtual void PaintSky(float time, float **dayvert, float **nightvert,
      float **topsun, float **sunset)
    {
      scfParent->PaintSky(time, dayvert, nightvert, topsun, sunset);
    }
  } scfiBallState;
  friend class BallState;

  //------------------ iPolygonMesh interface implementation ----------------//
  struct PolyMesh : public iPolygonMesh
  {
  private:
    csBallMeshObject* ball;
    csFlags flags;
  public:
    SCF_DECLARE_IBASE;

    void SetBall (csBallMeshObject* Ball)
    {
      ball = Ball;
      flags.SetAll (CS_POLYMESH_CLOSED | CS_POLYMESH_CONVEX |
      	CS_POLYMESH_TRIANGLEMESH);
    }
    void Cleanup ();

    virtual int GetVertexCount ();
    virtual csVector3* GetVertices ();
    virtual int GetPolygonCount ();
    virtual csMeshedPolygon* GetPolygons ();
    virtual int GetTriangleCount ()
    {
      return ball->GetTriangleCount ();
    }
    virtual csTriangle* GetTriangles ()
    {
      return (csTriangle*)(ball->ball_indices);
    }
    virtual void Lock () { }
    virtual void Unlock () { }

    virtual csFlags& GetFlags () { return flags;  }
    virtual uint32 GetChangeNumber() const { return 0; }

    PolyMesh ()
    { SCF_CONSTRUCT_IBASE (0); }
    virtual ~PolyMesh ()
    { SCF_DESTRUCT_IBASE (); }
  } scfiPolygonMesh;
  friend struct PolyMesh;
};

/**
 * Factory for balls.
 */
class csBallMeshObjectFactory : public iMeshObjectFactory
{
public:
  iObjectRegistry* object_reg;
  iMeshFactoryWrapper* logparent;
  iMeshObjectType* ball_type;
  iEngine* engine;
  csRef<iLightManager> light_mgr;
  csFlags flags;

  /// Constructor.
  csBallMeshObjectFactory (iMeshObjectType* pParent,
  	iObjectRegistry* object_reg);

  /// Destructor.
  virtual ~csBallMeshObjectFactory ();

  //------------------------ iMeshObjectFactory implementation --------------
  SCF_DECLARE_IBASE;

  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> NewInstance ();
  virtual csPtr<iMeshObjectFactory> Clone () { return 0; }
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
  virtual void SetMeshFactoryWrapper (iMeshFactoryWrapper* lp)
  { logparent = lp; }
  virtual iMeshFactoryWrapper* GetMeshFactoryWrapper () const
  { return logparent; }
  virtual iMeshObjectType* GetMeshObjectType () const { return ball_type; }
  virtual iObjectModel* GetObjectModel () { return 0; }
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
  virtual csPtr<iMeshObjectFactory> NewFactory ();
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

#endif // __CS_BALL_H__
