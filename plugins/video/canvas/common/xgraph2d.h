/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Ported to COM by Dan Ogles

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

#include "igraph2d.h"
#include "itexture.h"

#ifndef __XGRAPH2D_H__
#define __XGRAPH2D_H__

/**
 *  This serves as the composite interface that implements IGraphics2D
 *  for csGraphics2D and all derived classes.
 */
interface IXGraphics2D : public IGraphics2D
{
  friend class csGraphics2D;

  DECLARE_IUNKNOWN()

  ///
  STDMETHODIMP Initialize ();

 /**
  * This routine should be called before any draw operations.
  * It should return S_OK if graphics context is ready.
  */
  STDMETHODIMP BeginDraw ();
  /// This routine should be called when you finished drawing.
  STDMETHODIMP FinishDraw ();

  /// (*) Flip video pages (or dump backbuffer into framebuffer).
  STDMETHODIMP Print (csRect* pArea);

  /// Clear backbuffer.
  STDMETHODIMP Clear (int color);
  /// Clear all video pages.
  STDMETHODIMP ClearAll (int color);

  /// Draw a line.
  STDMETHODIMP DrawLine (float x1, float y1, float x2, float y2, int color);
  /// Draw a box
  STDMETHODIMP DrawBox (int x, int y, int w, int h, int color);
  /// Draw a pixel.
  STDMETHODIMP DrawPixel (int x, int y, int color);
  /// Returns the address of the pixel at the specified (x, y) coordinates.
  STDMETHODIMP GetPixelAt (int x, int y, unsigned char** pPixel );
  /// Draw a sprite using a rectangle from given texture
  STDMETHODIMP DrawSprite (ITextureHandle *hTex, int sx, int sy, int sw, int sh, int tx, int ty, int tw, int th);

  /// Enable or disable double buffering; returns S_OK or S_FALSE.
  STDMETHODIMP DoubleBuffer (bool Enable);
  /// Get the double buffer state
  STDMETHODIMP GetDoubleBufferState (bool& State);

 /**
  * Save a subarea of screen area into the variable Data.
  * Storage is allocated in this call, you should either FreeArea()
  * it after usage or RestoreArea() it.
  */
  STDMETHODIMP SaveArea (ImageArea** Area, int x, int y, int w, int h);
  /// Restore a subarea of screen saved with SaveArea()
  STDMETHODIMP RestoreArea (ImageArea* Area, bool bFree);
  /// Free storage allocated for a subarea of screen
  STDMETHODIMP FreeArea (ImageArea* Area);

  /// Retrieve clipping rectangle
  STDMETHODIMP GetClipRect (int& nMinX, int& nMinY, int& nMaxX, int& nMaxY);
  /// Set clipping rectangle
  STDMETHODIMP SetClipRect (int nMinX, int nMinY, int nMaxX, int nMaxY);

  /// Set cursor position.
  STDMETHODIMP SetMousePosition (int x, int y);

 /**
  * Set mouse cursor to one of predefined shape classes
  * (see csmcXXX enum above). If a specific mouse cursor shape
  * is not supported, return 'false'; otherwise return 'true'.
  * If system supports it and iBitmap != NULL, shape should be
  * set to the bitmap passed as second argument; otherwise cursor
  * should be set to its nearest system equivalent depending on
  * iShape argument.
  */
  STDMETHODIMP SetMouseCursor (csMouseCursorID Shape, ITextureHandle *hBitmap);
  /// (*) Set a color index to given R,G,B (0..255) values
  STDMETHODIMP SetRGB (int i, int r, int g, int b);
  /// Write a text string into the back buffer
  STDMETHODIMP Write (int x, int y, int fg, int bg, const char* str);
  /// Write a single character.
  STDMETHODIMP WriteChar (int x, int y, int fg, int bg, char c);

  /// Gets the type of the font.
  STDMETHODIMP GetFontID (int& id);
  /// Sets the type of the font.
  STDMETHODIMP SetFontID (int id);

  /// Get the string equivilent of the HRESULT.
  STDMETHODIMP GetStringError( HRESULT hRes, char* szErrorString );

  /// Open the device.
  STDMETHODIMP Open (const char *Title);

 /**
  * Clip a line against given rectangle.
  * Function returns S_OK if line is not visible.
  */
  STDMETHODIMP ClipLine (float &x1, float &y1, float &x2, float &y2, 
    int xmin, int ymin, int xmax, int ymax);

  /// Close down the device.
  STDMETHODIMP Close ();

  /**
   * Perform a system specific exension.
   * This is probably not the good way to do this but I see no other
   * way currently.
   */
  STDMETHODIMP PerformExtension (char* args);
};

/**
 *  This serves as the composite interface that implements IGraphicsInfo
 *  for csGraphics2D and all derived classes.
 */
interface IXGraphicsInfo : public IGraphicsInfo
{
  DECLARE_IUNKNOWN()

  /// Get active videopage number (starting from zero)
  STDMETHODIMP GetPage(int& nPage);

 /**
  * Return the number of bytes for every pixel.
  * This function is equivalent to the PixelBytes field that
  * you get from GetPixelFormat.
  */
  STDMETHODIMP GetPixelBytes(int& nPixelBytes);

 /**
  * Return information about about the pixel format.
  */
  STDMETHODIMP GetPixelFormat(csPixelFormat* PixelFormat);

  /// Get the current font ID
  STDMETHODIMP GetFontID(int& nID);

  /// Return the width of the framebuffer.
  STDMETHODIMP GetWidth(int& nWidth);
  /// Return the height of the framebuffer.
  STDMETHODIMP GetHeight(int& nHeight);
  /// Returns 'true' if the program is being run full-screen.
  STDMETHODIMP GetIsFullScreen(bool& bIsFullScreen);

 /**
  * Return the number of palette entries that can be modified.
  * This should return 0 if there is no palette (true color displays).
  * This function is equivalent to the PalEntries field that you
  * get from GetPixelFormat. It is just a little bit easier to obtain
  * this way.
  */
  STDMETHODIMP GetNumPalEntries(int& nEntries);

  /// Get the string equivilent of the HRESULT.
  STDMETHODIMP GetStringError( HRESULT hRes, char* szErrorString );

  /// Get the palette (if any)
  STDMETHODIMP GetPalette(RGBpaletteEntry**);

  /// Get the width of a string if it would be drawn with given font
  STDMETHODIMP GetTextWidth (int &Width, int Font, const char *text);
  /// Get the height of given font
  STDMETHODIMP GetTextHeight (int &Height, int Font);
};


#endif
