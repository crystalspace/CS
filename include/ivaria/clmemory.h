/*
  Copyright (C) 2011 by Matthieu Kraus

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
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

#ifndef __CS_OPENCL_MEMORY_H__
#define __CS_OPENCL_MEMORY_H__

#include "clmanager.h"

namespace CS
{
namespace CL
{
  struct iImage;
  struct iBuffer;
  struct iSampler;

  struct iMappedMemory : public virtual iBase
  {
    SCF_INTERFACE(iMappedMemory, 0, 0, 1);

    virtual void* GetPointer() const = 0;
    virtual iBuffer* GetBuffer() = 0;
    virtual iImage* GetImage() = 0;
    virtual size_t GetOffset(int dimension = 0) const = 0;
    virtual size_t GetSize(int dimension = 0) const = 0;
    virtual size_t GetPitch(int dimesion = 0) const = 0;
    virtual csPtr<iEvent> Release(const iEventList& = iEventList()) = 0;
  };

  struct iMemoryObject : public virtual iBase
  {
    SCF_INTERFACE(iMemoryObject, 0, 0, 1);

    virtual csPtr<iMemoryObject> Clone() = 0;

    virtual csRef<iEvent> Flush() = 0;
    virtual void Purge() = 0; // free not strictly required data

    virtual int GetAccessMode() const = 0;
    virtual int GetObjectType() const = 0;
    virtual iBuffer* GetBuffer() = 0;
    virtual iImage* GetImage() = 0;
    virtual iSampler* GetSampler() = 0;
  };

  struct iBuffer : public virtual iBase
  {
    SCF_INTERFACE(iBuffer, 0, 0, 1);

    virtual size_t GetSize() const = 0;

    virtual csPtr<iEvent> Request(size_t offset, size_t size,
                                  const iEventList& = iEventList()) = 0;

    virtual csRef<iEvent> Read(size_t offset, size_t size, void* dst,
                               const iEventList& = iEventList()) = 0;

    virtual csRef<iEvent> Write(size_t offset, size_t size, void* src,
                                const iEventList& = iEventList()) = 0;

    virtual csRef<iEvent> Copy(iBuffer* dst, size_t size,
                               size_t src_offset, size_t dst_offset,
                               const iEventList& = iEventList()) = 0;
    virtual csRef<iEvent> Copy(iImage* dst, size_t src_offset,
                               const size_t dst_offset[3], const size_t dst_size[3],
                               const iEventList& = iEventList()) = 0;
  };

  struct iImage : public virtual iBase
  {
    SCF_INTERFACE(iImage, 0, 0, 1);

    virtual int GetFormat() const = 0;
    virtual size_t GetWidth() const = 0;
    virtual size_t GetHeight() const = 0;
    virtual size_t GetDepth() const = 0;
    virtual size_t GetElementSize() const = 0;

    virtual csPtr<iEvent> Request(const size_t offset[3], const size_t size[3],
                                  const iEventList& = iEventList()) = 0;

    virtual csRef<iEvent> Read(void* dst, const size_t size[3],
                               const size_t src_offset[3], const size_t dst_pitch[2],
                               const iEventList& = iEventList()) = 0;

    virtual csRef<iEvent> Write(void* src, const size_t size[3],
                                const size_t dst_offset[3], const size_t src_pitch[2],
                                const iEventList& = iEventList()) = 0;

    virtual csRef<iEvent> Copy(iImage* dst, const size_t size[3],
                               const size_t src_offset[3], const size_t dst_offset[3],
                               const iEventList& = iEventList()) = 0;
    virtual csRef<iEvent> Copy(iBuffer* dst, size_t dst_offset,
                               const size_t src_offset[3], const size_t src_size[3],
                               const iEventList& = iEventList()) = 0;
  };

  struct iSampler : public virtual iBase
  {
    SCF_INTERFACE(iSampler, 0, 0, 1);

    virtual int GetAddressMode() const = 0;
    virtual int GetFilterMode() const = 0;
    virtual bool GetNormalized() const = 0;
  };
} // namespace CL
} // namespace CS

#endif // __CS_OPENCL_MEMORY_H__

