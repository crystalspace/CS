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
#include "imesh/thing/lightmap.h"	//@@@!!!
#include "imesh/thing/polygon.h"	//@@@!!!
#include "ivideo/graph3d.h"
#include "ivaria/reporter.h"

// need definitions of R24(), G24(), and B24()
#ifndef CS_NORMAL_LIGHT_LEVEL
#define CS_NORMAL_LIGHT_LEVEL 128
#endif

//----------------------------------------------------------------------------//


csLMCacheDataQueue::csLMCacheDataQueue()
{
  head = NULL;
  tail = NULL;
}
void* csLMCacheDataQueue::Alloc(int w, int h, SourceData s, csSubRectangles* r, 
      GLuint Handle)
{

  csRect raux;
  if(r->Alloc(w,h,raux))
  { //it fits in this superlightmap

    //let's allocate it
    csLMCacheData * clm = new csLMCacheData();
    clm->Source = s.LMDataSource;
    clm->Handle = Handle;
    clm->super_lm_rect = raux;
    
    if(head)
    {
      head->prev = clm;
      clm->prev = NULL;
      clm->next = head;
    }
    else
    {
      tail = clm;
      clm->next = NULL;
      clm->prev = NULL;
    }
    head = clm;
    return clm;
  }
  return NULL;
}


void csLMCacheDataQueue::Clear()
{
  while (head)
  {
    iLightMap* lm = (iLightMap *)head->Source;
    if (lm) lm->SetCacheData (NULL);
    csLMCacheData* h = head->next;
    delete head;
    head = h;
  }
  tail = NULL;
}

//------------------------------------------------------------------------//
void* csSLMCacheData::Alloc(int /*w*/, int /*h*/, SourceData s, csSubRectangles* /*r*/,
                            GLuint /*Handle*/)
{
   source = s.superLMDataSource;
   source->cacheData = this;
   isUnlit = source->isUnlit;
   return s.superLMDataSource;
}

void csSLMCacheData::Clear()
{
  source->cacheData = NULL;
}



//_---------------------------------------------------------------------------//

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

OpenGLTextureCache::OpenGLTextureCache (int max_size, csGraphics3DOGLCommon* g3d)
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
    csTextureHandleOpenGL *txt_mm = (csTextureHandleOpenGL *)txt_handle->GetPrivateObject ();

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
  csTextureHandleOpenGL *txt_mm = (csTextureHandleOpenGL *)txt_handle->GetPrivateObject ();

  if (reload)
  {
    glBindTexture (GL_TEXTURE_2D, d->Handle);
  }
  else
  {
    GLuint texturehandle;

    glGenTextures (1, &texturehandle);
    d->Handle = texturehandle;
    glBindTexture (GL_TEXTURE_2D, texturehandle);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  }

  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
    rstate_bilinearmap ? GL_LINEAR : GL_NEAREST);
  if (((txt_mm->GetFlags () & (CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_PROC)) 
      == CS_TEXTURE_3D))
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

int csLightMapQueue::AddVertices (int num)
{
  num_vertices += num;
  if (num_vertices > max_vertices)
  {
    GLfloat* new_ar;
    int old_num = num_vertices-num;
    max_vertices = num_vertices + 40;

    new_ar = new GLfloat [max_vertices*4];
    if (glverts) memcpy (new_ar, glverts, sizeof (GLfloat)*4*old_num);
    delete[] glverts; glverts = new_ar;

    new_ar = new GLfloat [max_vertices*2];
    if (gltxt) memcpy (new_ar, gltxt, sizeof (GLfloat)*2*old_num);
    delete[] gltxt; gltxt = new_ar;

    new_ar = new GLfloat [max_vertices*2];
    if (gltxt) memcpy (new_ar, gltxtFog, sizeof (GLfloat)*2*old_num);
    delete[] gltxtFog; gltxtFog = new_ar;

    new_ar = new GLfloat [max_vertices*3];
    if (gltxt) memcpy (new_ar, glcolorsFog, sizeof (GLfloat)*3*old_num);
    delete[] glcolorsFog; glcolorsFog = new_ar;


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



void csLightMapQueue::AddTrianglesArray(csTriangle* indices, int numTriangles)
{
   int old_num = num_triangles;
   num_triangles += numTriangles;   
   if (num_triangles > max_triangles)
   {
     max_triangles = num_triangles + 50; //
     int* new_ar = new int [max_triangles*3];
     if (tris) memcpy(new_ar, tris, sizeof(int)*3*old_num);
     delete[] tris; tris = new_ar;
   }

   int* aux = tris + old_num*3;
   
   memcpy(aux,(int*)indices,sizeof(int)*3*numTriangles);
   //aux = (int*)indices;
  /*
   int i;
  for(i = 0; i < numTriangles; i++)
    AddTriangle(indices[i].a,indices[i].b,indices[i].c);
    */
   
}

void csLightMapQueue::AddTexelsArray(csVector2* uvs, int numUV)
{
  GLfloat* aux = gltxt + (num_vertices-numUV)*2;
  //aux = (float*)uvs;
  memcpy(aux,(float*)uvs,sizeof(GLfloat)*2*numUV);
}

void csLightMapQueue::AddVerticesArray(csVector4 * verts,int numVerts)
{
  GLfloat* aux = glverts + (num_vertices - numVerts)*4;
  //aux = (float*)verts;
  memcpy(aux,(float*)verts,sizeof(GLfloat)*4*numVerts);
}

void csLightMapQueue::AddTrianglesArrayFast(csTriangle* indices, int numTriangles)
{
  tris = (int*)indices;
  num_triangles = numTriangles;

}

void csLightMapQueue::AddVerticesArrayFast(csVector4* verts, int numVerts)
{
  glverts = (GLfloat*)verts;
  num_vertices = numVerts;

}

void csLightMapQueue::AddTexelsArrayFast(csVector2* uvs)
{
  gltxt = (GLfloat*)uvs;
}



void csLightMapQueue::AddFogInfoArray(csColor* fogColors, int numColors)
{
  GLfloat* aux = glcolorsFog + (num_vertices-numColors)*3;
  memcpy(aux,(float*)fogColors,sizeof(GLfloat)*3*numColors);

}

void csLightMapQueue::AddFogTexelsArray(csVector2* fogTexels, int numUV)
{
  GLfloat* aux = gltxtFog + (num_vertices-numUV)*2;
  memcpy(aux,(float*)fogTexels,sizeof(GLfloat)*2*numUV);

}


void csLightMapQueue::AddFogInfoFast(csColor* work_verts)
{
  glcolorsFog = (GLfloat*)work_verts; 
}

void csLightMapQueue::AddFogTexelsFast(csVector2* work_fog_texels)
{
  gltxtFog = (GLfloat*)work_fog_texels;
}




void csLightMapQueue::Flush (GLuint Handle)
{
  if (num_triangles <= 0 || num_vertices <= 0) return;
  glBindTexture (GL_TEXTURE_2D, Handle);
  glVertexPointer (4, GL_FLOAT, 0, glverts);
  glTexCoordPointer (2, GL_FLOAT, 0, gltxt);
  glDrawElements (GL_TRIANGLES, num_triangles*3, GL_UNSIGNED_INT, tris); 
}

void csLightMapQueue::FlushFog (GLuint HandleFog)
{  
  if (num_triangles <= 0 || num_vertices <= 0) return;
  
  glBindTexture(GL_TEXTURE_2D, HandleFog);
  glVertexPointer (4, GL_FLOAT, 0, glverts);
  glTexCoordPointer(2,GL_FLOAT,0,gltxtFog);
  glColorPointer (3, GL_FLOAT, 0, glcolorsFog);
  glDrawElements (GL_TRIANGLES, num_triangles*3, GL_UNSIGNED_INT, tris);
  Reset();
}

void csLightMapQueue::Reset()
{
  if(!ownsData)
  {
    gltxt = NULL;
    glverts = NULL;
    tris = NULL;
    gltxtFog = NULL;
    glcolorsFog = NULL;
    max_triangles = 0;
    max_vertices = 0;
  }
  num_triangles = 0;
  num_vertices = 0;
}

void csLightMapQueue::SaveArrays()
{
  gltxtCached = gltxt;
  glvertsCached = glverts;
  max_trianglesCached = max_triangles;
  max_verticesCached = max_vertices;
  num_trianglesCached = num_triangles;
  num_verticesCached = num_vertices;
  trisCached = tris;
  glcolorsFogCached = glcolorsFog;
  gltxtFogCached = gltxtFog;
  max_fog_colorsCached = max_fog_colors;
  num_fog_colorsCached = num_fog_colors;
}

void csLightMapQueue::LoadArrays()
{
  gltxt = gltxtCached;
  glverts = glvertsCached;
  gltxtFog = gltxtFogCached;
  glcolorsFog = glcolorsFogCached;
  max_triangles = max_trianglesCached;
  max_vertices = max_verticesCached;
  max_fog_colors = max_fog_colorsCached;
  //num_triangles = num_trianglesCached;
  //num_vertices = num_verticesCached;
  tris = trisCached;
  gltxtCached = glvertsCached = NULL;
  trisCached = NULL;
  gltxtFogCached = NULL;
  glcolorsFogCached = NULL;



}


//----------------------------------------------------------------------------//

csSuperLightMap::csSuperLightMap ()
{
  region = new csSubRectangles (
  	csRect (0, 0, OpenGLLightmapCache::super_lm_size,
	OpenGLLightmapCache::super_lm_size));
  cacheData = NULL;
}

csSuperLightMap::~csSuperLightMap ()
{
  Clear ();
  glDeleteTextures (1, &Handle);
  delete region;
}

csLMCacheData* csSuperLightMap::Alloc (int w, int h, SourceData s)
{
  
  return (csLMCacheData*)cacheData->Alloc(w, h, s, region, Handle);
}

void csSuperLightMap::Clear ()
{
  region->Clear ();
  if(cacheData)
  {
    cacheData->Clear();
    cacheData = NULL;
  }
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
  queue_zbuf_mode = CS_ZBUF_NONE;
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
    memset (buf, 0, 4*super_lm_size*super_lm_size);
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

void OpenGLLightmapCache::Uncache (iPolygonTexture *polytex)
{
  iLightMap* lm = polytex->GetLightMap ();
  if (lm == NULL) return;
  csLMCacheData* clm = (csLMCacheData *)lm->GetCacheData ();
  if (!clm) return;
  lm->SetCacheData (NULL);
  csSuperLightMap* slm = &suplm[clm->super_lm_idx];
  csLMCacheDataQueue* data = (csLMCacheDataQueue*)slm->cacheData;

  if (clm->prev) clm->prev->next = clm->next;
  else data->head = clm->next;
  if (clm->next) clm->next->prev = clm->prev;
  else data->tail = clm->prev;
  delete clm;
}


int OpenGLLightmapCache::FindFreeSuperLightmap()
{
  int i;
  
  for(i = 0; i < super_lm_num; i++)
    if(suplm[i].cacheData == NULL) return i;
  return -1;
}

/**
 * Caches a whole precalculated superlighmap
 * s stores:
 * Distribution of the lightmaps in the superlightmap
 * Triangles
 * uv's for each triangles' vertex
 * Basically we have to blit all the lightmaps in a free super
 * lightmap
 */

void OpenGLLightmapCache::Cache(csTrianglesPerSuperLightmap* s)
{
  //First: Try to find a free superlightmap

  //Check if the superLightmap is already in the cache

  csRect* rectangleArray = s->rectangles.GetArray();
  iPolygonTexture** lmArray = s->lightmaps.GetArray();

  int i;
  int numLightmaps = s->numLightmaps;
  GLuint SLMHandle;
  if(s->cacheData)
  {    
    //The data is already in cache, let's see
    // if we need to recalculate the lightmaps
    // due the effect of dynamic lights
   
    SLMHandle = s->cacheData->Handle;
    for(i = 0; i < numLightmaps; i++)
    {
      if(lmArray[i]->RecalculateDynamicLights())
      {
        iLightMap* lm = lmArray[i]->GetLightMap();
        int lmwidth = lm->GetWidth();
        int lmheight = lm->GetHeight();
        csRGBpixel* lm_data = lm->GetMapData();
        csRect r = rectangleArray[i];
        glBindTexture (GL_TEXTURE_2D, SLMHandle);
        glTexSubImage2D(GL_TEXTURE_2D, 0,r.xmin,r.ymin,
          lmwidth, lmheight,GL_RGBA,GL_UNSIGNED_BYTE,lm_data);
      }
    }        
    return; 
  }
  
  //The superlightmap isn't in the cache, so we have to find
  
  int index = FindFreeSuperLightmap();
  if(index < 0)
  { //Flush one lightmap
    //Temporaly i will flush the following lightmap (it would be nice to 
    //implement a LRU algorithm here)
    cur_lm = (cur_lm + 1) % super_lm_num;
    Flush(cur_lm);
    suplm[cur_lm].Clear();
    index = cur_lm;
  }
  
  //Fill the superLightmap
  SourceData sd ;
  sd.superLMDataSource = s;
  suplm[index].cacheData = new csSLMCacheData();
  //We're going to fill the whole super lightmap, so we don't give 
  //width and height
  suplm[index].Alloc(0,0,sd);
  csSLMCacheData* superLMData = (csSLMCacheData*) suplm[index].cacheData;
  superLMData->Handle = SLMHandle = suplm[index].Handle;
  s->slId = index;
  
  for(i = 0; i < numLightmaps; i++)
  {
    iLightMap* lm = lmArray[i]->GetLightMap();
    int lmwidth = lm->GetWidth();
    int lmheigth = lm->GetHeight();
    csRGBpixel* lm_data = lm->GetMapData();
    csRect r = rectangleArray[i];
    glBindTexture (GL_TEXTURE_2D, SLMHandle);    
    glTexSubImage2D(GL_TEXTURE_2D, 0,r.xmin,r.ymin,
      lmwidth, lmheigth,GL_RGBA,GL_UNSIGNED_BYTE,lm_data);
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
    if(suplm[cur_lm].cacheData == NULL)
    {
      suplm[cur_lm].cacheData = new csLMCacheDataQueue();
      //We only can allocate this lightmap in non precalculated 
      //superlightmaps
      SourceData sd;
      sd.LMDataSource = piLM;
      clm = (csLMCacheData*)suplm[cur_lm].Alloc (lmwidth, lmheight, sd);
     }
      
    else if(!suplm[cur_lm].cacheData->IsPrecalcSuperlightmap())
    {

      //We only can allocate this lightmap in non precalculated 
      //superlightmaps
      SourceData sd;
      sd.LMDataSource = piLM;
      clm = (csLMCacheData*)suplm[cur_lm].Alloc (lmwidth, lmheight, sd);
     }

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
        if(suplm[prev_lm].cacheData == NULL) 
        {
          suplm[prev_lm].cacheData = new csLMCacheDataQueue();
          SourceData sd ;
          sd.LMDataSource = piLM;
          clm = (csLMCacheData*)suplm[prev_lm].Alloc (lmwidth, lmheight, sd);
        
        }
        else if(!suplm[prev_lm].cacheData->IsPrecalcSuperlightmap())
        {
          SourceData sd;
          sd.LMDataSource = piLM;
          clm = (csLMCacheData*)suplm[prev_lm].Alloc (lmwidth, lmheight, sd);        

        }

	if (clm)
  	{ 
	  clm->super_lm_idx = prev_lm;
	  num_proc = 0;	// Stop.
        }
	else 
  	{ 
	  num_proc--;
	}
      }

      if (!clm)
      {
        // There was no room in the current super-lightmap and neither
	// was there room in the previous one. We allocate a new
	// super-lightmap here.
        cur_lm = (cur_lm+1)%super_lm_num;
        num_lm_processed++;

        // Make sure all lightmaps are rendered.
        // If this actually renders lightmap then we might have a problem.
        // The number of super-lightmaps should be big enough so that
        // this is never needed.
        Flush (cur_lm);
        // Then free all lightmaps previously in this super lightmap.
        suplm[cur_lm].Clear ();
        suplm[cur_lm].cacheData = new csLMCacheDataQueue();

	// Now allocate the first lightmap in this super-lightmap.
	// This can not fail.
	SourceData sd;
	sd.LMDataSource = piLM;
	clm = (csLMCacheData*)suplm[cur_lm].Alloc (lmwidth, lmheight, sd);
	clm->super_lm_idx = cur_lm;
      }
    }

    piLM->SetCacheData (clm);
    //clm->Source = piLM;
    Load (clm);
    float lm_low_u, lm_low_v, lm_high_u, lm_high_v;
   
    polytex->GetTextureBox (lm_low_u, lm_low_v, lm_high_u, lm_high_v);

    // lightmap fudge factor
    if (lm_high_u <= lm_low_u)
      clm->lm_scale_u = 1.;       // @@@ Is this right?
    else
      clm->lm_scale_u = 1. / (lm_high_u - lm_low_u);

    if (lm_high_v <= lm_low_v)
      clm->lm_scale_v = 1.;       // @@@ Is this right?
    else
      clm->lm_scale_v = 1. / (lm_high_v - lm_low_v);

    lm_low_u -= .75 / (double (lmwidth) * clm->lm_scale_u);
    lm_high_u += .75 / (double (lmwidth) * clm->lm_scale_u);

    lm_low_v -= .75 / (double (lmheight) * clm->lm_scale_v);
    lm_high_v += .75 / (double (lmheight) * clm->lm_scale_v);

    clm->lm_scale_u = 1.00 / (lm_high_u - lm_low_u);
    clm->lm_scale_v = 1.00 / (lm_high_v - lm_low_v);

    // We take 95% of the total
    // Calculate position in super lightmap.
    float dlm = 1. / float (super_lm_size);
    float sup_u = float (clm->super_lm_rect.xmin) * dlm;
    float sup_v = float (clm->super_lm_rect.ymin) * dlm;
    clm->lm_scale_u = clm->lm_scale_u * float (lmwidth) * dlm;
    clm->lm_scale_v = clm->lm_scale_v * float (lmheight) * dlm;
    
    clm->lm_offset_u = lm_low_u - sup_u / clm->lm_scale_u;
    clm->lm_offset_v = lm_low_v - sup_v / clm->lm_scale_v;
  }
}

void OpenGLLightmapCache::Load (csLMCacheData *d)
{
  iLightMap* lightmap_info = d->Source;
  int lmwidth = lightmap_info->GetWidth ();
  int lmheight = lightmap_info->GetHeight ();
  csRGBpixel* lm_data = lightmap_info->GetMapData ();

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
  glColor4f (1, 1, 1, 0);
  csGraphics3DOGLCommon::SetupBlend (CS_FX_SRCDST, 0, false);
  csGraphics3DOGLCommon::SetClientStates (CS_CLIENTSTATE_VT);
  for (i = 0 ; i < super_lm_num ; i++)
  {
    if(suplm[i].cacheData)
      if(suplm[i].cacheData->IsUnlit()) continue;
    suplm[i].queue.Flush (suplm[i].Handle);
  }
  csGraphics3DOGLCommon::SetupBlend (CS_FX_ALPHA, 0, false);
  csGraphics3DOGLCommon::SetClientStates (CS_CLIENTSTATE_ALL);
  glShadeModel (GL_SMOOTH);
  for (i = 0; i < super_lm_num;i++)
  {
    if (!suplm[i].cacheData)
    {
      suplm[i].queue.Reset();
      continue;
    }
    if (suplm[i].cacheData->HasFog())
    { 
      csSLMCacheData* cacheData = (csSLMCacheData*) suplm[i].cacheData;
      suplm[i].queue.FlushFog(cacheData->FogHandle);
    }
    else suplm[i].queue.Reset();
  }
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
  csGraphics3DOGLCommon::SetupBlend (CS_FX_SRCDST, 0, false);
  csGraphics3DOGLCommon::SetClientStates (CS_CLIENTSTATE_VT);
  lm_queue.Flush (suplm[sup_idx].Handle);
}

bool OpenGLLightmapCache::IsLightmapOK (iPolygonTexture *polytex)
{
  return (polytex->GetLightMap () &&
    (polytex->GetLightMap ()->GetWidth () <= super_lm_size) &&
    (polytex->GetLightMap ()->GetHeight () <= super_lm_size));
}
