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

#ifndef __CS_GLSTATES_H__
#define __CS_GLSTATES_H__

#if defined(CS_OPENGL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENGL_PATH,gl.h)
#else
#include <GL/gl.h>
#endif

#include "video/canvas/openglcommon/iglstates.h"

#define IMPLEMENT_CACHED_BOOL( name ) \
  bool enabled_##name; \
  void Enable_##name () { \
    if( !enabled_##name ) { \
      enabled_##name = true;  \
      glEnable( name ); \
    } \
  } \
  void Disable_##name () { \
    if( enabled_##name ) { \
      enabled_##name = false;  \
      glDisable( name ); \
    } \
  } \
  bool IsEnabled_##name () const { \
    return enabled_##name; \
  }

#define MAX_LAYER 16

#define IMPLEMENT_CACHED_BOOL_LAYER( name ) \
  bool enabled_##name[MAX_LAYER]; \
  void Enable_##name (int l = 0) { \
    if( !enabled_##name[l] ) { \
      enabled_##name[l] = true;  \
      glEnable( name ); \
    } \
  } \
  void Disable_##name (int l = 0) { \
    if( enabled_##name[l] ) { \
      enabled_##name[l] = false;  \
      glDisable( name ); \
    } \
  } \
  bool IsEnabled_##name (int l = 0) const { \
    return enabled_##name[l]; \
  }

#define IMPLEMENT_CACHED_PARAMETER_1( func, name, type1, param1 ) \
  type1 parameter_##param1; \
  void Set##name (type1 param1, bool forced = false) { \
    if( forced || (param1 != parameter_##param1) ) { \
      parameter_##param1 = param1;  \
      func( param1 ); \
    } \
  } \
  void Get##name (type1 & param1) { \
    param1 = parameter_##param1;  \
  }

#define IMPLEMENT_CACHED_PARAMETER_2( func, name, type1, param1, type2, param2 ) \
  type1 parameter_##param1; \
  type2 parameter_##param2; \
  void Set##name (type1 param1, type2 param2, bool forced = false) { \
    if( forced || (param1 != parameter_##param1) || (param2 != parameter_##param2) ) { \
      parameter_##param1 = param1;  \
      parameter_##param2 = param2;  \
      func( param1, param2 ); \
    } \
  } \
  void Get##name (type1 & param1, type2 & param2) { \
    param1 = parameter_##param1;  \
    param2 = parameter_##param2;  \
  }

#define IMPLEMENT_CACHED_PARAMETER_3( func, name, type1, param1, type2, param2, type3, param3 ) \
  type1 parameter_##param1; \
  type2 parameter_##param2; \
  type3 parameter_##param3; \
  void Set##name (type1 param1, type2 param2, type3 param3, bool forced = false) { \
    if( forced || (param1 != parameter_##param1) || (param2 != parameter_##param2) || (param3 != parameter_##param3) ) { \
      parameter_##param1 = param1;  \
      parameter_##param2 = param2;  \
      parameter_##param3 = param3;  \
      func( param1, param2, param3 ); \
    } \
  } \
  void Get##name (type1 &param1, type2 & param2, type3 & param3) { \
    param1 = parameter_##param1;  \
    param2 = parameter_##param2;  \
    param3 = parameter_##param3;  \
  }


class csGLStateCache : public iGLStateCache
{
public:

  SCF_DECLARE_IBASE;

  csGLStateCache ()
  {
    SCF_CONSTRUCT_IBASE (NULL);
  }
  
  virtual ~csGLStateCache();

  /// Init cache
  void InitCache();

  // Standardized caches
  IMPLEMENT_CACHED_BOOL (GL_DEPTH_TEST)
  IMPLEMENT_CACHED_BOOL (GL_BLEND)
  IMPLEMENT_CACHED_BOOL (GL_DITHER)
  IMPLEMENT_CACHED_BOOL (GL_STENCIL_TEST)
  IMPLEMENT_CACHED_BOOL (GL_CULL_FACE)
  IMPLEMENT_CACHED_BOOL (GL_POLYGON_OFFSET_FILL)
  IMPLEMENT_CACHED_BOOL (GL_LIGHTING)
  IMPLEMENT_CACHED_BOOL (GL_ALPHA_TEST)
  IMPLEMENT_CACHED_BOOL_LAYER (GL_TEXTURE_2D)
  IMPLEMENT_CACHED_PARAMETER_2( glAlphaFunc, AlphaFunc, GLenum, alpha_func, GLclampf, alpha_ref )
  IMPLEMENT_CACHED_PARAMETER_2( glBlendFunc, BlendFunc, GLenum, blend_source, GLenum, blend_destination )
  IMPLEMENT_CACHED_PARAMETER_1( glCullFace, CullFace, GLenum, cull_mode )
  IMPLEMENT_CACHED_PARAMETER_1( glDepthFunc, DepthFunc, GLenum, depth_func )
  IMPLEMENT_CACHED_PARAMETER_1( glDepthMask, DepthMask, GLboolean, depth_mask )
  IMPLEMENT_CACHED_PARAMETER_1( glShadeModel, ShadeModel, GLenum, shade_model )
  IMPLEMENT_CACHED_PARAMETER_3( glStencilFunc, StencilFunc, GLenum, stencil_func, GLint, stencil_ref, GLuint, stencil_mask )
  IMPLEMENT_CACHED_PARAMETER_3( glStencilOp, StencilOp, GLenum, stencil_fail, GLenum, stencil_zfail, GLenum, stencil_zpass )

  // Special caches
  GLuint texture1d[32]; // 32 max texture layers
  GLuint texture2d[32]; // 32 max texture layers
  void SetTexture( GLenum target, GLuint texture, int layer = 0 );
  GLuint GetTexture( GLenum target, GLuint texture, int layer = 0 );

};

#undef IMPLEMENT_CACHED_BOOL
#undef IMPLEMENT_CACHED_PARAMETER_1
#undef IMPLEMENT_CACHED_PARAMETER_2
#undef IMPLEMENT_CACHED_PARAMETER_3

#endif // __CS_GLSTATES_H__
