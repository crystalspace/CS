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

#ifndef __CS_OGL_POLYBUF_H__
#define __CS_OGL_POLYBUF_H__

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

class csTrianglesPerMaterial
{
public:
  csTrianglesPerMaterial * next;
  int matIndex;

  // We need a better implementation here
  // We're duplicating info, but we need the number of vertices per
  // material, so later we can call ClipTriangleMesh

  csGrowingArray<csTriangle> triangles;

  csTrianglesPerMaterial ();

  ~csTrianglesPerMaterial ();

  void ClearVertexArray ();

  /// Return the number of triangles
  int TriangleCount () const { return triangles.Length (); }
};


class TrianglesList
{
public:
  csTrianglesPerMaterial* first;
  csTrianglesPerMaterial* last;

  TrianglesList ();
  ~TrianglesList ();
  int GetLastMaterial ()
  {
    if (last == NULL) return -1;
    return last->matIndex;
  }
  void Add (csTrianglesPerMaterial* t);
  csTrianglesPerMaterial* GetLast () { return last; }
};

/**
 * This class stores triangles that could share the same superlightmap
 */
class csTrianglesPerSuperLightmap
{
public:
  csTrianglesPerSuperLightmap* prev;

  /// triangles which shares the same superlightmap
  csGrowingArray<csTriangle> triangles;

  /// The lightmaps in the superlightmap
  csRefArray<iPolygonTexture> lightmaps;
  csGrowingArray<csRGBpixel*> lm_info;

  csGrowingArray<csRect> rectangles;

  csSubRectangles* region;

  csTrianglesPerSuperLightmap();
  ~csTrianglesPerSuperLightmap();

  /// Pointer to the cache data
  csSLMCacheData* cacheData;
  bool isUnlit;

  // Checks if the superlightmap is initialized or not
  bool initialized;

  // Cost of this super lightmap. A high number means that
  // this super lightmap requires a lot of work to update. This number
  // is related to the number of lightmaps in it and the total area.
  int cost;
  int CalculateCost ();

  // Timestamp indicating when this super lightmap was last used.
  uint32 timestamp;
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
  csTrianglesPerSuperLightmap* first;
  csTrianglesPerSuperLightmap* last;

  TrianglesSuperLightmapList ();
  ~TrianglesSuperLightmapList ();
  void Add (csTrianglesPerSuperLightmap* t);
  csTrianglesPerSuperLightmap* GetLast () { return last; }

  // Dirty due dynamic lights, needs recalculating.
  bool dirty;

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
	int* verts, int i, const csVector2& uv, int& cur_vt_idx);

  /*
   * Add a single vertex to the given suplm.
   * Returns vertex index.
   */
  int AddSingleVertexLM (const csVector2& uvLightmap, int& cur_vt_idx);

public:
  // SuperLightMap list.
  TrianglesSuperLightmapList superLM;
protected:
  // Mesh triangles grouped by material list.
  TrianglesList polygons;

  csRefArray<iMaterialHandle> materials;

  /**
   * Vertices per triangle (every vertex is duplicated here for every
   * triangle in the list).
   */
  csGrowingArray<csVector3> vec_vertices;
  /// Texels for those triangles
  csGrowingArray<csVector2> texels;
  /// Lumels for those triangles
  csGrowingArray<csVector2> lumels;

  csVector3 * vertices;
  int matCount;
  int verticesCount;
  csGrowingArray<csTriangle> orig_triangles;

  csTrianglesPerSuperLightmap* SearchFittingSuperLightmap (
    iPolygonTexture* poly_texture, csRect& rect);

  csTrianglesPerSuperLightmap* unlitPolysSL;

public:

  bool HaveUnlitPolys() {return unlitPolysSL != NULL;};

  csTrianglesPerMaterial* GetFirst () { return polygons.first; }
  csTrianglesPerSuperLightmap* GetFirstTrianglesSLM () { return superLM.last; }

  /// Gets the number of materials of the mesh
  virtual int GetMaterialCount() const { return matCount;}

  /// Gets the material handler for a given node (by material)
  iMaterialHandle* GetMaterialPolygon(csTrianglesPerMaterial* t)
  { return (iMaterialHandle*)(materials[t->matIndex]);}

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

  /// Gets the unlit polygons
  csTrianglesPerSuperLightmap* GetUnlitPolys(){ return unlitPolysSL;};

  /// Sets a material
  virtual void SetMaterial (int idx, iMaterialHandle* mat_handle);

  /// Sets the mesh vertices
  virtual void SetVertexArray (csVector3* verts, int num_verts);

  /// Clear the polygon buffer
  virtual void Clear ();

  /// Gets the mesh vertices
  virtual csVector3* GetVertices () const { return vertices; }
  /// Gets the original triangles.
  csTriangle* GetTriangles () { return orig_triangles.GetArray (); }

  /// Gets the original vertices count
  virtual int GetVertexCount () const { return verticesCount; }
  /// Gets the original triangle count
  int GetTriangleCount () const { return orig_triangles.Length (); }

  /// Get the total number of vertices.
  int GetTotalVertexCount () const { return vec_vertices.Length () ; }
  /// Get the total vertices.
  csVector3* GetTotalVertices () { return vec_vertices.GetArray (); }
  /// Get the total texels.
  csVector2* GetTotalTexels () { return texels.GetArray (); }
  /// Get the total lumels.
  csVector2* GetTotalLumels () { return lumels.GetArray (); }

  /// Given a polygon triangulize it and adds it to the polygon buffer
  void AddTriangles (csTrianglesPerMaterial* pol,
    int* verts, int num_vertices, const csMatrix3& m_obj2tex,
    const csVector3& v_obj2tex,iPolygonTexture* poly_texture, int mat_index,
    int cur_vt_idx);

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

#endif // __CS_OGL_POLYBUF_H__

