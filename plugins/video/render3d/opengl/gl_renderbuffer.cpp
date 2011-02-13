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

#include "cssysdef.h"
#include "csutil/scf.h"
#include "csgfx/renderbuffer.h"

#include "gl_render3d.h"
#include "gl_renderbuffer.h"
#include "profilescope.h"

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{

CS_LEAKGUARD_IMPLEMENT (csGLVBOBufferManager);


#if 1
const size_t csGLVBOBufferManager::VBO_SLOT_PER_BUFFER[] = 
{
  512,  //256b
  512,  //512b
  512,  //1kb
  512,  //2kb
  256,  //4kb
  128,  //8kb
  64,  //16kb
  32,  //32kb
  16,  //64kb
  8,  //128kb
  4,  //256kb
  4,  //512kb
  2,  //1MB
};
#else

const size_t csGLVBOBufferManager::VBO_SLOT_PER_BUFFER[] = 
{
  2,  //256b
  2,  //512b
  2,  //1kb
  2,  //2kb
  2,  //4kb
  2,  //8kb
  2,  //16kb
  2,  //32kb
  2,  //64kb
  2,  //128kb
  2,  //256kb
  2,  //512kb
  2,  //1MB
};
#endif

static const GLenum VBO_BUFFER_GL_TYPE[] = {
  GL_ARRAY_BUFFER_ARB, 
  GL_ELEMENT_ARRAY_BUFFER,
  GL_ARRAY_BUFFER_ARB, 
  GL_ELEMENT_ARRAY_BUFFER};
static const GLenum VBO_BUFFER_GL_USAGE[] = {
  GL_DYNAMIC_DRAW_ARB, 
  GL_DYNAMIC_DRAW_ARB,
  GL_STATIC_DRAW_ARB,
  GL_STATIC_DRAW_ARB};

csGLVBOBufferManager::csGLVBOBufferManager (csGLGraphics3D* G3D,
					    csGLExtensionManager *ext, 
                                            csGLStateCache *state,
                                            size_t maxAllocation,
					    bool forceSeparateVBOs)                                            
  : scfImplementationType (this), G3D (G3D), extensionManager (ext),
    stateCache (state), currentVBOAllocation (0),
    maxVBOAllocation (maxAllocation), forceSeparateVBOs (forceSeparateVBOs)
{
  memset (&vboBufferList, 0, sizeof(vboBufferList));
}

csGLVBOBufferManager::~csGLVBOBufferManager ()
{
  // Deallocate stuff@@@
  for (size_t type = 0; type < VBO_BUFFER_TYPE_COUNT; ++type)
  {
    for (size_t i = 0; i < VBO_NUM_SLOT_SIZES; ++i)
    {
      VBOBuffer* buffer = vboBufferList[type][i];

      while (buffer)
      {
        VBOBuffer* next = buffer->nextBuffer;
        FreeVBOBuffer (buffer);
        buffer = next;
      }
    }

    for (size_t i = 0; i < vboBigBuffers[type].GetSize (); ++i)
    {
      FreeVBOBuffer (vboBigBuffers[type][i]);
    }
  }
}


void* csGLVBOBufferManager::RenderLock (iRenderBuffer* buffer, 
                                        csGLRenderBufferLockType type)
{
  iRenderBuffer* masterBuffer = buffer->GetMasterBuffer ();
  iRenderBuffer* effectiveBuffer = masterBuffer ? masterBuffer : buffer;

  // Check that size haven't changed!

  // Make sure we lock it right way
  CS_ASSERT ((type==CS_GLBUF_RENDERLOCK_ELEMENTS) == effectiveBuffer->IsIndexBuffer ());

  // Mark render buffer as locked so it cannot be recycled
  lockedRenderBuffers.Add (effectiveBuffer);

  // Get a slot
  VBOSlot* slot = GetVBOSlot (effectiveBuffer);

  if (slot)
  {
    // Get VBO ID and offset from slot
    GLuint vboID = 0;
    size_t offset = 0;

    GetSlotIdAndOffset (slot, vboID, offset);

    // Activate the buffer
    stateCache->SetBufferARB (VBO_BUFFER_GL_TYPE[type], vboID);
  
    CS_ASSERT (slot->renderBuffer == effectiveBuffer);
    CS_ASSERT (slot->vboBuffer->slotSize >= effectiveBuffer->GetSize ());

    return (void*)(((uint8*)offset) + buffer->GetOffset ());
  }
  else
  {
    // Didn't get any slot, so go with sysmem buffer for now
    stateCache->SetBufferARB (VBO_BUFFER_GL_TYPE[type], 0);

    uint8* data = (uint8*)effectiveBuffer->Lock (CS_BUF_LOCK_READ);
    if (data != (uint8*)-1)
      return data + buffer->GetOffset ();
    else
      return (void*)-1;
  }

  return (void*)-1;
}

void csGLVBOBufferManager::RenderRelease (iRenderBuffer* buffer)
{
  // Remove lock
  iRenderBuffer* masterBuffer = buffer->GetMasterBuffer ();
  iRenderBuffer* effectiveBuffer = masterBuffer ? masterBuffer : buffer;

  lockedRenderBuffers.Delete (effectiveBuffer);
  effectiveBuffer->Release (); // Just to be sure if we didn't actually use a VBO
}

void csGLVBOBufferManager::DeactivateVBO ()
{
  stateCache->SetBufferARB (GL_ARRAY_BUFFER_ARB, 0);
  stateCache->SetBufferARB (GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

  // No buffers can be locked now
  lockedRenderBuffers.Empty ();
}

csGLVBOBufferManager::VBOSlot* csGLVBOBufferManager::GetVBOSlot (
  iRenderBuffer* buffer)
{
  // See if an existing slot exists
  VBOSlot* slot = 0;

  slot = renderBufferMappings.Get (buffer, 0);
  const bool isIndexBuffer = buffer->IsIndexBuffer ();
  size_t slotType =
    isIndexBuffer ? VBO_BUFFER_IS_INDEX : VBO_BUFFER_IS_VERTEX;
  if (buffer->GetBufferType() == CS_BUF_STATIC)
    slotType |= VBO_BUFFER_IS_STATIC;

  if (!slot)
  {
    const size_t slotSize = buffer->GetSize ();
    const size_t slotSizePO2 = GetSlotSizePO2 (slotSize);
    

    // No existing, try to get a new one
    slot = GetFreeVBOSlot (slotSizePO2, slotType);

    // No free one, try to allocate a buffer
    if (!slot)
    {     
      VBOBuffer* vboBuffer = GetNewVBOBuffer (slotSize, slotSizePO2, slotType);

      if (!vboBuffer)
      {
        // Try to evict a slot and use that
        slot = TryFreeVBOSlot (slotSizePO2, slotType);
      }
      else
      {
        slot = vboBuffer->vboSlots;        
      }      
    }

    if (slot)
    {
      slot->renderBuffer = buffer;
      slot->bufferVersion = ~0;
      slot->slotAge = ~0;
      SetSlotUsed (slot);

      buffer->SetCallback (this);
      renderBufferMappings.Put (buffer, slot);
      

    }
  }

  if (slot)
  {
    // Managed to get one, mark it as used
    slot->slotAge |= 0x80000000;

    if (slot->bufferVersion != buffer->GetVersion ())
    {
      ProfileScope _profile (G3D, "Buffer update");
      
      GLuint vboID;
      size_t offset;

      GetSlotIdAndOffset (slot, vboID, offset);
      stateCache->SetBufferARB (VBO_BUFFER_GL_TYPE[slotType], vboID);
      stateCache->ApplyBufferBinding (isIndexBuffer
	? csGLStateCacheContext::boIndexArray
	: csGLStateCacheContext::boElementArray);
      
      //Copy data into it
      void* bufferData = buffer->Lock (CS_BUF_LOCK_READ);
      extensionManager->glBufferSubDataARB (VBO_BUFFER_GL_TYPE[slotType],
        (GLsizei)offset, (GLsizei)buffer->GetSize (), bufferData);
      buffer->Release ();

      slot->bufferVersion = buffer->GetVersion ();
    }
  }

  return slot;
}

csGLVBOBufferManager::VBOSlot* csGLVBOBufferManager::GetFreeVBOSlot (
  size_t slotSizePO2, size_t slotType)
{
  // We don't cache and reuse big buffers, so theres never any free slots
  if (!IsSizePO2Slotted  (slotSizePO2))
    return 0;

  // Scan VBO buffers of given type for one with free slots
  const size_t slotSizeIdx = slotSizePO2 - VBO_MIN_SLOT_SIZE_PO2;
  const size_t numSlots = VBO_SLOT_PER_BUFFER[slotSizeIdx];
  const size_t numSlotBitmap = (numSlots + 31) / 32;

  VBOBuffer* buffer = vboBufferList[slotType][slotSizeIdx];

  // Scan over the buffers to find one with slots
  while (buffer)
  {
    uint32* bitmap = buffer->freeBitmap;
    // Scan the bitmap to find empty slots
    // Scan 32 bits at a time
    for (size_t bunchIdx = 0; bunchIdx < numSlotBitmap; bunchIdx++, bitmap++)
    {
      unsigned long localIndex;
      bool foundSlot = CS::Utility::BitOps::ScanBitForward (*bitmap, localIndex);
      if (foundSlot)
      {
        size_t slotIndex = 32*bunchIdx + localIndex;
        return buffer->vboSlots + slotIndex;
      }
    }

    buffer = buffer->nextBuffer;
  }

  return 0;//none found..let caller allocate one
}

csGLVBOBufferManager::VBOSlot* csGLVBOBufferManager::TryFreeVBOSlot (
  size_t slotSizePO2, size_t slotType)
{
  // We don't cache and reuse big buffers, so theres never any free slots
  if (!IsSizePO2Slotted (slotSizePO2))
    return 0;

  const size_t slotSizeIdx = slotSizePO2 - VBO_MIN_SLOT_SIZE_PO2;
  const size_t numSlots = VBO_SLOT_PER_BUFFER[slotSizeIdx];
  VBOBuffer* buffer = vboBufferList[slotType][slotSizeIdx];

  VBOSlot* lowestSlot = 0;
  size_t lowestSlotAge = ~0;
  
  while (buffer)
  {
    for (size_t i = 0; i < numSlots; ++i)
    {
      VBOSlot* slot = &buffer->vboSlots[i];
      size_t age = (slot->slotAge >>= 1);

      if ((age < lowestSlotAge) &&
        !lockedRenderBuffers.Contains (slot->renderBuffer))
      {
        lowestSlot = slot;
        lowestSlotAge = age;
      }
    }

    buffer = buffer->nextBuffer;
  }

  // We might (or might not) have a slot to use here
  if (lowestSlot)
  {
    // Release it without deallocating it
    ReleaseVBOSlot (lowestSlot, false);
  }

  return lowestSlot;
}

void csGLVBOBufferManager::ReleaseVBOSlot (VBOSlot* slot, bool deallocate /* = true */)
{
  // Mark it as free
  VBOBuffer* buffer = slot->vboBuffer;

  ClearSlotUsed (slot);

  renderBufferMappings.DeleteAll (slot->renderBuffer);
  slot->renderBuffer = 0;
  slot->slotAge = 0;

  // Deallocate it
  const size_t buffSizePO2 = GetSlotSizePO2 (buffer->slotSize);
  if (!IsSizePO2Slotted (buffSizePO2))
  {
    // Big buffer
    vboBigBuffers[buffer->bufferType].DeleteIndexFast ((size_t)buffer->nextBuffer);
  }
  else
  {
    if (buffer->prevBuffer == 0 && buffer->nextBuffer == 0)
      return; // Don't remove last one

    if (!deallocate)
      return;

    // Clean up buffer if empty
    // Check first 0 to n-2 buffer bitmaps
    uint32* bitmap = buffer->freeBitmap;
    size_t numSlots = buffer->numberOfSlots;
    const size_t numSlotBitmaps = (numSlots+31)/32;
    for (size_t i = 0; i < numSlotBitmaps-1; ++i)
    {
      numSlots -= 32;
      if (~(*bitmap++))
        return; // One of the bitmaps contained a 0, meaning non-free slot, no removal
    }
    for (size_t i = 0; i < numSlots; ++i)
    {
      if (!(*bitmap & (1<<i)))
        return; // One of the allocated slots are not free, no removal
    }

    // Normal one
    // Start by unlinking it
    if (buffer->prevBuffer)
      buffer->prevBuffer->nextBuffer = buffer->nextBuffer;
    else
      // No prev, must be first
      vboBufferList[buffer->bufferType][buffSizePO2 - VBO_MIN_SLOT_SIZE_PO2] = buffer->nextBuffer;

    if (buffer->nextBuffer)
      buffer->nextBuffer->prevBuffer = buffer->prevBuffer;
  }

  // Remove it
  FreeVBOBuffer (buffer);
}

csGLVBOBufferManager::VBOBuffer* csGLVBOBufferManager::GetNewVBOBuffer (
  size_t slotSize, size_t slotSizePO2, size_t slotType)
{
  VBOBuffer* buffer = 0;

  size_t numSlots;
  size_t numSlotBitmap;
  bool bigBuffer = false;

  const size_t slotSizeIdx = slotSizePO2 - VBO_MIN_SLOT_SIZE_PO2;
  if (IsSizePO2Slotted (slotSizePO2))
  {    
    numSlots = VBO_SLOT_PER_BUFFER[slotSizeIdx];
    numSlotBitmap = (numSlots + 31) / 32;
    bigBuffer = false;
    slotSize = (ptrdiff_t(1) << slotSizePO2);
  }
  else
  {
    // Bigger, get a separate one
    numSlots = 1;
    numSlotBitmap = 1;
    bigBuffer = true;
  }

  if (numSlots*slotSize+currentVBOAllocation > maxVBOAllocation)
  {
    // hit limit, bail
    return 0;
  }

  const size_t allocationSize = sizeof (VBOBuffer) + 
    sizeof (uint32) * numSlotBitmap + sizeof (VBOSlot) * numSlots;

  // Allocate it all in one call
  uint8* rawBuffer = static_cast<uint8*> (cs_malloc (allocationSize));
  memset (rawBuffer, 0, allocationSize);

  buffer = reinterpret_cast<VBOBuffer*> (rawBuffer);
  buffer->slotSize = slotSize;
  buffer->numberOfSlots = numSlots;
  buffer->bufferType = slotType;
  buffer->freeBitmap = reinterpret_cast<uint32*> (rawBuffer + sizeof (VBOBuffer));
  buffer->vboSlots = reinterpret_cast<VBOSlot*> (
    rawBuffer + sizeof (VBOBuffer) + sizeof (uint32) * numSlotBitmap);    

  // Set slot defaults
  for (size_t slotIndex = 0; slotIndex < numSlots; ++slotIndex)
  {
    const size_t slotBitmapIdx = slotIndex >> 5;
    const size_t slotBitIdx = slotIndex & 31;

    buffer->freeBitmap[slotBitmapIdx] |= 1 << slotBitIdx;
    buffer->vboSlots[slotIndex].vboBuffer = buffer;
  }

  // Save it
  if (bigBuffer)
  {
    buffer->nextBuffer = (VBOBuffer*)vboBigBuffers[slotType].Push (buffer);
  }
  else
  {
    buffer->nextBuffer = vboBufferList[slotType][slotSizeIdx];
    buffer->prevBuffer = 0;

    if (vboBufferList[slotType][slotSizeIdx])
      vboBufferList[slotType][slotSizeIdx]->prevBuffer = buffer;
    vboBufferList[slotType][slotSizeIdx] = buffer;
  }

  // Setup VBO area
  extensionManager->glGenBuffersARB (1, &(buffer->vboID));
  CS_ASSERT(buffer->vboID);
  stateCache->SetBufferARB (VBO_BUFFER_GL_TYPE[slotType], buffer->vboID);
  stateCache->ApplyBufferBinding ((slotType & VBO_BUFFER_IS_INDEX)
    ? csGLStateCacheContext::boIndexArray
    : csGLStateCacheContext::boElementArray);
  extensionManager->glBufferDataARB (VBO_BUFFER_GL_TYPE[slotType], 
    (GLsizei)(buffer->slotSize*buffer->numberOfSlots), 0,
    VBO_BUFFER_GL_USAGE[slotType]);
  stateCache->SetBufferARB (VBO_BUFFER_GL_TYPE[slotType], 0);

  currentVBOAllocation += buffer->slotSize*buffer->numberOfSlots;

  return buffer;
}

void csGLVBOBufferManager::FreeVBOBuffer (VBOBuffer* buffer)
{
  for (size_t i = 0; i < VBO_BUFFER_TYPE_COUNT; ++i)
  {
    if (stateCache->GetBufferARB (VBO_BUFFER_GL_TYPE[i]) == buffer->vboID)
      stateCache->SetBufferARB (VBO_BUFFER_GL_TYPE[i], 0);
  }

  currentVBOAllocation -= buffer->slotSize * buffer->numberOfSlots;

  extensionManager->glDeleteBuffersARB (1, &(buffer->vboID));
  cs_free (buffer);
}

void csGLVBOBufferManager::RenderBufferDestroyed (iRenderBuffer* buffer)
{
  // Remove it from the slots
  VBOSlot* slot = renderBufferMappings.Get (buffer, 0);

  if (slot)
  {
    ReleaseVBOSlot (slot);
  }
}

static csString ByteFormat (size_t size)
{
  csString str;
  if (size >= 1024*1024)
    str.Format ("%4zu MB", size / (1024*1024));
  else if (size >= 1024)
    str.Format ("%4zu KB", size / (1024));
  else
    str.Format ("%4zu By", size);
  return str;
}


void csGLVBOBufferManager::DumpStats ()
{
  csPrintf ("VBO memory statistics:\n");
  csPrintf ("VB Buffers\n");
  DumpStatsBufferType (VBO_BUFFER_IS_VERTEX);
  csPrintf ("Static VB Buffers\n");
  DumpStatsBufferType (VBO_BUFFER_IS_VERTEX | VBO_BUFFER_IS_STATIC);

  csPrintf ("IB Buffers\n");
  DumpStatsBufferType (VBO_BUFFER_IS_INDEX);
  csPrintf ("Static IB Buffers\n");
  DumpStatsBufferType (VBO_BUFFER_IS_INDEX | VBO_BUFFER_IS_STATIC);

  csPrintf ("VBO allocation: %s / %s (%3u %%)\n", 
    ByteFormat (currentVBOAllocation).GetDataSafe (),
    ByteFormat (maxVBOAllocation).GetDataSafe (),
    (uint)((float)currentVBOAllocation/maxVBOAllocation*100));
}



namespace
{
  struct SizeCountStruct
  {
    SizeCountStruct (size_t s) : size (s), count (0)
    {
    }

    bool operator< (const SizeCountStruct& other)
    {
      return size < other.size;
    }

    size_t size;
    size_t count;
  };
}

void csGLVBOBufferManager::DumpStatsBufferType (size_t type)
{
  csPrintf ("Fixed size buffers\n");
  csPrintf ("SS            NB       NS       SU      SU%%\n");

  size_t vboSize = 0;
  size_t sysmemSize = 0;

  for (size_t i = 0; i < VBO_NUM_SLOT_SIZES; ++i)
  {
    VBOBuffer* buffer = vboBufferList[type][i];

    size_t numberOfSlots = buffer ? buffer->numberOfSlots : 0;
    const size_t slotSize = 1 << (i + VBO_MIN_SLOT_SIZE_PO2);
    const size_t slotBitmapSize = (numberOfSlots + 31) / 32;
    size_t numBuffers = 0;
    size_t numSlotsUsed = 0;

    // Accumulate statistics
    while (buffer)
    {
      numBuffers++;
      numSlotsUsed += numberOfSlots;

      vboSize += slotSize * numberOfSlots;
      sysmemSize += sizeof (VBOBuffer) + sizeof(VBOSlot) * numberOfSlots + 
        sizeof(int32) * slotBitmapSize;

      uint32* bitmap = buffer->freeBitmap;
      for (size_t i = 0; i < slotBitmapSize; ++i)
        numSlotsUsed -= CS::Utility::BitOps::ComputeBitsSet (*bitmap++);

      buffer = buffer->nextBuffer;
    } 
    int percentSU = (!numberOfSlots || !numBuffers) ? 0 : 
      int(100.0f * numSlotsUsed / (numberOfSlots*numBuffers));

    csPrintf ("%s %8zu %8zu %8zu %8u\n", ByteFormat (slotSize).GetDataSafe (),
      numBuffers, numberOfSlots, numSlotsUsed, percentSU);
  }

  if (vboBigBuffers[type].GetSize ())
  {
    csArray<size_t> bigBufferSizes;
    for (size_t i = 0; i < vboBigBuffers[type].GetSize (); ++i)
    {    
      size_t size = vboBigBuffers[type][i]->slotSize;
      bigBufferSizes.Push (size);

      vboSize += size;
      sysmemSize += sizeof (VBOBuffer) + sizeof(VBOSlot) + sizeof(int32);
    }
    bigBufferSizes.Sort ();

    csPrintf ("Big buffers\n");
    size_t count = 0;
    size_t size = 0;
    for (size_t i = 0; i < bigBufferSizes.GetSize () - 1; ++i)
    {
      count++;
      size = bigBufferSizes[i];
      if (size != bigBufferSizes[i+1])
      {
        csPrintf ("%s %8zu\n", ByteFormat (size).GetDataSafe (), count);
        count = 0;
      }
    }
    if (count > 0)
    {
      csPrintf ("%s %8zu\n", ByteFormat (size).GetDataSafe (), count);
    }
  }

  csPrintf ("Total VBO size:    %s\n", ByteFormat (vboSize).GetDataSafe ());
  csPrintf ("Total sysmem size: %s\n", ByteFormat (sysmemSize).GetDataSafe ());
}

}
CS_PLUGIN_NAMESPACE_END(gl3d)
