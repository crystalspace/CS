/*
 *  CGDriver2D.cpp
 *
 *
 *  Created by mreda on Fri Oct 26 2001.
 *  Copyright (c) 2001 Matt Reda. All rights reserved.
 *
 */

// Must be first include
#include "cssysdef.h"

#include "csutil/scf.h"
#include "csutil/csinput.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "ivaria/reporter.h"
#include "csver.h"

#include "CGDriver2D.h"

#include "OSXDelegate2D_CGBlit.h"

#include <ApplicationServices/ApplicationServices.h>


#define CGDRIVER_REPORTER_ID "crystalspace.canvas.coregraphics"


CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY(CGDriver2D)




// Constructor
// Construct a graphics object for drawing
CGDriver2D::CGDriver2D(iBase *p) : csGraphics2D(p), OSXDriver2D(this)
{
}


// Destructor
// Clean up
CGDriver2D::~CGDriver2D()
{
    Close();		// Just in case it hasn't been called
}


// Initialize
// Initialize 2D plugin
bool CGDriver2D::Initialize(iObjectRegistry *reg)
{
    if (csGraphics2D::Initialize(reg) == false)
        return false;

    if (OSXDriver2D::Initialize(reg) == false)
        return false;

    return true;
}


// Open
// Open graphics system (set mode, open window, etc)
bool CGDriver2D::Open()
{
    // Check if already open has already been called
    if (is_open == true)
        return true;

    // Report driver information
    csReport(object_reg, CS_REPORTER_SEVERITY_NOTIFY, CGDRIVER_REPORTER_ID,
            CS_PLATFORM_NAME " 2D CoreGraphics driver for Crystal Space "
            CS_VERSION_NUMBER "\nWritten by Matt Reda <mreda@mac.com>");

    // Superclass implementation
    if (OSXDriver2D::Open() == false)
        return false;

    // Initialize function pointers
    SetupDrawingFunctions();

    // Allocate memory for drawing
    Memory = (unsigned char *) malloc(Width * Height * pfmt.PixelBytes);

    if (csGraphics2D::Open() == false)
        return false;

    // Start by clearing view
    BeginDraw();
    Clear(0);
    FinishDraw();
    Print(0);

    return true;
}


// Close
// Close graphics system
void CGDriver2D::Close()
{
    // Make sure we have actually been Open()'d
    if (is_open == false)
        return;

    // Free drawing buffer
    free(Memory);
    Memory = 0;

    // Superclasses
    csGraphics2D::Close();
    OSXDriver2D::Close();
}


// SetTitle
// Set window title
void CGDriver2D::SetTitle(char *title)
{
    OSXDelegate2D_setTitle(delegate, title);
    csGraphics2D::SetTitle(title);
}


// Print
// Flip video page (or dump to framebuffer)
void CGDriver2D::Print(csRect const* area)
{
    OSXDelegate2D_blitToWindow(delegate, Memory, Width, Height, Depth);
}


// SetMousePosition
// Set the mouse position
bool CGDriver2D::SetMousePosition(int x, int y)
{
    OSXDelegate2D_setMousePosition(delegate, CGPointMake(x, y));
    return true;
}


// SetMouseCursor
// Set the mouse cursor
bool CGDriver2D::SetMouseCursor(csMouseCursorID cursor)
{
    return OSXDelegate2D_setMouseCursor(delegate, cursor);
}


// AllowResize
// Enable/disable canvas resize
void CGDriver2D::AllowResize (bool allow)
{
    AllowResizing = allow;
}


// Resize
// Resize the canvas
bool CGDriver2D::Resize(int w, int h)
{
    bool success = csGraphics2D::Resize(w, h);

    if (success == true)
    {
        // Need to allocate new buffer
        Memory = (unsigned char *) realloc(Memory, Width * Height * pfmt.PixelBytes);

        // Should CGDriver2D inherit from iEventPlug and get it's own outlet?
        csRef<iEventQueue> queue = CS_QUERY_REGISTRY(object_reg, iEventQueue);
        if (queue.IsValid())
            queue->GetEventOutlet()->Broadcast(cscmdContextResize, (intptr_t) this);
    }

    return success;
}



// SetupDrawingFunctions
// Set up the function pointers for drawing based on the current Depth
void CGDriver2D::SetupDrawingFunctions()
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
