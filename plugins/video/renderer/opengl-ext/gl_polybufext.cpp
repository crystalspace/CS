/* ========== HTTP://WWW.SOURCEFORGE.NET/PROJECTS/CRYSTAL ==========
 *
 * FILE: gl_vbufext.cpp
 *
 * DESCRIPTION:
 *  Implements a Vertex Buffer suitable for forming triangle strips, as well
 *  as individual triangles.
 * 
 * LICENSE:
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 * AUTHOR:
 *    Thomas H. Hendrick
 *
 * CVS/RCS ID:
 *    $Id$
 *
 * === COPYRIGHT (c)2002 ============== PROJECT CRYSTAL SPACE 3D === */
/* -----------------------------------------------------------------
 * Preprocessor Includes
 * ----------------------------------------------------------------- */

#include <math.h>
#include <stdarg.h>

#include "cssysdef.h"
#include "gl_polybufext.h"
#include "csutil/util.h"
#include "imesh/thing/polygon.h"
#include "qint.h"
#include "gl_polybufext.h"
#include "ivideo/material.h"
#include "ivideo/texture.h"
#include "csgeom/transfrm.h"
#include "imesh/thing/lightmap.h"

SCF_IMPLEMENT_IBASE (csPolygonBufferEXT)
  SCF_IMPLEMENTS_INTERFACE (iPolygonBuffer)
SCF_IMPLEMENT_IBASE_END

csPolygonBufferEXT::csPolygonBufferEXT(iGraphics3D *g3d)
{
  SCF_CONSTRUCT_IBASE (NULL);
  m_g3d = g3d;
  m_vertices = 0;
  m_vertex_count = 0;
}
 
csPolygonBufferEXT::~csPolygonBufferEXT()
{
  if(m_vertices) delete[] m_vertices;
}

void csPolygonBufferEXT::AddPolygon (int* verts, int num_verts,
	const csPlane3& poly_normal,
	int mat_index,
	const csMatrix3& m_obj2tex, const csVector3& v_obj2tex,
	iPolygonTexture* poly_texture)
{
  csPolygonBufferEXTMaterial &tmpmat = m_materials[mat_index];

  unsigned char r,g,b;
  tmpmat.m_mat_handle->GetTexture()->GetMeanColor(r,g,b);

  csColor color;
  color.red   = (float) r / 255.0;
  color.green = (float) g / 255.0;
  color.blue  = (float) b / 255.0;

  csVector3 aux;
  csVector3 curvert;
  csVector2 curtex;
  csVector2 curlmcoord;
  csRect    tmprect;
  csTransform transform(m_obj2tex, v_obj2tex);

  // 1.)look for lightmap space, 
  // so that we know where we'll put the indices

  csPolygonBufferEXTIndices *tmplm = NULL;
  int i;
  for(i = 0; i < tmpmat.m_lightmaps.Length(); i++)
  {
    tmplm = &tmpmat.m_lightmaps[i];
    if(tmplm->m_lightmap.m_lmrects.Alloc(poly_texture->GetLightMap()->GetWidth(), poly_texture->GetLightMap()->GetHeight(), tmprect))
      break;
    else
      tmplm = NULL; 
  }

  if(tmplm == NULL) // we didn't find a space in the lightmaps, so we have to create another one..
  {
    tmpmat.m_lightmaps.Push(csPolygonBufferEXTIndices());
    tmplm = &tmpmat.m_lightmaps[tmpmat.m_lightmaps.Length() - 1]; // Get Last one (the one just added)
    tmplm->m_lightmap.m_lmrects.Alloc(poly_texture->GetLightMap()->GetWidth(), poly_texture->GetLightMap()->GetHeight(), tmprect);
  }


  
  for(i = 0; i < num_verts; i++)
  {
    curvert = m_vertices[verts[i]];
    aux = transform.Other2This(m_vertices[verts[0]]);
    curtex.x = aux.x;
    curtex.y = aux.y;
  }
}

void csPolygonBufferEXT::SetVertexArray (csVector3* verts, int num_verts)
{
  if(m_vertices) delete[] m_vertices;

  m_vertices = new csVector3[num_verts];
  memcpy((void*)m_vertices, (void*)verts, num_verts * sizeof(csVector3));
  m_vertex_count = num_verts;  
}

void csPolygonBufferEXT::AddMaterial (iMaterialHandle* mat_handle)
{
  csPolygonBufferEXTMaterial newmat;
  newmat.m_mat_handle = mat_handle;
  m_materials.Push(newmat);
}

int csPolygonBufferEXT::GetMaterialCount () const
{
  return m_materials.Length();
}

iMaterialHandle* csPolygonBufferEXT::GetMaterial (int idx) const
{
  return m_materials[idx].m_mat_handle;
}

int csPolygonBufferEXT::GetVertexCount() const
{
  return m_vertex_count;
}

csVector3* csPolygonBufferEXT::GetVertices() const
{
  return m_vertices;
}

void csPolygonBufferEXT::SetMaterial (int idx, iMaterialHandle* mat_handle)
{
  m_materials[idx].m_mat_handle = mat_handle;
}

void csPolygonBufferEXT::Clear ()
{
}

void csPolygonBufferEXT::MarkLightmapsDirty()
{
}
