#ifndef __CSSYS_MACOSX_OSXAssistant_h
#define __CSSYS_MACOSX_OSXAssistant_h
//=============================================================================
//
//	Copyright (C)1999-2003 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
//-----------------------------------------------------------------------------
// OSXAssistant.h
//
//	Declares SCF interface for an object which provides assistance to
//	MacOS/X-specific canvases, as well as to the Objective-C side of the
//	application delegate.
//
//-----------------------------------------------------------------------------
#include "csutil/scf.h"

/**\file
 */

/// An opaque handle to an Objective-C event record represented in C++.
typedef void* OSXEvent;
/// An opaque handle to an Objective-C view object represented in C++.
typedef void* OSXView;

SCF_VERSION (iOSXAssistant, 0, 0, 2);

/**
 * This is an interface for an object which provides assistance to
 * MacOS/X-specific canvases and Objective-C bridging code for application
 * run-loop support.  An instance of this object will be registered with the
 * object registry with tag 'OSXAssistant', however it is generally
 * recommended that you query the object registry for this object by simply
 * asking for the object implementing iOSXAssistant.
 *
 * \remarks As the name suggests, this interface provides functionality
 *  specific to the MacOS/X platform. To ensure that code using this 
 *  functionality compiles properly on all other platforms, the use of the
 *  interface and inclusion of the header file should be surrounded by
 *  appropriate '\#if defined(CS_PLATFORM_MACOSX) ... \#endif' statements.
 */
struct iOSXAssistant : public iBase
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
   * Query whether the application's event loop stops processing events
   * when the application does not have focus
   */
  virtual bool always_runs() = 0;

  /**
   * Query whether or not the application's event loop should continue running.
   * This will return `true' until some Crystal Space entity requests a
   * shutdown, in which case the caller of this method should proceed to
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
   * Notify Crystal Space that the AppKit application has been hidden.  This
   * causes a `cscmdCanvasHidden' event to be immediately posted to the Crystal
   * Space event queue.
   */
  virtual void application_hidden() = 0;

  /** 
   * Notify Crystal Space that the AppKit application has been unhidden.  This
   * causes a `cscmdCanvasExposed' event to be immediately posted to the
   * Crystal Space event queue.
   */
  virtual void application_unhidden() = 0;

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
   * allow this method to do so.  The first argument is a pointer to an
   * NSEvent, but is cast as an opaque OSXEvent handle for representation in
   * the C++ world.  The second argument is a pointer to the view with which
   * the event is associated, or 0 if not associated with any view.  The
   * view argument refers to an NSView, but is cast as an opaque OSXView
   * handle for representation in the C++ world.
   */
  virtual void dispatch_event(OSXEvent, OSXView) = 0;

  /**
   * Post a key-down event to the Crystal Space event queue.  The first number
   * is the raw key code, and the second is the cooked character code.
   */
  virtual void key_down(unsigned int raw, unsigned int cooked) = 0;

  /**
   * Post a key-up event to the Crystal Space event queue.  The first number is
   * the raw key code, and the second is the cooked character code.
   */
  virtual void key_up(unsigned int raw, unsigned int cooked) = 0;

  /**
   * Post a mouse-down event to the Crystal Space event queue.  The
   * mouse-button number is 1-based.  The coordinates are specified in terms of
   * the Crystal Space coordinate system where `x' increases from left to
   * right, and `y' increases from top to bottom.
   */
  virtual void mouse_down(int button, int x, int y) = 0;

  /**
   * Post a mouse-up event to the Crystal Space event queue.  The mouse-button
   * number is 1-based.  The coordinates are specified in terms of the Crystal
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

#endif // __CSSYS_MACOSX_OSXAssistant_h
