/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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

#ifndef __CS_GLIDECOMMON2D_H__
#define __CS_GLIDECOMMON2D_H__

#include "csutil/scf.h"
#include "video/canvas/common/graph2d.h"
#include "csgeom/csrect.h"
#include "iglide2d.h"
#include <glide.h>

/** This is the base implemementation for the glide 2D driver.
 *  Platform specific implementations inherit from this one.
 */
class csGraphics2DGlideCommon : public csGraphics2D, public iGraphics2DGlide
{
protected:
  /// Flag to indicate whether to draw fullscreen or not.
  bool m_DoGlideInWindow;
  bool m_bVRetrace;

  /// Current and last last mouse position
  int mx, my;
  int mxold, myold;

  /// Do we render into backbuffer ?
  FxI32 m_drawbuffer;
    
  // Cursor related data
  UShort *cursorBmp, *mcBack;
  int nCursor; // how many cursors defined
  int nCurCursor; // current cursor
  int mcCols, mcRows; // pixel rows and cols per cursor
  csRect mouseRect, writtenArea;
  
  // Palette has been changed
  bool bPaletteChanged;
  bool bPalettized;
  int glDrawMode;
  bool GraphicsReady;
  GrLfbInfo_t lfbInfo;
      
  /// Flag to prevent 2D drawing
  static bool locked;	

  /// Encode internal color representation from RGB scheme.
  void EncodeRGB ( UShort& color, UByte r, UByte g, UByte b );
  /// Decode internal color representation to RGB scheme.
  void DecodeRGB ( UShort color, UByte& r, UByte& g, UByte& b );
  
  /// Prepare the cursor. Return number of cursors.
  int PrepareCursors (char **shapes);
  /// Draw cursors.
  void DrawCursor ();
  
public:
  SCF_DECLARE_IBASE;

  /** Constructor initializes System member.. LocalFontServer and
   *  texture_cache are initialized in Open() */
  csGraphics2DGlideCommon (iBase *iParent);

  /// Destructor deletes texture_cache and LocalFontServer
  virtual ~csGraphics2DGlideCommon ();

  /* You must supply all the functions not supplied here, such
   * as SetMouseCursor etc. 
   * Note also that even though Initialize, Open, and Close are supplied here, 
   * you must still override these functions for your own
   * subclass to make system-specific calls for creating and showing
   * windows, etc. */

  /** Figure out draw functions...little else done here, most of
   *  the work is done in Open() */
  virtual bool Initialize (iSystem *pSystem);

  /** Initialize fonts, texture cache, prints renderer name and version.
   *  you should still print out the 2D driver type (X, Win, etc.) in your
   *  subclass code.  Note that the Open() method makes some OpenGL calls,
   *  so your GL context should already be created and bound before
   *  you call csGraphics2DGlideCommon::Open()!  This means you
   *  may or may not be calling this method at the beginning of your own
   *  class Open(), since the GL context may not have been bound yet...
   *  check the GLX class for an example. */
  virtual bool Open (const char *Title);

  /// Close the canvas (graphics context).
  virtual void Close ();

  // The remaining functions here do not need to be overridden when
  // inheriting from this class

  virtual void SetRGB (int i, int r, int g, int b);

  /// Glide-specific procedures
  void SetTMUPalette(int tmu);
  /// Draw a line
  virtual void DrawLine( float x1, float y1, float x2, float y2, int color);
  /// Draw a pixel
  virtual void DrawPixel (int x, int y, int color);
  /// Write a single character
  static void WriteStringGlide (csGraphics2D *This, iFont *font, int x, int y,
    int fg, int bg, const char *text);

  /// Prepare for drawing.
  virtual bool BeginDraw();
  /// Finalize drawing.
  virtual void FinishDraw();

 /**
  * Save a subarea of screen and return a handle to saved buffer.
  * Storage is allocated in this call, you should either FreeArea()
  * the handle after usage or RestoreArea () it.
  */
  virtual csImageArea *SaveArea (int x, int y, int w, int h);
  /// Restore a subarea of screen saved with SaveArea()
  virtual void RestoreArea (csImageArea *Area, bool Free);

  /// Enable or disable double buffering.
  virtual bool DoubleBuffer (bool Enable);
  /// Get double buffering status.
  virtual bool GetDoubleBufferState ()
  { return m_drawbuffer == GR_BUFFER_BACKBUFFER; }

  /// Get address of video RAM at given x,y coordinates.
  unsigned char* GetPixelAt (int x, int y);

  /// Flush a rectangle from frame buffer to screen.
  virtual void Print( csRect* area );  
  /// Set vertical retrace interval.
  virtual void SetVRetrace( bool wait4vr ){ m_bVRetrace = wait4vr; }
  /// Force resolution
  virtual void ForceResolution ( int w, int h )
  { Width = w; Height = h; }
  /// Get Z-buffer value at coordinates.
  virtual float GetZBuffValue (int x, int y);
};

#endif // __CS_GLIDECOMMON2D_H__
