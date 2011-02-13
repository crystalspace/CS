/*
  Copyright (C) 2011 by Frank Richter

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

#ifndef __INSTANCING_H__
#define __INSTANCING_H__

#include "ivideo/graph3d.h"
#include "csgfx/vertexlistwalker.h"
#include "csutil/blockallocator.h"

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{
  class csGLGraphics3D;

  class InstancingHelper
  {
    csGLGraphics3D* g3d;
    const csVertexAttrib* targets;
    csShaderVariable** const * allParams;
    
    size_t instNum;
    size_t numInstances;
    
    bool useInstancedArrays;
    iRenderBuffer** instancedArraysBuffers;
    
    enum
    {
      maxInstParamNum = CS_VATTRIB_SPECIFIC_NUM + CS_VATTRIB_GENERIC_NUM + CS_IATTRIB_NUM
    };
    uint numSVAttrs;
    uint svAttrs[maxInstParamNum];
    uint numSpecialTeardownAttrs;
    uint specialTeardownAttrs[CS_IATTRIB_NUM];
    
    typedef csVertexListWalker<float, csVector4> InstBufferWalker;
    typedef csFixedSizeAllocator<sizeof (InstBufferWalker)> WalkerAllocType;
    CS_DECLARE_STATIC_CLASSVAR_REF(walkerAlloc, WalkerAlloc, WalkerAllocType)
    uint numBufferAttrs;
    typedef csTuple2<InstBufferWalker*, csVertexAttrib> BufferAttr;
    BufferAttr bufferAttrs[maxInstParamNum];
    
    typedef float TransformParam[16];
    typedef csVertexListWalker<float, TransformParam> InstTransformBufferWalker;
    typedef csFixedSizeAllocator<sizeof (InstTransformBufferWalker)> TransformWalkerAllocType;
    CS_DECLARE_STATIC_CLASSVAR_REF(transformWalkerAlloc, TransformWalkerAlloc,
				   TransformWalkerAllocType)
    uint numTransformAttrs;
    struct TransformAttr
    {
      InstTransformBufferWalker* walker;
      csVertexAttrib attr;
      uint numVec4s;
    };
    TransformAttr transformAttrs[maxInstParamNum];
    
    void SetAttribute (csVertexAttrib target, const float* v);
    void SetMatrixAttribute (csVertexAttrib target, const float* v, uint vec4s);
    void SetParamFromSV (csShaderVariable* param, csVertexAttrib target);

    void SetupNextInstance ();
    void TeardownInstance ();
  public:
    InstancingHelper (csGLGraphics3D* g3d,
		      size_t numInstances,
		      size_t paramNum, const csVertexAttrib* paramsTargets,
 		      csShaderVariable** const * params,
		      iRenderBuffer** paramBuffers);
    ~InstancingHelper ();
    
    void DrawAllInstances (GLenum mode, GLuint start, 
      GLuint end, GLsizei count, GLenum type, const GLvoid* indices);
  };
}
CS_PLUGIN_NAMESPACE_END(gl3d)

#endif // __INSTANCING_H__
