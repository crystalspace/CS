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


struct Indexes
{
  int vertex;
  int uv;
};

class csVertexIndexArrayNode{ 
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

class csTrianglesPerMaterial{
  public:
    int matIndex;
    int numVertices;
    int numTriangles;
    

    //We need a better implementation here
    // We're duplicating info, but we need the number of vertices per
    // material, so later we can call ClipTriangleMesh    
    typedef csVertexIndexArrayNode* csIndexVertex;
    CS_DECLARE_GROWING_ARRAY(vertices,csIndexVertex);
    CS_DECLARE_GROWING_ARRAY(triangles,csTriangle);
    
    CS_DECLARE_GROWING_ARRAY(infoPolygons,iPolyTex_p);
    CS_DECLARE_GROWING_ARRAY(texels, csVector2);
    CS_DECLARE_GROWING_ARRAY(verticesPoints, csVector3);
    CS_DECLARE_GROWING_ARRAY(colors, csColor);

    csTrianglesPerMaterial();
    csTrianglesPerMaterial(int numVertex);
    void ClearVertexArray();
    void CopyTrianglesGrowToTriangles();
    void CopyInfoPolygons();

    int TriangleCount() { return numTriangles;};
};


//class TrianglesNode;
class TrianglesNode{
  public:
    csTrianglesPerMaterial *info;
    TrianglesNode * next;
    TrianglesNode();
    ~TrianglesNode();

};

class TrianglesList{
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




/**
 * This implementation is optimized to use glDrawElements.
 * It groups polygons per materials
 */
class csTriangleArrayPolygonBuffer : public csPolygonBuffer
{
protected:
  TrianglesList polygons;
  typedef iMaterialHandle* iMaterialHandleP;
  CS_DECLARE_GROWING_ARRAY (materials, iMaterialHandleP);
  CS_DECLARE_GROWING_ARRAY (normals, csVector3);
  
  csVector3 * vertices;
  int matCount;
  int verticesCount;
  

public:
  ///
  bool first_time_rendering;
  csTriangle* GetTriangles(TrianglesNode* t);
  int GetTriangleCount(TrianglesNode* t); 
  int GetVertexCount(TrianglesNode* t);
  int GetMatIndex(TrianglesNode* t);
  csVector2* GetUV(TrianglesNode* t);
  csVector3* GetVertices() {return vertices;}
  csVector3* GetVerticesPerMaterial(TrianglesNode* t);
  csColor* GetColors(TrianglesNode *t);
  TrianglesNode* GetFirst();
  TrianglesNode* GetNext(TrianglesNode* t);
  int GetUVCount(TrianglesNode* t);
  iPolyTex_p* GetLightMaps(TrianglesNode* t);
  


  iMaterialHandle* GetMaterialPolygon(TrianglesNode* t) //RETOCAR!!!
  { return (materials[GetMatIndex(t)]);}
  
  csTriangleArrayPolygonBuffer (iVertexBufferManager* mgr);
  ///
  virtual ~csTriangleArrayPolygonBuffer ();

  bool IsDirty(){return false;}; //RETOCAR!!!

  /// Get the number of polygons.
  //int GetPolygonCount () const { return polygons.Length (); }
  /// Get the polygon info.
  //const csPolArrayPolygon& GetPolygon (int i) const { return polygons[i]; }
  /// Get the number of vertices.
  //int GetVertexCount () const { return num_vertices; }
  /// Get the vertices array.
  //csVector3* GetVertices () const { return vertices; }

  virtual void AddPolygon (int* verts, int num_verts,
	const csPlane3& poly_normal,
	int mat_index,
	const csMatrix3& m_obj2tex, const csVector3& v_obj2tex,
	iPolygonTexture* poly_texture);
  virtual void AddMaterial (iMaterialHandle* mat_handle);
  virtual iMaterialHandle* GetMaterial (int idx) const
  {
    return materials[idx];
  }
  virtual void SetMaterial (int idx, iMaterialHandle* mat_handle);
  virtual void SetVertexArray (csVector3* verts, int num_verts);
  virtual void Clear ();
  virtual int GetMaterialCount() const { return matCount;}
  void AddTriangles(csTrianglesPerMaterial* pol, int* verts, 
    int num_vertices, const csMatrix3& m_obj2tex, 
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

