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

#ifndef _SPR2D_H_
#define _SPR2D_H_

#include "csgeom/vector3.h"
#include "csgeom/transfrm.h"
#include "csutil/cscolor.h"
#include "csutil/csvector.h"
#include "imesh/object.h"
#include "imesh/sprite2d.h"
#include "imesh/particle.h"
#include "ivideo/graph3d.h"
#include "iutil/config.h"
#include "isys/plugin.h"
#include "spr2duv.h"

#define ALL_FEATURES (CS_OBJECT_FEATURE_LIGHTING|CS_OBJECT_FEATURE_ANIMATION)

struct iMaterialWrapper;
struct iSprite2DUVAnimation;
class csSprite2DMeshObjectFactory;
class csBox2;

/**
 * Sprite 2D version of mesh object.
 */
class csSprite2DMeshObject : public iMeshObject
{
 protected:
  class uvAnimationControl
  {
  public:
    uvAnimationControl (){animate=false;}
    bool animate, loop, halted;
    csTicks last_time;
    int frameindex, framecount, style, counter;
    iSprite2DUVAnimation *ani;
    iSprite2DUVAnimationFrame *frame;
    void Advance (csTicks current_time);
    const csVector2 *GetVertices (int &num);
  };

  uvAnimationControl uvani;

 private:
  iMeshObjectFactory* ifactory;
  csSprite2DMeshObjectFactory* factory;

  iMaterialWrapper* material;
  UInt MixMode;
  bool initialized;
  iMeshObjectDrawCallback* vis_cb;
  csVector3 radius;
  long shapenr;
  float current_lod;
  uint32 current_features;
  csBox2 bbox_2d;
  csMatrix3 o2t; // Cached LookAt() matrix
  csVector3 cached_start;

  /**
   * Array of 3D vertices.
   */
  csColoredVertices vertices;

  /**
   * If false then we don't do lighting but instead use
   * the given colors.
   */
  bool lighting;

  /// Temporary camera space vector between DrawTest() and Draw().
  csVector3 cam;
  /// Polygon.
  G3DPolygonDPFX g3dpolyfx;

  /**
   * Setup this object. This function will check if setup is needed.
   */
  void SetupObject ();

  /// Update lighting given a position.
  void UpdateLighting (iLight** lights, int num_lights, const csVector3& pos);

  /// Check the start vector and recalculate the LookAt matrix if changed.
  void CheckBeam(const csVector3& start, const csVector3& plane, float dist_squared);

public:
  /// Constructor.
  csSprite2DMeshObject (csSprite2DMeshObjectFactory* factory);

  /// Destructor.
  virtual ~csSprite2DMeshObject ();

  /// Get the vertex array.
  csColoredVertices& GetVertices () { return vertices; }
  /** 
   * Set vertices to form a regular n-polygon around (0,0),
   * optionally also set u,v to corresponding coordinates in a texture.
   * Large n approximates a circle with radius 1. n must be > 2. 
   */
  void CreateRegularVertices (int n, bool setuv);

  ///------------------------ iMeshObject implementation ------------------------
  SCF_DECLARE_IBASE;

  virtual iMeshObjectFactory* GetFactory () const { return ifactory; }
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
	{ rad =  radius; cent.Set(0,0,0); }
  virtual void NextFrame (csTicks current_time);
  virtual bool WantToDie () const { return false; }
  virtual void HardTransform (const csReversibleTransform& t);
  virtual bool SupportsHardTransform () const { return false; }
  virtual int HitBeamBBox (const csVector3&, const csVector3&,
        csVector3&, float*);
  virtual bool HitBeamOutline (const csVector3&, const csVector3&,
        csVector3&, float*);
  /// 2D sprites have no depth, so this is equivalent to HitBeamOutline.
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr) 
  { return HitBeamOutline(start, end, isect, pr); }
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
    return 1;
  }

  //------------------------- iSprite2DState implementation ----------------
  class Sprite2DState : public iSprite2DState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSprite2DMeshObject);
    virtual void SetLighting (bool l) { scfParent->initialized = false; scfParent->lighting = l; }
    virtual bool HasLighting () const { return scfParent->lighting; }
    virtual void SetMaterialWrapper (iMaterialWrapper* material)
    {
      scfParent->material = material;
    }
    virtual iMaterialWrapper* GetMaterialWrapper () const { return scfParent->material; }
    virtual void SetMixMode (UInt mode) { scfParent->MixMode = mode; }
    virtual UInt GetMixMode () const { return scfParent->MixMode; }
    virtual csColoredVertices& GetVertices ()
    {
      return scfParent->GetVertices ();
    }
    virtual void CreateRegularVertices (int n, bool setuv)
    {
      scfParent->CreateRegularVertices (n, setuv);
    }

    virtual void SetUVAnimation (const char *name, int style, bool loop);
    virtual void StopUVAnimation (int idx);
    virtual void PlayUVAnimation (int idx, int style, bool loop);

    virtual int GetUVAnimationCount () const;
    virtual iSprite2DUVAnimation *CreateUVAnimation ();
    virtual void RemoveUVAnimation (iSprite2DUVAnimation *anim);
    virtual iSprite2DUVAnimation *GetUVAnimation (const char *name);
    virtual iSprite2DUVAnimation *GetUVAnimation (int idx);

  } scfiSprite2DState;
  friend class Sprite2DState;

  //------------------------- iParticle implementation ----------------
  class Particle : public iParticle
  {
  private:
    csVector3 part_pos;

  public:
    SCF_DECLARE_EMBEDDED_IBASE (csSprite2DMeshObject);
    virtual void SetPosition (const csVector3& pos) { part_pos = pos; }
    virtual void MovePosition (const csVector3& move) { part_pos += move; }
    virtual void SetColor (const csColor& col);
    virtual void AddColor (const csColor& col);
    virtual void ScaleBy (float factor);
    virtual void SetMixMode (UInt mode)
    {
      scfParent->MixMode = mode;
    }
    virtual void Rotate (float angle);
    virtual void Draw (iRenderView* rview,
    	const csReversibleTransform& transform, csZBufMode mode);
    virtual void UpdateLighting (iLight** lights, int num_lights,
	const csReversibleTransform& transform);
  } scfiParticle;
  friend class Particle;
};

/**
 * Factory for 2D sprites. This factory also implements iSprite2DFactoryState.
 */
class csSprite2DMeshObjectFactory : public iMeshObjectFactory
{
 protected:

  class animVector : public csVector
  {
  public:
    animVector () : csVector (8, 16){}
    virtual int CompareKey (csSome Item1, csConstSome Item2, int Mode) const
    { 
      (void)Mode;
      csSprite2DUVAnimation *f1 = (csSprite2DUVAnimation *)Item1;
      const char *f2 = (const char *)Item2;
      return strcmp (f1->GetName (), f2);
    }
  };

  animVector vAnims;

 private:
  iMaterialWrapper* material;
  UInt MixMode;
  /**
   * If false then we don't do lighting but instead use
   * the given colors.
   */
  bool lighting;
  
 public:
  /// Constructor.
  csSprite2DMeshObjectFactory (iBase *pParent);

  /// Destructor.
  virtual ~csSprite2DMeshObjectFactory ();

  /// Has this sprite lighting?
  bool HasLighting () const { return lighting; }
  /// Get the material for this 2D sprite.
  iMaterialWrapper* GetMaterialWrapper () const { return material; }
  /// Get mixmode.
  UInt GetMixMode () const { return MixMode; }
  
  int GetUVAnimationCount () const {return vAnims.Length ();}
  iSprite2DUVAnimation *CreateUVAnimation ()
  { 
    csSprite2DUVAnimation *p = new csSprite2DUVAnimation (this);
    vAnims.Push (p);
    return (iSprite2DUVAnimation *)p;
  }
  void RemoveUVAnimation (iSprite2DUVAnimation *anim)
  {
    int idx = vAnims.Find ((csSome)anim);
    if (idx != -1)
    {
      anim->DecRef ();
      vAnims.Delete (idx);
    }
  }
  iSprite2DUVAnimation *GetUVAnimation (const char *name)
  {
    int idx = vAnims.FindKey ((csSome)name);
    return (iSprite2DUVAnimation *)(idx != -1 ? vAnims.Get (idx) : NULL);
  }
  iSprite2DUVAnimation *GetUVAnimation (int idx)
  {
    return (iSprite2DUVAnimation *)vAnims.Get (idx);
  }

  //------------------------ iMeshObjectFactory implementation --------------
  SCF_DECLARE_IBASE;

  virtual iMeshObject* NewInstance ();
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }

  //------------------------- iSprite2DFactoryState implementation ----------------
  class Sprite2DFactoryState : public iSprite2DFactoryState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSprite2DMeshObjectFactory);
    virtual void SetLighting (bool l) { scfParent->lighting = l; }
    virtual bool HasLighting () const { return scfParent->HasLighting (); }
    virtual void SetMaterialWrapper (iMaterialWrapper* material)
    {
      scfParent->material = material;
    }
    virtual iMaterialWrapper* GetMaterialWrapper () const { return scfParent->material; }
    virtual void SetMixMode (UInt mode) { scfParent->MixMode = mode; }
    virtual UInt GetMixMode () const { return scfParent->MixMode; }

    virtual int GetUVAnimationCount () const {return scfParent->GetUVAnimationCount();}
    virtual iSprite2DUVAnimation *CreateUVAnimation ()
    { return scfParent->CreateUVAnimation (); }
    virtual void RemoveUVAnimation (iSprite2DUVAnimation *anim)
    { scfParent->RemoveUVAnimation(anim); }
    virtual iSprite2DUVAnimation *GetUVAnimation (const char *name)
    { return scfParent->GetUVAnimation (name); }
    virtual iSprite2DUVAnimation *GetUVAnimation (int idx)
    { return scfParent->GetUVAnimation (idx); }

  } scfiSprite2DFactoryState;
  friend class Sprite2DFactoryState;
};

/**
 * Sprite 2D type. This is the plugin you have to use to create instances
 * of csSprite2DMeshObjectFactory.
 */
class csSprite2DMeshObjectType : public iMeshObjectType
{
public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csSprite2DMeshObjectType (iBase*);
  /// Destructor.
  virtual ~csSprite2DMeshObjectType ();
  /// New Factory.
  virtual iMeshObjectFactory* NewFactory ();
  /// Get features.
  virtual uint32 GetFeatures () const
  {
    return ALL_FEATURES;
  }

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSprite2DMeshObjectType);
    virtual bool Initialize (iObjectRegistry*) { return true; }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
};

#endif // _SPR2D_H_
