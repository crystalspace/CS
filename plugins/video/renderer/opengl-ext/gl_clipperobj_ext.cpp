/* ========== HTTP://WWW.SOURCEFORGE.NET/PROJECTS/CRYSTAL ==========
 *
 * FILE: gl_clipperobj_ext.cpp
 *
 * DESCRIPTION:
 *  Implements a Vertex Buffer suitable for forming primitive types
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
 * AUTHOR:
 *    Philipp R. Aumayr
 *
 * CVS/RCS ID:
 *    $Id:
 *
 * === COPYRIGHT (c)2002 ============== PROJECT CRYSTAL SPACE 3D === */
/* -----------------------------------------------------------------
 * Preprocessor Includes
 * ----------------------------------------------------------------- */

#include "cssysdef.h"
#include "gl_clipperobj_ext.h"
#include "ivideo/graph3d.h"
#include "igeom/clip2d.h"



#if defined(CS_OPENGL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENGL_PATH,gl.h)
#else
#include <GL/gl.h>
#endif

ClipperObjectStencil::ClipperObjectStencil(iGraphics3D *g3d, 
                                           unsigned char layercount, 
                                           gl_states_ext &states)
                                           : m_glstates(states)
{
  m_curbit = 0;
  m_curclip = 0;
  m_g3d = g3d;
  m_layercount = layercount;
  m_stencilmask = layercount - 1; // example: layercount = 1 - 16, mask 0 - 15 ( 4 bits)
}

ClipperObjectStencil::~ClipperObjectStencil()
{
}

void ClipperObjectStencil::SetClipper(iClipper2D *clipper)
{
  if (clipper) clipper->IncRef ();
  if (m_curclip) m_curclip->DecRef ();
  m_curclip = clipper;
}

iClipper2D * ClipperObjectStencil::GetCurClipper()
{
  return m_curclip;
}

G3DPolygonDP  & ClipperObjectStencil::ClipPolygon (G3DPolygonDP& poly)
{
  return poly;
}

G3DPolygonDPFX& ClipperObjectStencil::ClipPolygonFX (G3DPolygonDPFX& poly)
{
  return poly;
}

G3DTriangleMesh& ClipperObjectStencil::ClipTriangleMesh (G3DTriangleMesh& mesh)
{
  return mesh;
}

G3DPolygonMesh & ClipperObjectStencil::ClipPolygonMesh (G3DPolygonMesh&  mesh)
{
  return mesh;
}

void ClipperObjectStencil::SetupClipper()
{
  if(m_curclip == 0) // there's no new clipper, 
                     // we'll have to disable the test functions and return;
  {
    glDisable(GL_STENCIL_TEST);
    return;
  }
  
  m_curbit++;

  if(m_curbit > m_stencilmask)
  {
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);
    m_curbit = 0;
  }

  glStencilMask(m_stencilmask);	
  glStencilFunc(GL_EQUAL, m_curbit, m_stencilmask);
  
  if(m_glstates.glenabled_stencil_test)
  {
    glDisable(GL_STENCIL_TEST);
    // we don't change the state now, well will set it back to enabled 
    // after the poly was drawn
  }
    
  if(m_glstates.glenabled_blend)
  {
    glDisable(GL_BLEND);
    m_glstates.glenabled_blend = false;
  }

  csVector2 * poly = m_curclip->GetClipPoly();

  if(m_glstates.glcsenabled_vertex_array = false)
    glEnableClientState(GL_VERTEX_ARRAY);
  
  glVertexPointer(2,GL_FLOAT, sizeof(csVector2), &poly->x);
  glDrawArrays(GL_LINE_LOOP, 0, m_curclip->GetVertexCount());

  glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // We want the stencil buffer 

  glEnable(GL_STENCIL_TEST); // Since the clipper is active, we have to test
  m_glstates.glenabled_stencil_test = true;
}
