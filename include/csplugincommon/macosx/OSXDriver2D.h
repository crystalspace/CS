//
//  OSXDriver2D.h
//
//  Created by mreda on Wed Oct 31 2001.
//  Copyright (c) 2001 Matt Reda. All rights reserved.
//

// This code must be callable from the ObjC delegate.  Since that uses the
// standard C compiler, it doesn't like C++ classes, so we create a C API to
// some functions of this object

#ifndef __CS_OSXDRIVER2D_H__
#define __CS_OSXDRIVER2D_H__

#if defined(__cplusplus)

#include "csextern_osx.h"
#include "OSXDelegate2D.h"

#include "csgeom/csrect.h"
#include "csutil/macosx/OSXAssistant.h"
#include "iutil/eventh.h"
#include "ivideo/graph2d.h"
#include "csplugincommon/canvas/graph2d.h"

#include <CoreFoundation/CoreFoundation.h>
#include <ApplicationServices/ApplicationServices.h>


/// Table for storing gamma values
struct GammaTable
{
  float r[256];
  float g[256];
  float b[256];
};


CS_CSPLUGINCOMMON_OSX_EXPORT
class OSXDriver2D
{
public:
  /// Constructor
  OSXDriver2D(csGraphics2D *inCanvas);

  /// Destructor
  virtual ~OSXDriver2D();

  /// Initialize 2D plugin
  virtual bool Initialize(iObjectRegistry *reg);

  /// Open graphics system (set mode, open window, etc)
  virtual bool Open();

  /// Close graphics system
  virtual void Close();

  /// Flip video page (or dump to framebuffer) - pure virtual
  virtual void Print(csRect const* area = 0) = 0;

  /// Pure virtual function - the driver must invlude code to handle resizing
  virtual bool Resize(int w, int h) = 0;

  /// Handle an event
  virtual bool HandleEvent(iEvent &ev);

  /// Dispatch an event to the assistant
  void DispatchEvent(OSXEvent ev, OSXView view);

  /// Show/Hide the mouse
  virtual void HideMouse();
  virtual void ShowMouse();

  /// Event handler
  struct EventHandler : public iEventHandler
  {
  private:
    OSXDriver2D *parent;
  public:
    SCF_DECLARE_IBASE;
    EventHandler(OSXDriver2D *p)
    {
      SCF_CONSTRUCT_IBASE(0);
      parent = p;
    };
    virtual ~EventHandler()
    {
      SCF_DESTRUCT_IBASE();
    };
    virtual bool HandleEvent (iEvent& e) { return parent->HandleEvent(e); }
  } *scfiEventHandler;

protected:
  /// Initialize pixel format for 16 bit depth
  void Initialize16();

  /// Initialize pixel format for 32 bit depth
  void Initialize32();

  /// Switch to fullscreen mode
  bool EnterFullscreenMode();

  /// Switch out of fullscreen mode, to mode stored in originalMode
  void ExitFullscreenMode();

  /// Toggle current state of fullscreen
  virtual bool ToggleFullscreen();

  /// Uses CoreGraphics to fade to a given color 
  void FadeToRGB(CGDirectDisplayID disp, float r, float g, float b);
  
  /// Fade to a given gamma table
  void FadeToGammaTable(CGDirectDisplayID disp, GammaTable table);
  
  /// Save the current gamma values to the given table
  void SaveGamma(CGDirectDisplayID disp, GammaTable &table);

  /// Choose which display to use
  void ChooseDisplay();

  /// Original display mode
  CFDictionaryRef originalMode;	
  /// Original gamma values
  GammaTable originalGamma;
  /// In full-screen mode
  bool inFullscreenMode;
  /// Screen to display on
  CGDirectDisplayID display;
  /// Screen number to display on
  unsigned int screen;
  
  /// Original dimensions. jept so they can be restored when switching modes
  int origWidth, origHeight;

  /// Delegate for ObjC stuff
  OSXDelegate2D delegate;
  /// Canvas (parent class)
  csGraphics2D *canvas;

  /// Assistant for dispatching events
  csRef<iOSXAssistant> assistant;
  /// Object registry
  iObjectRegistry *objectReg;
};

#else // __cplusplus

#define DRV2D_FUNC(ret, func) __private_extern__ ret OSXDriver2D_##func

typedef void *OSXDriver2D;
typedef void *OSXEventHandle;
typedef void *OSXViewHandle;

// C API to driver class
DRV2D_FUNC(void, DispatchEvent)(OSXDriver2D driver, OSXEventHandle ev,
  OSXViewHandle view);
DRV2D_FUNC(bool, Resize)(OSXDriver2D driver, int w, int h);
DRV2D_FUNC(void, HideMouse)(OSXDriver2D driver);
DRV2D_FUNC(void, ShowMouse)(OSXDriver2D driver);

#undef DRV2D_FUNC

#endif // __cplusplus

#endif // __CS_OSXDRIVER2D_H__
