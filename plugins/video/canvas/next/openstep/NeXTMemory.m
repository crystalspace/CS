//=============================================================================
//
//	Copyright (C)1999,2000 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
//-----------------------------------------------------------------------------
// NeXTFrameBuffer.cpp
//
//	A mostly abstract 2D frame buffer which is capabile of converting
//	a raw Crystal Space frame buffer into a NeXT format frame buffer.
//
//-----------------------------------------------------------------------------
#include "cssysdef.h"
#include "NeXTFrameBuffer.h"
extern "Objective-C" {
#import <Foundation/NSZone.h>
}

NeXTFrameBuffer::NeXTFrameBuffer( unsigned int w, unsigned int h ) :
    width(w), height(h) {}
NeXTFrameBuffer::~NeXTFrameBuffer() {}

//-----------------------------------------------------------------------------
// adjust_allocation_size
//-----------------------------------------------------------------------------
unsigned int NeXTFrameBuffer::adjust_allocation_size( unsigned int size )
    {
    return NSRoundUpToMultipleOfPageSize( size );
    }


//-----------------------------------------------------------------------------
// allocate_memory
//	Allocation via NSAllocateMemoryPages() is guaranteed to be
//	page-aligned (on Mach, at least) which is required for best video
//	optimization.  See CS/docs/texinfo/internal/platform/next.txi for
//	details.
//-----------------------------------------------------------------------------
unsigned char* NeXTFrameBuffer::allocate_memory( unsigned int nbytes )
    {
    return (unsigned char*)NSAllocateMemoryPages( nbytes );
    }


//-----------------------------------------------------------------------------
// deallocate_memory
//-----------------------------------------------------------------------------
void NeXTFrameBuffer::deallocate_memory(unsigned char* p, unsigned int nbytes)
    {
    NSDeallocateMemoryPages( p, nbytes );
    }
