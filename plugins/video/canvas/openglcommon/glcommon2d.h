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

#ifndef __GLCOMMON2D_H__
#define __GLCOMMON2D_H__

#include "csutil/scf.h"
#include "video/canvas/common/graph2d.h"
#include "gl2d_font.h"

class OpenGLTextureCache;
#define CsPrintf System->Printf
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
class csGraphics2DGLCommon : public csGraphics2D
{
  /// hold the CS fonts in an OpenGL-friendly format
  csGraphics2DOpenGLFontServer *LocalFontServer;

public:
  DECLARE_IBASE;

  bool is_double_buffered;
  /// Constructor does little, most initialization stuff happens in Initialize()
  csGraphics2DGLCommon (iBase *iParent);

  /// Destructor deletes LocalFontServer
  virtual ~csGraphics2DGLCommon ();

  /* You must supply all the functions not supplied here, such
   * as SetMouseCursor etc.
   * Note also that even though Initialize, Open, and Close are supplied here, 
   * you must still override these functions for your own
   * subclass to make system-specific calls for creating and showing
   * windows, etc. */

  /// Initialize the plugin
  virtual bool Initialize (iSystem *pSystem);

  /** initialize fonts, texture cache, prints renderer name and version.
   *  you should still print out the 2D driver type (X, Win, etc.) in your
   *  subclass code.  Note that the Open() method makes some OpenGL calls,
   *  so your GL context should already be created and bound before
   *  you call csGraphics2DGLCommon::Open()!  This means you
   *  may or may not be calling this method at the beginning of your own
   *  class Open(), since the GL context may not have been bound yet...
   * check the GLX class for an example */
  virtual bool Open (const char *Title);

  virtual void Close ();

  // the remaining functions here do not need to be overridden when
  // inheriting from this class

  virtual void Clear(int color);
  virtual void SetRGB (int i, int r, int g, int b);

  /// Draw a line
  virtual void DrawLine (float x1, float y1, float x2, float y2, int color);
  /// Draw a box
  virtual void DrawBox (int x, int y, int w, int h, int color);
  /// Draw a pixel
  virtual void DrawPixel (int x, int y, int color);
  /// Write a single character
  virtual void WriteChar (int x, int y, int fg, int bg, char c);

  /// Figure out GL RGB color from a packed color format
  void setGLColorfromint(int color);

  /**
   * Get address of video RAM at given x,y coordinates.
   * The OpenGL version of this function just returns NULL
   * if not doing a screenshot.
   */
  virtual unsigned char* GetPixelAt (int x, int y);

  /// Do a screenshot: return a new iImage object
  virtual iImage *ScreenShot ();

  /**
   * Save a subarea of screen area into the variable Data.
   * Storage is allocated in this call, you should either FreeArea()
   * it after usage or RestoreArea() it.
   */
  virtual csImageArea *SaveArea (int x, int y, int w, int h);
  /// Restore a subarea of screen saved with SaveArea()
  virtual void RestoreArea (csImageArea *Area, bool Free = true);
  /// Get the double buffer state
  virtual bool GetDoubleBufferState () { return is_double_buffered; }
  /// Enable or disable double buffering; returns success status
  virtual bool DoubleBuffer (bool Enable) 
  { is_double_buffered = Enable; return true; }
};

#endif
