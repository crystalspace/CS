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

#ifndef __CS_OPENCL_MANAGER_H__
#define __CS_OPENCL_MANAGER_H__

#include "csutil/ref.h"
#include "csutil/refarr.h"
#include "csutil/nullptr.h"
#include "clconsts.h"

struct iThreadReturn;
struct iImage;

namespace CS
{
namespace CL
{
  struct iBuffer;
  struct iImage;
  struct iSampler;
  struct ImageFormat;

  struct iKernel;

  typedef void (*Callback)(iEvent*,void*);

  /// structure that holds information about an event
  struct iEvent : public virtual iBase
  {
    SCF_INTERFACE(iEvent, 0, 0, 1);

    /// check whether the event has fired
    virtual bool IsFinished() = 0;
    /// check whether the associated operation was successful
    virtual bool WasSuccessful() = 0;
    /// wait for the event to fire
    virtual bool Wait() = 0;
    /// add a callback that is to be called if the event is fired
    virtual void AddCallback(Callback, void*) = 0;
    /// remove a callback that is to be called if the event is fired
    virtual bool RemoveCallback(Callback) = 0;
    /// fire the event (only for user-defined events)
    /// @see iManager::CreateEvent
    virtual bool Fire(bool success) = 0;
    /// query result if there is any
    virtual iBase* GetResult() = 0;
  };

  typedef csRefArray<iEvent> iEventList;

  struct iManager : public virtual iBase
  {
    SCF_INTERFACE(iManager, 0, 0, 1);

    virtual csRef<iEvent> Queue(iKernel*, const iEventList& = iEventList()) = 0;
    virtual csPtr<iEvent> QueueMarker() = 0;
    virtual bool QueueBarrier() = 0;
    virtual bool QueueWait(const iEventList&) = 0;

    virtual bool Wait(const iEventList&) = 0;
    virtual void Finish() = 0;
    virtual void Abort() = 0; // abort all tasks that aren't queued

    virtual void Purge() = 0; // free not strictly required memory
    virtual void FreeCompiler() = 0;

    virtual csPtr<iEvent> CreateUserEvent(iThreadReturn*) = 0;
    virtual csPtr<iEvent> CreateUserEvent() = 0;

    virtual csPtr<iLibrary> CreateLibrary(iStringArray* source) = 0;

    virtual csPtr<iBuffer> CreateBuffer(size_t size, int access = MEM_READ_WRITE,
                                        void* src = nullptr) = 0;
    virtual csPtr<iBuffer> CreateBuffer(iBuffer*, size_t size, size_t offset,
                                        int access = MEM_READ_WRITE) = 0;

    virtual csPtr<iImage> CreateImage(size_t width, size_t height, int format,
                                      int access = MEM_READ_WRITE,
                                      void* src = nullptr, size_t row_pitch = 0) = 0;
    virtual csPtr<iImage> CreateImage(size_t width, size_t height, size_t depth,
                                      int format, int access = MEM_READ_WRITE,
                                      void* src = nullptr, size_t row_pitch = 0,
                                      size_t slice_pitch = 0) = 0;
    virtual csPtr<iImage> CreateImage(::iImage, int access = MEM_READ_WRITE) = 0;

    virtual csPtr<iSampler> CreateSampler(int addressMode, int filterMode = FILTER_NEAREST,
                                          bool normalized = false) = 0;
  };
} // namespace CL
} // namespace CS

#endif // __CS_OPENCL_MANAGER_H__
