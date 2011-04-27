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

/**\file
 * OpenGL state cache.
 */

#if defined(CS_OPENGL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENGL_PATH,gl.h)
#else
#include <GL/gl.h>
#endif

#include "csextern_gl.h"
#include "csgeom/math.h"
#include "glextmanager.h"

/**\addtogroup plugincommon
 * @{ */

// Set to 'true' to force state changing commands. For debugging.
#define FORCE_STATE_CHANGE			  false/*true*/

#define DECLARE_CACHED_BOOL(name) \
  bool enabled_##name;

#define IMPLEMENT_CACHED_BOOL(name) \
  void Enable_##name () \
  { \
    if (!currentContext->enabled_##name || FORCE_STATE_CHANGE) \
    { \
      currentContext->enabled_##name = true;  \
      glEnable (name); \
    } \
  } \
  void Disable_##name () \
  { \
    if (currentContext->enabled_##name || FORCE_STATE_CHANGE) { \
      currentContext->enabled_##name = false;  \
      glDisable (name); \
    } \
  } \
  bool IsEnabled_##name () const \
  { \
    return currentContext->enabled_##name; \
  }

#define DECLARE_CACHED_BOOL_IMAGEUNIT(name) \
  AutoArray<bool> enabled_##name;

#define IMPLEMENT_CACHED_BOOL_IMAGEUNIT(name)                       \
  void Enable_##name ()                                                       \
  {                                                                           \
    const int currentUnit = currentContext->currentImageUnit;                 \
    if (!currentContext->enabled_##name[currentUnit] || FORCE_STATE_CHANGE)   \
    {                                                                         \
      ActivateImageUnit ();                                                   \
      currentContext->enabled_##name[currentUnit] = true;                     \
      glEnable (name);                                                        \
    }                                                                         \
  }                                                                           \
  void Disable_##name ()                                                      \
  {                                                                           \
    const int currentUnit = currentContext->currentImageUnit;                 \
    if (currentContext->enabled_##name[currentUnit] || FORCE_STATE_CHANGE)    \
    {                                                                         \
      ActivateImageUnit ();                                                   \
      currentContext->enabled_##name[currentUnit] = false;                    \
      glDisable (name);                                                       \
    }                                                                         \
  }                                                                           \
  bool IsEnabled_##name () const                                              \
  {                                                                           \
    const int currentUnit = currentContext->currentImageUnit;                 \
    return currentContext->enabled_##name[currentUnit];                       \
  }

#define DECLARE_CACHED_PARAMETER_1(func, name, type1, param1) \
  type1 parameter_##param1;

#define IMPLEMENT_CACHED_PARAMETER_1(func, name, type1, param1) \
  void Set##name (type1 param1, bool forced = false) \
  { \
    if (forced || (param1 != currentContext->parameter_##param1) || FORCE_STATE_CHANGE) \
    { \
      currentContext->parameter_##param1 = param1;  \
      func (param1); \
    } \
  } \
  void Get##name (type1 & param1) const\
  { \
    param1 = currentContext->parameter_##param1;  \
  }

#define DECLARE_CACHED_PARAMETER_2(func, name, type1, param1, type2, param2) \
  type1 parameter_##param1; \
  type2 parameter_##param2;

#define IMPLEMENT_CACHED_PARAMETER_2(func, name, type1, param1, type2, param2) \
  void Set##name (type1 param1, type2 param2, bool forced = false) \
  { \
    if (forced || (param1 != currentContext->parameter_##param1) || \
        (param2 != currentContext->parameter_##param2) || FORCE_STATE_CHANGE) \
    { \
      currentContext->parameter_##param1 = param1;  \
      currentContext->parameter_##param2 = param2;  \
      func (param1, param2); \
    } \
  } \
  void Get##name (type1 & param1, type2 & param2) const\
  { \
    param1 = currentContext->parameter_##param1;  \
    param2 = currentContext->parameter_##param2;  \
  }

#define DECLARE_CACHED_PARAMETER_3(func, name, type1, param1, type2, param2, type3, param3) \
  type1 parameter_##param1; \
  type2 parameter_##param2; \
  type3 parameter_##param3;

#define IMPLEMENT_CACHED_PARAMETER_3(func, name, type1, param1, type2, param2, type3, param3) \
  void Set##name (type1 param1, type2 param2, type3 param3, bool forced = false) \
  { \
    if (forced || (param1 != currentContext->parameter_##param1) \
        || (param2 != currentContext->parameter_##param2) \
        || (param3 != currentContext->parameter_##param3) || FORCE_STATE_CHANGE) \
    { \
      currentContext->parameter_##param1 = param1;  \
      currentContext->parameter_##param2 = param2;  \
      currentContext->parameter_##param3 = param3;  \
      func (param1, param2, param3); \
    } \
  } \
  void Get##name (type1 &param1, type2 & param2, type3 & param3) const\
  { \
    param1 = currentContext->parameter_##param1;  \
    param2 = currentContext->parameter_##param2;  \
    param3 = currentContext->parameter_##param3;  \
  }

#define DECLARE_CACHED_PARAMETER_3_BUF(func, name, type1, param1, type2, param2, type3, param3, vbo) \
  type1 parameter_##param1; \
  type2 parameter_##param2; \
  type3 parameter_##param3; \
  GLuint parameter_##vbo;

#define IMPLEMENT_CACHED_PARAMETER_3_BUF(func, name, type1, param1, type2, param2, type3, param3, vbo) \
  void Set##name (type1 param1, type2 param2, type3 param3, bool forced = false) \
  { \
    if (forced || (param1 != currentContext->parameter_##param1) \
        || (param2 != currentContext->parameter_##param2) \
        || (param3 != currentContext->parameter_##param3) \
        || (currentContext->currentBufferID[csGLStateCacheContext::boElementArray] \
          != currentContext->parameter_##vbo) \
        || FORCE_STATE_CHANGE) \
    { \
      currentContext->parameter_##param1 = param1;  \
      currentContext->parameter_##param2 = param2;  \
      currentContext->parameter_##param3 = param3;  \
      if (extmgr->CS_GL_ARB_vertex_buffer_object)                             \
      {                                                                       \
        ApplyBufferBinding (csGLStateCacheContext::boElementArray);           \
	currentContext->parameter_##vbo                                       \
	 = currentContext->currentBufferID[csGLStateCacheContext::boElementArray];\
      }                                                                       \
      func (param1, param2, param3); \
    } \
  } \
  void Get##name (type1 &param1, type2 & param2, type3 & param3) const\
  { \
    param1 = currentContext->parameter_##param1;  \
    param2 = currentContext->parameter_##param2;  \
    param3 = currentContext->parameter_##param3;  \
  }

#define DECLARE_CACHED_PARAMETER_4(func, name, type1, param1, \
  type2, param2, type3, param3, type4, param4) \
  type1 parameter_##param1; \
  type2 parameter_##param2; \
  type3 parameter_##param3; \
  type4 parameter_##param4;

#define IMPLEMENT_CACHED_PARAMETER_4(func, name, type1, param1, \
    type2, param2, type3, param3, type4, param4) \
  void Set##name (type1 param1, type2 param2, type3 param3, type4 param4, \
    bool forced = false) \
  { \
    if (forced || (param1 != currentContext->parameter_##param1) || \
      (param2 != currentContext->parameter_##param2) || \
      (param3 != currentContext->parameter_##param3) || \
      (param4 != currentContext->parameter_##param4) || FORCE_STATE_CHANGE) \
    { \
      currentContext->parameter_##param1 = param1;  \
      currentContext->parameter_##param2 = param2;  \
      currentContext->parameter_##param3 = param3;  \
      currentContext->parameter_##param4 = param4;  \
      func (param1, param2, param3, param4); \
    } \
  } \
  void Get##name (type1 &param1, type2 & param2, type3 & param3, type4& param4) const\
  { \
    param1 = currentContext->parameter_##param1;  \
    param2 = currentContext->parameter_##param2;  \
    param3 = currentContext->parameter_##param3;  \
    param4 = currentContext->parameter_##param4;  \
  }

#define DECLARE_CACHED_PARAMETER_4_BUF(func, name, type1, param1, \
  type2, param2, type3, param3, type4, param4, vbo) \
  type1 parameter_##param1; \
  type2 parameter_##param2; \
  type3 parameter_##param3; \
  type4 parameter_##param4; \
  GLuint parameter_##vbo;

#define IMPLEMENT_CACHED_PARAMETER_4_BUF(func, name, type1, param1, \
    type2, param2, type3, param3, type4, param4, vbo) \
  void Set##name (type1 param1, type2 param2, type3 param3, type4 param4, \
    bool forced = false) \
  { \
    if (forced || (param1 != currentContext->parameter_##param1) || \
      (param2 != currentContext->parameter_##param2) || \
      (param3 != currentContext->parameter_##param3) || \
      (param4 != currentContext->parameter_##param4) \
      || (currentContext->currentBufferID[csGLStateCacheContext::boElementArray] \
        != currentContext->parameter_##vbo) \
      || FORCE_STATE_CHANGE) \
    { \
      currentContext->parameter_##param1 = param1;  \
      currentContext->parameter_##param2 = param2;  \
      currentContext->parameter_##param3 = param3;  \
      currentContext->parameter_##param4 = param4;  \
      if (extmgr->CS_GL_ARB_vertex_buffer_object)                             \
      {                                                                       \
        ApplyBufferBinding (csGLStateCacheContext::boElementArray);           \
	currentContext->parameter_##vbo =                                     \
	  currentContext->currentBufferID[csGLStateCacheContext::boElementArray];\
      }                                                                       \
      func (param1, param2, param3, param4); \
    } \
  } \
  void Get##name (type1 &param1, type2 & param2, type3 & param3, type4& param4) const\
  { \
    param1 = currentContext->parameter_##param1;  \
    param2 = currentContext->parameter_##param2;  \
    param3 = currentContext->parameter_##param3;  \
    param4 = currentContext->parameter_##param4;  \
  }

#define DECLARE_CACHED_CLIENT_STATE(name)	      \
  bool enabled_##name;

#define IMPLEMENT_CACHED_CLIENT_STATE(name)	      \
  void Enable_##name () \
  { \
    if (!currentContext->enabled_##name || FORCE_STATE_CHANGE) \
    { \
      currentContext->enabled_##name = true;  \
      glEnableClientState (name); \
    } \
  } \
    void Disable_##name () \
  { \
    if (currentContext->enabled_##name || FORCE_STATE_CHANGE) { \
      currentContext->enabled_##name = false;  \
      glDisableClientState (name); \
    } \
  } \
  bool IsEnabled_##name () const \
  { \
    return currentContext->enabled_##name; \
  }

#define DECLARE_CACHED_CLIENT_STATE_TCS(name)	                              \
  AutoArray<bool> enabled_##name;

#define IMPLEMENT_CACHED_CLIENT_STATE_TCS(name)                      \
  void Enable_##name ()                                                       \
  {                                                                           \
    const int currentUnit = currentContext->currentTCUnit;                    \
    if (!currentContext->enabled_##name[currentUnit] || FORCE_STATE_CHANGE)   \
    {                                                                         \
      ActivateTCUnit (activateTexCoord);                                      \
      currentContext->enabled_##name[currentUnit] = true;                     \
      glEnableClientState (name);                                             \
    }                                                                         \
  }                                                                           \
  void Disable_##name ()                                                      \
  {                                                                           \
    const int currentUnit = currentContext->currentTCUnit;                    \
    if (currentContext->enabled_##name[currentUnit] || FORCE_STATE_CHANGE)    \
    {                                                                         \
      ActivateTCUnit (activateTexCoord);                                      \
      currentContext->enabled_##name[currentUnit] = false;                    \
      glDisableClientState (name);                                            \
    }                                                                         \
  }                                                                           \
  bool IsEnabled_##name () const                                              \
  {                                                                           \
    const int currentUnit = currentContext->currentTCUnit;                    \
    return currentContext->enabled_##name[currentUnit];                       \
  }

#define DECLARE_CACHED_PARAMETER_1_LAYER(func, name, type1, param1) \
  AutoArray<type1> parameter_##param1;

#define IMPLEMENT_CACHED_PARAMETER_1_LAYER(func, name, type1, param1, limit) \
  void Set##name (type1 param1, bool forced = false) \
  { \
    const int currentUnit = currentContext->currentUnit;                      \
    if (forced || \
        (param1 != currentContext->parameter_##param1[currentUnit] \
        || FORCE_STATE_CHANGE)) \
    { \
      ActivateTU (); \
      currentContext->parameter_##param1[currentUnit] = param1;  \
      func (param1); \
    } \
  } \
  void Get##name (type1 &param1) const\
  { \
    const int currentUnit = currentContext->currentUnit;                      \
    param1 = currentContext->parameter_##param1[currentUnit];  \
  }

#define DECLARE_CACHED_PARAMETER_2_LAYER(func, name, type1, param1, \
  type2, param2) \
  AutoArray<type1> parameter_##param1; \
  AutoArray<type2> parameter_##param2;

#define IMPLEMENT_CACHED_PARAMETER_2_LAYER(func, name, type1, param1, \
  type2, param2, limit) \
  void Set##name (type1 param1, type2 param2, bool forced = false) \
  { \
    const int currentUnit = currentContext->currentUnit;                      \
    if (forced || (param1 != currentContext->parameter_##param1[ \
                    currentUnit]) || \
                  (param2 != currentContext->parameter_##param2[ \
                    currentUnit]) \
               || FORCE_STATE_CHANGE) \
    { \
      ActivateTU (); \
      currentContext->parameter_##param1[currentUnit] = param1;  \
      currentContext->parameter_##param2[currentUnit] = param2;  \
      func (param1, param2); \
    } \
  } \
  void Get##name (type1 &param1, type2 & param2) const\
  { \
    const int currentUnit = currentContext->currentUnit;                      \
    param1 = currentContext->parameter_##param1[currentUnit];  \
    param2 = currentContext->parameter_##param2[currentUnit];  \
  }

#define DECLARE_CACHED_PARAMETER_3_LAYER(func, name, type1, param1, \
  type2, param2, type3, param3) \
  AutoArray<type1> parameter_##param1; \
  AutoArray<type2> parameter_##param2; \
  AutoArray<type3> parameter_##param3;


#define IMPLEMENT_CACHED_PARAMETER_3_LAYER(func, name, type1, param1, \
  type2, param2, type3, param3, limit) \
  void Set##name (type1 param1, type2 param2, type3 param3,\
    bool forced = false) \
  { \
    const int currentUnit = currentContext->currentUnit;                      \
    if (forced || (param1 != currentContext->parameter_##param1[ \
                    currentUnit]) || \
                  (param2 != currentContext->parameter_##param2[ \
                    currentUnit]) || \
                  (param3 != currentContext->parameter_##param3[ \
                    currentUnit]) \
               || FORCE_STATE_CHANGE) \
    { \
      ActivateTU (); \
      currentContext->parameter_##param1[currentUnit] = param1;  \
      currentContext->parameter_##param2[currentUnit] = param2;  \
      currentContext->parameter_##param3[currentUnit] = param3;  \
      func (param1, param2, param3); \
    } \
  } \
  void Get##name (type1 &param1, type2 & param2, type3 & param3) const\
  { \
    const int currentUnit = currentContext->currentUnit;                      \
    param1 = currentContext->parameter_##param1[currentUnit];  \
    param2 = currentContext->parameter_##param2[currentUnit];  \
    param3 = currentContext->parameter_##param3[currentUnit];  \
  }


#define DECLARE_CACHED_PARAMETER_4_BUF_TCS(func, name, type1, param1,       \
  type2, param2, type3, param3, type4, param4, vbo)                           \
  AutoArray<type1> parameter_##param1;                                        \
  AutoArray<type2> parameter_##param2;                                        \
  AutoArray<type3> parameter_##param3;                                        \
  AutoArray<type4> parameter_##param4;                                        \
  AutoArray<GLuint> parameter_##vbo;

#define IMPLEMENT_CACHED_PARAMETER_4_BUF_TCS(func, name, type1, param1,     \
    type2, param2, type3, param3, type4, param4, vbo)                  \
  void Set##name (type1 param1, type2 param2, type3 param3, type4 param4,     \
    bool forced = false)                                                      \
  {                                                                           \
    const int currentUnit = currentContext->currentTCUnit;                    \
    if (forced                                                                \
      || (param1 != currentContext->parameter_##param1[currentUnit])          \
      || (param2 != currentContext->parameter_##param2[currentUnit])          \
      || (param3 != currentContext->parameter_##param3[currentUnit])          \
      || (param4 != currentContext->parameter_##param4[currentUnit])          \
      || (currentContext->currentBufferID[csGLStateCacheContext::boElementArray]\
        != currentContext->parameter_##vbo[currentUnit])                      \
      || FORCE_STATE_CHANGE)                                                  \
    {                                                                         \
      ActivateTCUnit (activateTexCoord);                                      \
      currentContext->parameter_##param1[currentUnit] = param1;               \
      currentContext->parameter_##param2[currentUnit] = param2;               \
      currentContext->parameter_##param3[currentUnit] = param3;               \
      currentContext->parameter_##param4[currentUnit] = param4;               \
      if (extmgr->CS_GL_ARB_vertex_buffer_object)                             \
      {                                                                       \
        ApplyBufferBinding (csGLStateCacheContext::boElementArray);           \
        currentContext->parameter_##vbo[currentUnit] =                        \
	  currentContext->currentBufferID[csGLStateCacheContext::boElementArray];\
      }                                                                       \
      func (param1, param2, param3, param4);                                  \
    }                                                                         \
  }                                                                           \
  void Get##name (type1 &param1, type2 & param2, type3 & param3,              \
    type4& param4) const                                                      \
  {                                                                           \
    const int currentUnit = currentContext->currentTCUnit;                    \
    param1 = currentContext->parameter_##param1[currentUnit];                 \
    param2 = currentContext->parameter_##param2[currentUnit];                 \
    param3 = currentContext->parameter_##param3[currentUnit];                 \
    param4 = currentContext->parameter_##param4[currentUnit];                 \
  }


class CS_CSPLUGINCOMMON_GL_EXPORT csGLStateCacheContext
{
  template<typename T>
  struct AutoArray
  {
    T* p;
    
    AutoArray() : p (0) {}
    ~AutoArray()
    {
      delete[] p;
    }
    void Setup (size_t n)
    {
      CS_ASSERT (p == 0);
      p = new T[n];
    }
    
    T& operator[] (size_t idx)
    {
      return p[idx];
    }
  };
public:
  csGLExtensionManager* extmgr;

  // Special caches
  AutoArray<GLuint> boundtexture;
  GLint numImageUnits;
  GLint numTexCoords;
  int currentImageUnit, currentTCUnit;
  int activeUnit[2];
  enum
  {
    boElementArray = 0, boIndexArray, boPixelPack, boPixelUnpack,
    
    boCount
  };
  GLuint currentBufferID[boCount];
  GLuint activeBufferID[boCount];
  static int GLBufferTargetToCacheIndex (GLenum target)
  {
    switch (target)
    {
    case GL_ARRAY_BUFFER_ARB:         return boElementArray;
    case GL_ELEMENT_ARRAY_BUFFER_ARB: return boIndexArray;
    case GL_PIXEL_PACK_BUFFER_ARB:    return boPixelPack;
    case GL_PIXEL_UNPACK_BUFFER_ARB:  return boPixelUnpack;
    default: return -1;      
    }
  }
  static GLenum CacheIndexToGLBufferTarget (int index)
  {
    static const GLenum localIndexToGLBufferTarget[boCount] =
    { GL_ARRAY_BUFFER_ARB, GL_ELEMENT_ARRAY_BUFFER_ARB, 
      GL_PIXEL_PACK_BUFFER_ARB, GL_PIXEL_UNPACK_BUFFER_ARB };
    return localIndexToGLBufferTarget[index];
  }

  // BlendFunc/BlendFuncSeparate
  GLenum blend_sourceRGB;
  GLenum blend_destinationRGB;
  GLenum blend_sourceA;
  GLenum blend_destinationA;
  
  // Pixel storage
  GLint pixelUnpackAlignment;
  bool pixelUnpackSwapBytes;
  
  // Color clamp control
  enum
  {
    clampVertex = 0, clampFragment = 1, clampRead = 2,
    clampCount
  };
  GLenum clampState[clampCount];
  static int GLClampTargetToCacheIndex (GLenum target)
  {
    switch (target)
    {
    case GL_CLAMP_VERTEX_COLOR_ARB:         return clampVertex;
    case GL_CLAMP_FRAGMENT_COLOR_ARB:         return clampFragment;
    case GL_CLAMP_READ_COLOR_ARB:         return clampRead;
    default: return -1;      
    }
  }

  // Standardized caches
  DECLARE_CACHED_BOOL (GL_DEPTH_TEST)
  DECLARE_CACHED_BOOL (GL_BLEND)
  DECLARE_CACHED_BOOL (GL_DITHER)
  DECLARE_CACHED_BOOL (GL_STENCIL_TEST)
  DECLARE_CACHED_BOOL (GL_CULL_FACE)
  DECLARE_CACHED_BOOL (GL_POLYGON_OFFSET_FILL)
  DECLARE_CACHED_BOOL (GL_LIGHTING)
  DECLARE_CACHED_BOOL (GL_ALPHA_TEST)
  DECLARE_CACHED_BOOL (GL_SCISSOR_TEST)
  DECLARE_CACHED_BOOL (GL_TEXTURE_GEN_S)
  DECLARE_CACHED_BOOL (GL_TEXTURE_GEN_T)
  DECLARE_CACHED_BOOL (GL_TEXTURE_GEN_R)
  DECLARE_CACHED_BOOL (GL_TEXTURE_GEN_Q)
  DECLARE_CACHED_BOOL (GL_FOG)
  DECLARE_CACHED_BOOL (GL_COLOR_SUM_EXT)
  DECLARE_CACHED_BOOL (GL_VERTEX_PROGRAM_POINT_SIZE_ARB)
  DECLARE_CACHED_BOOL (GL_POINT_SPRITE_ARB)
  DECLARE_CACHED_BOOL (GL_TEXTURE_CUBE_MAP_SEAMLESS)
  DECLARE_CACHED_BOOL (GL_SAMPLE_ALPHA_TO_COVERAGE_ARB)
  DECLARE_CACHED_BOOL (GL_SAMPLE_ALPHA_TO_ONE_ARB)
  DECLARE_CACHED_BOOL_IMAGEUNIT (GL_TEXTURE_1D)
  DECLARE_CACHED_BOOL_IMAGEUNIT (GL_TEXTURE_2D)
  DECLARE_CACHED_BOOL_IMAGEUNIT (GL_TEXTURE_3D)
  DECLARE_CACHED_BOOL_IMAGEUNIT (GL_TEXTURE_CUBE_MAP)
  DECLARE_CACHED_BOOL_IMAGEUNIT (GL_TEXTURE_RECTANGLE_ARB)
  DECLARE_CACHED_PARAMETER_2 (glAlphaFunc, AlphaFunc, GLenum, alpha_func, GLclampf, alpha_ref)
  DECLARE_CACHED_PARAMETER_1 (glCullFace, CullFace, GLenum, cull_mode)
  DECLARE_CACHED_PARAMETER_1 (glDepthFunc, DepthFunc, GLenum, depth_func)
  DECLARE_CACHED_PARAMETER_1 (glDepthMask, DepthMask, GLboolean, depth_mask)
  DECLARE_CACHED_PARAMETER_1 (glShadeModel, ShadeModel, GLenum, shade_model)
  DECLARE_CACHED_PARAMETER_3 (glStencilFunc, StencilFunc, GLenum, stencil_func, GLint, stencil_ref, GLuint, stencil_mask)
  DECLARE_CACHED_PARAMETER_3 (glStencilOp, StencilOp, GLenum, stencil_fail, GLenum, stencil_zfail, GLenum, stencil_zpass)
  DECLARE_CACHED_PARAMETER_1 (glStencilMask, StencilMask, GLuint, maskl)
  DECLARE_CACHED_PARAMETER_4 (glColorMask, ColorMask, GLboolean, wmRed, \
    GLboolean, wmGreen, GLboolean, wmBlue, GLboolean, wmAlpha)

  DECLARE_CACHED_CLIENT_STATE (GL_VERTEX_ARRAY)
  DECLARE_CACHED_CLIENT_STATE (GL_COLOR_ARRAY)
  DECLARE_CACHED_CLIENT_STATE (GL_SECONDARY_COLOR_ARRAY_EXT)
  DECLARE_CACHED_CLIENT_STATE (GL_NORMAL_ARRAY)
  DECLARE_CACHED_CLIENT_STATE_TCS (GL_TEXTURE_COORD_ARRAY)

  DECLARE_CACHED_PARAMETER_1 (glMatrixMode, MatrixMode, GLenum, matrixMode)
  
  DECLARE_CACHED_PARAMETER_4_BUF (glVertexPointer, VertexPointer, GLint, vsize,
    GLenum, vtype, GLsizei, vstride, GLvoid*, vpointer, vvbo)
  DECLARE_CACHED_PARAMETER_3_BUF (glNormalPointer, NormalPointer, GLenum, ntype,
    GLsizei, nstride, GLvoid*, npointer, nvbo)
  DECLARE_CACHED_PARAMETER_4_BUF (glColorPointer, ColorPointer, GLint, csize,
    GLenum, ctype, GLsizei, cstride, GLvoid*, cpointer, cvbo)
  DECLARE_CACHED_PARAMETER_4_BUF (extmgr->glSecondaryColorPointerEXT, 
    SecondaryColorPointerEXT, GLint, scsize, GLenum, sctype, GLsizei, scstride, 
    GLvoid*, scpointer, scvbo);
  DECLARE_CACHED_PARAMETER_4_BUF_TCS (glTexCoordPointer, TexCoordPointer, GLint, tsize,
    GLenum, ttype, GLsizei, tstride, GLvoid*, tpointer, tvbo)
  
  csGLStateCacheContext (csGLExtensionManager* extmgr);

  /** 
   * Init cache. Does both retrieval of current GL state as well as setting
   * some states to known values.
   */
  void InitCache();
};


/**
 * OpenGL state cache.
 * All state changes that are made often (possibly with the same value, ie
 * actually no change) or across plugins should be done through the cache.
 * \remarks
 * Since this class is passed directly between plugins the
 * code in this class cannot do memory allocations or
 * deallocations. The functions in this class will only
 * manipulate member variables.
 */
class csGLStateCache
{
  enum
  {
    texServer = 0,
    texClient = 1
  };
public:
  csGLExtensionManager* extmgr;
  csGLStateCacheContext* currentContext;

  csGLStateCache (csGLExtensionManager* extmgr)
  {
    csGLStateCache::extmgr = extmgr;
    currentContext = 0;
  }

  void SetContext (csGLStateCacheContext *context)
  {
    currentContext = context;
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
  IMPLEMENT_CACHED_BOOL (GL_TEXTURE_GEN_Q)
  IMPLEMENT_CACHED_BOOL (GL_FOG)
  IMPLEMENT_CACHED_BOOL (GL_COLOR_SUM_EXT)
  IMPLEMENT_CACHED_BOOL (GL_VERTEX_PROGRAM_POINT_SIZE_ARB)
  IMPLEMENT_CACHED_BOOL (GL_POINT_SPRITE_ARB)
  IMPLEMENT_CACHED_BOOL (GL_TEXTURE_CUBE_MAP_SEAMLESS)
  IMPLEMENT_CACHED_BOOL (GL_SAMPLE_ALPHA_TO_COVERAGE_ARB)
  IMPLEMENT_CACHED_BOOL (GL_SAMPLE_ALPHA_TO_ONE_ARB)
  IMPLEMENT_CACHED_BOOL_IMAGEUNIT (GL_TEXTURE_1D)
  IMPLEMENT_CACHED_BOOL_IMAGEUNIT (GL_TEXTURE_2D)
  IMPLEMENT_CACHED_BOOL_IMAGEUNIT (GL_TEXTURE_3D)
  IMPLEMENT_CACHED_BOOL_IMAGEUNIT (GL_TEXTURE_CUBE_MAP)
  IMPLEMENT_CACHED_BOOL_IMAGEUNIT (GL_TEXTURE_RECTANGLE_ARB)
  IMPLEMENT_CACHED_PARAMETER_2 (glAlphaFunc, AlphaFunc, GLenum, alpha_func, GLclampf, alpha_ref)
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
  IMPLEMENT_CACHED_CLIENT_STATE (GL_SECONDARY_COLOR_ARRAY_EXT)
  IMPLEMENT_CACHED_CLIENT_STATE (GL_NORMAL_ARRAY)
  IMPLEMENT_CACHED_CLIENT_STATE_TCS (GL_TEXTURE_COORD_ARRAY)

  IMPLEMENT_CACHED_PARAMETER_1 (glMatrixMode, MatrixMode, GLenum, matrixMode)
  
  IMPLEMENT_CACHED_PARAMETER_4_BUF (glVertexPointer, VertexPointer, GLint, vsize,
    GLenum, vtype, GLsizei, vstride, GLvoid*, vpointer, vvbo);
  IMPLEMENT_CACHED_PARAMETER_3_BUF (glNormalPointer, NormalPointer, GLenum, ntype,
    GLsizei, nstride, GLvoid*, npointer, nvbo);
  IMPLEMENT_CACHED_PARAMETER_4_BUF (glColorPointer, ColorPointer, GLint, csize,
    GLenum, ctype, GLsizei, cstride, GLvoid*, cpointer, cvbo);
  IMPLEMENT_CACHED_PARAMETER_4_BUF (extmgr->glSecondaryColorPointerEXT, 
    SecondaryColorPointerExt, GLint, scsize, GLenum, sctype, GLsizei, scstride, 
    GLvoid*, scpointer, scvbo);
  IMPLEMENT_CACHED_PARAMETER_4_BUF_TCS (glTexCoordPointer, TexCoordPointer, GLint, tsize,
    GLenum, ttype, GLsizei, tstride, GLvoid*, tpointer, tvbo);
  
  // Special caches
  void SetTexture (GLenum target, GLuint texture)
  {
    const int currentUnit = currentContext->currentImageUnit;
    if (texture != currentContext->boundtexture[currentUnit])
    {
      ActivateImageUnit ();
      currentContext->boundtexture[currentUnit] = texture;
      glBindTexture (target, texture);
    }
  }
  GLuint GetTexture (GLenum /*target*/)
  {
    const int currentUnit = currentContext->currentImageUnit;
    return currentContext->boundtexture[currentUnit];
  }
  GLuint GetTexture (GLenum /*target*/, int unit)
  {
    return currentContext->boundtexture[unit];
  }
  
  /**
   * Select the currently active image unit. 
   * \remarks Doesn't check whether the  multitexture extension is actually 
   * supported, this must be done in calling code.
   */
  void SetCurrentImageUnit (int unit)
  {
    CS_ASSERT(unit < currentContext->numImageUnits);
    currentContext->currentImageUnit = unit;   
  }
  /// Query active image unit.
  int GetCurrentImageUnit ()
  {
    return currentContext->currentImageUnit;
  }
  /**
   * Activate the currently selected image unit. 
   * Use this to bind textures.
   * \remarks Doesn't check whether the  multitexture extension is actually 
   *   supported, this must be done in calling code.
   */
  void ActivateImageUnit ()
  {
    int currentUnit = currentContext->currentImageUnit;
    if (currentContext->activeUnit[texServer] != currentUnit)
    {
      GLuint tu = GL_TEXTURE0_ARB + currentUnit;
      extmgr->glActiveTextureARB (tu);
      currentContext->activeUnit[texServer] = currentUnit;
    }
  }
  
  /**
   * Select the currently active texture coordinate unit. 
   * Use this to change state of texture coordinate arrays, 
   * change texture matrices, texture environments,
   * texture coord generation and enabling/disabling textures.
   * \remarks Doesn't check whether the  multitexture extension is actually 
   * supported, this must be done in calling code.
   */
  void SetCurrentTCUnit (int unit)
  {
    CS_ASSERT(unit < currentContext->numTexCoords);
    currentContext->currentTCUnit = unit;   
  }
  /// Query active texture coordinate set.
  int GetCurrentTCUnit ()
  {
    return currentContext->currentTCUnit;
  }
  /// Flag that the active TU should be used for setting texture coords
  static const int activateTexCoord = 1 << texClient;
  /// Flag that the active TU should be used when changing the texture matrix
  static const int activateMatrix = 1 << texServer;
  /// Flag that the active TU should be used when changing the texture environment
  static const int activateTexEnv = 1 << texServer;
  /**
   * Flag that the active TU should be used when changing the texture coord
   * generation parameters.
   */
  static const int activateTexGen = 1 << texServer;
  /**
   * Activate the currently selected coordinate set. 
   * \remarks Doesn't check whether the  multitexture extension is actually 
   *   supported, this must be done in calling code.
   */
  void ActivateTCUnit (uint usage)
  {
    int currentUnit = currentContext->currentTCUnit;
    for (int i = 0; i < 2; i++)
    {
      if (currentContext->activeUnit[i] != currentUnit)
      {
        GLuint tu = GL_TEXTURE0_ARB + currentUnit;
        if (usage & (1 << i))
        {
          if (i == texClient)
            extmgr->glClientActiveTextureARB (tu);
          else
            extmgr->glActiveTextureARB (tu);
          currentContext->activeUnit[i] = currentUnit;
        }
      }
    }
  }
  
  void ApplyBufferBinding (int index)
  {
    GLuint id = currentContext->currentBufferID[index];
    if (currentContext->activeBufferID[index] != id)
    {
      extmgr->glBindBufferARB (
        csGLStateCacheContext::CacheIndexToGLBufferTarget (index), id);
      currentContext->activeBufferID[index] = id;
    }
  }

  /**
   * Bind a given VBO/PBO buffer.
   * \remarks Doesn't check whether the relevant buffer object extension is 
   *   actually supported, this must be done in calling code.
   */
  void SetBufferARB (GLenum target, GLuint id, bool applyNow = false)
  {
    int index = csGLStateCacheContext::GLBufferTargetToCacheIndex (target);
    CS_ASSERT (index >= 0);
    currentContext->currentBufferID[index] = id;
    if (applyNow) ApplyBufferBinding (index);
  }

  /**
   * Get the currently bound VBO/PBO buffer.
   * \remarks Doesn't check whether the relevant buffer object extension is 
   *   actually supported, this must be done in calling code.
   */
  GLuint GetBufferARB (GLenum target)
  {
    int index = csGLStateCacheContext::GLBufferTargetToCacheIndex (target);
    CS_ASSERT (index >= 0);
    return currentContext->currentBufferID[index];
  }

  /**\name Blend functions
   * @{ */
  void SetBlendFunc (GLenum blend_source, GLenum blend_destination, 
		      bool forced = false)
  {
    if (forced 
      || (blend_source != currentContext->blend_sourceRGB)
      || (blend_source != currentContext->blend_sourceA)
      || (blend_destination != currentContext->blend_destinationRGB) 
      || (blend_destination != currentContext->blend_destinationA) 
      || FORCE_STATE_CHANGE)
    {
      currentContext->blend_sourceRGB = blend_source;
      currentContext->blend_sourceA = blend_source;
      currentContext->blend_destinationRGB = blend_destination;
      currentContext->blend_destinationA = blend_destination;
      glBlendFunc (blend_source, blend_destination);
    }
  }
  void GetBlendFunc (GLenum& blend_source, GLenum& blend_destination) const
  {
    blend_source = currentContext->blend_sourceRGB;
    blend_destination = currentContext->blend_destinationRGB;
  }
  void SetBlendFuncSeparate (GLenum blend_sourceRGB, 
			      GLenum blend_destinationRGB, 
			      GLenum blend_sourceA, 
			      GLenum blend_destinationA, 
			      bool forced = false)
  {
    if (forced 
      || (blend_sourceRGB != currentContext->blend_sourceRGB)
      || (blend_sourceA != currentContext->blend_sourceA)
      || (blend_destinationRGB != currentContext->blend_destinationRGB) 
      || (blend_destinationA != currentContext->blend_destinationA) 
      || FORCE_STATE_CHANGE)
    {
      currentContext->blend_sourceRGB = blend_sourceRGB;
      currentContext->blend_sourceA = blend_sourceA;
      currentContext->blend_destinationRGB = blend_destinationRGB;
      currentContext->blend_destinationA = blend_destinationA;
      extmgr->glBlendFuncSeparateEXT (blend_sourceRGB, blend_destinationRGB, 
	blend_sourceA, blend_destinationA);
    }
  }
  void GetBlendFuncSeparate (GLenum& blend_sourceRGB, 
			      GLenum& blend_destinationRGB,
			      GLenum& blend_sourceA, 
			      GLenum& blend_destinationA) const
  {
    blend_sourceRGB = currentContext->blend_sourceRGB;
    blend_destinationRGB = currentContext->blend_destinationRGB;
    blend_sourceA = currentContext->blend_sourceA;
    blend_destinationA = currentContext->blend_destinationA;
  }
  /** @} */
  
  /**\name Pixel storage
   * @{ */
  GLint GetPixelUnpackAlignment ()
  { return currentContext->pixelUnpackAlignment; }
  void SetPixelUnpackAlignment (GLint alignment)
  {
    if (alignment != currentContext->pixelUnpackAlignment)
    {
      glPixelStorei (GL_UNPACK_ALIGNMENT, alignment);
      currentContext->pixelUnpackAlignment = alignment;
    }
  }
  bool GetPixelUnpackSwapBytes ()
  { return currentContext->pixelUnpackSwapBytes; }
  void SetPixelUnpackSwapBytes (GLint swap)
  {
    bool swapAsbool = (swap != 0);
    if (swapAsbool != currentContext->pixelUnpackSwapBytes)
    {
      glPixelStorei (GL_UNPACK_SWAP_BYTES, (GLint)swap);
      currentContext->pixelUnpackSwapBytes = swapAsbool;
    }
  }
  /** @} */
  
  /**\name Clamp control
   * @{ */
  void SetClampColor (GLenum target, GLenum clamp)
  {
    int index = csGLStateCacheContext::GLClampTargetToCacheIndex (target);
    CS_ASSERT (index >= 0);
    if (clamp != currentContext->clampState[index])
    {
      extmgr->glClampColorARB (target, clamp);
      currentContext->clampState[index] = clamp;
    }
  }
  GLenum GetClampColor (GLenum target) const
  {
    int index = csGLStateCacheContext::GLClampTargetToCacheIndex (target);
    CS_ASSERT (index >= 0);
    return currentContext->clampState[index];
  }
  /** @} */
  
  /// Query the number of texture image units supported by OpenGL
  GLint GetNumImageUnits() const { return currentContext->numImageUnits; }
  /// Query the number of texture coordinate sets supported by OpenGL
  GLint GetNumTexCoords() const { return currentContext->numTexCoords; }
};

#undef IMPLEMENT_CACHED_BOOL
#undef IMPLEMENT_CACHED_BOOL_IMAGEUNIT
#undef IMPLEMENT_CACHED_PARAMETER_1
#undef IMPLEMENT_CACHED_PARAMETER_2
#undef IMPLEMENT_CACHED_PARAMETER_3
#undef IMPLEMENT_CACHED_PARAMETER_4
#undef IMPLEMENT_CACHED_PARAMETER_1_LAYER
#undef IMPLEMENT_CACHED_PARAMETER_2_LAYER
#undef IMPLEMENT_CACHED_PARAMETER_3_LAYER
#undef IMPLEMENT_CACHED_PARAMETER_4_LAYER
#undef IMPLEMENT_CACHED_CLIENT_STATE
#undef IMPLEMENT_CACHED_CLIENT_STATE_LAYER

#undef DECLARE_CACHED_BOOL
#undef DECLARE_CACHED_BOOL_IMAGEUNIT
#undef DECLARE_CACHED_PARAMETER_1
#undef DECLARE_CACHED_PARAMETER_2
#undef DECLARE_CACHED_PARAMETER_3
#undef DECLARE_CACHED_PARAMETER_4
#undef DECLARE_CACHED_PARAMETER_1_LAYER
#undef DECLARE_CACHED_PARAMETER_2_LAYER
#undef DECLARE_CACHED_PARAMETER_3_LAYER
#undef DECLARE_CACHED_PARAMETER_4_LAYER
#undef DECLARE_CACHED_CLIENT_STATE
#undef DECLARE_CACHED_CLIENT_STATE_LAYER

#undef FORCE_STATE_CHANGE

/** @} */

#endif // __CS_GLSTATES_H__
