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
#include "csgeom/subrec.h"

#include "ogl_txtcache.h"
#include "ogl_txtmgr.h"
#include "ogl_g3dcom.h"
#include "imesh/thing/lightmap.h" //@@@!!!
#include "imesh/thing/polygon.h"  //@@@!!!
#include "ivideo/graph3d.h"
#include "ivaria/reporter.h"
#include "video/canvas/openglcommon/glstates.h"


// need definitions of R24(), G24(), and B24()
#ifndef CS_NORMAL_LIGHT_LEVEL
#define CS_NORMAL_LIGHT_LEVEL 128
#endif

//------------------------------------------------------------------------//
void csSLMCacheData::Alloc (csTrianglesPerSuperLightmap* s)
{
  source = s;
  source->cacheData = this;
  isUnlit = source->isUnlit;
}

void csSLMCacheData::Clear()
{
  source->cacheData = NULL;
}


//---------------------------------------------------------------------------//

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

//----------------------------------------------------------------------------//

OpenGLTextureCache::OpenGLTextureCache (int max_size,
  csGraphics3DOGLCommon* g3d)
{
  cache_size = max_size;
  num = 0;
  head = tail = NULL;
  total_size = 0;
  OpenGLTextureCache::g3d = g3d;
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
    csTextureHandleOpenGL *txt_mm = (csTextureHandleOpenGL *)
      txt_handle->GetPrivateObject ();

    // unit is not in memory. load it into the cache
    while (total_size + txt_mm->size >= cache_size)
      // out of memory. remove units from bottom of list.
      Unload (tail);

    // now load the unit.
    num++;
    total_size += txt_mm->size;

    cached_texture = new csTxtCacheData;

    cached_texture->next = head;
    cached_texture->prev = NULL;
    if (head)
      head->prev = cached_texture;
    else
      tail = cached_texture;
    head = cached_texture;
    cached_texture->Source = txt_handle;
    cached_texture->Size = txt_mm->size;

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
  csTextureHandleOpenGL *txt_mm = (csTextureHandleOpenGL *)
    txt_handle->GetPrivateObject ();

  if (reload)
  {
    csGraphics3DOGLCommon::statecache->SetTexture (GL_TEXTURE_2D, d->Handle);
  }
  else
  {
    GLuint texturehandle;

    glGenTextures (1, &texturehandle);
    d->Handle = texturehandle;
    csGraphics3DOGLCommon::statecache->SetTexture (
      GL_TEXTURE_2D, texturehandle);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  }

  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
    rstate_bilinearmap ? GL_LINEAR : GL_NEAREST);
  if (((txt_mm->GetFlags () & (CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS
    | CS_TEXTURE_PROC)) == CS_TEXTURE_3D))
  {
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
      rstate_bilinearmap ? GL_LINEAR_MIPMAP_LINEAR
                         : GL_NEAREST_MIPMAP_NEAREST);
  }
  else if (((txt_mm->GetFlags () & (CS_TEXTURE_PROC | CS_TEXTURE_NOMIPMAPS) )
    == CS_TEXTURE_PROC) && ( g3d->SGIS_generate_mipmap ) )
  {
    glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE );
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
      rstate_bilinearmap ? GL_LINEAR_MIPMAP_LINEAR
                         : GL_NEAREST_MIPMAP_NEAREST);
  }
  else
  {
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
      rstate_bilinearmap ? GL_LINEAR : GL_NEAREST);
  }

  for (int i=0; i < txt_mm->vTex.Length (); i++)
  {
    csTextureOpenGL *togl = txt_mm->vTex[i];
    if (togl->compressed == GL_FALSE)
      glTexImage2D (GL_TEXTURE_2D, i, txt_mm->TargetFormat (),
        togl->get_width (), togl->get_height (),
  0, txt_mm->SourceFormat (), txt_mm->SourceType (), togl->image_data);
    else
      csGraphics3DOGLCommon::glCompressedTexImage2DARB (
        GL_TEXTURE_2D, i, (GLenum)togl->internalFormat,
  togl->get_width (), togl->get_height (), 0,
  togl->size, togl->image_data);
  }
}

//----------------------------------------------------------------------------//

csSuperLightMap::csSuperLightMap ()
{
  cacheData = NULL;
  Handle = 0;
}

csSuperLightMap::~csSuperLightMap ()
{
  Clear ();
  glDeleteTextures (1, &Handle);
}

void csSuperLightMap::Alloc (csTrianglesPerSuperLightmap* s)
{
  cacheData->Alloc (s);
}

void csSuperLightMap::Clear ()
{
  if (cacheData)
  {
    cacheData->Clear ();
    cacheData = NULL;
  }
}

//----------------------------------------------------------------------------//

int OpenGLLightmapCache::super_lm_num = DEFAULT_SUPER_LM_NUM;
int OpenGLLightmapCache::super_lm_size = DEFAULT_SUPER_LM_SIZE;

OpenGLLightmapCache::OpenGLLightmapCache (csGraphics3DOGLCommon* g3d)
{
  suplm = new csSuperLightMap [super_lm_num];
  cur_lm = 0;
  initialized = false;
  OpenGLLightmapCache::g3d = g3d;
}

OpenGLLightmapCache::~OpenGLLightmapCache ()
{
  Clear ();
  delete[] suplm;

  // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
  // Delete the temporary handle
  glDeleteTextures (1, &TempHandle);
  // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
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
    csGraphics3DOGLCommon::statecache->SetTexture (
      GL_TEXTURE_2D, lightmaphandle);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // Normally OpenGL specs say that the last parameter to glTexImage2D
    // can be a NULL pointer. Unfortunatelly not all drivers seem to
    // support that. So I give a dummy texture here.
    char* buf = new char [super_lm_size*super_lm_size*4];
    memset (buf, 0, 4*super_lm_size*super_lm_size);
    glTexImage2D (GL_TEXTURE_2D, 0, 3, super_lm_size, super_lm_size,
        0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
    delete[] buf;
  }

  // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
  // Initialize the temporary handle
  glGenTextures (1, &TempHandle);
  csGraphics3DOGLCommon::statecache->SetTexture (
      GL_TEXTURE_2D, TempHandle);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  // Normally OpenGL specs say that the last parameter to glTexImage2D
  // can be a NULL pointer. Unfortunatelly not all drivers seem to
  // support that. So I give a dummy texture here.
  char* buf = new char [256*256*4];
  memset (buf, 0, 4*256*256);
  glTexImage2D (GL_TEXTURE_2D, 0, 3, 256, 256,
      0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
  delete[] buf;
  // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
}

void OpenGLLightmapCache::Clear ()
{
  cur_lm = 0;
  int i;
  for (i = 0 ; i < super_lm_num ; i++)
  {
    suplm[i].Clear ();
  }
}

int OpenGLLightmapCache::FindFreeSuperLightmap()
{
  int i;

  for(i = 0; i < super_lm_num; i++)
    if(suplm[i].cacheData == NULL) return i;
  return -1;
}

/*
 * Caches a whole precalculated superlighmap
 * s stores:
 * Distribution of the lightmaps in the superlightmap
 * Triangles
 * uv's for each triangles' vertex
 * Basically we have to blit all the lightmaps in a free super
 * lightmap
 */
void OpenGLLightmapCache::Cache (csTrianglesPerSuperLightmap* s, bool dirty,
  bool* modified)
{
  Setup ();
  modified = false;
  //First: Try to find a free superlightmap
  //Check if the superLightmap is already in the cache

  csRect* rectangleArray = s->rectangles.GetArray();
  const csRefArray<iPolygonTexture>& lmArray = s->lightmaps;
  //iPolygonTexture** lmArray = s->lightmaps.GetArray();

  int i;
  int numLightmaps = s->lightmaps.Length ();
  if (s->cacheData)
  {
    //The data is already in cache, let's see
    // if we need to recalculate the lightmaps
    // due the effect of dynamic lights

    if (dirty || !s->initialized)
    {
      GLuint SLMHandle = s->cacheData->Handle;
      for (i = 0; i < numLightmaps; i++)
      {
        if (lmArray[i]->RecalculateDynamicLights ())
        {
          iLightMap* lm = lmArray[i]->GetLightMap();
          int lmwidth = lm->GetWidth();
          int lmheight = lm->GetHeight();
          csRGBpixel* lm_data = lm->GetMapData();
          csRect r = rectangleArray[i];
          csGraphics3DOGLCommon::statecache->SetTexture (GL_TEXTURE_2D,
	  	SLMHandle);
          glTexSubImage2D (GL_TEXTURE_2D, 0, r.xmin, r.ymin,
            lmwidth, lmheight, GL_RGBA, GL_UNSIGNED_BYTE, lm_data);
        }
      }
      s->initialized = true;
    }

    return;
  }

  // The superlightmap isn't in the cache, so we have to cache it.
  int index = FindFreeSuperLightmap ();
  if (index < 0)
  {
    // Clear one lightmap
    // Temporaly i will clear the following lightmap (it would be nice to
    // implement a LRU algorithm here)
    cur_lm = (cur_lm + 1) % super_lm_num;
    suplm[cur_lm].Clear ();
    index = cur_lm;
  }
  s->initialized = false;
  //Fill the superLightmap
  suplm[index].cacheData = new csSLMCacheData ();
  //We're going to fill the whole super lightmap, so we don't give
  //width and height
  suplm[index].Alloc (s);
  csSLMCacheData* superLMData = (csSLMCacheData*) suplm[index].cacheData;
  GLuint SLMHandle;
  superLMData->Handle = SLMHandle = suplm[index].Handle;
  s->slId = index;
  for (i = 0; i < numLightmaps; i++)
  {
    lmArray[i]->RecalculateDynamicLights();
    iLightMap* lm = lmArray[i]->GetLightMap();
    int lmwidth = lm->GetWidth ();
    int lmheigth = lm->GetHeight ();
    csRGBpixel* lm_data = lm->GetMapData ();
    csRect r = rectangleArray[i];
    csGraphics3DOGLCommon::statecache->SetTexture (GL_TEXTURE_2D, SLMHandle);
    glTexSubImage2D (GL_TEXTURE_2D, 0, r.xmin, r.ymin,
      lmwidth, lmheigth, GL_RGBA, GL_UNSIGNED_BYTE, lm_data);
  }
  s->initialized = true;
}

bool OpenGLLightmapCache::IsLightmapOK (iPolygonTexture *polytex)
{
  return (polytex->GetLightMap () &&
    (polytex->GetLightMap ()->GetWidth () <= super_lm_size) &&
    (polytex->GetLightMap ()->GetHeight () <= super_lm_size));
}

