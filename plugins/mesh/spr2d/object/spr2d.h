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

#ifndef __CS_SPR2D_H__
#define __CS_SPR2D_H__

#include "csgeom/vector3.h"
#include "csgeom/transfrm.h"
#include "csgeom/objmodel.h"
#include "csgfx/shadervar.h"
#include "csgfx/shadervarcontext.h"

#include "csutil/array.h"
#include "csutil/cscolor.h"
#include "csutil/flags.h"
#include "csutil/refarr.h"
#include "csutil/leakguard.h"
#include "csutil/weakref.h"
#include "cstool/rendermeshholder.h"

#include "imesh/object.h"
#include "imesh/sprite2d.h"
#include "imesh/particle.h"
#include "ivideo/graph3d.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/pluginconfig.h"
#include "iengine/lightmgr.h"
#include "spr2duv.h"

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
    bool loop, halted;
    csTicks last_time;
    int frameindex, framecount, style, counter;
    iSprite2DUVAnimation *ani;
    iSprite2DUVAnimationFrame *frame;
    void Advance (csTicks current_time);
    const csVector2 *GetVertices (int &num);
  };

  uvAnimationControl* uvani;

  class eiRenderBufferAccessor : public iRenderBufferAccessor
  {
  private:
    csWeakRef<csSprite2DMeshObject> parent;
  public:
    CS_LEAKGUARD_DECLARE (eiRenderBufferAccessor);
    SCF_DECLARE_IBASE;

    eiRenderBufferAccessor (csSprite2DMeshObject* parent)
    {
      SCF_CONSTRUCT_IBASE(0);
      eiRenderBufferAccessor::parent = parent;
    }
    virtual ~eiRenderBufferAccessor() 
    {
      SCF_DESTRUCT_IBASE();
    }

    void PreGetBuffer (csRenderBufferHolder* holder, csRenderBufferName buffer)
    {
      if (parent) parent->PreGetBuffer (holder, buffer);
    }
  };
  friend class eiRenderBufferAccessor;

  void PreGetBuffer (csRenderBufferHolder* holder, csRenderBufferName buffer);

  csRef<csRenderBufferHolder> bufferHolder;
  csRenderMeshHolder rmHolder;
  csRef<iRenderBuffer> vertex_buffer;
  bool vertices_dirty;
  csRef<iRenderBuffer> texel_buffer;
  bool texels_dirty;
  csRef<iRenderBuffer> color_buffer;
  bool colors_dirty;
  csRef<iRenderBuffer> index_buffer;
  size_t indicesSize;
  csRef<csShaderVariableContext> svcontext;

private:
  csRef<iMeshObjectFactory> ifactory;
  iMeshWrapper* logparent;
  csSprite2DMeshObjectFactory* factory;

  csRef<iMaterialWrapper> material;
  uint MixMode;
  bool initialized;
  iMeshObjectDrawCallback* vis_cb;
  csVector3 radius;
  float current_lod;
  uint32 current_features;
  csBox2 bbox_2d;
  csFlags flags;

  /**
   * Array of 3D vertices.
   */
  csColoredVertices vertices;

  /**
   * If false then we don't do lighting but instead use
   * the given colors.
   */
  bool lighting;

  /**
   * Setup this object. This function will check if setup is needed.
   */
  void SetupObject ();

  /// Update lighting given a position.
  void UpdateLighting (const csArray<iLight*>& lights, const csVector3& pos);
  void UpdateLighting (const csArray<iLight*>& lights,
      	iMovable* movable, csVector3 offset);

  /// Check the start vector and recalculate the LookAt matrix if changed.
  void CheckBeam (const csVector3& start, const csVector3& plane,
  	float dist_squared, csMatrix3& o2t);

public:
  /// Constructor.
  csSprite2DMeshObject (csSprite2DMeshObjectFactory* factory);

  CS_LEAKGUARD_DECLARE (csSprite2DMeshObject);

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

  void GetObjectBoundingBox (csBox3& bbox);
  void SetObjectBoundingBox (const csBox3& bbox);
  void GetRadius (csVector3& rad, csVector3& cent)
  { rad = radius; cent.Set (0,0,0); }

  ///---------------------- iMeshObject implementation ------------------------
  SCF_DECLARE_IBASE;

  virtual iMeshObjectFactory* GetFactory () const { return ifactory; }
  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> Clone () { return 0; }
  csRenderMesh **GetRenderMeshes (int &n, iRenderView* rview, 
    iMovable* movable, uint32 frustum_mask, csVector3 offset);
  virtual csRenderMesh **GetRenderMeshes (int &n, iRenderView* rview, 
    iMovable* movable, uint32 frustum_mask)
  {
    return GetRenderMeshes (n, rview, movable, frustum_mask, csVector3 (0.0f));
  }
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
  virtual void NextFrame (csTicks current_time, const csVector3& /*pos*/);
  virtual void HardTransform (const csReversibleTransform& t);
  virtual bool SupportsHardTransform () const { return false; }
  virtual bool HitBeamOutline (const csVector3&, const csVector3&,
        csVector3&, float*);
  /// 2D sprites have no depth, so this is equivalent to HitBeamOutline.
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr, int* polygon_idx = 0)
  {
    if (polygon_idx) *polygon_idx = -1;
    return HitBeamOutline(start, end, isect, pr);
  }
  virtual void SetMeshWrapper (iMeshWrapper* lp) { logparent = lp; }
  virtual iMeshWrapper* GetMeshWrapper () const { return logparent; }

  //------------------------- iObjectModel implementation ----------------
  class ObjectModel : public csObjectModel
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSprite2DMeshObject);
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
  } scfiObjectModel;
  friend class ObjectModel;

  virtual iObjectModel* GetObjectModel () { return &scfiObjectModel; }
  virtual bool SetColor (const csColor&) { return false; }
  virtual bool GetColor (csColor&) const { return false; }
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

  //------------------------- iSprite2DState implementation ----------------
  class Sprite2DState : public iSprite2DState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSprite2DMeshObject);
    virtual void SetLighting (bool l)
    {
      scfParent->initialized = false;
      scfParent->lighting = l;
    }
    virtual bool HasLighting () const { return scfParent->lighting; }
    virtual void SetMaterialWrapper (iMaterialWrapper* material)
    {
      scfParent->material = material;
    }
    virtual iMaterialWrapper* GetMaterialWrapper () const
    { return scfParent->material; }
    virtual void SetMixMode (uint mode) { scfParent->MixMode = mode; }
    virtual uint GetMixMode () const { return scfParent->MixMode; }
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
    virtual iSprite2DUVAnimation *GetUVAnimation (const char *name) const;
    virtual iSprite2DUVAnimation *GetUVAnimation (int idx) const;
    virtual iSprite2DUVAnimation *GetUVAnimation (int idx, int &style, bool &loop) const;

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
    virtual const csVector3& GetPosition () const { return part_pos; }
    virtual void MovePosition (const csVector3& move) { part_pos += move; }
    virtual void SetColor (const csColor& col);
    virtual void AddColor (const csColor& col);
    virtual void ScaleBy (float factor);
    virtual void SetMixMode (uint mode)
    {
      scfParent->MixMode = mode;
    }
    virtual void Rotate (float angle);
    virtual csRenderMesh** GetRenderMeshes (int& n, iRenderView* rview, 
      iMovable* movable, uint32 frustum_mask);
    virtual void UpdateLighting (const csArray<iLight*>& lights,
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

  class animVector : public csArray<csSprite2DUVAnimation*>
  {
  public:
    animVector () : csArray<csSprite2DUVAnimation*> (8, 16){}
    static int CompareKey (csSprite2DUVAnimation* const& item,
			   char const* const& key)
    {
      return strcmp (item->GetName (), key);
    }
    csArrayCmp<csSprite2DUVAnimation*,char const*>KeyCmp(char const* key)
    {
      return csArrayCmp<csSprite2DUVAnimation*,char const*>(key, CompareKey);
    }
  };

  animVector vAnims;

private:
  csRef<iMaterialWrapper> material;
  iMeshFactoryWrapper* logparent;
  iMeshObjectType* spr2d_type;
  uint MixMode;
  /**
   * If false then we don't do lighting but instead use
   * the given colors.
   */
  bool lighting;
  csFlags flags;

public:
  CS_LEAKGUARD_DECLARE (csSprite2DMeshObjectFactory);

  csRef<iLightManager> light_mgr;
  iObjectRegistry* object_reg;
  csWeakRef<iGraphics3D> g3d;
public:
  /// Constructor.
  csSprite2DMeshObjectFactory (iMeshObjectType* pParent,
  	iObjectRegistry* object_reg);

  /// Destructor.
  virtual ~csSprite2DMeshObjectFactory ();

  /// Has this sprite lighting?
  bool HasLighting () const { return lighting; }
  /// Get the material for this 2D sprite.
  iMaterialWrapper* GetMaterialWrapper () const { return material; }
  /// Get mixmode.
  uint GetMixMode () const { return MixMode; }

  int GetUVAnimationCount () const {return (int)vAnims.Length ();}
  iSprite2DUVAnimation *CreateUVAnimation ()
  {
    csSprite2DUVAnimation *p = new csSprite2DUVAnimation (0);
    vAnims.Push (p);
    return (iSprite2DUVAnimation *)p;
  }
  void RemoveUVAnimation (iSprite2DUVAnimation *anim)
  {
    int idx = (int)vAnims.Find ((csSprite2DUVAnimation*)anim);
    if (idx != -1)
    {
      anim->DecRef ();
      vAnims.DeleteIndex (idx);
    }
  }
  iSprite2DUVAnimation *GetUVAnimation (const char *name)
  {
    int idx = (int)vAnims.FindKey (vAnims.KeyCmp(name));
    return (iSprite2DUVAnimation *)(idx != -1 ? vAnims.Get (idx) : 0);
  }
  iSprite2DUVAnimation *GetUVAnimation (int idx)
  {
    return (iSprite2DUVAnimation *)vAnims.Get (idx);
  }

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
  virtual iMeshObjectType* GetMeshObjectType () const { return spr2d_type; }
  virtual iObjectModel* GetObjectModel () { return 0; }

  //---------------------- iSprite2DFactoryState implementation ----------------
  class Sprite2DFactoryState : public iSprite2DFactoryState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSprite2DMeshObjectFactory);
    virtual void SetLighting (bool l) { scfParent->lighting = l; }
    virtual bool HasLighting () const { return scfParent->HasLighting (); }
    virtual void SetMaterialWrapper (iMaterialWrapper* material)
    {
      scfParent->material = material;
    }
    virtual iMaterialWrapper* GetMaterialWrapper () const
    { return scfParent->material; }
    virtual void SetMixMode (uint mode) { scfParent->MixMode = mode; }
    virtual uint GetMixMode () const { return scfParent->MixMode; }

    virtual int GetUVAnimationCount () const
    { return scfParent->GetUVAnimationCount(); }
    virtual iSprite2DUVAnimation *CreateUVAnimation ()
    { return scfParent->CreateUVAnimation (); }
    virtual void RemoveUVAnimation (iSprite2DUVAnimation *anim)
    { scfParent->RemoveUVAnimation(anim); }
    virtual iSprite2DUVAnimation *GetUVAnimation (const char *name) const
    { return scfParent->GetUVAnimation (name); }
    virtual iSprite2DUVAnimation *GetUVAnimation (int idx) const
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
  iObjectRegistry* object_reg;
public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csSprite2DMeshObjectType (iBase*);
  /// Destructor.
  virtual ~csSprite2DMeshObjectType ();
  /// New Factory.
  virtual csPtr<iMeshObjectFactory> NewFactory ();

  bool Initialize (iObjectRegistry* object_reg);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSprite2DMeshObjectType);
    virtual bool Initialize (iObjectRegistry* object_reg)
    {
      return scfParent->Initialize (object_reg);
    }
  } scfiComponent;
};

#endif // __CS_SPR2D_H__
