/*
    Copyright (C) 2001 by W.C.A. Wijngaards
    
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

#ifndef _HAZE_H_
#define _HAZE_H_

#include "csgeom/vector3.h"
#include "csgeom/transfrm.h"
#include "csutil/cscolor.h"
#include "csutil/csvector.h"
#include "imesh/object.h"
#include "imesh/haze.h"
#include "imesh/particle.h"
#include "ivideo/graph3d.h"
#include "iutil/config.h"
#include "isys/plugin.h"

struct iMaterialWrapper;
struct iCamera;
class csHazeMeshObjectFactory;

/** haze layer
*/
class csHazeLayer {
public:
  /// hull
  iHazeHull *hull;
  /// scale
  float scale;
  /// construct with values
  csHazeLayer(iHazeHull *h, float s) {hull=h; scale=s;}
  /// destroy (decref hull)
  ~csHazeLayer() {if(hull) hull->DecRef();}
};

/// a haze hull
class csHazeHull : iHazeHull {
protected:
  /// total counts
  int total_poly, total_vert, total_edge;
  /// array of vertices, [vert_idx]
  csVector3* verts;
  /// pt_1 and pt_2 of edges (the vertice_indices), [edge_idx]
  int *edgept1, *edgept2;
  /// number of vertices == number of edges per polygon
  int *pol_num;
  /** per polygon: the vertice indices [pol_idx][num]
   * use new[] and delete[] per pol_verts[i] as well as for pol_vert itself. 
   */
  int **pol_verts;
  /// per polygon the edge indices (for edges starting at that v, to v+1)
  int **pol_edges;

public:
  SCF_DECLARE_IBASE;
  /// create empty, new[] content by subclass.
  csHazeHull();
  /// destructs content, using delete[].
  virtual ~csHazeHull();

  /** 
   *  If already verts, total_vert, total_poly, pol_num and pol_verts are 
   *  given. This routine computes the edges, and fills
   *  edgept1, edgept2, pol_edges and total_edges.
   */
  void ComputeEdges();

  /**
   * given camera position will compute the clockwise convex outline
   * polygon for the hull.
   * if num_pts = 0, no polygon resulted.
   * array of vertice idx is new[]ed.
   * made static here, so not every custom iHazeHull has to implement it.
   */
  static void ComputeOutline(iHazeHull *hull, const csVector3& campos, 
    int& numv, int* pts);

  /// --------- iHazeHull implementation --------------------------
  virtual int GetPolygonCount() const {return total_poly;}
  virtual int GetVerticeCount() const {return total_vert;}
  virtual int GetEdgeCount() const {return total_edge;}

  virtual void GetVertex(csVector3& res, int vertex_idx) const
  { CS_ASSERT( vertex_idx >= 0 && vertex_idx < total_vert);
    res = verts[vertex_idx];}
  virtual void GetEdge(int edge_num, int& vertex_idx_1, int& vertex_idx_2) const
  { CS_ASSERT( edge_num >= 0 && edge_num < total_edge);
    vertex_idx_1 = edgept1[edge_num];
    vertex_idx_2 = edgept2[edge_num];
  }

  virtual int GetPolVerticeCount(int polygon_num) const
  { CS_ASSERT( polygon_num >= 0 && polygon_num < total_poly);
    return pol_num[polygon_num];
  }
  virtual int GetPolVertex(int polygon_num, int vertex_num) const
  { CS_ASSERT( polygon_num >= 0 && polygon_num < total_poly);
    CS_ASSERT( vertex_num >= 0 && vertex_num < pol_num[polygon_num]);
    return pol_verts[polygon_num][vertex_num];
  }
  virtual int GetPolEdge(int polygon_num, int vertex_num, int& start_idx,
    int& end_idx) const
  { CS_ASSERT( polygon_num >= 0 && polygon_num < total_poly);
    CS_ASSERT( vertex_num >= 0 && vertex_num < pol_num[polygon_num]);
    start_idx = pol_verts[polygon_num][vertex_num];
    int inc = (vertex_num+1)%pol_num[polygon_num];
    end_idx = pol_verts[polygon_num][inc];
    return pol_edges[polygon_num][vertex_num];
  }
};


/** box haze hull */
class csHazeHullBox : public csHazeHull {
  csVector3 min, max;
public:
  /// create new.
  csHazeHullBox(const csVector3& a, const csVector3& b);
  ///
  virtual ~csHazeHullBox();
  /// get settings
  void GetSettings(csVector3& a, csVector3& b) {a=min; b=max;}
};


/**
 * haze layer vector
 */
class csHazeLayerVector : public csVector
{
public:
  /// construct
  csHazeLayerVector(int ilimit = 8, int ithreshold = 16)
    : csVector(ilimit, ithreshold) {}
  /// destroy
  virtual ~csHazeLayerVector() {}
  /// delete correctly
  virtual bool FreeItem(csSome Item)
  { delete (csHazeLayer*)Item; return true; }
  /// get a layer
  csHazeLayer* GetLayer(int nr) {return (csHazeLayer*)Get(nr);}
};


/**
 * Haze mesh object.
 */
class csHazeMeshObject : public iMeshObject
{
 private:
  iMeshObjectFactory* ifactory;
  csHazeMeshObjectFactory* factory;

  iMaterialWrapper* material;
  UInt MixMode;
  bool initialized;
  iMeshObjectDrawCallback* vis_cb;
  csVector3 radius;
  long shapenr;
  float current_lod;
  uint32 current_features;

  /// Polygon.
  G3DPolygonDPFX g3dpolyfx;

  /// haze data
  csVector3 origin, directional;
  /// vector of csHazeLayer
  csHazeLayerVector layers;

  /**
   * Setup this object. This function will check if setup is needed.
   */
  void SetupObject ();

  /// Update lighting given a position.
  void UpdateLighting (iLight** lights, int num_lights, const csVector3& pos);

public:
  /// Constructor.
  csHazeMeshObject (csHazeMeshObjectFactory* factory);

  /// Destructor.
  virtual ~csHazeMeshObject ();

  /** draw (and clip) a polygon, give nr of vertices, arrays of vert, uv
   * pts are in screenspace (sx, sy, iz), uvs in texturespace (u, v)
   */
  void DrawPoly(iRenderView *rview, iGraphics3D *g3d, iMaterialHandle *mat,
    int num, const csVector3* pts, const csVector2* uvs);
  /** project a vertice in object space to screenspace */
  void ProjectO2S(csReversibleTransform& tr_o2c, float fov, float shiftx,
    float shifty, const csVector3& objpos, csVector3& scrpos);

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
  virtual bool HitBeamBBox (const csVector3&, const csVector3&)
  { return false; }
  virtual bool HitBeamOutline (const csVector3&, const csVector3&)
  { return false; }
  virtual bool HitBeamObject (const csVector3&, const csVector3&,
  	csVector3&, float*) { return false; }
  virtual long GetShapeNumber () const { return shapenr; }
  virtual uint32 GetLODFeatures () const { return current_features; }
  virtual void SetLODFeatures (uint32 mask, uint32 value)
  {
    mask &= 0;
    current_features = (current_features & ~mask) | (value & mask);
  }
  virtual void SetLOD (float lod) { current_lod = lod; }
  virtual float GetLOD () const { return current_lod; }
  virtual int GetLODPolygonCount (float /*lod*/) const
  {
    return 1;
  }

  //------------------------- iHazeState implementation ----------------
  class HazeState : public iHazeState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csHazeMeshObject);
    virtual void SetMaterialWrapper (iMaterialWrapper* material)
    {
      scfParent->material = material;
    }
    virtual iMaterialWrapper* GetMaterialWrapper () const { return scfParent->material; }
    virtual void SetMixMode (UInt mode) { scfParent->MixMode = mode; }
    virtual UInt GetMixMode () const { return scfParent->MixMode; }
    virtual void SetOrigin(const csVector3& pos) { scfParent->origin = pos; }
    virtual const csVector3& GetOrigin() const {return scfParent->origin;}
    virtual void SetDirectional(const csVector3& pos) 
    { scfParent->directional=pos;}
    virtual const csVector3& GetDirectional() const 
    {return scfParent->directional;}
    virtual int GetLayerCount() const {return scfParent->layers.Length();}
    virtual void AddLayer(iHazeHull *hull, float scale) 
    { csHazeLayer *lay = new csHazeLayer(hull, scale);
      scfParent->layers.Push(lay); }
    virtual void SetLayerHull(int layer, iHazeHull* hull) 
    { if(hull) hull->IncRef();
      if(scfParent->layers.GetLayer(layer)->hull)
        scfParent->layers.GetLayer(layer)->hull->DecRef();
      scfParent->layers.GetLayer(layer)->hull = hull;
    }
    virtual iHazeHull* GetLayerHull(int layer) const 
    { return scfParent->layers.GetLayer(layer)->hull; }
    virtual void SetLayerScale(int layer, float scale) 
    { scfParent->layers.GetLayer(layer)->scale = scale; }
    virtual float GetLayerScale(int layer) const 
    { return scfParent->layers.GetLayer(layer)->scale; }
  } scfiHazeState;
  friend class HazeState;
};

/**
 * Factory for 2D sprites. This factory also implements iHazeFactoryState.
 */
class csHazeMeshObjectFactory : public iMeshObjectFactory
{
 private:
  iMaterialWrapper* material;
  UInt MixMode;
  /// haze state info
  csVector3 origin, directional;
  /// vector of csHazeLayer
  csHazeLayerVector layers;
  
 public:
  /// Constructor.
  csHazeMeshObjectFactory (iBase *pParent);

  /// Destructor.
  virtual ~csHazeMeshObjectFactory ();

  /// Get the material for this 2D sprite.
  iMaterialWrapper* GetMaterialWrapper () const { return material; }
  /// Get mixmode.
  UInt GetMixMode () const { return MixMode; }
  
  //------------------------ iMeshObjectFactory implementation --------------
  SCF_DECLARE_IBASE;

  virtual iMeshObject* NewInstance ();
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }

  //------------------------- iHazeFactoryState implementation ----------------
  class HazeFactoryState : public iHazeFactoryState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csHazeMeshObjectFactory);
    virtual void SetMaterialWrapper (iMaterialWrapper* material)
    {
      scfParent->material = material;
    }
    virtual iMaterialWrapper* GetMaterialWrapper () const { return scfParent->material; }
    virtual void SetMixMode (UInt mode) { scfParent->MixMode = mode; }
    virtual UInt GetMixMode () const { return scfParent->MixMode; }
    virtual void SetOrigin(const csVector3& pos) { scfParent->origin = pos; }
    virtual const csVector3& GetOrigin() const {return scfParent->origin;}
    virtual void SetDirectional(const csVector3& pos) 
    { scfParent->directional=pos;}
    virtual const csVector3& GetDirectional() const 
    {return scfParent->directional;}
    virtual int GetLayerCount() const {return scfParent->layers.Length();}
    virtual void AddLayer(iHazeHull *hull, float scale) 
    { csHazeLayer *lay = new csHazeLayer(hull, scale);
      scfParent->layers.Push(lay); }
    virtual void SetLayerHull(int layer, iHazeHull* hull) 
    { if(hull) hull->IncRef();
      if(scfParent->layers.GetLayer(layer)->hull)
        scfParent->layers.GetLayer(layer)->hull->DecRef();
      scfParent->layers.GetLayer(layer)->hull = hull;
    }
    virtual iHazeHull* GetLayerHull(int layer) const 
    { return scfParent->layers.GetLayer(layer)->hull; }
    virtual void SetLayerScale(int layer, float scale) 
    { scfParent->layers.GetLayer(layer)->scale = scale; }
    virtual float GetLayerScale(int layer) const 
    { return scfParent->layers.GetLayer(layer)->scale; }
  } scfiHazeFactoryState;
  friend class HazeFactoryState;
};

/**
 * Sprite 2D type. This is the plugin you have to use to create instances
 * of csHazeMeshObjectFactory.
 */
class csHazeMeshObjectType : public iMeshObjectType
{
public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csHazeMeshObjectType (iBase*);
  /// Destructor.
  virtual ~csHazeMeshObjectType ();
  /// New Factory.
  virtual iMeshObjectFactory* NewFactory ();
  /// Get features.
  virtual uint32 GetFeatures () const
  {
    return 0;
  }

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csHazeMeshObjectType);
    virtual bool Initialize (iSystem*) { return true; }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
};

#endif // _HAZE_H_
