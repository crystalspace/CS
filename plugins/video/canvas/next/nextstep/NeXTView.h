#ifndef __NeXT_NeXTView_h
#define __NeXT_NeXTView_h
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
// NeXTView.h
//
//	View subclass which provides the AppKit mechanism for blitting
//	Crystal Space's rendered frame buffer to screen.
//
//-----------------------------------------------------------------------------
#import <appkit/View.h>
@class NXBitmapImageRep;

@interface NeXTView : View
    {
    NXBitmapImageRep* rep;
    }

- (id)initFrame:(NXRect const*)r;
- (id)free;
- (id)drawSelf:(NXRect const*)rects :(int)nrects;
- (id)keyDown:(NXEvent*)p;
- (id)keyUp:(NXEvent*)p;
- (id)flagsChanged:(NXEvent*)p;
- (id)mouseMoved:(NXEvent*)p;
- (id)mouseDown:(NXEvent*)p;
- (id)mouseUp:(NXEvent*)p;
- (id)mouseDragged:(NXEvent*)p;
- (id)rightMouseDown:(NXEvent*)p;
- (id)rightMouseUp:(NXEvent*)p;
- (id)rightMouseDragged:(NXEvent*)p;
- (BOOL)acceptsFirstResponder;

- (void)setFrameBuffer:(unsigned char*)p bitsPerSample:(int)bps;
- (void)flush;

@end

#endif // __NeXT_NeXTView_h
