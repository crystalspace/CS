#ifndef __CS_OPENCL_MEMORY_IMPL_H__
#define __CS_OPENCL_MEMORY_IMPL_H__

#include <ivaria/clmemory.h>
#include <csutil/hash.h>
#include <csutil/scf_implementation.h>

CS_PLUGIN_NAMESPACE_BEGIN(CL)
{
  using namespace CS::CL;

  class MappedMemory;
  class Context;
  class Event;
  class Image;
  class Buffer;

  class MemoryObject : public scfImplementation1<MemoryObject, iMemoryObject>
  {
  friend class MappedMemory;
  public:
    MemoryObject() : scfImplementationType(this)
    {
      status.PutUnique(nullptr, true); // start with host being up to date
    }

    ~MemoryObject();

    virtual iBuffer* GetBuffer()
    {
      return nullptr;
    }

    virtual iImage* GetImage()
    {
      return nullptr;
    }

    virtual iSampler* GetSampler()
    {
      return nullptr;
    }
    
    virtual csPtr<iMemoryObject> Clone()
    {
      return csPtr<iMemoryObject>(nullptr); // stub
    }

    int GetAccessMode() const
    {
      return accessMode;
    }

    csRef<iEvent> Flush()
    {
      return lastWrite;
    }

    // mark you're using this object in the given context
    void Use(Context* c, Event* e, int accessType);

    void Purge();

    cl_mem GetHandle(Context*);
    csRef<iEvent> MoveTo(Context*, const iEventList& = iEventList());

  protected:
    // builds a list with Event objects from a list of iEvent objects
    // and adds the according usage events to the list
    // useLock should be locked on both this and obj where applicable
    bool BuildEventList(const iEventList&, csRefArray<Event>&,
                        int accessMode, MemoryObject* = nullptr,
                        int objAccessMode = 0);
    // finds a context in which both objects are valid or moves one object
    Context* FindContext(MemoryObject* other, csRefArray<Event>&, bool moveOther = true);
    virtual cl_mem CreateHandle(Context*) = 0;
    virtual csPtr<iEvent> Request(const iEventList& = iEventList()) = 0;
    virtual csPtr<iEvent> Write(void* src, Context*, const iEventList& = iEventList()) = 0;

    int accessMode;

    void* data;
    csHash<cl_mem, csRef<Context> > handles;

    // hash that holds information on whether
    // a handle is up-to-date or not
    csHash<bool, csRef<Context> > status;

    // holds the last context this object was written in
    Context* lastWriteContext;

    CS::Threading::RecursiveMutex useLock;

    // holds the last write event for this buffer
    csRef<iEvent> lastWrite;

    // holds the last read event for this buffer
    csRefArray<iEvent> reads;

    static void EventHandler(iEvent*, void*);
  };

  class MappedMemory : public scfImplementation1<MappedMemory,iMappedMemory>
  {
  public:
    MappedMemory(Context* c, MemoryObject* p, void* data, const size_t size[3],
                 const size_t offset[3], const size_t pitch[2], Event* e)
               : scfImplementationType(this,nullptr),
                 done(e), data(data), parent(p), context(c)
    {
      for(size_t i = 0; i < 3; ++i)
      {
        this->size[i] = size[i];
        this->offset[i] = offset[i];
      }

      for(size_t i = 0; i < 2; ++i)
        this->pitch[i] = pitch[i];
    }

    ~MappedMemory();

    csPtr<iEvent> Release(const iEventList& = iEventList());

    void* GetPointer() const
    {
      return data;
    }

    iBuffer* GetBuffer()
    {
      return parent->GetBuffer();
    }

    iImage* GetImage()
    {
      return parent->GetImage();
    }

    size_t GetOffset(int dimension = 0) const
    {
      return offset[dimension];
    }

    size_t GetSize(int dimension = 0) const
    {
      return size[dimension];
    }

    size_t GetPitch(int dimension = 0) const
    {
      return pitch[dimension];
    }

  private:
    csRef<Event> done;
    void* data;
    csRef<MemoryObject> parent;
    csRef<Context> context;
    size_t size[3];
    size_t offset[3];
    size_t pitch[2];

    static void HandleUnmap(iEvent*, void*);
  };

  class Buffer : public scfImplementationExt1<Buffer, MemoryObject,
                                              iBuffer>
  {
  friend class Image;
  public:
    SCF_INTERFACE(Buffer, 0, 0, 1);

    Buffer(size_t size, int accessMode) : scfImplementationType(this),
                                          size(size)
    {
      this->accessMode = accessMode;
      data = cs_malloc(size);
    }

    int GetObjectType() const
    {
      return MEM_BUFFER;
    }

    iBuffer* GetBuffer()
    {
      return this;
    }

    size_t GetSize() const
    {
      return size;
    }

    csPtr<iEvent> Request(size_t offset, size_t size,
                          const iEventList& = iEventList());

    csRef<iEvent> Read(size_t offset, size_t size, void* dst,
                       const iEventList& = iEventList());

    csRef<iEvent> Write(size_t offset, size_t size, void* src,
                        const iEventList& = iEventList());

    csRef<iEvent> Copy(iBuffer* dst, size_t size,
                       size_t src_offset, size_t dst_offset,
                       const iEventList& = iEventList());
    csRef<iEvent> Copy(iImage* dst, size_t src_offset,
                       const size_t dst_offset[3], const size_t dst_size[3],
                       const iEventList& = iEventList());

  private:
    cl_mem CreateHandle(Context*);
    csPtr<iEvent> Write(void* src, Context*, const iEventList& = iEventList());
    csPtr<iEvent> Request(const iEventList& eventList = iEventList())
    {
      return Request(0, size, eventList);
    }

    size_t size;
  };

  class Image : public scfImplementationExt1<Image, MemoryObject,
                                             iImage>
  {
  friend class Buffer;
  public:
    SCF_INTERFACE(Image, 0, 0, 1);

    Image(const size_t size[3], int format, int accessMode) : scfImplementationType(this),
                                                              format(format)
    {
      this->accessMode = accessMode;

      for(size_t i = 0; i < 3; ++i)
        this->size[i] = size[i];

      data = cs_malloc(GetSize());
    }

    int GetObjectType() const
    {
      return MEM_IMAGE;
    }

    iImage* GetImage()
    {
      return this;
    }

    int GetFormat() const
    {
      return format;
    }

    size_t GetWidth() const
    {
      return size[0];
    }

    size_t GetHeight() const
    {
      return size[1];
    }

    size_t GetDepth() const
    {
      return size[2];
    }

    size_t GetElementSize() const
    {
      size_t elementSize = 1;
      elementSize <<= format & FMT_SIZE;
      elementSize <<= (format & FMT_COUNT) >> 8;
      return elementSize;
    }

    csPtr<iEvent> Request(const size_t offset[3], const size_t size[3],
                          const iEventList& = iEventList());

    csRef<iEvent> Read(void* dst, const size_t size[3], const size_t src_offset[3],
                       const size_t dst_pitch[2], const iEventList& = iEventList());

    csRef<iEvent> Write(void* src, const size_t size[3], const size_t dst_offset[3],
                        const size_t src_pitch[2], const iEventList& = iEventList());

    csRef<iEvent> Copy(iImage* dst, const size_t size[3], const size_t src_offset[3],
                       const size_t dst_offset[3], const iEventList& = iEventList());
    csRef<iEvent> Copy(iBuffer* dst, size_t dst_offset, const size_t src_offset[3],
                       const size_t src_size[3], const iEventList& = iEventList());

  private:
    size_t GetSize() const
    {
      size_t totalSize = size[0]*size[1]*size[2]*GetElementSize();
      return totalSize;
    }

    cl_mem CreateHandle(Context*);
    csPtr<iEvent> Write(void* src, Context*, const iEventList& = iEventList());
    csPtr<iEvent> Request(const iEventList& eventList = iEventList())
    {
      const size_t offset[] = {0,0,0};
      return Request(offset, size, eventList);
    }

    size_t size[3];
    int format;
  };

  class Sampler : public scfImplementation2<Sampler,
                                            iMemoryObject,
                                            iSampler>
  {
  public:
    SCF_INTERFACE(Sampler, 0, 0, 1);

    Sampler(int addressMode, int filterMode, bool normalized) : scfImplementationType(this,nullptr),
                                                                addressMode(addressMode),
                                                                filterMode(filterMode),
                                                                normalized(normalized)
    {
    }

    ~Sampler()
    {
      Purge();
    }

    iBuffer* GetBuffer()
    {
      return nullptr;
    }

    iImage* GetImage()
    {
      return nullptr;
    }

    iSampler* GetSampler()
    {
      return this;
    }

    csPtr<iMemoryObject> Clone()
    {
      csRef<iMemoryObject> c;
      c.AttachNew(new Sampler(addressMode, filterMode, normalized));
      return csPtr<iMemoryObject>(c);
    }

    csRef<iEvent> Flush()
    {
      // no need to flush a sampler
      return csRef<iEvent>(nullptr);
    }

    void Purge();

    int GetAccessMode() const
    {
      return MEM_READ;
    }

    int GetObjectType() const
    {
      return MEM_SAMPLER;
    }

    int GetAddressMode() const
    {
      return addressMode;
    }

    int GetFilterMode() const
    {
      return filterMode;
    }

    bool GetNormalized() const
    {
      return normalized;
    }

    cl_sampler GetHandle(Context*);

  private:
    csHash<cl_sampler, csRef<Context> > handles;

    int addressMode;
    int filterMode;
    bool normalized;
  };
}
CS_PLUGIN_NAMESPACE_END(CL)

#endif // __CS_OPENCL_MEMORY_IMPL_H__

