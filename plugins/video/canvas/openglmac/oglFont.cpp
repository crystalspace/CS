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
#include "cssysdef.h"
#include "video/canvas/common/graph2d.h"
#include "isys/isystem.h"
#include "oglFont.h"

/** The constructor initializes it member variables and constructs the
 * first font, if one was passed into the constructor */
csGraphics2DOpenGLFontServer::csGraphics2DOpenGLFontServer(int /*nFonts*/, iFontServer *pFS)
    : Font_Count(0), Font_Offsets(NULL), pFontServer(pFS)
{

}

csGraphics2DOpenGLFontServer::~csGraphics2DOpenGLFontServer()
{
    // kill all the fonts data we shoved into openGL...
    // (to be added)
}

void csGraphics2DOpenGLFontServer::BuildFont(int iFont)
{
    // we assume the FontDef is legal...

    // OK, update our member variables with new font information

    // need another spot in the array of offsets
    int height = pFontServer->GetMaximumHeight (iFont);
    if (height==-1) return;
    if (Font_Offsets!=NULL)
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
//    glGenLists(5);
    GLuint newfontoffset = glGenLists(256);

    // since openGL's raster data is reversed from CS, we must
    // flip every character vertically.
    // make a temporary buffer to hold each character as we send it to
    // the openGL driver


    // new shove all the raster data at openGL...
    unsigned char *flipbuffer = NULL;
    for (int characterindex=0; characterindex<256; characterindex++)
    {
      int charwidth = pFontServer->GetCharWidth (iFont, characterindex);
      int BytesPerRow = (charwidth+7)/8;
      int Rows = pFontServer->GetCharHeight (iFont, characterindex);
      flipbuffer = (unsigned char*)realloc (flipbuffer, BytesPerRow*Rows);
      unsigned char *basebuffer = pFontServer->GetCharBitmap (iFont, characterindex);
      
      glNewList(newfontoffset+characterindex,GL_COMPILE);

      // copy into the flip buffer -- see flipbuffer declaration for
      // the reason behind this code!

      for (int row=0; row < Rows; row++)
      {
        for (int col=0; col < BytesPerRow; col++)
          flipbuffer[row*BytesPerRow+col] = 255;
//			basebuffer[(Rows-row-1)*BytesPerRow+col];
      }
//printf("%d %d,%d\n", newfontoffset, charwidth, Rows);
	// we assume that stepping to the next character involves moving
	// 0 pixels in the y direction and moving 'charwidth' pixels to the
	// right
	glBitmap(charwidth, Rows,  /* bitmap size */
		 0.0 , 0.0 , 			     /* offset from bitmap origin */
		 charwidth, 0, 		     /* shift raster position by this */
		 flipbuffer
		 );
	glEndList();
    }

    Font_Offsets[Font_Count] = newfontoffset;

    // need to know there is another font stored here
    Font_Count++;

    free (flipbuffer);
}

void csGraphics2DOpenGLFontServer::AddFont(int iFont)
{
    BuildFont(iFont);
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
 
printf("%d\n", Font_Offsets[fontnumber] + writeme);
    glCallList(Font_Offsets[fontnumber] + writeme);
}
