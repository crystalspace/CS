#ifndef __NeXTFrameBuffer_h
#define __NeXTFrameBuffer_h
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
// NeXTFrameBuffer.h
//
//	An abstract 2D frame buffer which is capable of converting a raw
//	Crystal Space frame buffer into a NeXT-format frame buffer.
//
//	Subclasses must implement the following methods:
//
//	depth
//	    Used to initialize csGraphics2D::Depth.
//	bytes_per_pixel, palette_entries, red_mask, green_mask, blue_mask
//	    Used to initialize the csPixelFormat structure.
//	bits_per_sample
//	    Used by NeXTView to initialize its bitmap.
//	get_raw_buffer
//	    Returns the buffer into which Crystal Space should render the
//	    scene.  This buffer should be large enough to contain
//	    (width * height) pixels at the given depth.  Allocation and
//	    deallocation of this buffer is the responsibility of the subclass.
//	    This method is called only once during initialization of
//	    csGraphics2D::Memory.
//	get_cooked_buffer
//	    Returns the buffer into which the subclass will cook the raw data.
//	    This buffer should be large enough to contain (width * height)
//	    pixels in a format suitable for the NeXT Window Server.
//	    Allocation and deallocation of this buffer is the responsibility
//	    of the subclass.  This method may return the same value as that
//	    returned by get_raw_buffer() if the cooking process is performed
//	    in place.  It is called only once during initialization of the
//	    NeXTView object which actually flushes the data to the screen.
//	cook
//	    Converts the raw Crystal Space data into a format suitable for use
//	    by the NeXT Window Server.  After calling cook(), the data in the
//	    "cooked" buffer should be in a format native to the NeXT Window
//	    Server (or as close as possible) such that the Window Server has
//	    to do little or no work to actually display the data on-screen.
//
//-----------------------------------------------------------------------------
class NeXTFrameBuffer
{
protected:
  unsigned int width;
  unsigned int height;

public:
  NeXTFrameBuffer(unsigned int w, unsigned int h) : width(w),height(h) {}
  virtual ~NeXTFrameBuffer() {}

  virtual int depth() const = 0;
  virtual int bits_per_sample() const = 0;
  virtual int bytes_per_pixel() const = 0;
  virtual int palette_entries() const = 0;
  
  virtual int red_mask() const = 0;
  virtual int green_mask() const = 0;
  virtual int blue_mask() const = 0;
  
  virtual unsigned char* get_raw_buffer() const = 0;
  virtual unsigned char* get_cooked_buffer() const = 0;
  
  virtual void cook() = 0;
};

#endif // __NeXTFrameBuffer_h
