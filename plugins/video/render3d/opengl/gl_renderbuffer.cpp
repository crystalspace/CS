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

#include "cssysdef.h"
#include "csutil/scf.h"
#include "csgfx/renderbuffer.h"

#include "gl_render3d.h"
#include "gl_renderbuffer.h"

CS_LEAKGUARD_IMPLEMENT (csGLVBOBufferManager);

SCF_IMPLEMENT_IBASE (csGLVBOBufferManager)
SCF_IMPLEMENT_IBASE_END

//-----------------------------------------------------------------

void csGLVBOBufferManager::ParseByteSize (const char* sizeStr, size_t& size)
{
  const char* end = sizeStr + strspn (sizeStr, "0123456789"); 	 
  size_t sizeFactor = 1; 	 
  if ((*end == 'k') || (*end == 'K')) 	 
    sizeFactor = 1024; 	 
  else if ((*end == 'm') || (*end == 'M')) 	 
    sizeFactor = 1024*1024; 	 
  else if (*end != 0)
  { 	 
    Report (CS_REPORTER_SEVERITY_WARNING, 	 
      "Unknown suffix '%s' in maximum buffer size '%s'.", end, sizeStr); 	 
    sizeFactor = 0; 	 
  } 	 
  if (sizeFactor != 0) 	 
  { 	 
    if (sscanf (sizeStr, "%d", &size) != 0) 	 
      size *= sizeFactor; 	 
    else 	 
      Report (CS_REPORTER_SEVERITY_WARNING, 	 
        "Invalid buffer size '%s'.", sizeStr); 	 
  }
}

static csString ByteFormat (size_t size)
{
  csString str;
  if (size >= 1024*1024)
    str.Format ("%d MB", size / (1024*1024));
  else if (size >= 1024)
    str.Format ("%d KB", size / (1024));
  else
    str.Format ("%d Byte", size);
  return str;
}

csGLVBOBufferManager::csGLVBOBufferManager (csGLExtensionManager *ext, 
					    csGLStateCache *state,
                                            iObjectRegistry* p)
  : ext (ext), statecache (state), object_reg (p),
    verbose (false), superVerbose (false)
{
  SCF_CONSTRUCT_IBASE(0);
  
  config.AddConfig(p, "/config/r3dopengl.cfg");
  
  csRef<iVerbosityManager> verbosemgr (
    CS_QUERY_REGISTRY (p, iVerbosityManager));
  if (verbosemgr)
  {
    verbose = verbosemgr->CheckFlag ("renderer");
    if (verbose) superVerbose = verbosemgr->CheckFlag ("vbo");
  }

  size_t vbSize = 8*1024*1024;
  ParseByteSize (config->GetStr ("Video.OpenGL.VBO.VBsize", "8m"), vbSize);
  size_t ibSize = 8*1024*1024;
  ParseByteSize (config->GetStr ("Video.OpenGL.VBO.IBsize", "8m"), ibSize);

  if (verbose) Report (CS_REPORTER_SEVERITY_NOTIFY, 
    "Setting up VBO buffers, VB: %s IB: %s",
    ByteFormat (vbSize).GetData(), ByteFormat (ibSize).GetData());

  vertexBuffer.bufmgr = this;
  vertexBuffer.Setup (GL_ARRAY_BUFFER_ARB, vbSize, ext);
  indexBuffer.bufmgr = this;
  indexBuffer.Setup (GL_ELEMENT_ARRAY_BUFFER_ARB, ibSize, ext);
}

csGLVBOBufferManager::~csGLVBOBufferManager ()
{
  SCF_DESTRUCT_IBASE();
}

bool csGLVBOBufferManager::ActivateBuffer (iRenderBuffer *buffer)
{
  csGLVBOBufferSlot *slot = 0;
  RenderBufferAux* auxData = bufferData.GetElementPointer (buffer);
  if ((auxData != 0) && (auxData->vboSlot != 0) 
    && (auxData->vboSlot->renderBuffer == buffer))
  {
    slot = auxData->vboSlot;
    //we already have a slot, use it
    if (buffer->GetVersion() != slot->lastCachedVersion)
    {
      Precache (buffer, slot);
    }
  }
  else
  {
    //need a new slot
    slot = FindEmptySlot (buffer->GetSize(), buffer->IsIndexBuffer());
    AttachBuffer (slot, buffer);
    Precache (buffer, slot);
  }
  ActivateVBOSlot (slot);
  return true;
}

bool csGLVBOBufferManager::DeactivateBuffer (iRenderBuffer *buffer)
{
  RenderBufferAux* auxData = bufferData.GetElementPointer (buffer);
  if ((auxData != 0) && (auxData->vboSlot != 0)
    && (auxData->vboSlot->renderBuffer == buffer))
  {
    DeactivateVBOSlot (auxData->vboSlot);
  }
  return true;
}

void csGLVBOBufferManager::BufferRemoved (iRenderBuffer *buffer)
{
  RenderBufferAux* auxData = bufferData.GetElementPointer (buffer);
  if ((auxData != 0) && (auxData->vboSlot != 0)
    && (auxData->vboSlot->renderBuffer == buffer))
  {
    DeactivateBuffer (buffer);
    if (auxData->vboSlot->separateVBO)
    {
      ext->glDeleteBuffersARB (0, &auxData->vboSlot->vboID);
    }
    delete auxData->vboSlot;
    auxData->vboSlot = 0;
  }  
}

void csGLVBOBufferManager::DeactivateVBO ()
{
  statecache->SetBufferARB (GL_ARRAY_BUFFER_ARB, 0);
  statecache->SetBufferARB (GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}

void* csGLVBOBufferManager::RenderLock (iRenderBuffer* buffer, 
					csGLRenderBufferLockType type)
{
  iRenderBuffer* master = buffer->GetMasterBuffer();
  iRenderBuffer* useBuffer = master ? master : buffer;
  ActivateBuffer (useBuffer);
  RenderBufferAux* auxData = bufferData.GetElementPointer (useBuffer);
  if (auxData == 0) return (void*)-1;
  return (void*)(auxData->vbooffset + buffer->GetOffset());
}

void csGLVBOBufferManager::RenderRelease (iRenderBuffer* buffer)
{
  iRenderBuffer* master = buffer->GetMasterBuffer();
  DeactivateBuffer (master ? master : buffer);
}

void csGLVBOBufferManager::ActivateVBOSlot (csGLVBOBufferSlot *slot)
{
  statecache->SetBufferARB (slot->vboTarget, slot->vboID);
  
  slot->locked = true;

  if (slot->separateVBO) return;
  TouchSlot (slot);

  //some stats
  if (slot->indexBuffer) indexBuffer.slots[slot->listIdx].slotsActivatedThisFrame++;
  else vertexBuffer.slots[slot->listIdx].slotsActivatedThisFrame++;
}

void csGLVBOBufferManager::DeactivateVBOSlot (csGLVBOBufferSlot *slot)
{
  slot->locked = false;
}

void csGLVBOBufferManager::Precache (iRenderBuffer *buffer, 
                                     csGLVBOBufferSlot *slot)
{
  //slot must be active first
  ActivateVBOSlot (slot);

  void* bufferData = buffer->Lock (CS_BUF_LOCK_READ);
  ext->glBufferSubDataARB (slot->vboTarget, slot->offset, 
    buffer->GetSize(), bufferData);
  buffer->Release ();

  slot->lastCachedVersion = buffer->GetVersion();
}

csGLVBOBufferSlot* csGLVBOBufferManager::FindEmptySlot 
  (size_t size, bool ib)
{
  csGLVBOBufferSlot *slot = 0;
  if (ib)
  {
    if (size<=VBO_BIGGEST_SLOT_SIZE) slot = indexBuffer.FindEmptySlot (size);
  }
  else
  {
    if (size<=VBO_BIGGEST_SLOT_SIZE) slot = vertexBuffer.FindEmptySlot (size);
  }

  if (size>VBO_BIGGEST_SLOT_SIZE || slot == 0)
  {
    GLuint vboid = AllocateVBOBuffer (size, ib);
    slot = new csGLVBOBufferSlot;
    slot->vboID = vboid;
    slot->indexBuffer = ib;
    slot->vboTarget = ib ? GL_ELEMENT_ARRAY_BUFFER_ARB : GL_ARRAY_BUFFER_ARB;
    slot->offset = 0;
    slot->separateVBO = true;
  }

  return slot;
}

GLuint csGLVBOBufferManager::AllocateVBOBuffer (size_t size, bool ib)
{
  GLuint vboid;
  GLenum usage = ib ? GL_ELEMENT_ARRAY_BUFFER_ARB : GL_ARRAY_BUFFER_ARB;
  ext->glGenBuffersARB (1, &vboid);
  ext->glBindBufferARB (usage, vboid);
  ext->glBufferDataARB (usage, size, 0, GL_DYNAMIC_DRAW_ARB);
  ext->glBindBufferARB (usage, 0);
  return vboid;
}

void csGLVBOBufferManager::DetachBuffer (csGLVBOBufferSlot *slot)
{
  RenderBufferAux* auxData = bufferData.GetElementPointer (
    slot->renderBufferPtr);
  if (auxData == 0) return;
  slot->renderBuffer = 0;
  slot->renderBufferPtr = 0;
  slot->lastCachedVersion = 0;
  bufferData.DeleteAll (slot->renderBufferPtr);
}

void csGLVBOBufferManager::AttachBuffer (csGLVBOBufferSlot *slot, 
					 iRenderBuffer* buffer)
{
  RenderBufferAux auxData;
  if ((slot->inUse) && (slot->renderBuffer != buffer))
    DetachBuffer (slot);
  slot->renderBuffer = buffer;
  slot->renderBufferPtr = buffer;
  auxData.vboSlot = slot;
  auxData.vbooffset = slot->offset;
  const csRenderBufferComponentType componentType = buffer->GetComponentType();
  bufferData.PutUnique (buffer, auxData);
}

void csGLVBOBufferManager::DumpStats ()
{
  Report (CS_REPORTER_SEVERITY_DEBUG, "-------------------------------------------");
  Report (CS_REPORTER_SEVERITY_DEBUG, " VBO statistics ");
  Report (CS_REPORTER_SEVERITY_DEBUG, "-------------------------------------------");
  Report (CS_REPORTER_SEVERITY_DEBUG, "Vertex storage: %d MB (%d byte)", 
    vertexBuffer.size/(1024*1024), vertexBuffer.size);
  Report (CS_REPORTER_SEVERITY_DEBUG, "Index storage:  %d MB (%d byte)", 
    indexBuffer.size/(1024*1024), indexBuffer.size);
  
  if (superVerbose)
  {
    Report (CS_REPORTER_SEVERITY_DEBUG, "-------------------------------------------");
    Report (CS_REPORTER_SEVERITY_DEBUG, " Vertex storage - Allocation report ");
    Report (CS_REPORTER_SEVERITY_DEBUG, "-------------------------------------------");
    Report (CS_REPORTER_SEVERITY_DEBUG, " Slotsize Count    Total    Allocated  Used  Reused");
    unsigned int i;
    unsigned int countTotal = 0;
    
    for (i=0;i<VBO_NUMBER_OF_SLOTS;i++)
    {
      Report (CS_REPORTER_SEVERITY_DEBUG, " %8d %5d   %8d    %5d   %5d  %5d",
        vertexBuffer.slots[i].slotSize, vertexBuffer.slots[i].totalCount,
        vertexBuffer.slots[i].slotSize * vertexBuffer.slots[i].totalCount,
        vertexBuffer.slots[i].usedSlots, vertexBuffer.slots[i].slotsActivatedLastFrame,
        vertexBuffer.slots[i].slotsReusedLastFrame);
      countTotal += vertexBuffer.slots[i].totalCount;
    }
    Report (CS_REPORTER_SEVERITY_DEBUG, "-------------------------------------------");
    Report (CS_REPORTER_SEVERITY_DEBUG, " Total:   %5d   %8d",
      countTotal, vertexBuffer.size);
    Report (CS_REPORTER_SEVERITY_DEBUG, "-------------------------------------------");


    Report (CS_REPORTER_SEVERITY_DEBUG, "-------------------------------------------");
    Report (CS_REPORTER_SEVERITY_DEBUG, " Index storage - Allocation report ");
    Report (CS_REPORTER_SEVERITY_DEBUG, "-------------------------------------------");
    Report (CS_REPORTER_SEVERITY_DEBUG, " Slotsize Count    Total    Allocated  Used  Reused");
    countTotal = 0;

    for (i=0;i<VBO_NUMBER_OF_SLOTS;i++)
    {
      Report (CS_REPORTER_SEVERITY_DEBUG, " %8d %5d   %8d    %5d   %5d  %5d",
        indexBuffer.slots[i].slotSize, indexBuffer.slots[i].totalCount,
        indexBuffer.slots[i].slotSize * indexBuffer.slots[i].totalCount,
        indexBuffer.slots[i].usedSlots, indexBuffer.slots[i].slotsActivatedLastFrame,
        indexBuffer.slots[i].slotsReusedLastFrame);
      countTotal += indexBuffer.slots[i].totalCount;
    }
    Report (CS_REPORTER_SEVERITY_DEBUG, "-------------------------------------------");
    Report (CS_REPORTER_SEVERITY_DEBUG, " Total:   %5d   %8d",
      countTotal, indexBuffer.size);
    Report (CS_REPORTER_SEVERITY_DEBUG, "-------------------------------------------");

  }
}

void csGLVBOBufferManager::ResetFrameStats ()
{
  unsigned int i;
  for (i = 0; i < VBO_NUMBER_OF_SLOTS; i++)
  {
    vertexBuffer.slots[i].slotsActivatedLastFrame = vertexBuffer.slots[i].slotsActivatedThisFrame;
    vertexBuffer.slots[i].slotsActivatedThisFrame = 0;

    vertexBuffer.slots[i].slotsReusedLastFrame = vertexBuffer.slots[i].slotsReusedThisFrame;
    vertexBuffer.slots[i].slotsReusedThisFrame = 0;
  }

  for (i = 0; i < VBO_NUMBER_OF_SLOTS; i++)
  {
    indexBuffer.slots[i].slotsActivatedLastFrame = indexBuffer.slots[i].slotsActivatedThisFrame;
    indexBuffer.slots[i].slotsActivatedThisFrame = 0;

    indexBuffer.slots[i].slotsReusedLastFrame = indexBuffer.slots[i].slotsReusedThisFrame;
    indexBuffer.slots[i].slotsReusedThisFrame = 0;
  }
}

csGLVBOBufferManager::csGLVBOBuffer::~csGLVBOBuffer ()
{
  for (unsigned int i = 0; i < VBO_NUMBER_OF_SLOTS; i++)
  {
    csGLVBOBufferSlot *c = slots[i].head, *t;
    while (c)
    {
      t = c;
      c = c->next;
      delete t;
    }
  }
}

void csGLVBOBufferManager::csGLVBOBuffer::Setup (GLenum usage, 
  size_t totalSize, csGLExtensionManager *ext)
{
  bool isIndex = (usage == GL_ELEMENT_ARRAY_BUFFER_ARB ? true:false);
  vboTarget = usage;

  //round to lower 4mb boundary (for simplicity)
  unsigned int numBlocks = (totalSize/(8*1024*1024)); //number of 8Mb chunks to use
  if (numBlocks == 0) numBlocks = 1;
  size = numBlocks * (8*1024*1024);

  ext->glGenBuffersARB (1, &vboID);
  ext->glBindBufferARB (usage, vboID);
  ext->glBufferDataARB (usage, size, 0, GL_DYNAMIC_DRAW_ARB);
  ext->glBindBufferARB (usage, 0);

  //setup initial layout like below
  /* Size       Num     Total
      256      1024     262144 
      512       512     262144
     1024       512     524288
     2048       512    1048576
     4096       128     524288
     8192        64     524288
    16384        32     524288
    32768        16     524288
    65536        16     524288
   131072         8     524288
   262144         4    1048576
   524288         2    1048576
   Total: 8mb
   */
  uint countTable[VBO_NUMBER_OF_SLOTS] = 
    {1024, 512, 512, 512, 128, 64, 32, 16, 16, 8, 4, 2};
  size_t currentOffset = 0;
  unsigned int i;
  for (i = 0; i < VBO_NUMBER_OF_SLOTS; i++)
  {
    size_t currSize = GetSizeFromIndex (i);
    uint count = countTable[i]*numBlocks;
    slots[i].totalCount = count;
    slots[i].slotSize = currSize;

    while (count--)
    {
      csGLVBOBufferSlot *newslot = new csGLVBOBufferSlot;
      newslot->indexBuffer = isIndex;
      newslot->vboID = vboID;
      newslot->vboTarget = usage;
      newslot->offset = currentOffset;
      newslot->listIdx = i;
      
      slots[i].PushFront (newslot);

      currentOffset += currSize;
    }
  }
}

//comparefunction used below
int VBOSlotCompare(csGLVBOBufferSlot* const& r1, csGLVBOBufferSlot* const& r2)
{
  if (r1->offset < r2->offset) return -1;
  else if (r1->offset > r2->offset) return 1;
  else return 0;
}

//helperstruct to method below
struct SlotSortStruct
{
  csArray<csGLVBOBufferSlot*> slotList;
  size_t firstOffset;
  size_t lastOffset;
  SlotSortStruct() : firstOffset (0), lastOffset (0) {}

  static int CompareFunc (SlotSortStruct* const& r1, SlotSortStruct* const& r2)
  {
    if (r1->firstOffset < r2->firstOffset) return -1;
    else if (r1->firstOffset > r2->firstOffset) return -1;
    else return 0;
  }
};

csGLVBOBufferSlot* csGLVBOBufferManager::csGLVBOBuffer::FindEmptySlot (
  size_t size, bool splitStarted)
{
  uint idx = GetIndexFromSize (size);
  CS_ASSERT (idx < VBO_NUMBER_OF_SLOTS);
  csGLVBOBufferSlot * slot = slots[idx].head;

  if (!slot || slot->inUse)
  {
    float totalCount = (float)slots[idx].totalCount;
    float useRate = 0;
    float reuseRate = 0;
    if (slots[idx].totalCount>0)
    {
      useRate = (float)slots[idx].usedSlots / totalCount;
      reuseRate = (float)slots[idx].slotsReusedLastFrame / totalCount;
    }

    //need to find a new one..
    int idx2 = idx+1;
  
    if ((reuseRate>0.25 || useRate>1.25 || totalCount <= 1 || !slot || slot->locked) )
    {
      csGLVBOBufferSlot *biggerSlot = 0;
      if (idx2 < VBO_NUMBER_OF_SLOTS) 
        biggerSlot = FindEmptySlot (GetSizeFromIndex (idx2), true);

      if (biggerSlot)
      {
	bufmgr->DetachBuffer (biggerSlot);

        size_t currentOffset = biggerSlot->offset;
        //split biggerSlot
        slots[idx2].Remove (biggerSlot);
        slots[idx2].totalCount--;
        if (biggerSlot->inUse) slots[idx2].usedSlots--;

        for (int a=0;a<2;a++)
        {
          csGLVBOBufferSlot *newslot = new csGLVBOBufferSlot;
          
          newslot->indexBuffer = biggerSlot->indexBuffer;
          newslot->vboID = vboID;
          newslot->vboTarget = biggerSlot->vboTarget;
          newslot->offset = currentOffset;
          newslot->listIdx = idx;

          slots[idx].PushFront (newslot);
          slots[idx].totalCount++;

          currentOffset += slots[idx].slotSize;
        }
        delete biggerSlot;
      }
      else if (!splitStarted && idx != 0)
      {
        //only try to merge if we are not in the process of splitting blocks
        idx2 = idx-1;
        uint blocksToMerge = 0;
        int minIdx = -1;
        float blockMergePercent = 0, minBlockMergePercent = 1;
        float mergeDestFullPercent = 0;
        //try to determin best block to merge from
        do
        {
          if (slots[idx2].totalCount == 0) continue;
          blocksToMerge = 1<<(idx-idx2);
          blockMergePercent = (float)blocksToMerge / (float)slots[idx2].totalCount;
          mergeDestFullPercent = (float)slots[idx2].usedSlots / (float)slots[idx2].totalCount;
          if (blockMergePercent < 0.4 && mergeDestFullPercent < 0.6)
          {
            if (blockMergePercent < minBlockMergePercent)
            {
              minBlockMergePercent = blockMergePercent;
              minIdx = idx2;
            }
          }
        } while (idx2-- > 0);

        if (minIdx >= 0)
        {
          blocksToMerge = 1<<(idx-minIdx);
          //found a block, merge them
          //start by sorting all blocks in order of offset
          
          //try to find blocksToMerge blocks after eachother
          size_t blocksize = GetSizeFromIndex (minIdx);
          int slotsFound = 1, sortSlotidx = -1;
          uint j = 0;

          csGLVBOBufferSlot* tmpSlot = slots[minIdx].head;

          //loop over all slots, try to map up "blocksToMerge" slots in order
          csPDelArray<SlotSortStruct> sortList;

          while (tmpSlot && sortSlotidx < 0)
          {
            if (!tmpSlot->locked)
            {
              bool handled = false;
              //check if we follow on any of the already existant slots
              for (j = 0; j<sortList.Length (); j++)
              {
                SlotSortStruct *t = sortList[j];
                if (tmpSlot->offset == (sortList[j]->lastOffset+blocksize))
                {
                  //follows directly
                  sortList[j]->lastOffset = tmpSlot->offset;
                  sortList[j]->slotList.Push (tmpSlot);
                  handled = true;
                }
                if (tmpSlot->offset == (sortList[j]->firstOffset-blocksize))
                {
                  //follows directly
                  sortList[j]->firstOffset = tmpSlot->offset;
                  sortList[j]->slotList.Push (tmpSlot);
                  handled = true;
                }
                
                if (sortList[j]->slotList.Length ()>=blocksToMerge)
                {
                  sortSlotidx = j;
                  break;
                }

                if (tmpSlot->offset < sortList[j]->firstOffset ||handled) break;
              }

              //ok, don't follow, add it to a new pile
              if (!handled)
              {
                SlotSortStruct* st = new SlotSortStruct;
                st->firstOffset = st->lastOffset = tmpSlot->offset;
                st->slotList.Push (tmpSlot);
                sortList.InsertSorted (st, SlotSortStruct::CompareFunc);
              }

              //then run through all slotSortStructs and merge them,
              //stop if we find one with enough slots to merge
              for (j = 0; j<sortList.Length ()-1; j++)
              {
                SlotSortStruct* s = sortList[j];
                SlotSortStruct* s2 = sortList[j+1];
                if ((sortList[j]->lastOffset+blocksize) 
                     == sortList[j+1]->firstOffset)
                {
                  sortList.DeleteIndex (j+1);
                  s->lastOffset = s2->lastOffset;
                  for (uint n=0; n < s2->slotList.Length (); n++)
                  {
                    s->slotList.Push (s2->slotList[n]);
                  }
                  delete s2;
                }

                if (sortList[j]->slotList.Length ()>blocksToMerge)
                {
                  sortSlotidx = j;
                  break;
                }
              }
            }
            tmpSlot = tmpSlot->next;
          }


          if (sortSlotidx >= 0)
          {
            sortList[sortSlotidx]->slotList.Sort (VBOSlotCompare);
            //ok, really found enough blocks, so merge them
            csGLVBOBufferSlot *newslot = new csGLVBOBufferSlot;
            tmpSlot = sortList[sortSlotidx]->slotList[0];

            newslot->indexBuffer = tmpSlot->indexBuffer;
            newslot->vboID = vboID;
            newslot->vboTarget = tmpSlot->vboTarget;
            newslot->offset = tmpSlot->offset;
            newslot->listIdx = idx;

            slots[idx].PushFront (newslot);
            slots[idx].totalCount++;

            for (uint i = 0; i < sortList[sortSlotidx]->slotList.Length (); i++)
            {
              tmpSlot = sortList[sortSlotidx]->slotList[i];
              //remove old
              slots[minIdx].Remove (tmpSlot);
              slots[minIdx].totalCount--;
              if (tmpSlot->inUse) slots[minIdx].usedSlots--;
              delete tmpSlot;
            }
          }
        }
      }
    }
    slot = slots[idx].head;
  }

  if (slot)
  {
    TouchSlot (slot);

    if (!splitStarted)
    {
      if (slot->inUse) slots[idx].slotsReusedThisFrame++;
      else slots[idx].usedSlots++;

      slot->inUse = true;
    }
  }
  return slot;
}
