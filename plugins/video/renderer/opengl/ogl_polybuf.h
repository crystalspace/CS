/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __OGL_POLYBUF_H__
#define __OGL_POLYBUF_H__

#include "csgeom/vector3.h"
#include "csgeom/plane3.h"
#include "csgeom/matrix3.h"
#include "csutil/garray.h"
#include "csutil/refarr.h"
#include "plugins/video/renderer/common/vbufmgr.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "csutil/cscolor.h"
#include "csgeom/subrec.h"

class csSLMCacheData;

struct Indexes
{
  int vertex;
  int uv;
};

class csVertexIndexArrayNode
{
public:
  CS_DECLARE_GROWING_ARRAY(indices,Indexes);
};

typedef csVertexIndexArrayNode* csIndexVertex;

class csTrianglesPerMaterial
{
public:
  int matIndex;
  int numVertices;
  int numTriangles;

  // We need a better implementation here
  // We're duplicating info, but we need the number of vertices per
  // material, so later we can call ClipTriangleMesh

  CS_DECLARE_GROWING_ARRAY(idx_vertices,csIndexVertex);
  CS_DECLARE_GROWING_ARRAY(triangles,csTriangle);

  CS_DECLARE_GROWING_ARRAY(texels, csVector2);
  CS_DECLARE_GROWING_ARRAY(verticesPoints, csVector3);

  csTrianglesPerMaterial ();
  csTrianglesPerMaterial (int numVertex);

  ~csTrianglesPerMaterial ();

  void ClearVertexArray ();

  /// Return the number of triangles
  int TriangleCount () const { return numTriangles; }
};


class TrianglesNode
{
public:
  csTrianglesPerMaterial *info;
  TrianglesNode * next;
  TrianglesNode ();
  ~TrianglesNode ();
};

class TrianglesList
{
public:
  TrianglesNode* first;
  TrianglesNode* last;

  TrianglesList ();
  ~TrianglesList ();
  int GetLastMaterial ()
  {
    if (last == NULL) return -1;
    return last->info->matIndex;
  }
  void Add (TrianglesNode* t);
  TrianglesNode* GetLast () { return last; }
};

/**
 * This class stores triangles thatcould share the same superlightmap
 */
class csTrianglesPerSuperLightmap
{
public:
  /// triangles which shares the same superlightmap
  CS_DECLARE_GROWING_ARRAY (triangles, csTriangle);

  /// Vertices of those triangles
  CS_DECLARE_GROWING_ARRAY (vec_vertices, csVector3);

  /// texels of those triangles
  CS_DECLARE_GROWING_ARRAY (texels, csVector2);

  /// The lightmaps in the superlightmap
  csRefArray<iPolygonTexture> lightmaps;

  /**
   * Array for keeping which csFogInfo indexescorresponds to every
   * vertex. This is needed because we can create new vertices in the
   * polygon buffer. These new vertices will have the same geometric
   * coordinates than old ones, but can differ in uv coordinates.
   * The csFogInfo array that cames with the mesh when drawing only
   * contains info for the original vertices, so we have to store,
   * every time we create a new vertex, whic csFogInfo corresponds it
   * if it was an original vertex (Basically it stores original
   * vertices indices).
   */
  CS_DECLARE_GROWING_ARRAY (fogInfo, int);

  /**
   * Auxiliary array for vertices: because we want to create as few
   * vertex as possible, we only will create new vertices if the have the
   * same coordinates but
   * different lightmap coordinates
   */
  CS_DECLARE_GROWING_ARRAY (vertexIndices, csIndexVertex);
  CS_DECLARE_GROWING_ARRAY (rectangles, csRect);

  //SuperLightmap Id.
  int slId;

  csSubRectangles* region;

  int numTriangles;
  int numTexels;
  int numVertices;

  csTrianglesPerSuperLightmap();
  csTrianglesPerSuperLightmap(int numVertex);
  ~csTrianglesPerSuperLightmap();

  /// Pointer to the cache data
  csSLMCacheData* cacheData;
  bool isUnlit;

  // Checks if the superlightmap is initialized or not
  bool initialized;
};

/** Simple single list node*/
class TrianglesSuperLightmapNode
{
public:
  TrianglesSuperLightmapNode* prev;
  csTrianglesPerSuperLightmap * info;

  TrianglesSuperLightmapNode ();
  ~TrianglesSuperLightmapNode ();
};

/**
 * Single Linked List that stores all the lightmap's triangles and uv's.
 * every node strores all the lightmaps that could share the same
 * superlightmap and all the triangles and uv needed for lighting
 * with that superlightmap.
 * The list goes from the last to the first (as a stack would do, but without
 * popping)
 */
class TrianglesSuperLightmapList
{
public:
  TrianglesSuperLightmapNode* first;
  TrianglesSuperLightmapNode* last;

  TrianglesSuperLightmapList ();
  ~TrianglesSuperLightmapList ();
  void Add (TrianglesSuperLightmapNode* t);
  TrianglesSuperLightmapNode* GetLast () { return last; }

  // Dirty due dynamic lights, needs recalculating.
  bool dirty;

  // FirstTime says if it's the first time that the superlightmap
  // is cached.
  bool firstTime;

  // Marks as dirty.
  void MarkLightmapsDirty () { dirty = true; }

  // Clear the dirty state.
  void ClearLightmapsDirty() { dirty = false; }
  // Gets the dirty state.
  bool GetLightmapsDirtyState () const { return dirty; }
};


/**
 * This implementation is optimized to use glDrawElements.
 * It groups polygons per materials
 */
class csTriangleArrayPolygonBuffer : public csPolygonBuffer
{
private:
  /*
   * Add a single vertex to the given tri/mat buffer.
   * Returns vertex index.
   */
  int AddSingleVertex (csTrianglesPerMaterial* pol,
	int* verts, int i, const csVector2& uv);

  /*
   * Add a single vertex to the given suplm.
   * Returns vertex index. This version is for when there is
   * actually no lightmapping needed but the suplm is only
   * there for fog.
   */
  int AddSingleVertexLM (csTrianglesPerSuperLightmap* triSuperLM,
	int* verts, int i);

  /*
   * Add a single vertex to the given suplm.
   * Returns vertex index.
   */
  int AddSingleVertexLM (csTrianglesPerSuperLightmap* triSuperLM,
	int* verts, int i, const csVector2& uvLightmap);

public:
  // SuperLightMap list.
  TrianglesSuperLightmapList superLM;
protected:
  // Mesh triangles grouped by material list.
  TrianglesList polygons;

  csRefArray<iMaterialHandle> materials;
  CS_DECLARE_GROWING_ARRAY (normals, csVector3);

  csVector3 * vertices;
  int matCount;
  int verticesCount;

  csTrianglesPerSuperLightmap* SearchFittingSuperLightmap (
    iPolygonTexture* poly_texture, csRect& rect, int num_vertices);

  csTrianglesPerSuperLightmap* unlitPolysSL;

public:

  bool HaveUnlitPolys() {return unlitPolysSL != NULL;};

  TrianglesNode* GetFirst () { return polygons.first; }
  TrianglesSuperLightmapNode* GetFirstTrianglesSLM () { return superLM.last; }

  int GetUVCount (TrianglesSuperLightmapNode* t);
  int GetUVCount (TrianglesNode* t);

  /// Gets the number of materials of the mesh
  virtual int GetMaterialCount() const { return matCount;}

  /// Gets the Lightmap cache data for a given node (by super lightmap)
  csSLMCacheData* GetCacheData(TrianglesSuperLightmapNode* t);

  /// Gets the number of lightmaps for a given node (per super lightmap)
  int GetLightmapCount(TrianglesSuperLightmapNode* t);

  /// Gets the material handler for a given node (by material)
  iMaterialHandle* GetMaterialPolygon(TrianglesNode* t)
  { return (iMaterialHandle*)(materials[t->info->matIndex]);}

  /// Constructor
  csTriangleArrayPolygonBuffer (iVertexBufferManager* mgr);
  /// Destructor
  virtual ~csTriangleArrayPolygonBuffer ();

  /// Adds a polygon to the polygon buffer
  virtual void AddPolygon (int* verts, int num_verts,
    const csPlane3& poly_normal, int mat_index,
    const csMatrix3& m_obj2tex, const csVector3& v_obj2tex,
    iPolygonTexture* poly_texture);

  /// Adds a material to the polygon buffer
  virtual void AddMaterial (iMaterialHandle* mat_handle);

  /// Gets the material handler for a given index
  virtual iMaterialHandle* GetMaterial (int idx) const
  {
    return materials[idx];
  }

  /// Gets the fog indices

  int *GetFogIndices(TrianglesSuperLightmapNode* tSL);

  /// Gets the unlit polygons
  csTrianglesPerSuperLightmap* GetUnlitPolys(){ return unlitPolysSL;};

  /// Sets a material
  virtual void SetMaterial (int idx, iMaterialHandle* mat_handle);

  /// Sets the mesh vertices
  virtual void SetVertexArray (csVector3* verts, int num_verts);

  /// Clear the polygon buffer
  virtual void Clear ();

  /// Gets the mesh vertices
  virtual csVector3* GetVertices() const {return vertices;}

  /// Gets the original vertices count
  virtual int GetVertexCount() const {return verticesCount;}

  /// Given a polygon triangulize it and adds it to the polygon buffer
  void AddTriangles (csTrianglesPerMaterial* pol,
    csTrianglesPerSuperLightmap* triSuperLM,
    int* verts, int num_vertices, const csMatrix3& m_obj2tex,
    const csVector3& v_obj2tex,iPolygonTexture* poly_texture, int mat_index,
    const csPlane3& poly_normal);

  /// Marks the polygon buffer as affected by any light
  virtual void MarkLightmapsDirty();
};

/**
 * Version of the vertex buffer manager that understands
 * csPolArrayPolygonBuffer.
 */
class csTriangleArrayVertexBufferManager : public csVertexBufferManager
{
public:
  /// Initialize the vertex buffer manager
  csTriangleArrayVertexBufferManager (iObjectRegistry* object_reg);
  /// Destroy the vertex buffer manager
  virtual ~csTriangleArrayVertexBufferManager ();

  virtual iPolygonBuffer* CreatePolygonBuffer ();
};

#endif // __POLYBUF_H__

