#ifndef __CS_N_TERRAIN_RENDERER__
#define __CS_N_TERRAIN_RENDERER__

#include "csutil/scf.h"
#include "csgeom/sphere.h"
#include "csgeom/transfrm.h"
#include "csgeom/objmodel.h"
#include "csutil/mmapio.h"
#include "csutil/garray.h"
#include "csutil/cscolor.h"
#include "csutil/scanstr.h"
#include "csgfx/csrgbvct.h"
#include "iutil/vfs.h"
#include "iutil/comp.h"
#include "ivideo/graph3d.h"
#include "iengine/rview.h"
#include "iengine/lightmgr.h"
#include "imesh/terrbig.h"
#include <string.h>
#include <stdio.h>

struct iMeshObject;
struct iVertexBufferManager;
struct iImageIO;
struct iLight;

/******************************************************************************************
 
   Some design notes and rationalizations.

 1. I realize that there is a large redundancy of information in the disk-structure.  Part 
 of the reason behind that is that I either have to perform a lookup into the heightmap 
 using generated offsets, or store indexes into it. Since the heightmap needs to be stored 
 at all points anyhow, you could store a height and the variance together, and then just 
 look it up at runtime.  This bothers me, since the variance is different depending on the
 resolution of the block, and I can't see an easy wao of resolving coarse-resolution 
 variances.  I've decided to trade size for speed, thus the heightmap file is about 33% 
 larger than it absolutely needs to be.  However, nothing that can be stored needs to be 
 computed or looked up, so this should massively increase cache coherency.

 2. The algorithm basically follows "Visualization of Large Terrains Made Easy," except 
 that there are not two trees, but one, and they are not interleaved.  Also, I don't use
 his error metric.  Also, instead of generating parity lists, and triangle strips, I 
 generate one large mesh.  The detail levels may not be quite as fine-tuned, but they are 
 simple and fast.  

 ******************************************************************************************/

/// Calculates the binary logarithm of n
int ilogb (unsigned n) 
{
  int i = -1;
  while (n != 0) {
      ++ i;
      n >>= 1;
  }
  return i;
}


///////////////////////////////////////////////////////////////////////////////////////////////

/** This is a terrain block.  X and Y elements are generated dynamically 
 * each run, so they don't need storage. The constant nBlockLen should always be used, 
 * instead of sizeof, as the system may pad the structure. */
struct nBlock
{
  nBlock () {};
  nBlock (nBlock *b) : pos(b->pos), norm(b->norm), error(b->error), radius(b->radius) {}
  /// Position of Vertex
  csVector3 pos;
  /// Normal based on surrounding vertices
  csVector3 norm;
  /// Error parameter (including children)
  float error;
  /// radius of the block (including childreN)
  float radius;
};


////////////////////////////////////////////////////////////////////////////////////////////////

class nTerrainInfo
{
public:
  nTerrainInfo (iObjectRegistry *obj_reg);
  ~nTerrainInfo ();

  G3DTriangleMesh *GetMesh () { return mesh; }

  /// Starts the buffer with 2 copies of this vertex and sets parity
  void InitBuffer (const csVector3 &v, const csVector2 &t, const csColor &c, int p);
  /// Appends the vector, doubles up if parity is == to already stored
  void AddVertex (const csVector3 &v, const csVector2 &t, const csColor &c, int p);
  /// Appends a final vector onto the set (ignore parity):
  void EndBuffer (const csVector3 &v, const csVector2 &t, const csColor &c, iRenderView *rview, const csBox3& bbox);
 
private:
  void AddTriangle ();
  void ResizeVertices ();

  csRef<iGraphics3D> mG3D;

  /// Triangle meshes to draw for blocks - one mesh per texture
  G3DTriangleMesh *mesh;

  /// Pointer to the vertex buffer manager
  csRef<iVertexBufferManager> vbufmgr;
  /// Pointer to vertex buffer for this mesh.
  csRef<iVertexBuffer> vbuf;
  /// Buffer Counter;
  int bufcount;

  /// Keep track of tris, verts, tex, indexes, and color
  csTriangle *triangles;
  int triangle_count, triangle_size;
  bool triangle_parity;

  csVector3 *vertices;
  csVector2 *texels;
  csColor *colors;
  int vertex_count, vertex_size;
  int parity;
public: // TODO Fix this
  int num_lights;
  iLight **light_list;
};



////////////////////////////////////////////////////////////////////////////////////////////////


class nTerrain
{
  /// Maximum number of levels in this terrain 
  unsigned int max_levels;

  /// Error metric tolerance (that is, below this level, the error metric does not fail)
  float error_metric_tolerance;

  /// The info structure for the viewable terrain.
  nTerrainInfo *info;

  /// The source for the heightfield data, must have been created with BuildTree first.
  csMemoryMappedIO *hm;

  /// The width of the entire terrain, the height must be the same as this value.
  unsigned int terrain_w;

  /// The transform to get object to camera space.
  csOrthoTransform obj2cam;

  /// Stored movable
  iMovable* movable;

  /// This is the camera position, in object space.
  csVector3 cam;

  /// List of textures
  iMaterialWrapper **materials;
  
  /// Color mappings for terrain tile texturing. (RGB)
  csRGBVector rgb_colors;

  /// Color mappings for terrain tile texturing. (8-Bit palettized)
  //Disabled for now: not used: csArray<int> pal_colors;

  // Must be a power of two, otherwise things get stupid.
  int map_scale;

  // Map mode, whether we are looking up stuff in rgb_colors or pal_colors.
  int map_mode;
  
  /// Does the work of tree building, heightmap is the height data (0..1), w is the edge length of the heightmap, which must be square.
  void VerifyTreeNode(FILE *, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, nBlock *);
  void WriteTreeNode(FILE *, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, nBlock *, nBlock **);

  /// Calculates the insensity and color at a given vertex for a given light
  csColor CalculateLightIntensity (iLight *li, csVector3 v, csVector3 n);

  /// Buffers the node passed into it for later drawing, bounds are needed to generate all the verts.
  void BufferTreeNode(int p, nBlock *b);

  /// Processes a node for buffering, checks for visibility and detail levels.
  void ProcessTreeNode(iRenderView *rv, float kappa, unsigned int level, unsigned int parent_index, unsigned int child_num, unsigned int branch);
    
public:
  /// Sets the heightmap file
  void SetHeightMapFile (const char *filename) 
    {
    if (hm) { delete hm; }
    hm = new csMemoryMappedIO (sizeof (nBlock), (char *)filename);
    nBlock *b = (nBlock *)hm->GetPointer(0);
    if (!b) { return; }
    terrain_w = (unsigned int)b->radius;
    max_levels = ilogb(terrain_w) - 1;
    }
  /// Sets the error tolerance
  void SetErrorTolerance (float tolerance)
    {
	  error_metric_tolerance = tolerance;
    }
  /// Gets the width (and height) of the heightmap
  unsigned int GetWidth () { return terrain_w; }

  /** Builds a full-resolution quadtree terrain mesh object on disk, 
   *  heightmap is the data, w is the width and height (map must be square
   *  and MUST be a power of two + 1, e.g. 129x129, 257x257, 513x513.)
   */
  void BuildTree(FILE *f, nBlock *heightmap, unsigned int w);

  /// Assembles the terrain into the buffer when called by the engine.  
  void AssembleTerrain(iRenderView *rv, iMovable* m, nTerrainInfo *terrinfo,
  	const csBox3& bbox);

  /// Sets the object to camera transform
  void SetObjectToCamera(csReversibleTransform &o2c)
  { obj2cam = o2c; }

  /// Sets the camera origin
  void SetCameraOrigin(const csVector3 &camv)
  { cam=camv; }

  /// Set the materials list, copies the passed in list.
  void SetMaterialsList(iMaterialWrapper **matlist, unsigned int nMaterials);

  iMaterialWrapper **GetMaterialsList()
  {
    return materials;
  }

  ///////////////////////////////////////////////////////////////////////////////////

  /** Map materials to their colors in the image map.  The matmap is a handle to
   * an iFile that holds the text mapping information of color to material.  The 
   * terrtex is the image that contains the actual false-color map of texture 
   * data.  When the terrain file is built, this information will be encoded into
   * the file.  This function should only be called WHEN CREATING a terrain file.
   * When loading the terrain for render, call LoadMaterialMap(). 
   */
  void CreateMaterialMap(iFile *matmap, iImage *terrtex);

  nTerrain(csMemoryMappedIO *phm=0):max_levels(0), 
             error_metric_tolerance(2), 
	     info(0), hm(phm), materials(0), 
	     map_scale(0), map_mode(0) {}

  ~nTerrain()
  {
    delete hm;
    delete [] materials;
  }

};


//////////////////////////////////////////////// Mesh Object ///////////////////////////////////////////////////

/** 
 *  This is the big terrain object.  It lets you have terrains as large as your hard drive
 * will hold by using memory mapped i/o.  On Windows and Unix systems it should be pretty
 * fast.  On all others, it will resort to the software emulation, so it will be somewhat
 * slower.  
 */
class csBigTerrainObject : public iMeshObject
{
private:


  /// Logical parent
  iBase* logparent;

  /// Pointer to factory that created this object.
  iMeshObjectFactory *pFactory;

  /// Object registry reference
  iObjectRegistry* object_reg;

  /// Visible call-back
  csRef<iMeshObjectDrawCallback> vis_cb;

  csRef<iLightManager> light_mgr;

  /// Pointer to terrain object
  nTerrain *terrain;

  /// Render information structure
  nTerrainInfo *info;

  /// Number of textures
  unsigned short nTextures;

  /// scale factor
  csVector3 scale;

  csFlags flags;

protected:
  /// Initializes a mesh structure
  void InitMesh (nTerrainInfo *info);

public:
  ////////////////////////////// iTerrBigState implementation ///////////////////////////
  SCF_DECLARE_IBASE;

  struct eiTerrBigState : public iTerrBigState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csBigTerrainObject);
    virtual bool LoadHeightMapFile (const char *hm) 
    { return scfParent->LoadHeightMapFile (hm); }
	virtual void SetScaleFactor (const csVector3 &scale)
	{ scfParent->SetScaleFactor (scale); }
	virtual void SetErrorTolerance (float tolerance)
	{ scfParent->SetErrorTolerance (tolerance); }
    virtual bool ConvertImageToMapFile (iFile *input, iImageIO *imageio, const char *hm)
    { return scfParent->ConvertImageToMapFile (input, imageio, hm); }
    virtual bool ConvertArrayToMapFile (float *data, int width, const char *hm)
    { return scfParent->ConvertArrayToMapFile (data, width, hm); }
    virtual void SetMaterialsList(iMaterialWrapper **matlist, unsigned int nMaterials)
    { scfParent->SetMaterialsList(matlist, nMaterials); }
  } scfiTerrBigState;
  friend struct eiTerrBigState;

  ////////////////////////////// iObjectModel implementation ///////////////////////////

	struct eiObjectModel : public	csObjectModel
	{
		SCF_DECLARE_EMBEDDED_IBASE (csBigTerrainObject);
		virtual	void GetObjectBoundingBox( csBox3& bBBox,	int	iType	=	CS_BBOX_NORMAL )
		{
			scfParent->GetObjectBoundingBox(	bBBox, iType );
		}
		virtual	void GetRadius(	csVector3& rad,	csVector3& cent	)
		{
			scfParent->GetRadius( rad, cent );
		}
	}	scfiObjectModel;
  friend struct eiObjectModel;

  ////////////////////////////// iVertexBufferManagerClient implementation ///////////////////////////

	void ManagerClosing() {}

  struct eiVertexBufferManagerClient : public iVertexBufferManagerClient
  {
    SCF_DECLARE_EMBEDDED_IBASE( csBigTerrainObject );
    void ManagerClosing()
		{
			scfParent->ManagerClosing();
		}
  } scfiVertexBufferManagerClient;



  csBigTerrainObject(iObjectRegistry* _obj_reg, iMeshObjectFactory *_pFactory);
  virtual ~csBigTerrainObject();

  virtual bool LoadHeightMapFile (const char *hm);
  virtual void SetScaleFactor (const csVector3 &scale);
  virtual void SetErrorTolerance (float tolerance);
  virtual bool ConvertImageToMapFile (iFile *input, iImageIO *imageio, const char *hm);
  virtual bool ConvertArrayToMapFile (float *data, int width, const char *hm);
  virtual void ComputeLod (nBlock *heightmap, int i, int j, int di, int dj, int n, int width);

  /// Returns a pointer to the factory that made this.
  virtual iMeshObjectFactory* GetFactory () const { return pFactory; }

  /// Does some pre-draw work (buffers all vertices to be drawn, draw will render these.)
  virtual csFlags& GetFlags () { return flags; }
  virtual iMeshObject* Clone () { return 0; }
  virtual bool DrawTest (iRenderView* rview, iMovable* movable,
  	uint32 frustum_mask);
  virtual csRenderMesh** GetRenderMeshes (int& n, iRenderView*,
    iMovable*, uint32) { n = 0; return 0; }

  /// Update lighting on the terrain.
  void UpdateLighting (const csArray<iLight*>& lights, iMovable* movable);

  /// Draw the terrain.
  virtual bool Draw (iRenderView* rview, iMovable* movable, csZBufMode zbufMode);


  virtual void SetVisibleCallback (iMeshObjectDrawCallback* cb)
  {
    vis_cb = cb;
  }

  virtual iMeshObjectDrawCallback* GetVisibleCallback () const
  {
    return vis_cb;
  }

  /// Gets the bounding box for this terrain.
  virtual void GetObjectBoundingBox (csBox3& bbox, int type = CS_BBOX_NORMAL);

  /// Gets the radius and center of the terrain.
  virtual void GetRadius (csVector3& rad, csVector3& cent);

  /// For animation.
  virtual void NextFrame (csTicks, const csVector3& /*pos*/) { }

  /// We don't support this.
  virtual void HardTransform (const csReversibleTransform&) { }

  /// Note that we don't support hard transforming.
  virtual bool SupportsHardTransform () const { return false; }

  /// Set logical parent.
  virtual void SetLogicalParent (iBase* lp) { logparent = lp; }

  /// Get logical parent.
  virtual iBase* GetLogicalParent () const { return logparent; }

  /// Get object model
  virtual iObjectModel *GetObjectModel () { return &scfiObjectModel; }

  virtual bool SetColor (const csColor&) { return false; }
  virtual bool GetColor (csColor&) const { return false; }
  virtual bool SetMaterialWrapper (iMaterialWrapper*) { return false; }
  virtual iMaterialWrapper* GetMaterialWrapper () const { return 0; }
  virtual void InvalidateMaterialHandles () { }
  /**
   * see imesh/object.h for specification. The default implementation
   * does nothing.
   */
  virtual void PositionChild (iMeshObject* child,csTicks current_time) { }

  /// Get write object.
  virtual iPolygonMesh* GetWriteObject () { return 0; }

  /// Check if the terrain is hit by the given object space vector
  virtual bool HitBeamOutline (const csVector3& start, const csVector3& end, csVector3& isect, float* pr);

  /// Check exactly where the hit is.
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end, csVector3& isect, float* pr, int* = 0);

  ///////////////////////////////////////////////////////////////////

  /// Set the materials list, copies the passed in list.
  void SetMaterialsList(iMaterialWrapper **matlist, unsigned int nMaterials);
};


///////////////////////////////////////////////////////////////////////////////////////////

/**
 * Factory for Big Terrain Object.
 */
class csBigTerrainObjectFactory : public iMeshObjectFactory
{
private:
  iBase* logparent;
  csFlags flags;

public:
  iObjectRegistry *object_reg;

  /// Constructor.
  csBigTerrainObjectFactory (iBase *pParent, iObjectRegistry* object_reg);

  /// Destructor.
  virtual ~csBigTerrainObjectFactory ();

  SCF_DECLARE_IBASE;

  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> NewInstance ();

  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
  virtual void SetLogicalParent (iBase* lp) { logparent = lp; }
  virtual iBase* GetLogicalParent () const { return logparent; }
  virtual iObjectModel* GetObjectModel () { return 0; }
};

/**
 * Terrbig Type.  This is the plugin one uses to create the 
 * csBigTerrainMeshObjectFactory.
 */
class csBigTerrainObjectType : public iMeshObjectType
{
public:
  iObjectRegistry *object_reg;

  SCF_DECLARE_IBASE;

  /// Constructor
  csBigTerrainObjectType (iBase*);
  /// Destructor
  virtual ~csBigTerrainObjectType ();
  /// Create an instance of csBigTerrainObjectFactory
  virtual csPtr<iMeshObjectFactory> NewFactory ();
  /// Initialize
  bool Initialize (iObjectRegistry* oreg)
  {
    object_reg = oreg;
    return true;
  }
  
  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csBigTerrainObjectType);
    virtual bool Initialize (iObjectRegistry* object_reg)
    { return scfParent->Initialize (object_reg); }
  } scfiComponent;
};

#endif // __CS_N_TERRAIN_RENDERER__
