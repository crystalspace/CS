#ifndef __IGLSTATES_H__
#define __IGLSTATES_H__

#include "csutil/scf.h" 

#if defined(CS_OPENGL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENGL_PATH,gl.h)
#else
#include <GL/gl.h>
#endif


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

  /// Enable state
  virtual void EnableState( GLenum state, int layer = 0 ) = 0;

  /// Disable state
  virtual void DisableState( GLenum state, int layer = 0 ) = 0;

  /// Toggle state
  virtual void ToggleState( GLenum state, int layer = 0 ) = 0;

  /// Get state
  virtual bool GetState( GLenum state, int layer = 0 ) = 0;

  // Standardized caches
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

#undef IMPLEMENT_CACHED_PARAMETER_1
#undef IMPLEMENT_CACHED_PARAMETER_2
#undef IMPLEMENT_CACHED_PARAMETER_3

#endif
