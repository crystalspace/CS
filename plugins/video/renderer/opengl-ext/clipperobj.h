#ifndef CLIPPEROBJ_H_INCLUDED
#define CLIPPEROBJ_H_INCLUDED
/* ========== HTTP://WWW.SOURCEFORGE.NET/PROJECTS/CRYSTAL ==========
 *
 * FILE: gl_clipperobj.h
 *
 * DESCRIPTION:
 *  an Interface definition for diffrent OpenGL clippers
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

struct iGraphics3d;
struct iClipper2D;
struct G3DPolygonDP;
struct G3DPolygonDPFX;
struct G3DTriangleMesh;
struct G3DPolygonMesh;

class iClipperObject
{
public:

  virtual void SetClipper(iClipper2D *clipper) = 0;
  virtual iClipper2D * GetCurClipper() = 0;

  virtual G3DPolygonDP  & ClipPolygon     ( G3DPolygonDP& poly ) = 0;
  virtual G3DPolygonDPFX& ClipPolygonFX   ( G3DPolygonDPFX& poly ) = 0;

  virtual G3DTriangleMesh& ClipTriangleMesh( G3DTriangleMesh& mesh ) = 0;
  virtual G3DPolygonMesh & ClipPolygonMesh ( G3DPolygonMesh&  mesh ) = 0;

};

#endif