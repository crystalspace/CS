//
//  GLOSXDriver2D.cpp
//
//
//  Created by mreda on Tue Oct 30 2001.
//  Copyright (c) 2001 Matt Reda. All rights reserved.
//

// Must be first include
#include "cssysdef.h"

#include "csutil/scf.h"
#include "csutil/csinput.h"
#include "iutil/eventq.h"
#include "ivaria/reporter.h"
#include "csver.h"
#include "video/canvas/common/scrshot.h"
#include "GLOSXDriver2D.h"

#define GLOSXDRIVER_REPORTER_ID "crystalspace.canvas.glosx"

// Plugin stuff - create factory functions, etc
CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE_EXT (GLOSXDriver2D)
SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iOpenGLInterface)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (GLOSXDriver2D::eiOpenGLInterface)
SCF_IMPLEMENTS_INTERFACE (iOpenGLInterface)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY(GLOSXDriver2D)


// Constructor
GLOSXDriver2D::GLOSXDriver2D(iBase *p)
    : csGraphics2DGLCommon(p), OSXDriver2D(this)
{
  context = 0;
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiOpenGLInterface);
}


// Destructor
GLOSXDriver2D::~GLOSXDriver2D()
{
  Close();			// In case it hasn't already been called
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiOpenGLInterface);
}


// Initialize
// Plugin initialization
bool GLOSXDriver2D::Initialize(iObjectRegistry *reg)
{
  // Initialize both parent classes
  if (csGraphics2DGLCommon::Initialize(reg) == false)
      return false;

  if (OSXDriver2D::Initialize(reg) == false)
      return false;

  // We have to create our context early because all gl functions use it
  // (including things like glString() - the OpenGL renderer was using this
  // before this driver had been Open()'d) When the driver is actually Open()'d
  // all we need to do is bind the context to our window
  if ((context = createOpenGLContext(Depth, display)) == 0)
  {
    fprintf(stderr, "Failed to create OpenGL context\n");
    return false;
  }

  // Create the event outlet
  csRef<iEventQueue> queue = CS_QUERY_REGISTRY(reg, iEventQueue);
  if (queue != 0)
    EventOutlet = queue->CreateEventOutlet (this);

  return true;
}


// Open
// Open the window/switch to fullscreen as necessary
bool GLOSXDriver2D::Open()
{
  // Check if already open has already been called
  if (is_open)
    return true;

  // Report driver information
  csReport(object_reg, CS_REPORTER_SEVERITY_NOTIFY, GLOSXDRIVER_REPORTER_ID,
      CS_PLATFORM_NAME " 2D OpenGL driver for Crystal Space "
      CS_VERSION_NUMBER "\nWritten by Matt Reda <mreda@mac.com>");

  // Initialize base class - will create window, switch mdoes, etx
  if (OSXDriver2D::Open() == false)
      return false;

  // Initialize function pointers
  SetupDrawingFunctions();

  // Context was created in initialize, window was created in
  // OSXDriver2D::Open() - bind them
  updateOpenGLContext();

  // Initialize OpenGL base class
  if (csGraphics2DGLCommon::Open() == false)
      return false;

  return true;
}


// Close
// Close drawing operations
void GLOSXDriver2D::Close()
{
  if (is_open == false)
    return;

  // Close openGL
  csGraphics2DGLCommon::Close();

  // Close window/context
  OSXDriver2D::Close();
  [context clearDrawable];
  [context release];
  context = nil;
}


// SetTitle
// Set window title
void GLOSXDriver2D::SetTitle(char *newTitle)
{
  OSXDriver2D::SetTitle(newTitle);
  csGraphics2DGLCommon::SetTitle(newTitle);
}

// Print
// Swap OpenGL buffers
void GLOSXDriver2D::Print(csRect const* area)
{
  [context makeCurrentContext];
  [context flushBuffer];
}


// AllowResize
// Enable/disable canvas resize
void GLOSXDriver2D::AllowResize(bool allow)
{
  AllowResizing = allow;
}


// Resize
// Resize the canvas
bool GLOSXDriver2D::Resize(int w, int h)
{
  return csGraphics2DGLCommon::Resize(w, h);
}


// ToggleFullscreen
// Toggle between fullscreen/windowed mode
bool GLOSXDriver2D::ToggleFullscreen()
{
  bool success = OSXDriver2D::ToggleFullscreen();
  if (success == true)
    updateOpenGLContext();
  return success;
}


// SetupDrawingFunctions
// Set up the function pointers for drawing based on the current Depth
void GLOSXDriver2D::SetupDrawingFunctions()
{
  if (Depth == 32)
  {
    _DrawPixel = DrawPixel32;
    _GetPixelAt = GetPixelAt32;
  }
  else	// Depth is 16
  {
    _DrawPixel = DrawPixel16;
    _GetPixelAt = GetPixelAt16;
  }
}

// Create an OpenGL contexts
NSOpenGLContext *GLOSXDriver2D::createOpenGLContext(int depth, 
                                                    CGDirectDisplayID display)
{
    NSOpenGLPixelFormat *pixelFormat;

    // Attributes for OpenGL contexts
    NSOpenGLPixelFormatAttribute attribs[] = {
        NSOpenGLPFAWindow, NSOpenGLPFADoubleBuffer, NSOpenGLPFAAccelerated,
        NSOpenGLPFAColorSize, (NSOpenGLPixelFormatAttribute) depth, 
        NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute) depthBits,
#ifndef CS_USE_NEW_RENDERER
        NSOpenGLPFAStencilSize, (NSOpenGLPixelFormatAttribute) 1,
#else
        NSOpenGLPFAStencilSize, (NSOpenGLPixelFormatAttribute) 8,
#endif
        NSOpenGLPFAScreenMask, 
        (NSOpenGLPixelFormatAttribute) CGDisplayIDToOpenGLDisplayMask(display), 
        (NSOpenGLPixelFormatAttribute) nil
    };

    // Create a pixel format
    pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
    if (pixelFormat == nil)
        return 0;

    // Create a GL context
    context = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
    [pixelFormat release];
    if (context == nil)
        return 0;

    // Need to know when window is resized so we can update the OpenGL context
    if (window != nil)
    {
        [[NSNotificationCenter defaultCenter] addObserver:delegate 
                selector:@selector(windowResized:)
                name:NSWindowDidResizeNotification object:window];

        // Bind context
        [context setView:[window contentView]];
    }

    // Make the context we created be the current GL context
    [context makeCurrentContext];

    return context;
}

// Bind the given context to our window
void GLOSXDriver2D::updateOpenGLContext()
{
    // Listen for resizes on new window
    [[NSNotificationCenter defaultCenter] addObserver:delegate 
                            selector:@selector(windowResized:)
                            name:NSWindowDidResizeNotification object:window];

    [context setView:[window contentView]];
    [context makeCurrentContext];
    [context update];
}
