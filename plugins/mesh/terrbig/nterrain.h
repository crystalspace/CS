#ifndef __N_TERRAIN_RENDERER__
#define __N_TERRAIN_RENDERER__

#include "csutil/scf.h"
#include "csgeom/sphere.h"
#include "csgeom/transfrm.h"
#include "csutil/mmapio.h"
#include "csutil/garray.h"
#include "csutil/cscolor.h"
#include "csutil/scanstr.h"
#include "csutil/csvector.h"
#include "csutil/csrgbvct.h"
#include "iutil/vfs.h"
#include "iutil/comp.h"
#include "ivideo/graph3d.h"
#include "iengine/rview.h"
#include "igeom/objmodel.h"
#include "imesh/terrbig.h"
#include <string.h>
#include <stdio.h>

struct iMeshObject;
struct iVertexBufferManager;
struct iImageIO;

const int NTERRAIN_QUADTREE_ROOT=1;

/******************************************************************************************
 
   Some design notes and rationalizations.

 1. I realize that there is a large redundancy of information in the disk-structure.  Part 
 of the reason behind that is that I either have to perform a lookup into the heightmap 
 using generated offsets, or store indexes into it. Since the heightmap needs to be stored 
 at all points anyhow, you could store a height and the variance together, and then just 
 look it up at runtime.  This bothers me, since the variance is different depending on the
 resolution of the block, and I can't see an easy way of resolving coarse-resolution 
 variances.  I've decided to trade size for speed, thus the heightmap file is about 33% 
 larger than it absolutely needs to be.  However, nothing that can be stored needs to be 
 computed or looked up, so this should massively increase cache coherency.

 2. The algorithm basically follows "Visualization of Large Terrains Made Easy," except 
 that there are not two trees, but one, and they are not interleaved.  Also, I don't use
 his error metric.  Also, instead of generating parity lists, and triangle strips, I 
 generate one large mesh.  The detail levels may not be quite as fine-tuned, but they are 
 simple and fast.  

 ******************************************************************************************/


/// Simple rect structure for bounds.
struct nRect
{
  unsigned short x, y, w, h;

  /// Initializes the rect.
  nRect(unsigned short _x, unsigned short _y, unsigned short _w, unsigned short _h):
  x(_x), y(_y), w(_w), h(_h) {};
};



///////////////////////////////////////////////////////////////////////////////////////////////


/// The length of the nBlock structure
unsigned const nBlockLen=34;

typedef unsigned short ti_type;

/** This is a terrain block.  X and Y elements are generated dynamically 
 * each run, so they don't need storage. The constant nBlockLen should always be used, 
 * instead of sizeof, as the system may pad the structure. */
struct nBlock
{
  /// Height for vertices
  float ne, nw, se, sw, center;

  /// Variance for block
  float variance;

  /// Radius of block
  float radius;

  /// Middle height of the block
  float midh;

  /// Texture index
  ti_type ti;
};


////////////////////////////////////////////////////////////////////////////////////////////////


/// Growth factor for buffer for triangles
const unsigned int TriangleStackGrowBuffer=256;

/// Manages a stack of triangles.  This must be associated with some vertex array.
class nTriangleStack
{
  /// Number of triangle spots in buffer.
  unsigned int buffer_size;

  /// Number of actual triangles.
  unsigned int count;

  /// Buffer for triangles
  csTriangle *tribuf;

public:
  /// Initialize the buffer stuff
  nTriangleStack():buffer_size(0), count(0), tribuf(NULL) {};

  /// Destroy the buffer 
  ~nTriangleStack()
  { if (tribuf) delete [] tribuf; }

  /// Empty the stack
  void MakeEmpty()
  { count=0; }

  /// Push a triangle
  void Push(int i1, int i2, int i3)
  { 
    // Grow buffer if it needs it.
    if (count+1>=buffer_size)
    {
      if (tribuf)
      {
        csTriangle *temp = new csTriangle[buffer_size+TriangleStackGrowBuffer];

        memcpy(temp, tribuf, sizeof(csTriangle) * buffer_size);
        delete [] tribuf;

        tribuf=temp;
        buffer_size+=TriangleStackGrowBuffer;
      }
      else
      {
        tribuf = new csTriangle[TriangleStackGrowBuffer<<1];
        buffer_size=TriangleStackGrowBuffer<<1;

      }
    }

    // Push indexes
    tribuf[count].a=i1;
    tribuf[count].b=i2;
    tribuf[count].c=i3;
    
    // Increase triangle count
    ++count;
  }

};

struct nTerrainInfo
{
    /// Triangle meshes to draw for blocks - one mesh per texture
    G3DTriangleMesh *mesh;

    /// Keep track of tris, verts, tex, indexes, and color
    struct triangle_queue
    {
      CS_DECLARE_GROWING_ARRAY (triangles, csTriangle);
    } *triq;
    
    CS_DECLARE_GROWING_ARRAY (vertices, csVector3);
    CS_DECLARE_GROWING_ARRAY (texels, csVector2);
    CS_DECLARE_GROWING_ARRAY (tindexes, ti_type);
    CS_DECLARE_GROWING_ARRAY (colors, csColor);
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

  /// This is the camera position, in object space.
  csVector3 cam;

  /// List of textures
  iMaterialWrapper **materials;
  
  /// Color mappings for terrain tile texturing. (RGB)
  csRGBVector rgb_colors;

  /// Color mappings for terrain tile texturing. (8-Bit palettized)
  csVector  pal_colors;

  // Must be a power of two, otherwise things get stupid.
  int map_scale;

  // Map mode, whether we are looking up stuff in rgb_colors or pal_colors.
  int map_mode;
  
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

  /// Sets the variance and radius of a partially filled in block.
  void SetVariance(nBlock &b);

  /// Does the work of tree building, heightmap is the height data (0..1), w is the edge length of the heightmap, which must be square.
  float BuildTreeNode(FILE *f, unsigned int level, unsigned int parent_index, unsigned int child_num, nRect bounds, float *heightmap, unsigned int w);

  /// Buffers the node passed into it for later drawing, bounds are needed to generate all the verts.
  void BufferTreeNode(nBlock *b, nRect bounds);

  /// Processes a node for buffering, checks for visibility and detail levels.
  void ProcessTreeNode(iRenderView *rv, unsigned int level, unsigned int parent_index, unsigned int child_num, nRect bounds);
    
public:
  /// Sets the heightmap file
  void SetHeightMapFile (const char *filename) 
    {
    if (hm) { delete hm; }
    hm = new csMemoryMappedIO (nBlockLen, (char *)filename);
    }
  /** Builds a full-resolution quadtree terrain mesh object on disk, 
   *  heightmap is the data, w is the width and height (map must be square
   *  and MUST be a power of two + 1, e.g. 129x129, 257x257, 513x513.)
   */
  void BuildTree(FILE *f, float *heightmap, unsigned int w);

  /// Assembles the terrain into the buffer when called by the engine.  
  void AssembleTerrain(iRenderView *rv, nTerrainInfo *terrinfo);

  /// Sets the object to camera transform
  void SetObjectToCamera(csOrthoTransform &o2c)
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

  nTerrain(csMemoryMappedIO *phm=NULL):max_levels(0), 
			 /* this is 4 pixel accuracy on 800x600 */
             error_metric_tolerance(0.0025), 
	     info(NULL), hm(phm), materials(NULL), 
	     map_scale(0), map_mode(0) {}

  ~nTerrain()
  {
    if (hm) delete hm;
    if (materials) delete [] materials;
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
  /// Pointer to the vertex buffer manager
  iVertexBufferManager *vbufmgr;

  /// Pointer to vertex buffer for this mesh.
  iVertexBuffer *vbuf;

  /// Logical parent
  iBase* logparent;

  /// Pointer to factory that created this object.
  iMeshObjectFactory *pFactory;

  /// Object registry reference
  iObjectRegistry* object_reg;

  /// Visible call-back
  iMeshObjectDrawCallback* vis_cb;

  /// Pointer to terrain object
  nTerrain *terrain;

  /// Render information structure
  nTerrainInfo *info;

  /// Number of textures
  unsigned short nTextures;

  
protected:
  /// Creates and sets up a vertex buffer.
  void SetupVertexBuffer (iVertexBuffer *&vbuf1);

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
    virtual bool ConvertImageToMapFile (iFile *input, iImageIO *imageio, const char *hm)
    { return scfParent->ConvertImageToMapFile (input, imageio, hm); }
    virtual void SetMaterialsList(iMaterialWrapper **matlist, unsigned int nMaterials)
    { scfParent->SetMaterialsList(matlist, nMaterials); }
  } scfiTerrBigState;
  friend struct eiTerrBigState;

  ////////////////////////////// iObjectModel implementation ///////////////////////////

	struct eiObjectModel : public	iObjectModel
	{
		SCF_DECLARE_EMBEDDED_IBASE (csBigTerrainObject);
		virtual	long GetShapeNumber()	const	
		{	
			return scfParent->GetShapeNumber(); 
		}
		virtual	iPolygonMesh*	GetPolygonMesh() { return	NULL;	}
		virtual	iPolygonMesh*	GetSmallerPolygonMesh()	{	return NULL; }
		virtual	iPolygonMesh*	CreateLowerDetailPolygonMesh(	float	)	{	return NULL; }
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
  virtual bool ConvertImageToMapFile (iFile *input, iImageIO *imageio, const char *hm);

  /// Returns a pointer to the factory that made this.
  virtual iMeshObjectFactory* GetFactory () const { return pFactory; }

  /// Does some pre-draw work (buffers all vertices to be drawn, draw will render these.)
  virtual bool DrawTest (iRenderView* rview, iMovable* movable);

  /// Update lighting on the terrain.
  virtual void UpdateLighting (iLight** lights, int num_lights, iMovable* movable);

  /// Draw the terrain.
  virtual bool Draw (iRenderView* rview, iMovable* movable, csZBufMode zbufMode);


  virtual void SetVisibleCallback (iMeshObjectDrawCallback* cb)
  {
    SCF_SET_REF (vis_cb, cb);
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
  virtual void NextFrame (csTicks) { }

  /// For dieing.
  virtual bool WantToDie () const { return false; }

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

  /// Get write object.
  virtual iPolygonMesh* GetWriteObject () { return NULL; }

  /// Check if the terrain is hit by the given object space vector
  virtual bool HitBeamOutline (const csVector3& start, const csVector3& end, csVector3& isect, float* pr);

  /// Check exactly where the hit is.
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end, csVector3& isect, float* pr);

  /// This may eventually return a changing number.
  virtual long GetShapeNumber () const { return 1; }

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

public:
  iObjectRegistry *object_reg;

  /// Constructor.
  csBigTerrainObjectFactory (iBase *pParent, iObjectRegistry* object_reg);

  /// Destructor.
  virtual ~csBigTerrainObjectFactory ();

  SCF_DECLARE_IBASE;

  virtual iMeshObject* NewInstance ();

  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
  virtual void SetLogicalParent (iBase* lp) { logparent = lp; }
  virtual iBase* GetLogicalParent () const { return logparent; }
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
  virtual iMeshObjectFactory* NewFactory ();
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

#endif
