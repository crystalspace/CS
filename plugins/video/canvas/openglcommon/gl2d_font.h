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
#include "ifontsrv.h"

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
  struct GLGlyph 
  {
    GLuint hTexture; // the texture where we find this character in
    float x, y, width, texwidth; // location and size of the character
  };
  
  class GLFontInfo
  {
    public:
    
      GLFontInfo (){}
      ~GLFontInfo ();

      void DrawCharacter (unsigned char c);
           
      GLGlyph glyphs[256];
      float height, texheight;
      bool one_texture;
  };
  
  /// number of fonts currently stored here
  int mFont_Count, mMax_Font_Count;

  /**
   * each font needs some extra information connected to how it is stored
   * internally
   */
  GLFontInfo **mFont_Information_Array;
  iFontServer *pFontServer;
  
  /// Try and skip this..
  char space;
public:
  /**
   * The maximal number of fonts that can be registered.
   * Additional fonts must be added via AddFont()
   */
  csGraphics2DOpenGLFontServer (int MaxFonts, iFontServer *pFR);

  /// Destructor cleans up all the OpenGL mess we left around
  ~csGraphics2DOpenGLFontServer ();

  /**
   * Add more fonts to the font server by passing FontDef's into this
   * method.  The font bitmap data will be encoded into an openGL-friendly
   * form
   */
  void AddFont (int fontId);

  /// Check how many fonts are stored in here
  int CountFonts () const { return mFont_Count; }

  /**
   * Draw a string using OpenGL at x,y.  It is assumed you have
   * set up the render state using glColor.
   */

  void Write (int x, int y, int bg, const char *text, int Font);

};

#endif // __CS_GL2D_FONT_H__
