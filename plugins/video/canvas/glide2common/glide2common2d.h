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

#ifndef __GLIDECOMMON2D_H__
#define __GLIDECOMMON2D_H__

#include "csutil/scf.h"
#include "cs2d/common/graph2d.h"
#if defined(OS_BE)	// dh: is this OS-dependence necessary? 
#include "cssys/be/beitf.h"
#else
#include "cssys/unix/iunix.h"
#endif
#include "csutil/inifile.h"
//#include "cs2d/glide2common/gl2d_font.h"
#include "cs3d/glide2/gl_txtmgr.h"
#include "cs3d/glide2/glcache.h"
#include "iglide2d.h"

//struct iTextureHandle;
//class OpenGLTextureCache;

/** Basic OpenGL version of the 2D driver class
 *  You can look at the openGLX graphics class as an example
 *  of how to inherit and use this class.  In short,
 *  inherit from this common class instead of from csGraphics2D,
 *  and override all the functions you normally would except for
 *  the 2D drawing functions, which are supplied for you here.
 *  That way all OpenGL drawing functions are unified over platforms,
 *  so that a fix or improvement will be inherited by all platforms
 *  instead of percolating via people copying code over. -GJH
 */
class csGraphics2DGlideCommon : public csGraphics2D, public iGraphics2DGlide
{
protected:
  /// flag to indicate whether to draw fullscreen or not.
  bool m_DoGlideInWindow;
  bool m_bVRetrace;
  
  /// palette has been changed
  bool bPaletteChanged;
  bool bPalettized;
  int glDrawMode;
  bool GraphicsReady;
  GrLfbInfo_t lfbInfo;
    
  /// flag to prevent 2D drawing
  static bool locked;	
  /// my own private texture cache--for 2D sprites!
//  OpenGLTextureCache *texture_cache; //dh: not implemented yet

  /// hold the CS fonts in an OpenGL-friendly format
//  csGraphics2DOpenGLFontServer *LocalFontServer; //dh: not implemented yet

public:
  DECLARE_IBASE;

  /** constructor initializes System member.. LocalFontServer and
   *  texture_cache are initialized in Open() */
  csGraphics2DGlideCommon (iBase *iParent);

  /// Destructor deletes texture_cache and LocalFontServer
  virtual ~csGraphics2DGlideCommon ();

  /* You must supply all the functions not supplied here, such
   * as SetMouseCursor, ProcessEvents, etc. 
   * Note also that even though Initialize, Open, and Close are supplied here, 
   * you must still override these functions for your own
   * subclass to make system-specific calls for creating and showing
   * windows, etc. */

  /** Figure out draw functions...little else done here, most of
   *  the work is done in Open() */
  virtual bool Initialize (iSystem *pSystem);

  /** initialize fonts, texture cache, prints renderer name and version.
   *  you should still print out the 2D driver type (X, Win, etc.) in your
   *  subclass code.  Note that the Open() method makes some OpenGL calls,
   *  so your GL context should already be created and bound before
   *  you call csGraphics2DGlideCommon::Open()!  This means you
   *  may or may not be calling this method at the beginning of your own
   *  class Open(), since the GL context may not have been bound yet...
   * check the GLX class for an example */
  virtual bool Open (const char *Title);

  virtual void Close ();

  // the remaining functions here do not need to be overridden when
  // inheriting from this class

//  virtual void Clear(int color);
  virtual void SetRGB (int i, int r, int g, int b);

  //  Glide-specific procedures
  void SetTMUPalette(int tmu);
  /// Draw a line
  virtual void DrawLine( int x1, int y1, int x2, int y2, int color);
  /// Draw a pixel
  static void DrawPixelGlide (csGraphics2D *This, int x, int y, int color);
  /// Write a single character
  static void WriteCharGlide (csGraphics2D *This, int x, int y, int fg, int bg, char c);
  /// Draw a 2D sprite
  static void DrawSpriteGlide (csGraphics2D *This, iTextureHandle *hTex, int sx, int sy,
    int sw, int sh, int tx, int ty, int tw, int th);

  virtual bool BeginDraw();
  virtual void FinishDraw();
  /// Figure out GL RGB color from a packed color format
//  virtual void setGlideColorfromint(int color); //dh: not implemented yet

  /**
   * Get address of video RAM at given x,y coordinates.
   * The Glide version of this function just returns NULL.
   */
  static unsigned char* GetPixelAtGlide (csGraphics2D *This,int x, int y);

  virtual void Print( csRect* area );  
  // iGraphics2DGlide
  virtual void SetVRetrace( bool wait4vr ){ m_bVRetrace = wait4vr; }
};

#endif
