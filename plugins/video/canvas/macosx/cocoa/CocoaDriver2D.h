#ifndef __Cocoa_CocoaDriver2D_h
#define __Cocoa_CocoaDriver2D_h
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
// CocoaDriver2D.h
//
//	Cocoa-specific subclass of csGraphics2D which implements 2D graphic
//	functionality via the AppKit.  This is a very high-level iGraphics2D
//	implementation.  No specialized agents (such as CoreGraphics or OpenGL)
//	are employed.  See CocoaDelegate2D.h for the Cocoa-specific portion of
//	the 2D driver implementation.
//
//-----------------------------------------------------------------------------
#if defined(__cplusplus)

#include "video/canvas/common/graph2d.h"
#include "cssys/macosx/OSXAssistant.h"
#include "CocoaDelegate2D.h"
class CocoaFrameBuffer;

class CocoaDriver2D : public csGraphics2D
{
  typedef csGraphics2D superclass;

protected:
  CocoaDelegate2D controller;
  csRef<iOSXAssistant> assistant;
  CocoaFrameBuffer* frame_buffer;

  bool init_driver(int desired_depth);
  int  get_desired_depth() const;
  int  determine_bits_per_sample(int desired_depth) const;
  void setup_rgb_15();
  void setup_rgb_32();
  void usage_summary() const;

public:
  CocoaDriver2D(iBase* p) :
    superclass(p), controller(0), assistant(0), frame_buffer(0) {}
  virtual ~CocoaDriver2D();
  virtual bool Initialize(iObjectRegistry*);
  virtual bool Open();
  virtual void Close();
  virtual void SetTitle(char const*);
  virtual void Print(csRect* = 0);
  virtual bool SetMouseCursor(csMouseCursorID);
  virtual bool HandleEvent(iEvent&);

  void user_close();
  void flush_graphics_context();
  void hide_mouse_pointer();
  void show_mouse_pointer();
  void dispatch_event(OSXEvent, OSXView);
};

#else // __cplusplus

#define N2D_PROTO(RET,FUNC) extern RET CocoaDriver2D_##FUNC

typedef void* CocoaDriver2D;
typedef void* OSXEventHandle;
typedef void* OSXViewHandle;

N2D_PROTO(void,user_close)(CocoaDriver2D);
N2D_PROTO(void,flush_graphics_context)(CocoaDriver2D);
N2D_PROTO(void,hide_mouse_pointer)(CocoaDriver2D);
N2D_PROTO(void,show_mouse_pointer)(CocoaDriver2D);
N2D_PROTO(void,dispatch_event)(CocoaDriver2D, OSXEventHandle, OSXViewHandle);

#undef N2D_PROTO

#endif // __cplusplus

#endif //__Cocoa_CocoaDriver2D_h
