#ifndef __NeXTFrameBuffer15_h
#define __NeXTFrameBuffer15_h
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
// NeXTFrameBuffer15.h
//
//	A concrete subclass of NeXTFrameBuffer which knows how to convert
//	Crystal Space RGB:555 to NeXT RGBA:4444.
//
//-----------------------------------------------------------------------------
#include "NeXTFrameBuffer.h"

class NeXTFrameBuffer15 : public NeXTFrameBuffer
{
private:
  unsigned char* raw_buffer;
  unsigned char* cooked_buffer;
  unsigned long buffer_size;
  unsigned short* lookup_table;
  unsigned short* build_15_to_12_rgb_table() const;

public:
  NeXTFrameBuffer15(unsigned int width, unsigned int height);
  virtual ~NeXTFrameBuffer15();

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

#endif // __NeXTFrameBuffer15_h
