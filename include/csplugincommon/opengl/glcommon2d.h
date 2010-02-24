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

/**\file
 * Common OpenGL canvas superclass.
 */

#if defined(CS_OPENGL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENGL_PATH,gl.h)
#else
#include <GL/gl.h>
#endif

#include "csextern_gl.h"
#include "csutil/scf.h"
#include "csplugincommon/canvas/graph2d.h"
#include "csplugincommon/iopengl/driverdb.h"
#include "glfontcache.h"
#include "iutil/event.h"
#include "glstates.h"
#include "glextmanager.h"
#include "glss.h"
#include "driverdb.h"

class OpenGLTextureCache;
class GLFontCache;

/**\addtogroup plugincommon
 * @{ */
/**
 * Basic OpenGL version of the 2D driver class.
 * You can look at one of the OpenGL canvas classes as an example
 * of how to inherit and use this class.  In short,
 * inherit from this common class instead of from csGraphics2D,
 * and override all the functions you normally would except for
 * the 2D drawing functions, which are supplied for you here.
 * That way all OpenGL drawing functions are unified over platforms,
 * so that a fix or improvement will be inherited by all platforms
 * instead of percolating via people copying code over. 
 */
class CS_CSPLUGINCOMMON_GL_EXPORT csGraphics2DGLCommon : 
  public scfImplementationExt2<csGraphics2DGLCommon, 
	  csGraphics2D, 
	  iEventPlug,
	  iOpenGLDriverDatabase>
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
  
  class CS_CSPLUGINCOMMON_GL_EXPORT csGLPixelFormatPicker
  {
    csGraphics2DGLCommon* parent;

    /*
    size_t nextValueIndices[glpfvValueCount];
    csArray<int> values[glpfvValueCount];
    

    char* order;
    size_t orderPos;
    size_t orderNum;*/

    // Hold properties for a single pixelformat property
    struct PixelFormatPropertySet
    {
      GLPixelFormatValue valueType;
      size_t nextIndex;
      size_t firstIndex;
      csArray<int> possibleValues;
    };

    /* Pixel format properties, however this is _not_ indexed by 
    GLPixelFormatValue but sorted by order */
    PixelFormatPropertySet pixelFormats[glpfvValueCount];

    // Remapping table from real GLPixelFormatValue to index in table above
    size_t pixelFormatIndexTable[glpfvValueCount]; 

    GLPixelFormat currentValues;
    bool currentValid;

    void ReadStartValues ();
    void ReadPickerValues ();
    void ReadPickerValue (const char* valuesStr, csArray<int>& values);
    void SetInitialIndices ();
    void SetupIndexTable (const char* orderStr);
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
  csGLStateCacheContext *statecontext;

  bool hasRenderTarget;

  /// Decompose a color ID into r,g,b components
  void DecomposeColor (int iColor, GLubyte &oR, GLubyte &oG, GLubyte &oB, GLubyte &oA);
  /// Same but uses floating-point format
  void DecomposeColor (int iColor, float &oR, float &oG, float &oB, float &oA);
  /// Set up current GL RGB color from a packed color format
  void setGLColorfromint (int color);

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

  /// Open default driver database.
  void OpenDriverDB (const char* phase = 0);

  void Report (int severity, const char* msg, ...);
public:
  virtual const char* GetRendererString (const char* str);
  virtual const char* GetVersionString (const char* ver);
  
  virtual const char* GetHWRenderer();
  virtual const char* GetHWGLVersion();
  virtual const char* GetHWVendor();

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
  
  /// Query pixel R,G,B at given screen location
  virtual void GetPixel (int x, int y, uint8 &oR, uint8 &oG, uint8 &oB);
  /// As GetPixel() above, but with alpha
  virtual void GetPixel (int x, int y, uint8 &oR, uint8 &oG, uint8 &oB, uint8 &oA);

  /// Do a screenshot: return a new iImage object
  virtual csPtr<iImage> ScreenShot ();

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
  
  void SetViewport (int left, int top, int width, int height);

  /**\name iEventPlug implementation
   * @{ */
  virtual unsigned GetPotentiallyConflictingEvents ()
  { return CSEVTYPE_Keyboard | CSEVTYPE_Mouse; }
  virtual unsigned QueryEventPriority (unsigned /*iType*/)
  { return 150; }
  /** @} */

  /**\name iGLDriverDatabase implementation
   * @{ */
  void ReadDatabase (iDocumentNode* dbRoot, 
    int configPriority = iConfigManager::ConfigPriorityPlugin + 20,
    const char* phase = 0)
  {
    driverdb.Open (this, dbRoot, phase, configPriority);
  }
  /** @} */
};

/** @} */

#endif // __CS_GLCOMMON2D_H__
