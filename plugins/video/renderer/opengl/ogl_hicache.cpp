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
#include "cssys/system.h"
#include "cs3d/opengl/ogl_hicache.h"
#include "cs3d/opengl/ogl_txtmgr.h"
#include "ilghtmap.h"
#include "ipolygon.h"
#include "itexture.h"

HighColorCache::HighColorCache (int max_size, HIGHCOLOR_TYPE type, int bpp)
{
  HighColorCache::type = type;
  cache_size = max_size;
  num = 0;
  total_size = 0;
  head = tail = NULL;
  HighColorCache::bpp = bpp;
}

HighColorCache::~HighColorCache ()
{
}

void HighColorCache::Add (iTextureHandle *txt_handle)
{
  csHighColorCacheData *cached_texture;
  int size = 0;

  if (type != HIGHCOLOR_TEXCACHE)
    return;

  for (int c = 0; c < 4; c++)
  {
    int width, height;

    txt_handle->GetMipMapDimensions (c, width, height);
    size += width * height;
  }

  size *= bpp / 8;

  if (txt_handle->IsCached ())
  {
    // move unit to front (MRU)
    cached_texture = txt_handle->GetHighColorCacheData ();
    if (!cached_texture)
      return;

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
    {
      // out of memory. remove units from bottom of list.
      csHighColorCacheData *l = tail;
      iTextureHandle *piMMC = QUERY_INTERFACE (l->pSource, iTextureHandle);

      tail = tail->prev;
      if (tail)
        tail->next = NULL;
      else
        head = NULL;
      l->prev = NULL;

      piMMC->SetInCache (false);
      piMMC->SetHighColorCacheData (NULL);

      Unload (l);                       // unload it.

      num--;
      total_size -= l->lSize;

      delete l;

      piMMC->DecRef ();
    }

    // now load the unit.
    num++;
    total_size += size;

    CHK (cached_texture = new csHighColorCacheData);

    cached_texture->next = head;
    cached_texture->prev = NULL;
    if (head)
      head->prev = cached_texture;
    else
      tail = cached_texture;
    head = cached_texture;
    cached_texture->pSource = txt_handle;
    cached_texture->lSize = size;

    txt_handle->SetInCache (true);
    txt_handle->SetHighColorCacheData (cached_texture);
    Load (cached_texture);              // load it.
  }
}

void HighColorCache::Add (iPolygonTexture *polytex)
{
  csHighColorCacheData *cached_texture;
  csHighColorCacheData *l = NULL;

  iLightMap *piLM = polytex->GetLightMap ();

  if (type != HIGHCOLOR_LITCACHE || piLM == NULL)
    return;

  int width = piLM->GetWidth ();
  int height = piLM->GetHeight ();
  int size = width * height * (bpp / 8);

  // if lightmap has changed, uncache it if it was cached so we
  // can reload it
  if (polytex->RecalculateDynamicLights () && piLM->IsCached ())
  {
    l = piLM->GetHighColorCache ();

    if (l->next != NULL)
      l->next->prev = l->prev;
    if (l->prev != NULL)
      l->prev->next = l->next;
    piLM->SetInCache (false);
    piLM->SetHighColorCache (NULL);

    Unload (l);                         // unload it.

    num--;
    total_size -= l->lSize;

    delete l;
  }

  if (piLM->IsCached ())
  {
    // move unit to front (MRU)

    cached_texture = piLM->GetHighColorCache ();
    // ASSERT(cached_texture);

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
    {
      iLightMap *ilm = QUERY_INTERFACE (l->pSource, iLightMap);

      // out of memory. remove units from bottom of list.
      l = tail;
      tail = tail->prev;
      if (tail)
        tail->next = NULL;
      else
        head = NULL;
      l->prev = NULL;
      ilm->SetInCache (false);
      ilm->SetHighColorCache (NULL);

      Unload (l);                       // unload it.

      num--;
      total_size -= l->lSize;

      delete l;

      ilm->DecRef ();
    }

    // now load the unit.
    num++;
    total_size += size;

    CHK (cached_texture = new csHighColorCacheData);

    cached_texture->next = head;
    cached_texture->prev = NULL;
    if (head)
      head->prev = cached_texture;
    else
      tail = cached_texture;
    head = cached_texture;
    cached_texture->pSource = piLM;
    cached_texture->lSize = size;

    piLM->SetInCache (true);
    piLM->SetHighColorCache (cached_texture);
    Load (cached_texture);              // load it.
  }
}

void HighColorCache::Clear ()
{
  while (head)
  {
    csHighColorCacheData *n = head->next;

    head->next = head->prev = NULL;

    Unload (head);

    if (type == HIGHCOLOR_TEXCACHE)
    {
      iTextureHandle *piMMC = QUERY_INTERFACE (head->pSource, iTextureHandle);

      if (piMMC)
      {
        piMMC->SetInCache (false);
        piMMC->SetHighColorCacheData (NULL);

        piMMC->DecRef ();
      }
    }
    else if (type == HIGHCOLOR_LITCACHE)
    {
      iLightMap *piLM = QUERY_INTERFACE (head->pSource, iLightMap);

      if (piLM)
      {
        piLM->SetHighColorCache (NULL);
        piLM->SetInCache (false);

        piLM->DecRef ();
      }
    }
    head = n;
  }

  head = tail = NULL;
  total_size = 0;
  num = 0;
}
