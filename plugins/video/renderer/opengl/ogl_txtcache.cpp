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

#include "cssysdef.h"
#include "cssys/sysdriv.h"

#include <GL/gl.h>

#include "ogl_txtcache.h"
#include "ogl_txtmgr.h"
#include "ogl_g3dcom.h"
#include "isys/system.h"
#include "iengine/lightmap.h"
#include "iengine/polygon.h"
#include "ivideo/graph3d.h"
#include "isys/system.h"

// need definitions of R24(), G24(), and B24()
#ifndef NORMAL_LIGHT_LEVEL
#define NORMAL_LIGHT_LEVEL 128
#endif

//----------------------------------------------------------------------------//

/// Unload a texture cache element (common for both caches)
void OpenGLCache::Unload (csGLCacheData *d)
{
  if (d->next)
    d->next->prev = d->prev;
  else
    tail = d->prev;

  if (d->prev)
    d->prev->next = d->next;
  else
    head = d->next;

  glDeleteTextures (1, &d->Handle);
  d->Handle = 0;

  num--;
  total_size -= d->Size;

  if (type == CS_TEXTURE)
  {
    iTextureHandle *texh = (iTextureHandle *)d->Source;
    if (texh) texh->SetCacheData (NULL);
  }
  else if (type == CS_LIGHTMAP)
  {
    iLightMap *lm = (iLightMap *)d->Source;
    if (lm) lm->SetCacheData (NULL);
  }

  delete d;
}

#define Printf system->Printf
//----------------------------------------------------------------------------//

OpenGLCache::OpenGLCache (int max_size, csCacheType type, int bpp)
{
  OpenGLCache::type = type;
  OpenGLCache::bpp = bpp;
  cache_size = max_size;
  num = 0;
  head = tail = NULL;
  total_size = 0;
}

OpenGLCache::~OpenGLCache ()
{
  Clear ();
}

void OpenGLCache::cache_texture (iTextureHandle *txt_handle)
{
  if (type != CS_TEXTURE)
    return;

  csGLCacheData *cached_texture = (csGLCacheData *)txt_handle->GetCacheData ();
  if (cached_texture)
  {
    // move unit to front (MRU)
    if (cached_texture != head)
    {
      if (cached_texture->prev)
        cached_texture->prev->next = cached_texture->next;
      else
        head = cached_texture->next;
      if (cached_texture->next)
        cached_texture->next->prev = cached_texture->prev;
      else
        tail = cached_texture->prev;

      cached_texture->prev = NULL;
      cached_texture->next = head;
      if (head)
        head->prev = cached_texture;
      else
        tail = cached_texture;
      head = cached_texture;
    }
  }
  else
  {
    int size = 0;

    for (int c = 0; c < 4; c++)
    {
      int width, height;
      if (txt_handle->GetMipMapDimensions (c, width, height))
        size += width * height;
    }

    size *= bpp / 8;

    // unit is not in memory. load it into the cache
    while (total_size + size >= cache_size)
      // out of memory. remove units from bottom of list.
      Unload (tail);

    // now load the unit.
    num++;
    total_size += size;

    cached_texture = new csGLCacheData;

    cached_texture->next = head;
    cached_texture->prev = NULL;
    if (head)
      head->prev = cached_texture;
    else
      tail = cached_texture;
    head = cached_texture;
    cached_texture->Source = txt_handle;
    cached_texture->Size = size;

    txt_handle->SetCacheData (cached_texture);
    Load (cached_texture);              // load it.
  }
}

void OpenGLCache::cache_lightmap (iPolygonTexture *polytex)
{
  csGLCacheData *cached_texture;

  iLightMap *piLM = polytex->GetLightMap ();

  if (type != CS_LIGHTMAP || piLM == NULL)
    return;

  int width = piLM->GetWidth ();
  int height = piLM->GetHeight ();
  int size = width * height * (bpp / 8);

  // if lightmap has changed, uncache it if it was cached so we
  // can reload it
  cached_texture = (csGLCacheData *)piLM->GetCacheData ();
  if (polytex->RecalculateDynamicLights () && cached_texture)
  {
    Unload (cached_texture);
    cached_texture = NULL;
  }

  if (cached_texture)
  {
    // move unit to front (MRU)
    if (cached_texture != head)
    {
      if (cached_texture->prev)
        cached_texture->prev->next = cached_texture->next;
      else
        head = cached_texture->next;
      if (cached_texture->next)
        cached_texture->next->prev = cached_texture->prev;
      else
        tail = cached_texture->prev;

      cached_texture->prev = NULL;
      cached_texture->next = head;
      if (head)
        head->prev = cached_texture;
      else
        tail = cached_texture;
      head = cached_texture;
    }
  }
  else
  {
    // unit is not in memory. load it into the cache
    while (total_size + size >= cache_size)
      // out of memory. remove units from bottom of list.
      Unload (tail);

    // now load the unit.
    num++;
    total_size += size;

    cached_texture = new csGLCacheData;

    cached_texture->next = head;
    cached_texture->prev = NULL;
    if (head)
      head->prev = cached_texture;
    else
      tail = cached_texture;
    head = cached_texture;
    cached_texture->Source = piLM;
    cached_texture->Size = size;

    piLM->SetCacheData (cached_texture);
    Load (cached_texture);              // load it.

    // Precalculate the lm offsets relative to the original texture.
    int lmwidth = piLM->GetWidth ();
    int lmrealwidth = piLM->GetRealWidth ();
    int lmheight = piLM->GetHeight ();
    int lmrealheight = piLM->GetRealHeight ();
    float scale_u = (float) (lmrealwidth) / (float) (lmwidth);
    float scale_v = (float) (lmrealheight) / (float) (lmheight);

    float lightmap_low_u, lightmap_low_v, lightmap_high_u, lightmap_high_v;
    polytex->GetTextureBox (lightmap_low_u, lightmap_low_v,
                        lightmap_high_u, lightmap_high_v);

    // lightmap fudge factor
    lightmap_low_u -= 0.125;
    lightmap_low_v -= 0.125;
    lightmap_high_u += 0.125;
    lightmap_high_v += 0.125;

    if (lightmap_high_u <= lightmap_low_u)
      cached_texture->lm_scale_u = scale_u;       // @@@ Is this right?
    else
      cached_texture->lm_scale_u = scale_u / (lightmap_high_u - lightmap_low_u);

    if (lightmap_high_v <= lightmap_low_v)
      cached_texture->lm_scale_v = scale_v;       // @@@ Is this right?
    else
      cached_texture->lm_scale_v = scale_v / (lightmap_high_v - lightmap_low_v);

    cached_texture->lm_offset_u = lightmap_low_u;
    cached_texture->lm_offset_v = lightmap_low_v;
  }
}

void OpenGLCache::Clear ()
{
  while (head)
    Unload (head);

  CS_ASSERT (!head);
  CS_ASSERT (!tail);
  CS_ASSERT (!total_size);
  CS_ASSERT (!num);
}

//----------------------------------------------------------------------------//

OpenGLTextureCache::OpenGLTextureCache(int size, int bitdepth)
  : OpenGLCache (size, CS_TEXTURE, bitdepth)
{
}

OpenGLTextureCache::~OpenGLTextureCache ()
{
  Clear ();
}

void OpenGLTextureCache::Uncache (iTextureHandle *texh)
{
  csGLCacheData *cached_texture = (csGLCacheData *)texh->GetCacheData ();
  if (cached_texture)
    Unload (cached_texture);
}

void OpenGLTextureCache::Load (csGLCacheData *d)
{
  iTextureHandle *txt_handle = (iTextureHandle *)d->Source;
  csTextureHandle *txt_mm = (csTextureHandle *)txt_handle->GetPrivateObject ();

  GLuint texturehandle;
  glGenTextures (1, &texturehandle);
  glBindTexture (GL_TEXTURE_2D, texturehandle);

  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
		   rstate_bilinearmap ? GL_LINEAR : GL_NEAREST);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
		   rstate_bilinearmap ? GL_LINEAR : GL_NEAREST);

  csRGBpixel *src;
  iImage *img = NULL;
  int tw, th;
  csRGBpixel *transp;

  if ((txt_mm->GetFlags () & (CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS)) 
      == CS_TEXTURE_3D)
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
      rstate_bilinearmap ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST);

  if ((txt_mm->GetFlags () & (CS_TEXTURE_NOMIPMAPS)) != CS_TEXTURE_NOMIPMAPS)
  {
    // generate mipmaps
    // bind all mipmap levels required.  In OpenGL this means the original 
    // mipmap (level 0) with further levels (1,2,3...) being a reduction of 
    // the original mipmap by a factor of 2 in both directions.  You must keep
    //  reducing the mipmap size until you get down to the 1x1 size mipmap.
    int mipmaplevel = 0;
    transp = txt_handle->GetKeyColor () ? txt_mm->get_transparent () : NULL;
    iImage *previmg = NULL;
    int twhack = 0, thhack = 0;
    while (true)
    {
      // the first X mipmap levels are generated by CS
      csTextureOpenGL *txt = (csTextureOpenGL *)txt_mm->get_texture (mipmaplevel);
      if (txt)
      {
	tw = txt->get_width ();
	th = txt->get_height ();
	src = txt->get_image_data ();
      }
      else
      {
	if (!previmg)
	  previmg = img = ((csTextureOpenGL *)txt_mm->get_texture
			   (mipmaplevel - 1))->get_image ()->MipMap (1, transp);
	else
	{
	  // standard CS imaging will not properly mipmap a 1xN or Nx1 bitmap, 
	  // so we have to make a hack to short-circuit such situations with 
	  // manual shrinkage
	  if ( (previmg->GetWidth() == 1) || (previmg->GetHeight() == 1) )
	  {
	    img=previmg;
	    src = (csRGBpixel *)previmg->GetImageData();
	    int totalnewpixels = ( previmg->GetWidth() * previmg->GetHeight() ) / 2;
	    for (int skippixel = 0; skippixel < totalnewpixels; skippixel ++)
	      src[skippixel] = src[skippixel*2];

	    // as part of the hack we have to manually track the mipmap size,
	    //  as the  'actual' image size is invalid
	    if ( (twhack == 0) || (thhack = 0) )

	    {
	      twhack = previmg->GetWidth();
	      thhack = previmg->GetHeight();
	    }
	    twhack /= 2;
	    thhack /= 2;
	    if (twhack < 1) twhack = 1;
	    if (thhack < 1) thhack = 1;
	  }
	  else
	  {
	    img = previmg->MipMap (1, transp);
	    previmg->DecRef ();
	    previmg = img;
	  }
	}
	tw = img->GetWidth ();
	th = img->GetHeight ();
	src = (csRGBpixel *)img->GetImageData ();
      }

      if (twhack || thhack)
      {
	tw = twhack;
	th = thhack;
      }

      if (transp && !(txt && txt->KeyColorSet ()))
      {
	int pixels = tw * th;
	csRGBpixel *_src = src;
	while (pixels--)
	{
	  // By default, every csRGBpixel initializes its alpha component to 255.
	  // Thus, we should just drop to zero alpha for transparent pixels, 
	  // if any
	  if (transp->eq (*_src))
	    _src->alpha = 0;
	  _src++;
	}
	if (txt) txt->KeyColorSet (true);
      }

      // now that the texture has been generated, send it to openGL
      glTexImage2D (GL_TEXTURE_2D, mipmaplevel, 4, tw, th,
		    0, GL_RGBA, GL_UNSIGNED_BYTE, src);

      // shrink down to the next mipmap level
      if ((tw <= 1) && (th <= 1)) break;
      mipmaplevel++;

      // if the mipmapping does not properly shrink bitmaps, it will never 
      // terminate. This defensive check will terminate anyways 
      // if (mipmaplevel > 20) break;
    }

    if (previmg) previmg->DecRef ();
  }
  else
  {
    // Non-mipmapped texture
    csTextureOpenGL *txt = (csTextureOpenGL *)txt_mm->get_texture (0);
    transp = txt_handle->GetKeyColor () ? txt_mm->get_transparent () : NULL;
    if (txt)
    {
      tw = txt->get_width ();
      th = txt->get_height ();
      src = txt->get_image_data ();
      if (transp && !txt->KeyColorSet ())
      {
	int pixels = tw * th;
	csRGBpixel *_src = src;
	while (pixels--)
	{
	  // By default, every csRGBpixel initializes its alpha component to 255.
	  // Thus, we should just drop to zero alpha for transparent pixels,
	  // if any
	  if (transp->eq (*_src))
	    _src->alpha = 0;
	  _src++;
	}
	txt->KeyColorSet (true);
      }
      glTexImage2D (GL_TEXTURE_2D, 0, 4, tw, th,
		    0, GL_RGBA, GL_UNSIGNED_BYTE, src);
    }
  }
  d->Handle = texturehandle;
}

//----------------------------------------------------------------------------//
OpenGLLightmapCache::OpenGLLightmapCache(int size, int bitdepth)
  : OpenGLCache(size,CS_LIGHTMAP,bitdepth)
{
}

OpenGLLightmapCache::~OpenGLLightmapCache ()
{
  Clear ();
}


///
void OpenGLLightmapCache::Load(csGLCacheData *d)
{
  iLightMap *lightmap_info = (iLightMap *)d->Source;

  int lmwidth = lightmap_info->GetWidth ();
  int lmheight = lightmap_info->GetHeight ();
  int lmrealwidth = lightmap_info->GetRealWidth ();
  int lmrealheight = lightmap_info->GetRealHeight ();

  unsigned char *map_data = lightmap_info->GetMapData ();
  unsigned char* lm_data = map_data;

  GLuint lightmaphandle;
  glGenTextures (1, &lightmaphandle);
  glBindTexture (GL_TEXTURE_2D, lightmaphandle);

  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glTexImage2D (GL_TEXTURE_2D, 0, 3, lmwidth, lmheight, 0, GL_RGBA,
    GL_UNSIGNED_BYTE, lm_data);

  GLenum errtest;
  errtest = glGetError();
  if (errtest != GL_NO_ERROR)
  {
    //SysPrintf (MSG_DEBUG_0,"openGL error string: %s\n",
    	//gluErrorString(errtest) );
  }

/*
    CsPrintf(MSG_DEBUG_0,"caching lightmap in handle %d\n", lightmaphandle);
    CsPrintf(MSG_DEBUG_0,"size (%d,%d)\n",lmwidth,lmheight);
    CsPrintf(MSG_DEBUG_0,"lightmap data location %x\n",lightmap_info);
*/

  d->Handle = lightmaphandle;
}
