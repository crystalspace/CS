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

#include "cssysdef.h"

#if defined(CS_OPENGL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENGL_PATH,gl.h)
#else
#include <GL/gl.h>
#endif

#include "instancing.h"

#include "csplugincommon/opengl/glhelper.h"

#include "gl_render3d.h"
#include "profilescope.h"

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{
  CS_IMPLEMENT_STATIC_CLASSVAR_REF(InstancingHelper, walkerAlloc,
				   WalkerAlloc,
				   InstancingHelper::WalkerAllocType,
				   (CS_VATTRIB_GENERIC_NUM))
  CS_IMPLEMENT_STATIC_CLASSVAR_REF(InstancingHelper,
				   transformWalkerAlloc,
				   TransformWalkerAlloc,
				   InstancingHelper::TransformWalkerAllocType,
				   (CS_VATTRIB_GENERIC_NUM))

  InstancingHelper::InstancingHelper (csGLGraphics3D* g3d,
				      size_t numInstances,
				      size_t paramNum,
				      const csVertexAttrib* paramsTargets,
				      csShaderVariable** const * params,
				      iRenderBuffer** paramBuffers)
   : g3d (g3d), targets (paramsTargets), allParams (params),
     instNum (0), numInstances (numInstances),
     numSVAttrs (0), numSpecialTeardownAttrs (0),
     numBufferAttrs (0), numTransformAttrs (0)
  {
    if (g3d->ext->CS_GL_ARB_instanced_arrays)
    {
      useInstancedArrays = true;
      
      /* Apart from the ext, it's also necessary that all
         parameters are passed as render buffers and bound
         to generic vertex attribs. */
      for (size_t p = 0; p < paramNum; p++)
      {
	iRenderBuffer* buffer = paramBuffers ? paramBuffers[p] : nullptr;
	if (!buffer || !CS_VATTRIB_IS_GENERIC(paramsTargets[p]))
	{
	  useInstancedArrays = false;
	  break;
	}
      }
    }
    else
      useInstancedArrays = false;
    
    if (useInstancedArrays)
    {
      numBufferAttrs = paramNum;
      instancedArraysBuffers = paramBuffers;
    }
    else
    {
      for (size_t p = 0; p < paramNum; p++)
      {
	if (p >= maxInstParamNum) break;
	
	iRenderBuffer* buffer = paramBuffers ? paramBuffers[p] : nullptr;
	if (buffer)
	{
	  if (buffer->GetComponentCount() <= 4)
	  {
	    void* ptr = WalkerAlloc().Alloc ();
	    InstBufferWalker* walker = new (ptr) InstBufferWalker (buffer, 4, nullptr);
	    bufferAttrs[numBufferAttrs++] = BufferAttr (walker, paramsTargets[p]);
	  }
	  else
	  {
	    static const float defaultTransform[16] = {0};
	    
	    TransformAttr attr;
	    attr.numVec4s = csMin ((buffer->GetComponentCount()+3) / 4, 4);
	    void* ptr = TransformWalkerAlloc().Alloc ();
	    attr.walker = new (ptr) InstTransformBufferWalker (buffer, attr.numVec4s*4, defaultTransform);
	    attr.attr = paramsTargets[p];
	    transformAttrs[numTransformAttrs++] = attr;
	  }
	}
	else
	{
	  svAttrs[numSVAttrs++] = p;
	}
	
	// Target needs special instance teardown
	csVertexAttrib target = paramsTargets[p];
	if ((target >= CS_IATTRIB_FIRST) && (target <= CS_IATTRIB_LAST))
	{
	  CS_ASSERT (numSpecialTeardownAttrs < CS_IATTRIB_NUM);
	  specialTeardownAttrs[numSpecialTeardownAttrs++] = p;
	}
      }
    }
  }

  InstancingHelper::~InstancingHelper ()
  {
    if (!useInstancedArrays)
    {
      for (uint b = 0; b < numBufferAttrs; b++)
      {
	bufferAttrs[b].first->~InstBufferWalker();
	WalkerAlloc().Free (bufferAttrs[b].first);
      }
      for (uint b = 0; b < numTransformAttrs; b++)
      {
	transformAttrs[b].walker->~InstTransformBufferWalker();
	TransformWalkerAlloc().Free (transformAttrs[b].walker);
      }
    }
  }
       
  void InstancingHelper::SetAttribute (csVertexAttrib target, const float* v)
  {
    switch (target)
    {
    case CS_VATTRIB_WEIGHT:
      if (g3d->ext->CS_GL_EXT_vertex_weighting)
	g3d->ext->glVertexWeightfvEXT (const_cast<float*> (v));
      break;
    case CS_VATTRIB_NORMAL:
      glNormal3fv (const_cast<float*> (v));
      break;
    case CS_VATTRIB_PRIMARY_COLOR:
      glColor4fv (const_cast<float*> (v));
      break;
    case CS_VATTRIB_SECONDARY_COLOR:
      if (g3d->ext->CS_GL_EXT_secondary_color)
	g3d->ext->glSecondaryColor3fvEXT (const_cast<float*> (v));
      break;
    case CS_VATTRIB_FOGCOORD:
      if (g3d->ext->CS_GL_EXT_fog_coord)
	g3d->ext->glFogCoordfvEXT (const_cast<float*> (v));
      break;
    case CS_IATTRIB_OBJECT2WORLD:
      {
	g3d->statecache->SetMatrixMode (GL_MODELVIEW);
	glPushMatrix ();
      }
      break;
    default:
      if (g3d->ext->CS_GL_ARB_multitexture)
      {
	if ((target >= CS_VATTRIB_TEXCOORD0) 
	  && (target <= CS_VATTRIB_TEXCOORD7))
	{
	  g3d->ext->glMultiTexCoord4fvARB (
	    GL_TEXTURE0 + (target - CS_VATTRIB_TEXCOORD0), 
	    const_cast<float*> (v));
	}
      }
      else if (target == CS_VATTRIB_TEXCOORD)
      {
	glTexCoord4fv (const_cast<float*> (v));
      }
      if (g3d->ext->glVertexAttrib4fvARB)
      {
	if (CS_VATTRIB_IS_GENERIC (target))
	{
	  g3d->ext->glVertexAttrib4fvARB (target - CS_VATTRIB_0,
	    const_cast<float*> (v));
	}
      }
    }
  }

  void InstancingHelper::SetMatrixAttribute (csVertexAttrib target, const float* v, uint vec4s)
  {
    switch (target)
    {
    case CS_IATTRIB_OBJECT2WORLD:
      {
	g3d->statecache->SetMatrixMode (GL_MODELVIEW);
	glPushMatrix ();
	if (vec4s < 4)
	{
	  float matrix[16];
	  uint row;
	  for (row = 0; row < vec4s; row++)
	    memcpy (matrix + row*4, v + row*4, sizeof (float)*4);
	  for (; row < 4; row++)
	    memset (matrix + row*4, 0, sizeof (float)*4);
	  glMultMatrixf (matrix);
	}
	else
	  glMultMatrixf (v);
      }
      break;
    default:
      if (g3d->ext->CS_GL_ARB_multitexture)
      {
	if ((target >= CS_VATTRIB_TEXCOORD0) 
	  && (target <= CS_VATTRIB_TEXCOORD7))
	{
	  // numTCUnits is type GLint, while target is an enumerated
	  // type. These are not necessarily the same.
	  uint maxN = csMin (csMin (uint (3), vec4s),
			     uint (g3d->GetNumTCUnits() - (target - CS_VATTRIB_TEXCOORD0)));
	  GLenum tu = GL_TEXTURE0 + (target - CS_VATTRIB_TEXCOORD0);
	  for (uint n = 0; n < maxN; n++)
	  {
	    g3d->ext->glMultiTexCoord4fvARB (tu + (GLenum)n, const_cast<float*> (v + n*4));
	  }
	}
      }
      if (g3d->ext->glVertexAttrib4fvARB)
      {
	if (CS_VATTRIB_IS_GENERIC (target))
	{
	  uint maxN = csMin (csMin (uint (3), vec4s),
			     uint (CS_VATTRIB_GENERIC_LAST - target + 1));
	  GLenum attr = (target - CS_VATTRIB_GENERIC_FIRST);
	  for (size_t n = 0; n < maxN; n++)
	  {
	    g3d->ext->glVertexAttrib4fvARB (attr + (GLenum)n, const_cast<float*> (v + n*4));
	  }
	}
      }
    }
  }

  void InstancingHelper::SetParamFromSV (csShaderVariable* param, csVertexAttrib target)
  {
    csVector4 v;
    float matrix[16];
    bool uploadMatrix;
    
    switch (param->GetType())
    {
    case csShaderVariable::MATRIX:
      {
	csMatrix3 m;
	param->GetValue (m);
	makeGLMatrix (m, matrix, target != CS_IATTRIB_OBJECT2WORLD);
	uploadMatrix = true;
      }
      break;
    case csShaderVariable::TRANSFORM:
      {
	csReversibleTransform tf;
	param->GetValue (tf);
	makeGLMatrix (tf, matrix, target != CS_IATTRIB_OBJECT2WORLD);
	uploadMatrix = true;
      }
      break;
    default:
      uploadMatrix = false;
    }
    if (uploadMatrix)
    {
      SetMatrixAttribute (target, matrix, 4);
    }
    else
    {
      param->GetValue (v);
      SetAttribute (target, v.m);
    }
  }

  void InstancingHelper::SetupNextInstance ()
  {
    ProfileScope _profile (g3d, "Setup instance"); // @@@ Worthwhile to measure?
    
    for (uint p = 0; p < numSVAttrs; p++)
    {
      csShaderVariable* const* params = allParams[instNum];
      size_t n = svAttrs[p];
      csShaderVariable* param = params[n];
      csVertexAttrib target = targets[n];
      SetParamFromSV (param, target);
    }
    
    for (uint b = 0; b < numBufferAttrs; b++)
    {
      InstBufferWalker* walker = bufferAttrs[b].first;
      SetAttribute (bufferAttrs[b].second,
		   ((csVector4)(*walker)).m);
      ++*(bufferAttrs[b].first);
    }
    
    for (uint b = 0; b < numTransformAttrs; b++)
    {
      SetMatrixAttribute (transformAttrs[b].attr, *(transformAttrs[b].walker),
			  transformAttrs[b].numVec4s);
      ++*(transformAttrs[b].walker);
    }
  }
  
  void InstancingHelper::TeardownInstance ()
  {
    for (uint p = 0; p < numSpecialTeardownAttrs; p++)
    {
      size_t n = specialTeardownAttrs[p];
      csVertexAttrib target = targets[n];
      switch (target)
      {
      case CS_IATTRIB_OBJECT2WORLD:
	{
	  g3d->statecache->SetMatrixMode (GL_MODELVIEW);
	  glPopMatrix ();
	}
	break;
      default:
	/* Nothing to do */
	break;
      }
    }
    
    instNum++;
  }
  
  void InstancingHelper::DrawAllInstances (GLenum mode, GLuint start, GLuint end,
					   GLsizei count, GLenum type, const GLvoid* indices)
  {
    if (useInstancedArrays)
    {
      // Woo, fast path!
      
      // Bind all buffers with instance data
      for (size_t p = 0; p < numBufferAttrs; p++)
      {
	GLuint attrIndex = targets[p] - CS_VATTRIB_GENERIC_FIRST;
	iRenderBuffer* buffer = instancedArraysBuffers[p];
	// @@@ Lost of copy and paste from ApplyBufferChanges(); somehow share the buffer binding code?
	csRenderBufferComponentType compType = buffer->GetComponentType();
      csRenderBufferComponentType compTypeBase = csRenderBufferComponentType(compType & ~CS_BUFCOMP_NORMALIZED);
	bool isFloat = (compType == CS_BUFCOMP_FLOAT) 
	  || (compType == CS_BUFCOMP_DOUBLE)
	  || (compType == CS_BUFCOMP_HALF);
	bool normalized = !isFloat && (compType & CS_BUFCOMP_NORMALIZED);
	GLenum compGLType = compGLtypes[compTypeBase];

	void *data = g3d->RenderLock (buffer, CS_GLBUF_RENDERLOCK_ARRAY);
	g3d->statecache->ApplyBufferBinding (csGLStateCacheContext::boElementArray);
	
	/* Split buffers with more than 4 components over multiple attribs
	   (Those buffers are usually transforms ...) */
	uint numBinds = (buffer->GetComponentCount() + 3) / 4;
	for (uint b = 0; b < numBinds; b++)
	{
	  g3d->ext->glEnableVertexAttribArrayARB (attrIndex+b);
	  uint ncomps = csMin (buffer->GetComponentCount () - b*4, 4u);
	  unsigned char* compsData = (unsigned char*)data + b*4*csRenderBufferComponentSizes[compTypeBase];
	  g3d->ext->glVertexAttribPointerARB (attrIndex+b,
					      ncomps,
					      compGLType,
					      normalized,
					      (GLsizei)buffer->GetElementDistance (),
					      compsData);
	  g3d->ext->glVertexAttribDivisorARB (attrIndex+b, 1);
	}
      }
      
      g3d->ext->glDrawElementsInstancedARB (mode, count, type, indices, numInstances);
      
      // ...and unbind.
      for (size_t p = 0; p < numBufferAttrs; p++)
      {
	GLuint attrIndex = targets[p] - CS_VATTRIB_GENERIC_FIRST;
	iRenderBuffer* buffer = instancedArraysBuffers[p];
	uint numBinds = (buffer->GetComponentCount() + 3) / 4;
	for (uint b = 0; b < numBinds; b++)
	{
	  g3d->ext->glVertexAttribDivisorARB (attrIndex+b, 0);
	  g3d->ext->glDisableVertexAttribArrayARB (attrIndex+b);
	}
	g3d->RenderRelease (buffer);
      }
      return;
    }
    
    while (instNum < numInstances)
    {
      SetupNextInstance ();
      g3d->glDrawRangeElements (mode, start, end, count, type, indices);
      TeardownInstance ();
    }
  }
}
CS_PLUGIN_NAMESPACE_END(gl3d)
