#ifndef __NeXTMemory_h
#define __NeXTMemory_h
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
// NeXTMemory.h
//
//	Platform-specific functions for allocating and manipulating
//	page-aligned memory blocks.
//
//-----------------------------------------------------------------------------
#if defined(__cplusplus)
extern "C" {
#endif

unsigned int NeXTMemory_round_up_to_multiple_of_page_size(unsigned int nbytes);
unsigned char* NeXTMemory_allocate_memory_pages(unsigned int nbytes);
void NeXTMemory_deallocate_memory_pages(unsigned char*, unsigned int nbytes);

#if defined(__cplusplus)
}
#endif

#endif // __NeXTMemory_h
