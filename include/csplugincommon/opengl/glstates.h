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

#include "glextmanager.h"

// Set to 'true' to force state changing commands. For debugging.
#define FORCE_STATE_CHANGE			  false/*true*/

#define IMPLEMENT_CACHED_BOOL(name)					     \
  bool enabled_##name;							     \
  void Enable_##name ()							     \
  {									     \
    if (!enabled_##name || FORCE_STATE_CHANGE)				     \
    {									     \
      enabled_##name = true;						     \
      glEnable (name);							     \
    }									     \
  }									     \
  void Disable_##name ()						     \
  {									     \
    if (enabled_##name || FORCE_STATE_CHANGE) {				     \
      enabled_##name = false;						     \
      glDisable (name);							     \
    }									     \
  }									     \
  bool IsEnabled_##name () const					     \
  {									     \
    return enabled_##name;						     \
  }

#define CS_GL_MAX_LAYER 16


#define IMPLEMENT_CACHED_BOOL_CURRENTLAYER(name)			     \
  bool enabled_##name[CS_GL_MAX_LAYER]; 				     \
  void Enable_##name () 						     \
  { 									     \
    if (!enabled_##name[currentUnit] || FORCE_STATE_CHANGE)		     \
    { 									     \
      ActivateTU (); 							     \
      enabled_##name[currentUnit] = true;  				     \
      glEnable (name); 							     \
    } 									     \
  } 									     \
  void Disable_##name () 						     \
  { 									     \
    if (enabled_##name[currentUnit] || FORCE_STATE_CHANGE)		     \
    { 									     \
      ActivateTU (); 							     \
      enabled_##name[currentUnit] = false;  				     \
      glDisable (name); 						     \
    } 									     \
  } 									     \
  bool IsEnabled_##name () const 					     \
  { 									     \
    return enabled_##name[currentUnit]; 				     \
  }

#define IMPLEMENT_CACHED_PARAMETER_1(func, name, type1, param1) 	     \
  type1 parameter_##param1; 						     \
  void Set##name (type1 param1, bool forced = false)			     \
  { 									     \
    if (forced || (param1 != parameter_##param1)			     \
      || FORCE_STATE_CHANGE) 						     \
    { 									     \
      parameter_##param1 = param1;  					     \
      func (param1); 							     \
    } 									     \
  } 									     \
  void Get##name (type1 & param1) const					     \
  { 									     \
    param1 = parameter_##param1;  					     \
  }

#define IMPLEMENT_CACHED_PARAMETER_2(func, name, type1, param1, type2, param2) \
  type1 parameter_##param1; 						     \
  type2 parameter_##param2; 						     \
  void Set##name (type1 param1, type2 param2, bool forced = false)	     \
  { 									     \
    if (forced || (param1 != parameter_##param1)			     \
      || (param2 != parameter_##param2)					     \
      || FORCE_STATE_CHANGE)						     \
    { 									     \
      parameter_##param1 = param1;  					     \
      parameter_##param2 = param2;  					     \
      func (param1, param2);   						     \
    }   			  					     \
  }   			  						     \
  void Get##name (type1 & param1, type2 & param2) const			     \
  {   			  			  			     \
    param1 = parameter_##param1;  					     \
    param2 = parameter_##param2;  					     \
  }

#define IMPLEMENT_CACHED_PARAMETER_3(func, name, type1, param1, type2, param2, type3, param3) \
  type1 parameter_##param1; 						     \
  type2 parameter_##param2; 						     \
  type3 parameter_##param3; 						     \
  void Set##name (type1 param1, type2 param2, type3 param3, bool forced = false) \
  { 									     \
    if (forced || (param1 != parameter_##param1)			     \
      || (param2 != parameter_##param2)					     \
      || (param3 != parameter_##param3)					     \
      || FORCE_STATE_CHANGE)						     \
    { 									     \
      parameter_##param1 = param1;  					     \
      parameter_##param2 = param2;  					     \
      parameter_##param3 = param3;  					     \
      func (param1, param2, param3); 					     \
    } 									     \
  } 									     \
  void Get##name (type1 &param1, type2 & param2, type3 & param3) const	     \
  { 									     \
    param1 = parameter_##param1;  					     \
    param2 = parameter_##param2;  					     \
    param3 = parameter_##param3;  					     \
  }

#define IMPLEMENT_CACHED_PARAMETER_4(func, name, type1, param1, 	     \
    type2, param2, type3, param3, type4, param4) 			     \
  type1 parameter_##param1; 						     \
  type2 parameter_##param2; 						     \
  type3 parameter_##param3; 						     \
  type4 parameter_##param4; 						     \
  void Set##name (type1 param1, type2 param2, type3 param3, type4 param4,    \
    bool forced = false) 						     \
  { 									     \
    if (forced || (param1 != parameter_##param1) || 			     \
      (param2 != parameter_##param2) || 				     \
      (param3 != parameter_##param3) || 				     \
      (param4 != parameter_##param4)					     \
      || FORCE_STATE_CHANGE) 						     \
    { 									     \
      parameter_##param1 = param1;  					     \
      parameter_##param2 = param2;  					     \
      parameter_##param3 = param3;  					     \
      parameter_##param4 = param4;  					     \
      func (param1, param2, param3, param4); 				     \
    } 									     \
  } 									     \
  void Get##name (type1 &param1, type2 & param2, type3 & param3, type4& param4) const\
  { 									     \
    param1 = parameter_##param1;  					     \
    param2 = parameter_##param2; 					     \
    param3 = parameter_##param3;  					     \
    param4 = parameter_##param4;  					     \
  }

#define IMPLEMENT_CACHED_CLIENT_STATE(name)				     \
  bool enabled_##name;							     \
  void Enable_##name () 						     \
  { 									     \
    if (!enabled_##name || FORCE_STATE_CHANGE)				     \
    { 									     \
      enabled_##name = true;  						     \
      glEnableClientState (name); 					     \
    } 									     \
  } 									     \
  void Disable_##name () 						     \
  { 									     \
    if (enabled_##name || FORCE_STATE_CHANGE) {				     \
      enabled_##name = false;  						     \
      glDisableClientState (name); 					     \
    } 									     \
  } 									     \
  bool IsEnabled_##name () const 					     \
  { 									     \
    return enabled_##name; 						     \
  }


#define IMPLEMENT_CACHED_CLIENT_STATE_LAYER(name)	      		     \
  bool enabled_##name[CS_GL_MAX_LAYER]; 		     		     \
  void Enable_##name () 		     		     		     \
  { 		     		     		     		     	     \
    if (!enabled_##name[currentUnit] || FORCE_STATE_CHANGE)	     	     \
    { 		     		     		     		     	     \
      ActivateTU (); 		     		     		     	     \
      enabled_##name[currentUnit] = true;  		     		     \
      glEnableClientState (name); 		     		     	     \
    }									     \
  }								  	     \
  void Disable_##name ()					    	     \
  {									     \
    if (enabled_##name[currentUnit] || FORCE_STATE_CHANGE) {   	     	     \
      ActivateTU (); 	     	     	     	     	     		     \
      enabled_##name[currentUnit] = false;				     \
      glDisableClientState (name);					     \
    }									     \
  }									     \
  bool IsEnabled_##name () const					     \
  {									     \
    return enabled_##name[currentUnit];					     \
  }

#define IMPLEMENT_CACHED_PARAMETER_1_LAYER(func, name, type1, param1)	     \
  type1 parameter_##param1[CS_GL_MAX_LAYER];				     \
  void Set##name (type1 param1, bool forced = false)			     \
  {									     \
    if (forced || (param1 != parameter_##param1[currentUnit])		     \
       || FORCE_STATE_CHANGE)						     \
    {									     \
      ActivateTU ();							     \
      parameter_##param1[currentUnit] = param1;				     \
      func (param1);							     \
    }									     \
  }									     \
  void Get##name (type1 &param1) const					     \
  {									     \
    param1 = parameter_##param1[currentUnit];				     \
  }


#define IMPLEMENT_CACHED_PARAMETER_2_LAYER(func, name, type1, param1,	     \
  type2, param2)							     \
  type1 parameter_##param1[CS_GL_MAX_LAYER];				     \
  type2 parameter_##param2[CS_GL_MAX_LAYER];				     \
  void Set##name (type1 param1, type2 param2, bool forced = false)	     \
  {									     \
    if (forced || (param1 != parameter_##param1[currentUnit]) ||	     \
                  (param2 != parameter_##param2[currentUnit])		     \
       || FORCE_STATE_CHANGE)						     \
    {									     \
      ActivateTU ();							     \
      parameter_##param1[currentUnit] = param1;				     \
      parameter_##param2[currentUnit] = param2;				     \
      func (param1, param2);						     \
    }									     \
  }									     \
  void Get##name (type1 &param1, type2 & param2) const			     \
  {									     \
    param1 = parameter_##param1[currentUnit];				     \
    param2 = parameter_##param2[currentUnit];				     \
  }


#define IMPLEMENT_CACHED_PARAMETER_3_LAYER(func, name, type1, param1,	     \
  type2, param2, type3, param3)						     \
  type1 parameter_##param1[CS_GL_MAX_LAYER];				     \
  type2 parameter_##param2[CS_GL_MAX_LAYER];				     \
  type3 parameter_##param3[CS_GL_MAX_LAYER];				     \
  void Set##name (type1 param1, type2 param2, type3 param3,		     \
    bool forced = false)						     \
  {									     \
    if (forced || (param1 != parameter_##param1[currentUnit]) ||	     \
                  (param2 != parameter_##param2[currentUnit]) ||	     \
                  (param3 != parameter_##param3[currentUnit]		     \
       || FORCE_STATE_CHANGE))						     \
    {									     \
      ActivateTU ();							     \
      parameter_##param1[currentUnit] = param1;				     \
      parameter_##param2[currentUnit] = param2;				     \
      parameter_##param3[currentUnit] = param3;				     \
      func (param1, param2, param3);					     \
    }									     \
  }									     \
  void Get##name (type1 &param1, type2 & param2, type3 & param3) const	     \
  {									     \
    param1 = parameter_##param1[currentUnit];				     \
    param2 = parameter_##param2[currentUnit];				     \
    param3 = parameter_##param3[currentUnit];				     \
  }


#define IMPLEMENT_CACHED_PARAMETER_4_LAYER(func, name, type1, param1,	     \
    type2, param2, type3, param3, type4, param4)			     \
  type1 parameter_##param1[CS_GL_MAX_LAYER];				     \
  type2 parameter_##param2[CS_GL_MAX_LAYER];				     \
  type3 parameter_##param3[CS_GL_MAX_LAYER];				     \
  type4 parameter_##param4[CS_GL_MAX_LAYER];				     \
  void Set##name (type1 param1, type2 param2, type3 param3, type4 param4,    \
    bool forced = false)						     \
  {									     \
    if (forced || (param1 != parameter_##param1[currentUnit]) ||	     \
                  (param2 != parameter_##param2[currentUnit]) ||	     \
                  (param3 != parameter_##param3[currentUnit]) ||	     \
                  (param4 != parameter_##param4[currentUnit])		     \
       || FORCE_STATE_CHANGE)						     \
    {									     \
      ActivateTU ();							     \
      parameter_##param1[currentUnit] = param1;				     \
      parameter_##param2[currentUnit] = param2;				     \
      parameter_##param3[currentUnit] = param3;				     \
      parameter_##param4[currentUnit] = param4;				     \
      func (param1, param2, param3, param4);				     \
    }									     \
  }									     \
  void Get##name (type1 &param1, type2 & param2, type3 & param3, type4& param4) const\
  {									     \
    param1 = parameter_##param1[currentUnit];				     \
    param2 = parameter_##param2[currentUnit];				     \
    param3 = parameter_##param3[currentUnit];				     \
    param4 = parameter_##param4[currentUnit];				     \
  }


/**
 * Since this class is passed directly between plugins the
 * code in this class cannot do memory allocations or
 * deallocations. The functions in this class will only
 * manipulate member variables.
 */
class csGLStateCache
{
public:
  csGLExtensionManager* extmgr;

  csGLStateCache (csGLExtensionManager* extmgr)
  {
    csGLStateCache::extmgr = extmgr;
  }
  
  /// Init cache
  void InitCache()
  {
    int i;
    glGetIntegerv (GL_ALPHA_TEST_FUNC, (GLint*)&parameter_alpha_func);
    glGetFloatv (GL_ALPHA_TEST_REF, &parameter_alpha_ref);
    glGetIntegerv (GL_BLEND_SRC, (GLint*)&parameter_blend_source);
    glGetIntegerv (GL_BLEND_DST, (GLint*)&parameter_blend_destination);
    glGetIntegerv (GL_CULL_FACE_MODE, (GLint*)&parameter_cull_mode);
    glGetIntegerv (GL_DEPTH_FUNC, (GLint*)&parameter_depth_func);
    glGetBooleanv (GL_DEPTH_WRITEMASK, &parameter_depth_mask);
    glGetIntegerv (GL_SHADE_MODEL, (GLint*)&parameter_shade_model);
    glGetIntegerv (GL_STENCIL_BITS, (GLint*)&parameter_maskl);
    glGetIntegerv (GL_STENCIL_FUNC, (GLint*)&parameter_stencil_func);
    glGetIntegerv (GL_STENCIL_VALUE_MASK, (GLint*)&parameter_stencil_mask);
    glGetIntegerv (GL_STENCIL_REF, &parameter_stencil_ref);
    glGetIntegerv (GL_STENCIL_FAIL, (GLint*)&parameter_stencil_fail);
    glGetIntegerv (GL_STENCIL_PASS_DEPTH_FAIL, (GLint*)&parameter_stencil_zfail);
    glGetIntegerv (GL_STENCIL_PASS_DEPTH_PASS, (GLint*)&parameter_stencil_zpass);
    glGetIntegerv (GL_MATRIX_MODE, (GLint*)&parameter_matrixMode);
    GLboolean writemask[4];
    glGetBooleanv (GL_COLOR_WRITEMASK, writemask);
    parameter_wmRed = writemask[0];
    parameter_wmGreen = writemask[1];
    parameter_wmBlue = writemask[2];
    parameter_wmAlpha = writemask[3];
    enabled_GL_DEPTH_TEST = glIsEnabled (GL_DEPTH_TEST);
    enabled_GL_BLEND = glIsEnabled (GL_BLEND);
    enabled_GL_DITHER = glIsEnabled (GL_DITHER);
    enabled_GL_STENCIL_TEST = glIsEnabled (GL_STENCIL_TEST);
    enabled_GL_CULL_FACE = glIsEnabled (GL_CULL_FACE);
    enabled_GL_POLYGON_OFFSET_FILL = glIsEnabled (GL_POLYGON_OFFSET_FILL);
    enabled_GL_LIGHTING = glIsEnabled (GL_LIGHTING);
    enabled_GL_ALPHA_TEST = glIsEnabled (GL_ALPHA_TEST);
    enabled_GL_TEXTURE_GEN_S = glIsEnabled (GL_TEXTURE_GEN_S);
    enabled_GL_TEXTURE_GEN_T = glIsEnabled (GL_TEXTURE_GEN_T);
    enabled_GL_TEXTURE_GEN_R = glIsEnabled (GL_TEXTURE_GEN_R);
    enabled_GL_FOG = glIsEnabled (GL_FOG);
    enabled_GL_TEXTURE_1D[0] = glIsEnabled (GL_TEXTURE_1D);
    enabled_GL_TEXTURE_2D[0] = glIsEnabled (GL_TEXTURE_2D);
    enabled_GL_TEXTURE_3D[0] = glIsEnabled (GL_TEXTURE_3D);
    enabled_GL_TEXTURE_CUBE_MAP[0] = glIsEnabled (GL_TEXTURE_CUBE_MAP);
    enabled_GL_TEXTURE_COORD_ARRAY[0] = glIsEnabled (GL_TEXTURE_COORD_ARRAY);
    for (i = 1 ; i < CS_GL_MAX_LAYER; i++)
    {
      enabled_GL_TEXTURE_1D[i] = enabled_GL_TEXTURE_1D[0];
      enabled_GL_TEXTURE_2D[i] = enabled_GL_TEXTURE_2D[0];
      enabled_GL_TEXTURE_3D[i] = enabled_GL_TEXTURE_3D[0];
      enabled_GL_TEXTURE_CUBE_MAP[i] = enabled_GL_TEXTURE_CUBE_MAP[0];
      enabled_GL_TEXTURE_COORD_ARRAY[i] = enabled_GL_TEXTURE_COORD_ARRAY[0];
    }
    enabled_GL_SCISSOR_TEST = glIsEnabled (GL_SCISSOR_TEST);
    enabled_GL_VERTEX_ARRAY = glIsEnabled (GL_VERTEX_ARRAY);
    enabled_GL_COLOR_ARRAY = glIsEnabled (GL_COLOR_ARRAY);
    enabled_GL_NORMAL_ARRAY = glIsEnabled (GL_NORMAL_ARRAY);

    memset (boundtexture, 0, CS_GL_MAX_LAYER * sizeof (GLuint));
    currentUnit = 0;
    activeUnit = 0;
    currentBufferID = 0;
    currentIndexID = 0;
    
    glGetIntegerv (GL_VERTEX_ARRAY_SIZE, (GLint*)&parameter_vsize);
    glGetIntegerv (GL_VERTEX_ARRAY_STRIDE, (GLint*)&parameter_vstride);
    glGetIntegerv (GL_VERTEX_ARRAY_TYPE, (GLint*)&parameter_vtype);
    glGetPointerv (GL_VERTEX_ARRAY_POINTER, &parameter_vpointer);

    glGetIntegerv (GL_NORMAL_ARRAY_STRIDE, (GLint*)&parameter_nstride);
    glGetIntegerv (GL_NORMAL_ARRAY_TYPE, (GLint*)&parameter_ntype);
    glGetPointerv (GL_NORMAL_ARRAY_POINTER, &parameter_npointer);

    glGetIntegerv (GL_COLOR_ARRAY_SIZE, (GLint*)&parameter_csize);
    glGetIntegerv (GL_COLOR_ARRAY_STRIDE, (GLint*)&parameter_cstride);
    glGetIntegerv (GL_COLOR_ARRAY_TYPE, (GLint*)&parameter_ctype);
    glGetPointerv (GL_COLOR_ARRAY_POINTER, &parameter_cpointer);
  }

  // Standardized caches
  IMPLEMENT_CACHED_BOOL (GL_DEPTH_TEST)
  IMPLEMENT_CACHED_BOOL (GL_BLEND)
  IMPLEMENT_CACHED_BOOL (GL_DITHER)
  IMPLEMENT_CACHED_BOOL (GL_STENCIL_TEST)
  IMPLEMENT_CACHED_BOOL (GL_CULL_FACE)
  IMPLEMENT_CACHED_BOOL (GL_POLYGON_OFFSET_FILL)
  IMPLEMENT_CACHED_BOOL (GL_LIGHTING)
  IMPLEMENT_CACHED_BOOL (GL_ALPHA_TEST)
  IMPLEMENT_CACHED_BOOL (GL_SCISSOR_TEST)
  IMPLEMENT_CACHED_BOOL (GL_TEXTURE_GEN_S)
  IMPLEMENT_CACHED_BOOL (GL_TEXTURE_GEN_T)
  IMPLEMENT_CACHED_BOOL (GL_TEXTURE_GEN_R)
  IMPLEMENT_CACHED_BOOL (GL_FOG)
  IMPLEMENT_CACHED_BOOL_CURRENTLAYER (GL_TEXTURE_1D)
  IMPLEMENT_CACHED_BOOL_CURRENTLAYER (GL_TEXTURE_2D)
  IMPLEMENT_CACHED_BOOL_CURRENTLAYER (GL_TEXTURE_3D)
  IMPLEMENT_CACHED_BOOL_CURRENTLAYER (GL_TEXTURE_CUBE_MAP)
  IMPLEMENT_CACHED_PARAMETER_2 (glAlphaFunc, AlphaFunc, GLenum, alpha_func, GLclampf, alpha_ref)
  IMPLEMENT_CACHED_PARAMETER_2 (glBlendFunc, BlendFunc, GLenum, blend_source, GLenum, blend_destination)
  IMPLEMENT_CACHED_PARAMETER_1 (glCullFace, CullFace, GLenum, cull_mode)
  IMPLEMENT_CACHED_PARAMETER_1 (glDepthFunc, DepthFunc, GLenum, depth_func)
  IMPLEMENT_CACHED_PARAMETER_1 (glDepthMask, DepthMask, GLboolean, depth_mask)
  IMPLEMENT_CACHED_PARAMETER_1 (glShadeModel, ShadeModel, GLenum, shade_model)
  IMPLEMENT_CACHED_PARAMETER_3 (glStencilFunc, StencilFunc, GLenum, stencil_func, GLint, stencil_ref, GLuint, stencil_mask)
  IMPLEMENT_CACHED_PARAMETER_3 (glStencilOp, StencilOp, GLenum, stencil_fail, GLenum, stencil_zfail, GLenum, stencil_zpass)
  IMPLEMENT_CACHED_PARAMETER_1 (glStencilMask, StencilMask, GLuint, maskl)
  IMPLEMENT_CACHED_PARAMETER_4 (glColorMask, ColorMask, GLboolean, wmRed, \
    GLboolean, wmGreen, GLboolean, wmBlue, GLboolean, wmAlpha)

  IMPLEMENT_CACHED_CLIENT_STATE (GL_VERTEX_ARRAY)
  IMPLEMENT_CACHED_CLIENT_STATE (GL_COLOR_ARRAY)
  IMPLEMENT_CACHED_CLIENT_STATE (GL_NORMAL_ARRAY)
  IMPLEMENT_CACHED_CLIENT_STATE_LAYER (GL_TEXTURE_COORD_ARRAY)

  IMPLEMENT_CACHED_PARAMETER_1 (glMatrixMode, MatrixMode, GLenum, matrixMode)
  
  IMPLEMENT_CACHED_PARAMETER_4 (glVertexPointer, VertexPointer, GLint, vsize,
    GLenum, vtype, GLsizei, vstride, GLvoid*, vpointer);
  IMPLEMENT_CACHED_PARAMETER_3 (glNormalPointer, NormalPointer, GLenum, ntype,
    GLsizei, nstride, GLvoid*, npointer);
  IMPLEMENT_CACHED_PARAMETER_4 (glColorPointer, ColorPointer, GLint, csize,
    GLenum, ctype, GLsizei, cstride, GLvoid*, cpointer);
  IMPLEMENT_CACHED_PARAMETER_4_LAYER (glTexCoordPointer, TexCoordPointer, GLint, tsize,
    GLenum, ttype, GLsizei, tstride, GLvoid*, tpointer);
  
  // Special caches
  GLuint boundtexture[CS_GL_MAX_LAYER]; // 32 max texture layers
  int currentUnit, activeUnit;
  void SetTexture (GLenum target, GLuint texture)
  {
    if (texture != boundtexture[currentUnit])
    {
      ActivateTU ();
      boundtexture[currentUnit] = texture;
      glBindTexture (target, texture);
    }
  }
  GLuint GetTexture (GLenum target)
  {
    return boundtexture[currentUnit];
  }
  GLuint GetTexture (GLenum target, int unit)
  {
    return boundtexture[unit];
  }
  /**
   * Set active texture unit. Doesn't check whether the multitexture ext is
   * actually supported, this must be done in calling code.
   */
  void SetActiveTU (int unit)
  {
    currentUnit = unit;   
  }
  int GetActiveTU ()
  {
    return currentUnit;
  }
  void ActivateTU ()
  {
    if (activeUnit != currentUnit && extmgr->CS_GL_ARB_multitexture)
    {
      extmgr->glActiveTextureARB (GL_TEXTURE0_ARB + currentUnit);
      extmgr->glClientActiveTextureARB (GL_TEXTURE0_ARB + currentUnit);
    }
    activeUnit = currentUnit;
  }

  //VBO buffers
  GLuint currentBufferID, currentIndexID;
  void SetBufferARB (GLenum target, GLuint id)
  {
    if (target == GL_ELEMENT_ARRAY_BUFFER_ARB)
    {
      if (id != currentIndexID)
      {
        extmgr->glBindBufferARB (target, id);
        currentIndexID = id;
      }
    } 
    else 
    {
      if (id != currentBufferID)
      {
        extmgr->glBindBufferARB (target, id);
        currentBufferID = id;
        parameter_vpointer = (GLvoid*)~0; //invalidate vertexpointer
        parameter_npointer = (GLvoid*)~0; //invalidate vertexpointer
        parameter_cpointer = (GLvoid*)~0; //invalidate vertexpointer
        memset(&parameter_tpointer, ~0, sizeof(GLvoid*)*CS_GL_MAX_LAYER);
      }
    }
  }

  GLuint GetBufferARB (GLenum target)
  {
    if (target == GL_ELEMENT_ARRAY_BUFFER_ARB)
    {
      return currentIndexID;
    } 
    else 
    {
      return currentBufferID;
    }
  }
};

#undef IMPLEMENT_CACHED_BOOL
#undef IMPLEMENT_CACHED_BOOL_CURRENTLAYER
#undef IMPLEMENT_CACHED_PARAMETER_1
#undef IMPLEMENT_CACHED_PARAMETER_2
#undef IMPLEMENT_CACHED_PARAMETER_3

#undef FORCE_STATE_CHANGE

#endif // __CS_GLSTATES_H__
