/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
    Copyright (C) 1998 by Dan Ogles.

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

#ifndef __GL_TEXTURECACHE_H__
#define __GL_TEXTURECACHE_H__

#include "csutil/scf.h"
#include "csgeom/csrect.h"
#include "ivideo/graph3d.h"
#include "ogl_polybuf.h"

#if defined(CS_OPENGL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENGL_PATH,gl.h)
#else
#include <GL/gl.h>
#endif

struct iLightMap;
struct iTextureHandle;
struct iPolygonTexture;
struct iObjectRegistry;
class csSubRectangles;
class csGraphics3DOGLCommon;

/**
 * Cache element for a texture. This element will be stored
 * in the OpenGL texture cache and is also kept with the polygon
 * itself.
 */
struct csTxtCacheData
{
  /// The size of this texture.
  long Size;
  /// iTextureHandle.
  iTextureHandle* Source;
  /// GL texture handle.
  GLuint Handle;
  /// Linked list.
  csTxtCacheData *next, *prev;
};

/**
 * This is the OpenGL texture cache.
 */
class OpenGLTextureCache
{
private:
  csGraphics3DOGLCommon* g3d;
protected:
  bool rstate_bilinearmap;

  /// the head and tail of the cache data
  csTxtCacheData *head, *tail;

  /// the maximum size of the cache
  long cache_size;
  /// number of items
  int num;
  /// Total size of all loaded textures
  long total_size;
public:
  /// Takes the maximum size of the cache.
  OpenGLTextureCache (int max_size, csGraphics3DOGLCommon* g3d);
  ///
  ~OpenGLTextureCache ();

  /// Make sure this texture is active in OpenGL.
  void Cache (iTextureHandle *texture);
  /// Remove an individual texture from cache.
  void Uncache (iTextureHandle *texh);

  /// Clear the cache.
  void Clear ();

  ///
  void SetBilinearMapping (bool m) { rstate_bilinearmap = m; }
  ///
  bool GetBilinearMapping () { return rstate_bilinearmap; }

protected:

  /// Really load the texture in OpenGL memory.
  void Load (csTxtCacheData *d, bool reload = false);
  ///
  void Unload (csTxtCacheData *d);
};

#define DEFAULT_SUPER_LM_SIZE 256
#define DEFAULT_SUPER_LM_NUM 10

union SourceData
{
  iLightMap* LMDataSource;
  csTrianglesPerSuperLightmap* superLMDataSource;
};

/** Cache Data interface for lightmaps and csTrianglesPerLightmap
 * class
 */
struct iLMCache
{
  virtual void * Alloc( int w, int h, SourceData s, csSubRectangles* r, GLuint Handle) = 0;
  virtual void Clear() = 0;
  virtual bool IsPrecalcSuperlightmap() = 0;
  virtual bool HasFog() = 0;
};

/**
 * Cache data for lightmap. This is stored in one of the super
 * lightmaps and also kept with the lightmap itself.
 */
struct csLMCacheData
{
  /// iLightMap.
  iLightMap* Source;
  /// GL texture handle.
  GLuint Handle;
  /// Linked list.
  csLMCacheData *next, *prev;
  /**
   * This contains the precalculated scale and offset
   * for the lightmap relative to the texture.
   */
  float lm_offset_u, lm_scale_u;
  float lm_offset_v, lm_scale_v;

  /**
   * The following is a rectangle in the larger
   * super lightmap. super_lm_idx is the index of the super lightmap.
   */
  csRect super_lm_rect;
  int super_lm_idx;
};

/**
 * Queue of lightmaps for a superlightmap
 */

class csLMCacheDataQueue: public iLMCache
{

  public:
    csLMCacheData* head;
    csLMCacheData* tail;
    virtual void* Alloc(int w, int h, SourceData s, csSubRectangles* r, GLuint Handle);
    virtual void Clear();
    csLMCacheDataQueue();
    virtual bool IsPrecalcSuperlightmap() 
    {
      return false;
    };
    virtual bool HasFog() {return false;};

};

/**
 * Cache data for a whole superlightmap
 */

class csSLMCacheData: public iLMCache
{
  public:
    csTrianglesPerSuperLightmap* source;
    GLuint Handle;
    GLuint FogHandle;
    bool hasFog;
    virtual void* Alloc(int w, int h, SourceData s, csSubRectangles* r, GLuint Handle);
    virtual void Clear();
    virtual bool IsPrecalcSuperlightmap()
    {
      return true;
    };
    virtual bool HasFog() {return hasFog;};

};

/**
 * A queue for lightmap polygons and fog info. Every super-lightmap has
 * such a queue. Only Precalculated Super-Lightmaps have fog info
 */
class csLightMapQueue
{

 

public:
  /// Triangles.
  int num_triangles, max_triangles;
  int* tris;
  /// Vertices.
  int num_vertices, max_vertices;
  GLfloat* glverts;	// 4*max_vertices
  GLfloat* gltxt;	// 2*max_vertices

  int num_fog_colors, max_fog_colors;
  GLfloat* glcolorsFog;
  GLfloat* gltxtFog;

  /** The Lightmap Queue can have a copy of the data or a
  * reference (a pointer to) the original triangle array.
  * If it has a reference we have to be careful when
  * clean or destroy the lightmap
  */
  bool ownsData;

  /** When a superlightmap swaps between owning or not data 
   * it can lose references. Here we keep the last arrays
   * to avoid unnecessary mallocs
   */
  int* trisCached;
  GLfloat* glvertsCached;
  GLfloat* gltxtCached;
  int num_trianglesCached, max_trianglesCached;
  int num_verticesCached, max_verticesCached;
  int num_fog_colorsCached, max_fog_colorsCached;
  GLfloat* glcolorsFogCached;
  GLfloat* gltxtFogCached;

  /// Add some vertices. Return index of the added vertices.
  int AddVertices (int num);

  /// Add a triangle.
  void AddTriangle (int i1, int i2, int i3);

  /// Reset the queue to empty.
  void Reset ();
  
  GLfloat* GetGLVerts (int idx) { return &glverts[idx<<2]; }
  GLfloat* GetGLTxt (int idx) { return &gltxt[idx<<1]; }

  /*DeleteArrays()
  {
    //delete[] tris;
    //delete[] glverts;
    //delete[] gltxt;
  }*/

  /// Adds a whole triangle array
  void AddTrianglesArray(csTriangle* indices, int numTriangles);
  /// Adds a whole vertex array
  void AddVerticesArray(csVector4* verts, int numVerts);
  /// Adds a whole texel array
  void AddTexelsArray(csVector2* uvs, int numUV);


  /// Adds a whole triangle array (Fast Version)
  void AddTrianglesArrayFast(csTriangle* indices, int numTriangles);
  /// Adds a whole vertex array (Fast Version)
  void AddVerticesArrayFast(csVector4* verts, int numVerts);
  /// Adds a whole texel array (Fast Version)
  void AddTexelsArrayFast(csVector2* uvs);

  /// Adds a whole fog colors array
  void AddFogInfoArray(csColor* fogColors, int numColors);
  /// Adds a whole fog texels array
  void AddFogTexelsArray(csVector2* fogTexels, int numUV);


  ///Adds a whole fog color Array (Fast Version);
  void AddFogInfoFast(csColor* work_verts);
  ///Adds a whole fog color Array (Fast Version);
  void AddFogTexelsFast(csVector2* work_fog_texels);


  /// Flush this queue: i.e. render the lightmaps.
  void Flush (GLuint Handle);

  /// Flush the queue (renders the lightmaps and the fog)

  void FlushFog (GLuint HandleFog);

  ///Sets the state of ownsData attribute;
  //void SetOwnsData(bool value) {ownsData = value;};

  ///Gets the state of ownsData attribute;
  //bool GetOwnsData() {return ownsData;}

  /// Saves the arrays when swapping from owning to non owning data
  void SaveArrays();

  /// Loads the arrays when swapping from non owning to owning data
  void LoadArrays();

  /// Constructor.
  csLightMapQueue () :
    num_triangles (0), max_triangles (0), tris (NULL),
    num_vertices (0), max_vertices (0), glverts (NULL), gltxt (NULL),
    num_fog_colors (0), max_fog_colors (0),
    glcolorsFog(NULL), gltxtFog(NULL),
    ownsData (true), 
    trisCached (NULL), glvertsCached (NULL), gltxtCached (NULL),
    num_trianglesCached (0), max_trianglesCached (0), 
    num_verticesCached (0), max_verticesCached (0),
    num_fog_colorsCached (0), max_fog_colorsCached (0),
    glcolorsFogCached(NULL), gltxtFogCached(NULL)
  {}
  /// Destructor.
  ~csLightMapQueue ()
  {

    if(!ownsData){
      if(gltxtCached) delete[] gltxtCached;
      if(glvertsCached) delete[] glvertsCached;
      if(trisCached) delete[] trisCached;
      if(gltxtFogCached) delete[] gltxtFogCached;
      if(glcolorsFogCached) delete[]glcolorsFogCached;
    };
    delete[] tris;
    delete[] glverts;
    delete[] gltxt;
    /*if(glcolorsFog) delete[] glcolorsFog;
    if(gltxtFog) delete[] gltxtFog;*/
  }

};

/**
 * This class represents a super-lightmap.
 * A super-lightmap is a collection of smaller lightmaps that
 * fit together in one big texture.
 */
class csSuperLightMap
{
public:
  /// A class holding all the free regions in this texture.
  csSubRectangles* region;
  /// An OpenGL texture handle.
  GLuint Handle;
  /// the head and tail of the cache data
  iLMCache *cacheData;

  /**
   * The super-lightmap also behaves like a queue for lightmapped polygons.
   * The following field manage that queue.
   */
  csLightMapQueue queue;

  /// Constructor.
  csSuperLightMap ();
  /// Destructor.
  ~csSuperLightMap ();

  /// Try to allocate a lightmap here. Return NULL on failure.
  csLMCacheData* Alloc (int w, int h, SourceData s);
  /// Clear all lightmaps in this super lightmap.
  void Clear ();
};

/**
 * Cache for OpenGL lightmaps. This cache keeps a number of
 * super lightmaps. Every super lightmaps holds a number of lightmaps.
 */
class OpenGLLightmapCache
{
public:
  /// Number of static lightmaps.
  static int super_lm_num;
  /// Size of static lightmaps.
  static int super_lm_size;

private:
  csGraphics3DOGLCommon* g3d;

  /// A number of super-lightmaps to contain all other lightmaps.
  csSuperLightMap* suplm;
  /// Current super lightmap.
  int cur_lm;
  /**
   * Number of super lightmaps we already processed. This is
   * used to see how many super lightmaps we can go back to try
   * allocating lightmaps.
   */
  int num_lm_processed;
  /// If true then setup is ok.
  bool initialized;

  void Load (csLMCacheData *d);
  void Setup ();

  /// Flush all the lightmaps in one super lightmap (i.e. render them).
  void Flush (int sup_idx);

  /// This is the z-buf mode of the current queue.
  csZBufMode queue_zbuf_mode;

public:
  ///
  OpenGLLightmapCache (csGraphics3DOGLCommon* g3d);
  ///
  ~OpenGLLightmapCache ();

  /// Cache a lightmap.
  void Cache (iPolygonTexture *polytex);

  /// Cache a whole precalculated superlightmap
  void Cache(csTrianglesPerSuperLightmap* s);

  /// Finds an empty superlightmap (returns -1 if none is found)
  int FindFreeSuperLightmap();


  /// Uncache a lightmap.
  void Uncache (iPolygonTexture *polytex);

  /// Get the queue for a cached lightmap.
  csLightMapQueue* GetQueue (csLMCacheData* clm)
  {
    return &(suplm[clm->super_lm_idx].queue);
  }
  csLightMapQueue* GetQueue(csTrianglesPerSuperLightmap* tSLM)
  {
    return &(suplm[tSLM->slId].queue);
  }

  /**
   * Flush all the lightmaps (i.e. render them).
   * This will force a flush regardless of render modes.
   */
  void Flush ();

  /**
   * Test if a flush is needed and do flush. This can depend
   * on various settings like z-buffer mode and so on.
   */
  void FlushIfNeeded ();

  /// Clear the entire lightmap cache.
  void Clear ();

  /// Check if lightmap blows size limit
  bool IsLightmapOK (iPolygonTexture *polytex);
};

#endif // __GL_TEXTURECACHE_H__

