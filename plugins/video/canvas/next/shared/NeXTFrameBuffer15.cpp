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
// NeXTFrameBuffer15.cpp
//
//	A concrete subclass of NeXTFrameBuffer which knows how to convert
//	Crystal Space RGB:555 to NeXT RGBA:4444.
//
//	Conversion from Crystal Space RGB:555 format buffer to NeXT RGBA:4444
//	format is accomplished via a simple translation table.  The table has
//	one entry for each of the possible 15-bit RGB values which can occur
//	in the raw Crystal Space buffer.  The value at each position in the
//	table is the corresponding 16-bit RGBA NeXT-format pixel value, where
//	each color is represented by 4 bits (thus losing a small amount of
//	accuracy) and an alpha nybble which is set to 0x0f.  See the file
//	CS/docs/texinfo/internal/platform/next.txi for an explanation of why
//	the alpha nybble is set to 0x0f.  To convert the buffer, a simple loop
//	iterates over the raw data, and for each entry, the corresponding
//	cooked data is looked up in the translation table and stored in the
//	cooked buffer.  An alternate way to convert the data would be to
//	manually massage the raw data bits each time cook() is invoked.
//	However, using the lookup table for conversion is considerably faster,
//	although at a slightly higher cost in terms of memory.
//
//	Note that the 15-to-16 bit lookup table is actually twice the size it
//	needs to be for safety's sake.  The second half of the table is merely
//	a verbatim copy of the first half.  The reason for the doubled table
//	is that there are times when Crystal Space incorrectly places 16-bit
//	values in its buffer rather than the requested 15-bit color values.
//	Accessing a 15-bit lookup table with the incorrect 16-bit value can
//	easily result in a segmentation fault as memory is referenced beyond
//	the end of the table.  This problem can occur when bugs in the
//	software renderer cause it to paint polygons with random memory rather
//	than with the correctly prepared 15-bit textures.  The problem can
//	also occur when an improperly written program assumes that it can
//	safely use a hard-coded 16-bit color value in a 15-bit environment.
//	The doubled table makes the frame-buffer conversion process safer and
//	more tolerant of bugs elsewhere in Crystal Space.
//
//-----------------------------------------------------------------------------
#include "NeXTFrameBuffer15.h"
#include "NeXTMemory.h"
extern "C" {
#include <stdlib.h>
#include <string.h>
}

#define RED_SHIFT   10
#define GREEN_SHIFT  5
#define BLUE_SHIFT   0

#define RED_MASK   (0x1f << RED_SHIFT  )
#define GREEN_MASK (0x1f << GREEN_SHIFT)
#define BLUE_MASK  (0x1f << BLUE_SHIFT )

#define DECODE_RED(C)	(((C) >> (RED_SHIFT   + 1)) & 0x0f)
#define DECODE_GREEN(C)	(((C) >> (GREEN_SHIFT + 1)) & 0x0f)
#define DECODE_BLUE(C)	(((C) >> (BLUE_SHIFT  + 1)) & 0x0f)

#if defined(__LITTLE_ENDIAN__)
#define ENCODE_RED(C)	((C) <<  4)
#define ENCODE_GREEN(C)	((C) <<  0)
#define ENCODE_BLUE(C)	((C) << 12)
#define ENCODE_ALPHA(C)	((C) <<  8)
#else
#define ENCODE_RED(C)	((C) << 12)
#define ENCODE_GREEN(C)	((C) <<  8)
#define ENCODE_BLUE(C)	((C) <<  4)
#define ENCODE_ALPHA(C)	((C) <<  0)
#endif

int const CS_NEXT_DEPTH = 15;
int const CS_NEXT_BPS = 4;
int const CS_NEXT_BPP = 2;

int NeXTFrameBuffer15::depth() const { return CS_NEXT_DEPTH; }
int NeXTFrameBuffer15::bits_per_sample() const { return CS_NEXT_BPS; }
int NeXTFrameBuffer15::bytes_per_pixel() const { return CS_NEXT_BPP; }
int NeXTFrameBuffer15::palette_entries() const { return 0; }

int NeXTFrameBuffer15::red_mask  () const { return RED_MASK;   }
int NeXTFrameBuffer15::green_mask() const { return GREEN_MASK; }
int NeXTFrameBuffer15::blue_mask () const { return BLUE_MASK;  }

unsigned char* NeXTFrameBuffer15::get_raw_buffer() const
    { return raw_buffer; }
unsigned char* NeXTFrameBuffer15::get_cooked_buffer() const
    { return cooked_buffer; }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
NeXTFrameBuffer15::NeXTFrameBuffer15( unsigned int w, unsigned int h ) :
    NeXTFrameBuffer(w,h)
    {
    buffer_size   = NeXTMemory_round_up_to_multiple_of_page_size(
		    CS_NEXT_BPP * width * height );
    raw_buffer    = NeXTMemory_allocate_memory_pages( buffer_size );
    cooked_buffer = NeXTMemory_allocate_memory_pages( buffer_size );
    memset( cooked_buffer, 0, buffer_size );
    lookup_table  = build_15_to_12_rgb_table();
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
NeXTFrameBuffer15::~NeXTFrameBuffer15()
    {
    free( lookup_table );
    NeXTMemory_deallocate_memory_pages( cooked_buffer, buffer_size );
    NeXTMemory_deallocate_memory_pages( raw_buffer,    buffer_size );
    }


//-----------------------------------------------------------------------------
// build_15_to_12_rgb_table
//-----------------------------------------------------------------------------
unsigned short* NeXTFrameBuffer15::build_15_to_12_rgb_table() const
    {
    unsigned int const lim = 1 << 15;
    unsigned int const nbytes = 2 * lim * sizeof(short); // Doubled table.
    unsigned short* table = (unsigned short*)malloc(nbytes);
    unsigned short* p = table;
    for (unsigned int i = 0; i < lim; i++)
	*p++ = ENCODE_RED  (DECODE_RED  (i)) |
	       ENCODE_GREEN(DECODE_GREEN(i)) |
	       ENCODE_BLUE (DECODE_BLUE (i)) |
	       ENCODE_ALPHA(0x0f);
    memcpy( table + lim, table, nbytes / 2 );
    return table;
    }


//-----------------------------------------------------------------------------
// cook
//-----------------------------------------------------------------------------
void NeXTFrameBuffer15::cook()
    {
    unsigned short const* src = (unsigned short*)raw_buffer;
    unsigned short* dst = (unsigned short*)cooked_buffer;
    for (unsigned int n = width * height; n-- > 0; )
	*dst++ = lookup_table[ *src++ ];
    }
