/*
    Copyright (C) 1998 by Jorrit Tyberghein and Dan Ogles.

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

#include "sysdef.h"

#if defined(OS_WIN32)
#include <windows.h>
#endif

#include <GL/gl.h>
//#include <GL/glu.h>

#include "cs3d/opengl/ogl_txtcache.h"
#include "cs3d/opengl/ogl_txtmgr.h"
#include "cs3d/opengl/ogl_hicache.h"
#include "cs3d/opengl/ogl_g3d.h"
#include "isystem.h"
#include "ilghtmap.h"
#include "igraph3d.h"

// need definitions of R24(), G24(), and B24()
#ifndef NORMAL_LIGHT_LEVEL
#define NORMAL_LIGHT_LEVEL 128
#endif


// these are the number of mipmap levels generated by CS
static int const AUTO_MIPMAP_LEVELS = 3;

OpenGLTextureCache::OpenGLTextureCache(int size, int bitdepth)
  : HighColorCache(size,HIGHCOLOR_TEXCACHE,bitdepth)
{
}

OpenGLTextureCache::~OpenGLTextureCache ()
{
  Clear ();
}

void OpenGLTextureCache::Dump()
{
}


void OpenGLTextureCache::Load (csHighColorCacheData *d)
{
  iTextureHandle* txt_handle = (iTextureHandle*)d->pSource;
  csTextureMM* txt_mm = (csTextureMM*)txt_handle->GetPrivateObject ();
  bool const transparent = txt_handle->GetTransparent ();

  CHK (GLuint *texturehandle = new GLuint);

  // bind all 4 mipmap levels if possible
  glGenTextures (1,texturehandle);

  glBindTexture (GL_TEXTURE_2D, *texturehandle);

  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, rstate_bilinearmap ? GL_LINEAR : GL_NEAREST);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, rstate_bilinearmap ? GL_LINEAR : GL_NEAREST);
  if (!txt_mm->for_2d())
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, rstate_bilinearmap ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST);

  csTexture* txt_unl = txt_mm->get_texture (0);
  int texture_width = txt_unl->get_width ();
  int texture_height = txt_unl->get_height ();

  CHK (unsigned char * const tempdata = new unsigned char[texture_width*texture_height*4]);

  // generate mipmaps
  int mipmaplevel=0;
  while ( mipmaplevel < 12 )
  {
    // the first X mipmap levels are generated by CS
    if (mipmaplevel < AUTO_MIPMAP_LEVELS)
    {
      txt_unl = txt_mm->get_texture (mipmaplevel);
      ULong const *source = (ULong *)txt_unl->get_bitmap ();

      unsigned char * dest = tempdata;
      for (int count = texture_width * texture_height; count > 0; count--)
      {
        dest[0] = R24(*source);
        dest[1] = G24(*source);
        dest[2] = B24(*source);
	
        if (transparent && (*source == 0))
          dest[3] = 0;
        else
          dest[3] = 255;
	
        dest+=4; source++;
      }
    }
    else // manually generate the last mipmap levels by scaling down
    	 // the previous mipmap by 2
    {
      ULong *source = (ULong *)tempdata;
      ULong *dest = (ULong *)tempdata;
      for (int newy=0; newy < texture_height; newy++)
      {
        for (int newx=0; newx < texture_width; newx++)
        {
          *dest++ = *source;
	  source += 2;
        }
	source += texture_height*2;
      }
    }

    // now that the texture has been generated, send it to openGL
    if (transparent)
        glTexImage2D(GL_TEXTURE_2D,mipmaplevel,GL_RGBA,texture_width,
		texture_height,0,GL_RGBA,GL_UNSIGNED_BYTE,
		tempdata);
    else
        glTexImage2D(GL_TEXTURE_2D,mipmaplevel,GL_RGB,texture_width,
		texture_height,0,GL_RGBA,GL_UNSIGNED_BYTE,
		tempdata);

    // only 0 level mipmip for a 2d texture!
    if (txt_mm->for_2d()) break;

    // bugger out if we have just made the smallest mipmap needed
    if ( (texture_height == 1) &&
         (texture_width  == 1) )
    	break;

    // shrink down to the next mipmap level
    if (texture_height > 1) texture_height /= 2;
    if (texture_width > 1) texture_width /= 2;
    mipmaplevel++;
  }

  delete [] tempdata;

  d->pData = texturehandle;
}

///
void OpenGLTextureCache::Unload (csHighColorCacheData *d)
{
   if (d && d->pData)
   {
     glDeleteTextures(1,(GLuint *)d->pData);
     delete (GLuint *)d->pData;
   }
}

OpenGLLightmapCache::OpenGLLightmapCache(int size, int bitdepth)
  : HighColorCache(size,HIGHCOLOR_LITCACHE,bitdepth)
{
}

OpenGLLightmapCache::~OpenGLLightmapCache ()
{
  Clear ();
}


void OpenGLLightmapCache::Dump()
{
}

///
void OpenGLLightmapCache::Load(csHighColorCacheData *d)
{
  iLightMap *lightmap_info = (iLightMap *)d->pSource;

  int lmwidth = lightmap_info->GetWidth ();
  int lmheight = lightmap_info->GetHeight ();
  int lmrealwidth = lightmap_info->GetRealWidth ();
  int lmrealheight = lightmap_info->GetRealHeight ();

//lmheight = floor(pow(2.0, ceil(log(lmheight)/log(2.0)) ) );
//lmwidth = floor(pow(2.0, ceil(log(lmwidth)/log(2.0)) ) );

  unsigned char *red_data = lightmap_info->GetMap (0);
  unsigned char *green_data = lightmap_info->GetMap (1);
  unsigned char *blue_data = lightmap_info->GetMap (2);

  // @@@ Note @@@
  // The lightmap data used by Crystal Space is in another format
  // then the lightmap data needed by OpenGL. Maybe we should consider
  // switching Crystal Space to this format as it is more standard?
  // For the software renderer this would be almost no problem and
  // it would make the hardware renderers faster as the following
  // conversion would not be needed anymore.
  CHK (unsigned char *lm_data = new unsigned char[lmwidth*lmheight*4]);
  unsigned char *walkdata = lm_data;
  unsigned char *last_data = walkdata+(lmwidth*lmheight*4);
  while (walkdata < last_data)
  {
    *walkdata++ = *red_data++;
    *walkdata++ = *green_data++;
    *walkdata++ = *blue_data++;
    *walkdata++ = NORMAL_LIGHT_LEVEL;
  }

  // Because our actual lightmap is a smaller lightmap in a larger
  // po2 texture we can have problems when using GL_LINEAR for
  // the GL_TEXTURE_MAG_FILTER and GL_TEXTURE_MIN_FILTER.
  // We still have to copy the last row and column of the lightmap
  // to just outside the original lightmap boundaries.
  // Otherwise we get a random colored border. Because we use
  // GL_REPEAT the problem is even worse. In that case we also have to
  // copy the first row and column to the last row and column in
  // the po2 lightmap texture.
  int i;
  if (lmwidth != lmrealwidth)
  {
    // Point to last real column.
    unsigned char* lastrealcol = lm_data+(lmrealwidth-1)*4;
    // Copy to next column.
    for (i = 0 ; i < lmheight ; i++)
    {
      lastrealcol[4] = lastrealcol[0];
      lastrealcol[5] = lastrealcol[1];
      lastrealcol[6] = lastrealcol[2];
      lastrealcol += lmwidth*4;
    }

    // Point to last physical column.
    unsigned char* lastcol = lm_data+(lmwidth-1)*4;
    unsigned char* firstcol = lm_data;
    // Copy first column to last.
    for (i = 0 ; i < lmheight ; i++)
    {
      lastcol[0] = firstcol[0];
      lastcol[1] = firstcol[1];
      lastcol[2] = firstcol[2];
      lastcol += lmwidth*4;
      firstcol += lmwidth*4;
    }
  }
  if (lmheight != lmrealheight)
  {
    // Point to last real row.
    unsigned char* lastrealrow = lm_data+lmwidth*(lmrealheight-1)*4;
    // Copy to next row.
    for (i = 0 ; i < lmwidth*4 ; i++) lastrealrow[i+lmwidth*4] = lastrealrow[i];

    // Point to last physical row.
    unsigned char* lastrow = lm_data+lmwidth*(lmheight-1)*4;
    unsigned char* firstrow = lm_data;
    // Copy first row to last.
    for (i = 0 ; i < lmwidth*4 ; i++) lastrow[i] = firstrow[i];
  }

  CHK (GLuint *lightmaphandle = new GLuint);
  glGenTextures(1,lightmaphandle);
  glBindTexture(GL_TEXTURE_2D,*lightmaphandle);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,lmwidth,
		lmheight,0,GL_RGBA,GL_UNSIGNED_BYTE,
		lm_data);

  GLenum errtest;
  errtest = glGetError();
  if (errtest != GL_NO_ERROR)
  {
    //SysPrintf (MSG_DEBUG_0,"openGL error string: %s\n",gluErrorString(errtest) );
  }

  delete []lm_data;

/*
    CsPrintf(MSG_DEBUG_0,"caching lightmap in handle %d\n",*lightmaphandle);
    CsPrintf(MSG_DEBUG_0,"size (%d,%d)\n",lmwidth,lmheight);
    CsPrintf(MSG_DEBUG_0,"lightmap data location %x\n",lightmap_info);
    */

  d->pData = lightmaphandle;
}

///
void OpenGLLightmapCache::Unload(csHighColorCacheData *d)
{
  if (d && d->pData)
  {
    glDeleteTextures(1,(GLuint *)d->pData);
    delete (GLuint *)d->pData;
  }
}
