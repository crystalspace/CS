#ifndef __Cocoa_CocoaView_h
#define __Cocoa_CocoaView_h
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
// CocoaView.h
//
//	NSView subclass which provides the AppKit mechanism for blitting
//	Crystal Space's rendered frame buffer to screen.
//
//-----------------------------------------------------------------------------
#import <AppKit/NSView.h>
@class NSBitmapImageRep;

@interface CocoaView : NSView
{
  NSBitmapImageRep* rep;
}

- (id)initWithFrame:(NSRect)r;
- (void)dealloc;
- (void)drawRect:(NSRect)r;
- (void)keyDown:(NSEvent*)p;
- (void)keyUp:(NSEvent*)p;
- (void)flagsChanged:(NSEvent*)p;
- (void)mouseMoved:(NSEvent*)p;
- (void)mouseDown:(NSEvent*)p;
- (void)mouseUp:(NSEvent*)p;
- (void)mouseDragged:(NSEvent*)p;
- (void)rightMouseDown:(NSEvent*)p;
- (void)rightMouseUp:(NSEvent*)p;
- (void)rightMouseDragged:(NSEvent*)p;
- (BOOL)acceptsFirstResponder;

- (void)setFrameBuffer:(unsigned char*)p bitsPerSample:(int)bps;
- (void)flush;

@end

#endif // __Cocoa_CocoaView_h
