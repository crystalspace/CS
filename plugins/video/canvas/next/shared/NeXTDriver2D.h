#ifndef __NeXT_NeXTDriver2D_h
#define __NeXT_NeXTDriver2D_h
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
// NeXTDriver2D.h
//
//	NeXT-specific subclass of csGraphics2D which implements 2D-graphic
//	functionality via the AppKit.
//
//-----------------------------------------------------------------------------
#include "csutil/scf.h"
#include "cs2d/common/graph2d.h"
class NeXTFrameBuffer;
@class NeXTView;

class NeXTDriver2D : public csGraphics2D
    {
    typedef csGraphics2D superclass;

private:
    bool initialized;
    NeXTFrameBuffer* frame_buffer;
    NeXTView* view;

    bool init_driver( int simulate_depth );
    void shutdown_driver();
    void setup_rgb_15();
    void setup_rgb_32();
    bool open_window( char const* title );
    void flush();

    static int best_bits_per_sample();
    static int determine_bits_per_sample( int simulate_depth );

public:
    NeXTDriver2D::NeXTDriver2D( iBase* );
    virtual ~NeXTDriver2D();
    virtual bool Initialize( iSystem* );
    virtual bool Open( char const* title );
    virtual void Close();
    virtual void Print( csRect* = 0 );
    virtual bool SetMouseCursor( csMouseCursorID shape, iTextureHandle* );
    DECLARE_IBASE;
    };

#endif //__NeXT_NeXTDriver2D_h
