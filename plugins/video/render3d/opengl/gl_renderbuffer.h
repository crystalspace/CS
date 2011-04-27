/*
  Copyright (C) 2003, 2007 by Marten Svanfeldt
                2003 by Anders Stenberg

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
#include "csutil/bitops.h"
#include "csutil/leakguard.h"
#include "csutil/scf_implementation.h"
#include "csutil/set.h"

struct csGLExtensionManager;
class csGLStateCache;

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{

class csGLGraphics3D;

static const GLenum compGLtypes[CS_BUFCOMP_BASE_TYPECOUNT] =
{
  GL_BYTE, GL_UNSIGNED_BYTE,
  GL_SHORT, GL_UNSIGNED_SHORT,
  GL_INT, GL_UNSIGNED_INT,
  GL_FLOAT,
  GL_DOUBLE,
  GL_HALF_FLOAT
};

enum csGLRenderBufferLockType
{
  CS_GLBUF_RENDERLOCK_ARRAY = 0,
  CS_GLBUF_RENDERLOCK_ELEMENTS = 1
};

class csGLVBOBufferManager : public scfImplementation1<csGLVBOBufferManager,
  iRenderBufferCallback>
{
public:  
  CS_LEAKGUARD_DECLARE (csGLVBOBufferManager);
  csGLVBOBufferManager (csGLGraphics3D* G3D, csGLExtensionManager *ext,
    csGLStateCache *state, size_t maxAlloction = 64*1024*1024,
    bool forceSeparateVBOs = false);
  virtual ~csGLVBOBufferManager ();

  /**
   * Called when render buffer is about to be destroyed and removed
   */
  virtual void RenderBufferDestroyed (iRenderBuffer* buffer);

  /**
   * Lock buffer for rendering
   */
  void* RenderLock (iRenderBuffer* buffer, csGLRenderBufferLockType type);

  /**
   * Unlock buffer after rendering
   */
  void RenderRelease (iRenderBuffer* buffer);

  /**
   * Make sure no VBO-buffer is activated
   */
  void DeactivateVBO ();

  // Dump internal data
  void DumpStats ();
private:

  // Internal constants
  enum
  {
    // Bit 0: indicates whether the buffer is a vertex or index buffer
    VBO_BUFFER_IS_VERTEX = 0,
    VBO_BUFFER_IS_INDEX = 1,
    // Bit 1: if set, buffer is static
    VBO_BUFFER_IS_STATIC = 2,
    VBO_BUFFER_TYPE_COUNT = 4,

    // Smallest allocation slot is 256 byte
    VBO_MIN_SLOT_SIZE_PO2 = 8,
    // Biggest allocation slots are 1MB
    VBO_MAX_SLOT_SIZE_PO2 = 20,
    VBO_NUM_SLOT_SIZES = VBO_MAX_SLOT_SIZE_PO2 - VBO_MIN_SLOT_SIZE_PO2 + 1,

    VBO_MAX_SLOT_SIZE = (1 << VBO_MAX_SLOT_SIZE_PO2)
  };

  // Preset number of slots per buffer
  static const size_t VBO_SLOT_PER_BUFFER[VBO_NUM_SLOT_SIZES];

  // Global state
  csGLGraphics3D* G3D;
  csGLExtensionManager *extensionManager; 
  csGLStateCache *stateCache;

  size_t currentVBOAllocation;
  size_t maxVBOAllocation;
  bool forceSeparateVBOs;

  struct VBOSlot;

  // A single VBO buffer
  struct VBOBuffer
  {
    // OpenGL id of buffer
    GLuint vboID;

    // Size per slot
    size_t slotSize;

    // Number of slots
    size_t numberOfSlots;

    // Buffer type
    size_t bufferType;

    // Link to next buffer
    VBOBuffer *nextBuffer, *prevBuffer;

    // Bitmap of free slots, stored 4 bytes at a time, LSB to MSB 
    // (bit 0 of first byte is first slot, bit 1 of first byte second etc)
    uint32* freeBitmap;

    // The VBO slots
    VBOSlot* vboSlots;
  };

  // Single slot in the VBO buffer
  struct VBOSlot
  {
    VBOBuffer* vboBuffer;
    iRenderBuffer* renderBuffer;
    uint32 bufferVersion;
    uint32 slotAge;
  };

  // Hold linked list (per size) of VBO buffers for smaller ones
  VBOBuffer* vboBufferList[VBO_BUFFER_TYPE_COUNT][VBO_NUM_SLOT_SIZES];

  // Array of bigger buffers
  csArray<VBOBuffer*> vboBigBuffers[VBO_BUFFER_TYPE_COUNT];

  // Contains all iRenderBuffer<->VBOSlot mappings
  csHash<VBOSlot*, csPtrKey<iRenderBuffer> > renderBufferMappings;

  // Contains all locked render buffers
  csSet<csPtrKey<iRenderBuffer> > lockedRenderBuffers;

  // Slot functions
  // Get VBO Id and pointer offset from slot
  inline void GetSlotIdAndOffset (const VBOSlot* slot, GLuint& id, size_t& offset) const
  {
    const VBOBuffer* vbob = slot->vboBuffer;

    id = vbob->vboID;

    const size_t slotIndex = size_t(slot - vbob->vboSlots);
    offset = slotIndex * vbob->slotSize;
  }

  // Given a size, get the next bigger slot-size as power of 2
  inline size_t GetSlotSizePO2 (size_t size) const
  {
    return csMax<size_t> (csLog2 (csFindNearestPowerOf2 ((int)size)), VBO_MIN_SLOT_SIZE_PO2);
  }
  
  inline bool IsSizePO2Slotted (size_t slotSizePO2) const
  { return !forceSeparateVBOs && (slotSizePO2 <= VBO_MAX_SLOT_SIZE_PO2); }

  // Given a renderbuffer, get a VBO slot if possible, otherwise 0
  VBOSlot* GetVBOSlot (iRenderBuffer* buffer);

  // Given slot size and type, find a free slot
  VBOSlot* GetFreeVBOSlot (size_t slotSizePO2, size_t slotType);

  // Try to release a vbo-slot of given type
  VBOSlot* TryFreeVBOSlot (size_t slotSizePO2, size_t slotType);

  // Release (and optionally deallocate) a VBO slot
  void ReleaseVBOSlot (VBOSlot* slot, bool deallocate = true);

  // Mark slot as used
  inline void SetSlotUsed (VBOSlot* slot) const
  {
    const size_t slotIndex = (slot - slot->vboBuffer->vboSlots);
    const size_t slotBitmapIdx = slotIndex >> 5;
    const size_t slotBitIdx = slotIndex & 31;

    slot->vboBuffer->freeBitmap[slotBitmapIdx] &= ~(1 << slotBitIdx);    
  }

  // Mark slot as not used
  inline void ClearSlotUsed (VBOSlot* slot) const
  {
    const size_t slotIndex = (slot - slot->vboBuffer->vboSlots);
    const size_t slotBitmapIdx = slotIndex >> 5;
    const size_t slotBitIdx = slotIndex & 31;

    slot->vboBuffer->freeBitmap[slotBitmapIdx] |= 1 << slotBitIdx;
  }

  // Buffer functions
  // Allocate a new VBO buffer and add it into internal lists
  VBOBuffer* GetNewVBOBuffer (size_t slotSize, size_t slotSizePO2, size_t slotType);

  // Release a VBO buffer
  void FreeVBOBuffer (VBOBuffer* buffer);

  // Other functions
  void DumpStatsBufferType (size_t type);

  /*

  VBOBuffer* AllocateVBOBuffer (size_t slotSize, size_t slotSizePO2, 
    bool& bigBuffer) const;

  inline void FreeVBOBuffer (VBOBuffer* buffer) const
  {
    void* end = buffer + sizeof(VBOBuffer) + sizeof(VBOSlot) * buffer->numberOfSlots +
      sizeof(uint32) * (buffer->numberOfSlots+31)/32;
    csPrintf ("Freeing: %p - %p\n", buffer, end);
    cs_free (buffer);
  }

  void SetupVBOBuffer (VBOBuffer* buffer, size_t bufferType);

  VBOBuffer* GetVBOBufferForSlot (size_t bufferSize, size_t bufferType, size_t& slotIndex);

  inline VBOBuffer* GetVBOBufferWithFreeSlot (size_t slotSizePO2, size_t bufferType, size_t& slotIndex) const
  {
    if (slotSizePO2 > VBO_MAX_SLOT_SIZE_PO2)
      return 0;

    // Scan VBO buffers of given type for one with free slots
    const size_t slotSizeIdx = slotSizePO2 - VBO_MIN_SLOT_SIZE_PO2;
    const size_t numSlots = VBO_SLOT_PER_BUFFER[slotSizeIdx];
    const size_t numSlotBitmap = (numSlots + 31) / 32;
    
    VBOBuffer* buffer = vboBufferList[bufferType][slotSizeIdx];

    // Scan over the buffers to find one with slots
    while (buffer)
    {
      uint32* bitmap = buffer->freeBitmap;
      // Scan the bitmap to find empty slots
      // Scan 32 bits at a time
      for (size_t bunchIdx = 0; bunchIdx < numSlotBitmap; bunchIdx++, bitmap++)
      {
        size_t localIndex;
        bool foundSlot = CS::Math::BitOps::ScanBitForward (*bitmap, localIndex);
        if (foundSlot)
        {
          slotIndex = 32*bunchIdx + localIndex;
          return buffer;
        }
      }
      
      buffer = buffer->nextBuffer;
    }

    return 0;//none found..let caller allocate one
  }

  // Age all slots in a buffer(chain), return index of oldest slot
  VBOSlot* AgeVBOBufferChain (VBOBuffer* bufferChain);

  // Try to evict a slot fromg given buffer chain
  VBOSlot* TryEvictVBOSlotFromChain (VBOBuffer* bufferChain);

  // Given a size, get the next bigger slot-size as power of 2
  inline size_t GetSlotSizePO2 (size_t size) const
  {
    return csMax<size_t> (csLog2 (csFindNearestPowerOf2 ((int)size)), VBO_MIN_SLOT_SIZE_PO2);
  }

  // Given a renderbuffer, get a VBO slot 
  VBOSlot* GetVBOSlot (iRenderBuffer* buffer);

  // Release a VBO slot
  void ReleaseVBOSlot (VBOSlot* slot, bool doRelease = true);

  // Helper when dumping statistics
  */
};

}
CS_PLUGIN_NAMESPACE_END(gl3d)

#endif //__CS_GL_RENDERBUFFER_H__
