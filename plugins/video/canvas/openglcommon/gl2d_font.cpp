/*
   Copyright (c) 1999 Gary Haussmann

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

#include "sysdef.h"
#include "cssys/sysdriv.h"
#include "video/canvas/common/graph2d.h"
#include "csutil/util.h"

#include "video/canvas/openglcommon/gl2d_font.h"

csGraphics2DOpenGLFontServer::GLFontInfo::~GLFontInfo ()
{
  GLuint hTex= glyphs[0].hTexture + 1;
  for(int i=0; i < 256; i++)
  {
    if (hTex != glyphs[i].hTexture)
    {
      hTex = glyphs[i].hTexture;
      glDeleteTextures (1, &glyphs[i].hTexture); 
    }
  }
}

void csGraphics2DOpenGLFontServer::AddFont (int fontId)
{
  if (mFont_Count >= mMax_Font_Count)
    return;
  // we assume the FontDef is legal...
  GLFontInfo *font = new GLFontInfo;
  mFont_Information_Array [mFont_Count++] = font;

  unsigned char c=0;
  float x, y;
  int width, height;
  int rows=1;
  
  x=y=256.0;
  height = pFontRender->GetCharHeight (fontId, 'T'); // just a dummy parameter

  font->height = height;

  const int basetexturewidth = 256;
  // figure out how many charcter rows we need
  width=0;
  while (1)
  {
    width+= pFontRender->GetCharWidth (fontId, c);
    if (width>256)
    {
      rows++;
      width= pFontRender->GetCharWidth (fontId, c);
    }
    if (c==255) break;
    else c++;
  }
  
  int basetextureheight = FindNearestPowerOf2 (MIN(height * rows, 256));
  font->texheight = ((float)height) / basetextureheight;

  int nTextures = 1 + rows/(256/height);
  GLuint *nTexNames = new GLuint[ nTextures ];
  int nCurrentTex=0;
  unsigned int basepixelsize = 1;
  unsigned char *fontbitmapdata, *characterbitmapbase;
  bool GlyphBitsNotByteAligned;
  fontbitmapdata = new unsigned char [basetexturewidth * basetextureheight * basepixelsize];
  memset (fontbitmapdata, 0, basetexturewidth * basetextureheight * basepixelsize );
  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
  // make new textur handles
  glGenTextures (nTextures,nTexNames);

  c=0;
  while (1)
  {
    width = pFontRender->GetCharWidth (fontId, c);
    GlyphBitsNotByteAligned = width&7;

    // calculate the start of this character
    if (x+width > 256)
    {
      x = 0;
      y+=height;
      if (y+height > 256)
      {
        y=0;
	// if this is not the first handle we create, we hand over the data to opengl
	if (c)
	{
          glTexImage2D (GL_TEXTURE_2D, 0 /*mipmap level */, GL_ALPHA /* bytes-per-pixel */,
                        basetexturewidth, basetextureheight, 0 /*border*/,
                        GL_ALPHA, GL_UNSIGNED_BYTE, fontbitmapdata);
	  nCurrentTex++;
        }
        // set up the texture info
        glBindTexture (GL_TEXTURE_2D, nTexNames[nCurrentTex]);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

      }
    }

    characterbitmapbase = fontbitmapdata + ((int)y)*basetexturewidth*basepixelsize 
                          + ((int)x)*basepixelsize;

    // Construct a buffer of data for OpenGL. We must do some transformation
    // of the Crystal Space data:
    // -use unsigned bytes instead of bits (GL_BITMAP not supported? GJH)
    // -width and height must be a power of 2
    // -characters are laid out in a grid format, going across and
    //  then down

    // points to location of the font source data
    unsigned char *fontsourcebits =  pFontRender->GetCharBitmap (fontId, c);

    // grab bits from the source, and stuff them into the font bitmap
    // one at a time
    unsigned char currentsourcebyte = *fontsourcebits;
    for (int pixely = 0; pixely < height; pixely++)
    {
      for (int pixelx = 0; pixelx < width; pixelx++)
      {
        // strip a bit off and dump it into the base bitmap
        *characterbitmapbase++ = (currentsourcebyte & 128) ? 255 : 0;
        if ((pixelx&7)==7) currentsourcebyte = *++fontsourcebits;
        else currentsourcebyte = currentsourcebyte << 1;
      }
      if (GlyphBitsNotByteAligned){ currentsourcebyte = *++fontsourcebits; }
      // advance base bitmap pointer to the next row
      if ( pixely+1 != height ) characterbitmapbase += (basetexturewidth - width) *basepixelsize;
    }
    font->glyphs[c].width = width;
    font->glyphs[c].texwidth = ((float)width) / basetexturewidth ;
    font->glyphs[c].x = x / basetexturewidth;
    font->glyphs[c].y = y / basetextureheight;
    font->glyphs[c].hTexture = nTexNames[nCurrentTex];

    x += width;
    if (c==255) break;
    else c++;
  }

  glTexImage2D (GL_TEXTURE_2D, 0 /*mipmap level */, GL_ALPHA /* bytes-per-pixel */,
                basetexturewidth, basetextureheight, 0 /*border*/,
                GL_ALPHA, GL_UNSIGNED_BYTE, fontbitmapdata);

  delete [] nTexNames;
  CHK (delete [] fontbitmapdata);
}

void csGraphics2DOpenGLFontServer::GLFontInfo::DrawCharacter (
  unsigned char characterindex)
{
  // bind the texture containing this font
  glBindTexture(GL_TEXTURE_2D, glyphs[characterindex].hTexture);

  // other required settings
  glEnable(GL_TEXTURE_2D);
  glShadeModel(GL_FLAT);
  glEnable(GL_ALPHA_TEST);
  glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  // the texture coordinates must point to the correct character
  // the texture is a strip a wide as a single character and
  // as tall as 256 characters.  We must select a single
  // character from it
  float tx1 = glyphs[characterindex].x;
  float tx2 = tx1 + glyphs[characterindex].texwidth;
  float ty1 = glyphs[characterindex].y;
  float ty2 = ty1 + texheight;
  float x1 = 0.0, x2 = glyphs[characterindex].width;
  float y1 = 0.0, y2 = height;

  glAlphaFunc (GL_EQUAL,1.0);

  glBegin (GL_QUADS);
  glTexCoord2f (tx1,ty1); glVertex2f (x1,y2);
  glTexCoord2f (tx2,ty1); glVertex2f (x2,y2);
  glTexCoord2f (tx2,ty2); glVertex2f (x2,y1);
  glTexCoord2f (tx1,ty2); glVertex2f (x1,y1);
  glEnd ();

  glTranslatef (x2,0.0,0.0);
  glDisable(GL_ALPHA_TEST);
}

/* The constructor initializes it member variables and constructs the
 * first font, if one was passed into the constructor
 */
csGraphics2DOpenGLFontServer::csGraphics2DOpenGLFontServer (int MaxFonts, iFontRender *pFR)
  : mFont_Count (0), mMax_Font_Count (MaxFonts), mFont_Information_Array (NULL), pFontRender(pFR)
{
  CHK (mFont_Information_Array = new GLFontInfo * [MaxFonts]);
}

csGraphics2DOpenGLFontServer::~csGraphics2DOpenGLFontServer ()
{
  // kill all the font data we have accumulated
  if (mFont_Information_Array)
  {
    // cycle through all loaded fonts
    for (int index = 0; index < mFont_Count; index++)
      CHKB (delete mFont_Information_Array [index]);
    CHK (delete [] mFont_Information_Array);
  }
}

/* Print some characters (finally!)  This is basically a wrapper
 * around repeated calls to WriteCharacter
 */
void csGraphics2DOpenGLFontServer::WriteCharacters(char *writeme, int fontnumber)
{
  // do some error checking
  if (mFont_Count < 1) return;

  if (fontnumber >= mFont_Count)
    fontnumber = 0;

  // write the string
  for (char *curcharacter = writeme; *curcharacter != 0; curcharacter++)
    WriteCharacter (*curcharacter, fontnumber);
}

/* Print a character. This is basically a wrapper
 * around calls to the GLFontInfo method WriteCharacter()
 */
void csGraphics2DOpenGLFontServer::WriteCharacter(char writeme, int fontnumber)
{
  // do some error checking
  if (mFont_Count < 1) return;

  if (fontnumber >= mFont_Count)
    fontnumber = 0;

  mFont_Information_Array [fontnumber]->DrawCharacter (writeme);
}

