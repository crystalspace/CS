#ifndef __N_TERRAIN_RENDERER__
#define __N_TERRAIN_RENDERER__

#include "csgeom/poly3d.h"
#include "csgeom/sphere.h"
#include "csutil/mmapio.h"
#include "bitarray.h"
#include <string.h>
#include <stdio.h>

struct iMeshObject;
struct iVertexBufferManager;

/*******************************************************************************************************************
 
   Some design notes and rationalizations.

 1. I realize that there is a large redundancy of information in the disk-structure.  Part of the reason behind that
 is that I either have to perform a lookup into the heightmap using generated offsets, or store indexes into it.
 Since the heightmap needs to be stored at all points anyhow, you could store a height and the variance together,
 and then just look it up at runtime.  This bothers me, since the variance is different depending on the resolution
 of the block, and I can't see an easy way of resolving coarse-resolution variances.  I've decided to trade size
 for speed, thus the heightmap file is about 33% larger than it absolutely needs to be.  However, nothing that can
 be stored needs to be computed or looked up, so this should massively increase cache coherency.

 2. The algorithm basically follows "Visualization of Large Terrains Made Easy," except that there are not two trees,
 but one, and they are not interleaved.  Also, I don't use his error metric.  Also, instead of generating parity lists,
 and triangle strips, I generate one large mesh.  The detail levels may not be quite as fine-tuned, but they are 
 simple and fast.  

 *******************************************************************************************************************/


/// Simple rect structure for bounds.
struct nRect
{
  unsigned short x, y, w, h;

  /// Initializes the rect.
  nRect(unsigned short _x, unsigned short _y, unsigned short _w, unsigned short _h):
  x(_x), y(_y), w(_w), h(_h) {};
};


////////////////////////////////////////////////////////////////////////////////////////////////


/// The length of the nBlock structure
unsigned const nBlockLen=16;

/** This is a terrain block.  X and Y elements are generated dynamically 
 * each run, so they don't need storage. The constant nBlockLen should always be used, 
 * instead of sizeof, as the system may pad the structure. */
struct nBlock
{
  /// Height for vertices
  unsigned short ne, nw, se, sw, center;

  /// Variance for block
  unsigned short variance;

  /// Radius of block
  unsigned short radius;

  /// Middle height of the block
  unsigned short midh;
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
  void Push(int i1, i2, i3)
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


////////////////////////////////////////////////////////////////////////////////////////////////


class nTerrain
{
  /// Maximum number of levels in this terrain 
  unsigned int max_levels;

  /// Error metric tolerance (that is, below this level, the error metric does not fail)
  float error_metric_tolerance;

  /// Array of vertices to render this frame.
  csVector3Array verts;

  /// Stack of triangles to render this frame.
  nTriangleStack tris;

  /// The source for the heightfield data, must have been created with BuildTree first.
  csMemoryMappedIO *hm;

  /// The width of the entire terrain, the height must be the same as this value.
  unsigned int terrain_w;

  /// The transform to get object to camera space.
  csReversibleTransform obj2cam;

  /// This is the camera position, in object space.
  csVector3 cam;


private:
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
  void SetVarianceAndRadius(nBlock &b, nRect &bounds)
  {
    unsigned short low=0xffff, unsigned high=0x0;
    unsigned short radius;

    if (b.ne<low)  low=b.ne;
    if (b.ne>high) high=b.ne;

    if (b.nw<low)  low=b.nw;
    if (b.nw>high) high=b.nw;

    if (b.se<low)  low=b.se;
    if (b.se>high) high=b.se;

    if (b.sw<low)  low=b.sw;
    if (b.sw>high) high=b.sw;

    if (b.center<low)  low=b.center;
    if (b.center>high) high=b.center;

    // Store variance
    b.variance = high-low;
    b.midh = low + b.variance;
    
    // Generate radius, start with variance/2.
    radius = (b.variance>>1);

    // Check and see if the width/2 is bigger.
    if (radius<w>>1) radius=width>>1;

    // Store radius
    b.radius = radius;
  }

  /// Does the work of tree building, heightmap is the height data (0..1), w is the edge length of the heightmap, which must be square.
  void BuildTreeNode(FILE *f, unsigned int level, unsigned int parent_index, unsigned int child_num, nRect bounds, float *heightmap, unsigned int w)
  {
    unsigned int my_index = (parent_index<<2) + child_num + root;
    unsigned int mid = (bounds.w>>1)+1;
    nBlock b;

    // Get heights.
    b.ne = heightmap[bounds.x +              (bounds.y * w)] * 65535;
    b.nw = heightmap[bounds.x + bounds.w +   (bounds.y * w)] * 65535;
    b.se = heightmap[bounds.x +              ((bounds.y + bounds.h) * w)] * 65535;
    b.sw = heightmap[bounds.x + bounds.w +   ((bounds.y + bounds.h) * w)] * 65535;
    b.center = heightmap[bounds.x +  mid  +  ((bounds.y + mid) * w)] * 65535;

    // Set the variance for this block (the difference between the highest and lowest points.)
    SetVarianceAndRadius(b, bounds);

    // Store the block in the file.
    fseek(f, my_index*nBlockLen, SEEK_SET);
    fwrite(&b, nBlockLen, 1, f);

    // Expand the quadtree until we get to the max resolution.
    if (level<max_levels)
    {
      BuildTreeNode(f, level+1, my_index, 0, nRect(bounds.x,y,mid,mid), heightmap, w);
      BuildTreeNode(f, level+1, my_index, 1, nRect(bounds.x+mid,bounds.y,mid,mid), heightmap, w);
      BuildTreeNode(f, level+1, my_index, 2, nRect(bounds.x,bounds.y+mid,mid,mid), heightmap, w);
      BuildTreeNode(f, level+1, my_index, 3, nRect(bounds.x+mid,bounds.y+mid,mid,mid), heightmap, w);
    }
  }

  /// Buffers the node passed into it for later drawing, bounds are needed to generate all the verts.
  void BufferTreeNode(nBlock *b, nRect bounds)
  {
        
    int ne = verts.AddVertexSmart(bounds.x, b->ne, bounds.y),
        nw = verts.AddVertexSmart(bounds.x, b->ne, bounds.y),
        se = verts.AddVertexSmart(bounds.x, b->ne, bounds.y),
        sw = verts.AddVertexSmart(bounds.x, b->ne, bounds.y),
        center = verts.AddVertexSmart(bounds.x, b->ne, bounds.y);

    tris.Push(se, center, sw);
    tris.Push(sw, center, nw);
    tris.Push(nw, center, ne);
    tris.Push(ne, center, se);
  }

  /// Processes a node for buffering, checks for visibility and detail levels.
  void ProcessTreeNode(iRenderView *rv, unsigned int level, unsigned int parent_index, unsigned int child_num, nRect bounds)
  {
    unsigned int my_index = (parent_index<<2) + child_num + root;
    unsigned int mid = (bounds.w>>1)+1;
    bool render_this=false;
    nBlock *b;
    
    // Get the block we're currently checking.
    b=(nBlock *)hm->GetPointer(my_index);

    // Create a bounding sphere.
    csSphere bs(csVector3(bounds.x+mid, b->midh, bounds.y+mid), b->radius);

    // Test it for culling, return if it's not visible.
    if (!rv->TestBSphere(obj2cam, bs)) return;

    // If we are at the bottom level, we HAVE to render this block, so don't bother with the calcs.
    if (level<max_levels)
      render_this=true;

    else
    {
      // Get distance from center of block to camera, plus a small epsilon to avoid division by zero.
      float distance = ((cam-bs.GetCenter()).SquaredNorm())+0.0001;

      // Get the error metric, in this case it's the ratio between variance and distance.
      float error_metric = STATIC_CAST(float, b->variance) / distance;

      if (error_metric<=error_metric_tolerance)
        render_this=true;
    }

    // Don't render this block, resolve to the next level.
    if (!render_this)
    {
      ProcessTreeNode(rv, level+1, my_index, 0, nRect(x,y,mid,mid));
      ProcessTreeNode(rv, level+1, my_index, 1, nRect(x+mid,y,mid,mid));
      ProcessTreeNode(rv, level+1, my_index, 2, nRect(x,y+mid,mid,mid));
      ProcessTreeNode(rv, level+1, my_index, 3, nRect(x+mid,y+mid,mid,mid));
    }
    // Render this block to the buffer for later drawing.
    else
      BufferTreeNode(b, bounds);

  }

public:
  /** Builds a full-resolution quadtree terrain mesh object on disk, 
   *  heightmap is the data, w is the width and height (map must be square
   *  and MUST be a power of two + 1, e.g. 129x129, 257x257, 513x513.)
   */
  void BuildTree(FILE*f, float *heightmap, unsigned int w)
  {
    max_levels = ilogb(w-1);

    unsigned int mid = (w>>1)+1;
    unsigned int x=0, y=0;

    BuildTreeNode(f, 1, 0, 0, nRect(x,y,mid,mid), heightmap, w);
    BuildTreeNode(f, 1, 0, 1, nRect(x+mid,y,mid,mid), heightmap, w);
    BuildTreeNode(f, 1, 0, 2, nRect(x,y+mid,mid,mid), heightmap, w);
    BuildTreeNode(f, 1, 0, 3, nRect(x+mid,y+mid,mid,mid), heightmap, w);
  }

  /** Assembles the terrain into the buffer when called by the engine.  
   */
  void AssembleTerrain(iRenderView *rv)
  {
    // Clear mesh lists
    verts.MakeEmpty();
    tris.MakeEmpty();

    
    //  Buffer entire viewable terrain by first doing view culling on the block, then checking for the
    // error metric.  If the error metric fails, then we need to drop down another level. Begin that
    // process here.
    
    ProcessTreeNode(rv, 1, 0, 0, nRect(x,y,mid,mid));
    ProcessTreeNode(rv, 1, 0, 1, nRect(x+mid,y,mid,mid));
    ProcessTreeNode(rv, 1, 0, 2, nRect(x,y+mid,mid,mid));
    ProcessTreeNode(rv, 1, 0, 3, nRect(x+mid,y+mid,mid,mid));
  }

public:
  /// Sets the object to camera transform
  void SetObjectToCamera(csReversibleTransform &o2c)
  { obj2cam = o2c; }

  /// Sets the camera origin
  void SetCameraOrigin(csVector3 &camv)
  { cam=camv; }
};

class csBigTerrainObject : public iMeshObject
{
private:
  /// Pointer to the vertex buffer manager
  iVertexBufferManager *vbufmgr;

  /// Pointer to vertex buffer for this mesh.
  iVertexBuffer *vbuf;

  /// Pointer to factory that created this object.
  iMeshObjectFactory *pFactory;

  /// Pointer to terrain object
  nTerrain *terrain;

protected:
  void SetupVertexBuffer (iVertexBuffer *&vbuf1);

public:
  ////////////////////////////// iMeshObject implementation ///////////////////////////
  SCF_DECLARE_IBASE;

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

  /// Check if the terrain is hit by the given object space vector
  virtual bool HitBeamOutline (const csVector3& start, const csVector3& end, csVector3& isect, float* pr);

  /// Check exactly where the hit is.
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end, csVector3& isect, float* pr);

  /// This may eventually return a changing number.
  virtual long GetShapeNumber () const { return 1; }
};


#endif