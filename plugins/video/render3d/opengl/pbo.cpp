/*
    Copyright (C) 2009 by Frank Richter

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

#include "gl_render3d.h"
#include "pbo.h"

#include "csutil/sysfunc.h"

//#define PBO_DEBUG

#ifdef PBO_DEBUG
  #define PBO_PRINTF csPrintf
#else
  #define PBO_PRINTF while(0) csPrintf
#endif

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{
  PBOWrapper::~PBOWrapper()
  {
    if (pbo != 0)
    {
      csGLGraphics3D::ext->glDeleteBuffersARB (1, &pbo);
      PBO_PRINTF ("freed %zu PBO %lu\n", size, (unsigned long)pbo);
    }
  }

  GLuint PBOWrapper::GetPBO (GLenum target)
  {
    if (pbo == 0)
    {
      csGLGraphics3D::ext->glGenBuffersARB (1, &pbo);
      GLuint oldBuffer = csGLGraphics3D::statecache->GetBufferARB (target);
      csGLGraphics3D::statecache->SetBufferARB (target, pbo, true);
      csGLGraphics3D::ext->glBufferDataARB (target, (GLsizei)size, 0, GL_STREAM_READ_ARB);
      csGLGraphics3D::statecache->SetBufferARB (target, oldBuffer);
      PBO_PRINTF ("created %zu PBO %lu\n", size, (unsigned long)pbo);
    }
    return pbo;
  }
  //---------------------------------------------------------------------------

  TextureReadbackPBO::TextureReadbackPBO (PBOWrapper* pbo, size_t size)
   : SuperClass (this), pbo (pbo), size (size), mappedData (0) {}
   
  TextureReadbackPBO::~TextureReadbackPBO ()
  {
    //csGLGraphics3D::ext->glDeleteBuffersARB (1, &pbo);
    if (mappedData != 0)
    {
      GLuint _pbo = pbo->GetPBO (GL_PIXEL_PACK_BUFFER_ARB);
      csGLGraphics3D::statecache->SetBufferARB (GL_PIXEL_PACK_BUFFER_ARB, _pbo, true);
      csGLGraphics3D::ext->glUnmapBufferARB (GL_PIXEL_PACK_BUFFER_ARB);
      csGLGraphics3D::statecache->SetBufferARB (GL_PIXEL_PACK_BUFFER_ARB, 0, true);
    }
  }

  char* TextureReadbackPBO::GetData () const
  {
    if (mappedData == 0)
    {
      GLuint _pbo = pbo->GetPBO (GL_PIXEL_PACK_BUFFER_ARB);
      csGLGraphics3D::statecache->SetBufferARB (GL_PIXEL_PACK_BUFFER_ARB, _pbo, true);
      mappedData = csGLGraphics3D::ext->glMapBufferARB (
        GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY_ARB);
      csGLGraphics3D::statecache->SetBufferARB (GL_PIXEL_PACK_BUFFER_ARB, 0, true);
    }
    return (char*)mappedData;
  }
}
CS_PLUGIN_NAMESPACE_END(gl3d)
