/*
    Copyright (C) 2002 by Anders Stenberg
    Written by Anders Stenberg

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
#include "video/canvas/openglcommon/iglstates.h"
#include "video/canvas/openglcommon/glstates.h"

SCF_IMPLEMENT_IBASE (csGLStateCache)
  SCF_IMPLEMENTS_INTERFACE (iGLStateCache)
SCF_IMPLEMENT_IBASE_END

csGLStateCache::~csGLStateCache()
{
}

void csGLStateCache::InitCache()
{
  int i;
  glGetIntegerv( GL_ALPHA_TEST_FUNC, (GLint*)&parameter_alpha_func );
  glGetFloatv( GL_ALPHA_TEST_REF, &parameter_alpha_ref );
  glGetIntegerv( GL_BLEND_SRC, (GLint*)&parameter_blend_source );
  glGetIntegerv( GL_BLEND_DST, (GLint*)&parameter_blend_destination );
  glGetIntegerv( GL_CULL_FACE_MODE, (GLint*)&parameter_cull_mode );
  glGetIntegerv( GL_DEPTH_FUNC, (GLint*)&parameter_depth_func );
  glGetBooleanv( GL_DEPTH_WRITEMASK, &parameter_depth_mask );
  glGetIntegerv( GL_SHADE_MODEL, (GLint*)&parameter_shade_model );
  glGetIntegerv( GL_STENCIL_FUNC, (GLint*)&parameter_stencil_func );
  glGetIntegerv( GL_STENCIL_VALUE_MASK, (GLint*)&parameter_stencil_mask );
  glGetIntegerv( GL_STENCIL_REF, &parameter_stencil_ref );
  glGetIntegerv( GL_STENCIL_FAIL, (GLint*)&parameter_stencil_fail );
  glGetIntegerv( GL_STENCIL_PASS_DEPTH_FAIL, (GLint*)&parameter_stencil_zfail );
  glGetIntegerv( GL_STENCIL_PASS_DEPTH_PASS, (GLint*)&parameter_stencil_zpass );
  enabled_GL_DEPTH_TEST = glIsEnabled (GL_DEPTH_TEST);
  enabled_GL_BLEND = glIsEnabled (GL_BLEND);
  enabled_GL_DITHER = glIsEnabled (GL_DITHER);
  enabled_GL_STENCIL_TEST = glIsEnabled (GL_STENCIL_TEST);
  enabled_GL_CULL_FACE = glIsEnabled (GL_CULL_FACE);
  enabled_GL_POLYGON_OFFSET_FILL = glIsEnabled (GL_POLYGON_OFFSET_FILL);
  enabled_GL_LIGHTING = glIsEnabled (GL_LIGHTING);
  enabled_GL_ALPHA_TEST = glIsEnabled (GL_ALPHA_TEST);
  enabled_GL_TEXTURE_2D[0] = glIsEnabled (GL_TEXTURE_2D);
  for (i = 1 ; i < MAX_LAYER ; i++)
    enabled_GL_TEXTURE_2D[i] = enabled_GL_TEXTURE_2D[0];

  memset( texture1d, 0, 32*sizeof(GLuint) );
  memset( texture2d, 0, 32*sizeof(GLuint) );
}

void csGLStateCache::SetTexture( GLenum target, GLuint texture, int layer )
{
  if( target == GL_TEXTURE_1D )
  {
    if( texture != texture1d[layer] )
    {
      texture1d[layer] = texture;
      glBindTexture( target, texture );
    }
  }
  if( target == GL_TEXTURE_2D )
  {
    if( texture != texture2d[layer] )
    {
      texture2d[layer] = texture;
      glBindTexture( target, texture );
    }
  }
}
GLuint csGLStateCache::GetTexture(GLenum target, GLuint /*texture*/, int layer)
{
  if( target == GL_TEXTURE_1D )
    return texture1d[layer];
  if( target == GL_TEXTURE_2D )
    return texture2d[layer];
  return 0;
}
