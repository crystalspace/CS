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


csVector3& csVector4::GetcsVector3()
{
  csVector3* v = new csVector3(x,y,z);
  return *v;
}


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


csTriangleArrayPolygonBuffer::csTriangleArrayPolygonBuffer (iVertexBufferManager* mgr)
	: csPolygonBuffer (mgr)
{

  vertices = NULL;
  matCount = 0;
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
  vertices.SetLimit(numVertex);
  verticesPoints.SetLimit(numVertex);
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

void csTrianglesPerMaterial::CopyInfoPolygons()
{
  /*triangleInfo = new csTriangleInfo[numTriangles];
  int i;

  triangleInfo = infoPolygons.GetArray();
  for(i = 0; i < numTriangles; i++)
     delete infoPolygons[i];
  
  infoPolygons.SetLength(0);
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

  vertexIndices.SetLength(numVertex);

  int i;

  for(i = 0; i < numVertex; i++) vertexIndices[i] = NULL;
}

csTrianglesPerSuperLightmap::~csTrianglesPerSuperLightmap()
{
  int i;
  for(i = 0; i < numLightmaps; i++)
    lightmaps[i]->DecRef();
  int numIndices = vertexIndices.Length();
  for(i = 0; i < numIndices; i++)
  {
    if(vertexIndices[i] != NULL) 
    {      
      delete vertexIndices[i];
    }
  }  
  if(cacheData) cacheData->Clear();
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



/** Search a superlightmap to fit the lighmap in the superLM list
 * if it can't find any creates a new one
 */
csTrianglesPerSuperLightmap* csTriangleArrayPolygonBuffer::
           SearchFittingSuperLightmap(iPolygonTexture* poly_texture,
           csRect& rect,int num_vertices)
{

  int i;
  iLightMap* piLM = poly_texture->GetLightMap();
  int lm_width = piLM->GetWidth();
  int lm_height = piLM->GetHeight();

  TrianglesSuperLightmapNode* curr = superLM.last;
  for(i = 0; i < superLM.numElems; i++)
  {
    if(curr->info->region->Alloc(lm_width,lm_height,rect)) 
      return curr->info;    
    curr = curr->prev;
  }

  //We haven't found any, let's create a new one
  
  
  curr = new TrianglesSuperLightmapNode();
  curr->info = new csTrianglesPerSuperLightmap(verticesCount);

  if(!curr->info->region->Alloc(lm_width,lm_height,rect))
  {  
    return NULL;
  }

  superLM.Add(curr);    
  return curr->info;


}


void csTriangleArrayPolygonBuffer::AddTriangles(csTrianglesPerMaterial* pol,
    csTrianglesPerSuperLightmap* triSuperLM, int* verts, int num_vertices, 
    const csMatrix3& m_obj2tex, const csVector3& v_obj2tex, 
    iPolygonTexture* poly_texture, int mat_index, const csPlane3& poly_normal)
{

  csVector3 aux;
  csVector2 uv[3];
  csVector2 uvLightmap[3];
  int i;
  csTriangle triangle;  
  unsigned char r,g,b;
  csColor col;

  
  csTransform *transform  = new csTransform(m_obj2tex, v_obj2tex);
  aux = transform->Other2This(vertices[verts[0]]);
  uv[0].x = aux.x;
  uv[0].y = aux.y;


  if(pol->vertices[verts[0]] != NULL)
  {
    //Let's see if the vertex is already in the vertices array
    /* A vertex is in the vertices array if:
     * - pol->vertices[verts[0]] != NULL AND
     * - In pol->vertices[verts[0]] array exists an index that points
     * to a vertex that has the same uv's
     */
    int j;
    int indexVert = -1;
    for(j = 0; j < pol->vertices[verts[0]]->indices.Length(); j++)
    {
      indexVert = pol->vertices[verts[0]]->indices[j].vertex;
      int uvIndex = pol->vertices[verts[0]]->indices[j].uv;
      //let's see if this vertex has the same uv's
      if(uv[0] == pol->texels[uvIndex]) break;
      indexVert = -1;
    }
    if(indexVert < 0)
    {
     // It wasn't in the previous vertices.
     // We have to create a new vertex.
     // Where? We're going to push it in the verticesPoints
     // that position will give us an index, and that index is
     // the one we will push in the vertices array, position verts[0]
     Indexes ind;
     pol->verticesPoints.Push(vertices[verts[0]]);
     ind.vertex = pol->verticesPoints.Length() - 1; // this is the index!!                 
     pol->texels.Push(uv[0]);
     ind.uv = pol->texels.Length() - 1;
     pol->vertices[verts[0]]->indices.Push(ind);
     triangle.a = ind.vertex;
      
     materials[mat_index]->GetTexture()->GetMeanColor(r,g,b);
     pol->colors.Push(col);
     pol->numVertices++;
      
    }
    else triangle.a = indexVert;
  }
  else
  {
    /* It's is null, we have to create
     * the vertices[verts[0]] array and we can push it directly in
     * the verticesPoints array and the uv's can be pushed directly too
     */
    Indexes ind;
    pol->vertices[verts[0]] = new csVertexIndexArrayNode;
    pol->verticesPoints.Push(vertices[verts[0]]);
    ind.vertex = pol->verticesPoints.Length() - 1;
    pol->texels.Push(uv[0]);
    ind.uv = pol->texels.Length()-1;
    pol->vertices[verts[0]]->indices.Push(ind);
    triangle.a = ind.vertex;
    materials[mat_index]->GetTexture()->GetMeanColor(r,g,b);
    col.red = float(r)/255.;
    col.green = float(g)/255.;
    col.blue = float(b)/255.;
    pol->colors.Push(col);
    pol->numVertices++;
  }
     
   
  for(i = 1; i < num_vertices-1; i++)
  {

    aux = transform->Other2This(vertices[verts[i]]);
    uv[1].x = aux.x;
    uv[1].y = aux.y;
      

    aux = transform->Other2This(vertices[verts[i+1]]);
    uv[2].x = aux.x;
    uv[2].y = aux.y;
      
      
    if(pol->vertices[verts[i]] != NULL)
    {
      //Let's see if the vertex is already in the vertices array
      /* A vertex is in the vertices array if:
       * - pol->vertices[verts[0]] != NULL AND
       * - In pol->vertices[verts[0]] array exists an index that points
       * to a vertex that has the same uv's
       */
      int j;
      int indexVert = -1;
      for(j = 0; j < pol->vertices[verts[i]]->indices.Length(); j++)
      {
        indexVert = pol->vertices[verts[i]]->indices[j].vertex;
        int uvIndex = pol->vertices[verts[i]]->indices[j].uv;
        //let's see if this vertex has the same uv's
        if(uv[1] == pol->texels[uvIndex]) break;
        indexVert = -1;
      }
      if(indexVert < 0)
      {
        // It wasn't in the previous vertices.
        // We have to create a new vertex.
        // Where? We're going to push it in the verticesPoints
        // that position will give us an index, and that index is
        // the one we will push in the vertices array, position verts[0]
        Indexes ind;
        pol->verticesPoints.Push(vertices[verts[i]]);
        ind.vertex = pol->verticesPoints.Length() - 1; // this is the index!!                 
        pol->texels.Push(uv[1]);
        ind.uv = pol->texels.Length() - 1;
        pol->vertices[verts[i]]->indices.Push(ind);
        triangle.b = ind.vertex;
        materials[mat_index]->GetTexture()->GetMeanColor(r,g,b);
        col.red = float(r)/255.;
        col.green = float(g)/255.;
        col.blue = float(b)/255.;
        pol->colors.Push(col);
        pol->numVertices++;
      }
      else triangle.b = indexVert;
    }
    else
    {
      /* It's is null, we have to create
       * the vertices[verts[0]] array and we can push it directly in
       * the verticesPoints array and the uv's can be pushed directly too
       */
      Indexes ind;
      pol->vertices[verts[i]] = new csVertexIndexArrayNode;
      pol->verticesPoints.Push(vertices[verts[i]]);
      ind.vertex = pol->verticesPoints.Length() - 1; // this is the index!!                 
      pol->texels.Push(uv[1]);
      ind.uv = pol->texels.Length() - 1;
      pol->vertices[verts[i]]->indices.Push(ind);
      triangle.b = ind.vertex;
      materials[mat_index]->GetTexture()->GetMeanColor(r,g,b);
      col.red = float(r)/255.;
      col.green = float(g)/255.;
      col.blue = float(b)/255.;
      pol->colors.Push(col);
      pol->numVertices++;
    }

    if(pol->vertices[verts[i+1]]!= NULL)
    {
     //Let's see if the vertex is already in the vertices array
     /* A vertex is in the vertices array if:
      * - pol->vertices[verts[0]] != NULL AND
      * - In pol->vertices[verts[0]] array exists an index that points
      * to a vertex that has the same uv's
      */
     int j;
     int indexVert = -1;
     for(j = 0; j < pol->vertices[verts[i+1]]->indices.Length(); j++)
     {
        indexVert = pol->vertices[verts[i+1]]->indices[j].vertex;
        int uvIndex = pol->vertices[verts[i+1]]->indices[j].uv;
        //let's see if this vertex has the same uv's
        if(uv[2] == pol->texels[uvIndex]) break;
        indexVert = -1;
     }
     if(indexVert < 0)
     {

       // It wasn't in the previous vertices.
       // We have to create a new vertex.
       // Where? We're going to push it in the verticesPoints
       // that position will give us an index, and that index is
       // the one we will push in the vertices array, position verts[0]
       Indexes ind;
       pol->verticesPoints.Push(vertices[verts[i+1]]);
       ind.vertex = pol->verticesPoints.Length() - 1; // this is the index!!                 
       pol->texels.Push(uv[2]);
       ind.uv = pol->texels.Length() - 1;
       pol->vertices[verts[i+1]]->indices.Push(ind);
       triangle.c = ind.vertex;
       materials[mat_index]->GetTexture()->GetMeanColor(r,g,b);
       col.red = float(r)/255.;
       col.green = float(g)/255.;
       col.blue = float(b)/255.;
       pol->colors.Push(col);
       pol->numVertices++;
     }
     else triangle.c = indexVert;
    }
    else
    {
      /* It's is null, we have to create
       * the vertices[verts[0]] array and we can push it directly in
       * the verticesPoints array and the uv's can be pushed directly too
       */
      Indexes ind;
      pol->vertices[verts[i+1]] = new csVertexIndexArrayNode;
      pol->verticesPoints.Push(vertices[verts[i+1]]);
      ind.vertex = pol->verticesPoints.Length() - 1;
      pol->texels.Push(uv[2]);
      ind.uv = pol->texels.Length() -1;
      pol->vertices[verts[i+1]]->indices.Push(ind);
      triangle.c = ind.vertex;
      materials[mat_index]->GetTexture()->GetMeanColor(r,g,b);
      col.red = float(r)/255.;
      col.green = float(g)/255.;
      col.blue = float(b)/255.;
      pol->colors.Push(col);
      pol->numVertices++;
    }
              
    pol->triangles.Push(triangle);
    pol->infoPolygons.Push(poly_texture);    
    poly_texture->IncRef();
    
    pol->numTriangles++;           
  }
    
  pol->matIndex = mat_index;
  

  
  //Lightmap handling


  csRect rect;
  triSuperLM = SearchFittingSuperLightmap(poly_texture, rect,verticesCount);
  CS_ASSERT(triSuperLM != NULL);
    /* We have the superlightmap where the poly_texture fits
   * Now we can add the triangles
   */
  float lm_low_u,lm_low_v,lm_high_u,lm_high_v;
  float lm_scale_u, lm_scale_v, lm_offset_u, lm_offset_v;
  
  poly_texture->GetTextureBox(lm_low_u,lm_low_v,lm_high_u,lm_high_v);
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


  //Ok, we have the values, let's generate the triangles

  /* Vertices for lightmap triangles have four coordinates
   * so we generate x,y,z, 1.0 vertices for it.
   * Every vertex is unique (so if two vertices share the
   * same coordinates but different uv's a new vertex is
   * created)
   */

  

  uvLightmap[0].x = (uv[0].x - lm_offset_u) * lm_scale_u;
  uvLightmap[0].y = (uv[0].y - lm_offset_v) * lm_scale_v;
  int k;

  
  if(triSuperLM->vertexIndices[verts[0]] != NULL)
  {
    //The vertices could exist already, let's check
    int indexVertex = -1;
    for(k = 0; k < triSuperLM->vertexIndices[verts[0]]->indices.Length(); k++)
    {
      indexVertex = triSuperLM->vertexIndices[verts[0]]->indices[k].vertex;
      int indexUV = triSuperLM->vertexIndices[verts[0]]->indices[k].uv;
      if(uvLightmap[0] == triSuperLM->texels[indexUV]) break;
      indexVertex = -1;
    }
    if (indexVertex < 0)
    {
      /* The vertex isn't in the previous stored vertices, so let's
       * create it
       */
      Indexes ind;
      csVector4 point;
      point.x = vertices[verts[0]].x;
      point.y = vertices[verts[0]].y;
      point.z = vertices[verts[0]].z;
      point.w = 1.0;

      triSuperLM->vertices.Push(point);
      ind.vertex = triSuperLM->vertices.Length() - 1; // this is the index!!                 
      triSuperLM->texels.Push(uvLightmap[0]);
      ind.uv = triSuperLM->texels.Length() - 1;
      triSuperLM->vertexIndices[verts[0]]->indices.Push(ind);      
      triSuperLM->numTexels++;
      triSuperLM->numVertices++;
      triSuperLM->fogInfo.Push(verts[0]);
      triangle.a = ind.vertex;
      
    }
    else
    {
      //The vertex already exists
      triangle.a = indexVertex;
    }
  }
  else
  {
    //There is no vertices in the index. We have to create it.
    csVector4 point;
    Indexes ind;
    
    point.x = vertices[verts[0]].x;
    point.y = vertices[verts[0]].y;
    point.z = vertices[verts[0]].z;
    point.w = 1.0;

    
    triSuperLM->vertexIndices[verts[0]] = new csVertexIndexArrayNode;
    triSuperLM->vertices.Push(point);
    ind.vertex = triSuperLM->vertices.Length() - 1;
    triSuperLM->texels.Push(uvLightmap[0]);
    ind.uv = triSuperLM->texels.Length() -1;
    triSuperLM->vertexIndices[verts[0]]->indices.Push(ind);    
    
    triSuperLM->fogInfo.Push(verts[0]);
    triSuperLM->numVertices++;
    triSuperLM->numTexels++;
    triangle.a = ind.vertex;
  }

  for(i = 1; i < num_vertices - 1; i++)
  {
    aux = transform->Other2This(vertices[verts[i]]);
    uvLightmap[1].x = (aux.x - lm_offset_u) * lm_scale_u;
    uvLightmap[1].y = (aux.y - lm_offset_v) * lm_scale_v;
      

    aux = transform->Other2This(vertices[verts[i+1]]);
    uvLightmap[2].x = (aux.x - lm_offset_u) * lm_scale_u;
    uvLightmap[2].y = (aux.y - lm_offset_v) * lm_scale_v;
      
      
    if(triSuperLM->vertexIndices[verts[i]] != NULL)
    {      
      int j;
      int indexVert = -1;
      for(j = 0; j < triSuperLM->vertexIndices[verts[i]]->indices.Length(); 
          j++)
      {
        indexVert = triSuperLM->vertexIndices[verts[i]]->indices[j].vertex;
        int uvIndex = triSuperLM->vertexIndices[verts[i]]->indices[j].uv;
        //let's see if this vertex has the same uv's
        if(uvLightmap[1] == triSuperLM->texels[uvIndex]) break;
        indexVert = -1;
      }
      if(indexVert < 0)
      {     
        Indexes ind;
        csVector4 point;

        point.x = vertices[verts[i]].x;
        point.y = vertices[verts[i]].y;
        point.z = vertices[verts[i]].z;
        point.w = 1.0;


        triSuperLM->vertices.Push(point);
        ind.vertex = triSuperLM->vertices.Length() - 1; // this is the index!!                 
        triSuperLM->texels.Push(uvLightmap[1]);
        ind.uv = triSuperLM->texels.Length() - 1;
        triSuperLM->vertexIndices[verts[i]]->indices.Push(ind);
        triSuperLM->fogInfo.Push(verts[i]);
        triSuperLM->numVertices++;
        triSuperLM->numTexels++;
        triangle.b = ind.vertex;

      }
      else triangle.b = indexVert;
    }
    else
    {
      /* It's is null, we have to create
       * the vertices[verts[i]] array and we can push it directly in
       * the verticesPoints array and the uv's can be pushed directly too
       */
      Indexes ind;
      csVector4 point;

      point.x = vertices[verts[i]].x;
      point.y = vertices[verts[i]].y;
      point.z = vertices[verts[i]].z;
      point.w = 1.0;

      triSuperLM->vertexIndices[verts[i]] = new csVertexIndexArrayNode;
      triSuperLM->vertices.Push(point);
      ind.vertex = triSuperLM->vertices.Length() - 1; // this is the index!!                 
      triSuperLM->texels.Push(uvLightmap[1]);
      ind.uv = triSuperLM->texels.Length() - 1;
      triSuperLM->vertexIndices[verts[i]]->indices.Push(ind);
      triSuperLM->fogInfo.Push(verts[i]);
      triSuperLM->numVertices++;
      triSuperLM->numTexels++;
      triangle.b = ind.vertex;
    }

    if(triSuperLM->vertexIndices[verts[i+1]] != NULL)
    {
     int j;
     int indexVert = -1;
     for(j = 0; j < triSuperLM->vertexIndices[verts[i+1]]->indices.Length(); j++)
     {
        indexVert = triSuperLM->vertexIndices[verts[i+1]]->indices[j].vertex;
        int uvIndex = triSuperLM->vertexIndices[verts[i+1]]->indices[j].uv;
        //let's see if this vertex has the same uv's
        if(uvLightmap[2] == triSuperLM->texels[uvIndex]) break;
        indexVert = -1;
     }
     if(indexVert < 0)
     {

       // It wasn't in the previous vertices.
       // We have to create a new vertex.
       // Where? We're going to push it in the verticesPoints
       // that position will give us an index, and that index is
       // the one we will push in the vertices array, position verts[i+1]
       Indexes ind;
       csVector4 point;

       point.x = vertices[verts[i+1]].x;
       point.y = vertices[verts[i+1]].y;
       point.z = vertices[verts[i+1]].z;
       point.w = 1.0;

       triSuperLM->vertices.Push(point);
       ind.vertex = triSuperLM->vertices.Length() - 1; // this is the index!!                 
       triSuperLM->texels.Push(uvLightmap[2]);
       ind.uv = triSuperLM->texels.Length() - 1;
       triSuperLM->vertexIndices[verts[i+1]]->indices.Push(ind);       
       triSuperLM->fogInfo.Push(verts[i+1]);
       triSuperLM->numVertices++;
       triSuperLM->numTexels++;
       triangle.c = ind.vertex;
       
     }
     else triangle.c = indexVert;
    }
    else
    {
      Indexes ind;
      csVector4 point;
      point.x = vertices[verts[i+1]].x;
      point.y = vertices[verts[i+1]].y;
      point.z = vertices[verts[i+1]].z;
      point.w = 1.0;
      
      triSuperLM->vertexIndices[verts[i+1]] = new csVertexIndexArrayNode;
      triSuperLM->vertices.Push(point);
      ind.vertex = triSuperLM->vertices.Length() - 1;
      triSuperLM->texels.Push(uvLightmap[2]);
      ind.uv = triSuperLM->texels.Length() -1;
      triSuperLM->vertexIndices[verts[i+1]]->indices.Push(ind);
      triSuperLM->fogInfo.Push(verts[i+1]);
      triSuperLM->numVertices++;
      triSuperLM->numTexels++;
      triangle.c = ind.vertex;
    }  
    triSuperLM->triangles.Push(triangle);   
    poly_texture->IncRef();
    triSuperLM->numTriangles++;
    
  }
  triSuperLM->rectangles.Push(rect);  
  triSuperLM->numLightmaps++;
  triSuperLM->lightmaps.Push(poly_texture);
  poly_texture->IncRef();
}


void csTriangleArrayPolygonBuffer::AddPolygon (int* verts, int num_verts,
	const csPlane3& poly_normal,
  	int mat_index,
	const csMatrix3& m_obj2tex, const csVector3& v_obj2tex,
	iPolygonTexture* poly_texture)
{
  /* We have to:
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


  

  
  
  
  if (polygons.GetLastMaterial() < 0)
  {
  
    csTrianglesPerMaterial* pol = new csTrianglesPerMaterial(verticesCount);
    
    AddTriangles(pol,triSuperLM,verts,num_verts,m_obj2tex,v_obj2tex,poly_texture,
      mat_index,poly_normal);
    TrianglesNode* tNode = new TrianglesNode();
    tNode->info = pol;

    
    polygons.Add(tNode);
  
    matCount ++;
  }
  else
  {
    if(polygons.GetLastMaterial() == mat_index)
    { //we can add the triangles in the last PolygonPerMaterial
      // as long they share the same material
     AddTriangles(polygons.last->info,triSuperLM,verts,num_verts,m_obj2tex,
       v_obj2tex, poly_texture,mat_index,poly_normal);
    }
    else
    {
      // It is a new material. It's the same code that if it was the first      
      //We have to triangularizate it
      csTrianglesPerMaterial* pol = new csTrianglesPerMaterial(verticesCount);
      AddTriangles(pol,triSuperLM,verts,num_verts,m_obj2tex,v_obj2tex,poly_texture,
        mat_index,poly_normal);
      TrianglesNode* tNode = new TrianglesNode();
      tNode->info = pol;
      polygons.Add(tNode);        
      matCount ++;

    }
  }


}

int csTriangleArrayPolygonBuffer::GetSuperLMCount()
{
   return superLM.numElems;   
}
csSLMCacheData* csTriangleArrayPolygonBuffer::GetCacheData(TrianglesSuperLightmapNode* t)
{
  return t->info->cacheData;
}
int csTriangleArrayPolygonBuffer::GetLightmapCount(TrianglesSuperLightmapNode* t)
{
  return t->info->numLightmaps;
}
TrianglesSuperLightmapNode* csTriangleArrayPolygonBuffer::GetFirstTrianglesSLM()
{
  return superLM.last;
}

TrianglesSuperLightmapNode* csTriangleArrayPolygonBuffer::GetNextTrianglesSLM(TrianglesSuperLightmapNode* t)
{
  return t->prev;
}
csVector4* csTriangleArrayPolygonBuffer::GetVerticesPerSuperLightmap(TrianglesSuperLightmapNode* t)
{
  return t->info->vertices.GetArray();
}
csVector2* csTriangleArrayPolygonBuffer::GetUV(TrianglesSuperLightmapNode* t)
{
  return t->info->texels.GetArray();
}
csTriangle* csTriangleArrayPolygonBuffer::
GetTriangles(TrianglesSuperLightmapNode* t)
{
  return t->info->triangles.GetArray();
}

int csTriangleArrayPolygonBuffer::GetTriangleCount(TrianglesSuperLightmapNode* t)
{
  return t->info->numTriangles;
}

int csTriangleArrayPolygonBuffer::GetVertexCount(TrianglesSuperLightmapNode* t)
{
  return t->info->numVertices;
}

int * csTriangleArrayPolygonBuffer::
GetFogIndices(TrianglesSuperLightmapNode*tSL)
{
  return tSL->info->fogInfo.GetArray();
}



void csTriangleArrayPolygonBuffer::SetVertexArray (csVector3* verts, int num_verts)
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

csVector3* csTriangleArrayPolygonBuffer::GetVerticesPerMaterial(TrianglesNode* t)
{
  return t->info->verticesPoints.GetArray();
}


csColor* csTriangleArrayPolygonBuffer::GetColors(TrianglesNode*t)
{
  return t->info->colors.GetArray();
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

