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
#include "cs2d/common/graph2d.h"
#include "isystem.h"
#include "csutil/util.h"

#include "cs2d/openglcommon/gl2d_font.h"

/// we need a definition of GLFontInfo, declared in the header file
class csGraphics2DOpenGLFontServer::GLFontInfo
{
  public:
    /** Constructor.  Pass in a CS font, which will be analyzed and
     *  used to build a texture holding all the characters */ 
    GLFontInfo(FontDef &newfont);

    /** destructor.  Destroys the texture that holds the font characters. */
    ~GLFontInfo() {
        glDeleteTextures(1,&mTexture_Handle);
	}

    /** call this to draw a character.  The character is drawn on the
     *  screen using the current color, and a transform is applied
     *  to the modelview matrix to shift to the right.  This is
     *  such that the next call to DrawCharacter
     *  will draw a character in the next cell over; that way you can
     *  make repeated calls without having to manually position each character */
    void DrawCharacter(unsigned char characterindex);

  private:

    /// handle referring to the texture used for this font
    GLuint mTexture_Handle;

    /// size of characters in the texture, in texture coordinates
    float mTexture_CharacterWidth;
    float mTexture_CharacterHeight;

    // size of characters in screen pixels
    unsigned int mCharacterWidth;
    unsigned int mCharacterHeight;

    // describes layout of the characters in the texture
    unsigned int mTexture_Characters_Per_Row;
};

csGraphics2DOpenGLFontServer::GLFontInfo::GLFontInfo(FontDef &newfont)
{
    // allocate handle for a new texture to hold this font
    glGenTextures (1,&mTexture_Handle);

    // set up the texture info
    glBindTexture (GL_TEXTURE_2D, mTexture_Handle);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    // Construct a buffer of data for OpenGL. We must do some transformation
    // of the Crystal Space data:
    // -use unsigned bytes instead of bits (GL_BITMAP not supported? GJH)
    // -width and height must be a power of 2
    // -characters are laid out in a grid format, going across and
    //  then down

    // first figure out how many characters to cram into a row, and
    // thus how many rows.  There are 256 pixels in a row, and
    // 256 characters in the font as well.

    const int basetexturewidth = 256;
    const int fontcharactercount = 128;

    mCharacterHeight = newfont.Height;
    mCharacterWidth = FindNearestPowerOf2(newfont.Width);
    mTexture_Characters_Per_Row = basetexturewidth / mCharacterWidth;
    int fontcharacterrows = fontcharactercount / mTexture_Characters_Per_Row;

    int basetextureheight = 
    	FindNearestPowerOf2 ( newfont.Height * fontcharacterrows );

    // now figure out the size of a single character in the texture,
    // in texture coordinate space
    mTexture_CharacterWidth = mCharacterWidth / (float)basetexturewidth;
    mTexture_CharacterHeight = mCharacterHeight / (float)basetextureheight;

    // build bitmap data in a format proper for OpenGL
    // each pixel has 1 byte intensity + 1 byte alpha
    unsigned int basepixelsize=1;
    unsigned char *fontbitmapdata;
    CHK (fontbitmapdata = 
    	new unsigned char [ basetexturewidth * basetextureheight * basepixelsize ] );

    // transform each CS character onto a small rectangular section
    // of the fontbitmap we have allocated for this font
    for (int curcharacter = 0; curcharacter<128; curcharacter++)
    {
      // locations in fontbitmap at which the current character will
      // be stored
      unsigned int curcharx = mCharacterWidth *
      		(curcharacter % mTexture_Characters_Per_Row);
      unsigned int curchary = mCharacterHeight *
      		(curcharacter / mTexture_Characters_Per_Row);
      unsigned char *characterbitmapbase = 
    		fontbitmapdata + (curcharx + curchary * basetexturewidth * basepixelsize);
      
      // points to location of the font source data
      unsigned char *fontsourcebits = 
      	newfont.FontBitmap + curcharacter * newfont.BytesPerChar;

      // grab bits from the source, and stuff them into the font bitmap
      // one at a time
      for (unsigned int pixely = 0; pixely < mCharacterHeight; pixely++)
      {
        // grab a whole byte from the source; we will strip off the
	// bits one at a time
        unsigned char currentsourcebyte = *fontsourcebits++;
	unsigned char destbytesetting;

        for (unsigned int pixelx = 0; pixelx < mCharacterWidth; pixelx++)
        {
	  // strip a bit off and dump it into the base bitmap
	  destbytesetting = (currentsourcebyte & 128) ? 255 : 0;
	  *characterbitmapbase++ = destbytesetting;
	  currentsourcebyte = currentsourcebyte << 1;
	}

	// advance base bitmap pointer to the next row
	characterbitmapbase += (basetexturewidth - mCharacterWidth) *basepixelsize;
      }
    }

    // shove all the data at OpenGL... 
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glTexImage2D(GL_TEXTURE_2D,
   		0 /*mipmap level */,
		GL_LUMINANCE,
    		basetexturewidth,basetextureheight,
		0 /*border*/,
    		GL_LUMINANCE,GL_UNSIGNED_BYTE,fontbitmapdata);

    CHK ( delete [] fontbitmapdata );
}

void csGraphics2DOpenGLFontServer::GLFontInfo::DrawCharacter(unsigned char characterindex)
{
  // bind the texture containing this font
  glBindTexture(GL_TEXTURE_2D,mTexture_Handle);

  // other required settings
  glEnable(GL_TEXTURE_2D);
  glShadeModel(GL_FLAT);
  glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);


  // figure out location of the character in the texture
  float chartexturex = mTexture_CharacterWidth * (characterindex % mTexture_Characters_Per_Row);
  float chartexturey = mTexture_CharacterHeight * (characterindex / mTexture_Characters_Per_Row);

  // the texture coordinates must point to the correct character
  // the texture is a strip a wide as a single character and
  // as tall as 256 characters.  We must select a single
  // character from it
  float tx1=chartexturex, tx2 = chartexturex + mTexture_CharacterWidth;
  float ty1=chartexturey, ty2 = chartexturey + mTexture_CharacterHeight;

  float x1=0.0, x2=mCharacterWidth;
  float y1=0.0, y2=mCharacterHeight;

  glBegin (GL_QUADS);
  glTexCoord2f (tx1,ty1); glVertex2f (x1,y2);
  glTexCoord2f (tx2,ty1); glVertex2f (x2,y2);
  glTexCoord2f (tx2,ty2); glVertex2f (x2,y1);
  glTexCoord2f (tx1,ty2); glVertex2f (x1,y1);
  glEnd ();

  glTranslatef (8.0,0.0,0.0);
}

/** The constructor initializes it member variables and constructs the
 * first font, if one was passed into the constructor */
csGraphics2DOpenGLFontServer::csGraphics2DOpenGLFontServer(FontDef *startfont)
    : mFont_Count(0), mFont_Information_Array(NULL)
{

    // initialize a first font, if we have one
    if (startfont != NULL)
	BuildFont(*startfont);
}

csGraphics2DOpenGLFontServer::~csGraphics2DOpenGLFontServer()
{
    // kill all the font data we have accumulated
    if (mFont_Information_Array)
    {
      // cycle through all loaded fonts
      for (int index=0; index < mFont_Count; index++)
      {
        CHK ( delete mFont_Information_Array[index] );
      }
      CHK ( delete [] mFont_Information_Array );
    }
}

void csGraphics2DOpenGLFontServer::BuildFont(FontDef &newfont)
{
    // we assume the FontDef is legal...

    // We need to make room for another texture handle holding
    // data for the new font

    // need another spot in the array of handles
    if (mFont_Information_Array != NULL)
    {
    	GLFontInfo **newhandles;
	CHK ( newhandles = new GLFontInfo*[mFont_Count+1] );
	for (int index=0; index < mFont_Count; index++)
		newhandles[index] = mFont_Information_Array[index];
	
        CHK ( delete[] mFont_Information_Array );
	mFont_Information_Array = newhandles;
    }
    else
    	CHK ( mFont_Information_Array = new GLFontInfo*[1] );

    mFont_Information_Array[mFont_Count] = new GLFontInfo(newfont);

    // need to know there is another font stored here
    mFont_Count++;
}

void csGraphics2DOpenGLFontServer::AddFont(FontDef &addme)
{
    BuildFont(addme);
}

/** Print some characters (finally!)  This is basically a wrapper
 * around repeated calls to WriteCharacter */
void csGraphics2DOpenGLFontServer::WriteCharacters(char *writeme, int fontnumber)
{
    // do some error checking
    if (mFont_Count < 1) return;

    if (fontnumber >= mFont_Count)
        fontnumber = 0;
    
    // write the string
    for (char *curcharacter = writeme; *curcharacter != 0; curcharacter++)
    {
      WriteCharacter(*curcharacter, fontnumber);
    }
}

/** Print a character. This is basically a wrapper
 * around calls to the GLFontInfo method WriteCharacter() */
void csGraphics2DOpenGLFontServer::WriteCharacter(char writeme, int fontnumber)
{
    // do some error checking
    if (mFont_Count < 1) return;

    if (fontnumber >= mFont_Count)
        fontnumber = 0;

    mFont_Information_Array[fontnumber]->DrawCharacter(writeme);
}

