#ifndef __CS_OPENCL_MANAGER_IMPL_H__
#define __CS_OPENCL_MANAGER_IMPL_H__

#include <ivaria/clmanager.h>
#include <iutil/comp.h>
#include <csutil/scf_implementation.h>

#include "event.h"

CS_PLUGIN_NAMESPACE_BEGIN(CL)
{
  using namespace CS::CL;

  class Context;
  class Queue;
  class Library;

  class Manager : public scfImplementation2<Manager,
                                            iManager,
                                            iComponent>
  {
  public:
    Manager(iBase* parent) : scfImplementationType(this, parent)
    {
    }

    // iComponent
    bool Initialize(iObjectRegistry*);

    // iManager
    csRef<iEvent> Queue(iKernel*, const iEventList& = iEventList());
    csPtr<iEvent> QueueMarker();
    bool QueueBarrier();
    bool QueueWait(const iEventList&);

    bool Wait(const iEventList&);
    void Finish();
    void Abort() // abort all tasks that aren't queued
    {
      // stub
    }

    void Purge() // free not strictly required memory
    {
      // stub
    }

    void FreeCompiler();

    csPtr<iEvent> CreateUserEvent(iThreadReturn*)
    {
      CS_ASSERT(false);
      return csPtr<iEvent>(nullptr); // stub
    }

    csPtr<iEvent> CreateUserEvent()
    {
      csRef<iEvent> ev;
      ev.AttachNew(new Event());
      return csPtr<iEvent>(ev);
    }

    csPtr<iLibrary> CreateLibrary(iStringArray* source);

    csPtr<iBuffer> CreateBuffer(size_t size, int access = MEM_READ_WRITE,
                                void* src = nullptr);
    csPtr<iBuffer> CreateBuffer(iBuffer*, size_t size, size_t offset,
                                int access = MEM_READ_WRITE)
    {
      CS_ASSERT(false);
      return csPtr<iBuffer>(nullptr); // stub
    }

    csPtr<iImage> CreateImage(size_t width, size_t height, int format,
                              int access = MEM_READ_WRITE,
                              void* src = nullptr, size_t row_pitch = 0);
    csPtr<iImage> CreateImage(size_t width, size_t height, size_t depth,
                              int format, int access = MEM_READ_WRITE,
                              void* src = nullptr, size_t row_pitch = 0,
                              size_t slice_pitch = 0);

    csPtr<iImage> CreateImage(::iImage*, int access = MEM_READ_WRITE)
    {
      CS_ASSERT(false);
      return csPtr<iImage>(nullptr); // stub
    }

    csPtr<iSampler> CreateSampler(int addressMode, int filterMode = FILTER_NEAREST,
                                  bool normalized = false);

    // internal accessors
    const csRefArray<Context>& GetContexts() const
    {
      return contexts;
    }

  private:
    csRefArray<Context> contexts;
    //@@@todo: maybe seperate the queues by type/context here?
    csRefArray<CS_PLUGIN_NAMESPACE_NAME(CL)::Queue> queues;

    //@@@todo: keep track of jobs queued in a given queue
    //         so we can at least do a round-robin
  };
}
CS_PLUGIN_NAMESPACE_END(CL)

#endif // #ifndef __CS_OPENCL_MANAGER_IMPL_H__
