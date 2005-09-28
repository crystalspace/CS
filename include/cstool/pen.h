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

#ifndef __CS_CSTOOL_PEN_H__
#define __CS_CSTOOL_PEN_H__

/**\file
 * Vector shape drawing.
 */

#include "csgeom/poly3d.h"
#include "csgeom/polyidx.h"
#include "csgeom/vector4.h"
#include "csutil/cscolor.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/ref.h"

#include "ivideo/graph3d.h"

struct iFont;
struct iGraphics2D;
struct iGraphics3D;

enum CS_PEN_TEXT_ALIGN 
{ 
  CS_PEN_TA_TOP, 
  CS_PEN_TA_BOT, 
  CS_PEN_TA_LEFT, 
  CS_PEN_TA_RIGHT, 
  CS_PEN_TA_CENTER 
};


/** 
 * A pen is used to draw vector shapes. 
 */
struct iPen
{
  /** 
   * Sets the current color. 
   */
  virtual void SetColor (float r, float g, float b, float a) = 0;

  /** 
   * Sets the current color. 
   */
  virtual void SetColor(const csColor4 &color) = 0;

  /**
   * Swaps the current color and the alternate color. 
   */
  virtual void SwapColors() = 0;

  /**    
   * Clears the current transform, resets to identity.
   */
  virtual void ClearTransform() = 0;

  /** 
   * Pushes the current transform onto the stack. *
   */
  virtual void PushTransform() = 0;

  /**
   * Pops the transform stack. The top of the stack becomes the current
   * transform. 
   */
  virtual void PopTransform() = 0;

  /** 
   * Sets the origin of the coordinate system. 
   */
  virtual void SetOrigin(const csVector3 &o) = 0;

  /** 
   * Translates by the given vector
   */
  virtual void Translate(const csVector3 &t) = 0;

  /**
   * Rotates by the given angle.
   */
  virtual void Rotate(const float &a) = 0;

  /** 
   * Draws a single line. 
   */
  virtual void DrawLine (uint x1, uint y1, uint x2, uint y2) = 0;

  /** 
   * Draws a single point. 
   */
  virtual void DrawPoint (uint x1, uint y2) = 0;

  /** 
   * Draws a rectangle. 
   */
  virtual void DrawRect (uint x1, uint y1, uint x2, uint y2,
  	bool swap_colors = false, bool fill = false) = 0;
  
  /** 
   * Draws a mitered rectangle. The miter value should be between 0.0 and 1.0, 
   * and determines how much of the corner is mitered off and beveled. 
   */
  virtual void DrawMiteredRect (uint x1, uint y1, uint x2, uint y2, 
    float miter, bool swap_colors = false, bool fill = false) = 0;

  /** 
   * Draws a rounded rectangle. The roundness value should be between
   * 0.0 and 1.0, and determines how much of the corner is rounded off. 
   */
  virtual void DrawRoundedRect (uint x1, uint y1, uint x2, uint y2, 
    float roundness, bool swap_colors = false, bool fill = false) = 0; 

  /** 
   * Draws an elliptical arc from start angle to end angle.  Angle must be
   * specified in radians. The arc will be made to fit in the given box.
   * If you want a circular arc, make sure the box is a square.  If you want
   * a full circle or ellipse, specify 0 as the start angle and 2*PI as the end
   * angle.
   */
  virtual void DrawArc(uint x1, uint y1, uint x2, uint y2, float start_angle,
  	float end_angle, bool swap_colors = false, bool fill=false) = 0;

  /**
   * Draws a triangle around the given vertices. 
   */
  virtual void DrawTriangle(uint x1, uint y1, uint x2, uint y2, uint x3, uint y3, bool fill=false) = 0;

  /**
   * Writes text in the given font at the given location.
   */
  virtual void Write(iFont *font, uint x1, uint y1, char *text) = 0;

  /**
   * Writes text in the given font, in the given box.  The alignment
   * specified in h_align and v_align determine how it should be aligned.  
   */
  virtual void WriteBoxed(iFont *font, uint x1, uint y1, uint x2, uint y2, 
    uint h_align, uint v_align, char *text) = 0;
};

/** A pen specialized for CS. */
class csPen : public iPen
{
  /** The 3d context for drawing. */
  csRef<iGraphics3D> g3d;

  /** The 2d context for drawing. */
  csRef<iGraphics2D> g2d;

  /** The mesh that we reuse in developing the shapes we're making. */
  csSimpleRenderMesh mesh;
  
  /** The polygon index that we're generating. */
  csPolyIndexed poly_idx;
  
  /** The polygon that we're generating. */
  csPoly3D poly;

  /** The color we use. */
  csVector4 color;

  /** The alternate color we might use. */
  csVector4 alt_color;

  /** The translation we keep for text. */
  csVector3 tt;

  /** The color array generated for verts as we render. */
  csDirtyAccessArray<csVector4> colors;

  /** The array that stores the transformation stack. */
  csArray<csReversibleTransform> transforms;

protected:
  /** 
   * Initializes our working objects. 
   */
  void Start();

  /** 
   * Adds a vertex. 
   */
  void AddVertex(float x, float y);

  /** 
   * Worker, sets up the mesh with the vertices, color, and other information. 
   */
  void SetupMesh();

  /** 
   * Worker, draws the mesh. 
   */
  void DrawMesh(csRenderMeshType mesh_type);

public:
  csPen(iGraphics2D *_g2d, iGraphics3D *_g3d);
  virtual ~csPen();

  /** 
   * Sets the current color. 
   */
  virtual void SetColor (float r, float g, float b, float a);

  /** 
   * Sets the current color. 
   */
  virtual void SetColor(const csColor4 &color);  

  /**
   * Swaps the current color and the alternate color. 
   */
  virtual void SwapColors();

  /**    
   * Clears the current transform, resets to identity.
   */
  virtual void ClearTransform();

  /** 
   * Pushes the current transform onto the stack. *
   */
  virtual void PushTransform();

  /**
   * Pops the transform stack. The top of the stack becomes the current
   * transform. 
   */
  virtual void PopTransform();

  /** 
   * Sets the origin of the coordinate system. 
   */
  virtual void SetOrigin(const csVector3 &o);

  /** 
   * Translates by the given vector
   */
  virtual void Translate(const csVector3 &t);

  /**
   * Rotates by the given matrix.
   */
  virtual void Rotate(const float &a);

  /** 
   * Draws a single line. 
   */
  virtual void DrawLine (uint x1, uint y1, uint x2, uint y2);

  /** 
   * Draws a single point. 
   */
  virtual void DrawPoint (uint x1, uint y2);

  /** 
   * Draws a rectangle. 
   */
  virtual void DrawRect (uint x1, uint y1, uint x2, uint y2,
  	bool swap_colors = false, bool fill = false);

  /** 
   * Draws a mitered rectangle. The miter value should be between 0.0 and 1.0, 
   * and determines how much of the corner is mitered off and beveled. 
   */
  virtual void DrawMiteredRect (uint x1, uint y1, uint x2, uint y2, 
    float miter, bool swap_colors = false, bool fill = false);

  /** 
   * Draws a rounded rectangle. The roundness value should be between
   * 0.0 and 1.0, and determines how much of the corner is rounded off. 
   */
  virtual void DrawRoundedRect (uint x1, uint y1, uint x2, uint y2, 
    float roundness, bool swap_colors = false, bool fill = false);

  /** 
   * Draws an elliptical arc from start angle to end angle.  Angle must be
   * specified in radians. The arc will be made to fit in the given box.
   * If you want a circular arc, make sure the box is a square.  If you want
   * a full circle or ellipse, specify 0 as the start angle and 2*PI as the end
   * angle.
   */
  virtual void DrawArc(uint x1, uint y1, uint x2, uint y2,
  	float start_angle=0, float end_angle=6.2831853, 
    bool swap_colors = false, bool fill=false);

  /**
   * Draws a triangle around the given vertices. 
   */
  virtual void DrawTriangle(uint x1, uint y1, uint x2, uint y2, uint x3, uint y3, bool fill=false);

  /**
   * Writes text in the given font at the given location.
   */
  virtual void Write(iFont *font, uint x1, uint y1, char *text);

  /**
   * Writes text in the given font, in the given box.  The alignment
   * specified in h_align and v_align determine how it should be aligned.  
   */
  virtual void WriteBoxed(iFont *font, uint x1, uint y1, uint x2, uint y2, 
    uint h_align, uint v_align, char *text);
};


#endif
