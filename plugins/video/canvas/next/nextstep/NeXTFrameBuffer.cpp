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
//	A mostly abstract 2D frame buffer which is capable of converting a
//	raw Crystal Space frame buffer into a NeXT-format frame buffer.
//
//-----------------------------------------------------------------------------
#include "cssysdef.h"
#include "NeXTFrameBuffer.h"
extern "C" {
#include <assert.h>
#include <mach/mach.h>
}

NeXTFrameBuffer::NeXTFrameBuffer( unsigned int w, unsigned int h ) :
    width(w), height(h) {}
NeXTFrameBuffer::~NeXTFrameBuffer() {}

//-----------------------------------------------------------------------------
// adjust_allocation_size -- Round 'size' up to a multiple of vm_page_size.
//-----------------------------------------------------------------------------
unsigned int NeXTFrameBuffer::adjust_allocation_size( unsigned int size )
    {
    if (size % vm_page_size != 0)
	size = ((unsigned int)(size / vm_page_size) + 1) * vm_page_size;
    return size;
    }


//-----------------------------------------------------------------------------
// allocate_memory
//	Allocation via vm_allocate() is guaranteed to be page-aligned which
//	is required for best video optimization.  See the file
//	CS/docs/texinfo/internal/platform/next.txi for details.
//-----------------------------------------------------------------------------
unsigned char* NeXTFrameBuffer::allocate_memory( unsigned int nbytes )
    {
    unsigned char* p = 0;
    vm_allocate( task_self(), (vm_address_t*)&p, nbytes, TRUE );
    assert( p != 0 );
    return p;
    }


//-----------------------------------------------------------------------------
// deallocate_memory
//-----------------------------------------------------------------------------
void NeXTFrameBuffer::deallocate_memory(unsigned char* p, unsigned int nbytes)
    {
    vm_deallocate( task_self(), (vm_address_t)p, nbytes );
    }
