/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#ifndef __CS_GLCOMMON2D_H__
#define __CS_GLCOMMON2D_H__

#if defined(CS_OPENGL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENGL_PATH,gl.h)
#else
#include <GL/gl.h>
#endif

#include "csextern.h"
#include "csutil/scf.h"
#include "csplugincommon/canvas/graph2d.h"
#include "glfontcache.h"
#include "iutil/event.h"
#include "glstates.h"
#include "glextmanager.h"
#include "glss.h"
#include "driverdb.h"

class OpenGLTextureCache;
class GLFontCache;

/**
 * Basic OpenGL version of the 2D driver class
 * You can look at the openGLX graphics class as an example
 * of how to inherit and use this class.  In short,
 * inherit from this common class instead of from csGraphics2D,
 * and override all the functions you normally would except for
 * the 2D drawing functions, which are supplied for you here.
 * That way all OpenGL drawing functions are unified over platforms,
 * so that a fix or improvement will be inherited by all platforms
 * instead of percolating via people copying code over. -GJH
 */
class CS_CSPLUGINCOMMON_OGL_EXPORT csGraphics2DGLCommon : public csGraphics2D, 
							   public iEventPlug
{
public:
  enum GLPixelFormatValue
  {
    glpfvColorBits = 0,
    glpfvAlphaBits,
    glpfvDepthBits,
    glpfvStencilBits,
    glpfvAccumColorBits,
    glpfvAccumAlphaBits,
    glpfvMultiSamples,

    glpfvValueCount
  };
  typedef int GLPixelFormat[glpfvValueCount];
protected:
  friend class csGLScreenShot;
  friend class csGLFontCache;
  
  class CS_CSPLUGINCOMMON_OGL_EXPORT csGLPixelFormatPicker
  {
    csGraphics2DGLCommon* parent;

    GLPixelFormat currentValues;
    size_t nextValueIndices[glpfvValueCount];
    csArray<int> values[glpfvValueCount];
    bool currentValid;

    char* order;
    size_t orderPos;
    size_t orderNum;

    void ReadStartValues ();
    void ReadPickerValues ();
    void ReadPickerValue (const char* valuesStr, csArray<int>& values);
    void SetInitialIndices ();
    bool PickNextFormat ();
  public:
    csGLPixelFormatPicker (csGraphics2DGLCommon* parent);
    ~csGLPixelFormatPicker ();

    void Reset();
    bool GetNextFormat (GLPixelFormat& format);
  };
  friend class csGLPixelFormatPicker;

  /// Cache for GL states
  csGLStateCache* statecache;
  
  bool hasRenderTarget;

  /// Decompose a color ID into r,g,b components
  void DecomposeColor (int iColor, GLubyte &oR, GLubyte &oG, GLubyte &oB, GLubyte &oA);
  /// Same but uses floating-point format
  void DecomposeColor (int iColor, float &oR, float &oG, float &oB, float &oA);
  /// Set up current GL RGB color from a packed color format
  void setGLColorfromint (int color);

  uint8 *screen_shot;

  csGLScreenShot* ssPool;

  csGLScreenShot* GetScreenShot ();
  void RecycleScreenShot (csGLScreenShot* shot);

  /// Extension manager
  csGLExtensionManager ext;
  /// Multisample samples
  //int multiSamples;
  /// Whether to favor quality or speed.
  bool multiFavorQuality;
  /// Depth buffer resolution
  //int depthBits;
  GLPixelFormat currentFormat;
  /// Driver database
  csGLDriverDatabase driverdb;
  bool useCombineTE;

  void GetPixelFormatString (const GLPixelFormat& format, csString& str);
public:
  virtual const char* GetRendererString (const char* str);
  virtual const char* GetVersionString (const char* ver);

  SCF_DECLARE_IBASE_EXT(csGraphics2D);

  /// The event plug object
  csRef<iEventOutlet> EventOutlet;

  /**
   * Constructor does little, most initialization stuff happens in
   * Initialize().
   */
  csGraphics2DGLCommon (iBase *iParent);

  /// Clear font cache etc.
  virtual ~csGraphics2DGLCommon ();

  /*
   * You must supply all the functions not supplied here, such as
   * SetMouseCursor etc. Note also that even though Initialize, Open,
   * and Close are supplied here, you must still override these functions
   * for your own subclass to make system-specific calls for creating and
   * showing windows, etc.
   */

  /// Initialize the plugin
  virtual bool Initialize (iObjectRegistry *object_reg);

  /**
   * Initialize font cache, texture cache, prints renderer name and version.
   * you should still print out the 2D driver type (X, Win, etc.) in your
   * subclass code.
   */
  virtual bool Open ();

  virtual void Close ();

  virtual void SetClipRect (int xmin, int ymin, int xmax, int ymax);

  /**
   * This routine should be called before any draw operations.
   * It should return true if graphics context is ready.
   */
  virtual bool BeginDraw ();
  /// This routine should be called when you finished drawing
  virtual void FinishDraw ();

  /// Resize the canvas
  virtual bool Resize (int width, int height);


  /*
   * the remaining functions here do not need to be overridden when
   * inheriting from this class
   */

  /// Clear the screen with color
  virtual void Clear (int color);

  /// Set a palette entry
  virtual void SetRGB (int i, int r, int g, int b);
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

  /// Draw a line
  virtual void DrawLine (float x1, float y1, float x2, float y2, int color);
  /// Draw a box
  virtual void DrawBox (int x, int y, int w, int h, int color);
  /// Draw a pixel
  virtual void DrawPixel (int x, int y, int color);
  /// Draw a series of pixels.
  virtual void DrawPixels (csPixelCoord const* pixels, int num_pixels,
    int color);
  /// Blit.
  virtual void Blit (int x, int y, int w, int h, unsigned char const* data);

  /**
   * Get address of video RAM at given x,y coordinates.
   * The OpenGL version of this function just returns 0
   * if not doing a screenshot.
   */
  virtual unsigned char *GetPixelAt (int x, int y);

  /// Do a screenshot: return a new iImage object
  virtual csPtr<iImage> ScreenShot ();

  /**
   * Save a subarea of screen area into the variable Data.
   * Storage is allocated in this call, you should either FreeArea()
   * it after usage or RestoreArea() it.
   */
  virtual csImageArea *SaveArea (int x, int y, int w, int h);
  /// Restore a subarea of screen saved with SaveArea()
  virtual void RestoreArea (csImageArea *Area, bool Free = true);

  /// Get the double buffer state
  virtual bool GetDoubleBufferState ()
  { return false; }
  /// Enable or disable double buffering; returns success status
  virtual bool DoubleBuffer (bool Enable)
  { return !Enable; }

  /// Perform extension commands
  virtual bool PerformExtensionV (char const* command, va_list);

  /// Execute a debug command.
  virtual bool DebugCommand (const char* cmd);

  //------------------------ iEventPlug interface ---------------------------//

  virtual unsigned GetPotentiallyConflictingEvents ()
  { return CSEVTYPE_Keyboard | CSEVTYPE_Mouse; }
  virtual unsigned QueryEventPriority (unsigned /*iType*/)
  { return 150; }
};

#endif // __CS_GLCOMMON2D_H__
