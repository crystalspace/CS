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

csSoftPolygonRenderer::csSoftPolygonRenderer (
  csSoftwareGraphics3DCommon* parent)
{
  SCF_CONSTRUCT_IBASE(0);
  csRefTrackerAccess::AddAlias (CS_STATIC_CAST(iRenderBuffer*, this), this);
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
}

void csSoftPolygonRenderer::PrepareRenderMesh (csRenderMesh& mesh)
{
  if (!mesh.buffers) mesh.buffers.AttachNew (new csRenderBufferHolder);
  mesh.buffers->SetRenderBuffer (CS_BUFFER_INDEX, this);
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
