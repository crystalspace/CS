/*
    Copyright (C) 1998-2004 by Jorrit Tyberghein
	      (C) 2003 by Philip Aumayr
	      (C) 2004-2007 by Frank Richter

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

#include "csgfx/imagememory.h"
#include "csgfx/packrgb.h"
#include "csgfx/rgbpixel.h"
#include "csutil/blockallocator.h"

#include "gl_render3d.h"
#include "gl_txtmgr_imagetex.h"
#include "gl_txtmgr_lightmap.h"

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{

CS_LEAKGUARD_IMPLEMENT(csGLRendererLightmap);
CS_LEAKGUARD_IMPLEMENT(csGLSuperLightmap);

class csRLMAlloc : public csBlockAllocator<csGLRendererLightmap>
{
public:
  csRLMAlloc () : csBlockAllocator<csGLRendererLightmap> (512) { }
};

CS_IMPLEMENT_STATIC_VAR (GetRLMAlloc, csRLMAlloc, ())

//---------------------------------------------------------------------------

void csGLRendererLightmap::DecRef ()
{
  csRefTrackerAccess::TrackDecRef (scfObject, scfRefCount);
  scfRefCount--;							
  if (scfRefCount == 0)							
  {									
    CS_ASSERT (slm != 0);
    slm->FreeRLM (this);
    return;								
  }									
}

csGLRendererLightmap::csGLRendererLightmap () : scfImplementationType (this)
{
}

csGLRendererLightmap::~csGLRendererLightmap ()
{
#ifdef CS_DEBUG
  if (slm->texHandle != (GLuint)~0)
  {
    csRGBcolor* pat = new csRGBcolor[rect.Width () * rect.Height ()];
    int x, y;
    csRGBcolor* p = pat;
    for (y = 0; y < rect.Height (); y++)
    {
      for (x = 0; x < rect.Width (); x++)
      {
	p->red = ((x ^ y) & 1) * 0xff;
	p++;
      }
    }

    csGLGraphics3D::statecache->SetTexture (
      GL_TEXTURE_2D, slm->texHandle);

    glTexSubImage2D (GL_TEXTURE_2D, 0, rect.xmin, rect.ymin, 
      rect.Width (), rect.Height (),
      GL_RGB, GL_UNSIGNED_BYTE, pat);

    delete[] pat;
  }
#endif
}

void csGLRendererLightmap::GetSLMCoords (int& left, int& top, 
    int& width, int& height)
{
  left = rect.xmin; top  = rect.ymin;
  width = rect.Width (); height = rect.Height ();
}
    
void csGLRendererLightmap::SetData (csRGBcolor* data)
{
  slm->CreateTexture ();

  csGLGraphics3D::statecache->SetTexture (
    GL_TEXTURE_2D, slm->texHandle);

  const uint8* packed = csPackRGB::PackRGBcolorToRGB (data,
    rect.Width () * rect.Height ());
  glTexSubImage2D (GL_TEXTURE_2D, 0, rect.xmin, rect.ymin, 
    rect.Width (), rect.Height (),
    GL_RGB, GL_UNSIGNED_BYTE, packed);
  csPackRGB::DiscardPackedRGB (packed);
}

void csGLRendererLightmap::SetLightCellSize (int size)
{
  (void)size;
}

//---------------------------------------------------------------------------


void csGLSuperLightmap::DecRef ()
{
  csRefTrackerAccess::TrackDecRef (scfObject, scfRefCount);
  scfRefCount--;							
  if (scfRefCount == 0)							
  {
    if (txtmgr != 0)
      txtmgr->superLMs.Delete (this);
    delete this;
    return;								
  }									
}

csGLSuperLightmap::csGLSuperLightmap (csGLTextureManager* txtmgr, 
				      int width, int height) :
  scfImplementationType (this)
{
  w = width; h = height;
  texHandle = (GLuint)~0;
  numRLMs = 0;
  csGLSuperLightmap::txtmgr = txtmgr;
}

csGLSuperLightmap::~csGLSuperLightmap ()
{
  DeleteTexture ();
}

void csGLSuperLightmap::CreateTexture ()
{
  if (texHandle == (GLuint)~0)
  {
    glGenTextures (1, &texHandle);

    csGLGraphics3D::statecache->SetTexture (
      GL_TEXTURE_2D, texHandle);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    csRGBcolor* data = new csRGBcolor [w * h];
#ifdef CS_DEBUG
    // Fill the background for debugging purposes (to quickly see what's
    // a lightmap and what's not; esp. useful when LMs are rather dark -
    // would be hardly visible if at all on black)
    // And to have it not that boring, add a neat backdrop.
    static const uint16 debugBG[16] =
      {0x0000, 0x3222, 0x4a36, 0x422a, 0x3222, 0x0a22, 0x4a22, 0x33a2, 
       0x0000, 0x2232, 0x364a, 0x2a42, 0x2232, 0x220a, 0x224a, 0xa233};

    csRGBcolor* p = data;
    int y, x;
    for (y = 0; y < h; y++)
    {
      for (x = 0; x < w; x++)
      {
	const int bitNum = 6;
	int b = (~x) & 0xf;
	int px = (debugBG[y & 0xf] & (1 << b));
	if (b < bitNum)
	{
	  px <<= bitNum - b;
	}
	else
	{
	  px >>= b - bitNum;
	}
	p->blue = ~(1 << bitNum) + px;
	p++;
      }
    }
#endif
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, 
      GL_RGB, GL_UNSIGNED_BYTE, data);
    delete[] data;
  }
}

void csGLSuperLightmap::DeleteTexture ()
{
  if (texHandle != (GLuint)~0)
  {
    csGLTextureManager::UnsetTexture (GL_TEXTURE_2D, texHandle);

    glDeleteTextures (1, &texHandle);
    texHandle = (GLuint)~0;
    th = 0;
  }
}

void csGLSuperLightmap::FreeRLM (csGLRendererLightmap* rlm)
{
  if (--numRLMs == 0)
  {
    DeleteTexture ();
  }

  // IncRef() ourselves manually.
  // Otherwise freeing the RLM could trigger our own destruction -
  // causing an assertion in block allocator (due to how BA frees items and
  // the safety assertions on BA destruction.)
  csRefTrackerAccess::TrackIncRef (this, scfRefCount);
  scfRefCount++;
  GetRLMAlloc ()->Free (rlm);
  DecRef ();
}

csPtr<iRendererLightmap> csGLSuperLightmap::RegisterLightmap (int left, int top,
	int width, int height)
{
  csGLRendererLightmap* rlm = GetRLMAlloc ()->Alloc ();
  rlm->slm = this;
  rlm->rect.Set (left, top, left + width, top + height);

  numRLMs++;

  return csPtr<iRendererLightmap> (rlm);
}

csPtr<iImage> csGLSuperLightmap::Dump ()
{
  // @@@ hmm... or just return an empty image?
  if (texHandle == (GLuint)~0) return 0;

  GLint tw, th;
  csGLGraphics3D::statecache->SetTexture (GL_TEXTURE_2D, texHandle);

  glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tw);
  glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &th);

  uint8* data = new uint8[tw * th * 4];
  glGetTexImage (GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

  csImageMemory* lmimg = 
    new csImageMemory (tw, th,
    data, true, 
    CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA);

  return csPtr<iImage> (lmimg);
}

iTextureHandle* csGLSuperLightmap::GetTexture ()
{
  if (th == 0)
  {
    CreateTexture ();
    th.AttachNew (new csGLTextureHandle (iTextureHandle::CS_TEX_IMG_2D, 
      texHandle, txtmgr->G3D));
  }
  return th;
}

}
CS_PLUGIN_NAMESPACE_END(gl3d)
