#ifndef __NeXT_NeXTDriver2D_h
#define __NeXT_NeXTDriver2D_h
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
// NeXTDriver2D.h
//
//	NeXT-specific subclass of csGraphics2D which implements 2D graphic
//	functionality via the AppKit.  This file contains code which is shared
//	between the MacOS/X, MacOS/X Server 1.0 (Rhapsody), OpenStep, and
//	NextStep platforms.  See NeXTDelegate2D.h for the platform-specific
//	portion of the 2D driver implementation.
//
//-----------------------------------------------------------------------------
#if defined(__cplusplus)

#include "video/canvas/common/graph2d.h"
#include "NeXTDelegate2D.h"
class NeXTFrameBuffer;

class NeXTDriver2D : public csGraphics2D
{
  typedef csGraphics2D superclass;

protected:
  NeXTDelegate2D controller;
  NeXTFrameBuffer* frame_buffer;

  bool init_driver(int desired_depth);
  int  get_desired_depth() const;
  int  determine_bits_per_sample(int desired_depth) const;
  void setup_rgb_15();
  void setup_rgb_32();
  void usage_summary() const;

public:
  NeXTDriver2D(iBase* p) : superclass(p), controller(0), frame_buffer(0) {}
  virtual ~NeXTDriver2D();
  virtual bool Initialize(iSystem*);
  virtual bool Open(char const* title);
  virtual void Close();
  virtual void Print(csRect* = 0);
  virtual bool SetMouseCursor(csMouseCursorID);
  virtual bool HandleEvent(iEvent&);

  bool system_extension(char const* msg, void* = 0, void* = 0) const;
  void user_close() const;
};

#else // __cplusplus

#define N2D_PROTO(RET,FUNC) extern RET NeXTDriver2D_##FUNC

typedef void* NeXTDriver2D;

N2D_PROTO(void,user_close)(NeXTDriver2D);
N2D_PROTO(int,system_extension)
    (NeXTDriver2D, char const* msg, void* data1, void* data2);

#undef N2D_PROTO

#endif // __cplusplus

#endif //__NeXT_NeXTDriver2D_h
