#ifndef __CocoaFrameBuffer15_h
#define __CocoaFrameBuffer15_h
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
// CocoaFrameBuffer15.h
//
//	A concrete subclass of CocoaFrameBuffer which knows how to convert
//	Crystal Space RGB:555 to Cocoa RGBA:4444.
//
//-----------------------------------------------------------------------------
#include "CocoaFrameBuffer.h"

class CocoaFrameBuffer15 : public CocoaFrameBuffer
{
private:
  unsigned char* raw_buffer;
  unsigned char* cooked_buffer;
  unsigned long buffer_size;
  unsigned short* lookup_table;
  unsigned short* build_15_to_12_rgb_table() const;

public:
  CocoaFrameBuffer15(unsigned int width, unsigned int height);
  virtual ~CocoaFrameBuffer15();

  virtual int depth() const;
  virtual int bits_per_sample() const;
  virtual int bytes_per_pixel() const;
  virtual int palette_entries() const;

  virtual int red_mask() const;
  virtual int green_mask() const;
  virtual int blue_mask() const;

  virtual unsigned char* get_raw_buffer() const;
  virtual unsigned char* get_cooked_buffer() const;

  virtual void cook();
};

#endif // __CocoaFrameBuffer15_h
