//
//  GLOSXDriver2D.h
//
//
//  Created by mreda on Tue Oct 30 2001.
//  Copyright (c) 2001 Matt Reda. All rights reserved.
//


#ifndef __GLOSXDRIVER2D_H
#define __GLOSXDRIVER2D_H

#include "OSXDriver2D.h"


#if defined(__cplusplus)


#include "cssys/next/NeXTAssistant.h"
#include "video/canvas/openglcommon/glcommon2d.h"

#include "OSXDelegate2D_OpenGL.h"

#include <CoreFoundation/CoreFoundation.h>



class GLOSXDriver2D : public csGraphics2DGLCommon, public OSXDriver2D {
public:
    // Constructor
    GLOSXDriver2D(iBase *p);

    // Destructor
    virtual ~GLOSXDriver2D();

    // Plugin initialization
    virtual bool Initialize (iObjectRegistry *reg);

    // Open the window/switch to fullscreen as necessary
    virtual bool Open();

    // Close drawing operations
    virtual void Close();

    // Set window title
    virtual void SetTitle(char *title);

    // Flip video page (or dump to framebuffer)
    virtual void Print(csRect *area = NULL);

    // Set the mouse cursor
    virtual bool SetMouseCursor(csMouseCursorID cursor);

    // Enable/disable canvas resize
    virtual void AllowResize(bool allow);

    // Resize the canvas
    virtual bool Resize(int w, int h);

    // Toggle between fullscreen/windowed mode
    virtual bool ToggleFullscreen();

protected:

    // Set up the function pointers for drawing based on the current Depth
    virtual void SetupDrawingFunctions();

    // OpenGL context for drawing
    CGLContextObj context;
};


#endif


#endif
