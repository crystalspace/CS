#ifndef __NeXT_NeXTProxy2D_h
#define __NeXT_NeXTProxy2D_h
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
// NeXTProxy2D.h
//
//	C++ object which interacts with Objective-C world on behalf of
//	NeXTDriver2D which can not directly interface with Objective-C on
//	account of COM-related conflicts.  See README.NeXT for details.
//
// *WARNING* Do NOT include any COM or Objective-C headers in this file.
//-----------------------------------------------------------------------------
#include "util/types.h"	// For bool.
@class NeXTView, Window;
class IMipMapContainer;

class NeXTProxy2D
    {
private:
    Window* window;
    NeXTView* view;
    unsigned int width;
    unsigned int height;
    unsigned int frame_buffer_size;
    unsigned char* frame_buffer;
    unsigned char* palette;
    int bits_per_sample;	// Always 4 or 8; see README.NeXT for details.

    void flush_12( unsigned char const* );
    void flush_24( unsigned char const* );

public:
    NeXTProxy2D( unsigned int width, unsigned int height );
    ~NeXTProxy2D();
    bool open( char const* title );
    void close();
    void flush( unsigned char const* src );
    void set_rgb( int i, int r, int g, int b );
    bool set_mouse_cursor( int shape, IMipMapContainer* );
    };

#endif //__NeXT_NeXTProxy2D_h
