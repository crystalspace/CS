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

#ifndef __BUFFERSHADOWING_H__
#define __BUFFERSHADOWING_H__

#include "ivideo/rndbuf.h"

#include "csutil/scf_implementation.h"

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{
  /**
   * Helper class for shadowing render buffer data (ie when not the originally
   * provided data is used but some different data, usually due to necessary
   * data conversions).
   */
  class BufferShadowingHelper :
    public scfImplementation1<BufferShadowingHelper,
                              iRenderBufferCallback>
  {
    struct ShadowedData
    {
      csRef<iRenderBuffer> shadowBuffer;
      uint originalBufferVersion;
      
      ShadowedData() : originalBufferVersion(~0) {}
    };
    struct ShadowedBuffers
    {
      ShadowedData floatVertexData;
    };
    typedef csHash<ShadowedBuffers, csPtrKey<iRenderBuffer>,
      CS::Memory::AllocatorMalloc,
      csArraySafeCopyElementHandler<
        CS::Container::HashElement<ShadowedBuffers, csPtrKey<iRenderBuffer> > >
      > ShadowedBuffersHash;
    ShadowedBuffersHash shadowedBuffers;
    
    ShadowedBuffers& GetShadowedData (iRenderBuffer* originalBuffer);
  public:
    BufferShadowingHelper() : scfImplementationType (this) {}
    
    /**\name iRenderBufferCallback implementation
     * @{ */
    virtual void RenderBufferDestroyed (iRenderBuffer* buffer);
    /** @} */
    
    iRenderBuffer* QueryFloatVertexDataBuffer (iRenderBuffer* originalBuffer);
  };
}
CS_PLUGIN_NAMESPACE_END(gl3d)

#endif // __BUFFERSHADOWING_H__

