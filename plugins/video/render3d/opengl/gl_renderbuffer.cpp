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

#include "gl_render3d.h"
#include "gl_renderbuffer.h"

CS_LEAKGUARD_IMPLEMENT (csGLRenderBuffer);
CS_LEAKGUARD_IMPLEMENT (csGLVBOBufferManager);

SCF_IMPLEMENT_IBASE (csGLRenderBuffer)
  SCF_IMPLEMENTS_INTERFACE (iRenderBuffer)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csGLVBOBufferManager)
SCF_IMPLEMENT_IBASE_END

static const size_t compSizes[] = 
{
  sizeof (char), sizeof (unsigned char), 
  sizeof (short), sizeof (unsigned short),
  sizeof (int), sizeof (unsigned int),
  sizeof (float),
  sizeof (double)
};
static const GLenum compGLtypes[] =
{
  GL_BYTE, GL_UNSIGNED_BYTE,
  GL_SHORT, GL_UNSIGNED_SHORT,
  GL_INT, GL_UNSIGNED_INT,
  GL_FLOAT,
  GL_DOUBLE
};


csGLRenderBuffer::csGLRenderBuffer (csGLVBOBufferManager * vbomgr, size_t size, 
                                    csRenderBufferType type, 
                                    csRenderBufferComponentType componentType, 
                                    int componentCount, size_t rangeStart, 
                                    size_t rangeEnd, bool copy)
  : bufferType (type), comptype (componentType), bufferSize (size), 
  compCount (componentCount), stride (0), offset (0),
  vbooffset (0), rangeStart (rangeStart), rangeEnd (rangeEnd),
  version (0), doCopy (copy), isLocked (false),
  isIndex (false), buffer (0), vboSlot (0), vbomgr (vbomgr)
{
  SCF_CONSTRUCT_IBASE (0);
  compGLType = compGLtypes[componentType];
  compSize = compSizes[componentType];

  if (doCopy) 
  {
    buffer = new unsigned char[size];
    doDelete = true;
  }
  else
    doDelete = false;
}

csGLRenderBuffer::~csGLRenderBuffer ()
{
  if (vbomgr.IsValid ()) vbomgr->DeactivateBuffer (this);
  if (doDelete) delete[] buffer;
  buffer = 0;
  SCF_DESTRUCT_IBASE ();
}

void* csGLRenderBuffer::Lock(csRenderBufferLockType lockType,
                             bool samePointer)
{
  if (isLocked) return (void*)-1;

  lastLock = lockType;
  isLocked = true;
  return (void*)buffer;
}

void csGLRenderBuffer::Release ()
{
  if (lastLock == CS_BUF_LOCK_NORMAL)
  {
    version++;
  }
  isLocked = false;
}

void csGLRenderBuffer::CopyToBuffer(const void *data, size_t length)
{
  version++;
  if (doCopy)
  {
    memcpy (buffer, data, min(bufferSize, length));
  }
  else
  {
    buffer = (unsigned char*)data;
  }
}

void csGLRenderBuffer::SetComponentType (csRenderBufferComponentType type)
{
  comptype = type;
  compGLType = compGLtypes[type];
  compSize = compSizes[type];
}

void csGLRenderBuffer::SetupSUBBuffers (int count, csRef<iRenderBuffer>* buffers)
{
  for(int i=1;i<count;i++) 
  {
    csGLRenderBufferSub *sub = new csGLRenderBufferSub (this);    
    buffers[i].AttachNew (sub);
  }
}

void* csGLRenderBuffer::RenderLock (csGLRenderBufferLockType type)
{
  if (vbomgr.IsValid ())
  {
    vbomgr->ActivateBuffer (this);
    return (void*)(vbooffset + offset);
  }
  else return buffer + offset;
}

void csGLRenderBuffer::RenderRelease ()
{
  if (vbomgr.IsValid ()) vbomgr->DeactivateBuffer (this);
}

// subbuffer
csGLRenderBufferSub::csGLRenderBufferSub (csGLRenderBuffer *buffer)
  : csGLRenderBuffer (buffer->vbomgr, buffer->bufferSize, buffer->bufferType, 
    buffer->comptype, buffer->compCount, buffer->rangeStart, buffer->rangeEnd, 
    false)
{ 
  this->doCopy = true; //always copy subbuffers
  this->doDelete = false; //never delete the data
  this->isLocked = false;

  owner = buffer;
  this->buffer = owner->buffer;
}


//-----------------------------------------------------------------

csPtr<iRenderBuffer> csGLGraphics3D::CreateRenderBuffer (size_t size, 
  csRenderBufferType type, csRenderBufferComponentType componentType,
  int componentCount, bool copy)
{
 
  csGLRenderBuffer *buf = new csGLRenderBuffer (vboManager, size, type, 
    componentType, componentCount, 0, 0, copy);
  return csPtr<iRenderBuffer> (buf);
}

csPtr<iRenderBuffer> csGLGraphics3D::CreateIndexRenderBuffer (size_t size, 
    csRenderBufferType type, csRenderBufferComponentType componentType,
    size_t rangeStart, size_t rangeEnd, bool copy)
{
  csGLRenderBuffer *buf = new csGLRenderBuffer (vboManager, size, type, 
    componentType, 1, rangeStart, rangeEnd, copy);
  buf->isIndex = true;
  return csPtr<iRenderBuffer> (buf);
}

void csGLGraphics3D::CreateInterleavedRenderBuffers (size_t size,
  csRenderBufferType type, int count, csRef<iRenderBuffer>* buffers)
{

  csGLRenderBuffer *master = new csGLRenderBuffer (vboManager, size, type, 
    CS_BUFCOMP_BYTE, 1, 0, 0, true);
  buffers[0].AttachNew (master); 

  master->SetupSUBBuffers (count, buffers);
}

//-----------------------------------------------------------------

void csGLVBOBufferManager::ParseByteSize (const char* sizeStr, size_t& size)
{
  const char* end = sizeStr + strspn (sizeStr, "0123456789"); 	 
  size_t sizeFactor = 1; 	 
  if ((*end == 'k') || (*end == 'K')) 	 
    sizeFactor = 1024; 	 
  else if ((*end == 'm') || (*end == 'M')) 	 
    sizeFactor = 1024*1024; 	 
  else 	 
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
  
  bugplug = CS_QUERY_REGISTRY (p, iBugPlug);
  config.AddConfig(p, "/config/r3dopengl.cfg");
  
  csRef<iVerbosityManager> verbosemgr (
    CS_QUERY_REGISTRY (p, iVerbosityManager));
  if (verbosemgr)
  {
    verbose = verbosemgr->CheckFlag ("renderer");
    if (verbose) superVerbose = verbosemgr->CheckFlag ("vbo");
  }
  if (!verbose) bugplug = 0; 

  size_t vbSize = 8*1024*1024;
  ParseByteSize (config->GetStr ("Video.OpenGL.VBO.VBsize", "8m"), vbSize);
  size_t ibSize = 8*1024*1024;
  ParseByteSize (config->GetStr ("Video.OpenGL.VBO.IBsize", "8m"), ibSize);

  if (verbose) Report (CS_REPORTER_SEVERITY_NOTIFY, 
    "Setting up VBO buffers, VB: %s IB: %s",
    ByteFormat (vbSize).GetData(), ByteFormat (ibSize).GetData());

  vertexBuffer.Setup (GL_ARRAY_BUFFER_ARB, vbSize, ext);
  indexBuffer.Setup (GL_ELEMENT_ARRAY_BUFFER_ARB, ibSize, ext);
}

csGLVBOBufferManager::~csGLVBOBufferManager ()
{
  //delete all slots
  for (unsigned int i=0; i < VBO_NUMBER_OF_SLOTS; i++)
  {
    csGLVBOBufferSlot *c = vertexBuffer.slots[i].head, *t;
    while (c)
    {
      t = c;
      c = c->next;
      delete t;
    }

    c = indexBuffer.slots[i].head;
    while (c)
    {
      t = c;
      c = c->next;
      delete t;
    }
  }

  SCF_DESTRUCT_IBASE();
}

bool csGLVBOBufferManager::ActivateBuffer (csGLRenderBuffer *buffer)
{
  if (buffer->bufferSize > 128*1024) //@@TODO: Better way for big buffers
  {
    buffer->vbomgr = 0;
    buffer->vboSlot = 0;
    DeactivateVBO ();
    return true;
  }

  csGLVBOBufferSlot *slot = 0;
  if (buffer->IsMasterBuffer ())
  {
    if (buffer->vboSlot && buffer->vboSlot->renderBuffer == buffer)
    {
      slot = buffer->vboSlot;
      //we already have a slot, use it
      if (buffer->version != slot->lastCachedVersion)
      {
        Precache (buffer, slot);
      }
    }
    else
    {
      //need a new slot
      slot = FindEmptySlot (buffer->bufferSize, buffer->isIndex);
      slot->AttachBuffer (buffer);
      Precache (buffer, slot);
    }
    ActivateVBOSlot (slot);
  }
  else
  {
    return ActivateBuffer (((csGLRenderBufferSub*)buffer)->owner);
  }
  return true;
}

bool csGLVBOBufferManager::DeactivateBuffer (csGLRenderBuffer *buffer)
{
  if (buffer->vboSlot)
  {
    DeactivateVBOSlot (buffer->vboSlot);
  }
  return true;
}

void csGLVBOBufferManager::DeactivateVBO ()
{
  statecache->SetBufferARB (GL_ARRAY_BUFFER_ARB, 0);
  statecache->SetBufferARB (GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}

void csGLVBOBufferManager::ActivateVBOSlot (csGLVBOBufferSlot *slot)
{
  statecache->SetBufferARB (slot->vboTarget, slot->vboID);

  slot->locked = true;
  TouchSlot (slot);

  //some stats
#ifdef CS_DEBUG
  if (slot->indexBuffer) indexBuffer.slots[slot->listIdx].slotsActivatedThisFrame++;
  else vertexBuffer.slots[slot->listIdx].slotsActivatedThisFrame++;
#endif
}

void csGLVBOBufferManager::DeactivateVBOSlot (csGLVBOBufferSlot *slot)
{
  slot->locked = false;
}

void csGLVBOBufferManager::Precache (csGLRenderBuffer *buffer, 
                                     csGLVBOBufferSlot *slot)
{
  //slot must be active first
  ActivateVBOSlot (slot);

  ext->glBufferSubDataARB (slot->vboTarget, slot->offset, 
    buffer->bufferSize, buffer->buffer);

  slot->lastCachedVersion = buffer->version;
}

csGLVBOBufferSlot* csGLVBOBufferManager::FindEmptySlot 
  (size_t size, bool ib)
{
  if (ib)
  {
    return indexBuffer.FindEmptySlot (size);
  }
  else
  {
    return vertexBuffer.FindEmptySlot (size);
  }
  return 0;
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

void csGLVBOBufferManager::csGLVBOBuffer::Setup (GLenum usage, 
  size_t totalSize, csGLExtensionManager *ext)
{
  bool isIndex = (usage == GL_ELEMENT_ARRAY_BUFFER_ARB ? true:false);

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

csGLVBOBufferSlot* csGLVBOBufferManager::csGLVBOBuffer::FindEmptySlot (size_t size)
{
  uint idx = GetIndexFromSize (size);
  CS_ASSERT (idx < VBO_NUMBER_OF_SLOTS);
  csGLVBOBufferSlot * slot = slots[idx].head;

  //@@TODO: check for locked properly
  //CS_ASSERT(!slot->locked);
  TouchSlot (slot);

#ifdef CS_DEBUG
  if (slot->inUse) slots[idx].slotsReusedThisFrame++;
  else slots[idx].usedSlots++;
#endif

  slot->inUse = true;
  return slot;
}
