#include "cssysdef.h"
#include "video/canvas/openglcommon/iglstates.h"
#include "video/canvas/openglcommon/glstates.h"

SCF_IMPLEMENT_IBASE (csGLStateCache)
  SCF_IMPLEMENTS_INTERFACE (iGLStateCache)
SCF_IMPLEMENT_IBASE_END

void csGLStateCache::InitCache()
{
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

  memset( texture1d, 0, 32*sizeof(GLuint) );
  memset( texture2d, 0, 32*sizeof(GLuint) );
}

void csGLStateCache::EnableState( GLenum state, int layer )
{
  csHashIterator cIterator( &statecache, state );
  while( cIterator.HasNext() )
  {
    csOpenGLState* glstate =  (csOpenGLState*)cIterator.Next();
    if( (glstate->state == state) && (glstate->layer == layer) )
    {
      if( !glstate->enabled )
      {
        glEnable( state );
        glstate->enabled = true;
      }
      return;
    }
  }
  statecache.Put( state, new csOpenGLState( state, true, layer ) );
  glEnable( state );
}

void csGLStateCache::DisableState( GLenum state, int layer )
{
  csHashIterator cIterator( &statecache, state );
  while( cIterator.HasNext() )
  {
    csOpenGLState* glstate =  (csOpenGLState*)cIterator.Next();
    if( (glstate->state == state) && (glstate->layer == layer) )
    {
      if( glstate->enabled )
      {
        glDisable( state );
        glstate->enabled = false;
      }
      return;
    }
  }
  statecache.Put( state, new csOpenGLState( state, false, layer ) );
  glDisable( state );
}

void csGLStateCache::ToggleState( GLenum state, int layer )
{
  csHashIterator cIterator( &statecache, state );
  while( cIterator.HasNext() )
  {
    csOpenGLState* glstate =  (csOpenGLState*)cIterator.Next();
    if( (glstate->state == state) && (glstate->layer == layer) )
    {
      if( glstate->enabled )
        glDisable( state );
      else
        glEnable( state );
      glstate->enabled = !glstate->enabled;
      return;
    }
  }
  statecache.Put( state, new csOpenGLState( state, true, layer ) );
  glEnable( state );
}

bool csGLStateCache::GetState( GLenum state, int layer )
{
  csHashIterator cIterator( &statecache, state );
  while( cIterator.HasNext() )
  {
    csOpenGLState* glstate = (csOpenGLState*)cIterator.Next();
    if( (glstate->state == state) && (glstate->layer == layer) )
      return glstate->enabled;
  }
  return false;
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
GLuint csGLStateCache::GetTexture( GLenum target, GLuint texture, int layer)
{
  if( target == GL_TEXTURE_1D )
    return texture1d[layer];
  if( target == GL_TEXTURE_2D )
    return texture2d[layer];
}
