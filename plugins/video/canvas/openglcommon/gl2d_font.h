/*
    Copyright (C) 1999 by Gary Haussmann
    Accelerated by Samuel Humphreys

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

#ifndef __CS_GL2D_FONT_H__
#define __CS_GL2D_FONT_H__

#include <GL/gl.h>
#include "csutil/csvector.h"
#include "ifontsrv.h"

// Define the following macro to use "font-on-a-texture" approach.
// Otherwise we will use glBitmap()
// Currently this is defined in csosdefs.h for platforms with
// buggy OpenGL's.
//#define OPENGL_BITMAP_FONT

class GLFontInfo;

/**
  This class contains
  basic code to read font information and build appropriate bitmaps in
  OpenGL.  It acts as a GL 'font server'.  You add a font by passing
  in a FontDef struct (see graph2d.h), and the server will add it to
  the list of fonts.  Destruction of the server will free up all the
  bitmaps currently in use.
*/
class csGraphics2DOpenGLFontServer
{
protected:
  // The font cache array
  class csFontVector : public csVector
  {
  public:
    csFontVector (int iLimit, int iDelta) : csVector (iLimit, iDelta)
    { }
    GLFontInfo *Get (int iIndex)
    { return (GLFontInfo *)csVector::Get (iIndex); }
  } FontCache;
  // The font server
  iFontServer *FontServer;
  
  /// the current clipping rect
  int ClipX1, ClipY1, ClipX2, ClipY2;

#ifndef OPENGL_BITMAP_FONT
  // Auxiliary routine for "font-on-a-texture" approach
  bool csGraphics2DOpenGLFontServer::ClipRect (float x, float y,
    float &x1, float &y1, float &x2, float &y2, 
    float &tx1, float &ty1, float &tx2, float &ty2);
#endif

public:
  /**
   * The maximal number of fonts that can be registered.
   * Additional fonts must be added via AddFont()
   */
  csGraphics2DOpenGLFontServer (iFontServer *pFR);

  /// Destructor cleans up all the OpenGL mess we left around
  ~csGraphics2DOpenGLFontServer ();

  /**
   * Add more fonts to the font server by passing FontDef's into this
   * method.  The font bitmap data will be encoded into an openGL-friendly
   * form
   */
  void AddFont (int fontId);

  /**
   * Draw a string using OpenGL at x,y.  It is assumed you have
   * set up the render state using glColor.
   */
  void Write (int x, int y, const char *text, int Font);

  void SetClipRect (int x1, int y1, int x2, int y2)
  { ClipX1 = x1; ClipY1 = y1; ClipX2 = x2; ClipY2 = y2; }   
};

#endif // __CS_GL2D_FONT_H__
