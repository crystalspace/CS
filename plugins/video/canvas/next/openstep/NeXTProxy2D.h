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
#include "types.h"	// For bool.
@class NeXTView, NSWindow;
class NeXTFrameBuffer;
class ITextureHandle;

class NeXTProxy2D
    {
private:
    NSWindow* window;
    NeXTView* view;
    unsigned int width;
    unsigned int height;
    NeXTFrameBuffer* frame_buffer;

public:
    NeXTProxy2D( unsigned int width, unsigned int height, int simulate_depth );
    ~NeXTProxy2D();
    NeXTFrameBuffer* get_frame_buffer() const { return frame_buffer; }
    bool open( char const* title );
    void close();
    void flush();
    bool set_mouse_cursor( int shape, ITextureHandle* );
    };

#endif //__NeXT_NeXTProxy2D_h
