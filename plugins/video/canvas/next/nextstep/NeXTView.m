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
// NeXTView.m
//
//	View subclass which provides the AppKit mechanism for blitting
//	Crystal Space's rendered frame buffer to screen.
//
//-----------------------------------------------------------------------------
#include "NeXTView.h"
#include "NeXTDelegate2D.h"
#include <limits.h>	// CHAR_BIT
#include <assert.h>
#import <appkit/Application.h>
#import <appkit/NXBitmapImageRep.h>

//=============================================================================
// IMPLEMENTATION
//=============================================================================
@implementation NeXTView

- (BOOL)acceptsFirstResponder { return YES; }

#define MSG(M) - (id)M:(NXEvent*)e \
    { [[[self window] delegate] dispatchEvent:e forView:self]; return self; }

MSG(keyDown)
MSG(keyUp)
MSG(flagsChanged)
MSG(mouseMoved)
MSG(mouseDown)
MSG(mouseUp)
MSG(mouseDragged)
MSG(rightMouseDown)
MSG(rightMouseUp)
MSG(rightMouseDragged)

#undef MSG

//-----------------------------------------------------------------------------
// initFrame:
//-----------------------------------------------------------------------------
- (id)initFrame:(NXRect const*)r
{
  [super initFrame:r];
  [self setClipping:NO];
  [self setOpaque:YES];
  [self allocateGState];
  rep = 0;
  return self;
}


//-----------------------------------------------------------------------------
// free
//-----------------------------------------------------------------------------
- (id)free
{
  [rep free];
  return [super free];
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
  NXSize s;
  unsigned int const w = (unsigned int)bounds.size.width;
  unsigned int const h = (unsigned int)bounds.size.height;
  
  unsigned int const samples_per_pixel = 3;	// RGB (A is not included).
  unsigned int const bytes_per_pixel = bits_per_sample / 2;
  unsigned int const bytes_per_row = bytes_per_pixel * w;
  unsigned int const bits_per_pixel = bytes_per_pixel * CHAR_BIT;
  
  assert(bits_per_sample == 4 || bits_per_sample == 8);
  
  rep = [[NXBitmapImageRep allocFromZone:[self zone]]
    initData:buff
    pixelsWide:w
    pixelsHigh:h
    bitsPerSample:bits_per_sample
    samplesPerPixel:samples_per_pixel
    hasAlpha:NO
    isPlanar:NO
    colorSpace:NX_RGBColorSpace
    bytesPerRow:bytes_per_row
    bitsPerPixel:bits_per_pixel];
  s.width  = w;
  s.height = h;
  [rep setSize:&s];
}


//-----------------------------------------------------------------------------
// drawSelf::
//-----------------------------------------------------------------------------
- (id)drawSelf:(NXRect const*)rects :(int)nrects
{
  [rep draw];
  return self;
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
