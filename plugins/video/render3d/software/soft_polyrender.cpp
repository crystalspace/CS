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

#include "soft_polyrender.h"
#include "sft3dcom.h"

SCF_IMPLEMENT_IBASE(csSoftPolygonRenderer)
  SCF_IMPLEMENTS_INTERFACE(iPolygonRenderer)
SCF_IMPLEMENT_IBASE_END

csStringID csSoftPolygonRenderer::vertex_name   = csInvalidStringID;
//csStringID csSoftPolygonRenderer::texel_name    = csInvalidStringID;
//csStringID csSoftPolygonRenderer::normal_name   = csInvalidStringID;
//csStringID csSoftPolygonRenderer::color_name    = csInvalidStringID;
csStringID csSoftPolygonRenderer::index_name    = csInvalidStringID;

csSoftPolygonRenderer::csSoftPolygonRenderer (
  csSoftwareGraphics3DCommon* parent)
{
  SCF_CONSTRUCT_IBASE(0);
  csSoftPolygonRenderer::parent = parent;
  renderBufferNum = ~0;
  polysNum = 0;
}

csSoftPolygonRenderer::~csSoftPolygonRenderer ()
{
  SCF_DESTRUCT_IBASE();
}

void csSoftPolygonRenderer::PrepareBuffers (uint& indexStart, uint& indexEnd)
{
  if ((vertex_name  == csInvalidStringID) ||
    /*
    (texel_name     == csInvalidStringID) ||
    (normal_name    == csInvalidStringID) ||
    (color_name     == csInvalidStringID) ||
    */
    (index_name     == csInvalidStringID))
  {
    iStringSet* strings = parent->GetStrings ();

    vertex_name   = strings->Request ("vertices");
    /*
    texel_name    = strings->Request ("texture coordinates");
    normal_name   = strings->Request ("normals");
    color_name    = strings->Request ("colors");
    */
    index_name    = strings->Request ("indices");
  }
}

void csSoftPolygonRenderer::PrepareRenderMesh (csRenderMesh& mesh)
{
  PrepareBuffers (mesh.indexstart, mesh.indexend);

  csShaderVariable* sv;
  sv = mesh.variablecontext->GetVariableAdd (index_name);
  sv->SetValue (this /*index_buffer*/);
  sv = mesh.variablecontext->GetVariableAdd (vertex_name);
  sv->SetValue (vertex_buffer);
  /*
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
  */
}

void csSoftPolygonRenderer::Clear ()
{
  polys.DeleteAll ();
  polysNum++;
}

void csSoftPolygonRenderer::AddPolygon (csPolygonRenderData* poly)
{
  polys.Push (poly);
  polysNum++;

  float u1, v1, u2, v2;
  poly->tmapping->GetCoordsOnSuperLM (u1, v1, u2, v2);
  const int m = (int)u1, n = (int)v1;
  rlmIDs.Push (csSoftSuperLightmap::ComputeRlmID (m, n));
}
