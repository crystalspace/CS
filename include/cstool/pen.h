/*
    Copyright (C) 2005 by Christopher Nelson

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

#ifndef __CS_PEN_H__
#define __CS_PEN_H__

#include "ivideo/graph3d.h"
#include "csgeom/poly3d.h"
#include "csgeom/vector4.h"
#include "csutil/garray.h"

  /** A pen is used to draw vector shapes. */
  struct iPen
  {
    /** Sets the current color. */
    virtual void setColor(int32 r, int32 g, int32 b, int32 a)=0;

    /** Draws a single line. */
    virtual void line(int32 x1, int32 y1, int32 x2, int32 y2)=0;

    /** Draws a single point. */
    virtual void point(int32 x1, int32 y2)=0;

    /** Draws a rectangle. */
    virtual void rect(int32 x1, int32 y1, int32 x2, int32 y2, bool fill=false)=0;
    
     /** Draws a mitered rectangle. The miter value should be between 0.0 and 1.0, and determines how
    * much of the corner is mitered off and beveled. */    
    virtual void mitered_rect(int32 x1, int32 y1, int32 x2, int32 y2, float miter, bool fill=false)=0;

    /** Draws a rounded rectangle. The roundness value should be between 0.0 and 1.0, and determines how
     * much of the corner is rounded off. */
    virtual void rounded_rect(int32 x1, int32 y1, int32 x2, int32 y2, float roundness, bool fill=false)=0;    
  };

  /** A pen specialized for CS. */
  class csPen : public iPen
  {
    /** The context for drawing. */
    csRef< iGraphics3D > g3d;

    /** The mesh that we reuse in developing the shapes we're making. */
    csSimpleRenderMesh mesh;
    
    /** The list of vectors we're creating. */
    csVector3Array va;

    /** The list of indices we use. */
    csDirtyAccessArray< uint > ia;

    /** The color we use. */
    csVector4 color;

    /** The color array generated for verts as we render. */
    csDirtyAccessArray< csVector4 > colors;

  protected:
    /** Initializes our working objects. */
    void start();

    /** Adds a vertex. */
    void vert(float x, float y);

    /** Worker, sets up the mesh with the vertices, color, and other information. */
    void setupMesh();

    /** Worker, draws the mesh. */
    void drawMesh(csRenderMeshType mesh_type);

  public:
    csPen(iGraphics3D *_g3d);
    virtual ~csPen();

    /** Sets the current color. */
    virtual void setColor(int32 r, int32 g, int32 b, int32 a);

    /** Draws a single line. */
    virtual void line(int32 x1, int32 y1, int32 x2, int32 y2);

    /** Draws a single point. */
    virtual void point(int32 x1, int32 y2);

    /** Draws a rectangle. */
    virtual void rect(int32 x1, int32 y1, int32 x2, int32 y2, bool fill=false);
    
    /** Draws a mitered rectangle. The miter value should be between 0.0 and 1.0, and determines how
    * much of the corner is mitered off and beveled. */    
    virtual void mitered_rect(int32 x1, int32 y1, int32 x2, int32 y2, float miter, bool fill=false);

    /** Draws a rounded rectangle. The roundness value should be between 0.0 and 1.0, and determines how
     * much of the corner is rounded off. */
    virtual void rounded_rect(int32 x1, int32 y1, int32 x2, int32 y2, float roundness, bool fill=false);        
  };


#endif
