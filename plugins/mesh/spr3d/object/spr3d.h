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

#include "csutil/sysfunc.h"
#include "csutil/cscolor.h"
#include "csutil/array.h"
#include "csutil/parray.h"
#include "csutil/refarr.h"
#include "csutil/weakref.h"
#include "csutil/randomgen.h"
#include "csutil/leakguard.h"
#include "csgeom/math3d.h"
#include "csgeom/math2d.h"
#include "csgeom/poly2d.h"
#include "csgeom/poly3d.h"
#include "csgeom/box.h"
#include "csgeom/objmodel.h"
#include "csgeom/trimeshlod.h"
#include "imesh/sprite3d.h"
#include "imesh/object.h"
#include "iengine/material.h"
#include "iengine/lod.h"
#include "iengine/sharevar.h"
#include "iengine/lightmgr.h"
#include "iutil/config.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/virtclk.h"
#include "ivideo/graph3d.h"
#include "ivideo/rendermesh.h"
#include "ivideo/rndbuf.h"
#include "cstool/userrndbuf.h"
#include "cstool/rendermeshholder.h"
#include "ivideo/material.h"
#include "csqint.h"
#include "csgfx/shadervarcontext.h"
#include "csutil/dirtyaccessarray.h"

struct iObjectRegistry;
struct iEngine;

/**
 * A listener to listen to the variables.
 */
class csSpriteLODListener : public iSharedVariableListener
{
private:
  float* variable;
public:
  SCF_DECLARE_IBASE;
  csSpriteLODListener (float* variable)
  {
    SCF_CONSTRUCT_IBASE (0);
    csSpriteLODListener::variable = variable;
  }
  virtual ~csSpriteLODListener ()
  {
    SCF_DESTRUCT_IBASE ();
  }

  virtual void VariableChanged (iSharedVariable* var)
  {
    *variable = var->Get ();
  }
};

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

  /// Add a frame to this action.
  void AddCsFrame (csSpriteFrame* frame, int delay, float displace);
  /// Add a frame to this action
  virtual void AddFrame (iSpriteFrame* frame, int delay, float displace);
  /// Set action name
  virtual void SetName (char const*);
  /// Get action name
  virtual char const* GetName () const
  { return name; }
  /// Get total number of frames in this action
  virtual int GetFrameCount () { return (int)frames.Length (); }
  /// Query the frame number f.
  csSpriteFrame* GetCsFrame (int f)
  {
    return ((size_t)f < frames.Length ())
    	? frames [f]
	: (csSpriteFrame*)0;
	}
  /// Returns the looping frame after frame number f.
  csSpriteFrame* GetCsNextFrame (int f)
  {
    if (!reverse_action)
    {
      f++;
      return (size_t)f<frames.Length() ? frames[f] : frames[0];
    }
    else
    {
      f--;
      return f>=0
        ? frames[f]
	: frames[frames.Length()-1];
    }
  }
  /// Query the frame number f.
  virtual iSpriteFrame* GetFrame (int f)
  {
    return (iSpriteFrame*)(((size_t)f < frames.Length ())
    	? frames [f]
	: (csSpriteFrame*)0);
  }
  /// Returns the looping frame after frame number f.
  virtual iSpriteFrame* GetNextFrame (int f)
  {
    if (!reverse_action)
    {
      f++;
      return (iSpriteFrame*)((size_t)f<frames.Length()
      	? frames[f]
	: frames[0]);
    }
    else
    {
      f--;
      return (iSpriteFrame*)(f>=0
      	? frames[f]
	: frames[frames.Length()-1]);
    }
  }
  /// Get delay for frame number f
  virtual int GetFrameDelay (int f)
  { return delays [f]; }
  /// Get displacement for frame number f
  virtual float GetFrameDisplacement (int f)
  { return displacements [f]; }

  void SetReverseAction (bool reverse)
  { reverse_action = reverse; }

  SCF_DECLARE_IBASE;

private:
  char *name;
  bool reverse_action;
  csArray<csSpriteFrame*> frames;
  csArray<int> delays;
  csArray<float> displacements;
};


/**
 * A socket for specifying where sprites can plug into
 * other sprites.
 */
class csSpriteSocket : public iSpriteSocket
{
private:
  char* name;
  int triangle_index;
  iMeshWrapper *attached_mesh;

public:
  
  /// Default Constructor
  csSpriteSocket();
  
  virtual ~csSpriteSocket ();

  /// Set the name.
  virtual void SetName (char const*);
  /// Get the name.
  virtual char const* GetName () const { return name; }
  
  /// Set the attached sprite.
  virtual void SetMeshWrapper (iMeshWrapper* mesh) {attached_mesh = mesh;}
  /// Get the attached sprite.
  virtual iMeshWrapper* GetMeshWrapper () const {return attached_mesh;}

  /// Set the index of the triangle for the socket.
  virtual void SetTriangleIndex (int tri_index) { triangle_index = tri_index; }
  /// Get the index of the triangle for the socket.
  virtual int GetTriangleIndex () const { return triangle_index; }
  
  SCF_DECLARE_IBASE;
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
  csRef<iMaterialWrapper> cstxt;
  iBase* logparent;
  iMeshObjectType* spr3d_type;

  /// Cache name for caching sprite specific data.
  char* cachename;

  /**
   * The order in which to introduce levels in order to get to a higher LOD.
   * The index of this array is the vertex number which is introduced.
   * The vertices of this template were reordered (by GenerateLOD()) so that
   * the first vertices are used in low-detail. The contents of this array
   * is the vertex number to emerge from.
   */
  int* emerge_from;

  /// The frames.
  csPDelArray<csSpriteFrame> frames;
  /// The actions.
  csPDelArray<csSpriteAction2> actions;
  /// The sockets.
  csPDelArray<csSpriteSocket> sockets;

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

  /**
   * Values for the function <code>lod=m*distance+a</code> that is used
   * to compute the actual LOD level for this object.
   */
  float lod_m, lod_a;

  /**
   * It is also possible to use variables. If these are not 0 then the
   * lod values are taken from variables.
   */
  csRef<iSharedVariable> lod_varm;
  csRef<iSharedVariable> lod_vara;
  csRef<csSpriteLODListener> lod_varm_listener;
  csRef<csSpriteLODListener> lod_vara_listener;

  /**
   * The lod_level_config for this template.
   * See macros CS_SPR_LOD_*
   * This is used to set new sprites lod_level_config to this one.
   */
  int lod_level_config;

  /// The base mesh is also the texture alignment mesh.
  csTriangleMesh* texel_mesh;
  /// The array of texels
  csPDelArray<csPoly2D> texels;
  /// The vertices
  csPDelArray<csPoly3D> vertices;
  /// The normals
  csPDelArray<csPoly3D> normals;

  csFlags flags;

  /**
   * Connectivity information for this sprite template.
   * Also contains temporary vertex position information
   * for one sprite (@@@this should be avoided!!!!)
   */
  csTriangleVerticesCost* tri_verts;

  /// The default mixing mode for new sprites
  int MixMode;

  /// If true then this factory has been initialized.
  bool initialized;

  void GenerateCacheName ();
  const char* GetCacheName ();

public:
  CS_LEAKGUARD_DECLARE(csSprite3DMeshObjectFactory);

  iObjectRegistry* object_reg;
  iVirtualClock* vc;

  csWeakRef<iGraphics3D> g3d;
  csRef<iLightManager> light_mgr;

  /**
   * Reference to the engine (optional because sprites can also be
   * used for the isometric engine).
   */
  iEngine* engine;

public:
  /// Create the sprite template.
  csSprite3DMeshObjectFactory (iMeshObjectType* pParent);
  /// Destroy the template.
  virtual ~csSprite3DMeshObjectFactory ();

  void Report (int severity, const char* msg, ...);

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
  csTriangleMesh* GetTexelMesh () const {return texel_mesh;}

  /// Add some vertices, normals, and texels
  void AddVertices (int num);
  /// Add a vertex, normal, and texel
  void AddVertex () { AddVertices (1); }
  /// Query the number of vertices.
  int GetVertexCount () const { return (int)vertices.Get (0)->GetVertexCount (); }

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
  csTriangle GetTriangle (int x) const { return texel_mesh->GetTriangle (x); }
  /// returns the triangles of the texel_mesh
  csTriangle* GetTriangles () const { return texel_mesh->GetTriangles (); }
  /// returns the number of triangles in the sprite
  int GetTriangleCount () const { return (int)texel_mesh->GetTriangleCount (); }
  /// Size triangle buffer size
  void SetTriangleCount (int count) { texel_mesh->SetSize (count); }
  /// Set a bank of triangles.  The bank is copied.
  void SetTriangles (csTriangle const* trig, int count)
  { texel_mesh->SetTriangles(trig, count); }

  /// Create and add a new frame to the sprite.
  csSpriteFrame* AddFrame ();
  /// find a named frame into the sprite.
  csSpriteFrame* FindFrame (const char * name);
  /// Query the number of frames
  int GetFrameCount () const { return (int)frames.Length (); }
  /// Query the frame number f
  csSpriteFrame* GetFrame (int f) const
  {
    return ((size_t)f < frames.Length ())
  	? (csSpriteFrame *)frames [f]
	: (csSpriteFrame*)0;
  }

  /// Create and add a new action frameset to the sprite.
  csSpriteAction2* AddAction ();
  /// find a named action into the sprite.
  csSpriteAction2* FindAction (const char * name) const;
  /// Get the first action.
  csSpriteAction2* GetFirstAction () const
  { return (csSpriteAction2 *)actions [0]; }
  /// Get number of actions in sprite
  int GetActionCount () const
  { return (int)actions.Length (); }
  /// Get action number No
  csSpriteAction2* GetAction (int No) const
  { return (csSpriteAction2 *)actions [No]; }

  /// Create and add a new socket to the sprite.
  csSpriteSocket* AddSocket ();
  /// find a named socket into the sprite.
  csSpriteSocket* FindSocket (const char * name) const;
  /// find a socked based on the sprite attached to it
  csSpriteSocket* FindSocket (iMeshWrapper *mesh) const;
  /// Query the number of sockets
  int GetSocketCount () const { return (int)sockets.Length (); }
  /// Query the socket number f
  csSpriteSocket* GetSocket (int f) const
  {
    return ((size_t)f < sockets.Length ())
  	? (csSpriteSocket *)sockets [f]
	: (csSpriteSocket*)0;
  }


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

  void SetMixMode (uint mode) { MixMode = mode; }
  uint GetMixMode () const { return MixMode; } 

  /// For LOD.
  int GetLODPolygonCount (float lod) const;
  /// Default LOD level for this factory.
  void GetLod (float& m, float& a) const
  {
    m = lod_m;
    a = lod_a;
  }
  void ClearLODListeners ();
  void SetupLODListeners (iSharedVariable* varm, iSharedVariable* vara);

  void GetObjectBoundingBox (csBox3& bbox);
  void SetObjectBoundingBox (const csBox3& bbox);
  void GetRadius (csVector3& rad, csVector3 &cent);

  //------------------------ iMeshObjectFactory implementation --------------
  SCF_DECLARE_IBASE;

  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> NewInstance ();
  virtual csPtr<iMeshObjectFactory> Clone () { return 0; }
  virtual void HardTransform (const csReversibleTransform& t);
  virtual bool SupportsHardTransform () const { return true; }
  virtual void SetLogicalParent (iBase* lp) { logparent = lp; }
  virtual iBase* GetLogicalParent () const { return logparent; }
  virtual iMeshObjectType* GetMeshObjectType () const { return spr3d_type; }

  //------------------ iPolygonMesh interface implementation ----------------//
  struct PolyMesh : public iPolygonMesh
  {
  private:
    csSprite3DMeshObjectFactory* factory;
    csFlags flags;

  public:
    SCF_DECLARE_IBASE;

    void SetFactory (csSprite3DMeshObjectFactory* Factory)
    {
      factory = Factory;
    }

    virtual int GetVertexCount ()
    {
      return factory->GetVertexCount ();
    }
    virtual csVector3* GetVertices ()
    {
      return factory->GetVertices (0);
    }
    virtual int GetPolygonCount ()
    {
      return factory->GetTriangleCount ();
    }

    virtual csMeshedPolygon* GetPolygons ();

    virtual int GetTriangleCount ()
    {
      return factory->GetTriangleCount ();
    }
    virtual csTriangle* GetTriangles ()
    {
      return factory->GetTriangles ();
    }

    virtual void Lock () { } //PM@@@
    virtual void Unlock () { }
 
    virtual csFlags& GetFlags () { return flags;  }
    virtual uint32 GetChangeNumber() const { return 0; }

    PolyMesh () : polygons (0)
    {
      SCF_CONSTRUCT_IBASE (0);
      flags.Set (CS_POLYMESH_TRIANGLEMESH);
    }
    virtual ~PolyMesh ()
    {
      Cleanup ();
      SCF_DESTRUCT_IBASE ();
    }
    void Cleanup () { delete[] polygons; polygons = 0; }

    csMeshedPolygon* polygons;
  } scfiPolygonMesh;
  friend struct PolyMesh;

  //------------------------- iObjectModel implementation ----------------
  class ObjectModel : public csObjectModel
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSprite3DMeshObjectFactory);
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
      csRef<iSpriteFrame> ifr (
      	SCF_QUERY_INTERFACE_SAFE (scfParent->AddFrame (),
      	iSpriteFrame));
      return ifr;	// DecRef is ok here.
    }
    virtual iSpriteFrame* FindFrame (const char* name) const
    {
      csRef<iSpriteFrame> ifr (SCF_QUERY_INTERFACE_SAFE (
      	scfParent->FindFrame (name), iSpriteFrame));
      return ifr;	// DecRef is ok here.
    }
    virtual int GetFrameCount () const
    {
      return scfParent->GetFrameCount ();
    }
    virtual iSpriteFrame* GetFrame (int f) const
    {
      csRef<iSpriteFrame> ifr (
      	SCF_QUERY_INTERFACE_SAFE (scfParent->GetFrame (f),
      	iSpriteFrame));
      return ifr;	// DecRef is ok here.
    }
    virtual iSpriteAction* AddAction ()
    {
      csRef<iSpriteAction> ia (
      	SCF_QUERY_INTERFACE_SAFE (scfParent->AddAction (),
      	iSpriteAction));
      return ia;	// DecRef is ok here.
    }
    virtual iSpriteAction* FindAction (const char* name) const
    {
      csRef<iSpriteAction> ia (SCF_QUERY_INTERFACE_SAFE (
      	scfParent->FindAction (name), iSpriteAction));
      return ia;	// DecRef is ok here.
    }
    virtual iSpriteAction* GetFirstAction () const
    {
      csRef<iSpriteAction> ia (SCF_QUERY_INTERFACE_SAFE (
      	scfParent->GetFirstAction (), iSpriteAction));
      return ia;	// DecRef is ok here.
    }
    virtual int GetActionCount () const
    {
      return scfParent->GetActionCount ();
    }
    virtual iSpriteAction* GetAction (int No) const
    {
      csRef<iSpriteAction> ia (
      	SCF_QUERY_INTERFACE_SAFE (scfParent->GetAction (No),
      	iSpriteAction));
      return ia;	// DecRef is ok here.
    }
    virtual iSpriteSocket* AddSocket ()
    {
      csRef<iSpriteSocket> ifr (
      	SCF_QUERY_INTERFACE_SAFE (scfParent->AddSocket (),
      	iSpriteSocket));
      return ifr;	// DecRef is ok here.
    }
    virtual iSpriteSocket* FindSocket (const char* name) const
    {
      csRef<iSpriteSocket> ifr (SCF_QUERY_INTERFACE_SAFE (
      	scfParent->FindSocket (name), iSpriteSocket));
      return ifr;	// DecRef is ok here.
    }
    virtual iSpriteSocket* FindSocket (iMeshWrapper* mesh) const
    {
      csRef<iSpriteSocket> ifr (SCF_QUERY_INTERFACE_SAFE (
      	scfParent->FindSocket (mesh), iSpriteSocket));
      return ifr;	// DecRef is ok here.
    }
    virtual int GetSocketCount () const
    {
      return scfParent->GetSocketCount ();
    }
    virtual iSpriteSocket* GetSocket (int f) const
    {
      csRef<iSpriteSocket> ifr (
      	SCF_QUERY_INTERFACE_SAFE (scfParent->GetSocket (f),
      	iSpriteSocket));
      return ifr;	// DecRef is ok here.
    }
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
    virtual void SetMixMode (uint mode)
    { scfParent->SetMixMode (mode); }
    virtual uint GetMixMode () const
    { return scfParent->GetMixMode (); }
  } scfiSprite3DFactoryState;

  //--------------------- iLODControl implementation -------------//
  struct LODControl : public iLODControl
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSprite3DMeshObjectFactory);
    virtual void SetLOD (float m, float a)
    {
      scfParent->ClearLODListeners ();
      scfParent->lod_m = m;
      scfParent->lod_a = a;
    }
    virtual void GetLOD (float& m, float& a) const
    {
      m = scfParent->lod_m;
      a = scfParent->lod_a;
    }
    virtual void SetLOD (iSharedVariable* varm, iSharedVariable* vara)
    {
      scfParent->SetupLODListeners (varm, vara);
      scfParent->lod_m = varm->Get ();
      scfParent->lod_a = vara->Get ();
    }
    virtual void GetLOD (iSharedVariable*& varm, iSharedVariable*& vara) const
    {
      varm = scfParent->lod_varm;
      vara = scfParent->lod_vara;
    }
    virtual int GetLODPolygonCount (float lod) const
    {
      return scfParent->GetLODPolygonCount (lod);
    }
  } scfiLODControl;
  friend struct LODControl;
};

/**
 * A 3D sprite based on a triangle mesh with a single texture.
 * Animation is done with frames.
 */
class csSprite3DMeshObject : public iMeshObject
{
private:
  /// Set the size of internally used tables
  static void UpdateWorkTables (int max_size);
  iBase* logparent;

public:
  /**
   * Configuration values for global LOD (function <code>m*distance+a</code>).
   */
  static float global_lod_m, global_lod_a;
  /**
   * It is also possible to use variables. If these are not 0 then the
   * lod values are taken from variables.
   */
  static csWeakRef<iSharedVariable> global_lod_varm;
  static csWeakRef<iSharedVariable> global_lod_vara;

  CS_LEAKGUARD_DECLARE(csSprite3DMeshObject);

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
   * Configuration values for an individuals LOD
   * (function <code>m*distance+a</code>).
   */
  float local_lod_m, local_lod_a;

  /**
   * It is also possible to use variables. If these are not 0 then the
   * lod values are taken from variables.
   */
  csRef<iSharedVariable> local_lod_varm;
  csRef<iSharedVariable> local_lod_vara;
  csRef<csSpriteLODListener> local_lod_varm_listener;
  csRef<csSpriteLODListener> local_lod_vara_listener;

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

  /**
   * Setting to manipulate sprite animation speed
   * The bigger the value is, the faster the sprite moves.
   */
  float speedfactor;

  /**
   * Setting to indicate if the animation should be played in an endless loop.
   */
  bool loopaction;

  /**
   * Setting to indicate if the animation should be stopped.
   */
  bool fullstop;

  /**
   * Each mesh must have its own individual socket assignments,
   * but the vector must be copied down from the factory at create time.
   */
  csPDelArray<csSpriteSocket> sockets;

  csFlags flags;

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
      case CS_SPR_LIGHT_GLOBAL:      return global_lighting_quality;
      case CS_SPR_LIGHT_TEMPLATE:    return factory->GetLightingQuality();
      case CS_SPR_LIGHT_LOCAL:       return local_lighting_quality;
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
  void SetLocalLightingQuality (int lighting_quality)
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
   * Get the lod settings relevant for this sprite.
   */
  void GetRelevantLodSettings (float& m, float& a) const
  {
    switch (lod_level_config)
    {
      case CS_SPR_LOD_GLOBAL:
        m = global_lod_m;
	a = global_lod_a;
	break;
      case CS_SPR_LOD_LOCAL:
        m = local_lod_m;
	a = local_lod_a;
	break;
      //case CS_SPR_LOD_TEMPLATE:
      default:
        factory->GetLod (m, a);
	break;
    }
  }

  /**
   * Returns the lod level used by this sprite.
   */
  float GetLodLevel (float distance) const
  {
    float m, a;
    GetRelevantLodSettings (m, a);
    return m * distance + a;
  }

  /// Return true if LOD is enabled.
  bool IsLodEnabled () const
  {
    float m, a;
    GetRelevantLodSettings (m, a);
    return ABS (m) > SMALL_EPSILON || ABS (a) < (1-SMALL_EPSILON);
  }

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

public:
  /**
   * A mesh which contains a number of triangles as generated
   * by the LOD algorithm. This is static since it will likely
   * change every frame anyway. We hold it static also since
   * we don't want to allocate it again every time.
   */
  CS_DECLARE_STATIC_CLASSVAR (mesh, GetLODMesh, csTriangleMesh)
private:

  /// Mixmode for the triangles/polygons of the sprite.
  uint MixMode;

  /**
   * Array of colors for the vertices. If not set then this
   * sprite does not have colored vertices.
   */
  csColor4* vertex_colors;
  /**
   * Base color that will be added to the sprite colors.
   */
  csColor4 base_color;

  /// The parent.
  csSprite3DMeshObjectFactory* factory;

  /// The material handle as returned by iTextureManager.
  csRef<iMaterialWrapper> cstxt;

  /// The current frame number.
  int cur_frame;
  /// The current action.
  csSpriteAction2* cur_action;
  /// Is action running in reverse?  This is either 1 or -1 depending.
  int frame_increment;

  /// The action to restore after running an override action
  csSpriteAction2* last_action;
  /// The loop setting to restore after running an override action
  bool last_loop;
  /// The speed setting to restore after running an override action
  float last_speed;
  /// The reversal setting to restore after an override action
  bool last_reverse;

  /// The last frame time action.
  csTicks last_time;
  /// The last frame position (used for displacement calcs).
  csVector3 last_pos;
  /// The last frame displacement left over from whole frame increment.
  float last_displacement;
  /// Animation tweening ratio:  next frame / this frame.
  float tween_ratio;

  /// Enable tweening.
  bool do_tweening;
  /// Enable single-step mode on actions
  bool single_step;

  /// Enable or disable lighting.
  bool do_lighting;

  ///
  bool force_otherskin;

  iMeshObjectDrawCallback* vis_cb;

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

  csRenderMeshHolderSingle rmHolder;  

  bool initialized;

  /**
   * Our vertex buffer.
   * This is only temporarily here. I suppose it is better to
   * origanize this so that the vertex buffer is shared accross several
   * sprites using the same factory. On the other hand there are lots
   * of frames. Do we create a vertex buffer for every frame?
   * @@@
   */

  csVector2* final_texcoords;
  csColor4* final_colors;
  csTriangle* final_triangles;
  csVector3* real_obj_verts;
  csVector3* real_tween_verts;
  csVector3* real_obj_norms;
  csVector3* real_tween_norms;
  
  int final_num_vertices;
  int final_num_triangles;

  csRef<iRenderBuffer> vertices;
  csRef<iRenderBuffer> normals;
  csRef<iRenderBuffer> texcoords;
  csRef<iRenderBuffer> colors;
  csRef<iRenderBuffer> indices;
  csRef<csRenderBufferHolder> bufferHolder;

  /// Setup this object.
  void SetupObject ();


private:
  /**
   * Update the lighting on this sprite.
   */
  void UpdateLighting (const csArray<iLight*>& lights, iMovable* movable);

  /**
   * High quality version of UpdateLighting() which recalculates
   * the distance between the light and every vertex.
   * This version can use tweening of the normals and vertices
   */
  void UpdateLightingHQ (const csArray<iLight*>& lights, iMovable* movable);

  /**
   * Low quality version of UpdateLighting() which only
   * calculates the distance once (from the center of the sprite.)
   * This method can use tweening of the normals.
   */
  void UpdateLightingLQ (const csArray<iLight*>& lights, iMovable* movable);

  /**
   * Low quality Fast version of UpdateLighting() which only
   * calculates the distance once (from the center of the sprite.)
   * This version can NOT use any tweening.
   */
  void UpdateLightingFast (const csArray<iLight*>& lights, iMovable* movable);

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

  /// Force a new material skin other than default
  void SetMaterial (iMaterialWrapper *material);

  /// Get the material for this sprite.
  iMaterialWrapper* GetMaterial () const { return cstxt; }

  /// Sets the mode that is used, when drawing that sprite.
  void SetMixMode (uint mode)
  {
    MixMode = mode;
    if (MixMode & CS_FX_ALPHA)
      base_color.alpha = 1.0 - float (MixMode & CS_FX_MASK_ALPHA) / 255.0;
    else
      base_color.alpha = 1.0;
  }


  /// Gets the mode that is used, when drawing that sprite.
  uint GetMixMode () const { return MixMode; }

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

  float GetTweenRatio(){return tween_ratio;}
  /// Set base color.
  void SetBaseColor (const csColor& col)
  {
    base_color.Set (col);
    if (MixMode & CS_FX_ALPHA)
      base_color.alpha = 1.0 - float (MixMode & CS_FX_MASK_ALPHA) / 255.0;
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
   * Go to the next frame depending on the current time in milliseconds OR
   * depending on the distance displacement from the last render.
   */
  bool OldNextFrame (csTicks current_time, const csVector3& new_pos, 
    bool onestep = false, bool stoptoend = false);

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
   * Select an action animation by name.
   */
  bool SetAction (const char * name, bool loop = true, float speed = 1)
  {
    csSpriteAction2 *act;
    if ((act = factory->FindAction (name)) != 0)
      return SetAction (act,loop,speed);
    else
      return false;
  }

  /**
   * Select an action animation by index.
   */
  bool SetAction (int index, bool loop = true, float speed = 1)
  {
    csSpriteAction2 *act;
    if ((act = factory->GetAction(index)) != 0)
      return SetAction (act,loop,speed);
    else
      return false;
  }

  /**
   * Internal function to actually set the action.  External users should
   * not need to call this version.
   */
  bool SetAction (csSpriteAction2 *act, bool loop = true, float speed = 1)
  {
    speedfactor = speed;
    // If SetAction is called while an OverrideAction is in progress,
    // this 0 overrides the override, so this action will keep going.
    last_action = 0;

    loopaction = loop;
    fullstop = false;
    single_step = false;
    SetReverseAction(false); // always go forward by default.
    if (act != 0)
    {
      cur_action = act;
      SetFrame (0);
      last_time = factory->vc->GetCurrentTicks ();
      return true;
    }
    return false;
  }

  void SetReverseAction (bool reverse)
  { 
    frame_increment = (reverse) ? -1:1;
    if (cur_action)
	cur_action->SetReverseAction(reverse);
  }

  bool GetReverseAction () const
  {
      return (frame_increment < 0);
  }

  void SetSingleStepAction(bool singlestep)
  { single_step = singlestep; }

  bool SetOverrideAction (const char *name,float speed = 1)
  {
    csSpriteAction2* save_last;
    
    save_last    = cur_action;
    last_loop    = loopaction;
    last_speed   = speedfactor;
    last_reverse = (frame_increment==-1)?true:false;

    bool flag = SetAction (name,false,speed);
    last_action = save_last;
    return flag;
  }

  bool SetOverrideAction (int index,float speed = 1)
  {
    csSpriteAction2* save_last;
    
    save_last    = cur_action;
    last_loop    = loopaction;
    last_speed   = speedfactor;
    last_reverse = (frame_increment==-1)?true:false;

    bool flag = SetAction (index,false,speed);
    last_action = save_last;
    return flag;
  }

  /**
   * Propogate set action to all children
   */
  virtual bool PropagateAction (const char *name)
  {
    // TODO:: Implement across children
    return SetAction(name);
  }

  /**
   * Gets the center of a sprite socket 
   */
  void GetSocketCenter(const char *name, csVector3 & center);

  /**
   * Initialize a sprite. This function is called automatically
   * from within 'load'. However you should call it directly
   * if you created the sprite on the fly (without 'load').
   */
  void InitSprite ();

  /**
   * Get an array of object vertices which is valid for the given frame.
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

  /// For LOD.
  int GetLODPolygonCount (float lod) const;
  void ClearLODListeners ();
  void SetupLODListeners (iSharedVariable* varm, iSharedVariable* vara);

  void GetObjectBoundingBox (csBox3& bbox);
  void SetObjectBoundingBox (const csBox3& bbox);
  void GetRadius (csVector3& rad, csVector3 &cent);

  /// Create and add a new socket to the sprite.
  csSpriteSocket* AddSocket ();
  /// find a named socket into the sprite.
  csSpriteSocket* FindSocket (const char * name) const;
  /// find a socked based on the sprite attached to it
  csSpriteSocket* FindSocket (iMeshWrapper *mesh) const;
  /// Query the number of sockets
  int GetSocketCount () const { return (int)sockets.Length (); }
  /// Query the socket number f
  csSpriteSocket* GetSocket (int f) const
  {
    return ((size_t)f < sockets.Length ())
  	? (csSpriteSocket *)sockets [f]
	: (csSpriteSocket*)0;
  }

  ///------------------------ iMeshObject implementation ----------------------
  SCF_DECLARE_IBASE;

  virtual iMeshObjectFactory* GetFactory () const
  {
    csRef<iMeshObjectFactory> ifact (SCF_QUERY_INTERFACE (factory,
    	iMeshObjectFactory));
    return ifact;	// DecRef is ok here.
  }
  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> Clone () { return 0; }
  virtual csRenderMesh **GetRenderMeshes (int &n, iRenderView* rview, 
    iMovable* movable, uint32 frustum_mask);
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
  virtual void NextFrame (csTicks current_time,const csVector3& new_pos)
  {   
    OldNextFrame (current_time, new_pos, single_step, !loopaction);
  }
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
  virtual bool HitBeamOutline (const csVector3& start, const csVector3& end,
    csVector3& intersect, float* pr);
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
    csVector3& intersect, float* pr, int* polygon_idx = 0);
  virtual void SetLogicalParent (iBase* lp) { logparent = lp; }
  virtual iBase* GetLogicalParent () const { return logparent; }

  //------------------ iPolygonMesh interface implementation ----------------//
  struct PolyMesh : public iPolygonMesh
  {
  private:
    csFlags flags;

  public:
    SCF_DECLARE_EMBEDDED_IBASE (csSprite3DMeshObject);

    virtual int GetVertexCount ()
    {
      csSprite3DMeshObjectFactory* fact = scfParent->GetFactory3D ();
      return fact->GetVertexCount ();
    }
    virtual csVector3* GetVertices ()
    {
      csSprite3DMeshObjectFactory* fact = scfParent->GetFactory3D ();
      return fact->GetVertices (0);
    }
    virtual int GetPolygonCount ()
    {
      csSprite3DMeshObjectFactory* fact = scfParent->GetFactory3D ();
      return fact->GetTriangleCount ();
    }

    virtual csMeshedPolygon* GetPolygons ();

    virtual int GetTriangleCount ()
    {
      csSprite3DMeshObjectFactory* fact = scfParent->GetFactory3D ();
      return fact->GetTriangleCount ();
    }
    virtual csTriangle* GetTriangles ()
    {
      csSprite3DMeshObjectFactory* fact = scfParent->GetFactory3D ();
      return fact->GetTriangles ();
    }

    virtual void Lock () { }
    virtual void Unlock () { }

    virtual csFlags& GetFlags () { return flags;  }
    virtual uint32 GetChangeNumber() const { return 0; }

    PolyMesh () : polygons (0)
    {
      flags.Set (CS_POLYMESH_TRIANGLEMESH);
    }
    virtual ~PolyMesh () { Cleanup (); }
    void Cleanup () { delete[] polygons; polygons = 0; }

    csMeshedPolygon* polygons;
  } scfiPolygonMesh;
  friend struct PolyMesh;

  virtual iObjectModel* GetObjectModel () { return factory->GetObjectModel (); }

  virtual bool SetColor (const csColor& col)
  {
    SetBaseColor (col);
    return true;
  }
  virtual bool GetColor (csColor& col) const
  {
    GetBaseColor (col);
    return true;
  }
  virtual bool SetMaterialWrapper (iMaterialWrapper* mat)
  {
    SetMaterial (mat);
    return true;
  }
  virtual iMaterialWrapper* GetMaterialWrapper () const
  {
    return GetMaterial ();
  }
  virtual void InvalidateMaterialHandles () { }
  /**
   * see imesh/object.h for specification. The default implementation
   * does nothing.
   */
  virtual void PositionChild (iMeshObject* child,csTicks current_time);

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
    virtual void SetMixMode (uint mode)
    {
      scfParent->SetMixMode (mode);
    }
    virtual uint GetMixMode () const
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
    virtual bool SetAction (const char * name, bool loop = true,
    	float speed = 1)
    {
      return scfParent->SetAction (name, loop, speed);
    }
    virtual bool SetAction (int index, bool loop = true,
    	float speed = 1)
    {
      return scfParent->SetAction (index, loop, speed);
    }
    virtual void SetReverseAction (bool reverse)
    {
      scfParent->SetReverseAction (reverse);
    }
    virtual bool GetReverseAction () const
    {
      return scfParent->GetReverseAction ();
    }
    virtual bool SetOverrideAction (const char *name,float speed = 1)
    {
      return scfParent->SetOverrideAction (name,speed);
    }
    virtual bool SetOverrideAction (int index,float speed = 1)
    {
      return scfParent->SetOverrideAction (index,speed);
    }
    virtual void SetSingleStepAction(bool singlestep)
    {
	scfParent->SetSingleStepAction(singlestep);
    }

    virtual bool PropagateAction (const char *name)
    {
      return scfParent->PropagateAction (name);
    }
    virtual iSpriteAction* GetCurAction () const
    {
      csRef<iSpriteAction> ia (
      	SCF_QUERY_INTERFACE_SAFE (scfParent->GetCurAction (),
      	iSpriteAction));
      return ia;	// DecRef is ok here.
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
      return scfParent->IsLodEnabled ();
    }
    virtual void SetBaseColor (const csColor& col)
    {
      scfParent->SetBaseColor (col);
    }
    virtual void GetBaseColor (csColor& col) const
    {
      scfParent->GetBaseColor (col);
    }
    virtual iSpriteSocket* AddSocket ()
    {
      csRef<iSpriteSocket> ifr (
      	SCF_QUERY_INTERFACE_SAFE (scfParent->AddSocket (),
      	iSpriteSocket));
      return ifr;	// DecRef is ok here.
    }
    virtual iSpriteSocket* FindSocket (const char* name) const
    {
      csRef<iSpriteSocket> ifr (SCF_QUERY_INTERFACE_SAFE (
      	scfParent->FindSocket (name), iSpriteSocket));
      return ifr;	// DecRef is ok here.
    }
    virtual iSpriteSocket* FindSocket (iMeshWrapper* mesh) const
    {
      csRef<iSpriteSocket> ifr (SCF_QUERY_INTERFACE_SAFE (
      	scfParent->FindSocket (mesh), iSpriteSocket));
      return ifr;	// DecRef is ok here.
    }
    virtual int GetSocketCount () const
    {
      return scfParent->GetSocketCount ();
    }
    virtual iSpriteSocket* GetSocket (int f) const
    {
      csRef<iSpriteSocket> ifr (
      	SCF_QUERY_INTERFACE_SAFE (scfParent->GetSocket (f),
      	iSpriteSocket));
      return ifr;	// DecRef is ok here.
    }
  } scfiSprite3DState;

  //--------------------- iLODControl implementation -------------//
  struct LODControl : public iLODControl
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSprite3DMeshObject);
    virtual void SetLOD (float m, float a)
    {
      scfParent->SetLodLevelConfig (CS_SPR_LOD_LOCAL);
      scfParent->ClearLODListeners ();
      scfParent->local_lod_varm = 0;
      scfParent->local_lod_vara = 0;
    }
    virtual void GetLOD (float& m, float& a) const
    {
      m = scfParent->local_lod_m;
      a = scfParent->local_lod_a;
    }
    virtual void SetLOD (iSharedVariable* varm, iSharedVariable* vara)
    {
      scfParent->SetLodLevelConfig (CS_SPR_LOD_LOCAL);
      scfParent->SetupLODListeners (varm, vara);
      scfParent->local_lod_m = varm->Get ();
      scfParent->local_lod_a = vara->Get ();
    }
    virtual void GetLOD (iSharedVariable*& varm, iSharedVariable*& vara) const
    {
      varm = scfParent->local_lod_varm;
      vara = scfParent->local_lod_vara;
    }
    virtual int GetLODPolygonCount (float lod) const
    {
      return scfParent->GetLODPolygonCount (lod);
    }
  } scfiLODControl;
  friend struct LODControl;

  //------------------ iRenderBufferAccessor implementation ------------
  class eiRenderBufferAccessor : public iRenderBufferAccessor
  {
  private:
    csSprite3DMeshObject* parent;

  public:
    CS_LEAKGUARD_DECLARE (eiRenderBufferAccessor);
    eiRenderBufferAccessor (csSprite3DMeshObject* p)
    {
      SCF_CONSTRUCT_IBASE (0);
      parent = p;
    }
    virtual ~eiRenderBufferAccessor ()
    {
      SCF_DESTRUCT_IBASE ();
    }
    SCF_DECLARE_IBASE;
    virtual void PreGetBuffer (csRenderBufferHolder* holder, csRenderBufferName buffer)
    {
      parent->PreGetBuffer (holder, buffer);
    }
  };
  friend class eiRenderBufferAccessor;
  csRef<eiRenderBufferAccessor> scfiRenderBufferAccessor;

  void PreGetBuffer (csRenderBufferHolder* holder, csRenderBufferName buffer);
};

/**
 * Sprite 3D type. This is the plugin you have to use to create instances
 * of csSprite3DMeshObjectFactory.
 */
class csSprite3DMeshObjectType : public iMeshObjectType
{
private:
  iObjectRegistry* object_reg;
  csRef<iVirtualClock> vc;
  iEngine* engine;

public:
  /// Constructor.
  csSprite3DMeshObjectType (iBase*);

  /// Destructor.
  virtual ~csSprite3DMeshObjectType ();

  bool Initialize (iObjectRegistry* p);

  //------------------------ iMeshObjectType implementation --------------
  SCF_DECLARE_IBASE;

  /// New Factory.
  virtual csPtr<iMeshObjectFactory> NewFactory ();

  //------------------- iConfig interface implementation -------------------
  struct csSprite3DConfig : public iConfig
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSprite3DMeshObjectType);
    virtual bool GetOptionDescription (int idx, csOptionDescription *option);
    virtual bool SetOption (int id, csVariant* value);
    virtual bool GetOption (int id, csVariant* value);
  } scfiConfig;
  friend struct csSprite3DConfig;

  //--------------------- iComponent interface implementation
  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSprite3DMeshObjectType);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize (p); }
  } scfiComponent;
  friend struct eiComponent;

  //--------------------- iLODControl implementation -------------//
  struct LODControl : public iLODControl
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSprite3DMeshObjectType);
    virtual void SetLOD (float m, float a)
    {
      csSprite3DMeshObject::global_lod_m = m;
      csSprite3DMeshObject::global_lod_a = a;
      csSprite3DMeshObject::global_lod_varm = 0;
      csSprite3DMeshObject::global_lod_vara = 0;
    }
    virtual void GetLOD (float& m, float& a) const
    {
      m = csSprite3DMeshObject::global_lod_m;
      a = csSprite3DMeshObject::global_lod_a;
    }
    virtual void SetLOD (iSharedVariable* varm, iSharedVariable* vara)
    {
      csSprite3DMeshObject::global_lod_varm = varm;
      csSprite3DMeshObject::global_lod_vara = vara;
      csSprite3DMeshObject::global_lod_m = varm->Get ();
      csSprite3DMeshObject::global_lod_a = vara->Get ();
    }
    virtual void GetLOD (iSharedVariable*& varm, iSharedVariable*& vara) const
    {
      varm = csSprite3DMeshObject::global_lod_varm;
      vara = csSprite3DMeshObject::global_lod_vara;
    }
    virtual int GetLODPolygonCount (float /*lod*/) const
    {
      return 0;
    }
  } scfiLODControl;
  friend struct LODControl;
};

#endif // __CS_SPR3D_H__
