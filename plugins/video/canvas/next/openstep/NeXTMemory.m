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
#import <Foundation/NSZone.h>

//-----------------------------------------------------------------------------
// NeXTMemory_round_up_to_multiple_of_page_size
//-----------------------------------------------------------------------------
unsigned int NeXTMemory_round_up_to_multiple_of_page_size(unsigned int size)
{
  return NSRoundUpToMultipleOfPageSize(size);
}


//-----------------------------------------------------------------------------
// NeXTMemory_allocate_memory_pages
//	Allocation via NSAllocateMemoryPages() is guaranteed to be page-aligned
//	(on Mach, at least).
//-----------------------------------------------------------------------------
unsigned char* NeXTMemory_allocate_memory_pages(unsigned int nbytes)
{
  return (unsigned char*)NSAllocateMemoryPages(nbytes);
}


//-----------------------------------------------------------------------------
// NeXTMemory_deallocate_memory_pages
//-----------------------------------------------------------------------------
void NeXTMemory_deallocate_memory_pages(unsigned char* p, unsigned int nbytes)
{
  NSDeallocateMemoryPages(p, nbytes);
}
