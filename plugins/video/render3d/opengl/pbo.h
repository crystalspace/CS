/*
    Copyright (C) 2009 by Frank Richter

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

#ifndef __GL3D_PBO_H__
#define __GL3D_PBO_H__

#if defined(CS_OPENGL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENGL_PATH,gl.h)
#else
#include <GL/gl.h>
#endif

#include "iutil/databuff.h"
#include "csutil/customallocated.h"
#include "csutil/pooledscfclass.h"
#include "csutil/refcount.h"
#include "csutil/scf_implementation.h"

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{
  /// Wrapper for an OpenGL PBO
  class PBOWrapper : public CS::Memory::CustomAllocatedDerived<
			      CS::Utility::FastRefCount<PBOWrapper> >
  {
    GLuint pbo;
    size_t size;
  public:
    PBOWrapper (size_t size) : pbo (0), size (size) {}
    ~PBOWrapper();
    
    size_t GetSize() const { return size; }
    GLuint GetPBO (GLenum target);
  };
  
  /// iDataBuffer implementation returning data from a PBO
  class TextureReadbackPBO :
    public scfImplementationPooled<scfImplementation1<TextureReadbackPBO,
						      iDataBuffer> >
  {
    typedef scfImplementationPooled<scfImplementation1<TextureReadbackPBO,
						      iDataBuffer> > SuperClass;
    csRef<PBOWrapper> pbo;
    size_t size;
    mutable void* mappedData;
  public:
    TextureReadbackPBO (PBOWrapper* pbo, size_t size);
    ~TextureReadbackPBO ();
  
    size_t GetSize () const { return size; }
    char* GetData () const;
  };
  
  namespace CacheSorting
  {
    struct PBO
    {
      typedef size_t KeyType;
  
      static bool IsLargerEqual (const csRef<PBOWrapper>& b1, 
				const csRef<PBOWrapper>& b2)
      {
	return b1->GetSize() >= b2->GetSize();
      }
    
      static bool IsEqual (const csRef<PBOWrapper>& b1, 
			  const csRef<PBOWrapper>& b2)
      {
	return b1->GetSize() == b2->GetSize();
      }
  
      static bool IsLargerEqual (const csRef<PBOWrapper>& b1, 
				const size_t& b2)
      {
	return b1->GetSize() >= b2;
      }
    
      static bool IsLargerEqual (const size_t& b1, 
				 const csRef<PBOWrapper>& b2)
      {
	return b1 >= b2->GetSize();
      }
    
      static bool IsEqual (const csRef<PBOWrapper>& b1, 
			  const size_t& b2)
      {
	return b1->GetSize() == b2;
      }
    };
  } // namespace CacheSorting
}
CS_PLUGIN_NAMESPACE_END(gl3d)
  
#endif // __GL3D_PBO_H__
