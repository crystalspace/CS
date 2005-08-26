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
#include "csplugincommon/canvas/scrshot.h"
#include "GLOSXDriver2D.h"

#include <ApplicationServices/ApplicationServices.h>

#define GLOSXDRIVER_REPORTER_ID "crystalspace.canvas.glosx"

// Plugin stuff - create factory functions, etc
CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY(GLOSXDriver2D)

// Constructor
GLOSXDriver2D::GLOSXDriver2D(iBase *p)
    : scfImplementationType (this, p), OSXDriver2D(this)
{
  context = 0;
}


// Destructor
GLOSXDriver2D::~GLOSXDriver2D()
{
  Close();			// In case it hasn't already been called
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
  if ((context = OSXDelegate2D_createOpenGLContext(delegate, Depth, display))
    == 0)
  {
    csFPrintf(stderr, "Failed to create OpenGL context\n");
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
  OSXDelegate2D_updateOpenGLContext(delegate);

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
  CGLClearDrawable(context);
  CGLDestroyContext(context);
}


// SetTitle
// Set window title
void GLOSXDriver2D::SetTitle(char *title)
{
  OSXDelegate2D_setTitle(delegate, title);
  csGraphics2DGLCommon::SetTitle(title);
}


// Print
// Swap OpenGL buffers
void GLOSXDriver2D::Print(csRect const* area)
{
  CGLSetCurrentContext(context);
  CGLFlushDrawable(context);
}


// SetMousePosition
// Set the mouse position
bool GLOSXDriver2D::SetMousePosition(int x, int y)
{
  OSXDelegate2D_setMousePosition(delegate, CGPointMake(x, y));
  return true;
}


// SetMouseCursor
// Set the mouse cursor
bool GLOSXDriver2D::SetMouseCursor(csMouseCursorID cursor)
{
  return OSXDelegate2D_setMouseCursor(delegate, cursor);
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
    OSXDelegate2D_updateOpenGLContext(delegate);
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
