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
#include "plugins/video/renderer/common/vbufmgr.h"
#include "ivideo/graph3d.h"
#include "csutil/cscolor.h"
#include "csgeom/subrec.h"

class csSLMCacheData;

class csVector4
{
  public:
    float x;
    float y;
    float z;
    float w;

    csVector3& GetcsVector3();    
};

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

/*class csTriangleInfo
{
  public:
    csPlane3 normal;
    csMatrix3 m_obj2tex;
    csVector3 v_obj2tex;
    int matIndex;
    iPolygonTexture* poly_texture;

    csTriangleInfo();
};
*/

typedef iPolygonTexture* iPolyTex_p;
typedef csVertexIndexArrayNode* csIndexVertex;

class csTrianglesPerMaterial{
  public:
    int matIndex;
    int numVertices;
    int numTriangles;
    

    //We need a better implementation here
    // We're duplicating info, but we need the number of vertices per
    // material, so later we can call ClipTriangleMesh    
    
    CS_DECLARE_GROWING_ARRAY(vertices,csIndexVertex);
    CS_DECLARE_GROWING_ARRAY(triangles,csTriangle);
    
    CS_DECLARE_GROWING_ARRAY(infoPolygons,iPolyTex_p);
    CS_DECLARE_GROWING_ARRAY(texels, csVector2);
    CS_DECLARE_GROWING_ARRAY(verticesPoints, csVector3);
    CS_DECLARE_GROWING_ARRAY(colors, csColor);

    csTrianglesPerMaterial();
    csTrianglesPerMaterial(int numVertex);

    ~csTrianglesPerMaterial();


    void ClearVertexArray();
    void CopyTrianglesGrowToTriangles();
    void CopyInfoPolygons();

    ///Return the number of triangles
    int TriangleCount() { return numTriangles;};
};


//class TrianglesNode;
class TrianglesNode
{
  public:
    csTrianglesPerMaterial *info;
    TrianglesNode * next;
    TrianglesNode();
    ~TrianglesNode();

};

class TrianglesList
{
  public:
    TrianglesNode* first;
    TrianglesNode* last;
    int numElems;
    
    TrianglesList();
    ~TrianglesList();
    int GetLastMaterial();
    void Add(TrianglesNode* t);
    TrianglesNode* GetLast();
};


/**This class stores triangles thatcould share the same superlightmap
 */

class csTrianglesPerSuperLightmap
{
  public:
    ///triangles which shares the same superlightmap
    CS_DECLARE_GROWING_ARRAY(triangles,csTriangle);

    ///Vertices of those triangles
    CS_DECLARE_GROWING_ARRAY(vertices,csVector4);

    ///texels of those triangles
    CS_DECLARE_GROWING_ARRAY(texels,csVector2);

    ///The lightmaps in the superlightmap
    CS_DECLARE_GROWING_ARRAY(lightmaps,iPolyTex_p);

    /**Array for keeping which csFogInfo indexescorresponds to every 
     *vertex. This is needed because we can create new vertices in the
     *polygon buffer. These new vertices will have the same geometric
     * coordinates than old ones, but can differ in uv coordinates.
     * The csFogInfo array that cames with the mesh when drawing only
     * contains info for the original vertices, so we have to store,
     * every time we create a new vertex, whic csFogInfo corresponds it
     * if it was an original vertex (Basically it stores original
     * vertices indices).
     */
    CS_DECLARE_GROWING_ARRAY(fogInfo, int);

    /** Auxiliary array for vertices: because we want to create as few 
     *vertex as possible, we only will create new vertices if the have the 
     *same coordinates but
     *different lightmap coordinates
     */
    CS_DECLARE_GROWING_ARRAY(vertexIndices,csIndexVertex);
    CS_DECLARE_GROWING_ARRAY(rectangles, csRect);

    //SuperLightmap Id.
    int slId;

    csSubRectangles* region;

    int numTriangles;
    int numTexels;
    int numVertices;
    int numLightmaps;
    
    csTrianglesPerSuperLightmap();
    csTrianglesPerSuperLightmap(int numVertex);
    ~csTrianglesPerSuperLightmap();

    ///Pointer to the cache data
    csSLMCacheData* cacheData;

};

/** Simple single list node*/

class TrianglesSuperLightmapNode
{

  public:
    TrianglesSuperLightmapNode* prev;
    csTrianglesPerSuperLightmap * info;

    TrianglesSuperLightmapNode();
    ~TrianglesSuperLightmapNode();
};

/** Single Linked List that stores all the lightmap's triangles and uv's.
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
    int numElems;

    TrianglesSuperLightmapList();
    ~TrianglesSuperLightmapList();
    void Add(TrianglesSuperLightmapNode* t);
    TrianglesSuperLightmapNode* GetLast();
};


/**
 * This implementation is optimized to use glDrawElements.
 * It groups polygons per materials
 */
class csTriangleArrayPolygonBuffer : public csPolygonBuffer
{
protected:

  //Mesh triangles grouped by material list
  TrianglesList polygons;
  
  //SuperLightMap list
  TrianglesSuperLightmapList superLM; 
  typedef iMaterialHandle* iMaterialHandleP;

  CS_DECLARE_GROWING_ARRAY (materials, iMaterialHandleP);

  CS_DECLARE_GROWING_ARRAY (normals, csVector3);

  //It stores the lightmaps already stored in some SuperLightMap  
  CS_DECLARE_GROWING_ARRAY (lightmapArray, iPolyTex_p);
  
  csVector3 * vertices;
  int matCount;
  int verticesCount;

  csTrianglesPerSuperLightmap* SearchFittingSuperLightmap(
    iPolygonTexture* poly_texture, csRect& rect, int num_vertices);
  

public:
  

  /// Gets the triangles for a given node (by material)
  csTriangle* GetTriangles(TrianglesNode* t);

  /// Gets the triangles for a given node (by super lightmap)
  csTriangle* GetTriangles(TrianglesSuperLightmapNode* t);

  /// Gets the triangles count for a given node (by material)
  int GetTriangleCount(TrianglesNode* t); 

  /// Gets the triangles count for a given node (by super lightmap)
  int GetTriangleCount(TrianglesSuperLightmapNode* t); 

 
  ///Gets the  vertices count for a given node (by material)
  int GetVertexCount(TrianglesNode* t);

  ///Gets the  vertices count for a given node (by super lightmap)
  int GetVertexCount(TrianglesSuperLightmapNode* t);
  
  ///Gets the material index for a given node
  int GetMatIndex(TrianglesNode* t);


  ///Gets the UV coordinates for a given node (by material)
  csVector2* GetUV(TrianglesNode* t);

  ///Gets the UV coordinates for a given node (by super lightmap)
  csVector2* GetUV(TrianglesSuperLightmapNode* t);
  
  ///Gets the vertices for a given node (by material)
  csVector3* GetVerticesPerMaterial(TrianglesNode* t);

  ///Gets the vertices for a given node (by super lightmap)
  csVector4* GetVerticesPerSuperLightmap(TrianglesSuperLightmapNode* t);

  ///Gets the vertices' colors for a given node (per material)
  csColor* GetColors(TrianglesNode *t);
  
  TrianglesNode* GetFirst();
  TrianglesNode* GetNext(TrianglesNode* t);
  TrianglesSuperLightmapNode* GetFirstTrianglesSLM();
  TrianglesSuperLightmapNode* GetNextTrianglesSLM(TrianglesSuperLightmapNode* t);

  int GetUVCount(TrianglesSuperLightmapNode* t);
  int GetUVCount(TrianglesNode* t);

  //iPolyTex_p* GetLightMaps(TrianglesNode* t);

  ///Gets the number of materials of the mesh
  virtual int GetMaterialCount() const { return matCount;}
  
  ///Gets the number of super lightmaps needed for this mesh
  int GetSuperLMCount();

  /// Gets the Lightmap cache data for a given node (by super lightmap)
  csSLMCacheData* GetCacheData(TrianglesSuperLightmapNode* t);

  ///Gets the number of lightmaps for a given node (per super lightmap)
  int GetLightmapCount(TrianglesSuperLightmapNode* t);
  
  ///Gets the material handler for a given node (by material)
  iMaterialHandle* GetMaterialPolygon(TrianglesNode* t)
  { return (materials[GetMatIndex(t)]);}
  
  ///Constructor
  csTriangleArrayPolygonBuffer (iVertexBufferManager* mgr);
  ///Destructor
  virtual ~csTriangleArrayPolygonBuffer ();
  
  ///Adds a polygon to the polygon buffer
  virtual void AddPolygon (int* verts, int num_verts,
	const csPlane3& poly_normal,
	int mat_index,
	const csMatrix3& m_obj2tex, const csVector3& v_obj2tex,
	iPolygonTexture* poly_texture);

  ///Adds a material to the polygon buffer
  virtual void AddMaterial (iMaterialHandle* mat_handle);

  ///Gets the material handler for a given index
  virtual iMaterialHandle* GetMaterial (int idx) const
  {
    return materials[idx];
  }

  ///Gets the fog indices

  int *GetFogIndices(TrianglesSuperLightmapNode* tSL);

  /// Sets a material
  virtual void SetMaterial (int idx, iMaterialHandle* mat_handle);

  ///Sets the mesh vertices
  virtual void SetVertexArray (csVector3* verts, int num_verts);

  ///Clear the polygon buffer
  virtual void Clear ();

  ///Gets the mesh vertices
  virtual csVector3* GetVertices() const {return vertices;}

  /// Gets the original vertices count
  virtual int GetVertexCount() const {return verticesCount;}

  ///Given a polygon triangularizes it and adds it to the polygon buffer
  void AddTriangles(csTrianglesPerMaterial* pol, 
    csTrianglesPerSuperLightmap* triSuperLM,
    int* verts, int num_vertices, const csMatrix3& m_obj2tex, 
    const csVector3& v_obj2tex,iPolygonTexture* poly_texture, int mat_index,
    const csPlane3& poly_normal);
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

