/*
  Copyright (C) 2003 by Marten Svanfeldt
                        Anders Stenberg

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

#ifndef __CS_GL_RENDERBUFFER_H__
#define __CS_GL_RENDERBUFFER_H__

#include "ivideo/rndbuf.h"
#include "csutil/leakguard.h"

struct csGLVBOBufferSlot;
class csGLVBOBufferManager;

#undef min
#undef max
template<typename T>
T min(T a, T b)
{
  return (a<b?a:b);
}

template<typename T>
T max(T a, T b)
{
  return (a>b?a:b);
}

enum csGLRenderBufferLockType
{
  CS_GLBUF_RENDERLOCK_ARRAY,
  CS_GLBUF_RENDERLOCK_ELEMENTS
};

class csGLRenderBufferSub;

SCF_VERSION (csGLRenderBuffer, 0, 2, 0);
/**
 * Basic renderbuffer for OpenGL renderer.
 */
class csGLRenderBuffer : public iRenderBuffer
{
public:
  SCF_DECLARE_IBASE;
  CS_LEAKGUARD_DECLARE (csGLRenderBuffer);

  /**
   * Constructor.
   */
  csGLRenderBuffer (csGLVBOBufferManager * vbomgr, size_t size, csRenderBufferType type, 
    csRenderBufferComponentType componentType, int compoentcount, 
    size_t rangeStart, size_t rangeEnd, bool copy);

  /**
   * Destructor
   */
  virtual ~csGLRenderBuffer ();

  /**
  * Lock the buffer to allow writing and give us a pointer to the data.
  * The pointer will be (void*)-1 if there was some error.
  * \param lockType The type of lock desired.
  * \param samePointer Specifies whether the same pointer as last time should 
  *  be returned (ie all the old data will be still there, useful if only
  *  a part of the data is changed). However, setting this to 'true' may 
  *  cause a performance penalty - specifically, if the data is currently in
  *  use, the driver may have to wait until the buffer is available again.
  */
  virtual void* Lock(csRenderBufferLockType lockType, 
    bool samePointer = false);

  /// Releases the buffer. After this all writing to the buffer is illegal
  virtual void Release();

  /// Copy data to the render buffer.
  virtual void CopyToBuffer(const void *data, size_t length);

  /// Sets the number of components per element
  virtual void SetComponentCount (int count)
  {
    compCount = count;
  }

  /// Gets the number of components per element
  virtual int GetComponentCount () const
  {
    return compCount;
  }

  /// Sets the component type (float, int, etc)
  virtual void SetComponentType (csRenderBufferComponentType type);

  /// Gets the component type (float, int, etc)
  virtual csRenderBufferComponentType GetComponentType () const 
  {
    return comptype;
  }

  /// Get type of buffer (static/dynamic)
  virtual csRenderBufferType GetBufferType() const
  {
    return bufferType;
  }

  /// Get the size of the buffer (in bytes)
  virtual size_t GetSize() const
  {
    return bufferSize;
  }

  /// Set the stride of buffer (in bytes)
  virtual void SetStride(int stride)
  {
    this->stride = stride;
  }

  /// Get the stride of the buffer (in bytes)
  virtual int GetStride() const 
  {
    return stride;
  }

  /// Get the offset of the buffer (in bytes)
  virtual void SetOffset(int offset)
  {
    this->offset = offset;
  }

  /// Get version
  virtual uint GetVersion ()
  {
    return version;
  }

  virtual bool IsMasterBuffer ()
  {
    return true;
  }

  // Get "subbuffers" for interleaved scheme
  void SetupSUBBuffers (int count, csRef<iRenderBuffer>* buffers);

  // Do a lock for rendering
  virtual void* RenderLock (csGLRenderBufferLockType type);

  virtual void RenderRelease ();

protected:
  csRenderBufferType bufferType; //hint about main usage
  csRenderBufferComponentType comptype; //datatype for each component
  
  size_t bufferSize; //size in bytes
  int compCount; //numer of components per element
  int stride; //buffer stride
  int offset; //offset from buffer start to data
  int vbooffset; //offset from VBO-start

  size_t rangeStart; //range start for index-buffer
  size_t rangeEnd; //range end for index-buffer
  
  unsigned int version; //modification number

  //"cached" values to speed up usage
  GLenum compGLType;
  size_t compSize;

  bool doCopy; //should we copy data, or just use supplied buffer
  bool doDelete; //if buffer should be deleted on deallocation
  bool isLocked; //currently locked? (to prevent recursive locking)
  bool isIndex; //if this is index-buffer

  unsigned char *buffer; //buffer holding the data
  csRenderBufferLockType lastLock; //last type of lock used

  csGLVBOBufferSlot* vboSlot;
  csWeakRef<csGLVBOBufferManager> vbomgr;

  friend class csGLRenderBufferSub;
  friend class csGLGraphics3D;
  friend class csGLVBOBufferSlot;
  friend class csGLVBOBufferManager;
};

class csGLRenderBufferSub : public csGLRenderBuffer
{
public:
  csGLRenderBufferSub (csGLRenderBuffer *buffer);

  /// Copy data to the render buffer.
  virtual void CopyToBuffer(const void *data, size_t length)
  {
    csGLRenderBuffer::CopyToBuffer (data, length);
    owner->version++;
  }

  virtual void Release()
  {
    if (lastLock == CS_BUF_LOCK_NORMAL)
    {
      version++;
      owner->version++;
    }
    isLocked = false;
  }

  virtual bool IsMasterBuffer ()
  {
    return false;
  }

  // Do a lock for rendering
  void* RenderLock (csGLRenderBufferLockType type)
  {
    return ((unsigned char*)owner->RenderLock (type)) + offset;
  }

protected:
  csGLRenderBuffer *owner;
  friend class csGLVBOBufferManager;
};

/**
 * Single slot in a VBO buffer. Holds content of precisely 0 or 1 renderbuffers.
 */
struct csGLVBOBufferSlot
{
public:
  csGLVBOBufferSlot()
    : vboTarget (GL_ARRAY_BUFFER_ARB), vboID (0), lastCachedVersion (0), offset (0),
    listIdx (0), next (0), prev (0), inUse (false), locked (false), indexBuffer (false),
    separateVBO (false)
  {
  }

  void AttachBuffer (csGLRenderBuffer* buffer)
  {
    if (inUse && buffer!=renderBuffer) DeattachBuffer ();

    renderBuffer = buffer;
    buffer->vboSlot = this;
    buffer->vbooffset = offset;
  }

  void DeattachBuffer ()
  {
    if (renderBuffer.IsValid ())
    {
      renderBuffer->vboSlot = 0;
      renderBuffer->vbooffset = 0;
    }
    renderBuffer = 0;
    lastCachedVersion = 0;

  }

  GLenum vboTarget;   //opengl type, GL_ARRAY_BUFFER_ARB or GL_ELEMENT_ARRAY_BUFFER_ARB
  GLuint vboID;       //id of buffer

  unsigned int lastCachedVersion; //last version of the associated buffer we cached
  size_t offset;                  //offset from start of buffer
  unsigned int listIdx; 

  csWeakRef<csGLRenderBuffer> renderBuffer;

  csGLVBOBufferSlot *next, *prev;

  bool inUse;         //Currently allocated to some buffer
  bool locked;        //Locked due to active renderering
  bool indexBuffer;   //if this slot contains indexbuffer
  bool separateVBO;   //this slot have a separate VBO buffer
};

/**
 * Manager responsible for allocation and management of VBO buffers
 * and buffer slots
 */
class csGLVBOBufferManager : public iBase
{
public:
  SCF_DECLARE_IBASE;
  CS_LEAKGUARD_DECLARE (csGLVBOBufferManager);
  csGLVBOBufferManager (csGLExtensionManager *ext, csGLStateCache *state, 
    iObjectRegistry* p);
  virtual ~csGLVBOBufferManager ();

  /**
   * Activate a buffer before rendering. Make sure it is cached and the VBObuffer
   * is activated
   */
  bool ActivateBuffer (csGLRenderBuffer* buffer);

  /**
   * Deactivate a buffer
   */
  bool DeactivateBuffer (csGLRenderBuffer *buffer);

  /**
   * Buffer removed
   */
  void BufferRemoved (csGLRenderBuffer *buffer);

  /**
   * Make sure no VBO-buffer is activated
   */
  void DeactivateVBO ();


  // Dump stats about buffers to console
  void DumpStats ();

  // Reset frame stats
  void ResetFrameStats ();

protected:
  csGLExtensionManager *ext; //extension manager
  csGLStateCache *statecache;
  csConfigAccess config;
  iObjectRegistry *object_reg;
  bool verbose, superVerbose;

  enum
  {
    VBO_NUMBER_OF_SLOTS = 12,
    VBO_BIGGEST_SLOT_SIZE = 512*1024
  };

  // Precache buffer to specific slot
  void Precache (csGLRenderBuffer* buffer, csGLVBOBufferSlot* slot);

  // Activate bufferslot
  void ActivateVBOSlot (csGLVBOBufferSlot *slot);

  // Deactivate slot
  void DeactivateVBOSlot (csGLVBOBufferSlot *slot);

  // Find or allocate a new slot big enough, of given type
  // indexBuffer indicates if it is indexbuffer (true) or normal
  // render buffer (false)
  csGLVBOBufferSlot* FindEmptySlot (size_t size, bool ib);

  // allocate a vbo-buffer of given size
  GLuint AllocateVBOBuffer (size_t size, bool ib);

  // Touch slot as used
  void TouchSlot (csGLVBOBufferSlot *slot)
  {
    if (slot->separateVBO) return;
    if (slot->indexBuffer) indexBuffer.TouchSlot (slot);
    else vertexBuffer.TouchSlot (slot);
  }

  struct csGLVBOBuffer
  {
    ~csGLVBOBuffer();

    GLuint vboID;
    GLenum vboTarget; //opengl type, GL_ARRAY_BUFFER_ARB or GL_ELEMENT_ARRAY_BUFFER_ARB
    size_t size; //total size (in bytes);

    // Find an empty slot
    csGLVBOBufferSlot* FindEmptySlot (size_t size, bool splitStarted = false);

    // Setup, create VBO buffers and initial slots
    void Setup (GLenum usage, size_t totalSize, csGLExtensionManager *ext);


    // Touch a slot (say it is in use)
    void TouchSlot (csGLVBOBufferSlot *slot)
    {
      slots[slot->listIdx].MoveToBack (slot);
    }

    //helper for listmanagement
    size_t GetSizeFromIndex (uint index) { return max(1<<(index+8), 256); }
    uint GetIndexFromSize (size_t size) { return max(csLog2 (size-1)-7, 0); }

    struct slotList
    {
      slotList () : head (0), tail (0), usedSlots (0), slotsActivatedLastFrame (0),
        slotsActivatedThisFrame(0), slotSize (0), totalCount (0),
        slotsReusedLastFrame (0), slotsReusedThisFrame (0)
      {}
      csGLVBOBufferSlot *head, *tail;
      
      // Statistical data
      uint usedSlots;
      uint slotsActivatedLastFrame, slotsActivatedThisFrame;
      size_t slotSize;
      uint totalCount;
      uint slotsReusedLastFrame, slotsReusedThisFrame;

      //listmanagements
      void PushFront (csGLVBOBufferSlot *slot)
      {
        slot->next = head;
        if (head)
          head->prev = slot;
        else
          tail = slot;
        head = slot;
      }

      void MoveToBack (csGLVBOBufferSlot *slot)
      {
        if (slot == tail) return;
        //unlink
        Remove (slot);
        slot->next = 0;

        slot->prev = tail;
        if (tail)
          tail->next = slot;
        else
          head = slot;
        tail = slot;
      }

      void Remove (csGLVBOBufferSlot *slot)
      {
        if (slot->prev)
          slot->prev->next = slot->next;
        else
          head = slot->next;

        if (slot->next)
          slot->next->prev = slot->prev;
        else
          tail = slot->prev;

        slot->next = slot->prev = 0;
      }
      void VerifyList ()
      {
        csGLVBOBufferSlot *slot = head; 
        csGLVBOBufferSlot *prevSlot = 0;
        while (slot && slot!=tail)
        {
          CS_ASSERT(slot->prev == prevSlot);
          if (prevSlot) CS_ASSERT(prevSlot->next == slot);

          if (!slot->prev) CS_ASSERT(slot == head);
          if (!slot->next) CS_ASSERT(slot == tail);

          prevSlot = slot;
          slot = slot->next;
        }

        CS_ASSERT (slot==tail);
      }
    };
    // List of slots
    slotList slots[VBO_NUMBER_OF_SLOTS];
  };
  friend struct csGLVBOBuffer; // Borland: For access to VBO_NUMBER_OF_SLOTS.

  csGLVBOBuffer vertexBuffer; // List of all VBO buffers for VB storage.
  csGLVBOBuffer indexBuffer;  // List of all VBO buffers for IB storage.

  void ParseByteSize (const char* sizeStr, size_t& size);

  void Report (int severity, const char* msg, ...)
  {
    va_list arg;
    va_start (arg, msg);
    csReportV (object_reg, severity, "crystalspace.graphics3d.opengl.vbo", msg, arg);
    va_end (arg);
  }

};

#endif //__CS_GL_RENDERBUFFER_H__
