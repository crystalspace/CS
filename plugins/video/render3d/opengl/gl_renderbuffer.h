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

static const GLenum compGLtypes[CS_BUFCOMP_TYPECOUNT] =
{
  GL_BYTE, GL_UNSIGNED_BYTE,
  GL_SHORT, GL_UNSIGNED_SHORT,
  GL_INT, GL_UNSIGNED_INT,
  GL_FLOAT,
  GL_DOUBLE
};

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

#if 0
class csGLRenderBufferSub;

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

protected:
  csGLRenderBuffer *owner;
  friend class csGLVBOBufferManager;
};
#endif

/**
 * Single slot in a VBO buffer. Holds content of precisely 0 or 1 renderbuffers.
 */
struct csGLVBOBufferSlot
{
public:
  csGLVBOBufferSlot()
    : vboTarget (GL_ARRAY_BUFFER_ARB), vboID (0), lastCachedVersion (0), offset (0),
    listIdx (0), renderBufferPtr(0), next (0), prev (0), inUse (false), locked (false), 
    indexBuffer (false), separateVBO (false)
  {
  }

  GLenum vboTarget;   //opengl type, GL_ARRAY_BUFFER_ARB or GL_ELEMENT_ARRAY_BUFFER_ARB
  GLuint vboID;       //id of buffer

  unsigned int lastCachedVersion; //last version of the associated buffer we cached
  size_t offset;                  //offset from start of buffer
  unsigned int listIdx; 

  csWeakRef<iRenderBuffer> renderBuffer;
  iRenderBuffer* renderBufferPtr; // Needed to find hash entry

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
  bool ActivateBuffer (iRenderBuffer* buffer);

  /**
   * Deactivate a buffer
   */
  bool DeactivateBuffer (iRenderBuffer *buffer);

  /**
   * Buffer removed
   */
  void BufferRemoved (iRenderBuffer *buffer);

  /**
   * Make sure no VBO-buffer is activated
   */
  void DeactivateVBO ();

  void* RenderLock (iRenderBuffer* buffer, csGLRenderBufferLockType type);
  void RenderRelease (iRenderBuffer* buffer);

  // Dump stats about buffers to console
  void DumpStats ();

  // Reset frame stats
  void ResetFrameStats ();

protected:
  struct RenderBufferAux
  {
    size_t vbooffset; //offset from VBO-start
    //"cached" values to speed up usage
    csGLVBOBufferSlot* vboSlot;
  };
  csGLExtensionManager *ext; //extension manager
  csGLStateCache *statecache;
  csConfigAccess config;
  iObjectRegistry *object_reg;
  bool verbose, superVerbose;
  csHash<RenderBufferAux, iRenderBuffer*> bufferData;

  enum
  {
    VBO_NUMBER_OF_SLOTS = 12,
    VBO_BIGGEST_SLOT_SIZE = 512*1024
  };

  // Precache buffer to specific slot
  void Precache (iRenderBuffer* buffer, csGLVBOBufferSlot* slot);

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

  void DetachBuffer (csGLVBOBufferSlot *slot);
  void AttachBuffer (csGLVBOBufferSlot *slot, iRenderBuffer* buffer);
 
  struct csGLVBOBuffer;
  friend struct csGLVBOBuffer; // Borland/VC6: For access to VBO_NUMBER_OF_SLOTS.
  struct csGLVBOBuffer
  {
    ~csGLVBOBuffer();

    csGLVBOBufferManager* bufmgr;
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

  csGLVBOBuffer vertexBuffer; // List of all VBO buffers for VB storage.
  csGLVBOBuffer indexBuffer;  // List of all VBO buffers for IB storage.

  void ParseByteSize (const char* sizeStr, size_t& size);

  void Report (int severity, const char* msg, ...)
  {
    va_list arg;
    va_start (arg, msg);
    csReportV (object_reg, severity, "crystalspace.graphics3d.opengl.vbo", 
      msg, arg);
    va_end (arg);
  }

};

#endif //__CS_GL_RENDERBUFFER_H__
