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
  SCF_IMPLEMENTS_INTERFACE(iRenderBufferSource)
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

iRenderBufferSource* csSoftPolygonRenderer::GetBufferSource (uint& indexStart, 
							     uint& indexEnd)
{
  if ((vertex_name  == csInvalidStringID) ||
    //(texel_name     == csInvalidStringID) ||
    //(normal_name    == csInvalidStringID) ||
    //(color_name     == csInvalidStringID) ||
    (index_name     == csInvalidStringID))
  {
    iStringSet* strings = parent->GetStrings ();

    vertex_name   = strings->Request ("vertices");
    //texel_name    = strings->Request ("texture coordinates");
    //normal_name   = strings->Request ("normals");
    //color_name    = strings->Request ("colors");
    index_name    = strings->Request ("indices");
  }

  return ((iRenderBufferSource*)this);
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
}

iRenderBuffer* csSoftPolygonRenderer::GetRenderBuffer (csStringID name)
{
  if (renderBufferNum != polysNum) return 0;

  if (name == vertex_name)
  {
    return vertex_buffer;
  } 
/*  else if (name == texel_name)
  {
    return texel_buffer;
  }*/
/*  else if (name == normal_name)
  {
    return normal_buffer;
  }*/
/*  else if (name == color_name)
  {
    return color_buffer;
  }*/
  else if (name == index_name)
  {
    return index_buffer;
  }
  else
  {
    return 0;
  }
}
