/*
    Copyright (C) 2002 by Mårten Svanfeldt
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
#include "csutil/cscolor.h"
#include "csgeom/vector2.h"
#include "csgeom/vector3.h"
#include "ivideo/rndbuf.h"
#include "csutil/list.h"

#include "gl_sysbufmgr.h"
#include "gl_varbufmgr.h"
#include "gl_render3d.h"

SCF_IMPLEMENT_IBASE (csVARRenderBuffer)
  SCF_IMPLEMENTS_INTERFACE (iRenderBuffer)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csVARRenderBufferManager)
  SCF_IMPLEMENTS_INTERFACE (iRenderBufferManager)
SCF_IMPLEMENT_IBASE_END

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

int csBuddyAllocator::treeSize(int size)
{
  int s=size>>CS_BUDDY_SHIFT;               /*get size in blocks*/
  if(s<=0)                             /*sanity check: >0*/
    return -1;
  while(s>1)                           /*sanity check: power of two*/
    s=s>>1;
  if(s!=1)
    return -1;
  return (2*(size>>CS_BUDDY_SHIFT)-1)*sizeof(char);
}

int csBuddyAllocator::ptoi(void *ptr)
{
  int i;
  assert(m_Tree&&m_Size>0&&m_Height>0);
  assert((((int)ptr)&CS_BUDDY_ADD)==0);
  i=((int)ptr)>>CS_BUDDY_SHIFT;
  if(i<0||i>=m_Size)                 /*doesn't fit?*/
    return -1;
  else
    return (i+m_Size-1);             /*i is an index now*/
}

void* csBuddyAllocator::itop(int i, char level)
{
  assert(m_Tree&&m_Size>0&&m_Height>0);
  assert(i<2*m_Size-1&&i>=0);
  assert(level<m_Height);
  i-=(1<<m_Height-level-1)-1;      /*cryptic conversion i->address*/
  return (void*)((i<<level+CS_BUDDY_SHIFT));
}

void csBuddyAllocator::stabilize1(int i)
{
  assert(m_Tree&&m_Size>0&&m_Height>0);
  assert(i<2*m_Size-1&&i>0);
  while(i>0)
  {
    i=(i-1)/2;                         /*go parent*/
    m_Tree[i]=max(m_Tree[2*i+1],m_Tree[2*i+2]);
  }                                    /*parent=greater of children*/
}

void csBuddyAllocator::stabilize2(int i, char level)
{
  assert(m_Tree&&m_Size>0&&m_Height>0);
  assert(i<2*m_Size-1&&i>0);
  assert(level<m_Height);
  while(i>0)
  {
    i=(i-1)/2;                         /*go parent*/
    assert(m_Tree[i]<=level+1);
    if(m_Tree[2*i+1]>m_Tree[2*i+2])
      m_Tree[i]=m_Tree[2*i+1]; /*-use left-*/
    else if(m_Tree[2*i+1]<m_Tree[2*i+2])
      m_Tree[i]=m_Tree[2*i+2]; /*-use right-*/
    else if(m_Tree[2*i+1]==level)
      m_Tree[i]=level+1;           /*coalesce*/
    else
      m_Tree[i]=m_Tree[2*i+1]; /*otherwise, use one*/
    level++;                           /*next level*/
  }
}

void csBuddyAllocator::traverse(int i, char level)
{
  int   block_no=(i-((1<<m_Height-level-1)-1))*(1<<level);
  void *block_addr=itop(i,level);
  int   block_sz=1<<level;
  assert(m_Tree&&m_Size>0&&m_Height>0);
  assert(i<2*m_Size-1&&i>=0);
  assert(level<m_Height);
  if(m_Tree[i]==level)
    printf("#%6d, at %08x, size %6d, Free\n",block_no,(unsigned)block_addr,block_sz);
  else if(m_Tree[i]<0)
    printf("#%6d, at %08x, size %6d, Busy\n",block_no,(unsigned)block_addr,block_sz);
  else
  {
    traverse(2*i+1,level-1);
    traverse(2*i+2,level-1);
  }
}

bool csBuddyAllocator::Initialize(int size)
{
  m_Tree=0;         /*destroy previous info*/
  m_Size=0;
  if(treeSize(size)<0)                 /*sanity check*/
    return false;
  if(size<0)         /*sanity check*/
    return false;
  m_Tree= new char[treeSize(size)];

  m_Size=size=size>>CS_BUDDY_SHIFT;         /*remember*/
  
  for(m_Height=1;size>1;m_Height++)
    size=size>>1;
  
  {
    int i;
    assert(m_Tree&&m_Size>0&&m_Height>0);
    for(i=0;i<2*m_Size-1;i++)
      m_Tree[i]=-1;
    m_Freeblk=0;
  }

  {
    int  i,j,k;
    char level=m_Height-1;
    assert(m_Tree&&m_Size>0&&m_Height>0);
    for(i=1,j=0;i<=m_Size;level--,i=2*i)
      for(k=i;k>0;j++,k--)               /*fill line*/
        m_Tree[j]=level;
    m_Freeblk=m_Size;
  }
  return true;
}

void* csBuddyAllocator::alloc(int size)
{
  int  i,found;
  char level=m_Height-1;
  assert(m_Tree&&m_Size>0&&m_Height>0);
  size=(size+CS_BUDDY_ADD)>>CS_BUDDY_SHIFT;      /*sz in blocks now*/
  if(size<=0||1<<m_Tree[0]<size)       /*oops?*/
    return (void*)-1;

  for(found=0,i=0;!found&&2*i+2<2*m_Size-1;level--)
  {
    assert(m_Tree[i]<=level&&m_Tree[i]>=0);
                                       /**left smaller than right?**/
    if(m_Tree[2*i+1]<=m_Tree[2*i+2])
      if(1<<m_Tree[2*i+1]>=size)     /*...and still large enough?*/
        i=2*i+1;                       /*- go left -*/
      else if(1<<m_Tree[2*i+2]>=size)/*at least, right large enough?*/
        i=2*i+2;                       /*- go right -*/
      else
        found=1;                       /*none enough, use current*/
    else                               /**right smaller than left**/
      if(1<<m_Tree[2*i+2]>=size)     /*...and still large enough?*/
        i=2*i+2;                       /*- go right -*/
      else if(1<<m_Tree[2*i+1]>=size)/*at least, left large enough?*/
        i=2*i+1;                       /*- go left -*/
      else
        found=1;                       /*none enough, use current*/
  }
  assert(1<<m_Tree[i]>=size);
  level=m_Tree[i];
  m_Tree[i]=-1;                    /*mark allocated*/
  stabilize1(i);                 /*correct tree*/
  m_Freeblk-=size;
  return itop(i,level);
}

bool csBuddyAllocator::free(void *ptr)
{
  char level;
  int  i=ptoi(ptr);
  assert(m_Tree&&m_Size>0&&m_Height>0);
  if(i<0)
    return 0;                          /*find parent with tree[i]<=0*/
  for(level=0;i>=0&&m_Tree[i]>=0;level++)
    i=(i-1)/2;
  if(i>=0)                             /*if found, free it*/
  {
    m_Tree[i]=level;
    stabilize2(i,level);         /*correct tree*/
    m_Freeblk+=1<<level;
    return true;
  }
  else
    return false;
}

void csBuddyAllocator::PrintStats()
{
  int  s;
  char level;
  if(!m_Tree||m_Size<=0||m_Height<=0)
  {
    printf("Buddy system not initialized.\n\n");
    return;
  }
  else
  {
    printf("Buddy system initialized.\n");
    printf("Address %x, %d blocks (%dKB), ",0,m_Size,m_Size<<CS_BUDDY_SHIFT-10);
    printf("%d blocks free (%dKB).\n",m_Freeblk,m_Freeblk<<CS_BUDDY_SHIFT-10);
    traverse(0,m_Height-1);
    printf("\n");
  }
}

#define CS_VAR_ALLOC_SIZE (500*1024)

bool csVARRenderBufferManager::Initialize(csGLRender3D* render3d)
{
  csVARRenderBufferManager::render3d = render3d;

  // Alloc some VAR-memory
  // HACK: this should be fetched from config-file.. for now it's hardcoded
  if(render3d->ext.wglAllocateMemoryNV)
    var_buffer = (unsigned char*) render3d->ext.wglAllocateMemoryNV(CS_VAR_ALLOC_SIZE, 0.0f,0.0f,0.5f);
  else
    return false;

  if(!var_buffer)
    return false;

  render3d->ext.glVertexArrayRangeNV(CS_VAR_ALLOC_SIZE, var_buffer);
  glEnableClientState(GL_VERTEX_ARRAY_RANGE_NV);


  myalloc = new csBuddyAllocator();
  myalloc->Initialize(CS_VAR_ALLOC_SIZE);

  return true;
}

csVARRenderBufferManager::~csVARRenderBufferManager()
{
  glDisableClientState(GL_VERTEX_ARRAY_RANGE_NV);

  if(var_buffer)
    render3d->ext.wglFreeMemoryNV(var_buffer);
  var_buffer = 0;
  
  myalloc->PrintStats();
  delete myalloc;
}

csPtr<iRenderBuffer> csVARRenderBufferManager::CreateBuffer(int size, CS_RENDERBUFFER_TYPE location)
{
  csVARRenderBuffer* buffer = new csVARRenderBuffer( NULL, size, location, this);

  printf("Created buffer at: %X\n", (long)buffer);
  buffer->IncRef();
  return buffer;
}

void csVARRenderBufferManager::AddBlockInLRU(csVARRenderBuffer* buf)
{
  if (buf->type == CS_BUF_STATIC)
  {
    buf->listEl = staticList.PushFront (buf);
    return;
  }
  else if (buf->type == CS_BUF_DYNAMIC)
  {
    buf->listEl = dynamicList.PushFront (buf);
    return;
  }
}

void csVARRenderBufferManager::RemoveBlockInLRU(csVARRenderBuffer* buf)
{
  if ((*buf->listEl) == NULL) return;

  if (buf->type == CS_BUF_STATIC)
  {
    staticList.Delete (buf->listEl);
    return;
  }
  else if (buf->type == CS_BUF_DYNAMIC)
  {
    dynamicList.Delete (buf->listEl);
    return;
  }
}

void csVARRenderBufferManager::TouchBlockInLRU(csVARRenderBuffer* buf)
{
  if ((*buf->listEl) == NULL) return;

  if (buf->type == CS_BUF_STATIC)
  {
    staticList.Delete (buf->listEl);
    buf->listEl = staticList.PushFront (buf);
    return;
  }
  else if (buf->type == CS_BUF_DYNAMIC)
  {
    dynamicList.Delete (buf->listEl);
    buf->listEl = dynamicList.PushFront (buf);
    return;
  }
}

bool csVARRenderBufferManager::ReclaimMemory()
{
  csList<csVARRenderBuffer*>::Iterator dynamicIt(dynamicList, true);
  while(dynamicIt.HasPrevious())
  {
    if((*dynamicIt)->Discard())
      return true;
    dynamicIt--;
  }

  csList<csVARRenderBuffer*>::Iterator staticIt(staticList, true);
  while(staticIt.HasPrevious())
  {
    if((*staticIt)->Discard())
      return true;
    staticIt--;
  }
  return false;
}

void* csVARRenderBuffer::AllocData(int size)
{
  long offset;
  while( ((offset = (long) bm->myalloc->alloc(size)) == -1) && bm->ReclaimMemory())
    {};

  if(offset == -1) return NULL; //could not alloc memory

  return bm->var_buffer + offset;
}

csVARRenderBuffer::csVARRenderBuffer(void *buffer, int size, CS_RENDERBUFFER_TYPE type, csVARRenderBufferManager* bm)
{
  SCF_CONSTRUCT_IBASE (NULL);
  currentBlock = 0;

  csVARRenderBuffer::size = size;
  csVARRenderBuffer::type = type;

  csVARRenderBuffer::bm = bm;
  locked = false;
  discarded = false;
  discardable = true;

  memblock = new csVARMemoryBlock[MAXMEMORYBLOCKS];
  int i;
  for(i = 0; i < MAXMEMORYBLOCKS; ++i)
  {
    memblock[i].buffer = NULL;
    memblock[i].fence_id = -1;
  }
}

csVARRenderBuffer::~csVARRenderBuffer()
{
  bm->RemoveBlockInLRU(this);
  if( type == CS_BUF_INDEX)
  {
    int i;
    for(i = 0; i < MAXMEMORYBLOCKS; ++i)
      delete memblock[i].buffer;
  }else
  {
    int i;
    for(i = 0; i < MAXMEMORYBLOCKS;++i)
    {
      bm->render3d->ext.glFinishFenceNV(memblock[i].fence_id);
      if(memblock[i].buffer)
        bm->myalloc->free( (void*)((long)(memblock[i].buffer) - (long)(bm->var_buffer)));
      bm->render3d->ext.glDeleteFencesNV(1, &memblock[i].fence_id);
    }
  }
  delete[] memblock;
}

void* csVARRenderBuffer::Lock(CS_BUFFER_LOCK_TYPE lockType)
{
  //if its already locked, we cannot return it
  if(locked) return NULL;

  //first, if doing render-locking, just return our current memblock buffer
  if(lockType == CS_BUF_LOCK_RENDER)
  {
    if(memblock[currentBlock].buffer && !discarded)
    {
      locked = true;
      lastlock = lockType;
      bm->TouchBlockInLRU(this);
      return memblock[currentBlock].buffer;
    }
    return NULL;
  }

  if(!memblock[currentBlock].buffer || discarded)
  {
    //have no buffer.. create one
    if(type == CS_BUF_INDEX)
    {
      //indexbuffer is always in system-memory
      memblock[currentBlock].buffer = new char[size];
      memblock[currentBlock].fence_id = -1;
      discarded = false;
      locked = true;
      lastlock = lockType;
      return memblock[currentBlock].buffer;
    }else
    {
      memblock[currentBlock].buffer = AllocData(size);
      if(memblock[currentBlock].buffer)
      {
        locked = true;
        discarded = false;
        lastlock = lockType;
        bm->AddBlockInLRU(this);
        bm->render3d->ext.glGenFencesNV(1, &(memblock[currentBlock].fence_id));
      }
      return memblock[currentBlock].buffer;
    }
  }

  if(type == CS_BUF_INDEX)
  {
    locked = true;
    discarded = false;
    lastlock = lockType;
    return memblock[currentBlock].buffer;
  }
  else if (type == CS_BUF_STATIC)
  {
    bm->render3d->ext.glFinishFenceNV(memblock[currentBlock].fence_id);
    locked = true;
    discarded = false;
    lastlock = lockType;
    return memblock[currentBlock].buffer;
  }
  else if (type == CS_BUF_DYNAMIC)
  {
    if(bm->render3d->ext.glTestFenceNV(memblock[currentBlock].fence_id))
    {
      //it was free, use it
      locked = true;
      discarded = false;
      lastlock = lockType;
      return memblock[currentBlock].buffer;
    }
    else
    {
      //we need to use the other block
      ++currentBlock;
      currentBlock %= MAXMEMORYBLOCKS;
      return Lock(lockType);
    }

  }
  
  return NULL;
}

void csVARRenderBuffer::Release()
{
  if(lastlock == CS_BUF_LOCK_RENDER && type != CS_BUF_INDEX)
  {
    bm->render3d->ext.glSetFenceNV(memblock->fence_id, GL_ALL_COMPLETED_NV);
  }

  discardable = true;
  lastlock = CS_BUF_LOCK_NOLOCK;
  locked = false;
}

bool csVARRenderBuffer::Discard()
{
  //never discard a locked buffer, or a non-discardable buffer
  if (locked || !discardable) return false;

  int i;
  for(i = 0; i < MAXMEMORYBLOCKS; ++i)
  {
    if(memblock[i].buffer != NULL)
    {
      bm->render3d->ext.glFinishFenceNV(memblock[i].fence_id);
      bm->myalloc->free( (void*)((long)(memblock[i].buffer) - (long)(bm->var_buffer)));
      bm->render3d->ext.glDeleteFencesNV(1, &memblock[i].fence_id);
      memblock[i].buffer = NULL;
    }
  }
  discarded = true;
  bm->RemoveBlockInLRU(this);
  return true;
}


