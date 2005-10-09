/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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

/**\file
 * Databuffer pointing into another databuffer.
 */
 
#ifndef __CS_CSUTIL_PARASITICDATABUFFER_H__
#define __CS_CSUTIL_PARASITICDATABUFFER_H__

#include "csextern.h"
#include "csutil/pooledscfclass.h"
#include "csutil/scf_implementation.h"
#include "iutil/databuff.h"

/**
 * Base class for pooled and unpooled parasitic data buffer.
 */
class csParasiticDataBufferBase : 
  public scfImplementation1<csParasiticDataBufferBase, iDataBuffer>
{
protected:
  csRef<iDataBuffer> parentBuffer;
  uint8* data;
  size_t size;
  
  void SetContents (iDataBuffer* parent, size_t offs, size_t size)
  {
    parentBuffer = parent;
    data = parent->GetUint8 () + offs;
    size_t parRemaining = parent->GetSize () - offs;
    if (size == (size_t)~0)
      csParasiticDataBufferBase::size = parRemaining;
    else
      csParasiticDataBufferBase::size = MIN(size, parRemaining);
  }

  csParasiticDataBufferBase (iDataBuffer* parent, size_t offs,
    size_t size = (size_t)~0)
    : scfImplementationType (this)
  {
    SetContents (parent, offs, size);
  }

  csParasiticDataBufferBase () 
    : scfImplementationType (this), data(0), size(0) 
  {}
public:
  virtual ~csParasiticDataBufferBase () { }

  virtual size_t GetSize () const
  { return size; }
  virtual char* GetData () const
  { return (char*)data; }
  inline char *operator * () const
  { return (char *)GetData (); }
  inline int8* GetInt8 ()
  { return (int8 *)GetData (); }
  inline uint8* GetUint8 ()
  { return (uint8 *)GetData (); }
};

/**
 * A databuffer pointing into another databuffer.
 */
class CS_CRYSTALSPACE_EXPORT csParasiticDataBuffer
  : public scfImplementationExt0<csParasiticDataBuffer, csParasiticDataBufferBase>
{
public:
  /**
   * Construct this data buffer. 
   * \param parent The buffer to point into.
   * \param offs Offset into the buffer of the data to return.
   * \param size Data size to return. Can not be larger than the parent's size
   *   minus the offset. This maximum possible size is also taken when 
   *   \a size == ~0.
   */
  csParasiticDataBuffer (iDataBuffer* parent, size_t offs,
    size_t size = (size_t)~0) 
    : scfImplementationType (this, parent, offs, size)
  {
  }

  virtual ~csParasiticDataBuffer ()
  {
  }
};

/**
 * A databuffer pointing into another databuffer, pooled version.
 * Use like this:
 *  \code
 *  csParasiticDataBufferPooled::Pool bufferPool;
 *  ...
 *  csRef<iDataBuffer> buf;
 *  {
 *    csParasiticDataBufferPooled* bufPtr = bufferPool.Alloc ();
 *    bufPtr->SetContents (someOtherBuffer, Offset);
 *    buf.AttachNew (bufPtr);
 *  }
 *  \endcode
 */
class CS_CRYSTALSPACE_EXPORT csParasiticDataBufferPooled : 
  public csParasiticDataBufferBase
{
public:
  SCF_DECLARE_IBASE_POOLED_EXTERN(CS_CRYSTALSPACE_EXPORT, 
    csParasiticDataBufferPooled);

  virtual ~csParasiticDataBufferPooled ()
  {
    SCF_DESTRUCT_IBASE();
  }

  /**
   * Set contents of this data buffer.
   * Warning: Only call if you did not pass this buffer around yet!
   */
  void SetContents (iDataBuffer* parent, size_t offs, 
    size_t size = (size_t)~0)
  {
    csParasiticDataBufferBase::SetContents (parent, offs, size);
  }
protected:
  csParasiticDataBufferPooled (Pool* pool) : csParasiticDataBufferBase()
  {
    SCF_CONSTRUCT_IBASE_POOLED (pool);
  }
};

#endif // __CS_CSUTIL_PARASITICDATABUFFER_H__
