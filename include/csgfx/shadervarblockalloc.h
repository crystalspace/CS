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

#ifndef __CS_CSGFX_SHADERVARBLOCKALLOC_H__
#define __CS_CSGFX_SHADERVARBLOCKALLOC_H__

#include "csextern.h"

#include "csutil/blockallocator.h"
#include "csutil/ref.h"
#include "csgfx/shadervar.h"

/**\file
 * Block allocator for shader variables.
 */

/**
 * A block allocator for shader variables.
 * \remarks Check the csBlockAllocator documentation for information on
 *   general block allocators.
 */
class csShaderVarBlockAlloc
{
  struct BlockAllocatedSV : public csShaderVariable
  {
    csShaderVarBlockAlloc* allocator;
    virtual void Delete() { allocator->blockAlloc.Free (this); }
  };
  friend class BlockAllocatedSV;
  csBlockAllocator<BlockAllocatedSV> blockAlloc;
public:
  /**
   * Construct a new allocator.
   * \remarks The parameters are the same as to 
   *   csBlockAllocator<T>::csBlockAllocator().
   */
  csShaderVarBlockAlloc (size_t nelem = 32, bool warn_unfreed = false) :
    blockAlloc(nelem, warn_unfreed) {}
  /**
   * Allocate a new shader variable.
   * \remarks Returned object freed when all references are released, but 
   *  when the allocating block allocator is freed. Make sure no references
   *  are kept to objects created by an allocator past its lifetime.
   */
  csRef<csShaderVariable> Alloc ()
  { 
    BlockAllocatedSV* sv = blockAlloc.Alloc ();
    sv->allocator = this;
    return csPtr<csShaderVariable> (sv); 
  }
};

#endif // __CS_CSGFX_SHADERVARBLOCKALLOC_H__
