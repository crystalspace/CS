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
// NeXTFrameBuffer32.cpp
//
//	A concrete subclass of NeXTFrameBuffer which knows how to convert
//	Crystal Space RGB:888 to NeXT RGBA:8888.
//
//	For performance reasons, the Crystal Space software renderer has
//	hard-coded notions of pixel format (in particular, ARGB on
//	little-endian and ABGR on big-endian).  The AppKit, on the other hand
//	expects pixel data to be in the format (from a CPU register perspetive)
//	ABGR on little-endian and RGBA on big-endian.  (In memory, the pixel
//	format is RGBA regardless of endian.)  It is possible to remove the
//	hard-coded notions of pixel format from the renderer, but the loss of
//	performance would likely be dramatic.  Consequently, this frame buffer
//	implementation cooks the Crystal Space generated ARGB or ABGR data into
//	the RGBA:8888 data expected by the AppKit.  Thus modification to the
//	software renderer is unnecessary.  (Modifying the renderer would also
//	tightly couple it to this 2D driver implementation.  That would be
//	undesirable since it would make it difficult to use a different 2D
//	driver with the same renderer.)
//
//	A further restriction is that the NeXT Window Server release notes for
//	NextStep 3.0 stipulate that the alpha byte must be set to 0xff.  (See
//	CS/docs/texinfo/internal/platform/next.txi for more details.)  This
//	requirement is met by setting all of the alpha bytes to 0xff, once
//	only, when the destination buffer is first created.
//
//-----------------------------------------------------------------------------
#include "NeXTFrameBuffer32.h"
#include "NeXTMemory.h"
#include <string.h>

#define RED_MASK     0x00ff0000
#define GREEN_MASK   0x0000ff00
#define BLUE_MASK    0x000000ff

#if defined(__LITTLE_ENDIAN__)
#define RED_OFFSET   2
#define GREEN_OFFSET 1
#define BLUE_OFFSET  0
#else
#define RED_OFFSET   1
#define GREEN_OFFSET 2
#define BLUE_OFFSET  3
#endif

int const CS_NEXT_DEPTH = 32;
int const CS_NEXT_BPS = 8;
int const CS_NEXT_BPP = 4;

int NeXTFrameBuffer32::depth() const { return CS_NEXT_DEPTH; }
int NeXTFrameBuffer32::bits_per_sample() const { return CS_NEXT_BPS; }
int NeXTFrameBuffer32::bytes_per_pixel() const { return CS_NEXT_BPP; }
int NeXTFrameBuffer32::palette_entries() const { return 0; }

int NeXTFrameBuffer32::red_mask  () const { return RED_MASK;   }
int NeXTFrameBuffer32::green_mask() const { return GREEN_MASK; }
int NeXTFrameBuffer32::blue_mask () const { return BLUE_MASK;  }

unsigned char* NeXTFrameBuffer32::get_raw_buffer() const
  { return raw_buffer; }
unsigned char* NeXTFrameBuffer32::get_cooked_buffer() const
  { return cooked_buffer; }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
NeXTFrameBuffer32::NeXTFrameBuffer32(unsigned int w, unsigned int h) :
  NeXTFrameBuffer(w,h)
{
  buffer_size =
    NeXTMemory_round_up_to_multiple_of_page_size(CS_NEXT_BPP * width * height);
  raw_buffer    = NeXTMemory_allocate_memory_pages(buffer_size);
  cooked_buffer = NeXTMemory_allocate_memory_pages(buffer_size);
  memset(cooked_buffer, 0, buffer_size);
  unsigned char* p = cooked_buffer + 3;	// See 0xff note above.
  for (unsigned int n = width * height; n > 0; n--, p += 4)
    *p = 0xff;
}


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
NeXTFrameBuffer32::~NeXTFrameBuffer32()
{
  NeXTMemory_deallocate_memory_pages(cooked_buffer, buffer_size);
  NeXTMemory_deallocate_memory_pages(raw_buffer,    buffer_size);
}


//-----------------------------------------------------------------------------
// cook
//-----------------------------------------------------------------------------
void NeXTFrameBuffer32::cook()
{
  unsigned char const* p = raw_buffer;
  unsigned char* q = cooked_buffer;
  for (unsigned int n = width * height; n > 0; n--, p += 4, q += 1)
  {
    *q++ = *(p +   RED_OFFSET);
    *q++ = *(p + GREEN_OFFSET);
    *q++ = *(p +  BLUE_OFFSET);
  }
}
