/*
  Copyright (C) 2010 by Frank Richter

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
#include "buffershadowing.h"

#include "csgfx/renderbuffer.h"
#include "csgfx/vertexlistwalker.h"

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{
  void BufferShadowingHelper::RenderBufferDestroyed (iRenderBuffer* buffer)
  {
    shadowedBuffers.DeleteAll (buffer);
  }
      
  BufferShadowingHelper::ShadowedBuffers& BufferShadowingHelper::GetShadowedData (
    iRenderBuffer* originalBuffer)
  {
    ShadowedBuffers* buffers = shadowedBuffers.GetElementPointer (originalBuffer);
    if (!buffers)
    {
      ShadowedBuffers& newBuffers = shadowedBuffers.GetOrCreate (originalBuffer);
      originalBuffer->SetCallback (this);
      return newBuffers;
    }
    return *buffers;
  }
  
  iRenderBuffer* BufferShadowingHelper::QueryFloatVertexDataBuffer (
    iRenderBuffer* originalBuffer)
  {
    CS_ASSERT(!originalBuffer->IsIndexBuffer());

    ShadowedData& shadowData = GetShadowedData (originalBuffer).floatVertexData;
    if (!shadowData.shadowBuffer
	|| (shadowData.originalBufferVersion != originalBuffer->GetVersion()))
    {
      if (!shadowData.shadowBuffer
	  || (shadowData.shadowBuffer->GetComponentCount()
	    != originalBuffer->GetComponentCount())
	  || (shadowData.shadowBuffer->GetElementCount()
	    != originalBuffer->GetElementCount()))
      {
	shadowData.shadowBuffer = csRenderBuffer::CreateRenderBuffer (
	  originalBuffer->GetElementCount(),
	  originalBuffer->GetBufferType(),
	  CS_BUFCOMP_FLOAT,
	  originalBuffer->GetComponentCount());
      }
      shadowData.originalBufferVersion = originalBuffer->GetVersion();
      
      // The vertex list walker actually already does all the conversion we need
      csVertexListWalker<float> src (originalBuffer);
      csRenderBufferLock<float> dst (shadowData.shadowBuffer);
      size_t copySize = sizeof(float) * shadowData.shadowBuffer->GetComponentCount();
      for (size_t i = 0; i < dst.GetSize(); i++)
      {
	memcpy ((float*)(dst++), (const float*)src, copySize);
	++src;
      }
    }
    
    return shadowData.shadowBuffer;
  }
  
}
CS_PLUGIN_NAMESPACE_END(gl3d)
