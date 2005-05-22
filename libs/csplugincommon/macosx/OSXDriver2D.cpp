//
//  OSXDriver2D.cpp
//
//
//  Created by mreda on Wed Oct 31 2001.
//  Copyright (c) 2001 Matt Reda. All rights reserved.
//

#include "cssysdef.h"
#include "csutil/scf.h"
#include "iutil/cmdline.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include "csver.h"
#include "csutil/event.h"

#include "csplugincommon/macosx/OSXDriver2D.h"

#include <sys/time.h>


SCF_IMPLEMENT_IBASE(OSXDriver2D::EventHandler)
  SCF_IMPLEMENTS_INTERFACE(iEventHandler)
SCF_IMPLEMENT_IBASE_END


// Constructor
// Initialize graphics driver
OSXDriver2D::OSXDriver2D(csGraphics2D *inCanvas)
{
    scfiEventHandler = 0;

    canvas = inCanvas;
    originalMode = 0;

    origWidth = 0;
    origHeight = 0;
    display = kCGDirectMainDisplay;

    delegate = OSXDelegate2D_new(this);
}


// Destructor
// Destroy driver
OSXDriver2D::~OSXDriver2D()
{
    if (scfiEventHandler != 0)
    {
        csRef<iEventQueue> queue = CS_QUERY_REGISTRY(objectReg, iEventQueue);
        if (queue.IsValid())
            queue->RemoveListener(scfiEventHandler);
        scfiEventHandler->DecRef();
    }

    Close();	// Just in case it hasn't been called

    OSXDelegate2D_delete(delegate);
}


// Initialize
// Initialize 2D canvas plugin
bool OSXDriver2D::Initialize(iObjectRegistry *reg)
{
    objectReg = reg;

    // Get assistant
    assistant = CS_QUERY_REGISTRY(reg, iOSXAssistant);

    // Create event handler
    if (scfiEventHandler == 0)
        scfiEventHandler = new EventHandler(this);

    // Listen for key down events
    csRef<iEventQueue> queue = CS_QUERY_REGISTRY(reg, iEventQueue);
    if (queue.IsValid())
        queue->RegisterListener(scfiEventHandler, 
	CSMASK_Broadcast | CSMASK_Keyboard);

    // Figure out what screen we will be using
    ChooseDisplay();

    // Get and save original mode and gamma - released in Close()
    originalMode = CGDisplayCurrentMode(display);
    SaveGamma(display, originalGamma);

    return true;
}


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
        csFPrintf(stderr, "Depth %d not supported in CGDriver2D yet",
	    canvas->Depth);
        return false;
    }

    // Switch to fullscreen mode if necessary
    if (canvas->FullScreen == true)
        if ((inFullscreenMode = EnterFullscreenMode()) == false)
            return false;

    // Create window
    if (OSXDelegate2D_openWindow(delegate, canvas->win_title,
	canvas->Width, canvas->Height, canvas->Depth,
	canvas->FullScreen, display, screen) == false)
        return false;

    return true;
}


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
    }
}


// HandleEvent
// Handle an event
// Look for Alt-Enter to toggle fullscreen mode
bool OSXDriver2D::HandleEvent(iEvent &ev)
{
    bool handled = false;

    if (ev.Type == csevBroadcast)
    {
        if (csCommandEventHelper::GetCode(&ev) == cscmdFocusChanged)
        {
            bool shouldPause = !assistant->always_runs();
            OSXDelegate2D_focusChanged(delegate, 
	      csCommandEventHelper::GetInfo (&ev), shouldPause);
            handled = true;
        }
        if (csCommandEventHelper::GetCode(&ev) == cscmdCommandLineHelp)
        {
            csPrintf("Options for MacOS X 2D graphics drivers:\n"
             "  -screen=<num>      Screen number to display on (default=0)\n");
            handled = true;
        }
    }
    else if ((ev.Type == csevKeyboard) && 
            (csKeyEventHelper::GetEventType(&ev) == csKeyEventTypeDown))
    {
        if ((csKeyEventHelper::GetRawCode(&ev) == '\r') && 
            (csKeyEventHelper::GetModifiersBits(&ev) & CSMASK_ALT))
            handled = ToggleFullscreen();
    }

    return handled;
}


// DispatchEvent
// Dispatch an event to the assistant
void OSXDriver2D::DispatchEvent(OSXEvent ev, OSXView view)
{
    assistant->dispatch_event(ev, view);
}


// HideMouse
// Hide the mouse
void OSXDriver2D::HideMouse()
{
    assistant->hide_mouse_pointer();
}


// ShowMouse
// Show the mouse cursor
void OSXDriver2D::ShowMouse()
{
    assistant->show_mouse_pointer();
}


// Initialize16
// Initialize pixel format for 16 bit depth
void OSXDriver2D::Initialize16()
{
    canvas->pfmt.PalEntries = 0;
    canvas->pfmt.PixelBytes = 2;
    canvas->pfmt.RedMask = 0x1F << 10;
    canvas->pfmt.GreenMask = 0x1F << 5;
    canvas->pfmt.BlueMask = 0x1F;
    canvas->pfmt.AlphaMask = 0;
    canvas->pfmt.complete();
}


// Initialize32
// Initialize pixel format for 32 bit depth
void OSXDriver2D::Initialize32()
{
    canvas->pfmt.PalEntries = 0;
    canvas->pfmt.PixelBytes = 4;
    canvas->pfmt.RedMask = 0xFF0000;
    canvas->pfmt.GreenMask = 0x00FF00;
    canvas->pfmt.BlueMask = 0x0000FF;
    canvas->pfmt.AlphaMask = 0xFF000000;
    canvas->pfmt.complete();
}


// EnterFullscreenMode
// Switch to fullscreen mode - returns true on success
bool OSXDriver2D::EnterFullscreenMode()
{
    // Find mode and copy parameters
    CFDictionaryRef mode = CGDisplayBestModeForParameters(display, 
                        canvas->Depth, canvas->Width, canvas->Height, 0);
    if (mode == 0)
        return false;

    // Fade to black
    FadeToRGB(display, 0.0, 0.0, 0.0);

    // Lock displays
    if (CGDisplayCapture(display) != CGDisplayNoErr)
        return false;

    // Switch to new mode
    if (CGDisplaySwitchToMode(display, mode) == CGDisplayNoErr)
    {
        // Extract actual Width/Height/Depth
        CFNumberGetValue(
	    (CFNumberRef) CFDictionaryGetValue(mode, kCGDisplayWidth),
	    kCFNumberLongType, &canvas->Width);
        CFNumberGetValue(
	    (CFNumberRef) CFDictionaryGetValue(mode, kCGDisplayHeight),
	    kCFNumberLongType, &canvas->Height);
        CFNumberGetValue(
	    (CFNumberRef) CFDictionaryGetValue(mode, kCGDisplayBitsPerPixel), 
	    kCFNumberLongType, &canvas->Depth);
        
        // Fade back to original gamma
        FadeToGammaTable(display, originalGamma);
    }
    else
    {
        CGDisplayRelease(display);
        return false;
    }

    return true;
}


// ExitFullscreenMode
// Switch out of fullscreen mode, to mode stored in originalMode
void OSXDriver2D::ExitFullscreenMode()
{
    FadeToRGB(display, 0.0, 0.0, 0.0);
    CGDisplaySwitchToMode(display, originalMode);
    CGDisplayRelease(display);
    FadeToGammaTable(display, originalGamma);
}


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
    }

    // Restore original dimensions; force resize by forcing AllowResize to true
    canvas->FullScreen = !canvas->FullScreen;
    canvas->AllowResizing = true;
    canvas->Resize(origWidth, origHeight);
    canvas->AllowResizing = oldAllowResizing;

    if ((success == true) && (canvas->FullScreen == true))
    {
        inFullscreenMode = EnterFullscreenMode();
        success = inFullscreenMode;
    }

    if (success == true)
        OSXDelegate2D_openWindow(delegate, canvas->win_title,
                                canvas->Width, canvas->Height, canvas->Depth, 
                                canvas->FullScreen, display, screen);

    return success;
}


// FadeToRGB
// Uses CoreGraphics to fade to a given color 
void OSXDriver2D::FadeToRGB(CGDirectDisplayID disp, float r, float g, float b)
{
    int i;
    GammaTable gamma;
    
    for (i = 0; i < 256; i++)
    {
        gamma.r[i] = r;
        gamma.g[i] = g;
        gamma.b[i] = b;
    }
    
    FadeToGammaTable(disp, gamma);
}


// FadeToGamma
// Fade to a given gamma table
void OSXDriver2D::FadeToGammaTable(CGDirectDisplayID disp, GammaTable table)
{
    static const float TOTAL_USEC = 1000000.0;	// 1 second total
    
    int i;
    float x, start_usec, end_usec, current_usec;
    timeval start, current;
    GammaTable temp;
    
    gettimeofday(&start, 0);
    start_usec = (start.tv_sec * 1000000) + start.tv_usec;
    end_usec = start_usec + TOTAL_USEC;
    
    do {
        gettimeofday(&current, 0);
        current_usec = (current.tv_sec * 1000000) + current.tv_usec;
        
        // Calculate fraction of elapsed time
        x = (current_usec - start_usec) / TOTAL_USEC;        
        
        for (i = 0; i < 256; i++)
        {
            temp.r[i] = originalGamma.r[i] + 
                                (x * (table.r[i] - originalGamma.r[i])); 
            temp.g[i] = originalGamma.g[i] + 
                                (x * (table.g[i] - originalGamma.g[i])); 
            temp.b[i] = originalGamma.b[i] + 
                                (x * (table.b[i] - originalGamma.b[i]));
        }
        
        CGSetDisplayTransferByTable(disp, 256, temp.r, temp.g, temp.b);
    } while (current_usec < end_usec);

    CGSetDisplayTransferByTable(disp, 256, table.r, table.g, table.b);
}


// SaveGamma
// Save the current gamma values to the given table
void OSXDriver2D::SaveGamma(CGDirectDisplayID disp, GammaTable &table)
{
    CGDisplayErr err;
    CGTableCount sampleCount;
    
    err = CGGetDisplayTransferByTable(disp, 256, table.r, table.g, 
                                        table.b, &sampleCount);
    if (err != kCGErrorSuccess)
        csFPrintf(stderr, "Error %d reading gamma values\n", err);
}


// ChooseDisplay
// Choose which display to use
// Updates the screen and display members
void OSXDriver2D::ChooseDisplay()
{
    csRef<iCommandLineParser> parser = 
	CS_QUERY_REGISTRY(objectReg, iCommandLineParser);
    const char *s = parser->GetOption("screen");
    if (s != 0)
        screen = (unsigned int)atoi(s);
    else
    {
        csConfigAccess cfg(objectReg, "/config/video.cfg");
        screen = (unsigned int)cfg->GetInt("Video.ScreenNumber", 0);
    }

    // Get list of displays and get id of display to use if not default
    display = kCGDirectMainDisplay;
    if (screen != 0)
    {
        CGDisplayErr err;
        CGDisplayCount numDisplays;
	// Who is going to have more than 32 displays??
        CGDirectDisplayID displayList[32];
        
        err = CGGetActiveDisplayList(32, displayList, &numDisplays);
        if (err == CGDisplayNoErr)
        {
            if (screen < numDisplays)
                display = displayList[screen];
            else
                csReport(objectReg, CS_REPORTER_SEVERITY_WARNING, 
                        "crystalspace.canvas.osxdriver2d",
                        "WARNING: Requested screen %u but only %d are "
			"available - using main display\n", 
                        screen, numDisplays);
        }
        else
            csReport(objectReg, CS_REPORTER_SEVERITY_WARNING, 
                    "crystalspace.canvas.osxdriver2d",
                    "WARNING: Requested screen %u but unable to get screen "
		    "list - using main display\n", screen);
                    
        if (display == kCGDirectMainDisplay)
            screen = 0;
    }
}


/// C API to driver class
#define DRV2D_FUNC(ret, func) __private_extern__ "C" ret OSXDriver2D_##func

typedef void *OSXDriver2DHandle;
typedef void *OSXEventHandle;
typedef void *OSXViewHandle;


// C API to driver class
DRV2D_FUNC(void, DispatchEvent)(OSXDriver2DHandle driver, OSXEventHandle ev,
    OSXViewHandle view)
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
}

DRV2D_FUNC(void, ShowMouse)(OSXDriver2DHandle driver)
{
    ((OSXDriver2D *) driver)->ShowMouse();
}

#undef DRV2D_FUNC
