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

#include "imesh/thing/polygon.h"

#include "gl_polyrender.h"
#include "gl_render3d.h"

SCF_IMPLEMENT_IBASE(csGLPolygonRenderer)
  SCF_IMPLEMENTS_INTERFACE(iPolygonRenderer)
  SCF_IMPLEMENTS_INTERFACE(iRenderBufferSource)
SCF_IMPLEMENT_IBASE_END

csStringID csGLPolygonRenderer::vertex_name   = csInvalidStringID;
csStringID csGLPolygonRenderer::texel_name    = csInvalidStringID;
csStringID csGLPolygonRenderer::normal_name   = csInvalidStringID;
csStringID csGLPolygonRenderer::color_name    = csInvalidStringID;
csStringID csGLPolygonRenderer::index_name    = csInvalidStringID;
csStringID csGLPolygonRenderer::tangent_name  = csInvalidStringID;
csStringID csGLPolygonRenderer::binormal_name = csInvalidStringID;

csGLPolygonRenderer::csGLPolygonRenderer (csGLGraphics3D* parent)
{
  SCF_CONSTRUCT_IBASE(0);

  csGLPolygonRenderer::parent = parent;
  renderBufferNum = ~0;
  polysNum = 0;
}

csGLPolygonRenderer::~csGLPolygonRenderer ()
{
}

void csGLPolygonRenderer::PrepareBuffers (uint& indexStart, uint& indexEnd)
{
  if ((vertex_name  == csInvalidStringID) ||
    (texel_name     == csInvalidStringID) ||
    (normal_name    == csInvalidStringID) ||
    (color_name     == csInvalidStringID) ||
    (index_name     == csInvalidStringID) ||
    (tangent_name   == csInvalidStringID) ||
    (binormal_name  == csInvalidStringID))
  {
    iStringSet* strings = parent->GetStrings ();

    vertex_name   = strings->Request ("vertices");
    texel_name    = strings->Request ("texture coordinates");
    normal_name   = strings->Request ("normals");
    color_name    = strings->Request ("colors");
    index_name    = strings->Request ("indices");
    tangent_name  = strings->Request ("tangents");
    binormal_name = strings->Request ("binormals");
  }

  if (renderBufferNum != polysNum)
  {
    int num_verts = 0, num_indices = 0, max_vc = 0;
    int i;

    for (i = 0; i < polys.Length(); i++)
    {
      csPolygonRenderData* poly = polys[i];

      int pvc = poly->vertices.GetVertexCount ();

      num_verts += pvc;
      num_indices += (pvc - 2) * 3;
      max_vc = MAX (max_vc, pvc);
    }

    vertex_buffer = parent->CreateRenderBuffer (num_verts * sizeof (csVector3), 
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3, false);
    csVector3* vertices = (csVector3*)vertex_buffer->Lock (CS_BUF_LOCK_NORMAL);

    normal_buffer = parent->CreateRenderBuffer (num_verts * sizeof (csVector3), 
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3, false);
    csVector3* normals = (csVector3*)normal_buffer->Lock (CS_BUF_LOCK_NORMAL);

    texel_buffer = parent->CreateRenderBuffer (num_verts * sizeof (csVector2), 
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 2, false);
    csVector2* texels = (csVector2*)texel_buffer->Lock (CS_BUF_LOCK_NORMAL);

    index_buffer = parent->CreateRenderBuffer (num_indices  * sizeof (int), 
      CS_BUF_STATIC, CS_BUFCOMP_UNSIGNED_INT, 1, true);
    int* indices = (int*)index_buffer->Lock (CS_BUF_LOCK_NORMAL);

    tangent_buffer = parent->CreateRenderBuffer (num_verts * sizeof (csVector3), 
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3, false);
    csVector3* tangents = (csVector3*)tangent_buffer->Lock (CS_BUF_LOCK_NORMAL);

    binormal_buffer = parent->CreateRenderBuffer (num_verts * sizeof (csVector3), 
      CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3, false);
    csVector3* binormals = (csVector3*)binormal_buffer->Lock (CS_BUF_LOCK_NORMAL);

    int vindex = 0, iindex = 0;
    indexStart = rbIndexStart = iindex;

    CS_ALLOC_STACK_ARRAY (int, pvIndices, max_vc);

    for (i = 0; i < polys.Length(); i++)
    {
      csPolygonRenderData* static_data = polys[i];

      //int* poly_indices = static_data->GetVertexIndices ();

      csVector3 polynormal;
      //if (!smoothed)
      //{
	// hmm... It seems that both polynormal and obj_normals[] need to be inverted.
	//  Don't know why, just found it out empirical.
	polynormal = -static_data->plane_obj.Normal();
      //}

      /*
	To get the texture coordinates of a vertex, the coordinates
	in object space have to be transformed to the texture space.
	The Z part is simply dropped then.
      */
      csMatrix3 t_m;
      csVector3 t_v;
      if (static_data->tmapping)
      {
        t_m = static_data->tmapping->m_obj2tex;
        t_v = static_data->tmapping->v_obj2tex;
      }
      else
      {
        CS_ASSERT (false);	// @@@ NEED TO SUPPORT FLAT SHADING!!!
      }
      csTransform object2texture (t_m, t_v);

      /*
        Calculate the 'tangent' vector of this poly, needed for dot3.
        It is "a tangent to the surface which represents the direction 
        of increase of the t texture coordinate." (Quotation from
         http://www.ati.com/developer/sdk/rage128sdk/Rage128BumpTutorial.html)
         Conveniently, all polys have a object->texture space transformatin
         associated with them.

         @@@ Ignores the fact things can be smooth.
         But it's simpler for now :)
       */
      csTransform tangentTF (t_m.GetInverse (), csVector3 (0));
      csVector3 tangent = tangentTF.Other2This (csVector3 (1, 0, 0));
      tangent.Normalize ();

      /*
        Calculate the 'binormal' vector of this poly, needed for dot3.
      */
      csVector3 binormal = tangentTF.Other2This (csVector3 (0, -1, 0));
      binormal.Normalize ();

      // First, fill the normal/texel/vertex buffers.
      csVector3* obj_verts = *(static_data->p_obj_verts);
      int j, vc = static_data->vertices.GetVertexCount();
      for (j = 0; j < vc; j++)
      {
	//int vidx = *poly_indices++;
        const csVector3& vertex = obj_verts[
		static_data->vertices.GetVertex (j)];
        *vertices++ = vertex;
/*	*vertices++ = obj_verts[vidx];
	@@@ FIXME
        if (smoothed)
	{
	  CS_ASSERT (obj_normals != 0);
	  *normals++ = -obj_normals[vidx];
	}
	else*/
	  *normals++ = polynormal;
	csVector3 t = object2texture.Other2This (vertex);
	*texels++ = csVector2 (t.x, t.y);
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
}

iRenderBufferSource* csGLPolygonRenderer::GetBufferSource (uint& indexStart, 
                                                           uint& indexEnd)
{
  PrepareBuffers (indexStart, indexEnd);
  return ((iRenderBufferSource*)this);
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

iRenderBuffer* csGLPolygonRenderer::GetRenderBuffer (csStringID name)
{
  if (renderBufferNum != polysNum) return 0;

  if (name == vertex_name)
  {
    return vertex_buffer;
  } 
  else if (name == texel_name)
  {
    return texel_buffer;
  }
  else if (name == normal_name)
  {
    return normal_buffer;
  }
  else if (name == color_name)
  {
    return color_buffer;
  }
  else if (name == index_name)
  {
    return index_buffer;
  }
  else if (name == tangent_name)
  {
    return tangent_buffer;
  }
  else if (name == binormal_name)
  {
    return binormal_buffer;
  }
  else
  {
    return 0;
  }
}
