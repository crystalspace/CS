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

#include "csplugincommon/opengl/glstates.h"

csGLStateCacheContext::csGLStateCacheContext (csGLExtensionManager* extmgr)
{
  csGLStateCacheContext::extmgr = extmgr;

  // Need to init exts here b/c we need the image units count
  extmgr->InitGL_ARB_multitexture ();
  extmgr->InitGL_ARB_fragment_program ();
  
  numTexCoords = numImageUnits = 1;
  if (extmgr->CS_GL_ARB_fragment_program)
  {
    glGetIntegerv (GL_MAX_TEXTURE_IMAGE_UNITS_ARB, &numImageUnits);
    glGetIntegerv (GL_MAX_TEXTURE_COORDS_ARB, &numTexCoords);
  }
  else if (extmgr->CS_GL_ARB_multitexture)
  {
    glGetIntegerv (GL_MAX_TEXTURE_UNITS_ARB, &numImageUnits);
    numTexCoords = numImageUnits;
  }
  
  boundtexture.Setup (numImageUnits);
  enabled_GL_TEXTURE_1D.Setup (numImageUnits);
  enabled_GL_TEXTURE_2D.Setup (numImageUnits);
  enabled_GL_TEXTURE_3D.Setup (numImageUnits);
  enabled_GL_TEXTURE_CUBE_MAP.Setup (numImageUnits);
  enabled_GL_TEXTURE_RECTANGLE_ARB.Setup (numImageUnits);
  
  enabled_GL_TEXTURE_COORD_ARRAY.Setup (numTexCoords);
  parameter_tsize.Setup (numTexCoords);
  parameter_ttype.Setup (numTexCoords);
  parameter_tstride.Setup (numTexCoords);
  parameter_tpointer.Setup (numTexCoords);
  parameter_tvbo.Setup (numTexCoords);
  
  memset (activeBufferID, 0, sizeof (activeBufferID));
  
  clampState[clampVertex] = GL_TRUE;
  clampState[clampFragment] = GL_FIXED_ONLY_ARB;
  clampState[clampRead] = GL_FIXED_ONLY_ARB;
}

void csGLStateCacheContext::InitCache()
{
  int i;
  glGetIntegerv (GL_ALPHA_TEST_FUNC, (GLint*)&parameter_alpha_func);
  glGetFloatv (GL_ALPHA_TEST_REF, &parameter_alpha_ref);
  if (extmgr->CS_GL_EXT_blend_func_separate)
  {
    glGetIntegerv (GL_BLEND_SRC_RGB_EXT, (GLint*)&blend_sourceRGB);
    glGetIntegerv (GL_BLEND_SRC_ALPHA_EXT, (GLint*)&blend_sourceA);
    glGetIntegerv (GL_BLEND_DST_RGB_EXT, (GLint*)&blend_destinationRGB);
    glGetIntegerv (GL_BLEND_DST_ALPHA_EXT, (GLint*)&blend_destinationA);
  }
  else
  {
    glGetIntegerv (GL_BLEND_SRC, (GLint*)&blend_sourceRGB);
    blend_sourceA = blend_sourceRGB;
    glGetIntegerv (GL_BLEND_DST, (GLint*)&blend_destinationRGB);
    blend_destinationA = blend_destinationRGB;
  }
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
  enabled_GL_DEPTH_TEST = (glIsEnabled (GL_DEPTH_TEST) == GL_TRUE);
  enabled_GL_BLEND = (glIsEnabled (GL_BLEND) == GL_TRUE);
  enabled_GL_DITHER = (glIsEnabled (GL_DITHER) == GL_TRUE);
  enabled_GL_STENCIL_TEST = (glIsEnabled (GL_STENCIL_TEST) == GL_TRUE);
  enabled_GL_CULL_FACE = (glIsEnabled (GL_CULL_FACE) == GL_TRUE);
  enabled_GL_POLYGON_OFFSET_FILL = (glIsEnabled (GL_POLYGON_OFFSET_FILL) == GL_TRUE);
  enabled_GL_LIGHTING = (glIsEnabled (GL_LIGHTING) == GL_TRUE);
  enabled_GL_ALPHA_TEST = (glIsEnabled (GL_ALPHA_TEST) == GL_TRUE);
  enabled_GL_TEXTURE_GEN_S = (glIsEnabled (GL_TEXTURE_GEN_S) == GL_TRUE);
  enabled_GL_TEXTURE_GEN_T = (glIsEnabled (GL_TEXTURE_GEN_T) == GL_TRUE);
  enabled_GL_TEXTURE_GEN_R = (glIsEnabled (GL_TEXTURE_GEN_R) == GL_TRUE);
  enabled_GL_TEXTURE_GEN_Q = (glIsEnabled (GL_TEXTURE_GEN_Q) == GL_TRUE);
  enabled_GL_FOG = (glIsEnabled (GL_FOG) == GL_TRUE);

  memset (boundtexture.p, 0, numImageUnits * sizeof (GLuint));
  currentImageUnit = 0;
  currentTCUnit = 0;
  memset (activeUnit, 0, sizeof (activeUnit));
  if (extmgr->CS_GL_ARB_multitexture)
  {
    for (i = numImageUnits; i-- > 0; )
    {
      extmgr->glActiveTextureARB (GL_TEXTURE0_ARB + i);
      enabled_GL_TEXTURE_1D[i] = (glIsEnabled (GL_TEXTURE_1D) == GL_TRUE);
      enabled_GL_TEXTURE_2D[i] = (glIsEnabled (GL_TEXTURE_2D) == GL_TRUE);
      enabled_GL_TEXTURE_3D[i] = (glIsEnabled (GL_TEXTURE_3D) == GL_TRUE);
      enabled_GL_TEXTURE_CUBE_MAP[i] = (glIsEnabled (GL_TEXTURE_CUBE_MAP) == GL_TRUE);
	if (extmgr->CS_GL_ARB_texture_rectangle
	  || extmgr->CS_GL_EXT_texture_rectangle
	  || extmgr->CS_GL_NV_texture_rectangle)
	  enabled_GL_TEXTURE_RECTANGLE_ARB[i] = (glIsEnabled (GL_TEXTURE_RECTANGLE_ARB) == GL_TRUE);
	else
	  enabled_GL_TEXTURE_RECTANGLE_ARB[i] = false;
    }
    for (i = numTexCoords; i-- > 0; )
    {
      extmgr->glClientActiveTextureARB (GL_TEXTURE0_ARB + i);
      enabled_GL_TEXTURE_COORD_ARRAY[i] = (glIsEnabled (GL_TEXTURE_COORD_ARRAY) == GL_TRUE);
      glGetIntegerv (GL_TEXTURE_COORD_ARRAY_SIZE, (GLint*)&parameter_tsize[i]);
      glGetIntegerv (GL_TEXTURE_COORD_ARRAY_STRIDE, (GLint*)&parameter_tstride[i]);
      glGetIntegerv (GL_TEXTURE_COORD_ARRAY_TYPE, (GLint*)&parameter_ttype[i]);
      glGetPointerv (GL_TEXTURE_COORD_ARRAY_POINTER, &parameter_tpointer[i]);
	if (extmgr->CS_GL_ARB_vertex_buffer_object)
	  glGetIntegerv (GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB,
	    (GLint*)&parameter_tvbo[i]);
    }
  } 
  else 
  {
    enabled_GL_TEXTURE_1D[0] = (glIsEnabled (GL_TEXTURE_1D) == GL_TRUE);
    enabled_GL_TEXTURE_2D[0] = (glIsEnabled (GL_TEXTURE_2D) == GL_TRUE);
    enabled_GL_TEXTURE_3D[0] = (glIsEnabled (GL_TEXTURE_3D) == GL_TRUE);
    enabled_GL_TEXTURE_CUBE_MAP[0] = (glIsEnabled (GL_TEXTURE_CUBE_MAP) == GL_TRUE);
    enabled_GL_TEXTURE_COORD_ARRAY[0] = (glIsEnabled (GL_TEXTURE_COORD_ARRAY) == GL_TRUE);
	if (extmgr->CS_GL_ARB_texture_rectangle
	  || extmgr->CS_GL_EXT_texture_rectangle
	  || extmgr->CS_GL_NV_texture_rectangle)
	  enabled_GL_TEXTURE_RECTANGLE_ARB[0] = (glIsEnabled (GL_TEXTURE_RECTANGLE_ARB) == GL_TRUE);
	else
	  enabled_GL_TEXTURE_RECTANGLE_ARB[0] = false;
    glGetIntegerv (GL_TEXTURE_COORD_ARRAY_SIZE, (GLint*)&parameter_tsize[0]);
    glGetIntegerv (GL_TEXTURE_COORD_ARRAY_STRIDE, (GLint*)&parameter_tstride[0]);
    glGetIntegerv (GL_TEXTURE_COORD_ARRAY_TYPE, (GLint*)&parameter_ttype[0]);
    glGetPointerv (GL_TEXTURE_COORD_ARRAY_POINTER, &parameter_tpointer[0]);
    if (extmgr->CS_GL_ARB_vertex_buffer_object)
	glGetIntegerv (GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB,
	  (GLint*)&parameter_tvbo[0]);
    for (i = 1 ; i < numImageUnits; i++)
    {
      enabled_GL_TEXTURE_1D[i] = enabled_GL_TEXTURE_1D[0];
      enabled_GL_TEXTURE_2D[i] = enabled_GL_TEXTURE_2D[0];
      enabled_GL_TEXTURE_3D[i] = enabled_GL_TEXTURE_3D[0];
      enabled_GL_TEXTURE_CUBE_MAP[i] = enabled_GL_TEXTURE_CUBE_MAP[0];
      enabled_GL_TEXTURE_COORD_ARRAY[i] = enabled_GL_TEXTURE_COORD_ARRAY[0];
	enabled_GL_TEXTURE_RECTANGLE_ARB[i] = enabled_GL_TEXTURE_RECTANGLE_ARB[0];
	parameter_tsize[i] = parameter_tsize[0];
	parameter_tstride[i] = parameter_tstride[0];
	parameter_ttype[i] = parameter_ttype[0];
	parameter_tpointer[i] = parameter_tpointer[0];
    }
  }
  enabled_GL_SCISSOR_TEST = (glIsEnabled (GL_SCISSOR_TEST) == GL_TRUE);
  enabled_GL_VERTEX_ARRAY = (glIsEnabled (GL_VERTEX_ARRAY) == GL_TRUE);
  enabled_GL_COLOR_ARRAY = (glIsEnabled (GL_COLOR_ARRAY) == GL_TRUE);
  if (extmgr->CS_GL_EXT_secondary_color)
    enabled_GL_SECONDARY_COLOR_ARRAY_EXT = 
      (glIsEnabled (GL_SECONDARY_COLOR_ARRAY_EXT) == GL_TRUE);
  else
    enabled_GL_SECONDARY_COLOR_ARRAY_EXT = false;
  enabled_GL_NORMAL_ARRAY = (glIsEnabled (GL_NORMAL_ARRAY) == GL_TRUE);
  
  if (extmgr->CS_GL_ARB_vertex_program)
    enabled_GL_VERTEX_PROGRAM_POINT_SIZE_ARB =
      (glIsEnabled (GL_VERTEX_PROGRAM_POINT_SIZE_ARB) == GL_TRUE);
  else
    enabled_GL_VERTEX_PROGRAM_POINT_SIZE_ARB = false;
  if (extmgr->CS_GL_ARB_point_sprite)
    enabled_GL_POINT_SPRITE_ARB =
      (glIsEnabled (GL_POINT_SPRITE_ARB) == GL_TRUE);
  else
    enabled_GL_POINT_SPRITE_ARB = false;
  if (extmgr->CS_GL_ARB_seamless_cube_map)
    enabled_GL_TEXTURE_CUBE_MAP_SEAMLESS =
      (glIsEnabled (GL_TEXTURE_CUBE_MAP_SEAMLESS) == GL_TRUE);
  else
    enabled_GL_TEXTURE_CUBE_MAP_SEAMLESS = false;

  if (extmgr->CS_GL_ARB_multisample)
  {
    enabled_GL_SAMPLE_ALPHA_TO_COVERAGE_ARB =
      (glIsEnabled (GL_SAMPLE_ALPHA_TO_COVERAGE_ARB) == GL_TRUE);
    enabled_GL_SAMPLE_ALPHA_TO_ONE_ARB =
      (glIsEnabled (GL_SAMPLE_ALPHA_TO_ONE_ARB) == GL_TRUE);
  }
  else
  {
    enabled_GL_SAMPLE_ALPHA_TO_ONE_ARB = false;
  }
  
  memset (currentBufferID, 0, sizeof (currentBufferID));
  {
    enum { extVBO = 1, extPBO = 2 };
    static const GLenum requiredExt[boCount] =
    { extVBO, extVBO, extPBO, extPBO };
    
    int boExt = 0;
    if (extmgr->CS_GL_ARB_vertex_buffer_object) boExt |= extVBO;
    if (extmgr->CS_GL_ARB_pixel_buffer_object) boExt |= extPBO;
    for (int b = 0; b < boCount; b++)
    {
	if (requiredExt[b] & boExt)
	{
	  static const GLenum localIndexToGLBufferBinding[boCount] =
	  { GL_ARRAY_BUFFER_BINDING_ARB, GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB, 
	    GL_PIXEL_PACK_BUFFER_BINDING_ARB, GL_PIXEL_UNPACK_BUFFER_BINDING_ARB };
	  glGetIntegerv (localIndexToGLBufferBinding[b], 
	    (GLint*)&activeBufferID[b]);
	}
    }
  }

  memset (currentBufferID, 0, sizeof (currentBufferID));
  {
    static const GLenum localIndexToGLBufferBinding[boCount] =
    { GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB, GL_ARRAY_BUFFER_BINDING_ARB, 
	GL_PIXEL_PACK_BUFFER_BINDING_ARB, GL_PIXEL_UNPACK_BUFFER_BINDING_ARB };

    enum { extVBO = 1, extPBO = 2 };
    static const GLenum requiredExt[boCount] =
    { extVBO, extVBO, extPBO, extPBO };
    
    int boExt = 0;
    if (extmgr->CS_GL_ARB_vertex_buffer_object) boExt |= extVBO;
    if (extmgr->CS_GL_ARB_pixel_buffer_object) boExt |= extPBO;
    for (int b = 0; b < boCount; b++)
    {
	if (requiredExt[b] & boExt)
	{
	  glGetIntegerv (localIndexToGLBufferBinding[b], 
	    (GLint*)&currentBufferID[b]);
	}
    }
  }

  glGetIntegerv (GL_VERTEX_ARRAY_SIZE, (GLint*)&parameter_vsize);
  glGetIntegerv (GL_VERTEX_ARRAY_STRIDE, (GLint*)&parameter_vstride);
  glGetIntegerv (GL_VERTEX_ARRAY_TYPE, (GLint*)&parameter_vtype);
  glGetPointerv (GL_VERTEX_ARRAY_POINTER, &parameter_vpointer);
  if (extmgr->CS_GL_ARB_vertex_buffer_object)
    glGetIntegerv (GL_VERTEX_ARRAY_BUFFER_BINDING_ARB, (GLint*)&parameter_vvbo);

  glGetIntegerv (GL_NORMAL_ARRAY_STRIDE, (GLint*)&parameter_nstride);
  glGetIntegerv (GL_NORMAL_ARRAY_TYPE, (GLint*)&parameter_ntype);
  glGetPointerv (GL_NORMAL_ARRAY_POINTER, &parameter_npointer);
  if (extmgr->CS_GL_ARB_vertex_buffer_object)
    glGetIntegerv (GL_NORMAL_ARRAY_BUFFER_BINDING_ARB, (GLint*)&parameter_nvbo);

  glGetIntegerv (GL_COLOR_ARRAY_SIZE, (GLint*)&parameter_csize);
  glGetIntegerv (GL_COLOR_ARRAY_STRIDE, (GLint*)&parameter_cstride);
  glGetIntegerv (GL_COLOR_ARRAY_TYPE, (GLint*)&parameter_ctype);
  glGetPointerv (GL_COLOR_ARRAY_POINTER, &parameter_cpointer);
  if (extmgr->CS_GL_ARB_vertex_buffer_object)
    glGetIntegerv (GL_COLOR_ARRAY_BUFFER_BINDING_ARB, (GLint*)&parameter_cvbo);
  
  if (extmgr->CS_GL_EXT_secondary_color)
  {
    glGetIntegerv (GL_SECONDARY_COLOR_ARRAY_SIZE_EXT, 
      (GLint*)&parameter_scsize);
    glGetIntegerv (GL_SECONDARY_COLOR_ARRAY_STRIDE_EXT, 
      (GLint*)&parameter_scstride);
    glGetIntegerv (GL_SECONDARY_COLOR_ARRAY_TYPE_EXT, 
      (GLint*)&parameter_sctype);
    glGetPointerv (GL_SECONDARY_COLOR_ARRAY_POINTER_EXT, 
      &parameter_scpointer);
    if (extmgr->CS_GL_ARB_vertex_buffer_object)
	glGetIntegerv (GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB,
	  (GLint*)&parameter_scvbo);
    enabled_GL_COLOR_SUM_EXT = glIsEnabled (GL_COLOR_SUM_EXT) != GL_FALSE;
  }
  else
  {
    parameter_scsize = 0;
    parameter_scstride = 0;
    parameter_sctype = 0;
    parameter_scpointer = 0;
    enabled_GL_COLOR_SUM_EXT = false;
  }
  
  glGetIntegerv (GL_UNPACK_ALIGNMENT, &pixelUnpackAlignment);
  GLint v;
  glGetIntegerv (GL_UNPACK_SWAP_BYTES, &v);
  pixelUnpackSwapBytes = v != 0;
  
  if (extmgr->CS_GL_ARB_color_buffer_float)
  {
    GLint clampState;
    glGetIntegerv (GL_CLAMP_VERTEX_COLOR_ARB, &clampState);
    this->clampState[clampVertex] = (GLenum)clampState;
    glGetIntegerv (GL_CLAMP_FRAGMENT_COLOR_ARB, &clampState);
    this->clampState[clampFragment] = (GLenum)clampState;
    glGetIntegerv (GL_CLAMP_READ_COLOR_ARB, &clampState);
    this->clampState[clampRead] = (GLenum)clampState;
  }
}
