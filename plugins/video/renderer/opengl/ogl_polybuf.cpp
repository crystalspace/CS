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

void csTriangleArrayPolygonBuffer::AddTriangles(csTrianglesPerMaterial* pol,
    int* verts, int num_vertices, const csMatrix3& m_obj2tex, 
    const csVector3& v_obj2tex, iPolygonTexture* poly_texture, int mat_index,
    const csPlane3& poly_normal)
{

  csVector3 aux;
  csVector2 uv[2];
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
    int indexVert;
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
      int indexVert;
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
     int indexVert;
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
  
  
  csTriangle triangle;
  csVector2 uv[3];
  csColor col;
  unsigned char r,g,b;
  first_time_rendering = true;


  


  if (polygons.GetLastMaterial() < 0)
  {
  
    csTrianglesPerMaterial* pol = new csTrianglesPerMaterial(verticesCount);
    AddTriangles(pol,verts,num_verts,m_obj2tex,v_obj2tex,poly_texture,
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
     AddTriangles(polygons.last->info,verts,num_verts,m_obj2tex,
       v_obj2tex, poly_texture,mat_index,poly_normal);
    }
    else
    {
      // It is a new material. It's the same code that if it was the first      
      //We have to triangularizate it
      csTrianglesPerMaterial* pol = new csTrianglesPerMaterial(verticesCount);
      AddTriangles(pol,verts,num_verts,m_obj2tex,v_obj2tex,poly_texture,
        mat_index,poly_normal);
      TrianglesNode* tNode = new TrianglesNode();
      tNode->info = pol;
      polygons.Add(tNode);        
      matCount ++;
    }
  }
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
  int i;

  materials.SetLength (0);
  delete[] vertices; vertices = NULL;  
  
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

iPolyTex_p* csTriangleArrayPolygonBuffer::GetLightMaps(TrianglesNode* t)            
{
  return t->info->infoPolygons.GetArray();

}


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

