/*
 *  CGDriver2D.h
 *
 *
 *  Created by mreda on Fri Oct 26 2001.
 *  Copyright (c) 2001 Matt Reda. All rights reserved.
 *
 */

// This code must be callable from the ObjC delegate.  Since that uses the standard C compiler, it doesn't like
// C++ classes, so we create a C API to some functions of this object

#ifndef __CGDRIVER2D_H
#define __CGDRIVER2D_H


#include "OSXDriver2D.h"


#if defined(__cplusplus)

#include "cssys/next/NeXTAssistant.h"
#include "video/canvas/common/graph2d.h"

#include <CoreFoundation/CoreFoundation.h>


class CGDriver2D : public csGraphics2D, public OSXDriver2D
{
public:
    // Constructor
    CGDriver2D(iBase *p);

    // Destructor
    virtual ~CGDriver2D();

    // Initialize 2D plugin
    virtual bool Initialize(iObjectRegistry *reg);

    // Open graphics system (set mode, open window, etc)
    virtual bool Open();

    // Close graphics system
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

protected:

    // Set up the function pointers for drawing based on the current Depth
    virtual void SetupDrawingFunctions();
};

#endif

#endif
