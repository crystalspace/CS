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
#include "ivideo/material.h"
#include "ivideo/texture.h"
#include "csgeom/transfrm.h"
#include "imesh/thing/lightmap.h"
#include "gl_polygonbuffermaterialext.h"


SCF_IMPLEMENT_IBASE (csPolygonBufferEXT)
  SCF_IMPLEMENTS_INTERFACE (iPolygonBuffer)
SCF_IMPLEMENT_IBASE_END

csPolygonBufferEXT::csPolygonBufferEXT(iGraphics3D *g3d)
{
  SCF_CONSTRUCT_IBASE (NULL);
  m_g3d = g3d;
  m_tempverts = 0;
  m_tempverts_count = 0;
}
 
csPolygonBufferEXT::~csPolygonBufferEXT()
{
}

void csPolygonBufferEXT::AddPolygon (int* verts, int num_verts,
	const csPlane3& poly_normal,
	int mat_index,
	const csMatrix3& m_obj2tex, const csVector3& v_obj2tex,
	iPolygonTexture* poly_texture)
{
}

void csPolygonBufferEXT::SetVertexArray (csVector3* verts, int num_verts)
{
  if(m_tempverts) delete m_tempverts;
  m_tempverts = new csVector3[num_verts];
  memcpy(m_tempverts, verts, num_verts);
  m_tempverts_count = num_verts;
}

void csPolygonBufferEXT::AddMaterial (iMaterialHandle* mat_handle)
{
  csPolygonBufferMaterialEXT newmat(m_g3d, m_combverts);
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
  return m_tempverts_count;
}

csVector3* csPolygonBufferEXT::GetVertices() const
{
  return m_tempverts;
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
