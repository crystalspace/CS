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
#include "csgeom/vector2.h"
#include "csutil/cscolor.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/ref.h"
#include "csutil/refarr.h"
#include "csutil/memfile.h"

#include "ivideo/graph3d.h"
#include "ivideo/texture.h"

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

enum CS_PEN_FLAGS
{
	CS_PEN_FILL = 1,
	CS_PEN_SWAPCOLORS= 2,	
	CS_PEN_TEXTURE_ONLY=4,
	CS_PEN_TEXTURE = 5 /* fill | 4 */	
};


/** 
 * A pen is used to draw vector shapes. 
 */
struct iPen
{
protected:
  virtual ~iPen() {}
public:
  /** 
   * Sets the given flag. 
   * @param flag The flag to set. 
   */
  virtual void SetFlag(uint flag) = 0;	
  
  /** 
   * Clears the given flag. 
   * @param flag The flag to clear. 
   */
  virtual void ClearFlag(uint flag) = 0;		
  
  /** Sets the given mix (blending) mode.
   * @param mode The mixmode to set.
   */
  virtual void SetMixMode(uint mode) = 0;
	
  /** 
   * Sets the current color.
   * @param r The red component. 
   * @param g The green component.
   * @param b The blue component.
   * @param a The alpha component.
   */
  virtual void SetColor (float r, float g, float b, float a) = 0;

  /** 
   * Sets the current color. 
   */
  virtual void SetColor(const csColor4 &color) = 0;
  
  /** 
   * Sets the texture handle. 
   * @param tex A reference to the texture to use.
   */
  virtual void SetTexture(csRef<iTextureHandle> tex) = 0;

  /**
   * Swaps the current color and the alternate color. 
   */
  virtual void SwapColors() = 0;
  
  /**
   * Sets the width of the pen for line drawing. 
   */
  virtual void SetPenWidth(float width)=0;

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
  virtual void DrawRect (uint x1, uint y1, uint x2, uint y2) = 0;
  
  /** 
   * Draws a mitered rectangle. The miter value should be between 0.0 and 1.0, 
   * and determines how much of the corner is mitered off and beveled. 
   */
  virtual void DrawMiteredRect (uint x1, uint y1, uint x2, uint y2, 
    uint miter) = 0;

  /** 
   * Draws a rounded rectangle. The roundness value should be between
   * 0.0 and 1.0, and determines how much of the corner is rounded off. 
   */
  virtual void DrawRoundedRect (uint x1, uint y1, uint x2, uint y2, 
    uint roundness) = 0; 

  /** 
   * Draws an elliptical arc from start angle to end angle.  Angle must be
   * specified in radians. The arc will be made to fit in the given box.
   * If you want a circular arc, make sure the box is a square.  If you want
   * a full circle or ellipse, specify 0 as the start angle and 2*PI as the end
   * angle.
   */
  virtual void DrawArc(uint x1, uint y1, uint x2, uint y2, float start_angle,
  	float end_angle) = 0;

  /**
   * Draws a triangle around the given vertices. 
   */
  virtual void DrawTriangle(uint x1, uint y1, uint x2, uint y2, uint x3, uint y3) = 0;

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
class CS_CRYSTALSPACE_EXPORT csPen : public iPen
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
  
  /** The texture handle that we might use. */
  csRef<iTextureHandle> tex;

  /** The translation we keep for text. */
  csVector3 tt;

  /** The color array generated for verts as we render. */
  csDirtyAccessArray<csVector4> colors;
  
  /** The texture coordinates are generated as we render too. */
  csDirtyAccessArray<csVector2> texcoords;

  /** The array that stores the transformation stack. */
  csArray<csReversibleTransform> transforms;
  
  /** The array that stores the translation stack for text. */
  csArray<csVector3> translations;
  
  /** The width of the pen for non-filled shapes. */
  float pen_width;
    
  /** The flags currently set. */
  uint flags;
    
  /** The type of a point. */  
  struct point
  {
	  float x,y;
  };
  
  /** Used to keep track of points when drawing thick lines. */
  csArray<point> line_points; 
  
  /** The last two calculated points. */
  point last[2];
  
  /** For most shapes, we can set the width and height, and enable
   * automatic texture coordinate generation. */
  float sh_w, sh_h; 
  
  /** When this is set, automatic texture coordinate generation will occur
   * in add vertex. */
  bool gen_tex_coords;

protected:
  /** 
   * Initializes our working objects. 
   */
  void Start();

  /** 
   * Adds a vertex. 
   * @param x X coord
   * @param y Y coord
   * @param force_add Forces the coordinate to be added as a vertex, instead
   *  of trying to be smart and make it a thick vertex.
   */
  void AddVertex(float x, float y, bool force_add=false);
  
  /** Adds a texture coordinate. 
   * @param x The texture's x coord.
   * @param y The texture's y coord.
   */
  inline void AddTexCoord(float x, float y);

  /** 
   * Worker, sets up the mesh with the vertices, color, and other information. 
   */
  void SetupMesh();
  
  /** 
   * Worker, draws the mesh. 
   */
  void DrawMesh(csRenderMeshType mesh_type);
  
  /**
   * Worker, sets up the pen to do auto texturing.
   */
  void SetAutoTexture(float w, float h);
  
  /**
   *  Worker, adds thick line points.  A thick point is created when the
   * pen width is greater than 1.  It uses polygons to simulate thick
   * lines.
   */
  void AddThickPoints(float x1, float y1, float x2, float y2);

public:
  csPen(iGraphics2D *_g2d, iGraphics3D *_g3d);
  virtual ~csPen();
  
   /** 
   * Sets the given flag. 
   * @param flag The flag to set. 
   */
  virtual void SetFlag(uint flag);
  
  /** 
   * Clears the given flag. 
   * @param flag The flag to clear. 
   */
  virtual void ClearFlag(uint flag);
  
  
  /** Sets the given mix (blending) mode.
   * @param mode The mixmode to set.
   */
  virtual void SetMixMode(uint mode);

  /** 
   * Sets the current color. 
   */
  virtual void SetColor (float r, float g, float b, float a);

  /** 
   * Sets the current color. 
   */
  virtual void SetColor(const csColor4 &color);  
  
  /** 
   * Sets the texture handle. 
   * @param tex A reference to the texture to use.
   */
  virtual void SetTexture(csRef<iTextureHandle> tex);

  /**
   * Swaps the current color and the alternate color. 
   */
  virtual void SwapColors();
  
  /** 
   * Sets the current pen width 
   */
  virtual void SetPenWidth(float width);
  
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
   * Draws a thick line.
   */
  void DrawThickLine(uint x1, uint y1, uint x2, uint y2);

  /** 
   * Draws a single point. 
   */
  virtual void DrawPoint (uint x1, uint y2);

  /** 
   * Draws a rectangle. 
   */
  virtual void DrawRect (uint x1, uint y1, uint x2, uint y2);

  /** 
   * Draws a mitered rectangle. The miter value should be between 0.0 and 1.0, 
   * and determines how much of the corner is mitered off and beveled. 
   */
  virtual void DrawMiteredRect (uint x1, uint y1, uint x2, uint y2, 
    uint miter);

  /** 
   * Draws a rounded rectangle. The roundness value should be between
   * 0.0 and 1.0, and determines how much of the corner is rounded off. 
   */
  virtual void DrawRoundedRect (uint x1, uint y1, uint x2, uint y2, 
    uint roundness);

  /** 
   * Draws an elliptical arc from start angle to end angle.  Angle must be
   * specified in radians. The arc will be made to fit in the given box.
   * If you want a circular arc, make sure the box is a square.  If you want
   * a full circle or ellipse, specify 0 as the start angle and 2*PI as the end
   * angle.
   */
  virtual void DrawArc(uint x1, uint y1, uint x2, uint y2,
  	float start_angle=0, float end_angle=6.2831853);

  /**
   * Draws a triangle around the given vertices. 
   */
  virtual void DrawTriangle(uint x1, uint y1, uint x2, uint y2, uint x3, uint y3);

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



/** @brief The memory pen is a caching pen object.  The commands are written
 * to memory, and then later may be replayed to some other pen. */
class CS_CRYSTALSPACE_EXPORT csMemoryPen : public iPen
{ 	    
	/** The buffer that contains the shtuff we want. */
	csMemFile *buf;      
	
	/** The textures we're currently using. */
	csRefArray<iTextureHandle> textures;
     
public:
	 csMemoryPen():buf(0) { Clear(); }
	 virtual ~csMemoryPen() {}
	  	 
	   	 
	 //////////////// Drawing ///////////////////////////////////////////
	 
	  /** Clears the draw buffer out. */
	  virtual void Clear();
	 
	  /** Draws the cached contents of this buffer into the pen. */
	  void Draw(iPen *_pen_);
	  
	  /** 
   * Sets the given flag. 
   * @param flag The flag to set. 
   */
  virtual void SetFlag(uint flag);	
  
  /** 
   * Clears the given flag. 
   * @param flag The flag to clear. 
   */
  virtual void ClearFlag(uint flag);	
  
  
  /** Sets the given mix (blending) mode.
   * @param mode The mixmode to set.
   */
  virtual void SetMixMode(uint mode);
	 
	  /** 
   * Sets the current color. 
   */
  virtual void SetColor (float r, float g, float b, float a);

  /** 
   * Sets the current color. 
   */
  virtual void SetColor(const csColor4 &color);
  
  /** 
   * Sets the texture handle. 
   * @param tex A reference to the texture to use.
   */
  virtual void SetTexture(csRef<iTextureHandle> tex);

  /**
   * Swaps the current color and the alternate color. 
   */
  virtual void SwapColors();
  
  /**
   * Sets the width of the pen for line drawing. 
   * @param width The width in pixels.
   */
  virtual void SetPenWidth(float width);

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
   * Rotates by the given angle.
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
  virtual void DrawRect (uint x1, uint y1, uint x2, uint y2);
  
  /** 
   * Draws a mitered rectangle. The miter value should be between 0.0 and 1.0, 
   * and determines how much of the corner is mitered off and beveled. 
   */
  virtual void DrawMiteredRect (uint x1, uint y1, uint x2, uint y2, uint miter);

  /** 
   * Draws a rounded rectangle. The roundness value should be between
   * 0.0 and 1.0, and determines how much of the corner is rounded off. 
   */
  virtual void DrawRoundedRect (uint x1, uint y1, uint x2, uint y2, 
    uint roundness); 

  /** 
   * Draws an elliptical arc from start angle to end angle.  Angle must be
   * specified in radians. The arc will be made to fit in the given box.
   * If you want a circular arc, make sure the box is a square.  If you want
   * a full circle or ellipse, specify 0 as the start angle and 2*PI as the end
   * angle.
   */
  virtual void DrawArc(uint x1, uint y1, uint x2, uint y2, float start_angle,
  	float end_angle);

  /**
   * Draws a triangle around the given vertices. 
   */
  virtual void DrawTriangle(uint x1, uint y1, uint x2, uint y2, uint x3, uint y3);

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
