/*
    Copyright (C) 1999 by Gary Haussmann

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


#include "video/canvas/common/graph2d.h"
#include <GL/gl.h>
#include "ifontsvr.h"

/**
  This class contains
  basic code to read font information and build appropriate bitmaps in
  OpenGL.  It acts as a GL 'font server'.  You add a font by passing
  in a FontDef struct (see graph2d.h), and the server will add it to
  the list of fonts.  Destruction of the server will free up all the
  bitmaps currently in use.

  To write a series of characters, set the color and position using
  glColor(r,g,b) and glRasterPos2i(x,y)  Note that the raster
  position points to the lower left corner of the first character!
  Then call WriteChars() with the string to print.  If you want to
  use a different font than the default font (the first font built
  in the server) there exists a version of WriteChars() for that
  as well
  */
class csGraphics2DOpenGLFontServer
{
    /// number of fonts currently stored here
    int Font_Count;

    /** each font is stored as a set of opengl display lists, one
     * per character.  Each display list, or character, has a number; the
     * number of the first character for each font is stored here
     */
    GLuint *Font_Offsets;

    iFontServer *pFontServer;
    /// Build a font from font data
    void BuildFont(int iFont);

  public:
    /** The font server starts with 0 fonts in it, but you can pass
     * one in immediately to the constructor and start with one font.
     * Additional fonts must be added via AddFont()
     */
    csGraphics2DOpenGLFontServer(int nFonts, iFontServer *pFR);

    /** Add more fonts to the font server by passing FontDef's into this
     * method.  The font bitmap data will be encoded into an openGL-friendly
     * form
     */
    void AddFont(int fontId);

    /// Check how many fonts are stored in here
    int CountFonts() const { return Font_Count; }

    /** Draw a string using OpenGL.  It is assumed you have
     * set up the render state using glColor and glRasterPos.
     * to use a non-default font, pass in a second argument with
     * the number of the font to use */
    void WriteCharacters(char *writeme, int fontnumber = 0);

    /** Draw a single character using OpenGL.  It is assumed you have
     * set up the render state using glColor and glRasterPos.
     * to use a non-default font, pass in a second argument with
     * the number of the font to use */
    void WriteCharacter(char writeme, int fontnumber = 0);

    /// Destructor cleans up all the OpenGL mess we left around
    ~csGraphics2DOpenGLFontServer();
};




