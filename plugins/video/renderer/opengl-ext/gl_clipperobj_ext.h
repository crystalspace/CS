#ifndef GL_CLIPPEROBJ_H_INCLUDED
#define GL_CLIPPEROBJ_H_INCLUDED
/* ========== HTTP://WWW.SOURCEFORGE.NET/PROJECTS/CRYSTAL ==========
 *
 * FILE: gl_clipperobj_ext.h
 *
 * DESCRIPTION:
 *  Implements a Stencil Clipper
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

#include "clipperobj.h"
#include "gl_states_ext.h"

struct iGraphics3D;
struct iClipper2D;

class ClipperObjectStencil : public iClipperObject
{
public:
  ClipperObjectStencil(iGraphics3D *g3d, unsigned char layercount, gl_states_ext &states);
  ~ClipperObjectStencil();

  virtual void SetClipper(iClipper2D *clipper);
  virtual iClipper2D * GetCurClipper();

  virtual G3DPolygonDP  & ClipPolygon     ( G3DPolygonDP& poly );
  virtual G3DPolygonDPFX& ClipPolygonFX   ( G3DPolygonDPFX& poly );

  virtual G3DTriangleMesh& ClipTriangleMesh( G3DTriangleMesh& mesh );
  virtual G3DPolygonMesh & ClipPolygonMesh ( G3DPolygonMesh&  mesh );

private:

  void SetupClipper();
  unsigned char m_stencilmask;
  unsigned char m_layercount;
  unsigned char m_curbit;
  gl_states_ext &m_glstates;

  iClipper2D *  m_curclip;  
  iGraphics3D * m_g3d;
};


#endif