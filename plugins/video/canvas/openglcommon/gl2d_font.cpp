/*
    Copyright (c) 1999 Gary Haussmann
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

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "cssysdef.h"
#include "cssys/sysdriv.h"
#include "video/canvas/common/graph2d.h"
#include "csutil/util.h"
#include "qint.h"

#include "gl2d_font.h"

#if !defined (OPENGL_BITMAP_FONT)

struct GLGlyph
{
  GLuint hTexture; // the texture where we find this character in
  float x, y, width, texwidth; // location and size of the character
};

class GLFontInfo
{
  friend class csGraphics2DOpenGLFontServer;
  public:

    GLFontInfo (){}
    ~GLFontInfo ();

    GLGlyph glyphs[256];
    float height, texheight;
    bool one_texture;
};

/* MACOS driver do not currently support GL_ALPHA textures needed for
 * shaped text, so for that platform we have a hack that gives blocky
 * text.  This is superior to the other option which is NO text. -GJH 05-20-2000
 */

GLFontInfo::~GLFontInfo ()
{
  GLuint hTex = glyphs [0].hTexture + 1;
  for (int i = 0; i < 256; i++)
  {
    if (hTex != glyphs[i].hTexture)
    {
      hTex = glyphs [i].hTexture;
      glDeleteTextures (1, &glyphs [i].hTexture);
    }
  }
}


//--------------------------------------------------------------------------------
//----------------------------------------------csGraphics2DOpenGLFontServer------
//--------------------------------------------------------------------------------

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
  GLFontInfo *font = new GLFontInfo ();
  FontCache.Push (font);

  unsigned c;
  float x, y;
  int width, height;
  int rows=1;

  x = y = 256.0;

  int maxheight;
  font->height = maxheight = FontServer->GetMaximumHeight (fontId);
  const int basetexturewidth = 256;

  // figure out how many charcter rows we need
  int texwidth = 0;
  for (c = 0; c < 256; c++)
  {
    FontServer->GetGlyphSize (fontId, c, width, height);
    texwidth += width;
    if (texwidth > 256)
    {
      rows++;
      texwidth = width;
    }
  }

  int basetextureheight = FindNearestPowerOf2 (MIN (maxheight * rows, 256));
  font->texheight = ((float)maxheight) / basetextureheight;

  int nTextures = 1 + rows / (256 / maxheight);
  if (nTextures > 1)
    font->one_texture = false;
  else
    font->one_texture = true;

  int nCurrentTex = 0;
  unsigned int basepixelsize = 1;
  unsigned char *fontbitmapdata, *characterbitmapbase;
  bool GlyphBitsNotByteAligned;
  fontbitmapdata = new unsigned char [basetexturewidth * basetextureheight * basepixelsize];
  memset (fontbitmapdata, 0, basetexturewidth * basetextureheight * basepixelsize);
  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
  // make new texture handles
  GLuint *nTexNames = new GLuint [nTextures];
  glGenTextures (nTextures, nTexNames);

  for (c = 0; c < 256; c++)
  {
    unsigned char *fontsourcebits = FontServer->GetGlyphBitmap (
      fontId, c, width, height);
    GlyphBitsNotByteAligned = width & 7;

    // calculate the start of this character
    if (x + width > 256)
    {
      x = 0;
      y += maxheight;
      if (y + maxheight > 256)
      {
        y = 0;
	// if this is not the first handle we create, we hand over the data
	// to opengl
	if (c)
	{
#ifdef OS_MACOS
          glTexImage2D (GL_TEXTURE_2D, 0 /*mipmap level */,
			GL_LUMINANCE /* bytes-per-pixel */,
                        basetexturewidth, basetextureheight, 0 /*border*/,
                        GL_LUMINANCE, GL_UNSIGNED_BYTE, fontbitmapdata);
#else
          glTexImage2D (GL_TEXTURE_2D, 0 /*mipmap level */,
			GL_ALPHA /* bytes-per-pixel */,
                        basetexturewidth, basetextureheight, 0 /*border*/,
                        GL_ALPHA, GL_UNSIGNED_BYTE, fontbitmapdata);
#endif
	  nCurrentTex++;
        }
        // set up the texture info
        glBindTexture (GL_TEXTURE_2D, nTexNames [nCurrentTex]);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      }
    }

    characterbitmapbase = fontbitmapdata + ((int)y) * basetexturewidth *
      basepixelsize + ((int)x) * basepixelsize;

    // Construct a buffer of data for OpenGL. We must do some transformation
    // of the Crystal Space data:
    // -use unsigned bytes instead of bits (GL_BITMAP not supported? GJH)
    // -width and maxheight must be a power of 2
    // -characters are laid out in a grid format, going across and
    //  then down

    // grab bits from the source, and stuff them into the font bitmap
    // one at a time
    unsigned char currentsourcebyte = *fontsourcebits;
    for (int pixely = 0; pixely < maxheight; pixely++)
    {
      for (int pixelx = 0; pixelx < width; pixelx++)
      {
        // strip a bit off and dump it into the base bitmap
        *characterbitmapbase++ = (currentsourcebyte & 128) ? 255 : 0;
        if ((pixelx & 7) == 7)
          currentsourcebyte = *++fontsourcebits;
        else
          currentsourcebyte = currentsourcebyte << 1;
      }
      if (GlyphBitsNotByteAligned)
        currentsourcebyte = *++fontsourcebits;
      // advance base bitmap pointer to the next row
      if (pixely + 1 != maxheight)
        characterbitmapbase += (basetexturewidth - width) *basepixelsize;
    }
    font->glyphs [c].width = width;
    font->glyphs [c].texwidth = ((float)width) / basetexturewidth ;
    font->glyphs [c].x = x / basetexturewidth;
    font->glyphs [c].y = y / basetextureheight;
    font->glyphs [c].hTexture = nTexNames [nCurrentTex];

    x += width;
  }

#ifdef OS_MACOS
  glTexImage2D (GL_TEXTURE_2D, 0 /*mipmap level */,
		GL_LUMINANCE /* bytes-per-pixel */,
                basetexturewidth, basetextureheight, 0 /*border*/,
                GL_LUMINANCE, GL_UNSIGNED_BYTE, fontbitmapdata);
#else
  glTexImage2D (GL_TEXTURE_2D, 0 /*mipmap level */,
		GL_ALPHA /* bytes-per-pixel */,
                basetexturewidth, basetextureheight, 0 /*border*/,
                GL_ALPHA, GL_UNSIGNED_BYTE, fontbitmapdata);
#endif

  delete [] nTexNames;
  delete [] fontbitmapdata;
}

bool csGraphics2DOpenGLFontServer::ClipRect (float x, float y,
  float &x1, float &y1, float &x2, float &y2,
  float &tx1, float &ty1, float &tx2, float &ty2)
{
  float nx1 = x1 + x, ny1 = y1 + y, nx2 = x2 + x, ny2 = y2 + y;
  float ntx1 = tx1, nty1 = ty1, ntx2 = tx2, nty2 = ty2;

  if ((nx1 > float (ClipX2)) || (nx2 < float (ClipX1))
   || (ny1 > float (ClipY2)) || (ny2 < float (ClipY1)))
      return false;

  if (nx1 < ClipX1)
    tx1 += (ntx2 - ntx1) * (ClipX1 - nx1) / (nx2 - nx1), x1 = ClipX1 - x;
  if (nx2 > ClipX2)
    tx2 -= (ntx2 - ntx1) * (nx2 - ClipX2) / (nx2 - nx1), x2 = ClipX2 - x;
  if (tx2 <= tx1)
    return false;

  if (ny1 < ClipY1)
    ty2 -= (nty2 - nty1) * (ClipY1 - ny1) / (ny2 - ny1), y1 = ClipY1 - y;
  if (ny2 > ClipY2)
    ty1 += (nty2 - nty1) * (ny2 - ClipY2) / (ny2 - ny1), y2 = ClipY2 - y;
  if (ty2 <= ty1)
    return false;

  return true;
}

// Rasterize string
void csGraphics2DOpenGLFontServer::Write (int x, int y, const char *text, int Font)
{
  if (!text || !*text) return;

  y = y - QInt (FontCache.Get (Font)->height) + 1;
  if (y >= ClipY2) return;

  GLGlyph *glyphs = FontCache.Get (Font)->glyphs;
  GLuint hLastTexture = 0;
  GLuint hTexture     = glyphs[*text].hTexture;

  glPushMatrix();
  glTranslatef (x, y, 0);

  glEnable(GL_TEXTURE_2D);
  glShadeModel(GL_FLAT);
  glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  // bind the texture containing this font
  glBindTexture(GL_TEXTURE_2D, hTexture);

#ifndef OS_MACOS
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc (GL_EQUAL,1.0);
#endif

  glBegin (GL_QUADS);

  float tx1, tx2, ty1, ty2, x1, x2, y1, y2, x_right;
  float maxheight = FontCache.Get (Font)->height;
  float texheight = FontCache.Get (Font)->texheight;
  bool bOneTexture = FontCache.Get (Font)->one_texture;

  x1 = 0.0;

  for (; *text; ++text)
  {
    x_right = x2 = x1 + glyphs[*text].width;

    if (!bOneTexture)
    {
      hTexture = glyphs[*text].hTexture;
      if (hTexture != hLastTexture)
      {
        hLastTexture = hTexture;
        glBindTexture(GL_TEXTURE_2D, hTexture);
      }
    }
    // the texture coordinates must point to the correct character
    // the texture is a strip a wide as a single character and
    // as tall as 256 characters.  We must select a single
    // character from it
    tx1 = glyphs[*text].x;
    tx2 = tx1 + glyphs[*text].texwidth;
    ty1 = glyphs[*text].y;
    ty2 = ty1 + texheight;
    y1 = 0.0;
    y2 = maxheight;

    if (ClipRect (x, y, x1, y1, x2, y2, tx1, ty1, tx2, ty2))
    {
      glTexCoord2f (tx1,ty1); glVertex2f (x1,y2);
      glTexCoord2f (tx2,ty1); glVertex2f (x2,y2);
      glTexCoord2f (tx2,ty2); glVertex2f (x2,y1);
      glTexCoord2f (tx1,ty2); glVertex2f (x1,y1);
    }

    x1 = x_right;
  }

  glEnd ();

#ifndef OS_MACOS
  glDisable(GL_ALPHA_TEST);
#endif
  glPopMatrix ();
}

#endif
