/*
    Copyright (C) 2003 by Jorrit Tyberghein, Daniel Duhprey

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

#ifndef __CS_CHUNKLOD_H__
#define __CS_CHUNKLOD_H__

#include "csutil/cscolor.h"

#include "csgeom/transfrm.h"
#include "csgeom/objmodel.h"

#include "imesh/object.h"
#include "imesh/terrain.h"

#include "iutil/comp.h"

#include "ivideo/rendermesh.h"

struct iMaterialWrapper;
struct iImage;
struct iGraphics3D;

class csChunkLodTerrainType;
class csChunkLodTerrainFactory;
class csChunkLodTerrainObject;


/**
 * ChunkLod terrain type, instantiates factories which create meshes
 */
class csChunkLodTerrainType : public iMeshObjectType
{
private:
  iObjectRegistry *object_reg;
  iBase* parent;

public:
  SCF_DECLARE_IBASE;	

  csChunkLodTerrainType (iBase* p);
  virtual ~csChunkLodTerrainType ();

  csPtr<iMeshObjectFactory> NewFactory();

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csChunkLodTerrainType);
    bool Initialize (iObjectRegistry* p)
    { scfParent->object_reg = p; return true; }
  } scfiComponent;
  friend struct eiComponent;
};


/**
 * The factory will keep track of the precomputed quad-tree hierarchy
 * each individual instance of the terrain (if there are more than one)
 * will determine which nodes in the tree are meant to be rendered 
 * (that determination is made during drawtest)
 */
class csChunkLodTerrainFactory : public iMeshObjectFactory
{
  friend class csChunkLodTerrainObject;
private:
  iBase* parent;
  iObjectRegistry *object_reg;

  csVector3 scale;
  struct Data {
    csVector3 pos;
    csVector3 norm;
    csVector3 tan;
    csVector3 bin;
    csVector2 tex;
    csColor col;
    float error;
  };
  csArray<Data> datamap;
  int hm_x, hm_y;

  csStringID vertex_name, normal_name, tangent_name, binormal_name, texcors_name, color_name, index_name;
  csRef<iGraphics3D> r3d;
  csRef<iShaderManager> shmgr;

  class MeshTreeNode : public iRenderBufferSource
  {
  private:
    csChunkLodTerrainFactory *pFactory;
    MeshTreeNode *children[4];

    csVector3 center;
    float radius; // x/z plane only
    float error;

    csArray<csVector3> vertices;
    csRef<iRenderBuffer> vertex_buffer;
    csArray<csVector3> normals;
    csRef<iRenderBuffer> normal_buffer;
    csArray<csVector3> tangents;
    csRef<iRenderBuffer> tangent_buffer;
    csArray<csVector3> binormals;
    csRef<iRenderBuffer> binormal_buffer;
    csArray<csVector2> texcors;
    csRef<iRenderBuffer> texcors_buffer;
    csArray<csColor> colors;
    csRef<iRenderBuffer> color_buffer;
    csRef<iRenderBuffer> index_buffer;

    int parity;
    int max_levels;
  
    void InitBuffer (const Data &d, int p);
    void AddVertex (const Data &d, int p);
    void EndBuffer (const Data &d, int p);
    void AddEdgeVertex (const Data& d);
    void AddSkirtVertex (const Data& d, const Data& mod);
    void ProcessMap (int l, int i, int j, int k);
    void ProcessEdge (int start, int end, int move, const Data& mod);
  
  public:
    SCF_DECLARE_IBASE;
  
    MeshTreeNode (csChunkLodTerrainFactory* p, int x, int y, int w, int h, float error);
    virtual ~MeshTreeNode ();

    MeshTreeNode *GetChild (int i) 
    { CS_ASSERT (i >= 0 && i < 4); return (error > 0) ? children[i] : 0; }

    iRenderBuffer *GetRenderBuffer (csStringID name);

    const csVector3 &Center () { return center; }
    float Radius () { return radius; }
    float Error () { return error; }

    int Count () { return vertices.Length(); }
  } *root;

  void ComputeError (int i, int j, int di, int dj, int n, int w);

public:
  SCF_DECLARE_IBASE;

  csChunkLodTerrainFactory (csChunkLodTerrainType* p, iObjectRegistry* objreg);
  virtual ~csChunkLodTerrainFactory ();

  csPtr<iMeshObject> NewInstance ();
  void HardTransform (const csReversibleTransform&) { }
  bool SupportsHardTransform () const { return false; }
  void SetLogicalParent (iBase* lp) { parent = lp; }
  iBase* GetLogicalParent () const { return parent; }
  iObjectModel* GetObjectModel () { return &scfiObjectModel; }

  void GetObjectBoundingBox (csBox3& bbox, int type = CS_BBOX_NORMAL);
  void GetRadius (csVector3& rad, csVector3& c);

  struct eiObjectModel : public csObjectModel
  {
    SCF_DECLARE_EMBEDDED_IBASE (csChunkLodTerrainFactory);
    void GetObjectBoundingBox (csBox3& b, int t = CS_BBOX_NORMAL)
    { scfParent->GetObjectBoundingBox (b, t); }
    void GetRadius (csVector3& r, csVector3& c)
    { scfParent->GetRadius (r, c); }
  } scfiObjectModel;
  friend struct eiObjectModel;

  void SetScale (const csVector3& scale);
  csVector3 GetScale ();
  bool SetHeightMap (const csArray<float>& data, int x, int y);
  bool SetHeightMap (iImage* map);
  csArray<float> GetHeightMap ();
  bool SaveState (const char *filename);
  bool RestoreState (const char *filename);

  struct eiTerrainFactoryState : public iTerrainFactoryState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csChunkLodTerrainFactory);
    void SetScale (const csVector3& scale)
    { scfParent->SetScale (scale); }
    csVector3 GetScale ()
    { return scfParent->GetScale (); }
    bool SetHeightMap (const csArray<float>& data, int x, int y)
    { return scfParent->SetHeightMap (data, x, y); }
    bool SetHeightMap (iImage* map)
    { return scfParent->SetHeightMap (map); }
    csArray<float> GetHeightMap ()
    { return scfParent->GetHeightMap (); }
    bool SaveState (const char *filename)
    { return scfParent->SaveState (filename); }
    bool RestoreState (const char *filename) 
    { return scfParent->RestoreState (filename); }
  } scfiTerrainFactoryState;
  friend struct eiTerrainFactoryState;

};


/**
 * Instance of an implementation of Thatcher Ulritch's Chunked LOD algorithm
 * for terrain rendering.  http://www.tulrich.com/geekstuff/chunklod.html
 * The factory is responsible for the preprocessing step on the quad-tree
 * hierarchy and each instance (this class) is responsible for determining
 * which of those nodes in the tree should be rendered (draw_test) and render
 * them (GetRenderMeshes)
 */
class csChunkLodTerrainObject : public iMeshObject
{
private:
  iBase* logparent;
  csChunkLodTerrainFactory* pFactory;
  iMeshObjectDrawCallback* vis_cb;

  csColor basecolor;
  csRef<iMaterialWrapper> matwrap;
  csArray<iMaterialWrapper*> palette;
  csRefArray<iImage> alphas;

  float error_tolerance;

  csArray<csRenderMesh> meshes;
  csArray< csArray<csRenderMesh> > palette_meshes;
  csRenderMesh **meshpp;
  int meshppsize;
  csReversibleTransform tr_o2c;
  int tricount;
public: 
  SCF_DECLARE_IBASE;

  csChunkLodTerrainObject (csChunkLodTerrainFactory* f);
  virtual ~csChunkLodTerrainObject ();

  /// Returns a point to the factory that made this
  iMeshObjectFactory* GetFactory () const { return (iMeshObjectFactory*)pFactory; }

  /**
   * Does all pre-render calculation.  Determines which LOD children in the 
   * tree should be drawn
   */
  bool DrawTestQuad (iRenderView* rv, 
	csChunkLodTerrainFactory::MeshTreeNode* node, float kappa);
  bool DrawTest (iRenderView* rview, iMovable* movable);

  /// Updates the lighting on the terrain
  void UpdateLighting (iLight** lights, int num_lights, iMovable* movable);
  
  bool Draw (iRenderView*, iMovable*, csZBufMode) 
  { /* deprecated */ return false; }

  /// Returns the mesh, ready for rendering
  csRenderMesh** GetRenderMeshes (int &n);

  void SetVisibleCallback (iMeshObjectDrawCallback* cb) { vis_cb = cb; }
  iMeshObjectDrawCallback* GetVisibleCallback () const { return vis_cb; }

  /// For animation ... ha ha
  void NextFrame (csTicks, const csVector3&) { }

  /// Unsupported
  void HardTransform (const csReversibleTransform&) { }

  /// Shows that HardTransform is not supported by this mesh
  bool SupportsHardTransform () const { return false; }

  /// Check if the terrain is hit by the beam
  bool HitBeamOutline (const csVector3& start, const csVector3& end, 
	csVector3& isect, float* pr);
  /// Find exact position of a beam hit
  bool HitBeamObject (const csVector3& start, const csVector3& end, 
	csVector3& isect, float* pr, int* polygon_idx = 0);

  /// Set/Get logical parent
  void SetLogicalParent (iBase* lp) { logparent = lp; }
  iBase* GetLogicalParent () const { return logparent; }

  /// Gets the objects model, not sure what this means yet
  iObjectModel *GetObjectModel () { return &scfiObjectModel; }

  /// Set (Get) the terrain to a constant base color 
  bool SetColor (const csColor& c) { basecolor = c; return true; }
  bool GetColor (csColor &c) const { c = basecolor; return true; }

  /** 
   * Set (Get) the terrain to a single material, useful only with 
   * large textures or small terrains (or terrains in the distance)
   * See TerrainState for better texture settings
   */
  bool SetMaterialWrapper (iMaterialWrapper* m)
  { matwrap = m; return true; }
  iMaterialWrapper* GetMaterialWrapper () const { return matwrap; }
  void InvalidateMaterialHandles () { }

  bool SetMaterialPalette (const csArray<iMaterialWrapper*>& pal);
  csArray<iMaterialWrapper*> GetMaterialPalette ();
  bool SetMaterialMap (csArray<char> data, int x, int y);
  bool SetMaterialMap (iImage* map);
  csArray<char> GetMaterialMap ();
  void SetErrorTolerance (float error) { error_tolerance = error; }
  float GetErrorTolerance () { return error_tolerance; }
  /// Saves the texture quad-tree into the file specified
  bool SaveState (const char *filename);
  bool RestoreState (const char *filename);

  struct eiTerrainObjectState : public iTerrainObjectState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csChunkLodTerrainObject);
    bool SetMaterialPalette (const csArray<iMaterialWrapper*>& pal) 
    { return scfParent->SetMaterialPalette (pal); }
    csArray<iMaterialWrapper*> GetMaterialPalette ()
    { return scfParent->GetMaterialPalette (); }
    bool SetMaterialMap (csArray<char> data, int x, int y)
    { return scfParent->SetMaterialMap (data, x, y); }
    bool SetMaterialMap (iImage* map)
    { return scfParent->SetMaterialMap (map); }
    csArray<char> GetMaterialMap ()
    { return scfParent->GetMaterialMap (); }
    void SetErrorTolerance (float error)
    { scfParent->SetErrorTolerance (error); }
    float GetErrorTolerance ()
    { return scfParent->GetErrorTolerance (); }
    bool SaveState (const char *filename)
    { return scfParent->SaveState (filename); }
    bool RestoreState (const char *filename) 
    { return scfParent->RestoreState (filename); }
  } scfiTerrainObjectState;
  friend struct eiTerrainObjectState;

  struct eiObjectModel : public csObjectModel
  {
    SCF_DECLARE_EMBEDDED_IBASE (csChunkLodTerrainObject);
    void GetObjectBoundingBox (csBox3& b, int t = CS_BBOX_NORMAL)
    { scfParent->pFactory->GetObjectBoundingBox (b, t); }
    void GetRadius (csVector3& r, csVector3& c)
    { scfParent->pFactory->GetRadius (r, c); }
  } scfiObjectModel;
  friend struct eiObjectModel;

};

#endif // __CS_CHNKLOD_H__
