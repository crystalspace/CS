#ifndef __CocoaMemory_h
#define __CocoaMemory_h
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
// CocoaMemory.h
//
//	Platform-specific functions for allocating and manipulating
//	page-aligned memory blocks.
//
//-----------------------------------------------------------------------------
#if defined(__cplusplus)
extern "C" {
#endif

unsigned int CocoaMemory_round_up_to_multiple_of_page_size(unsigned int n);
unsigned char* CocoaMemory_allocate_memory_pages(unsigned int nbytes);
void CocoaMemory_deallocate_memory_pages(unsigned char*, unsigned int nbytes);

#if defined(__cplusplus)
}
#endif

#endif // __CocoaMemory_h
