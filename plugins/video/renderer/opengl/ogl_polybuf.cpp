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

#include <math.h>
#include <stdarg.h>

#include "cssysdef.h"
#include "ogl_polybuf.h"
#include "csutil/util.h"
#include "imesh/thing/polygon.h"
#include "csgeom/transfrm.h"
#include "qint.h"
#include "ivideo/material.h"
#include "ivideo/texture.h"
#include "imesh/thing/lightmap.h" //@@@
#include "ogl_txtcache.h"


TrianglesNode::TrianglesNode()
{
  info = NULL;
  next = NULL;
}

TrianglesNode::~TrianglesNode()
{
  delete info;
  next = NULL;
}

TrianglesList::TrianglesList()
{
  first = NULL;
  last = NULL;
  numElems = 0;
}

TrianglesList::~TrianglesList()
{
  int i;
  TrianglesNode* aux = first;
  TrianglesNode* aux2;

  for ( i = 0; i < numElems; i++)
  {
    aux2 = aux->next;
    delete aux;
    aux = aux2;
  }
}

int TrianglesList::GetLastMaterial()
{
  if(numElems == 0) return -1;
  return last->info->matIndex;
}

void TrianglesList::Add(TrianglesNode* t)
{
  if(first == NULL)
  {
    first = last = t;
    numElems++;
    return;
  }
  last->next = t;
  last = t;
  numElems++;
}

TrianglesNode* TrianglesList::GetLast()
{
  return last;
}


/*
csTriangleInfo::csTriangleInfo()
{
   poly_texture = NULL;
}
*/


csTriangleArrayPolygonBuffer::csTriangleArrayPolygonBuffer (
  iVertexBufferManager* mgr) : csPolygonBuffer (mgr)
{

  vertices = NULL;
  matCount = 0;
  unlitPolysSL = NULL;
}


csTriangleArrayPolygonBuffer::~csTriangleArrayPolygonBuffer ()
{
  Clear ();
}


csTrianglesPerMaterial::csTrianglesPerMaterial()
{
  numVertices = 0;
  numTriangles = 0;
}

csTrianglesPerMaterial::csTrianglesPerMaterial(int numVertex)
{
  numVertices = 0;
  numTriangles = 0;
  vertices.SetLimit (numVertex);
  verticesPoints.SetLimit (numVertex);
  int i;
  for(i = 0; i < numVertex; i++) vertices[i] = NULL;

}

csTrianglesPerMaterial::~csTrianglesPerMaterial()
{
}


/**
 * Copy all the triangles to a csTriangle vector
 * this is needed to ClipTriangleMesh, we just advance the work
 * here.
 * Deletes trianglesGrow, it's no needed anymore
 */
void csTrianglesPerMaterial::CopyTrianglesGrowToTriangles()
{
  /*numTriangles = trianglesGrow.Length();
  csTriangle * triangles = new csTriangle[numTriangles];
  int i;

  for(i = 0; i < numTriangles; i++)
  {
    triangles[i] = *trianglesGrow[i];
    //Maybe are just pointers to triangles or the triangle itself?
    // I suppose pointers
    delete trianglesGrow[i];
  };
  trianglesGrow.SetLength(0);
  */
}

void csTrianglesPerMaterial::ClearVertexArray()
{
  /*int i;

  for(i = 0; i < vertices.Length(); i++)
    if(vertices[i] != NULL) delete vertices[i];
  vertices.SetLength(0);
  */

}

csTrianglesPerSuperLightmap::csTrianglesPerSuperLightmap()
{
  region = new csSubRectangles (
    csRect (0, 0, OpenGLLightmapCache::super_lm_size,
  OpenGLLightmapCache::super_lm_size));

  numTriangles = 0;
  numTexels = 0;
  numVertices = 0;
  numLightmaps = 0;
  cacheData = NULL;
  isUnlit = false;
  initialized = false;
}

csTrianglesPerSuperLightmap::csTrianglesPerSuperLightmap(int numVertex)
{
  region = new csSubRectangles (
    csRect (0, 0, OpenGLLightmapCache::super_lm_size,
  OpenGLLightmapCache::super_lm_size));

  numTriangles = 0;
  numTexels = 0;
  numVertices = 0;
  numLightmaps = 0;
  cacheData = NULL;
  initialized = false;

  vertexIndices.SetLength(numVertex);

  int i;

  for(i = 0; i < numVertex; i++) vertexIndices[i] = NULL;
  isUnlit = false;
}

csTrianglesPerSuperLightmap::~csTrianglesPerSuperLightmap ()
{
  int i;
  int numIndices = vertexIndices.Length();
  for (i = 0 ; i < numIndices ; i++)
  {
    delete vertexIndices[i];
  }
  if (cacheData) cacheData->Clear();
  delete region;
}


TrianglesSuperLightmapNode::TrianglesSuperLightmapNode()
{
  info = NULL;
  prev = NULL;
}

TrianglesSuperLightmapNode::~TrianglesSuperLightmapNode()
{
  delete info;
}

TrianglesSuperLightmapList::TrianglesSuperLightmapList()
{
  first = NULL;
  last = NULL;
  numElems = 0;
  dirty = true;
  firstTime = true;
}

TrianglesSuperLightmapList::~TrianglesSuperLightmapList()
{
  TrianglesSuperLightmapNode* cur;
  int i;
  for(i = 0; i < numElems; i++)
  {
    cur = last;
    last = last->prev;
    delete cur;
  }
  first = NULL;
  last = NULL;
  numElems = 0;
}

TrianglesSuperLightmapNode* TrianglesSuperLightmapList::GetLast()
{
  return last;
}

void TrianglesSuperLightmapList::Add(TrianglesSuperLightmapNode* t)
{
  if (numElems == 0) first = t;
  t->prev = last;
  last = t;
  numElems++;
}

/**
 * Search a superlightmap to fit the lighmap in the superLM list
 * if it can't find any creates a new one.
 * The case that the polygon has no superlightmap is supported too.
 * If the polygontexture has no lightmap it means its not lighted,
 * then a special superlightmap has to be created, just to store
 * the triangles and vertices that will be used in fog
 */
csTrianglesPerSuperLightmap* csTriangleArrayPolygonBuffer::
    SearchFittingSuperLightmap (iPolygonTexture* poly_texture,
                                csRect& rect,int /*num_vertices*/)
{
  int i;
  if (poly_texture == NULL || poly_texture->GetLightMap () == NULL)
  {
    // OK This polygon has no lightmap.
    // Let's check if we have to create a unlitPolygonsSL or is already
    // created
    if (unlitPolysSL) return unlitPolysSL;
    unlitPolysSL = new csTrianglesPerSuperLightmap (verticesCount);
    unlitPolysSL->isUnlit = true;
    return unlitPolysSL;
  }
  iLightMap* piLM = poly_texture->GetLightMap();
  int lm_width = piLM->GetWidth();
  int lm_height = piLM->GetHeight();

  TrianglesSuperLightmapNode* curr = superLM.last;
  for (i = 0; i < superLM.numElems; i++)
  {
    if (curr->info->region->Alloc (lm_width, lm_height, rect))
      return curr->info;
    curr = curr->prev;
  }

  //We haven't found any, let's create a new one

  curr = new TrianglesSuperLightmapNode();
  curr->info = new csTrianglesPerSuperLightmap (verticesCount);

  if (!curr->info->region->Alloc (lm_width, lm_height, rect))
  {
    return NULL;
  }

  superLM.Add (curr);
  return curr->info;
}

int csTriangleArrayPolygonBuffer::AddSingleVertex (csTrianglesPerMaterial* pol,
	int* verts, int i, const csVector2& uv)
{
  int indexVert = -1;
  if (pol->vertices[verts[i]] != NULL)
  {
    /*
     * Let's see if the vertex is already in the vertices array.
     * A vertex is in the vertices array if:
     * - pol->vertices[verts[i]] != NULL AND
     * - In pol->vertices[verts[i]] array exists an index that points
     * to a vertex that has the same uv's
     */
    int j;
    for (j = 0; j < pol->vertices[verts[i]]->indices.Length(); j++)
    {
      indexVert = pol->vertices[verts[i]]->indices[j].vertex;
      int uvIndex = pol->vertices[verts[i]]->indices[j].uv;
      // Let's see if this vertex has the same uv's.
      if (uv == pol->texels[uvIndex]) break;
      indexVert = -1;
    }
  }
  else
  {
    /*
     * It's is null, we have to create
     * the vertices[verts[i]] array and we can push it directly in
     * the verticesPoints array and the uv's can be pushed directly too
     */
    pol->vertices[verts[i]] = new csVertexIndexArrayNode;
  }

  if (indexVert < 0)
  {
    Indexes ind;
    pol->verticesPoints.Push(vertices[verts[i]]);
    ind.vertex = pol->verticesPoints.Length() - 1; // This is the index!!
    pol->texels.Push (uv);
    ind.uv = pol->texels.Length() - 1;
    pol->vertices[verts[i]]->indices.Push(ind);
    indexVert = ind.vertex;
    pol->numVertices++;
  }
  return indexVert;
}

int csTriangleArrayPolygonBuffer::AddSingleVertexLM (
	csTrianglesPerSuperLightmap* triSuperLM,
	int* verts, int i)
{
  if (triSuperLM->vertexIndices[verts[i]] != NULL)
    return triSuperLM->vertexIndices[verts[i]]->indices[0].vertex;
  else
  {
    /*
     * It's is null, we have to create
     * the vertices[verts[i]] array and we can push it directly in
     * the verticesPoints array and the uv's can be pushed directly too
     */
    Indexes ind;
    triSuperLM->vertexIndices[verts[i]] = new csVertexIndexArrayNode;
    triSuperLM->vertices.Push (vertices[verts[i]]);
    ind.vertex = triSuperLM->vertices.Length() - 1; // this is the index!!
    triSuperLM->vertexIndices[verts[i]]->indices.Push (ind);
    triSuperLM->fogInfo.Push(verts[i]);
    triSuperLM->numVertices++;
    return ind.vertex;
  }
}

int csTriangleArrayPolygonBuffer::AddSingleVertexLM (
	csTrianglesPerSuperLightmap* triSuperLM,
	int* verts, int i, const csVector2& uvLightmap)
{
  int indexVert = -1;
  if (triSuperLM->vertexIndices[verts[i]] != NULL)
  {
    int j;
    for (j = 0; j < triSuperLM->vertexIndices[verts[i]]->indices.Length();
        j++)
    {
      indexVert = triSuperLM->vertexIndices[verts[i]]->indices[j].vertex;
      int uvIndex = triSuperLM->vertexIndices[verts[i]]->indices[j].uv;
      //let's see if this vertex has the same uv's
      if (uvLightmap == triSuperLM->texels[uvIndex]) break;
      indexVert = -1;
    }
  }
  else
  {
    /*
     * It's is null, we have to create
     * the vertices[verts[i]] array and we can push it directly in
     * the verticesPoints array and the uv's can be pushed directly too
     */
    triSuperLM->vertexIndices[verts[i]] = new csVertexIndexArrayNode;
  }

  if (indexVert < 0)
  {
    Indexes ind;
    triSuperLM->vertices.Push (vertices[verts[i]]);
    ind.vertex = triSuperLM->vertices.Length () - 1; // this is the index!!
    triSuperLM->texels.Push (uvLightmap);
    ind.uv = triSuperLM->texels.Length() - 1;
    triSuperLM->vertexIndices[verts[i]]->indices.Push(ind);
    triSuperLM->fogInfo.Push (verts[i]);
    triSuperLM->numVertices++;
    triSuperLM->numTexels++;
    indexVert = ind.vertex;
  }
  return indexVert;
}

void csTriangleArrayPolygonBuffer::AddTriangles (csTrianglesPerMaterial* pol,
    csTrianglesPerSuperLightmap* triSuperLM, int* verts, int num_vertices,
    const csMatrix3& m_obj2tex, const csVector3& v_obj2tex,
    iPolygonTexture* poly_texture, int mat_index,
    const csPlane3& /*poly_normal*/)
{
  csVector3 aux;
  csVector2 uv[3];
  int i;
  csTriangle triangle;

  // Triangulate the polygon and add all vertices with correct uv
  // index.
  csTransform transform (m_obj2tex, v_obj2tex);
  aux = transform.Other2This (vertices[verts[0]]);
  uv[0].x = aux.x;
  uv[0].y = aux.y;
  triangle.a = AddSingleVertex (pol, verts, 0, uv[0]);

  for (i = 1; i < num_vertices-1; i++)
  {
    aux = transform.Other2This (vertices[verts[i]]);
    uv[1].x = aux.x;
    uv[1].y = aux.y;
    triangle.b = AddSingleVertex (pol, verts, i, uv[1]);

    aux = transform.Other2This (vertices[verts[i+1]]);
    uv[2].x = aux.x;
    uv[2].y = aux.y;
    triangle.c = AddSingleVertex (pol, verts, i+1, uv[2]);

    pol->triangles.Push (triangle);

    pol->numTriangles++;
  }

  pol->matIndex = mat_index;

  // Lightmap handling.
  csRect rect;
  triSuperLM = SearchFittingSuperLightmap (poly_texture, rect, verticesCount);
  CS_ASSERT (triSuperLM != NULL);

  /*
   * We have the superlightmap where the poly_texture fits
   * Now we can add the triangles
   */

  /*
   * Let's check if it's the unlitPolySL
   */
  if (triSuperLM == unlitPolysSL)
  {
    // It is, we can avoid most of this stuff, no
    // lightmap is needed, only the triangles, vertex
    // and vertex indices are needed.

    /*
     * It's easier than the lightmap case.
     * Considerations:
     * 1) This logic superlightmap will store:
     *    a) The mesh's triangles that are unlit
     *    b) The mesh's vertices
     * 2) We don't need to duplicate vertex
     * 3) We don't need to comapare with uv's
     *
     */
    triangle.a = AddSingleVertexLM (triSuperLM, verts, 0);

    for (i = 1; i < num_vertices - 1; i++)
    {
      triangle.b = AddSingleVertexLM (triSuperLM, verts, i);
      triangle.c = AddSingleVertexLM (triSuperLM, verts, i+1);

      triSuperLM->triangles.Push (triangle);
      triSuperLM->numTriangles++;
    }
    return;
  }

  float lm_low_u, lm_low_v, lm_high_u, lm_high_v;
  float lm_scale_u, lm_scale_v, lm_offset_u, lm_offset_v;

  poly_texture->GetTextureBox (lm_low_u,lm_low_v,lm_high_u,lm_high_v);
  lm_low_u -= 0.125;
  lm_low_v -= 0.125;
  lm_high_u += 0.125;
  lm_high_v += 0.125;

  CS_ASSERT((lm_high_u > lm_low_u) && (lm_high_v > lm_low_v));

  lm_scale_u = 1./(lm_high_u - lm_low_u);
  lm_scale_v = 1./(lm_high_v - lm_low_v);

  iLightMap* piLM = poly_texture->GetLightMap();

  float lm_width = float(piLM->GetWidth());
  float lm_height = float(piLM->GetHeight());
  int superLMsize = OpenGLLightmapCache::super_lm_size;

  float dlm = 1./ float(superLMsize);

  float sup_u = float (rect.xmin) * dlm;
  float sup_v = float (rect.ymin) * dlm;

  lm_scale_u = lm_scale_u * lm_width * dlm;
  lm_scale_v = lm_scale_v * lm_height * dlm;

  lm_offset_u = lm_low_u - sup_u / lm_scale_u;
  lm_offset_v = lm_low_v - sup_v / lm_scale_v;

  // Ok, we have the values, let's generate the triangles

  /*
   * Vertices for lightmap triangles have four coordinates
   * so we generate x,y,z, 1.0 vertices for it.
   * Every vertex is unique (so if two vertices share the
   * same coordinates but different uv's a new vertex is
   * created)
   */
  csVector2 uvLightmap;
  uvLightmap.x = (uv[0].x - lm_offset_u) * lm_scale_u;
  uvLightmap.y = (uv[0].y - lm_offset_v) * lm_scale_v;
  triangle.a = AddSingleVertexLM (triSuperLM, verts, 0, uvLightmap);

  for (i = 1; i < num_vertices - 1; i++)
  {
    uvLightmap.x = (uv[1].x - lm_offset_u) * lm_scale_u;
    uvLightmap.y = (uv[1].y - lm_offset_v) * lm_scale_v;
    triangle.b = AddSingleVertexLM (triSuperLM, verts, i, uvLightmap);

    uvLightmap.x = (uv[2].x - lm_offset_u) * lm_scale_u;
    uvLightmap.y = (uv[2].y - lm_offset_v) * lm_scale_v;
    triangle.c = AddSingleVertexLM (triSuperLM, verts, i+1, uvLightmap);

    triSuperLM->triangles.Push (triangle);
    poly_texture->IncRef ();
    triSuperLM->numTriangles++;
  }

  triSuperLM->rectangles.Push (rect);
  triSuperLM->numLightmaps++;
  triSuperLM->lightmaps.Push (poly_texture);
}

void csTriangleArrayPolygonBuffer::MarkLightmapsDirty()
{
  superLM.MarkLightmapsDirty();
}

void csTriangleArrayPolygonBuffer::AddPolygon (int* verts, int num_verts,
  const csPlane3& poly_normal,
  int mat_index,
  const csMatrix3& m_obj2tex, const csVector3& v_obj2tex,
  iPolygonTexture* poly_texture)
{
  /*
   * We have to:
   * Generate triangles
   * Generate uv per vertex
   * Group the triangles per materials
   * We know this:
   * - The polygons are sent to AddPolygon sorted by material
   * - m_obj2tex and v_obj2tex are the matrix and vector to obtain the uv
   * cordinates
   *
   * We can do the following:
   * For every polygon:
   * if it is the first material add the triangles to the array
   *    and calculate the uv for it's vertices
   * if preceding material is the same that polygon's material then
   *    add the triangles to the preceding material array and calculate the
   *    uv
   * else add the triangles to the next position in the array and calculate
   *    their uv's
   *
   * IMPORTANT: poly_texture is stored per triangle? and normal per vertice?
   * it would be great!
   * - Can be done in load time?, it can, but does this affect to lighting
   * part? Ask Jorrit!
   * How can we this done? Every time an AddPolygon is done you know
   * the face normal, just add it to the vertex normal and normalize
   */

  csTrianglesPerSuperLightmap* triSuperLM = NULL;

  int last_mat_index = polygons.GetLastMaterial ();
  if (last_mat_index != mat_index)
  {
    // First polygon or material of this polygon is different from
    // last material.
    csTrianglesPerMaterial* pol = new csTrianglesPerMaterial (verticesCount);
    AddTriangles (pol, triSuperLM, verts, num_verts, m_obj2tex, v_obj2tex,
      poly_texture, mat_index, poly_normal);
    TrianglesNode* tNode = new TrianglesNode ();
    tNode->info = pol;
    polygons.Add (tNode);

    matCount ++;
  }
  else
  {
    // We can add the triangles in the last PolygonPerMaterial
    // as long they share the same material.
    AddTriangles (polygons.last->info, triSuperLM, verts, num_verts, m_obj2tex,
      v_obj2tex, poly_texture, mat_index, poly_normal);
  }
}

int csTriangleArrayPolygonBuffer::GetSuperLMCount()
{
   return superLM.numElems;
}

csSLMCacheData* csTriangleArrayPolygonBuffer::GetCacheData(
  TrianglesSuperLightmapNode* t)
{
  return t->info->cacheData;
}

int csTriangleArrayPolygonBuffer::GetLightmapCount(
  TrianglesSuperLightmapNode* t)
{
  return t->info->numLightmaps;
}

TrianglesSuperLightmapNode* csTriangleArrayPolygonBuffer::GetFirstTrianglesSLM()
{
  return superLM.last;
}

TrianglesSuperLightmapNode* csTriangleArrayPolygonBuffer::GetNextTrianglesSLM(
  TrianglesSuperLightmapNode* t)
{
  return t->prev;
}

csVector3* csTriangleArrayPolygonBuffer::GetVerticesPerSuperLightmap(
  TrianglesSuperLightmapNode* t)
{
  return t->info->vertices.GetArray();
}

csVector2* csTriangleArrayPolygonBuffer::GetUV(
  TrianglesSuperLightmapNode* t)
{
  return t->info->texels.GetArray();
}

csTriangle* csTriangleArrayPolygonBuffer::GetTriangles(
  TrianglesSuperLightmapNode* t)
{
  return t->info->triangles.GetArray();
}

int csTriangleArrayPolygonBuffer::GetTriangleCount(
  TrianglesSuperLightmapNode* t)
{
  return t->info->numTriangles;
}

int csTriangleArrayPolygonBuffer::GetVertexCount(
  TrianglesSuperLightmapNode* t)
{
  return t->info->numVertices;
}

int * csTriangleArrayPolygonBuffer::
GetFogIndices(TrianglesSuperLightmapNode*tSL)
{
  return tSL->info->fogInfo.GetArray();
}

void csTriangleArrayPolygonBuffer::SetVertexArray (csVector3* verts,
  int num_verts)
{
  delete[] vertices;
  vertices = new csVector3 [num_verts];
  memcpy (vertices, verts, num_verts * sizeof (csVector3));
  normals.SetLimit(num_verts);
  verticesCount = num_verts;
}

void csTriangleArrayPolygonBuffer::AddMaterial (iMaterialHandle* mat_handle)
{
  materials.Push (mat_handle);
}

void csTriangleArrayPolygonBuffer::SetMaterial (int idx,
  iMaterialHandle* mat_handle)
{
  materials[idx] = mat_handle;
}

void csTriangleArrayPolygonBuffer::Clear ()
{
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!Falta el Clear
  delete[] vertices; vertices = NULL;
  materials.SetLimit(0);
  delete unlitPolysSL;
}

csTriangle* csTriangleArrayPolygonBuffer::GetTriangles(TrianglesNode* t)
{
  return t->info->triangles.GetArray();
}

int csTriangleArrayPolygonBuffer::GetTriangleCount(TrianglesNode* t)
{
  return t->info->numTriangles;
}

int csTriangleArrayPolygonBuffer::GetVertexCount(TrianglesNode* t)
{
  return t->info->numVertices;
}

int csTriangleArrayPolygonBuffer::GetMatIndex(TrianglesNode* t)
{
  return t->info->matIndex;
}

csVector2* csTriangleArrayPolygonBuffer::GetUV(TrianglesNode* t)
{
  return t->info->texels.GetArray();
}

int csTriangleArrayPolygonBuffer::GetUVCount(TrianglesNode* t)
{
  return t->info->texels.Length();
}

/*iPolyTex_p* csTriangleArrayPolygonBuffer::GetLightMaps(TrianglesNode* t)
{
  return t->info->infoPolygons.GetArray();
}
*/


TrianglesNode* csTriangleArrayPolygonBuffer::GetFirst()
{
  return polygons.first;
}

TrianglesNode* csTriangleArrayPolygonBuffer::GetNext(TrianglesNode* t)
{
  if (t == NULL) return NULL; //better if exit 1?
  return t->next;
}

csVector3* csTriangleArrayPolygonBuffer::GetVerticesPerMaterial(
  TrianglesNode* t)
{
  return t->info->verticesPoints.GetArray();
}

csTriangleArrayVertexBufferManager::csTriangleArrayVertexBufferManager
  (iObjectRegistry* object_reg) : csVertexBufferManager (object_reg)
{
}

csTriangleArrayVertexBufferManager::~csTriangleArrayVertexBufferManager()
{
}

iPolygonBuffer* csTriangleArrayVertexBufferManager::CreatePolygonBuffer ()
{
  csTriangleArrayPolygonBuffer* buf = new csTriangleArrayPolygonBuffer (this);
  return buf;
}

