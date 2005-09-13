/*
  Copyright (C) 2003 by Jorrit Tyberghein
            (C) 2003 by Frank Richter
            (C) 2005 by Marten Svanfeldt

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
#include "iutil/strset.h"
#include "ivideo/graph3d.h"
#include "ivideo/rndbuf.h"
#include "ivideo/rendermesh.h"
#include "csgfx/renderbuffer.h"
#include "csgfx/shadervarcontext.h"

#include "polyrender.h"
#include "thing.h"

CS_LEAKGUARD_IMPLEMENT (csPolygonRenderer);
CS_LEAKGUARD_IMPLEMENT (csPolygonRenderer::BufferAccessor);

SCF_IMPLEMENT_IBASE(csPolygonRenderer::BufferAccessor)
  SCF_IMPLEMENTS_INTERFACE(iRenderBufferAccessor)
SCF_IMPLEMENT_IBASE_END

csPolygonRenderer::csPolygonRenderer (csThingObjectType* parent) : csRefCount()
{
  csPolygonRenderer::parent = parent;
  renderBufferNum = (uint)~0;
  polysNum = 0;
  buffer_accessor.AttachNew (new BufferAccessor(this));
  shadermanager = parent->shadermgr;
  bufferHolder.AttachNew (new csRenderBufferHolder);
}

csPolygonRenderer::~csPolygonRenderer ()
{
}

void csPolygonRenderer::PrepareBuffers (uint& indexStart, uint& indexEnd)
{
  csRef<iRenderBuffer> masterBuffer;
  if (renderBufferNum != polysNum)
  {
    int num_verts = 0, num_indices = 0, max_vc = 0;
    size_t i;

    for (i = 0; i < polys.Length(); i++)
    {
      csPolygonRenderData* poly = polys[i];

      int pvc = poly->num_vertices;

      num_verts += pvc;
      num_indices += (pvc - 2) * 3;
      max_vc = MAX (max_vc, pvc);
    }

#define INTERLEAVE 1
#if INTERLEAVE
    static const csInterleavedSubBufferOptions interleavedElements[3] =
      {{CS_BUFCOMP_FLOAT, 3}, {CS_BUFCOMP_FLOAT, 2}, {CS_BUFCOMP_FLOAT, 2}};
    csRef<iRenderBuffer> buffers[3];
    masterBuffer = csRenderBuffer::CreateInterleavedRenderBuffers (num_verts, 
      CS_BUF_STATIC, 3, interleavedElements, buffers);
    vertex_buffer = buffers[0];
    texel_buffer = buffers[1];
    lmcoords_buffer = buffers[2];
    float* interleaved_data = (float*)masterBuffer->Lock (CS_BUF_LOCK_NORMAL);
#else
    vertex_buffer = csRenderBuffer::CreateRenderBuffer (num_verts, 
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
    csVector3* vertices = (csVector3*)vertex_buffer->Lock (CS_BUF_LOCK_NORMAL);

    texel_buffer = csRenderBuffer::CreateRenderBuffer (num_verts, 
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 2);
    csVector2* texels = (csVector2*)texel_buffer->Lock (CS_BUF_LOCK_NORMAL);

    lmcoords_buffer = csRenderBuffer::CreateRenderBuffer (num_verts, 
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 2);
    csVector2* lmcoords = (csVector2*)lmcoords_buffer->Lock(CS_BUF_LOCK_NORMAL);
#endif

    csHash<csRef<iRenderBuffer>, csStringID> extraBufferData;

    index_buffer = csRenderBuffer::CreateIndexRenderBuffer (num_indices, 
      CS_BUF_STATIC, CS_BUFCOMP_UNSIGNED_INT, 0, num_verts - 1);

    int* indices = (int*)index_buffer->Lock (CS_BUF_LOCK_NORMAL);

    bufferHolder->SetRenderBuffer (CS_BUFFER_POSITION, vertex_buffer);
    bufferHolder->SetRenderBuffer (CS_BUFFER_TEXCOORD0, texel_buffer);
    bufferHolder->SetRenderBuffer (CS_BUFFER_TEXCOORD_LIGHTMAP, lmcoords_buffer);
    bufferHolder->SetRenderBuffer (CS_BUFFER_INDEX, index_buffer);

    int vindex = 0, iindex = 0;
    indexStart = rbIndexStart = iindex;

    CS_ALLOC_STACK_ARRAY (int, pvIndices, max_vc);

    for (i = 0; i < polys.Length(); i++)
    {
      csPolygonRenderData* static_data = polys[i];

      /*
	To get the texture coordinates of a vertex, the coordinates
	in object space have to be transformed to the texture space.
	The Z part is simply dropped then.
      */
      csMatrix3 t_m;
      csVector3 t_v;
      if (static_data->tmapping)
      {
        t_m = static_data->tmapping->GetO2T ();
        t_v = static_data->tmapping->GetO2TTranslation ();
      }
      else
      {
        CS_ASSERT (false);	// @@@ NEED TO SUPPORT FLAT SHADING!!!
      }
      csTransform object2texture (t_m, t_v);

      csTransform tex2lm;
      if (static_data->useLightmap)
      {
        struct csPolyLMCoords
        {
          float u1, v1, u2, v2;
        };

        csPolyLMCoords lmc;
        /*lm->GetRendererCoords (lmc.u1, lmc.v1,
        lmc.u2, lmc.v2);*/
        static_data->tmapping->GetCoordsOnSuperLM (lmc.u1, lmc.v1,
          lmc.u2, lmc.v2);

        float lm_low_u = 0.0f, lm_low_v = 0.0f;
        float lm_high_u = 1.0f, lm_high_v = 1.0f;
        static_data->tmapping->GetTextureBox (
          lm_low_u, lm_low_v, lm_high_u, lm_high_v);

        float lm_scale_u = ((lmc.u2 - lmc.u1) / (lm_high_u - lm_low_u));
        float lm_scale_v = ((lmc.v2 - lmc.v1) / (lm_high_v - lm_low_v));

        tex2lm.SetO2T (
          csMatrix3 (lm_scale_u, 0, 0,
          0, lm_scale_v, 0,
          0, 0, 1));
        tex2lm.SetO2TTranslation (
          csVector3 (
          (lm_scale_u != 0.0f) ? (lm_low_u - lmc.u1 / lm_scale_u) : 0,
          (lm_scale_v != 0.0f) ? (lm_low_v - lmc.v1 / lm_scale_v) : 0,
          0));
      }

      // First, fill the normal/texel/vertex buffers.
      csVector3* obj_verts = *(static_data->p_obj_verts);
      int j, vc = static_data->num_vertices;
      for (j = 0; j < vc; j++)
      {
        int vidx = static_data->vertices[j];
        const csVector3& vertex = obj_verts[vidx];
#if INTERLEAVE
	*interleaved_data++ = vertex.x;
	*interleaved_data++ = vertex.y;
	*interleaved_data++ = vertex.z;
        csVector3 t = object2texture.Other2This (vertex);
	*interleaved_data++ = t.x;
	*interleaved_data++ = t.y;
        csVector3 l = tex2lm.Other2This (t);
	*interleaved_data++ = l.x;
	*interleaved_data++ = l.y;
#else
        *vertices++ = vertex;
        csVector3 t = object2texture.Other2This (vertex);
        *texels++ = csVector2 (t.x, t.y);
        csVector3 l = tex2lm.Other2This (t);
        *lmcoords++ = csVector2 (l.x, l.y);
#endif

        pvIndices[j] = vindex++;
      }

      if (extraBuffers[i] != 0)
      {
	// Assemble user buffer data
	while (extraBuffers[i]->HasNext())
	{
	  csRef<iRenderBuffer> srcBuf;
	  csStringID name = extraBuffers[i]->Next (&srcBuf);
	  csRef<iRenderBuffer> dstBuf = extraBufferData.Get (name, 0);
	  if (dstBuf == 0)
	  {
	    dstBuf = csRenderBuffer::CreateRenderBuffer (num_verts, CS_BUF_STATIC,
	      srcBuf->GetComponentType(), srcBuf->GetComponentCount());
	    void* bufData = dstBuf->Lock (CS_BUF_LOCK_NORMAL);
	    memset (bufData, 0, dstBuf->GetSize());
	    extraBufferData.Put (name, dstBuf);
	    dstBuf->Release();
	  }
	  void* srcData = srcBuf->Lock (CS_BUF_LOCK_READ);
	  dstBuf->CopyInto (srcData, srcBuf->GetElementCount(), pvIndices[0]);
	  srcBuf->Release();
	}
	extraBuffers[i]->Reset();
      }

      // Triangulate poly.
      for (j = 2; j < vc; j++)
      {
        *indices++ = pvIndices[0];
        iindex++;
        *indices++ = pvIndices[j - 1];
        iindex++;
        *indices++ = pvIndices[j];
        iindex++;
      }
    }

    indexEnd = rbIndexEnd = iindex;

    renderBufferNum = polysNum;

    index_buffer->Release ();
  #if !INTERLEAVE
    vertex_buffer->Release ();
    texel_buffer->Release ();
    lmcoords_buffer->Release ();
  #else
    masterBuffer->Release ();
  #endif

    // Finally, add custom buffers to accessor
    uint accessorMask = 
      CS_BUFFER_NORMAL_MASK | CS_BUFFER_TANGENT_MASK | CS_BUFFER_BINORMAL_MASK;
    csHash<csRef<iRenderBuffer>, csStringID>::GlobalIterator userBufIt =
      extraBufferData.GetIterator();

    while (userBufIt.HasNext())
    {
      csStringID name;
      csRef<iRenderBuffer> buf = userBufIt.Next (name);
      csRenderBufferName bufName = 
	csRenderBuffer::GetBufferNameFromDescr (parent->stringset->Request (name));
      if (bufName >= CS_BUFFER_POSITION)
      {
	accessorMask &= ~(1 << bufName);
	bufferHolder->SetRenderBuffer (bufName, buf);
      }
      else
      {
	if (svcontext == 0) 
	  svcontext.AttachNew (new csShaderVariableContext());
	csShaderVariable* sv = svcontext->GetVariableAdd (name);
	sv->SetValue (buf);
      }
    }

    bufferHolder->SetAccessor (buffer_accessor, accessorMask);
  }
  else
  {
    indexStart = rbIndexStart;
    indexEnd = rbIndexEnd;
  }
}


void csPolygonRenderer::PrepareRenderMesh (csRenderMesh& mesh)
{
  PrepareBuffers (mesh.indexstart, mesh.indexend);
  mesh.geometryInstance = this;
  mesh.buffers = bufferHolder;
  
  if (svcontext != 0)
  {
    if (mesh.variablecontext.IsValid())
    {
      const csRefArray<csShaderVariable>& svcVars = 
	svcontext->GetShaderVariables();
      for (size_t i = 0; i < svcVars.Length(); i++)
      {
	mesh.variablecontext->AddVariable (svcVars[i]);
      }
    }
    else
      mesh.variablecontext = svcontext;
  }
}

void csPolygonRenderer::Clear ()
{
  polys.DeleteAll ();
  polysNum++;
}

void csPolygonRenderer::AddPolygon (csPolygonRenderData* poly,
				      iUserRenderBufferIterator* extraBuffers)
{
  polys.Push (poly);
  this->extraBuffers.Push (extraBuffers);
  polysNum++;
}

void csPolygonRenderer::BufferAccessor::PreGetBuffer (
  csRenderBufferHolder* holder, csRenderBufferName buffer)
{
  if (!holder) return;

  switch (buffer)
  {
  case CS_BUFFER_NORMAL:
    UpdateNormals ();
    holder->SetRenderBuffer (CS_BUFFER_NORMAL, normal_buffer);
    break;
  case CS_BUFFER_TANGENT:
    UpdateTangents ();
    holder->SetRenderBuffer (CS_BUFFER_TANGENT, tangent_buffer);
    break;
  case CS_BUFFER_BINORMAL:
    UpdateBinormals ();
    holder->SetRenderBuffer (CS_BUFFER_BINORMAL, binormal_buffer);
    break;
  default:
    break;
  }
}

bool csPolygonRenderer::BufferAccessor::UpdateNormals ()
{
  if (normalVerticesNum != renderer->polysNum)
  {
    int num_verts = 0;
    size_t i;

    for (i = 0; i < renderer->polys.Length(); i++)
    {
      csPolygonRenderData* poly = renderer->polys[i];
      num_verts += poly->num_vertices;
    }

    if (!normal_buffer 
      || (normal_buffer->GetElementCount() != (size_t)num_verts))
      normal_buffer = csRenderBuffer::CreateRenderBuffer (num_verts, 
	CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);

    csVector3* normals = (csVector3*)normal_buffer->Lock (CS_BUF_LOCK_NORMAL);

    for (size_t i = 0; i < renderer->polys.Length(); i++)
    {
      csPolygonRenderData* static_data = renderer->polys[i];
      bool smoothed = static_data->objNormals && *static_data->objNormals;

      csVector3 polynormal;
      if (!smoothed)
      {
	// hmm... It seems that both polynormal and obj_normals[] need to be
	// inverted. Don't know why, just found it out empirical.
	polynormal = -static_data->plane_obj.Normal();
      }

      // First, fill the normal buffer.
      int j, vc = static_data->num_vertices;
      for (j = 0; j < vc; j++)
      {
	int vidx = static_data->vertices[j];
	if (smoothed)
	  *normals++ = -(*static_data->objNormals)[vidx];
	else
	  *normals++ = polynormal;
      }

    }
    normal_buffer->Release ();

    normalVerticesNum = renderer->polysNum;
  }
  return true;
}

bool csPolygonRenderer::BufferAccessor::UpdateBinormals ()
{
  if (binormalVerticesNum != renderer->polysNum)
  {
    int num_verts = 0;
    size_t i;

    for (i = 0; i < renderer->polys.Length(); i++)
    {
      csPolygonRenderData* poly = renderer->polys[i];
      num_verts += poly->num_vertices;
    }

    if (!binormal_buffer 
      || (binormal_buffer->GetElementCount() != (size_t)num_verts))
      binormal_buffer = csRenderBuffer::CreateRenderBuffer (num_verts, 
	CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);

    csVector3* binormals = (csVector3*)binormal_buffer->Lock (CS_BUF_LOCK_NORMAL);

    for (size_t i = 0; i < renderer->polys.Length(); i++)
    {
      csPolygonRenderData* static_data = renderer->polys[i];

      csMatrix3 t_m;
      csVector3 t_v;
      if (static_data->tmapping)
      {
	t_m = static_data->tmapping->GetO2T ();
	t_v = static_data->tmapping->GetO2TTranslation ();
      }
      else
      {
	CS_ASSERT (false);	// @@@ NEED TO SUPPORT FLAT SHADING!!!
      }

      /*
      Calculate the 'tangent' vector of this poly, needed for dot3.
      It is "a tangent to the surface which represents the direction 
      of increase of the t texture coordinate." (Quotation from
      http://www.ati.com/developer/sdk/rage128sdk/Rage128BumpTutorial.html)
      Conveniently, all polys have a object->texture space transformation
      associated with them.

      @@@ Ignores the fact things can be smooth.
      But it's simpler for now :)
      */
      csTransform tangentTF (t_m.GetInverse (), csVector3 (0));
      csVector3 origin = tangentTF.Other2This (csVector3 (0, 0, 0));
      //csVector3 tangent = 
      //tangentTF.Other2This (csVector3 (1, 0, 0)) - origin;
      //tangent.Normalize ();

      /*
      Calculate the 'binormal' vector of this poly, needed for dot3.
      */
      csVector3 binormal = 
	tangentTF.Other2This (csVector3 (0, 1, 0)) - origin;
      binormal.Normalize ();

      int j, vc = static_data->num_vertices;
      for (j = 0; j < vc; j++)
      {
	//*tangents++ = tangent;
	*binormals++ = binormal;
      }

    }

    binormal_buffer->Release ();
    binormalVerticesNum = renderer->polysNum;
  }
  return true;
}

bool csPolygonRenderer::BufferAccessor::UpdateTangents ()
{
  if (tangentVerticesNum != renderer->polysNum)
  {
    int num_verts = 0;
    size_t i;

    for (i = 0; i < renderer->polys.Length(); i++)
    {
      csPolygonRenderData* poly = renderer->polys[i];
      num_verts += poly->num_vertices;
    }

    if (!tangent_buffer 
      || (tangent_buffer->GetElementCount() != (size_t)num_verts))
      tangent_buffer = csRenderBuffer::CreateRenderBuffer (num_verts, 
	CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);

    csVector3* tangents = (csVector3*)tangent_buffer->Lock (CS_BUF_LOCK_NORMAL);

    for (size_t i = 0; i < renderer->polys.Length(); i++)
    {
      csPolygonRenderData* static_data = renderer->polys[i];

      csMatrix3 t_m;
      csVector3 t_v;
      if (static_data->tmapping)
      {
	t_m = static_data->tmapping->GetO2T ();
	t_v = static_data->tmapping->GetO2TTranslation ();
      }
      else
      {
	CS_ASSERT (false);	// @@@ NEED TO SUPPORT FLAT SHADING!!!
      }

      /*
      Calculate the 'tangent' vector of this poly, needed for dot3.
      It is "a tangent to the surface which represents the direction 
      of increase of the t texture coordinate." (Quotation from
      http://www.ati.com/developer/sdk/rage128sdk/Rage128BumpTutorial.html)
      Conveniently, all polys have a object->texture space transformation
      associated with them.

      @@@ Ignores the fact things can be smooth.
      But it's simpler for now :)
      */
      csTransform tangentTF (t_m.GetInverse (), csVector3 (0));
      csVector3 origin = tangentTF.Other2This (csVector3 (0, 0, 0));
      csVector3 tangent = 
	tangentTF.Other2This (csVector3 (1, 0, 0)) - origin;
      tangent.Normalize ();

      int j, vc = static_data->num_vertices;
      for (j = 0; j < vc; j++)
      {
	*tangents++ = tangent;
      }

    }

    tangent_buffer->Release ();
    tangentVerticesNum = renderer->polysNum;
  }
  return true;
}
