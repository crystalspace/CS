//
//  OSXDriver2D.cpp
//
//
//  Created by mreda on Wed Oct 31 2001.
//  Copyright (c) 2001 Matt Reda. All rights reserved.
//


// Must be first include
#include "cssysdef.h"
#include "csutil/scf.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include "csver.h"

#include "OSXDriver2D.h"

#include <ApplicationServices/ApplicationServices.h>

SCF_IMPLEMENT_IBASE(OSXDriver2D::EventHandler)
  SCF_IMPLEMENTS_INTERFACE(iEventHandler)
SCF_IMPLEMENT_IBASE_END


// Constructor
// Initialize graphics driver
OSXDriver2D::OSXDriver2D(csGraphics2D *inCanvas)
{
    scfiEventHandler = NULL;

    canvas = inCanvas;
    originalMode = NULL;

    origWidth = 0;
    origHeight = 0;

    delegate = OSXDelegate2D_new(this);
};


// Destructor
// Destroy driver
OSXDriver2D::~OSXDriver2D()
{
    if (scfiEventHandler != NULL)
    {
        iEventQueue *queue = CS_QUERY_REGISTRY(objectReg, iEventQueue);
        if (queue != NULL)
        {
            queue->RemoveListener(scfiEventHandler);
            queue->DecRef();
        }
        scfiEventHandler->DecRef();
    }

    Close();					// Just in case it hasn't been called

    OSXDelegate2D_delete(delegate);

    // Release assistant
    if (assistant != NULL)
        assistant->DecRef();
};


// Initialize
// Initialize 2D canvas plugin
bool OSXDriver2D::Initialize(iObjectRegistry *reg)
{
    objectReg = reg;

    // Get assistant
    assistant = CS_QUERY_REGISTRY(reg, iNeXTAssistant);

    // Create event handler
    if (scfiEventHandler == NULL)
        scfiEventHandler = new EventHandler(this);

    // Listen for key down events
    iEventQueue* queue = CS_QUERY_REGISTRY(reg, iEventQueue);
    if (queue != 0)
    {
        queue->RegisterListener(scfiEventHandler, CSMASK_Broadcast | CSMASK_KeyDown);
        queue->DecRef();
    }

    // Get and save original mode - released in Close()
    originalMode = CGDisplayCurrentMode(kCGDirectMainDisplay);

    return true;
};


// Open
// Open graphics system (set mode, open window, etc)
bool OSXDriver2D::Open()
{
    // Copy original values
    origWidth = canvas->Width;
    origHeight = canvas->Height;

        // Set up pixel format
    if (canvas->Depth == 32)
        Initialize32();
    else if (canvas->Depth == 16)
        Initialize16();
    else
    {
        fprintf(stderr, "Depth %d not supported in CGDriver2D yet", canvas->Depth);
        return false;
    };

    // Switch to fullscreen mode if necessary
    if (canvas->FullScreen == true)
        if ((inFullscreenMode = EnterFullscreenMode()) == false)
            return false;

    // Create window
    if (OSXDelegate2D_openWindow(delegate, canvas->win_title,
                                    canvas->Width, canvas->Height, canvas->Depth, canvas->FullScreen) == false)
        return false;

    return true;
};


// Close
// Close graphics system
void OSXDriver2D::Close()
{
    // Close window
    OSXDelegate2D_closeWindow(delegate);

    // If we're in fullscreen mode, get out of it
    if (inFullscreenMode == true)
    {
        ExitFullscreenMode();
        inFullscreenMode = false;
    };
};


// HandleEvent
// Handle an event
// Look for Alt-Enter to toggle fullscreen mode
bool OSXDriver2D::HandleEvent(iEvent &ev)
{
    bool handled = false;

    if (ev.Type == csevBroadcast)
    {
        if (ev.Command.Code == cscmdFocusChanged)
        {
            OSXDelegate2D_focusChanged(delegate, ev.Command.Info);
            handled = true;
        };
    }
    else if (ev.Type == csevKeyDown)
    {
        if ((ev.Key.Code == CSKEY_ENTER) && (ev.Key.Modifiers & CSMASK_ALT))
            handled = ToggleFullscreen();
    }

    return handled;
};


// DispatchEvent
// Dispatch an event to the assistant
void OSXDriver2D::DispatchEvent(NeXTEvent ev, NeXTView view)
{
    assistant->dispatch_event(ev, view);
};


// HideMouse
// Hide the mouse
void OSXDriver2D::HideMouse()
{
    assistant->hide_mouse_pointer();
};


// ShowMouse
// Show the mouse cursor
void OSXDriver2D::ShowMouse()
{
    assistant->show_mouse_pointer();
};


// Initialize16
// Initialize pixel format for 16 bit depth
void OSXDriver2D::Initialize16()
{
    canvas->pfmt.PalEntries = 0;
    canvas->pfmt.PixelBytes = 2;
    canvas->pfmt.RedMask = 0x1F << 10;
    canvas->pfmt.GreenMask = 0x1F << 5;
    canvas->pfmt.BlueMask = 0x1F;
    canvas->pfmt.complete();
};


// Initialize32
// Initialize pixel format for 32 bit depth
void OSXDriver2D::Initialize32()
{
    canvas->pfmt.PalEntries = 0;
    canvas->pfmt.PixelBytes = 4;
    canvas->pfmt.RedMask = 0xFF0000;
    canvas->pfmt.GreenMask = 0x00FF00;
    canvas->pfmt.BlueMask = 0x0000FF;
    canvas->pfmt.complete();
};


// EnterFullscreenMode
// Switch to fullscreen mode - returns true on success
bool OSXDriver2D::EnterFullscreenMode()
{
    // Find mode and copy parameters
    CFDictionaryRef mode = CGDisplayBestModeForParameters(kCGDirectMainDisplay,
                                                            canvas->Depth, canvas->Width, canvas->Height, NULL);
    if (mode == NULL)
        return false;

    // Lock displays
    if (CGCaptureAllDisplays() != CGDisplayNoErr)
        return false;

    // Switch to new mode
    if (CGDisplaySwitchToMode(kCGDirectMainDisplay, mode) == CGDisplayNoErr)
    {
        // Extract actual Width/Height/Depth
        CFNumberGetValue((CFNumberRef) CFDictionaryGetValue(mode, kCGDisplayWidth), kCFNumberLongType,
                                                                                    &canvas->Width);
        CFNumberGetValue((CFNumberRef) CFDictionaryGetValue(mode, kCGDisplayHeight), kCFNumberLongType,
                                                                                    &canvas->Height);
        CFNumberGetValue((CFNumberRef) CFDictionaryGetValue(mode, kCGDisplayBitsPerPixel), kCFNumberLongType,
                                                                                    &canvas->Depth);
    }
    else
    {
        CGReleaseAllDisplays();
        return false;
    };

    return true;
};


// ExitFullscreenMode
// Switch out of fullscreen mode, to mode stored in originalMode
void OSXDriver2D::ExitFullscreenMode()
{
    CGDisplaySwitchToMode(kCGDirectMainDisplay, originalMode);
    CGReleaseAllDisplays();
};


// ToggleFullscreen
// Toggle current state of fullscreen
bool OSXDriver2D::ToggleFullscreen()
{
    bool oldAllowResizing = canvas->AllowResizing;
    bool success = true;

    OSXDelegate2D_closeWindow(delegate);

    if (canvas->FullScreen == true)
    {
        ExitFullscreenMode();
        inFullscreenMode = false;
    };

    // Restore original dimensions - force resize by forcing AllowResize to true
    canvas->FullScreen = !canvas->FullScreen;
    canvas->AllowResizing = true;
    canvas->Resize(origWidth, origHeight);
    canvas->AllowResizing = oldAllowResizing;

    if ((success == true) && (canvas->FullScreen == true))
    {
        inFullscreenMode = EnterFullscreenMode();
        success = inFullscreenMode;
    };

    if (success == true)
        OSXDelegate2D_openWindow(delegate, canvas->win_title,
                                    canvas->Width, canvas->Height, canvas->Depth, canvas->FullScreen);

    return success;
};



/// C API to driver class
#define DRV2D_FUNC(ret, func) __private_extern__ "C" ret OSXDriver2D_##func

typedef void *OSXDriver2DHandle;
typedef void *NeXTEventHandle;
typedef void *NeXTViewHandle;


// C API to driver class
DRV2D_FUNC(void, DispatchEvent)(OSXDriver2DHandle driver, NeXTEventHandle ev, NeXTViewHandle view)
{
    ((OSXDriver2D *) driver)->DispatchEvent(ev, view);
}

DRV2D_FUNC(bool, Resize)(OSXDriver2DHandle driver, int w, int h)
{
    return ((OSXDriver2D *) driver)->Resize(w, h);
}

DRV2D_FUNC(void, HideMouse)(OSXDriver2DHandle driver)
{
    ((OSXDriver2D *) driver)->HideMouse();
};

DRV2D_FUNC(void, ShowMouse)(OSXDriver2DHandle driver)
{
    ((OSXDriver2D *) driver)->ShowMouse();
};


#undef DRV2D_FUNC


