/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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

#ifndef __CS_SPR3D_H__
#define __CS_SPR3D_H__

#include "csutil/cscolor.h"
#include "csutil/typedvec.h"
#include "csutil/rng.h"
#include "csgeom/math3d.h"
#include "csgeom/math2d.h"
#include "csgeom/poly2d.h"
#include "csgeom/poly3d.h"
#include "csgeom/box.h"
#include "plugins/mesh/spr3d/object/sprtri.h"
#include "plugins/mesh/spr3d/object/skel3d.h"
#include "ivideo/graph3d.h"
#include "ivaria/polymesh.h"
#include "imesh/sprite3d.h"
#include "ivideo/material.h"
#include "iengine/material.h"
#include "imesh/object.h"
#include "iutil/config.h"
#include "isys/plugin.h"

#define ALL_FEATURES (CS_OBJECT_FEATURE_LIGHTING|CS_OBJECT_FEATURE_ANIMATION)

struct iSystem;

/**
 * A frame for 3D sprite animation.
 */
class csSpriteFrame : public iSpriteFrame
{
private:
  int animation_index;
  int texturing_index;
  char* name;

  /// If true then normals are already calculated for this frame.
  bool normals_calculated;

  /// Bounding box in object space for this frame.
  csBox3 box;
  /// Radius in object space for this frame.
  csVector3 radius;

public:
  ///
  csSpriteFrame (int anm_idx, int tex_idx);
  ///
  virtual ~csSpriteFrame ();

  ///
  void SetTexIndex (int tex_idx) { texturing_index = tex_idx; }

  /// Return true if normals are already calculated.
  bool NormalsCalculated () const { return normals_calculated; }
  /// Set normals calculated to value.
  void SetNormalsCalculated (bool n) { normals_calculated = n; }

  /// Set the name.
  virtual void SetName (char const*);
  /// Get the name.
  virtual char const* GetName () const { return name; }
  ///
  virtual int GetAnmIndex () const { return animation_index; }
  ///
  virtual int GetTexIndex () const { return texturing_index; }

  /**
   * Compute the object space bounding box for this frame.
   * This has to be called after setting up the frame and before
   * using it.
   */
  void SetBoundingBox (const csBox3& b) { box = b; }

  /**
   * Get the bounding box in object space.
   */
  void GetBoundingBox (csBox3& b) const { b = box; }

  /**
   * Set the radius of this frame in object space.
   */
  void SetRadius (const csVector3& r) { radius = r; }

  /**
   * Get the radius of this frame in object space.
   */
  void GetRadius (csVector3& r) const { r = radius; }

  SCF_DECLARE_IBASE;
};

/**
 * An action frameset for a 3D sprite animation.
 */
class csSpriteAction2 : public iSpriteAction
{
public:
  /// Initialize a action object
  csSpriteAction2 ();
  /// Destroy this action object
  virtual ~csSpriteAction2 ();

  /// Add a frame to this action @@@ OBSOLETE WHEN MOVED TO MESH SYSTEM
  void AddCsFrame (csSpriteFrame* frame, int delay);
  /// Add a frame to this action
  virtual void AddFrame (iSpriteFrame* frame, int delay);
  /// Set action name
  virtual void SetName (char const*);
  /// Get action name
  virtual char const* GetName () const
  { return name; }
  /// Get total number of frames in this action
  virtual int GetFrameCount () { return frames.Length (); }
  /// Query the frame number f. @@@ OBSOLETE WHEN MOVED TO MESH SYSTEM
  csSpriteFrame* GetCsFrame (int f)
  { return (f < frames.Length ()) ? (csSpriteFrame *)frames [f] : (csSpriteFrame*)NULL; }
  /// Returns the looping frame after frame number f. @@@ OBSOLETE WHEN MOVED TO MESH SYSTEM
  csSpriteFrame* GetCsNextFrame (int f)
  { f++; return f<frames.Length() ? (csSpriteFrame*)frames[f]:(csSpriteFrame*)frames[0]; }
  /// Query the frame number f.
  virtual iSpriteFrame* GetFrame (int f)
  { return (iSpriteFrame*)((f < frames.Length ()) ? (csSpriteFrame *)frames [f] : (csSpriteFrame*)NULL); }
  /// Returns the looping frame after frame number f.
  virtual iSpriteFrame* GetNextFrame (int f)
  { f++; return (iSpriteFrame*)(f<frames.Length() ? (csSpriteFrame*)frames[f]:(csSpriteFrame*)frames[0]); }
  /// Get delay for frame number f
  virtual int GetFrameDelay (int f)
  { return (int)delays [f]; }

  SCF_DECLARE_IBASE;

private:
  char *name;
  csVector frames;
  csVector delays;
};

/**
 * A vector for frames which knows how to clean them up.
 */
class csSpriteFrameVector : public csVector
{
public:
  /// Delete all inserted objects before deleting the object itself.
  virtual ~csSpriteFrameVector ();

  /// Free a item as a frame.
  virtual bool FreeItem (csSome Item);
};

/**
 * A vector for actions which knows how to clean them up.
 */
class csSpriteActionVector : public csVector
{
public:
  /// Delete all inserted objects before deleting the object itself.
  virtual ~csSpriteActionVector ();

  /// Free a item as an action.
  virtual bool FreeItem (csSome Item);
};

class csSprite3DMeshObject;

/**
 * A 3D sprite based on a triangle mesh with a single texture.
 * Animation is done with frames.
 * This class represents a template from which a csSprite3D
 * class can be made.
 */
class csSprite3DMeshObjectFactory : public iMeshObjectFactory
{
private:
  friend class csSprite3DMeshObject;

  /// Material handle as returned by iTextureManager.
  iMaterialWrapper* cstxt;

  /// An optional skeleton.
  csSkel* skeleton;

  /**
   * The order in which to introduce levels in order to get to a higher LOD.
   * The index of this array is the vertex number which is introduced.
   * The vertices of this template were reordered (by GenerateLOD()) so that
   * the first vertices are used in low-detail. The contents of this array
   * is the vertex number to emerge from.
   */
  int* emerge_from;

  /// The frames
  csSpriteFrameVector frames;
  /// The actions (a vector of csSpriteAction2 objects)
  csSpriteActionVector actions;

  /// Enable tweening.
  bool do_tweening;

  /// The lighting_quality for this template.  See macros CS_SPR_LIGHTING_*
  int lighting_quality;
 
  /**
   * The lighting_quality_config for this template.
   * See macros CS_SPR_LIGHT_*
   * This is used to set new sprites lighting_quality_config to this one.
   */
  int lighting_quality_config;

  /*
   * Configuration value for template LOD. 0 is lowest detail, 1 is maximum.
   * If negative then the base mesh is used and no LOD reduction/computation
   * is done.
   */
  float lod_level;

  /**
   * The lod_level_config for this template.
   * See macros CS_SPR_LOD_*
   * This is used to set new sprites lod_level_config to this one.
   */
  int lod_level_config;
 
  /// The base mesh is also the texture alignment mesh.
  csTriangleMesh2* texel_mesh;
  /// The array of texels
  CS_DECLARE_TYPED_VECTOR (csTexelsVector,csPoly2D) texels;
  /// The vertices
  CS_DECLARE_TYPED_VECTOR (csVerticesVector,csPoly3D) vertices;
  /// The normals
  CS_DECLARE_TYPED_VECTOR (csNormalsVector,csPoly3D) normals;

  /**
   * Connectivity information for this sprite template.
   * Also contains temporary vertex position information
   * for one sprite (@@@this should be avoided!!!!)
   */
  csTriangleVertices2* tri_verts;

  /// The default mixing mode for new sprites
  int MixMode;

  /// If true then this factory has been initialized.
  bool initialized;

public:
  iSystem* System;

public:
  /// Create the sprite template
  csSprite3DMeshObjectFactory (iBase *pParent);
  /// Destroy the template
  virtual ~csSprite3DMeshObjectFactory ();

  /// Set the skeleton for this sprite template.
  void SetSkeleton (csSkel* sk);

  /// Get the skeleton for this sprite template.
  csSkel* GetSkeleton () const { return skeleton; }

  /// Get the 'emerge_from' array from which you can construct triangles.
  int* GetEmergeFrom () const { return emerge_from; }

  /// Enable or disable tweening frames (default false).
  void EnableTweening (bool en) { do_tweening = en; }

  /// Is tweening enabled?
  bool IsTweeningEnabled () const { return do_tweening; }
 
  /// Returns the lighting quality for this template.
  int GetLightingQuality() const { return lighting_quality; }

  /// Sets the lighting quality for this template.  See CS_SPR_LIGHTING_* defs.
  void SetLightingQuality(int quality) {lighting_quality = quality; }

  /**
   * Sets which lighting config variable that all new sprites created
   * from this template will use.
   * The options are:
   * <ul>
   * <li>CS_SPR_LIGHT_GLOBAL (default)
   * <li>CS_SPR_LIGHT_TEMPLATE
   * <li>CS_SPR_LIGHT_LOCAL
   * </ul>
   */
  void SetLightingQualityConfig (int config_flag)
  { lighting_quality_config = config_flag; };
 
  /**
   * Returns what this template is using for determining the lighting quality.
   */
  int GetLightingQualityConfig () const
  { return lighting_quality_config; };

  /// Returns the lod_level for this template.
  float GetLodLevel() const { return lod_level; }

  /// Sets the lod level for this template.  See CS_SPR_LOD_* defs.
  void SetLodLevel(float level) {lod_level = level; }

  /**
   * Sets which lod config variable that all new sprites created
   * from this template will use.
   * The options are:
   * <ul>
   * <li>CS_SPR_LOD_GLOBAL (default)
   * <li>CS_SPR_LOD_TEMPLATE
   * <li>CS_SPR_LOD_LOCAL
   * </ul>
   */
  void SetLodLevelConfig (int config_flag)
  { lod_level_config = config_flag; };
 
  /**
   * Returns what this template is using for determining the lighting quality.
   */
  int GetLodLevelConfig () const
  { return lod_level_config; };

  /**
   * Generate the collapse order.
   * This function will also reorder all the vertices in the template.
   * So be careful!
   */
  void GenerateLOD ();

  /**
   * Compute the object space bounding box and radius for all frames in this
   * template. This has to be called after setting up the template and before
   * using it.
   */
  void ComputeBoundingBox ();

  ///
  csTriangleMesh2* GetTexelMesh () const {return texel_mesh;}

  /// Add some vertices, normals, and texels
  void AddVertices (int num);
  /// Add a vertex, normal, and texel
  void AddVertex () { AddVertices (1); }
  /// Query the number of vertices.
  int GetVertexCount () const { return vertices.Get (0)->GetVertexCount (); }

  /// Get a texel.
  csVector2& GetTexel (int frame, int vertex) const
    { return (*texels.Get(frame)) [vertex]; }
  /// Get array of texels.
  csVector2* GetTexels (int frame) const
    { return (*texels.Get(frame)).GetVertices (); }
  /// Set texel array.  The array is copied.
  void SetTexels(csVector2 const* tex, int frame)
    { (*texels.Get(frame)).SetVertices(tex, GetVertexCount ()); }

  /// Get a vertex.
  csVector3& GetVertex (int frame, int vertex) const
    { return (*vertices.Get(frame)) [vertex]; }
  /// Get vertex array.
  csVector3* GetVertices (int frame) const
    { return (*vertices.Get(frame)).GetVertices (); }
  /// Set vertex array.  The array is copied.
  void SetVertices(csVector3 const* verts, int frame)
    { (*vertices.Get(frame)).SetVertices(verts, GetVertexCount ()); }
	
  /// Get a normal.
  csVector3& GetNormal (int frame, int vertex) const
    { return (*normals.Get(frame)) [vertex]; }
  /// Get normal array.
  csVector3* GetNormals (int frame) const
    { return (*normals.Get(frame)).GetVertices (); }
  /// Set normal array.  The array is copied.
  void SetNormals(csVector3 const* norms, int frame)
    { (*normals.Get(frame)).SetVertices(norms, GetVertexCount ()); }

  /**
   * Add a triangle to the normal, texel, and vertex meshes
   * a, b and c are indices to texel vertices
   */
  void AddTriangle (int a, int b, int c);
  /// returns the texel indices for triangle 'x'
  csTriangle GetTriangle (int x) const { return texel_mesh->GetTriangle(x); }
  /// returns the triangles of the texel_mesh
  csTriangle* GetTriangles () const { return texel_mesh->GetTriangles(); }
  /// returns the number of triangles in the sprite
  int GetTriangleCount () const { return texel_mesh->GetTriangleCount(); }
  /// Size triangle buffer size
  void SetTriangleCount( int count ) { texel_mesh->SetSize(count); }
  /// Set a bank of triangles.  The bank is copied.
  void SetTriangles( csTriangle const* trig, int count ) { texel_mesh->SetTriangles(trig, count); }

  /// Create and add a new frame to the sprite.
  csSpriteFrame* AddFrame ();
  /// find a named frame into the sprite.
  csSpriteFrame* FindFrame (const char * name);
  /// Query the number of frames
  int GetFrameCount () const { return frames.Length (); }
  /// Query the frame number f
  csSpriteFrame* GetFrame (int f) const
  { return (f < frames.Length ()) ? (csSpriteFrame *)frames [f] : (csSpriteFrame*)NULL; }

  /// Create and add a new action frameset to the sprite.
  csSpriteAction2* AddAction ();
  /// find a named action into the sprite.
  csSpriteAction2* FindAction (const char * name) const;
  /// Get the first action.
  csSpriteAction2* GetFirstAction () const
  { return (csSpriteAction2 *)actions [0]; }
  /// Get number of actions in sprite
  int GetActionCount () const
  { return actions.Length (); }
  /// Get action number No
  csSpriteAction2* GetAction (int No) const
  { return (csSpriteAction2 *)actions [No]; }

  /// Get the material
  iMaterialWrapper* GetMaterial () const
  { return cstxt; }
  /// Get the material handle.
  iMaterialHandle* GetMaterialHandle () const
  { return cstxt->GetMaterialHandle (); }
  /// Set the material used for this sprite
  void SetMaterial (iMaterialWrapper *material);

  /**
   * Compute all normals in a frame.
   */
  void ComputeNormals (csSpriteFrame* frame);

  /**
   * Smooth out the gouraud shading by merging the precalculated
   * vertex normals along seams in frame 'frame' based on which
   * vertices are very close in frame 'base'
   */
  void MergeNormals (int base, int frame);

  /**
   * Smooth out the gouraud shading by merging the precalculated
   * vertex normals along seams in all frames based on which
   * vertices are very close in frame 'base'
   */
  void MergeNormals (int base);

  /**
   * Smooth out the gouraud shading by merging the precalculated
   * vertex normals along seams in all frames based on which
   * vertices are very close in each frame
   */
  void MergeNormals ();

  void SetMixMode (UInt mode)
  { MixMode = mode; }
  UInt GetMixMode () const
  { return MixMode; }

  //------------------------ iMeshObjectFactory implementation --------------
  SCF_DECLARE_IBASE;

  virtual iMeshObject* NewInstance ();
  virtual void HardTransform (const csReversibleTransform& t);
  virtual bool SupportsHardTransform () const { return true; }

  //--------------------- iSprite3DFactoryState implementation -------------//
  struct Sprite3DFactoryState : public iSprite3DFactoryState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSprite3DMeshObjectFactory);
    virtual void SetMaterialWrapper (iMaterialWrapper* material)
    {
      scfParent->SetMaterial (material);
    }
    virtual iMaterialWrapper* GetMaterialWrapper () const
    {
      return scfParent->GetMaterial ();
    }
    virtual void AddVertices (int num)
    {
      scfParent->AddVertices (num);
    }
    virtual int GetVertexCount () const
    {
      return scfParent->GetVertexCount ();
    }
    virtual const csVector3& GetVertex (int frame, int vertex) const
    {
      return scfParent->GetVertex (frame, vertex);
    }
    virtual void SetVertex (int frame, int vertex, const csVector3 &val)
    {
      scfParent->GetVertex (frame, vertex) = val;
    }
    virtual csVector3* GetVertices (int frame) const
    {
      return scfParent->GetVertices (frame);
    }
    virtual void SetVertices(csVector3 const* verts, int frame)
    {
      scfParent->SetVertices(verts, frame);
    }
    virtual const csVector2& GetTexel (int frame, int vertex) const
    {
      return scfParent->GetTexel (frame, vertex);
    }
    virtual void SetTexel (int frame, int vertex, const csVector2 &val)
    {
      scfParent->GetTexel (frame, vertex) = val;
    }
    virtual csVector2* GetTexels (int frame) const
    {
      return scfParent->GetTexels (frame);
    }
    virtual void SetTexels(csVector2 const* tex, int frame)
    {
      scfParent->SetTexels(tex, frame);
    }
    virtual const csVector3& GetNormal (int frame, int vertex) const
    {
      return scfParent->GetNormal (frame, vertex);
    }
    virtual void SetNormal (int frame, int vertex, const csVector3 &val)
    {
      scfParent->GetNormal (frame, vertex) = val;
    }
    virtual csVector3* GetNormals (int frame) const
    {
      return scfParent->GetNormals (frame);
    }
    virtual void SetNormals(csVector3 const* norms, int frame)
    {
      scfParent->SetNormals(norms, frame);
    }
    virtual void AddTriangle (int a, int b, int c)
    {
      scfParent->AddTriangle (a, b, c);
    }
    virtual csTriangle GetTriangle (int x) const
    {
      return scfParent->GetTriangle (x);
    }
    virtual csTriangle* GetTriangles () const
    {
      return scfParent->GetTriangles ();
    }
    virtual int GetTriangleCount () const
    {
      return scfParent->GetTriangleCount ();
    }
    virtual void SetTriangleCount( int count )
    {
      scfParent->SetTriangleCount(count);
    }
    virtual void SetTriangles( csTriangle const* trig, int count)
    {
      scfParent->SetTriangles(trig, count);
    }
    virtual iSpriteFrame* AddFrame ()
    {
      iSpriteFrame* ifr = SCF_QUERY_INTERFACE_SAFE (scfParent->AddFrame (), iSpriteFrame);
      if (ifr) ifr->DecRef ();
      return ifr;
    }
    virtual iSpriteFrame* FindFrame (const char* name) const
    {
      iSpriteFrame* ifr = SCF_QUERY_INTERFACE_SAFE (scfParent->FindFrame (name), iSpriteFrame);
      if (ifr) ifr->DecRef ();
      return ifr;
    }
    virtual int GetFrameCount () const
    {
      return scfParent->GetFrameCount ();
    }
    virtual iSpriteFrame* GetFrame (int f) const
    {
      iSpriteFrame* ifr = SCF_QUERY_INTERFACE_SAFE (scfParent->GetFrame (f), iSpriteFrame);
      if (ifr) ifr->DecRef ();
      return ifr;
    }
    virtual iSpriteAction* AddAction ()
    {
      iSpriteAction* ia = SCF_QUERY_INTERFACE_SAFE (scfParent->AddAction (), iSpriteAction);
      if (ia) ia->DecRef ();
      return ia;
    }
    virtual iSpriteAction* FindAction (const char* name) const
    {
      iSpriteAction* ia = SCF_QUERY_INTERFACE_SAFE (scfParent->FindAction (name), iSpriteAction);
      if (ia) ia->DecRef ();
      return ia;
    }
    virtual iSpriteAction* GetFirstAction () const
    {
      iSpriteAction* ia = SCF_QUERY_INTERFACE_SAFE (scfParent->GetFirstAction (), iSpriteAction);
      if (ia) ia->DecRef ();
      return ia;
    }
    virtual int GetActionCount () const
    {
      return scfParent->GetActionCount ();
    }
    virtual iSpriteAction* GetAction (int No) const
    {
      iSpriteAction* ia = SCF_QUERY_INTERFACE_SAFE (scfParent->GetAction (No), iSpriteAction);
      if (ia) ia->DecRef ();
      return ia;
    }
    virtual void EnableSkeletalAnimation ();
    virtual iSkeleton* GetSkeleton () const;
    virtual void EnableTweening (bool en)
    {
      scfParent->EnableTweening (en);
    }
    virtual bool IsTweeningEnabled () const
    {
      return scfParent->IsTweeningEnabled ();
    }
    virtual void SetLightingQuality (int qual)
    {
      scfParent->SetLightingQuality (qual);
    }
    virtual int GetLightingQuality () const
    {
      return scfParent->GetLightingQuality ();
    }
    virtual void SetLightingQualityConfig (int qual)
    {
      scfParent->SetLightingQualityConfig (qual);
    }
    virtual int GetLightingQualityConfig () const
    {
      return scfParent->GetLightingQualityConfig ();
    }
    virtual float GetLodLevel () const
    {
      return scfParent->GetLodLevel ();
    }
    virtual void SetLodLevel (float level)
    {
      scfParent->SetLodLevel (level);
    }
    virtual void SetLodLevelConfig (int config_flag)
    {
      scfParent->SetLodLevelConfig (config_flag);
    }
    virtual int GetLodLevelConfig () const
    {
      return scfParent->GetLodLevelConfig ();
    }
    virtual void MergeNormals (int base, int frame)
    {
      scfParent->MergeNormals (base, frame);
    }
    virtual void MergeNormals (int base)
    {
      scfParent->MergeNormals (base);
    }
    virtual void MergeNormals ()
    {
      scfParent->MergeNormals ();
    }
    virtual void SetMixMode (UInt mode)
    { scfParent->SetMixMode (mode); }
    virtual UInt GetMixMode () const
    { return scfParent->GetMixMode (); }
  } scfiSprite3DFactoryState;
};

/**
 * A 3D sprite based on a triangle mesh with a single texture.
 * Animation is done with frames (a frame may be controlled by
 * a skeleton).
 */
class csSprite3DMeshObject : public iMeshObject
{
private:
  /// Set the size of internally used tables
  static void UpdateWorkTables (int max_size);

public:
  /**
   * Configuration value for global LOD. 0 is lowest detail, 1 is maximum.
   * If negative then the base mesh is used and no LOD reduction/computation
   * is done.
   */
  static float global_lod_level;
 
private:
 
  /**
   * Used to determine where to look for the lod detail level.
   * The possible values are:
   *   <ul>
   *     <li>CS_SPR_LOD_GLOBAL (default)
   *     <li>CS_SPR_LOD_TEMPLATE
   *     <li>CS_SPR_LOD_LOCAL
   *   </ul>
   */
  int lod_level_config;
 
  /**
   * Configuration value for an individuals LOD. 0 is lowest detail,
   * 1 is maximum.  If negative then the base mesh is used and no LOD
   * reduction/computation is done.
   */
  float local_lod_level;

  /**
   * Quality setting for sprite lighting.
   * See the CS_SPR_LIGHTING_* macros defined in this header file for the
   * different types of lighting.  This is the local setting.  It overrides the
   * template, and global lighting settings.
   */
  int local_lighting_quality;
 
  /**
   * Used to determine where to look for the quality setting of the lighting.
   * The possible values are:
   *   <ul>
   *     <li>CS_SPR_LIGHT_GLOBAL (default)
   *     <li>CS_SPR_LIGHT_TEMPLATE
   *     <li>CS_SPR_LIGHT_LOCAL
   *   </ul>
   */
  int lighting_quality_config;

  float current_lod;
  uint32 current_features;

public:
 
  /**
   * Quality setting for sprite lighting.
   * See the CS_SPR_LIGHTING_* macros defined in this header file for the
   * different types of lighting.  This is the global setting that is used for
   * all csSprite3Ds(unless the template or the individual sprite overrides
   * it).
   */
  static int global_lighting_quality;

  /**
   * Returns the lighting quality level used by this sprite.
   * See SPT_LIGHTING_* macros defined in this header for the different types
   * of lighting.
   */
  int GetLightingQuality ()
  {
    switch (lighting_quality_config)
    {
      case CS_SPR_LIGHT_GLOBAL:      return global_lighting_quality; break;
      case CS_SPR_LIGHT_TEMPLATE:    return factory->GetLightingQuality(); break;
      case CS_SPR_LIGHT_LOCAL:       return local_lighting_quality; break;
      default:
      {
	lighting_quality_config = factory->GetLightingQualityConfig();
	return factory->GetLightingQuality();
      }
    }
  }
 
  /**
   * Sets the local lighting quality for this sprite.  NOTE: you must use
   * SetLightingQualityConfig (CS_SPR_LIGHT_LOCAL) for the sprite to use this.
   */
  void SetLocalLightingQuality(int lighting_quality)
  { local_lighting_quality = lighting_quality; }
 
  /**
   * Sets the global lighting quality for all csSprite3Ds.
   * NOTE: You must use SetLightingQualityConfig(CS_SPR_LIGHT_GLOBAL) for the
   * sprite to use this.
   */
  void SetGlobalLightingQuality (int lighting_quality)
  { global_lighting_quality = lighting_quality; }
 
  /**
   * Sets which lighting config variable this sprite will use.
   * The options are:
   * <ul>
   * <li>CS_SPR_LIGHT_GLOBAL (default)
   * <li>CS_SPR_LIGHT_TEMPLATE
   * <li>CS_SPR_LIGHT_LOCAL
   * </ul>
   */
  void SetLightingQualityConfig (int config_flag)
  { lighting_quality_config = config_flag; }
 
  /**
   * Returns what this sprite is using for determining the lighting quality.
   */
  int GetLightingQualityConfig () const
  { return lighting_quality_config; }

  /**
   * Returns the lod level used by this sprite.
   */
  float GetLodLevel ()
  {
    switch (lod_level_config)
    {
      case CS_SPR_LOD_GLOBAL:      return global_lod_level; break;
      case CS_SPR_LOD_TEMPLATE:    return factory->GetLodLevel(); break;
      case CS_SPR_LOD_LOCAL:       return local_lod_level; break;
      default:
      {
	lod_level_config = factory->GetLodLevelConfig();
	return factory->GetLodLevel();
      }
    }
  }
 
  /**
   * Sets the local lod level for this sprite.  NOTE: you must use
   * SetLodLevelConfig (CS_SPR_LOD_LOCAL) for the sprite to use this.
   */
  void SetLocalLodLevel (float lod_level)
  { local_lod_level = lod_level; }
 
  /**
   * Sets the global lod level for all csSprite3Ds.  NOTE: you must use
   * SetLodLevelConfig(CS_SPR_LOD_GLOBAL) for the sprite to use this.
   */
  void SetGlobalLodLevel (float lod_level)
  { global_lod_level = lod_level; }
 
  /**
   * Sets which lighting config variable this sprite will use.
   * The options are:
   * <ul>
   *   <li>CS_SPR_LOD_GLOBAL (default)
   *   <li>CS_SPR_LOD_TEMPLATE
   *   <li>CS_SPR_LOD_LOCAL
   * </ul>
   */
  void SetLodLevelConfig (int config_flag)
  { lod_level_config = config_flag; }
 
  /**
   * Returns what this sprite is using for determining the lighting quality.
   */
  int GetLodLevelConfig () const
  { return lod_level_config; }

  /**
   * GetVertexToLightCount returns the number of vertices to light based on LOD.
   */
  int GetVertexToLightCount ();
 
private:

  /**
   * num_verts_for_lod represents the number of lights that are used by lod.
   * If -1 means that it is not used.
   */
  int num_verts_for_lod;
 
  /**
   * A mesh which contains a number of triangles as generated
   * by the LOD algorithm. This is static since it will likely
   * change every frame anyway. We hold it static also since
   * we don't want to allocate it again every time.
   */
  static csTriangleMesh2 mesh;

  /// Mixmode for the triangles/polygons of the sprite.
  UInt MixMode;

  /**
   * Array of colors for the vertices. If not set then this
   * sprite does not have colored vertices.
   */
  csColor* vertex_colors;

  /**
   * Base color that will be added to the sprite colors.
   */
  csColor base_color;

  /// The parent.
  csSprite3DMeshObjectFactory* factory;

  /// The material handle as returned by iTextureManager.
  iMaterialWrapper* cstxt;

  /// The current frame number.
  int cur_frame;
  /// The current action.
  csSpriteAction2* cur_action;

  /// The last frame time action.
  csTime last_time;

  /// Animation tweening ratio:  next frame / this frame.
  float tween_ratio;

  /// Enable tweening.
  bool do_tweening;

  /// Enable or disable lighting.
  bool do_lighting;

  ///
  bool force_otherskin;

  /// Skeleton state (optional).
  csSkelState* skeleton_state;

  iMeshObjectDrawCallback* vis_cb;
  long shapenr;

  /**
   * Camera space bounding box is cached here.
   * GetCameraBoundingBox() will check the current camera number
   * to see if it needs to recalculate this.
   */
  csBox3 camera_bbox;

  /// Current camera number.
  long cur_cameranr;
  /// Current movable number.
  long cur_movablenr;

  // Remembered info between DrawTest and Draw.
  G3DTriangleMesh g3dmesh;

  bool initialized;

  /// Setup this object.
  void SetupObject ();

private:
  /**
   * High quality version of UpdateLighting() which recalculates
   * the distance between the light and every vertex.
   * This version can use tweening of the normals and vertices
   */
  void UpdateLightingHQ (iLight** lights, int num_lights, iMovable* movable);

  /**
   * Low quality version of UpdateLighting() which only
   * calculates the distance once (from the center of the sprite.)
   * This method can use tweening of the normals.
   */
  void UpdateLightingLQ (iLight** lights, int num_lights, iMovable* movable);
 
  /**
   * Low quality Fast version of UpdateLighting() which only
   * calculates the distance once (from the center of the sprite.)
   * This version can NOT use any tweening.
   */
  void UpdateLightingFast (iLight** lights, int num_lights, iMovable* movable);

  /**
   * A fairly fast :P totally inaccurate(usually) lighting method.
   *  Intended for use for things like powerups.
   */
  void UpdateLightingRandom ();

  /// random number generator used for random lighting.
  csRandomGen *rand_num;

public:
  /// Constructor.
  csSprite3DMeshObject ();
  /// Destructor.
  virtual ~csSprite3DMeshObject ();

  /// Set the factory.
  void SetFactory (csSprite3DMeshObjectFactory* factory);

  /// Get the factory.
  csSprite3DMeshObjectFactory* GetFactory3D () const { return factory; }

  /// Get the skeleton state for this sprite.
  csSkelState* GetSkeletonState () const { return skeleton_state; }

  /// Force a new material skin other than default
  void SetMaterial (iMaterialWrapper *material);

  /// Get the material for this sprite.
  iMaterialWrapper* GetMaterial () const { return cstxt; }

  /// Sets the mode that is used, when drawing that sprite.
  void SetMixMode (UInt m) { MixMode = m; }

  /// Gets the mode that is used, when drawing that sprite.
  UInt GetMixMode () const { return MixMode; }

  /// Enable or disable tweening frames (default false).
  void EnableTweening (bool en) { do_tweening = en; }

  /// Is tweening enabled?
  bool IsTweeningEnabled () const { return do_tweening; }

  /// Set lighting.
  void SetLighting (bool l)
  {
    do_lighting = l;
    ResetVertexColors ();
  }
  /// Is lighting enabled?
  bool IsLighting () const
  {
    return do_lighting;
  }

  /// Set base color.
  void SetBaseColor (const csColor& col)
  {
    delete[] vertex_colors;
    vertex_colors = NULL;
    base_color = col;
    ResetVertexColors ();
  }

  /// Get base color.
  void GetBaseColor (csColor& col) const
  {
    col = base_color;
  }

  /**
   * Add a color for a vertex.
   * As soon as you use this function this sprite will be rendered
   * using gouraud shading. Calling this function for the first time
   * will initialize all colors to black.
   */
  void AddVertexColor (int i, const csColor& col);

  /**
   * Reset the color list. If you call this function then the
   * sprite will no longer use gouraud shading.
   */
  void ResetVertexColors ();

  /**
   * Clamp all vertice colors to 2.0. This is called inside
   * csSprite3D::UpdateLighting() so that 3D renderer doesn't have
   * to deal with brightness lighter than 2.0
   */
  void FixVertexColors ();

  /// Unset the texture.
  void UnsetTexture ()
  { force_otherskin = false; }

  /**
   * Fill the static mesh with the current sprite
   * for a given LOD level.
   */
  void GenerateSpriteLOD (int num_vts);

  /**
   * Go to the next frame depending on the current time in milliseconds.
   */
  bool OldNextFrame (csTime current_time, bool onestep = false,
    bool stoptoend = false);

  /**
   * Go to a specified frame.
   */
  void SetFrame (int f)
  {
    if (cur_action && f < cur_action->GetFrameCount ()) cur_frame = f;
    //@@@!!! WHAT DO WE DO HERE!!! UpdateInPolygonTrees ();
  }

  /**
   * Get the current frame number.
   */
  int GetCurFrame () const { return cur_frame; }

  /**
   * Get the current frame number.
   */
  csSpriteAction2* GetCurAction () const { return cur_action; }

  /**
   * Get the number of frames.
   */
  int GetFrameCount () const { return cur_action->GetFrameCount (); }

  /**
   * Select an action.
   */
  bool SetAction (const char * name)
  {
    csSpriteAction2 *act;
    if ((act = factory->FindAction (name)) != NULL)
    {
      cur_action = act;
      SetFrame (0);
      return true;
    }
    return false;
  }

  /**
   * Initialize a sprite. This function is called automatically
   * from within 'load'. However you should call it directly
   * if you created the sprite on the fly (without 'load').
   */
  void InitSprite ();

  /**
   * Get an array of object vertices which is valid for the given frame.
   * This function correcty acounts for sprites which use skeletons. In
   * that case it will use the current transformation state of the skeleton
   * to compute object space vertices.<br>
   * Warning! The returned array should be used immediatelly or copied. It
   * points to a private static array in the sprite class and can be reused
   * if other calls to the sprite happen.
   */
  csVector3* GetObjectVerts (csSpriteFrame* fr);

  /// Get the bounding box in transformed space.
  void GetTransformedBoundingBox (long cameranr, long movablenr,
      const csReversibleTransform& trans, csBox3& cbox);

  /**
   * Get the coordinates of the sprite in screen coordinates.
   * Fills in the boundingBox with the X and Y locations of the cube.
   * Returns the max Z location of the sprite, or -1 if not
   * on-screen. If the sprite is not on-screen, the X and Y values are not
   * valid.
   */
  float GetScreenBoundingBox (long cameranr, long movablenr,
  	float fov, float sx, float sy,
	const csReversibleTransform& trans, csBox2& sbox, csBox3& cbox);

  ///------------------------ iMeshObject implementation ----------------------
  SCF_DECLARE_IBASE;

  virtual iMeshObjectFactory* GetFactory () const
  {
    iMeshObjectFactory* ifact = SCF_QUERY_INTERFACE (factory, iMeshObjectFactory);
    ifact->DecRef ();
    return ifact;
  }
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
  virtual void GetRadius (csVector3& rad, csVector3 &cent);
  virtual void NextFrame (csTime current_time)
  {
    OldNextFrame (current_time);
  }
  virtual bool WantToDie () const { return false; }
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
  virtual bool HitBeamBBox (const csVector3&, const csVector3&);
  virtual bool HitBeamOutline (const csVector3&, const csVector3&);
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

  //------------------ iPolygonMesh interface implementation ----------------//
  struct PolyMesh : public iPolygonMesh
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSprite3DMeshObject);

    /// Get the number of vertices for this mesh.
    virtual int GetVertexCount ()
    {
      csSprite3DMeshObjectFactory* fact = scfParent->GetFactory3D ();
      return fact->GetVertexCount ();
    }
    /// Get the pointer to the array of vertices.
    virtual csVector3* GetVertices ()
    {
      csSprite3DMeshObjectFactory* fact = scfParent->GetFactory3D ();
      return fact->GetVertices (0);
    }
    /// Get the number of polygons for this mesh.
    virtual int GetPolygonCount ()
    {
      csSprite3DMeshObjectFactory* fact = scfParent->GetFactory3D ();
      return fact->GetTriangleCount ();
    }
    
    /// Get the pointer to the array of polygons.
    virtual csMeshedPolygon* GetPolygons ();

    PolyMesh ()
    {
      polygons = NULL;
    }

    virtual ~PolyMesh ()
    {
      delete[] polygons;
    }

    csMeshedPolygon* polygons;
  } scfiPolygonMesh;
  friend struct PolyMesh;

  //--------------------- iSprite3DState implementation -------------//
  struct Sprite3DState : public iSprite3DState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSprite3DMeshObject);
    virtual void SetMaterialWrapper (iMaterialWrapper* material)
    {
      scfParent->SetMaterial (material);
    }
    virtual iMaterialWrapper* GetMaterialWrapper () const
    {
      return scfParent->GetMaterial ();
    }
    virtual void SetMixMode (UInt mode)
    {
      scfParent->SetMixMode (mode);
    }
    virtual UInt GetMixMode () const
    {
      return scfParent->GetMixMode ();
    }
    virtual void SetLighting (bool l)
    {
      scfParent->SetLighting (l);
    }
    virtual bool IsLighting () const
    {
      return scfParent->IsLighting ();
    }
    virtual iSkeletonState* GetSkeletonState () const;
    virtual void SetFrame (int f)
    {
      scfParent->SetFrame (f);
    }
    virtual int GetCurFrame () const
    {
      return scfParent->GetCurFrame ();
    }
    virtual int GetFrameCount () const
    {
      return scfParent->GetFrameCount ();
    }
    virtual bool SetAction (const char * name)
    {
      return scfParent->SetAction (name);
    }
    virtual iSpriteAction* GetCurAction () const
    {
      iSpriteAction* ia = SCF_QUERY_INTERFACE_SAFE (scfParent->GetCurAction (), iSpriteAction);
      if (ia) ia->DecRef ();
      return ia;
    }
    virtual void EnableTweening (bool en)
    {
      scfParent->EnableTweening (en);
    }
    virtual bool IsTweeningEnabled () const
    {
      return scfParent->IsTweeningEnabled ();
    }
    virtual void UnsetTexture ()
    {
      scfParent->UnsetTexture ();
    }
    virtual int GetLightingQuality ()
    {
      return scfParent->GetLightingQuality ();
    }
    virtual void SetLocalLightingQuality (int lighting_quality)
    {
      scfParent->SetLocalLightingQuality (lighting_quality);
    }
    virtual void SetLightingQualityConfig (int config_flag)
    {
      scfParent->SetLightingQualityConfig (config_flag);
    }
    virtual int GetLightingQualityConfig () const
    {
      return scfParent->GetLightingQualityConfig ();
    }
    virtual float GetLodLevel ()
    {
      return scfParent->GetLodLevel ();
    }
    virtual void SetLocalLodLevel (float lod_level)
    {
      scfParent->SetLocalLodLevel (lod_level);
    }
    virtual void SetLodLevelConfig (int config_flag)
    {
      scfParent->SetLodLevelConfig (config_flag);
    }
    virtual int GetLodLevelConfig () const
    {
      return scfParent->GetLodLevelConfig ();
    }
    virtual bool IsLodEnabled () const
    {
      return scfParent->GetLodLevel () >= 0;
    }
    virtual void SetBaseColor (const csColor& col)
    {
      scfParent->SetBaseColor (col);
    }
    virtual void GetBaseColor (csColor& col) const
    {
      scfParent->GetBaseColor (col);
    }
  } scfiSprite3DState;
};

/**
 * Sprite 3D type. This is the plugin you have to use to create instances
 * of csSprite3DMeshObjectFactory.
 */
class csSprite3DMeshObjectType : public iMeshObjectType
{
private:
  iSystem* System;

public:
  /// Constructor.
  csSprite3DMeshObjectType (iBase*);

  /// Destructor.
  virtual ~csSprite3DMeshObjectType ();

  //------------------------ iMeshObjectType implementation --------------
  SCF_DECLARE_IBASE;

  /// New Factory.
  virtual iMeshObjectFactory* NewFactory ();
  virtual uint32 GetFeatures () const
  {
    return ALL_FEATURES;
  }

  //------------------- iConfig interface implementation -------------------
  struct csSprite3DConfig : public iConfig
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSprite3DMeshObjectType);
    virtual bool GetOptionDescription (int idx, csOptionDescription *option);
    virtual bool SetOption (int id, csVariant* value);
    virtual bool GetOption (int id, csVariant* value);
  } scfiConfig;
  friend struct csSprite3DConfig;

  //--------------------- iPlugin interface implementation
  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSprite3DMeshObjectType);
    virtual bool Initialize (iSystem* p)
    { scfParent->System = p; return true; }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
  friend struct eiPlugin;
};

#endif // __CS_SPR3D_H__
