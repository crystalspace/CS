/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
    Written by Andrew Zabolotny <bit@eltech.ru>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_CSPLUGINCOMMON_CANVAS_GRAPH2D_H__
#define __CS_CSPLUGINCOMMON_CANVAS_GRAPH2D_H__

/**\file
 * Base class for 2D canvases.
 */

#include "csextern.h"

#include "csutil/cfgacc.h"
#include "csutil/scf.h"
#include "csutil/scf_implementation.h"
#include "csutil/weakref.h"

#include "iutil/comp.h"
#include "iutil/dbghelp.h"
#include "iutil/eventh.h"
#include "iutil/plugin.h"
#include "iutil/pluginconfig.h"
#include "iutil/string.h"
#include "ivideo/fontserv.h"
#include "ivideo/graph2d.h"
#include "ivideo/natwin.h"

/**
 * \addtogroup plugincommon
 * @{ */

struct iObjectRegistry;
struct iPluginManager;

class csFontCache;

#include "csutil/deprecated_warn_off.h"

/**
 * This is the base class for 2D canvases. Plugins should derive their 
 * own class from this one and implement required (marked with an 
 * asterisk (*)) functions.
 * Functions not marked with an asterisk are optional, but possibly
 * slow since they are too general.
 */
class CS_CRYSTALSPACE_EXPORT csGraphics2D : 
  public scfImplementation7<csGraphics2D, 
    iGraphics2D, iComponent, iNativeWindow, iNativeWindowManager,
    iPluginConfig, iDebugHelper, iEventHandler>
{
public:
  /// The configuration file.
  csConfigAccess config;

  /// The clipping rectangle.
  int ClipX1, ClipX2, ClipY1, ClipY2;

  /// Open/Close state.
  bool is_open;

  /// The object registry.
  iObjectRegistry* object_reg;
  /// The plugin manager.
  csWeakRef<iPluginManager> plugin_mgr;

  /// The font server
  csWeakRef<iFontServer> FontServer;
  /// The font cache
  csFontCache* fontCache;

  /// Pointer to a title.
  csString win_title;

  /// The width, height and depth of visual.
  int fbWidth, fbHeight, Depth;

  int vpLeft, vpTop, vpWidth, vpHeight;

  /**
   * Display number.  If 0, use primary display; else if greater than 0, use
   * that display number.  If that display number is not present, use primary
   * display.
   */
  int DisplayNumber;
  /// True if visual is full-screen.
  bool FullScreen;
  /// Whether to allow resizing.
  bool AllowResizing;
  /**
   * The counter that is incremented inside BeginDraw and decremented in
   * FinishDraw().
   */
  int FrameBufferLocked;
  /**
   * Change the depth of the canvas.
   */
  virtual void ChangeDepth (int d);
  /**
   * Get the name of this canvas
   */
  virtual const char *GetName() const;

  /// Hardware mouse cursor setting
  enum HWMouseMode
  {
    /// Never use hardware cursor
    hwmcOff,
    /// Always use hardware cursor, if possible
    hwmcOn,
    /// Only use hardware cursor if true RGBA cursor is available
    hwmcRGBAOnly
  };
  HWMouseMode hwMouse;
protected:
  /// Screen refresh rate
  int refreshRate;
  /// Activate Vsync
  bool vsync;
  /// Reduce window size to fit into workspace, if necessary
  bool fitToWorkingArea;

  csString name;
  csRef<iEventHandler> weakEventHandler;

  /**
   * Helper function for FitSizeToWorkingArea(): obtain workspace dimensions.
   */
  virtual bool GetWorkspaceDimensions (int& width, int& height);
  /**
   * Helper function for FitSizeToWorkingArea(): compute window dimensions
   * with the window frame included.
   */
  virtual bool AddWindowFrameDimensions (int& width, int& height);
public:
  /// Create csGraphics2D object
  csGraphics2D (iBase*);

  /// Destroy csGraphics2D object
  virtual ~csGraphics2D ();

  /// Initialize the plugin
  virtual bool Initialize (iObjectRegistry*);
  /// Event handler for plugin.
  virtual bool HandleEvent (iEvent&);

  /// (*) Open graphics system (set videomode, open window etc)
  virtual bool Open ();
  /// (*) Close graphics system
  virtual void Close ();

  /// Set clipping rectangle
  virtual void SetClipRect (int xmin, int ymin, int xmax, int ymax);
  /// Query clipping rectangle
  virtual void GetClipRect (int &xmin, int &ymin, int &xmax, int &ymax);

  /**
   * This routine should be called before any draw operations.
   * It should return true if graphics context is ready.
   */
  virtual bool BeginDraw ();
  /// This routine should be called when you finished drawing
  virtual void FinishDraw ();

  /// (*) Flip video pages (or dump backbuffer into framebuffer).
  virtual void Print (csRect const* /*area*/ = 0) { }

  /// Clear backbuffer
  virtual void Clear (int color);
  /// Clear all video pages
  virtual void ClearAll (int color);

  virtual int FindRGB (int r, int g, int b, int a = 255)
  {
    if (r < 0) r = 0; else if (r > 255) r = 255;
    if (g < 0) g = 0; else if (g > 255) g = 255;
    if (b < 0) b = 0; else if (b > 255) b = 255;
    if (a < 0) a = 0; else if (a > 255) a = 255;
    return ((255 - a) << 24) | (r << 16) | (g << 8) | b;
    /* Alpha is "inverted" so '-1' can be decomposed to a 
       transparent color. (But alpha not be inverted, '-1'
       would be "opaque white". However, -1 is the color
       index for "transparent text background". */
  }
  virtual void GetRGB (int color, int& r, int& g, int& b)
  {
    r = (color >> 16) & 0xff;
    g = (color >> 8) & 0xff;
    b = color & 0xff;
  }
  virtual void GetRGB (int color, int& r, int& g, int& b, int& a)
  {
    a = 255 - (color >> 24);
    GetRGB (color, r, g, b);
  }
  
  //@{
  /// Write a text string into the back buffer
  virtual void Write (iFont *font , int x, int y, int fg, int bg,
    const char *text, uint flags = 0);
  virtual void Write (iFont *font , int x, int y, int fg, int bg,
    const wchar_t* text, uint flags = 0);  
  //@}

  virtual bool SetGamma (float /*gamma*/) { return false; }
  virtual float GetGamma () const { return 1.0; }

private:
    /// helper function for ClipLine()
  bool CLIPt (float denom, float num, float& tE, float& tL);
public:

  /**
   * Clip a line against given rectangle
   * Function returns true if line is not visible
   */
  virtual bool ClipLine (float &x1, float &y1, float &x2, float &y2,
    int xmin, int ymin, int xmax, int ymax);

  /// Gets the font server
  virtual iFontServer *GetFontServer ()
  { return FontServer; }

  virtual int GetWidth () { return vpWidth; }
  virtual int GetHeight () { return vpHeight; }
  int GetColorDepth () { return Depth; }

  /**
   * Perform a system specific extension. Return false if extension
   * not supported.
   */
  virtual bool PerformExtension (char const* command, ...);

  /**
   * Perform a system specific extension. Return false if extension
   * not supported.
   */
  virtual bool PerformExtensionV (char const* command, va_list);

  /// Enable/disable canvas resize (Over-ride in sub classes)
  virtual void AllowResize (bool /*iAllow*/) { };

  /// Resize the canvas
  virtual bool Resize (int w, int h);

  /// Return the Native Window interface for this canvas (if it has one)
  virtual iNativeWindow* GetNativeWindow ();

  /// Returns 'true' if the program is being run full-screen.
  virtual bool GetFullScreen ()
  { return FullScreen; }

  /**
   * Change the fullscreen state of the canvas.
   */
  virtual void SetFullScreen (bool b);

  /// Set mouse cursor position; return success status
  virtual bool SetMousePosition (int x, int y);

  /**
   * Set mouse cursor to one of predefined shape classes
   * (see csmcXXX enum above). If a specific mouse cursor shape
   * is not supported, return 'false'; otherwise return 'true'.
   * If system supports it and iBitmap != 0, shape should be
   * set to the bitmap passed as second argument; otherwise cursor
   * should be set to its nearest system equivalent depending on
   * iShape argument.
   */
  virtual bool SetMouseCursor (csMouseCursorID iShape);

  /**
   * Set mouse cursor using an image.  If the operation is unsupported, 
   * return 'false' otherwise return 'true'.
   * On some platforms there is only monochrome pointers available.  In this
   * all black colors in the image will become the value of 'bg' and all 
   * non-black colors will become 'fg'
   */
  virtual bool SetMouseCursor (iImage *image, const csRGBcolor* keycolor = 0, 
                               int hotspot_x = 0, int hotspot_y = 0,
                               csRGBcolor fg = csRGBcolor(255,255,255),
                               csRGBcolor bg = csRGBcolor(0,0,0));
  
  void SetViewport (int left, int top, int width, int height);
  void GetViewport (int& left, int& top, int& width, int& height)
  { left = vpLeft; top = vpTop; width = vpWidth; height = vpHeight; }
  
  void GetFramebufferDimensions (int& width, int& height)
  { width = fbWidth; height = fbHeight; }
  
  const char* GetHWRenderer ()
  { return 0; }
  const char* GetHWGLVersion ()
  { return 0; }
  const char* GetHWVendor ()
  { return 0; }

  CS_EVENTHANDLER_NAMES("crystalspace.graphics2d.common")
  CS_EVENTHANDLER_NIL_CONSTRAINTS

protected:
  /**\name iNativeWindowManager implementation
   * @{ */
  // Virtual Alert function so it can be overridden by subclasses
  // of csGraphics2D.
  virtual void AlertV (int type, const char* title, const char* okMsg,
    const char* msg, va_list args);
  virtual void Alert (int type, const char* title, const char* okMsg,
      const char* msg, ...);
  virtual void AlertV (int type, const wchar_t* title, const wchar_t* okMsg,
    const wchar_t* msg, va_list args);
  virtual void Alert (int type, const wchar_t* title, const wchar_t* okMsg,
      const wchar_t* msg, ...);
  /** @} */

  /**\name iNativeWindow implementation
   * @{ */
  // Virtual SetTitle function so it can be overridden by subclasses
  // of csGraphics2D.
  virtual void SetTitle (const char* title);
  virtual void SetTitle (const wchar_t* title)
  { SetTitle (csString (title)); }

  /** Sets the icon of this window with the provided one.
   *  
   *  @note Virtual SetIcon function so it can be overridden by subclasses of csGraphics2D.
   *  @param image the iImage to set as the icon of this window.
   */  
  virtual void SetIcon (iImage *image);

  virtual bool IsWindowTransparencyAvailable() { return false; }
  virtual bool SetWindowTransparent (bool transparent) { return false; }
  virtual bool GetWindowTransparent () { return false; }

  virtual bool SetWindowDecoration (WindowDecoration decoration, bool flag)
  { return false; }
  virtual bool GetWindowDecoration (WindowDecoration decoration);

  virtual bool FitSizeToWorkingArea (int& desiredWidth,
                                     int& desiredHeight);
  /** @} */

  /**\name iPluginConfig implementation
   * @{ */
  virtual bool GetOptionDescription (int idx, csOptionDescription*);
  virtual bool SetOption (int id, csVariant* value);
  virtual bool GetOption (int id, csVariant* value);
  /** @} */

  /**\name iDebugHelper implementation
   * @{ */
  virtual bool DebugCommand (const char* cmd);
  virtual int GetSupportedTests () const { return 0; }
  virtual csPtr<iString> UnitTest () { return 0; }
  virtual csPtr<iString> StateTest ()  { return 0; }
  virtual csTicks Benchmark (int /*num_iterations*/) { return 0; }
  virtual csPtr<iString> Dump ()  { return 0; }
  virtual void Dump (iGraphics3D* /*g3d*/)  { }
  /** @} */
};

#include "csutil/deprecated_warn_on.h"

/** @} */

#endif // __CS_CSPLUGINCOMMON_CANVAS_GRAPH2D_H__
