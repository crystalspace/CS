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
    return 0;

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


#define CS_VAR_ALLOC_SIZE (8*1024*1024)

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
  if(var_buffer)
    render3d->ext.wglFreeMemoryNV(var_buffer);
  var_buffer = 0;
}

csPtr<iRenderBuffer> csVARRenderBufferManager::GetBuffer(int size, CS_RENDERBUFFER_TYPE location)
{
  csVARRenderBuffer* buffer = new csVARRenderBuffer( NULL, size, location, this);

  return buffer;
}

csVARRenderBuffer::csVARRenderBuffer(void *buffer, int size, CS_RENDERBUFFER_TYPE type, csVARRenderBufferManager* bm)
{
  SCF_CONSTRUCT_IBASE (NULL);
  memblock = new csVARMemoryBlock();

  /// decide wheter to use VAR or not (some special cases)
  if(type == CS_BUF_INDEX)
  {
    isRealVAR = false;
    memblock->buffer = new char[size];
  }
  else if(type == CS_BUF_STATIC)
  {
    isRealVAR = true;
    memblock->buffer = bm->var_buffer + (int) bm->myalloc->alloc(size);
    bm->render3d->ext.glGenFencesNV(1, &memblock->fence_id);
  } else 
  {
    isRealVAR = true;
  }

  csVARRenderBuffer::size = size;
  csVARRenderBuffer::type = type;

  csVARRenderBuffer::bm = bm;
  locked = false;
}

csVARRenderBuffer::~csVARRenderBuffer()
{
  if (memblock && type != CS_BUF_INDEX)
  {
    bm->render3d->ext.glFinishFenceNV(memblock->fence_id);
    bm->myalloc->free(memblock->buffer);
  }else if(memblock && type == CS_BUF_INDEX)
  {
    delete memblock->buffer;
    delete memblock;
  }
}

void* csVARRenderBuffer::Lock(CS_BUFFER_LOCK_TYPE lockType)
{
  if(locked) return NULL;
  

  if(memblock->buffer)
  {
    if(lockType == CS_BUF_LOCK_RENDER)
    {
      lastlock = lockType;
      locked = true;
      return memblock->buffer;
    }
    if(type == CS_BUF_STATIC)
    {
      bm->render3d->ext.glFinishFenceNV(memblock->fence_id); //for now.. lockup until finnished
      lastlock = lockType;
      locked = true;
      return memblock->buffer;
    }else if(type == CS_BUF_INDEX)
    {
      lastlock = lockType;
      locked = true;
      return memblock->buffer;
    }else if(type == CS_BUF_DYNAMIC)
    {
      if(bm->render3d->ext.glTestFenceNV(memblock->fence_id))
      {
        lastlock = lockType;
        locked = true;
        return memblock->buffer;
      }
      else
        return NULL;
    }else
      return NULL;
  }else
  {
    if(type == CS_BUF_DYNAMIC)
    {
      //alloc a new VAR buffer..
      memblock->buffer = bm->myalloc->alloc(size);
      bm->render3d->ext.glGenFencesNV(1, &memblock->fence_id);
      lastlock = lockType;
      locked = true;
      return memblock->buffer;
    }else
      return NULL;
  }
  return NULL;
}

void csVARRenderBuffer::Release()
{
  if(lastlock == CS_BUF_LOCK_RENDER && isRealVAR)
  {
    bm->render3d->ext.glSetFenceNV(memblock->fence_id, GL_ALL_COMPLETED_NV);
  }

  lastlock = CS_BUF_LOCK_NOLOCK;
  locked = false;
}

