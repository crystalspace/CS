//=============================================================================
//
//	Copyright (C)1999 by Eric Sunshine <sunshine@sunshineco.com>
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
//	Conversion from Crystal Space format frame buffer to NeXT format
//	occurs in place.  Using the red, green, and blue masks, Crystal Space
//	is instructed to generate RGB data in the three high-order bytes of a
//	four byte pixel.  During the cooking process, the fourth (alpha) byte
//	is set to 0xff.  The resulting data can then be used directly by the
//	NeXT Window Server.  See the file README.NeXT for an explanation of
//	why the alpha byte of RGBA:8888 is set to 0xff before handing the
//	data off to the Window Server.
//
//-----------------------------------------------------------------------------
#include "NeXTFrameBuffer32.h"

#if defined(__LITTLE_ENDIAN__)
#define RED_MASK   0x000000ff
#define GREEN_MASK 0x0000ff00
#define BLUE_MASK  0x00ff0000
#else
#define RED_MASK   0xff000000
#define GREEN_MASK 0x00ff0000
#define BLUE_MASK  0x0000ff00
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
    { return frame_buffer; }
unsigned char* NeXTFrameBuffer32::get_cooked_buffer() const
    { return frame_buffer; }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
NeXTFrameBuffer32::NeXTFrameBuffer32( unsigned int w, unsigned int h ) :
    NeXTFrameBuffer(w,h)
    {
    buffer_size = adjust_allocation_size( CS_NEXT_BPP * width * height );
    frame_buffer = allocate_memory( buffer_size );
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
NeXTFrameBuffer32::~NeXTFrameBuffer32()
    {
    deallocate_memory( frame_buffer, buffer_size );
    }


//-----------------------------------------------------------------------------
// cook
//-----------------------------------------------------------------------------
void NeXTFrameBuffer32::cook()
    {
    unsigned char* p = frame_buffer + 3;
    for (unsigned int n = width * height; n-- > 0; p += 4)
	*p = 0xff;
    }
