//=============================================================================
//
//	Copyright (C)1999-2002 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
//-----------------------------------------------------------------------------
// CocoaMemory.m
//
//	Platform-specific functions for allocating and manipulating
//	page-aligned memory blocks.
//
//-----------------------------------------------------------------------------
#include "CocoaMemory.h"
#import <Foundation/NSZone.h>

//-----------------------------------------------------------------------------
// CocoaMemory_round_up_to_multiple_of_page_size
//-----------------------------------------------------------------------------
unsigned int CocoaMemory_round_up_to_multiple_of_page_size(unsigned int size)
{
  return NSRoundUpToMultipleOfPageSize(size);
}


//-----------------------------------------------------------------------------
// CocoaMemory_allocate_memory_pages
//	Allocation via NSAllocateMemoryPages() is guaranteed to be page-aligned
//	(on Mach, at least).
//-----------------------------------------------------------------------------
unsigned char* CocoaMemory_allocate_memory_pages(unsigned int nbytes)
{
  return (unsigned char*)NSAllocateMemoryPages(nbytes);
}


//-----------------------------------------------------------------------------
// CocoaMemory_deallocate_memory_pages
//-----------------------------------------------------------------------------
void CocoaMemory_deallocate_memory_pages(unsigned char* p, unsigned int nbytes)
{
  NSDeallocateMemoryPages(p, nbytes);
}
