/*
    Copyright (c) 2000 Andrew Zabolotny

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

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "cssysdef.h"
#include "cssys/sysdriv.h"
#include "video/canvas/common/graph2d.h"
#include "csutil/util.h"

#include "gl2d_font.h"

#if defined (OPENGL_BITMAP_FONT)

class GLFontInfo
{
  friend class csGraphics2DOpenGLFontServer;
public:
  GLFontInfo (int fontId, int nGlyphs);
  ~GLFontInfo ();
  void DefineGlyph (int c, iFontServer *pFS);

  // Font identifier
  int FontID;
  // Number of the glyphs
  int NumGlyphs;
  // All the glyphs
  unsigned char **Glyph;
  // Max character height
  int Height;
  // Per-glyph info
  struct GLGlyphInfo
  {
    // Glyph width and height
    unsigned char w, h, bw;
  } *GlyphInfo;
};
  
GLFontInfo::GLFontInfo (int fontId, int nGlyphs)
{
  FontID = fontId;
  Glyph = new unsigned char * [NumGlyphs = nGlyphs];
  GlyphInfo = new GLGlyphInfo [NumGlyphs];
  memset (Glyph, 0, nGlyphs * sizeof (unsigned char *));
  Height = 0;
}

GLFontInfo::~GLFontInfo ()
{
  delete [] GlyphInfo;
  for (int c = 0; c < NumGlyphs; c++)
    delete [] Glyph [c];
  delete [] Glyph;
}

void GLFontInfo::DefineGlyph (int c,
  iFontServer *pFS)
{
  int width, height;
  // points to location of the font source data
  unsigned char *font;
  font = pFS->GetGlyphBitmap (FontID, c, width, height);
  int bytes = (width + 7) / 8;

  if (height > Height)
    Height = height;

  GlyphInfo [c].w = width;
  GlyphInfo [c].h = height;
  GlyphInfo [c].bw = bytes;

  // Now we'll have to reverse character lines from bottom up
  unsigned char *data = new unsigned char [bytes * height];
  for (int line = 0; line < height; line++)
    memcpy (data + bytes * (height - 1 - line), font + bytes * line, bytes);
  delete [] Glyph [c];
  Glyph [c] = data;
}

//-------------------------------------// csGraphics2DOpenGLFontServer //-----//

/* The constructor initializes it member variables and constructs the
 * first font, if one was passed into the constructor
 */
csGraphics2DOpenGLFontServer::csGraphics2DOpenGLFontServer (iFontServer *pFS)
  : FontCache (8, 8), FontServer (pFS)
{
}

csGraphics2DOpenGLFontServer::~csGraphics2DOpenGLFontServer ()
{
  // kill all the font data we have accumulated
  for (int index = 0; index < FontCache.Length (); index++)
    delete FontCache.Get (index);
}

void csGraphics2DOpenGLFontServer::AddFont (int fontId)
{
  GLFontInfo *font = new GLFontInfo (fontId, 256);
  FontCache.Push (font);

  // Now get characters from font server, one by one,
  // and store them into the lists we have allocated
  for (int c = 0; c < 256; c++)
    font->DefineGlyph (c, FontServer);
}

void csGraphics2DOpenGLFontServer::Write (int x, int y, const char *text,
  int Font)
{
  if (!text || !*text) return;

  GLFontInfo *font = FontCache.Get (Font);
  y = y - font->Height + 1;
  if (y >= ClipY2) return;

  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
  glRasterPos2i (x < ClipX1 ? ClipX1 : x, y < ClipY1 ? ClipY1 : y);

  int dy = (y < ClipY1) ? ClipY1 - y : 0;
  if (dy >= font->Height)
    return;

  while (*text)
  {
    unsigned char c = *text++;
    int w = font->GlyphInfo [c].w;
    int h = font->GlyphInfo [c].h;

    // Perform clipping
    if (x >= ClipX2)
      return;

    int dx = 0;
    if (x < ClipX1)
    {
      dx = ClipX1 - x;
      if (w <= dx) { x += w; continue; }
      x = ClipX1; w -= dx;
    }
    if ((h -= dy) <= 0)
      continue;

    if (x + w >= ClipX2)
      w = ClipX2 - x;
    if (y + h >= ClipY2)
      h = ClipY2 - y;

    glBitmap (w, h, dx, 0, w, 0, font->Glyph [c] + font->GlyphInfo [c].bw * dy);
    x += w;
  }
}

#endif
