#ifndef __cssys_NeXT_NeXTAssistant_h
#define __cssys_NeXT_NeXTAssistant_h
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
// NeXTAssistant.h
//
//	Declares SCF interface for an object which provides assistance to
//	NeXT-specific canvases, as well as to the Objective-C side of the
//	application delegate.
//
//	This object is available to the MacOS/X, MacOS/X Server 1.0 (Rhapsody),
//	OpenStep, and NextStep ports of Crystal Space.
//
//-----------------------------------------------------------------------------
#include "csutil/scf.h"

/// An opaque handle to an Objective-C event record represented in C++.
typedef void* NeXTEvent;
/// An opaque handle to an Objective-C view object represented in C++.
typedef void* NeXTView;

SCF_VERSION (iNeXTAssistant, 0, 0, 1);

/**
 * This is an interface for an object which provides assistance to
 * NeXT-specific canvases and Objective-C bridging code for application
 * run-loop support.  An instance of this object will be registered to the
 * object registry with tag 'NeXTAssistant', however it is generally
 * recommended that you query the object registry for this object by simply
 * asking for the object implementing NeXTAssistant.
 */
struct iNeXTAssistant : public iBase
{
  /**
   * Ask to have both the AppKit and Crystal Space event-loops terminated.
   */
  virtual void request_shutdown() = 0;

  /**
   * Sent by some mechanism on a periodic basis to process the Crystal Space
   * event queue and advance the state of the engine and all plugins.
   */
  virtual void advance_state() = 0;

  /**
   * Query whether or not the application's event loop should continue running.
   * This will return `false' until some Crystal Space entity requests a
   * shutdown, in which case the caller of this method will proceed to
   * terminate the AppKit's run-loop.
   */
  virtual bool continue_running() = 0;

  /**
   * Notify Crystal Space that the AppKit application has come to the
   * foreground.  This causes a `cscmdFocusChanged' event to be posted to the
   * Crystal Space event queue.
   */
  virtual void application_activated() = 0;

  /**
   * Notify Crystal Space that the AppKit application has been sent to the
   * background.  This causes a `cscmdFocusChanged' event to be posted to the
   * Crystal Space event queue.
   */
  virtual void application_deactivated() = 0;

  /**
   * Flush the connection to the current graphics context (the Quartz or DPS
   * server, for instance).  This forces the graphics context to perform all
   * pending drawing operations.
   */
  virtual void flush_graphics_context() = 0;

  /**
   * Hide the mouse pointer.
   */
  virtual void hide_mouse_pointer() = 0;

  /**
   * Unhide the mouse pointer.
   */
  virtual void show_mouse_pointer() = 0;

  /**
   * Interpret an AppKit event and post the appropriate csEvent to the Crystal
   * Space event queue.  This is a convenience method for canvases.  It
   * performs all of the difficult work of interpreting an AppKit event and
   * translating it into a csEvent.  Canvases are free to do their own
   * interpretation of events and instead use one of the methods below for
   * directly placing a csEvent in the queue, but it is much more convenient to
   * allow this method to do so.  The first argument is a pointer to an NSEvent
   * (Cocoa and OpenStep) or an NXEvent (NextStep), but cast as an opaque
   * NeXTEvent handle for representation in the C++ world.  The second argument
   * is a pointer to the view with which the event is associated, or NULL if
   * not associated with any view.  The view argument refers to an NSView
   * (Cocoa and OpenStep) or a View (NextStep), but cast as an opaque NeXTView
   * handle for representation in the C++ world.
   */
  virtual void dispatch_event(NeXTEvent, NeXTView) = 0;

  /**
   * Post a key-down event to the Crystal Space event queue.  The first number
   * is the raw key code, and the second is the cooked character code.
   */
  virtual void key_down(int raw, int cooked) = 0;

  /**
   * Post a key-up event to the Crystal Space event queue.  The first number is
   * the raw key code, and the second is the cooked character code.
   */
  virtual void key_up(int raw, int cooked) = 0;

  /**
   * Post a mouse-down event to the Crystal Space event queue.  The
   * mouse-button number 1-based.  The coordinates are specified in terms of
   * the Crystal Space coordinate system where `x' increases from left to
   * right, and `y' increases from top to bottom.
   */
  virtual void mouse_down(int button, int x, int y) = 0;

  /**
   * Post a mouse-up event to the Crystal Space event queue.  The mouse-button
   * number 1-based.  The coordinates are specified in terms of the Crystal
   * Space coordinate system where `x' increases from left to right, and `y'
   * increases from top to bottom.
   */
  virtual void mouse_up(int button, int x, int y) = 0;

  /**
   * Post a mouse-moved event to the Crystal Space event queue.  The
   * coordinates are specified in terms of the Crystal Space coordinate system
   * where `x' increases from left to right, and `y' increases from top to
   * bottom.
   */
  virtual void mouse_moved(int x, int y) = 0;
};

#endif // __cssys_NeXT_NeXTAssistant_h
