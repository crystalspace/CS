//=============================================================================
//
//	Copyright (C)1999,2000 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
#include "cssysdef.h"
#include "NeXTView.h"
#include "NeXTDelegate.h"
extern "Objective-C" {
#import <AppKit/NSApplication.h>
#import <AppKit/NSBitmapImageRep.h>
}
extern "C" {
#include <limits.h>	// CHAR_BIT
}

//=============================================================================
// IMPLEMENTATION
//=============================================================================
@implementation NeXTView

- (BOOL)acceptsFirstResponder { return YES; }
- (BOOL)isOpaque { return YES; }

#define DEL(C,E) [[NSApp delegate] C:E inView:self]
- (void)keyDown:(NSEvent*)p  { DEL( keyDown,           p ); } 
- (void)keyUp:            (NSEvent*)p  { DEL( keyUp,             p ); }
- (void)flagsChanged:     (NSEvent*)p  { DEL( flagsChanged,      p ); }
- (void)mouseMoved:       (NSEvent*)p  { DEL( mouseMoved,        p ); }
- (void)mouseDown:        (NSEvent*)p  { DEL( mouseDown,         p ); }
- (void)mouseUp:          (NSEvent*)p  { DEL( mouseUp,           p ); }
- (void)mouseDragged:     (NSEvent*)p  { DEL( mouseDragged,      p ); }
- (void)rightMouseDown:   (NSEvent*)p  { DEL( rightMouseDown,    p ); }
- (void)rightMouseUp:     (NSEvent*)p  { DEL( rightMouseUp,      p ); }
- (void)rightMouseDragged:(NSEvent*)p  { DEL( rightMouseDragged, p ); }
#undef DEL

//-----------------------------------------------------------------------------
// initWithFrame:
//-----------------------------------------------------------------------------
- (id)initWithFrame:(NSRect)r
    {
    [super initWithFrame:r];
    [self allocateGState];
    rep = 0;
    return self;
    }


//-----------------------------------------------------------------------------
// dealloc
//-----------------------------------------------------------------------------
- (void)dealloc
    {
    [rep release];
    [super dealloc];
    }


//-----------------------------------------------------------------------------
// setFrameBuffer:bitsPerSample:
//	Allocate an NXBitmapImageRep as a cover for the raw frame buffer.  The
//	bits per sample will always be either 4 or 8.  A bps of 4 requires 2
//	bytes per pixel (RRRRGGGGBBBBAAAA where A=1).  A bps of 8 requires 4
//	bytes (RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA where A=1).  See also:
//	CS/docs/texinfo/internal/platform/next.txi for a discussion of the
//	frame buffer layout.
//-----------------------------------------------------------------------------
- (void)setFrameBuffer:(unsigned char*)buff bitsPerSample:(int)bits_per_sample
    {
    NSRect const r = [self bounds];
    unsigned int const w = int(r.size.width);
    unsigned int const h = int(r.size.height);

    CS_ASSERT( bits_per_sample == 4 || bits_per_sample == 8 );
    unsigned int const samples_per_pixel = 3;	// RGB (A is not included).
    unsigned int const bytes_per_pixel = bits_per_sample / 2;
    unsigned int const bytes_per_row = bytes_per_pixel * w;
    unsigned int const bits_per_pixel = bytes_per_pixel * CHAR_BIT;

    rep = [[NSBitmapImageRep allocWithZone:[self zone]]
	initWithBitmapDataPlanes:&buff
	pixelsWide:w
	pixelsHigh:h
	bitsPerSample:bits_per_sample
	samplesPerPixel:samples_per_pixel
	hasAlpha:NO
	isPlanar:NO
	colorSpaceName:NSCalibratedRGBColorSpace
	bytesPerRow:bytes_per_row
	bitsPerPixel:bits_per_pixel];
    [rep setSize:NSMakeSize( w, h )];
    }


//-----------------------------------------------------------------------------
// drawRect:
//-----------------------------------------------------------------------------
- (void)drawRect:(NSRect)r
    {
    [rep draw];
    }


//-----------------------------------------------------------------------------
// flush -- Flush the frame buffer to the window.
//-----------------------------------------------------------------------------
- (void)flush
    {
    if ([self canDraw])
	{
	[self lockFocus];
	[rep draw];
	[self unlockFocus];
	}
    }

@end
