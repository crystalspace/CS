//
//  OSXDriver2D.h
//
//
//  Created by mreda on Wed Oct 31 2001.
//  Copyright (c) 2001 Matt Reda. All rights reserved.
//


// This code must be callable from the ObjC delegate.  Since that uses the standard C compiler, it doesn't like
// C++ classes, so we create a C API to some functions of this object

#ifndef __OSXDRIVER2D_H
#define __OSXDRIVER2D_H

#if defined(__cplusplus)

#include "OSXDelegate2D.h"

#include "csgeom/csrect.h"
#include "cssys/next/NeXTAssistant.h"
#include "iutil/eventh.h"
#include "ivideo/graph2d.h"
#include "plugins/video/canvas/common/graph2d.h"

#include <CoreFoundation/CoreFoundation.h>


class OSXDriver2D
{
public:
    // Constructor
    OSXDriver2D(csGraphics2D *inCanvas);

    // Destructor
    virtual ~OSXDriver2D();

    // Initialize 2D plugin
    virtual bool Initialize(iObjectRegistry *reg);

    // Open graphics system (set mode, open window, etc)
    virtual bool Open();

    // Close graphics system
    virtual void Close();

    // Flip video page (or dump to framebuffer) - pure virtual
    virtual void Print(csRect *area = NULL) = 0;

    // Pure virtual function - the driver must invlude code to handle resizing
    virtual bool Resize(int w, int h) = 0;

    // Handle an event
    virtual bool HandleEvent(iEvent &ev);

    // Dispatch an event to the assistant
    inline void DispatchEvent(NeXTEvent ev, NeXTView view);

    // Show/Hide the mouse
    virtual void HideMouse();
    virtual void ShowMouse();

    // Event handler
    struct EventHandler : public iEventHandler
    {
    private:
        OSXDriver2D *parent;
    public:
        EventHandler(OSXDriver2D *p)
        {
            SCF_CONSTRUCT_IBASE(NULL);
            parent = p;
        };

        SCF_DECLARE_IBASE;
        virtual bool HandleEvent (iEvent& e) { return parent->HandleEvent(e); }
    } * scfiEventHandler;

protected:
    // Initialize pixel format for 16 bit depth
    void Initialize16();

    // Initialize pixel format for 32 bit depth
    void Initialize32();

    // Switch to fullscreen mode
    bool EnterFullscreenMode();

    // Switch out of fullscreen mode, to mode stored in originalMode
    void ExitFullscreenMode();

    // Toggle current state of fullscreen
    virtual bool ToggleFullscreen();

protected:

    CFDictionaryRef originalMode;		// Original display mode
    bool inFullscreenMode;			// Flag to indicate that we have correctly switched to fs mode

    int origWidth, origHeight;			// It is necessary to keep the original values so the can be
                                                // restored when switching modes

    OSXDelegate2D delegate;			// Delegate for ObjC stuff
    csGraphics2D *canvas;			// Canvas (parent class)

    iNeXTAssistant *assistant;			// Assistant for dispatching events
    iObjectRegistry *objectReg;			// Object registry
};

#else

#define DRV2D_FUNC(ret, func) __private_extern__ inline ret OSXDriver2D_##func

typedef void *OSXDriver2D;
typedef void *NeXTEventHandle;
typedef void *NeXTViewHandle;

// C API to driver class
DRV2D_FUNC(void, DispatchEvent)(OSXDriver2D driver, NeXTEventHandle ev, NeXTViewHandle view);
DRV2D_FUNC(bool, Resize)(OSXDriver2D driver, int w, int h);
DRV2D_FUNC(void, HideMouse)(OSXDriver2D driver);
DRV2D_FUNC(void, ShowMouse)(OSXDriver2D driver);

#undef DRV2D_FUNC


#endif

#endif
