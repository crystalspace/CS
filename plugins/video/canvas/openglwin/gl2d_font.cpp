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
#include "cs2d/openglwin/gl2d_font.h"

/** The constructor initializes it member variables and constructs the
 * first font, if one was passed into the constructor */
csGraphics2DOpenGLFontServer::csGraphics2DOpenGLFontServer(FontDef *startfont)
    : Font_Count(0), Font_Offsets(NULL)
{

    // initialize a first font, if we have one
    if (startfont != NULL)
	BuildFont(*startfont);

}

csGraphics2DOpenGLFontServer::~csGraphics2DOpenGLFontServer()
{
    // kill all the fonts data we shoved into openGL...
    // (to be added)
}

void csGraphics2DOpenGLFontServer::BuildFont(FontDef &newfont)
{
    // we assume the FontDef is legal...

    // We need to make room for another offset, which tells the
    // offset to the bank of openGL display lists used for the
    // new font.

    // need another spot in the array of offsets
    if (Font_Offsets != NULL)
    {
    	GLuint *newoffsets = new GLuint[Font_Count+1];
	for (int index=0; index < Font_Count; index++)
		newoffsets[index] = Font_Offsets[index];
	
        delete[] Font_Offsets;
	Font_Offsets = newoffsets;
    }
    else
    	Font_Offsets = new GLuint[1];

    // get a bank of list indices from GL, and stored in our array
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    GLuint newfontoffset = glGenLists(128);

    // since openGL's raster data is reversed from CS, we must
    // flip every character vertically.
    // make a temporary buffer to hold each character as we send it to
    // the openGL driver

    unsigned char *flipbuffer = new unsigned char[newfont.BytesPerChar];
    unsigned char *basebuffer = NULL;
    int wordsize = (newfont.Width+7)/8;
    int WordsPerChar = newfont.BytesPerChar/wordsize;

    // new shove all the raster data at openGL...
    int charwidth = newfont.Width;
    for (int characterindex=0; characterindex<128; characterindex++)
    {
    	glNewList(newfontoffset+characterindex,GL_COMPILE);

	// if the FontDef member IndividualWidth is non-NULL, we
	// need to extract the character width of each character
	// separately from that member, otherwise the width is
	// the same for all characters

	if (newfont.IndividualWidth != NULL)
	    charwidth = newfont.IndividualWidth[characterindex];

	// copy into the flip buffer -- see flipbuffer declaration for
	// the reason behind this code!

	basebuffer = newfont.FontBitmap + characterindex*newfont.BytesPerChar;
	for (int wordindex=0; wordindex < newfont.Height; wordindex++)
	{
	    for (int charindex=0; charindex < wordsize; charindex++)
	    	flipbuffer[wordindex*wordsize+charindex] =
			basebuffer[(newfont.Height-wordindex-1)*wordsize+charindex];
	}

	// we assume that stepping to the next character involves moving
	// 0 pixels in the y direction and moving 'charwidth' pixels to the
	// right
	glBitmap(charwidth, newfont.Height,  /* bitmap size */
		 0.0 , 0.0,      /* offset from bitmap origin */
		 charwidth, 0, 		     /* shift raster position by this */
		 flipbuffer
		 );
	glEndList();
    }

    Font_Offsets[Font_Count] = newfontoffset;

    // need to know there is another font stored here
    Font_Count++;

    delete[] flipbuffer;
}

void csGraphics2DOpenGLFontServer::AddFont(FontDef &addme)
{
    BuildFont(addme);
}

/** Print some characters (finally!)  This is basically a wrapper
 * around glListBase() and glCallList() invocations */
void csGraphics2DOpenGLFontServer::WriteCharacters(char *writeme, int fontnumber)
{
    // do some error checking
    if (Font_Count < 1) return;

    if (fontnumber >= Font_Count)
        fontnumber = 0;
    
    // save GL state, set a new list base
    glPushAttrib(GL_LIST_BIT);
    glListBase(Font_Offsets[fontnumber]);

    // write the string
    glCallLists(strlen(writeme),GL_UNSIGNED_BYTE,(GLubyte *)writeme);

    // get old list offset back
    glPopAttrib();
}

/** Print a character. This is basically a wrapper
 * around glListBase() and glCallList() invocations */
void csGraphics2DOpenGLFontServer::WriteCharacter(char writeme, int fontnumber)
{
    // do some error checking
    if (Font_Count < 1) return;

    if (fontnumber >= Font_Count)
        fontnumber = 0;

    //printf("font %d, char %d, list %d",
    //		fontnumber, writeme, Font_Offsets[fontnumber] + writeme);
 
    glCallList(Font_Offsets[fontnumber] + writeme);
}


