#ifndef __CS_IGLSTATES_H__
#define __CS_IGLSTATES_H__

#include "csutil/scf.h" 

#if defined(CS_OPENGL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENGL_PATH,gl.h)
#else
#include <GL/gl.h>
#endif


#define IMPLEMENT_CACHED_BOOL( name ) \
  virtual void Enable_##name () = 0; \
  virtual void Disable_##name () = 0; \
  virtual bool IsEnabled_##name () const = 0;

#define IMPLEMENT_CACHED_BOOL_LAYER( name ) \
  virtual void Enable_##name (int l = 0) = 0; \
  virtual void Disable_##name (int l = 0) = 0; \
  virtual bool IsEnabled_##name (int l = 0) const = 0;

#define IMPLEMENT_CACHED_PARAMETER_1( func, name, type1, param1 ) \
  virtual void Set##name (type1 param1, bool forced = false) = 0; \
  virtual void Get##name (type1 & param1) = 0;

#define IMPLEMENT_CACHED_PARAMETER_2( func, name, type1, param1, type2, param2 ) \
  virtual void Set##name (type1 param1, type2 param2, bool forced = false) = 0; \
  virtual void Get##name (type1 & param1, type2 & param2) = 0;

#define IMPLEMENT_CACHED_PARAMETER_3( func, name, type1, param1, type2, param2, type3, param3 ) \
  virtual void Set##name (type1 param1, type2 param2, type3 param3, bool forced = false) = 0; \
  virtual void Get##name (type1 &param1, type2 & param2, type3 & param3) = 0;

SCF_VERSION (iGLStateCache, 0, 0, 1);

class iGLStateCache : public iBase
{
public:

  /// Init cache
  virtual void InitCache() = 0;

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
  virtual void SetTexture( GLenum target, GLuint texture, int layer = 0 ) = 0;
  virtual GLuint GetTexture( GLenum target, GLuint texture, int layer = 0 ) = 0;
};

#undef IMPLEMENT_CACHED_BOOL
#undef IMPLEMENT_CACHED_PARAMETER_1
#undef IMPLEMENT_CACHED_PARAMETER_2
#undef IMPLEMENT_CACHED_PARAMETER_3

#endif // __CS_IGLSTATES_H__
