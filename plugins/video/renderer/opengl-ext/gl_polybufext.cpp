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
#include "csutil/util.h"
#include "imesh/thing/polygon.h"
#include "qint.h"

csPolygonBufferEXT::csPolygonBufferEXT(iGraphics3D *g3d)
{
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
}

void csPolygonBufferEXT::AddMaterial (iMaterialHandle* mat_handle)
{
}

int csPolygonBufferEXT::GetMaterialCount ()
{
}

iMaterialHandle* csPolygonBufferEXT::GetMaterial (int idx)
{
}

int csPolygonBufferEXT::GetVertexCount()
{
}

csVector3* csPolygonBufferEXT::GetVertices()
{
}

void csPolygonBufferEXT::SetMaterial (int idx, iMaterialHandle* mat_handle)
{
}

void csPolygonBufferEXT::Clear ()
{
}

void csPolygonBufferEXT::MarkLightmapsDirty()
{
}