//=============================================================================
//
//	Copyright (C)1999-2001 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
//-----------------------------------------------------------------------------
// NeXTMemory.m
//
//	Platform-specific functions for allocating and manipulating
//	page-aligned memory blocks.
//
//-----------------------------------------------------------------------------
#include "NeXTMemory.h"
#include <assert.h>
#include <mach/mach.h>

//-----------------------------------------------------------------------------
// NeXTMemory_round_up_to_multiple_of_page_size
//-----------------------------------------------------------------------------
unsigned int NeXTMemory_round_up_to_multiple_of_page_size( unsigned int size )
    {
    if (size % vm_page_size != 0)
	size = ((unsigned int)(size / vm_page_size) + 1) * vm_page_size;
    return size;
    }


//-----------------------------------------------------------------------------
// NeXTMemory_allocate_memory_pages
//	Allocation via vm_allocate() is guaranteed to be page-aligned.
//-----------------------------------------------------------------------------
unsigned char* NeXTMemory_allocate_memory_pages( unsigned int nbytes )
    {
    unsigned char* p = 0;
    vm_allocate( task_self(), (vm_address_t*)&p, nbytes, TRUE );
    assert( p != 0 );
    return p;
    }


//-----------------------------------------------------------------------------
// NeXTMemory_deallocate_memory_pages
//-----------------------------------------------------------------------------
void NeXTMemory_deallocate_memory_pages(unsigned char* p, unsigned int nbytes)
    {
    vm_deallocate( task_self(), (vm_address_t)p, nbytes );
    }
