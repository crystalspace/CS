/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
    Copyright (C) 1998 by Dan Ogles.

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
#include "csgeom/subrec.h"

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
void OpenGLTextureCache::Unload (csTxtCacheData *d)
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

  iTextureHandle *texh = (iTextureHandle *)d->Source;
  if (texh) texh->SetCacheData (NULL);

  delete d;
}

#define Printf system->Printf
//----------------------------------------------------------------------------//

OpenGLTextureCache::OpenGLTextureCache (int max_size, int bpp)
{
  OpenGLTextureCache::bpp = bpp;
  cache_size = max_size;
  num = 0;
  head = tail = NULL;
  total_size = 0;
}

OpenGLTextureCache::~OpenGLTextureCache ()
{
  Clear ();
}

void OpenGLTextureCache::Cache (iTextureHandle *txt_handle)
{
  csTxtCacheData *cached_texture = (csTxtCacheData *)
  	txt_handle->GetCacheData ();
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

    cached_texture = new csTxtCacheData;

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

void OpenGLTextureCache::Clear ()
{
  while (head)
    Unload (head);

  CS_ASSERT (!head);
  CS_ASSERT (!tail);
  CS_ASSERT (!total_size);
  CS_ASSERT (!num);
}

void OpenGLTextureCache::Uncache (iTextureHandle *texh)
{
  csTxtCacheData *cached_texture = (csTxtCacheData *)texh->GetCacheData ();
  if (cached_texture)
    Unload (cached_texture);
}

void OpenGLTextureCache::Load (csTxtCacheData *d, bool reload)
{
  iTextureHandle *txt_handle = (iTextureHandle *)d->Source;
  csTextureHandle *txt_mm = (csTextureHandle *)txt_handle->GetPrivateObject ();

  GLuint texturehandle;
  if (reload)
  {
    texturehandle = d->Handle;
    glBindTexture (GL_TEXTURE_2D, texturehandle);
  }
  else
  {
    glGenTextures (1, &texturehandle);
    d->Handle = texturehandle;
    glBindTexture (GL_TEXTURE_2D, texturehandle);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
		     rstate_bilinearmap ? GL_LINEAR : GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
		     rstate_bilinearmap ? GL_LINEAR : GL_NEAREST);
  }

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
      csTextureOpenGL *txt = (csTextureOpenGL *)
      	txt_mm->get_texture (mipmaplevel);
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
	  // Standard CS imaging will not properly mipmap a 1xN or Nx1 bitmap, 
	  // so we have to make a hack to short-circuit such situations with 
	  // manual shrinkage.
	  if ( (previmg->GetWidth() == 1) || (previmg->GetHeight() == 1) )
	  {
	    img = previmg;
	    src = (csRGBpixel *)previmg->GetImageData();
	    int totalnewpixels = (previmg->GetWidth() * previmg->GetHeight())/2;
	    for (int skippixel = 0; skippixel < totalnewpixels; skippixel ++)
	      src[skippixel] = src[skippixel*2];

	    // As part of the hack we have to manually track the mipmap size,
	    // as the  'actual' image size is invalid.
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
	  // By default, every csRGBpixel initializes its alpha component to
	  // 255. Thus, we should just drop to zero alpha for transparent
	  // pixels, if any.
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
	  // By default, every csRGBpixel initializes its alpha component to
	  // 255. Thus, we should just drop to zero alpha for transparent
	  // pixels, if any.
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
}

//----------------------------------------------------------------------------//

int csLightMapQueue::AddVertices (int num)
{
  num_vertices += num;
  if (num_vertices > max_vertices)
  {
    GLfloat* new_ar;
    int old_num = num_vertices-num;
    max_vertices += 40;

    new_ar = new GLfloat [max_vertices*4];
    if (glverts) memcpy (new_ar, glverts, sizeof (GLfloat)*4*old_num);
    delete[] glverts; glverts = new_ar;

    new_ar = new GLfloat [max_vertices*2];
    if (gltxt) memcpy (new_ar, gltxt, sizeof (GLfloat)*2*old_num);
    delete[] gltxt; gltxt = new_ar;
  }
  return num_vertices-num;
}

void csLightMapQueue::AddTriangle (int i1, int i2, int i3)
{
  int old_num = num_triangles;
  num_triangles++;
  if (num_triangles > max_triangles)
  {
    max_triangles += 20;
    int* new_ar;
    new_ar = new int [max_triangles*3];
    if (tris) memcpy (new_ar, tris, sizeof (int) * 3 * old_num);
    delete[] tris; tris = new_ar;
  }
  tris[old_num*3+0] = i1;
  tris[old_num*3+1] = i2;
  tris[old_num*3+2] = i3;
}

void csLightMapQueue::Flush (GLuint Handle)
{
  if (num_triangles <= 0 || num_vertices <= 0) return;
  glBindTexture (GL_TEXTURE_2D, Handle);
  glVertexPointer (4, GL_FLOAT, 0, glverts);
  glTexCoordPointer (2, GL_FLOAT, 0, gltxt);
  glDrawElements (GL_TRIANGLES, num_triangles*3, GL_UNSIGNED_INT, tris);
  Reset ();
}

//----------------------------------------------------------------------------//

csSuperLightMap::csSuperLightMap ()
{
  region = new csSubRectangles (
  	csRect (0, 0, OpenGLLightmapCache::super_lm_size,
	OpenGLLightmapCache::super_lm_size));
  head = tail = NULL;
}

csSuperLightMap::~csSuperLightMap ()
{
  Clear ();
  glDeleteTextures (1, &Handle);
  delete region;
}

csLMCacheData* csSuperLightMap::Alloc (int w, int h)
{
  csRect r;
  if (region->Alloc (w, h, r))
  {
    csLMCacheData* cdata = new csLMCacheData ();
    cdata->super_lm_rect = r;
    cdata->Handle = Handle;
    cdata->next = head;
    cdata->prev = NULL;
    if (head) head->prev = cdata;
    head = cdata;
    return cdata;
  }
  return NULL;
}

void csSuperLightMap::Clear ()
{
  region->Clear ();
  while (head)
  {
    iLightMap* lm = (iLightMap *)head->Source;
    if (lm) lm->SetCacheData (NULL);
    csLMCacheData* h = head->next;
    delete head;
    head = h;
  }
  tail = NULL;
  queue.Reset ();
}

//----------------------------------------------------------------------------//

int OpenGLLightmapCache::super_lm_num = DEFAULT_SUPER_LM_NUM;
int OpenGLLightmapCache::super_lm_size = DEFAULT_SUPER_LM_SIZE;

OpenGLLightmapCache::OpenGLLightmapCache (csGraphics3DOGLCommon* g3d)
{
  suplm = new csSuperLightMap [super_lm_num];
  cur_lm = 0;
  num_lm_processed = 0;
  initialized = false;
  OpenGLLightmapCache::g3d = g3d;
}

OpenGLLightmapCache::~OpenGLLightmapCache ()
{
  Clear ();
  delete[] suplm;
}

void OpenGLLightmapCache::Setup ()
{
  if (initialized) return;
  initialized = true;
  int i;
  for (i = 0 ; i < super_lm_num ; i++)
  {
    GLuint lightmaphandle;
    glGenTextures (1, &lightmaphandle);
    suplm[i].Handle = lightmaphandle;
    glBindTexture (GL_TEXTURE_2D, lightmaphandle);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // Normally OpenGL specs say that the last parameter to glTexImage2D
    // can be a NULL pointer. Unfortunatelly not all drivers seem to
    // support that. So I give a dummy texture here.
    char* buf = new char [super_lm_size*super_lm_size*4];
    glTexImage2D (GL_TEXTURE_2D, 0, 3, super_lm_size, super_lm_size,
		    0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
    delete[] buf;
  }
}

void OpenGLLightmapCache::Clear ()
{
  cur_lm = 0;
  num_lm_processed = 0;
  int i;
  for (i = 0 ; i < super_lm_num ; i++)
  {
    suplm[i].Clear ();
  }
}

void OpenGLLightmapCache::Cache (iPolygonTexture *polytex)
{
  Setup ();

  iLightMap* piLM = polytex->GetLightMap ();
  if (piLM == NULL) return;

  queue_zbuf_mode = g3d->z_buf_mode;

  // If lightmap has changed, we don't uncache it but instead
  // we leave it in place and just reload the new data.
  csLMCacheData* clm = (csLMCacheData *)piLM->GetCacheData ();
  if (polytex->RecalculateDynamicLights () && clm)
  {
    Load (clm);
  }
  else if (!clm)
  {
    int lmwidth = piLM->GetWidth ();
    int lmheight = piLM->GetHeight ();
    // First try to allocate in the current super lightmap.
    clm = suplm[cur_lm].Alloc (lmwidth, lmheight);
    if (clm)
    {
      clm->super_lm_idx = cur_lm;
    }
    else
    {
      // There is no room in the current one. So we first try if we
      // can allocate in a number of super-lightmaps (if there are some).
      int num_proc = num_lm_processed;
      if (num_proc > 2) num_proc = 2;
      int prev_lm = cur_lm;
      while (num_proc > 0)
      {
	prev_lm = (prev_lm+super_lm_num-1)%super_lm_num;
        clm = suplm[prev_lm].Alloc (lmwidth, lmheight);
	if (clm)
	{
	  clm->super_lm_idx = prev_lm;
	  num_proc = 0;	// Stop.
        }
	else num_proc--;
      }

      if (!clm)
      {
        // There was no room in the current super-lightmap and neither
	// was there room in the previous one. We allocate a new
	// super-lightmap here.
        cur_lm = (cur_lm+1)%super_lm_num;
	printf ("New cur_lm=%d\n", cur_lm); fflush (stdout);//@@@
        num_lm_processed++;

        // Make sure all lightmaps are rendered.
        // If this actually renders lightmap then we might have a problem.
        // The number of super-lightmaps should be big enough so that
        // this is never needed.
        Flush (cur_lm);
        // Then free all lightmaps previously in this super lightmap.
        suplm[cur_lm].Clear ();

	// Now allocate the first lightmap in this super-lightmap.
	// This can not fail.
	clm = suplm[cur_lm].Alloc (lmwidth, lmheight);
	clm->super_lm_idx = cur_lm;
      }
    }

    piLM->SetCacheData (clm);
    clm->Source = piLM;
    Load (clm);

    float lm_low_u, lm_low_v, lm_high_u, lm_high_v;
    polytex->GetTextureBox (lm_low_u, lm_low_v, lm_high_u, lm_high_v);

#if 0
    // @@@ Experiment to try to get the lightmap mapped correctly
    // on the texture.
    if (lm_high_u <= lm_low_u)
      clm->lm_scale_u = 1.;       // @@@ Is this right?
    else
      clm->lm_scale_u = 1. / (lm_high_u - lm_low_u);

    if (lm_high_v <= lm_low_v)
      clm->lm_scale_v = 1.;       // @@@ Is this right?
    else
      clm->lm_scale_v = 1. / (lm_high_v - lm_low_v);

    lm_low_u -= 0.5 / (float (lmwidth) * clm->lm_scale_u);
    lm_low_v -= 0.5 / (float (lmheight) * clm->lm_scale_v);
    //lm_low_u -= .06;
    //lm_low_v -= .06;
    //clm->lm_scale_u *= .96;
    //clm->lm_scale_v *= .96;

    // Calculate position in super texture.
    float dlm = 1. / float (super_lm_size);
    float sup_u = float (clm->super_lm_rect.xmin) * dlm;
    float sup_v = float (clm->super_lm_rect.ymin) * dlm;
    clm->lm_scale_u = clm->lm_scale_u * float (lmwidth-1) * dlm;
    clm->lm_scale_v = clm->lm_scale_v * float (lmheight-1) * dlm;
    clm->lm_offset_u = lm_low_u - sup_u / clm->lm_scale_u;
    clm->lm_offset_v = lm_low_v - sup_v / clm->lm_scale_v;
#else
    // lightmap fudge factor
    lm_low_u -= 0.125;
    lm_low_v -= 0.125;
    lm_high_u += 0.125;
    lm_high_v += 0.125;

    if (lm_high_u <= lm_low_u)
      clm->lm_scale_u = 1.;       // @@@ Is this right?
    else
      clm->lm_scale_u = 1. / (lm_high_u - lm_low_u);

    if (lm_high_v <= lm_low_v)
      clm->lm_scale_v = 1.;       // @@@ Is this right?
    else
      clm->lm_scale_v = 1. / (lm_high_v - lm_low_v);

    // Calculate position in super texture.
    float dlm = 1. / float (super_lm_size);
    float sup_u = float (clm->super_lm_rect.xmin) * dlm;
    float sup_v = float (clm->super_lm_rect.ymin) * dlm;
    clm->lm_scale_u = clm->lm_scale_u * float (lmwidth) * dlm;
    clm->lm_scale_v = clm->lm_scale_v * float (lmheight) * dlm;
    clm->lm_offset_u = lm_low_u - sup_u / clm->lm_scale_u;
    clm->lm_offset_v = lm_low_v - sup_v / clm->lm_scale_v;
#endif
  }
}

void OpenGLLightmapCache::Load (csLMCacheData *d)
{
  iLightMap* lightmap_info = d->Source;
  int lmwidth = lightmap_info->GetWidth ();
  int lmheight = lightmap_info->GetHeight ();
  unsigned char* lm_data = lightmap_info->GetMapData ();

  glBindTexture (GL_TEXTURE_2D, d->Handle);
  csRect& r = d->super_lm_rect;
  glTexSubImage2D (GL_TEXTURE_2D, 0, r.xmin, r.ymin,
  	lmwidth, lmheight, GL_RGBA, GL_UNSIGNED_BYTE, lm_data);
}

void OpenGLLightmapCache::FlushIfNeeded ()
{
  Setup ();
  if (!g3d->CompatibleZBufModes (g3d->z_buf_mode, queue_zbuf_mode))
    Flush ();
}

void OpenGLLightmapCache::Flush ()
{
  Setup ();

  int i;
  bool flush_needed = false;
  for (i = 0 ; i < super_lm_num ; i++)
  {
    csLightMapQueue& lm_queue = suplm[i].queue;
    if (lm_queue.num_triangles > 0 && lm_queue.num_vertices > 0)
    {
      flush_needed = true;
      break;
    }
  }
  if (!flush_needed) return;
  g3d->SetGLZBufferFlagsPass2 (queue_zbuf_mode, true);
  glEnable (GL_TEXTURE_2D);
  glEnable (GL_BLEND);
  glColor4f (1, 1, 1, 0);
  // The following blend function is configurable.
  glBlendFunc (g3d->m_config_options.m_lightmap_src_blend,
	       g3d->m_config_options.m_lightmap_dst_blend);
  glEnableClientState (GL_VERTEX_ARRAY);
  glEnableClientState (GL_TEXTURE_COORD_ARRAY);
  for (i = 0 ; i < super_lm_num ; i++)
  {
    suplm[i].queue.Flush (suplm[i].Handle);
  }
  glDisableClientState (GL_VERTEX_ARRAY);
  glDisableClientState (GL_TEXTURE_COORD_ARRAY);
}

void OpenGLLightmapCache::Flush (int sup_idx)
{
  csLightMapQueue& lm_queue = suplm[sup_idx].queue;
  if (lm_queue.num_triangles <= 0 || lm_queue.num_vertices <= 0) return;

//@@@ We might want to add code here to save the state of OpenGL.
//The problem is that this code can be called at unexpected places.
//Luckily this is rare. It can only happen if one frame needs lightmaps
//from ALL the super-lightmaps at once.
  g3d->SetGLZBufferFlagsPass2 (queue_zbuf_mode, true);
  glEnable (GL_TEXTURE_2D);
  glEnable (GL_BLEND);
  // The following blend function is configurable.
  glBlendFunc (g3d->m_config_options.m_lightmap_src_blend,
	       g3d->m_config_options.m_lightmap_dst_blend);
  glEnableClientState (GL_VERTEX_ARRAY);
  glEnableClientState (GL_TEXTURE_COORD_ARRAY);
  lm_queue.Flush (suplm[sup_idx].Handle);
  glDisableClientState (GL_VERTEX_ARRAY);
  glDisableClientState (GL_TEXTURE_COORD_ARRAY);
}

