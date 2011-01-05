/*
    Copyright (C) 2005 by Jorrit Tyberghein

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
#include "cssysdef.h"
#include "csqsqrt.h"

#include "cstool/vertexcompress.h"

#include "csgeom/math3d.h"
#include "csgfx/renderbuffer.h"
#include "csgfx/vertexlistwalker.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/sysfunc.h"

#include "ivideo/rndbuf.h"

static int compare_vt_orig (const void *p1, const void *p2)
{
  csCompressVertexInfo *sp1 = (csCompressVertexInfo *)p1;
  csCompressVertexInfo *sp2 = (csCompressVertexInfo *)p2;
  if (sp1->orig_idx < sp2->orig_idx)
    return -1;
  else if (sp1->orig_idx > sp2->orig_idx)
    return 1;
  return 0;
}

static int compare_vt_full (const void *p1, const void *p2)
{
  csCompressVertexInfo *sp1 = (csCompressVertexInfo *)p1;
  csCompressVertexInfo *sp2 = (csCompressVertexInfo *)p2;
  if (sp1->x < sp2->x)
    return -1;
  else if (sp1->x > sp2->x)
    return 1;
  if (sp1->y < sp2->y)
    return -1;
  else if (sp1->y > sp2->y)
    return 1;
  if (sp1->z < sp2->z)
    return -1;
  else if (sp1->z > sp2->z)
    return 1;

  if (sp1->u < sp2->u)
    return -1;
  else if (sp1->u > sp2->u)
    return 1;
  if (sp1->v < sp2->v)
    return -1;
  else if (sp1->v > sp2->v)
    return 1;

  if (sp1->nx < sp2->nx)
    return -1;
  else if (sp1->nx > sp2->nx)
    return 1;
  if (sp1->ny < sp2->ny)
    return -1;
  else if (sp1->ny > sp2->ny)
    return 1;
  if (sp1->nz < sp2->nz)
    return -1;
  else if (sp1->nz > sp2->nz)
    return 1;

  if (sp1->r < sp2->r)
    return -1;
  else if (sp1->r > sp2->r)
    return 1;
  if (sp1->g < sp2->g)
    return -1;
  else if (sp1->g > sp2->g)
    return 1;
  if (sp1->b < sp2->b)
    return -1;
  else if (sp1->b > sp2->b)
    return 1;
  if (sp1->a < sp2->a)
    return -1;
  else if (sp1->a > sp2->a)
    return 1;

  return 0;
}

template <class T, class U, class C>
static csCompressVertexInfo* TemplatedCompressVertices (
	T& vertices, U& texels, T& normals, C& colors,
	size_t num_vertices, bool doColors,
	csVector3*& new_vertices, csVector2*& new_texels,
	csVector3*& new_normals, csColor4*& new_colors, size_t& new_count)
{
  new_vertices = 0;
  new_texels = 0;
  new_normals = 0;
  new_colors = 0;
  new_count = 0;
  if (num_vertices <= 0) return 0;

  // Copy all the vertices.
  csCompressVertexInfo *vt = new csCompressVertexInfo[num_vertices];
  size_t i, j;
  for (i = 0; i < num_vertices; i++)
  {
    vt[i].orig_idx = i;
    vt[i].x = (int)ceil (vertices[i].x * 1000000);
    vt[i].y = (int)ceil (vertices[i].y * 1000000);
    vt[i].z = (int)ceil (vertices[i].z * 1000000);
    vt[i].u = (int)ceil (texels[i].x * 1000000);
    vt[i].v = (int)ceil (texels[i].y * 1000000);
    vt[i].nx = (int)ceil (normals[i].x * 1000000);
    vt[i].ny = (int)ceil (normals[i].y * 1000000);
    vt[i].nz = (int)ceil (normals[i].z * 1000000);
    if (doColors)
    {
      vt[i].r = (int)ceil (colors[i].red * 1000000);
      vt[i].g = (int)ceil (colors[i].green * 1000000);
      vt[i].b = (int)ceil (colors[i].blue * 1000000);
      vt[i].a = (int)ceil (colors[i].alpha * 1000000);
    }
    else
    {
      vt[i].r = vt[i].g = vt[i].b = vt[i].a = 0;
    }
  }

  // First sort so that all (nearly) equal vertices are together.
  qsort (vt, num_vertices, sizeof (csCompressVertexInfo), compare_vt_full);

  // Count unique values and tag all doubles with the index of the unique one.
  // new_idx in the vt table will be the index inside vt to the unique vector.
  new_count = 1;
  size_t last_unique = 0;
  vt[0].new_idx = last_unique;
  for (i = 1; i < num_vertices; i++)
  {
    if (
      vt[i].x != vt[last_unique].x ||
      vt[i].y != vt[last_unique].y ||
      vt[i].z != vt[last_unique].z ||
      vt[i].u != vt[last_unique].u ||
      vt[i].v != vt[last_unique].v ||
      vt[i].nx != vt[last_unique].nx ||
      vt[i].ny != vt[last_unique].ny ||
      vt[i].nz != vt[last_unique].nz ||
      vt[i].r != vt[last_unique].r ||
      vt[i].g != vt[last_unique].g ||
      vt[i].b != vt[last_unique].b ||
      vt[i].a != vt[last_unique].a)
    {
      last_unique = i;
      new_count++;
    }

    vt[i].new_idx = last_unique;
  }

  // If new_count == num_vertices then there is nothing to do.
  if (new_count == num_vertices)
  {
    delete[] vt;
    return 0;
  }

  // Now allocate and fill new vertex tables.
  // After this new_idx in the vt table will be the new index
  // of the vector.
  new_vertices = new csVector3[new_count];
  new_vertices[0] = vertices[vt[0].orig_idx];
  new_texels = new csVector2[new_count];
  new_texels[0] = texels[vt[0].orig_idx];
  new_normals = new csVector3[new_count];
  new_normals[0] = normals[vt[0].orig_idx];
  if (doColors)
  {
    new_colors = new csColor4[new_count];
    new_colors[0] = colors[vt[0].orig_idx];
  }
  else
    new_colors = 0;

  vt[0].new_idx = 0;
  j = 1;
  for (i = 1; i < num_vertices; i++)
  {
    if (vt[i].new_idx == i)
    {
      new_vertices[j] = vertices[vt[i].orig_idx];
      new_texels[j] = texels[vt[i].orig_idx];
      new_normals[j] = normals[vt[i].orig_idx];
      if (doColors) new_colors[j] = colors[vt[i].orig_idx];
      vt[i].new_idx = j;
      j++;
    }
    else
    {
      vt[i].new_idx = j - 1;
    }
  }

  // Now we sort the table back on orig_idx so that we have
  // a mapping from the original indices to the new one (new_idx).
  qsort (vt, num_vertices, sizeof (csCompressVertexInfo),
  	compare_vt_orig);

  return vt;
}

csCompressVertexInfo* csVertexCompressor::Compress (
	csVector3* vertices, csVector2* texels, csVector3* normals,
	csColor4* colors,
	size_t num_vertices,
	csVector3*& new_vertices, csVector2*& new_texels,
	csVector3*& new_normals, csColor4*& new_colors, size_t& new_count)
{
  return TemplatedCompressVertices (vertices, texels, normals, colors,
  	num_vertices, colors != 0,
  	new_vertices, new_texels, new_normals, new_colors, new_count);
}

csCompressVertexInfo* csVertexCompressor::Compress (
	csArray<csVector3>& vertices,
	csArray<csVector2>& texels,
	csArray<csVector3>& normals,
	csArray<csColor4>& colors)
{
  csVector3* new_vertices;
  csVector2* new_texels;
  csVector3* new_normals;
  csColor4* new_colors;
  size_t new_count;
  bool doColors = colors.GetSize() > 0;
  csCompressVertexInfo* vt = TemplatedCompressVertices (vertices,
        texels, normals, colors, vertices.GetSize (), doColors,
	new_vertices, new_texels, new_normals, new_colors, new_count);
  if (vt == 0) return 0;

  size_t i;
  vertices.Empty ();
  texels.Empty ();
  normals.Empty ();
  colors.Empty ();
  for (i = 0 ; i < new_count ; i++)
  {
    vertices.Push (new_vertices[i]);
    texels.Push (new_texels[i]);
    normals.Push (new_normals[i]);
    if (doColors) colors.Push (new_colors[i]);
  }
  delete[] new_vertices;
  delete[] new_texels;
  delete[] new_normals;
  delete[] new_colors;
  return vt;
}

size_t* csVertexCompressor::Compress (csRef<iRenderBuffer>* buffers, size_t numBuffers,
				      size_t& newCount)
{
  newCount = 0;
  if (!buffers || (numBuffers == 0))
    return nullptr;
  
  // Some input validation
  size_t numVerts = buffers[0]->GetElementCount();
  for (size_t b = 0; b < numBuffers; b++)
  {
    size_t bufVerts = buffers[b]->GetElementCount();
    if (bufVerts != numVerts)
    {
      csPrintfErr ("%s: buffersize mismatch for buffer %zu (expected %zu, got %zu)\n",
		   CS_FUNCTION_NAME, b, numVerts, bufVerts);
      return nullptr;
    }
  }
  
  if (numVerts == 0)
    return nullptr;
  
  // Collect all components ...
  size_t totalComps = 0;
  for (size_t b = 0; b < numBuffers; b++)
  {
    totalComps += buffers[b]->GetComponentCount();
  }
  // ... and convert to format suitable for comparison.
  const int precision = 1000000;
  csDirtyAccessArray<int> allData;
  allData.SetSize (totalComps * numVerts);
  {
    size_t compOffs = 0;
    for (size_t b = 0; b < numBuffers; b++)
    {
      size_t bufComps = buffers[b]->GetComponentCount();
      /* @@@ TODO: Don't fetch components as floats if buffer contains
	integer data anyway */
      csVertexListWalker<float> bufWalker (buffers[b]);
      for (size_t v = 0; v < numVerts; v++)
      {
	for (size_t c = 0; c < bufComps; c++)
	{
	  allData[v*totalComps+compOffs+c] = int (ceil ((*bufWalker)[c] * precision));
	}
	++bufWalker;
      }
      compOffs += bufComps;
    }
  }
  
  size_t* newIndices = new size_t[numVerts];
  csArray<size_t> vertexMapNewToOld;
  vertexMapNewToOld.SetCapacity (numVerts);
  typedef csHash<size_t, uint> VerticesHash;
  VerticesHash newVertices;
  for (size_t v = 0; v < numVerts; v++)
  {
    const int* vertPtr = allData.GetArray() + v*totalComps;
    uint hash = csHashCompute ((char*)vertPtr, totalComps * sizeof (int));
    VerticesHash::Iterator it (newVertices.GetIterator (hash));
    size_t prevVertex = csArrayItemNotFound;
    while (it.HasNext())
    {
      size_t oldIndex = it.Next();
      const int* oldVertPtr = allData.GetArray() + oldIndex*totalComps;
      if (memcmp (vertPtr, oldVertPtr, totalComps * sizeof (int)) == 0)
      {
	prevVertex = oldIndex;
	break;
      }
    }
    if (prevVertex == csArrayItemNotFound)
    {
      prevVertex = vertexMapNewToOld.Push (v);
      newVertices.Put (hash, prevVertex);
    }
    newIndices[v] = prevVertex;
  }
  newCount = vertexMapNewToOld.GetSize();
  
  // No change
  if (newCount == numVerts)
  {
    delete[] newIndices;
    return nullptr;
  }

  for (size_t b = 0; b < numBuffers; b++)
  {
    iRenderBuffer* oldbuf = buffers[b];
    csRef<iRenderBuffer> newbuf (
      csRenderBuffer::CreateRenderBuffer (newCount,
					  oldbuf->GetBufferType(),
					  oldbuf->GetComponentType(),
					  oldbuf->GetComponentCount()));
    csRenderBufferLock<uint8> oldBufLock (oldbuf, CS_BUF_LOCK_READ);
    csRenderBufferLock<uint8> newBufLock (newbuf);
    size_t compDataSize = oldbuf->GetComponentCount()
      * csRenderBufferComponentSizes[oldbuf->GetComponentType() & ~CS_BUFCOMP_NORMALIZED];
    size_t oldBufDist = oldbuf->GetElementDistance();
    size_t newBufDist = newbuf->GetElementDistance();
    for (size_t v = 0; v < newCount; v++)
    {
      memcpy (newBufLock + v * newBufDist,
	      oldBufLock + vertexMapNewToOld[v] * oldBufDist,
	      compDataSize);
    }
    buffers[b] = newbuf;
  }
  
  return newIndices;
}

//---------------------------------------------------------------------------
