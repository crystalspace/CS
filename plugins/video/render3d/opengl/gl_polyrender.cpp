/*
  Copyright (C) 2003 by Jorrit Tyberghein
  (C) 2003 by Frank Richter

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

#include "gl_polyrender.h"
#include "gl_render3d.h"

CS_LEAKGUARD_IMPLEMENT (csGLPolygonRenderer)
CS_LEAKGUARD_IMPLEMENT (csGLPolygonRenderer::FogAccesor)

SCF_IMPLEMENT_IBASE(csGLPolygonRenderer)
  SCF_IMPLEMENTS_INTERFACE(iPolygonRenderer)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE(csGLPolygonRenderer::FogAccesor)
  SCF_IMPLEMENTS_INTERFACE(iShaderVariableAccessor)
SCF_IMPLEMENT_IBASE_END

csStringID csGLPolygonRenderer::vertex_name   = csInvalidStringID;
csStringID csGLPolygonRenderer::texel_name    = csInvalidStringID;
csStringID csGLPolygonRenderer::normal_name   = csInvalidStringID;
csStringID csGLPolygonRenderer::color_name    = csInvalidStringID;
csStringID csGLPolygonRenderer::index_name    = csInvalidStringID;
csStringID csGLPolygonRenderer::tangent_name  = csInvalidStringID;
csStringID csGLPolygonRenderer::binormal_name = csInvalidStringID;
csStringID csGLPolygonRenderer::lmcoords_name = csInvalidStringID;
csStringID csGLPolygonRenderer::fog_name      = csInvalidStringID;

csStringID csGLPolygonRenderer::o2c_matrix_name = csInvalidStringID;
csStringID csGLPolygonRenderer::o2c_vector_name = csInvalidStringID;
csStringID csGLPolygonRenderer::fogplane_name   = csInvalidStringID;
csStringID csGLPolygonRenderer::fogdensity_name = csInvalidStringID;

csGLPolygonRenderer::csGLPolygonRenderer (csGLGraphics3D* parent)
{
  SCF_CONSTRUCT_IBASE(0);
  csGLPolygonRenderer::parent = parent;
  renderBufferNum = ~0;
  polysNum = 0;
  fog_accessor.AttachNew (new FogAccesor(this));
  shadermanager = parent->shadermgr;
}

csGLPolygonRenderer::~csGLPolygonRenderer ()
{
  SCF_DESTRUCT_IBASE()
}

void csGLPolygonRenderer::PrepareBuffers (uint& indexStart, uint& indexEnd)
{
  if ((vertex_name  == csInvalidStringID) ||
    (texel_name     == csInvalidStringID) ||
    (normal_name    == csInvalidStringID) ||
    (color_name     == csInvalidStringID) ||
    (index_name     == csInvalidStringID) ||
    (tangent_name   == csInvalidStringID) ||
    (binormal_name  == csInvalidStringID) ||
    (lmcoords_name  == csInvalidStringID))
  {
    iStringSet* strings = parent->GetStrings ();

    vertex_name   = strings->Request ("vertices");
    texel_name    = strings->Request ("texture coordinates");
    normal_name   = strings->Request ("normals");
    color_name    = strings->Request ("colors");
    index_name    = strings->Request ("indices");
    tangent_name  = strings->Request ("tangents");
    binormal_name = strings->Request ("binormals");
    lmcoords_name = strings->Request ("lightmap coordinates");
    fog_name      = strings->Request ("fog value");
    o2c_matrix_name = strings->Request ("object2camera matrix");
    o2c_vector_name = strings->Request ("object2camera vector");
    fogplane_name = strings->Request ("fogplane");
    fogdensity_name = strings->Request ("fog density");
  }

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

    vertex_buffer = parent->CreateRenderBuffer (num_verts * sizeof (csVector3), 
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
    csVector3* vertices = (csVector3*)vertex_buffer->Lock (CS_BUF_LOCK_NORMAL);

    normal_buffer = parent->CreateRenderBuffer (num_verts * sizeof (csVector3), 
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
    csVector3* normals = (csVector3*)normal_buffer->Lock (CS_BUF_LOCK_NORMAL);

    texel_buffer = parent->CreateRenderBuffer (num_verts * sizeof (csVector2), 
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 2);
    csVector2* texels = (csVector2*)texel_buffer->Lock (CS_BUF_LOCK_NORMAL);

    index_buffer = parent->CreateIndexRenderBuffer (num_indices  * sizeof (int), 
      CS_BUF_STATIC, CS_BUFCOMP_UNSIGNED_INT, 0, num_verts - 1);
    int* indices = (int*)index_buffer->Lock (CS_BUF_LOCK_NORMAL);

    tangent_buffer = parent->CreateRenderBuffer (num_verts * sizeof (csVector3), 
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
    csVector3* tangents = (csVector3*)tangent_buffer->Lock (CS_BUF_LOCK_NORMAL);

    binormal_buffer = parent->CreateRenderBuffer (num_verts * sizeof (csVector3), 
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
    csVector3* binormals = (csVector3*)binormal_buffer->Lock (CS_BUF_LOCK_NORMAL);

    lmcoords_buffer = parent->CreateRenderBuffer (num_verts * sizeof (csVector2), 
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 2);
    csVector2* lmcoords = (csVector2*)lmcoords_buffer->Lock (CS_BUF_LOCK_NORMAL);

    int vindex = 0, iindex = 0;
    indexStart = rbIndexStart = iindex;

    CS_ALLOC_STACK_ARRAY (int, pvIndices, max_vc);

    for (i = 0; i < polys.Length(); i++)
    {
      csPolygonRenderData* static_data = polys[i];
      bool smoothed = static_data->objNormals && *static_data->objNormals;

      //int* poly_indices = static_data->GetVertexIndices ();

      csVector3 polynormal;
      if (!smoothed)
      {
	// hmm... It seems that both polynormal and obj_normals[] need to be inverted.
	//  Don't know why, just found it out empirical.
	polynormal = -static_data->plane_obj.Normal();
      }

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

      /*
	Calculate the 'binormal' vector of this poly, needed for dot3.
      */
      csVector3 binormal = 
	tangentTF.Other2This (csVector3 (0, 1, 0)) - origin;
      binormal.Normalize ();

      // First, fill the normal/texel/vertex buffers.
      csVector3* obj_verts = *(static_data->p_obj_verts);
      int j, vc = static_data->num_vertices;
      for (j = 0; j < vc; j++)
      {
        int vidx = static_data->vertices[j];
        const csVector3& vertex = obj_verts[vidx];
        *vertices++ = vertex;
        if (smoothed)
        {
	  *normals++ = -(*static_data->objNormals)[vidx];
        }
        else
	  *normals++ = polynormal;

        csVector3 t = object2texture.Other2This (vertex);
        *texels++ = csVector2 (t.x, t.y);
        csVector3 l = tex2lm.Other2This (t);
        *lmcoords++ = csVector2 (l.x, l.y);
        *tangents++ = tangent;
        *binormals++ = binormal;

        pvIndices[j] = vindex++;
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
  }
  else
  {
    indexStart = rbIndexStart;
    indexEnd = rbIndexEnd;
  }

  index_buffer->Release ();
  texel_buffer->Release ();
  normal_buffer->Release ();
  vertex_buffer->Release ();
  tangent_buffer->Release ();
  binormal_buffer->Release ();
  lmcoords_buffer->Release ();
}


void csGLPolygonRenderer::PrepareRenderMesh (csRenderMesh& mesh)
{
  PrepareBuffers (mesh.indexstart, mesh.indexend);

  csShaderVariable* sv;
  sv = mesh.variablecontext->GetVariableAdd (index_name);
  sv->SetValue (index_buffer);
  sv = mesh.variablecontext->GetVariableAdd (vertex_name);
  sv->SetValue (vertex_buffer);
  sv = mesh.variablecontext->GetVariableAdd (texel_name);
  sv->SetValue (texel_buffer);
  sv = mesh.variablecontext->GetVariableAdd (normal_name);
  sv->SetValue (normal_buffer);
  sv = mesh.variablecontext->GetVariableAdd (binormal_name);
  sv->SetValue (binormal_buffer);
  sv = mesh.variablecontext->GetVariableAdd (tangent_name);
  sv->SetValue (tangent_buffer);
  sv = mesh.variablecontext->GetVariableAdd (lmcoords_name);
  sv->SetValue (lmcoords_buffer);
  sv = mesh.variablecontext->GetVariableAdd (color_name);
  sv->SetValue (color_buffer);
  sv = mesh.variablecontext->GetVariableAdd (fog_name);
  sv->SetAccessor (fog_accessor);

  mesh.geometryInstance = this;
}

void csGLPolygonRenderer::Clear ()
{
  polys.DeleteAll ();
  polysNum++;
}

void csGLPolygonRenderer::AddPolygon (csPolygonRenderData* poly)
{
  polys.Push (poly);
  polysNum++;
}

void csGLPolygonRenderer::FogAccesor::PreGetValue (csShaderVariable *variable)
{
  //get the variables we need
  csShaderVariable *sv;
  csShaderVarStack& svStack = renderer->shadermanager->GetShaderVariableStack();

  csMatrix3 transMatrix;
  csVector3 transVector;
  csVector4 planeAsVector;
  float density;

  sv = svStack[renderer->o2c_matrix_name].Top();
  if (!sv->GetValue (transMatrix)) return;
  sv = svStack[renderer->o2c_vector_name].Top();
  if (!sv->GetValue (transVector)) return;
  sv = svStack[renderer->fogplane_name].Top();
  if (!sv->GetValue (planeAsVector)) return;
  sv = svStack[renderer->fogdensity_name].Top();
  if (!sv->GetValue (density)) return;


  csPlane3 fogPlane;
  fogPlane.norm = csVector3 (planeAsVector.x, planeAsVector.y, planeAsVector.z);
  fogPlane.DD = planeAsVector.w;

  if (fogVerticesNum != renderer->polysNum)
  {
    int num_verts = 0;
    size_t i;

    for (i = 0; i < renderer->polys.Length(); i++)
    {
      csPolygonRenderData* poly = renderer->polys[i];
      num_verts += poly->num_vertices;
    }

    fog_buffer = renderer->parent->CreateRenderBuffer (
    	num_verts * sizeof (csVector3), 
        CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
    void *buf = fog_buffer->Lock(CS_BUF_LOCK_NORMAL);
    memset (buf,0,num_verts*sizeof(csVector3));
    fog_buffer->Release();
    fogVerticesNum = renderer->polysNum;
  }

  csVector3* fog = (csVector3*)fog_buffer->Lock (CS_BUF_LOCK_NORMAL);

  
  csReversibleTransform rt(transMatrix, transVector);
  csVector3 planeNormal = fogPlane.Normal ();
  csVector3 refpos = rt.GetO2TTranslation();
  
  for (size_t i = 0; i < renderer->polys.Length(); i++)
  {
    csPolygonRenderData* static_data = renderer->polys[i];

    // First, fill the normal/texel/vertex buffers.
    csVector3* obj_verts = *(static_data->p_obj_verts);
    int j, vc = static_data->num_vertices;
    for (j = 0; j < vc; j++)
    {
      //int vidx = *poly_indices++;
      const csVector3& vertex = obj_verts[static_data->vertices[j]];
      csVector3 diff = vertex-refpos;
      float f=density*(fogPlane.Classify (diff))/(diff.Unit ()*planeNormal); 
      f = MAX(MIN(f,1.0f), 0.0f); //clamp at 0
      *fog = csVector3(f,f,f);
      fog++;
    }
  }
  
  
  fog_buffer->Release ();

  variable->SetValue(fog_buffer);
}
