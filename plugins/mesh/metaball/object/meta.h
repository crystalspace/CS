/*
    Metaballs Demo
    Copyright (C) 1999 by Denis Dmitriev
    Pluggified by Samuel Humphreys

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

#ifndef __META_H__
#define __META_H__

#include <stdarg.h>
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/tesselat.h"
#include "imesh/object.h"
#include "imesh/metaball.h"
#include "isys/plugin.h"

#define ALL_FEATURES (CS_OBJECT_FEATURE_LIGHTING|CS_OBJECT_FEATURE_ANIMATION)

class csMaterialHandle;
struct G3DTriangleMesh;
struct iSystem;
struct iGraphics3D;
struct iGraphics2D;
struct iMaterialWrapper;
struct iMeshObject;

struct MetaBall
{
  csVector3 center;
};

class csMetaBall : public iMeshObject
{

//------------- MetaBall Data
  int num_meta_balls;
  int max_vertices;
  int vertices_tesselated;
  EnvMappingModes env_mapping;
  float env_map_mult;
  float alpha;
  MetaParameters mp;
  iSystem *Sys;
  iMaterialWrapper *th;
  G3DTriangleMesh mesh;
  MetaBall *meta_balls;
  char frame;
//------------- MeshObject Data
  iMeshObjectFactory* factory;
  csBox3 camera_bbox;
  csBox3 object_bbox;
  iMeshObjectDrawCallback* vis_cb;
  bool do_lighting;
  bool initialize;
  long shape_num;
  long cur_camera_num;
  long cur_movable_num;
  UInt MixMode;
  csVector3 rad;
  float current_lod;
  uint32 current_features;

public:
  SCF_DECLARE_IBASE;

  csMetaBall (iMeshObjectFactory *fact);
  virtual ~csMetaBall ();
  virtual bool Initialize ();

  virtual void SetMaterial (iMaterialWrapper *tex)
  { th = tex; }

  virtual void SetMetaBallCount (int number);
  virtual int GetMetaBallCount ()
  { return num_meta_balls; }

  virtual void SetQualityEnvironmentMapping (bool toggle);
  virtual bool GetQualityEnvironmentMapping ()
  { return env_mapping; }

  virtual void SetEnvironmentMappingFactor (float env_mult);
  virtual float GetEnvironmentMappingFactor ()
  { return env_map_mult; }

  virtual MetaParameters *GetParameters ()
  { return &mp; }

  virtual int ReportTriangleCount ()
  { return mesh.num_triangles; }

// Where the real work gets done....
  void CalculateMetaBalls (void);
  void CalculateBlob (int x, int y, int z);
  void FillCell (int x, int y, int z, csTesselator::GridCell &c);
  float map (float x);
  float potential (const csVector3 &p);
  int check_cell_assume_inside (const csTesselator::GridCell &c);
  void InitTables (void);
///-------------------- iMeshObject implementation --------------

  virtual iMeshObjectFactory * GetFactory() const { return factory; }
  virtual bool DrawTest ( iRenderView* rview, iMovable* movable );
  virtual void UpdateLighting ( iLight** lights, int num_lights,
	  iMovable* movable );
  virtual bool Draw ( iRenderView* rview, iMovable* movable,
	  csZBufMode mode );
  virtual void SetVisibleCallback( iMeshObjectDrawCallback* cb )
  {
    if (cb) cb->IncRef ();
    if (vis_cb) vis_cb->DecRef ();
    vis_cb = cb;
  }
  virtual iMeshObjectDrawCallback* GetVisibleCallback() const
  { return vis_cb; }
  float GetScreenBoundingBox( long cam_num, long mov_num,
    float fov, float sx, float sy, const csReversibleTransform& trans,
    csBox2& sbox, csBox3& cbox );
  void GetTransformedBoundingBox( long cam_num, long move_num,
    const csReversibleTransform& trans, csBox3& cbox);
  void CreateBoundingBox();
  virtual void GetObjectBoundingBox ( csBox3& bbox, int type = CS_BBOX_NORMAL );
  virtual void NextFrame(csTime /* current_time */ );
  virtual bool WantToDie() const { return false; }
  virtual void HardTransform( const csReversibleTransform &t );
  virtual bool SupportsHardTransform() const { return false; }
  virtual bool HitBeamObject( const csVector3&, const csVector3&,
    csVector3 &, float *);
  virtual long GetShapeNumber() const { return shape_num; }
  virtual csVector3 GetRadius() { return rad; }
  virtual UInt GetMixMode() { return MixMode; }
  virtual void SetMixMode(UInt mode) { MixMode = mode; }
  virtual bool IsLighting() { return do_lighting; }
  virtual void SetLighting( bool set ) { do_lighting = set; }
  virtual iMaterialWrapper *GetMaterial() { return th; }
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

///-------------------- Meta Ball state implementation
  class MetaBallState : public iMetaBallState
  {
    SCF_DECLARE_EMBEDDED_IBASE(csMetaBall);

    virtual int GetMetaBallCount()
      { return scfParent->GetMetaBallCount(); }
    virtual void SetMetaBallCount( int num )
      { scfParent->SetMetaBallCount( num ); }
    virtual void SetQualityEnvironmentMapping ( bool tog )
      { scfParent->SetQualityEnvironmentMapping( tog ); }
    virtual bool GetQualityEnvironmentMapping()
      { return scfParent->GetQualityEnvironmentMapping(); }
    virtual float GetEnvironmentMappingFactor()
      { return scfParent->GetEnvironmentMappingFactor(); }
    virtual void SetEnvironmentMappingFactor(float env )
      { scfParent->SetEnvironmentMappingFactor( env ); }
    virtual MetaParameters *GetParameters()
      { return scfParent->GetParameters(); }
    virtual void SetMaterial ( iMaterialWrapper *mat )
      { scfParent->SetMaterial(mat); }
    virtual int ReportTriangleCount()
      { return scfParent->ReportTriangleCount(); }
    virtual UInt GetMixMode ()
      { return scfParent->GetMixMode(); }
    virtual void SetMixMode( UInt mode )
      { scfParent->SetMixMode( mode ); }
    virtual bool IsLighting()
      { return scfParent->IsLighting(); }
    virtual void SetLighting(bool set)
      { scfParent->SetLighting(set); }
    virtual iMaterialWrapper* GetMaterial()
      { return scfParent->GetMaterial(); }
  } scfiMetaBallState;
  friend class MetaBallState;
};

class csMetaBallFactory : public iMeshObjectFactory
{
public:
  csMetaBallFactory( iBase *parent );
  virtual ~csMetaBallFactory();
  SCF_DECLARE_IBASE;
  virtual iMeshObject* NewInstance();
  virtual void HardTransform ( const csReversibleTransform & ) {};
  virtual bool SupportsHardTransform() const { return false; }
};


/*
 *	MetaBall type. Use this plugin to create instances of
 *	csMetaBallMeshObjectFactory.
 */

class csMetaBallType : public iMeshObjectType
{
public:
  SCF_DECLARE_IBASE;

  csMetaBallType ( iBase * );
  virtual ~csMetaBallType();
  virtual iMeshObjectFactory* NewFactory();
  virtual uint32 GetFeatures () const
  {
    return ALL_FEATURES;
  }

  struct eiPlugIn : public iPlugIn
  {
    SCF_DECLARE_EMBEDDED_IBASE(csMetaBallType);
    virtual bool Initialize (iSystem*) { return true; }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugIn;
};

#endif // __META_H__
