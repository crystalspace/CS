/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Andrew Zabolotny <bit@eltech.ru>
    Ported to COM by Dan Ogles <dogles@peachtree.com>

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

#ifndef __IGRAPH2D_H__
#define __IGRAPH2D_H__

#include "cscom/com.h"
#include "itexture.h"

class csRect;
interface ISystem;

enum FontType
{
  csFontParent = -1,
  csFontPolice,
  csFontPoliceFixed,
  csFontItalic,
  csFontItalicFixed,
  csFontCourier,
  csFontCourierFixed,
  csFontTiny,
  csFontTinyFixed
};

/// This structure is used for saving/restoring areas of screen
struct ImageArea
{
  int x, y, w, h;
  char *data;

  // @@@ not sure if this has binary compatibility in all cases - DAN
  inline ImageArea (int sx, int sy, int sw, int sh)
  { x = sx; y = sy; w = sw; h = sh; data = NULL; }
};

/// A RGB palette entry
struct RGBpaletteEntry
{
  UByte red;
  UByte green;
  UByte blue;
  UByte flags;
};

/**
 * Standard mouse cursor IDs
 */
enum csMouseCursorID
{
  /// No cursor
  csmcNone = -1,
  /// Arrow cursor
  csmcArrow = 0,
  /// Lens cursor
  csmcLens,
  /// Cross-hatch cursor
  csmcCross,
  /// Pen cursor
  csmcPen,
  /// Window move cursor
  csmcMove,
  /// Diagonal (\) resizing cursor
  csmcSizeNWSE,
  /// Diagonal (/) resizing cursor
  csmcSizeNESW,
  /// Vertical sizing cursor
  csmcSizeNS,
  /// Horizontal sizing cursor
  csmcSizeEW,
  /// Invalid operation cursor
  csmcStop,
  /// Wait (longplay operation) cursor
  csmcWait
};

/**
 * Structure describing the pixel format.
 */
struct csPixelFormat
{
  /**
   * The masks to extract the color information from a pixel (truecolor mode only).
   * Currently only masks for 16-bit/15-bit colors are supported.
   * Ignore the Mask and Shift fields of this structure if PalEntries != 0.
   */
  long RedMask, GreenMask, BlueMask;
  /// The shifts to extract the color information from a pixel (truecolor mode only).
  int RedShift, GreenShift, BlueShift;
  /// The number of significant bits for every color.
  int RedBits, GreenBits, BlueBits;

  /**
   * Number of palette entries. 0 for truecolor, else the number of palette
   * entries (this number should be equal to 256 even if not all of these 256
   * colors can be modified (like is the case in Windows)).
   * Currently only 0 and 256 are supported here.
   */
  int PalEntries;

  /**
   * Number of bytes for every pixel.
   * The only supported values currently are:<p>
   * <ul>
   * <li>1: for palette mode (256 palette entries)
   * <li>2: for truecolor 15/16-bit mode (no palette entries)
   * <li>4: for truecolor 32-bit mode (no palette entries)
   * </ul>
   */
  int PixelBytes;
};

extern const IID IID_IGraphics2D;
extern const IID IID_IGraphicsInfo;

/**
 * This is the interface for 2D renderer. System-dependent ports
 * should derive their own class from the standard implementation,
 * csGraphics2D, and implement required (marked with an asterisk (*))
 * functions. Functions not marked with an asterisk are optional,
 * but possibly slow since they are too general.</p>
 *
 */
interface IGraphics2D : public IUnknown
{
  /// Initialize the 2D graphics system.
  STDMETHOD (Initialize) () PURE;

 /**
  * This routine should be called before any draw operations.
  * It should return S_OK if graphics context is ready.
  */
  STDMETHOD (BeginDraw) () PURE;
  /// This routine should be called when you finished drawing.
  STDMETHOD (FinishDraw) () PURE;

  /// (*) Flip video pages (or dump backbuffer into framebuffer).
  STDMETHOD (Print) (csRect* pArea) PURE;

  /// Clear backbuffer.
  STDMETHOD (Clear) (int color) PURE;
  /// Clear all video pages.
  STDMETHOD (ClearAll) (int color) PURE;

  /// Draw a line.
  STDMETHOD (DrawLine) (float x1, float y1, float x2, float y2, int color) PURE;
  /// Draw a box
  STDMETHOD (DrawBox) (int x1, int x2, int y1, int y2, int color) PURE;
 /**
  * Clip a line against given rectangle.
  * Function returns S_OK if line is not visible.
  */
  STDMETHOD (ClipLine) (float& x1, float& y1, float& x2, float& y2,
    int xmin, int ymin, int xmax, int ymax) PURE;
  /// Draw a pixel.
  STDMETHOD (DrawPixel) (int x, int y, int color) PURE;
  /// Returns the address of the pixel at the specified (x, y) coordinates.
  STDMETHOD (GetPixelAt) (int x, int y, unsigned char** pPixel) PURE;
  /// Draw a sprite using a rectangle from given texture
  STDMETHOD (DrawSprite) (ITextureHandle *hTex, int sx, int sy, int sw, int sh, int tx, int ty, int tw, int th) PURE;

  /// Enable or disable double buffering; returns S_OK or S_FALSE.
  STDMETHOD (DoubleBuffer) (bool Enable) PURE;
  /// Get the double buffer state
  STDMETHOD (GetDoubleBufferState) (bool& State) PURE;

 /**
  * Save a subarea of screen area into the variable Data.
  * Storage is allocated in this call, you should either FreeArea()
  * it after usage or RestoreArea () it.
  */
  STDMETHOD (SaveArea) (ImageArea** Area, int x, int y, int w, int h) PURE;
  /// Restore a subarea of screen saved with SaveArea()
  STDMETHOD (RestoreArea) (ImageArea* Area, bool bFree) PURE;
  /// Free storage allocated for a subarea of screen
  STDMETHOD (FreeArea) (ImageArea* Area) PURE;

  /// Retrieve clipping rectangle
  STDMETHOD (GetClipRect) (int& nMinX, int& nMinY, int& nMaxX, int& nMaxY) PURE;
  /// Set clipping rectangle
  STDMETHOD (SetClipRect) (int nMinX, int nMinY, int nMaxX, int nMaxY) PURE;

  /// Set mouse position (relative to top-left of CS window).
  STDMETHOD (SetMousePosition) (int x, int y) PURE;

 /**
  * Set mouse cursor to one of predefined shape classes
  * (see csmcXXX enum above). If a specific mouse cursor shape
  * is not supported, return 'false'; otherwise return 'true'.
  * If system supports it and iBitmap != NULL, shape should be
  * set to the bitmap passed as second argument; otherwise cursor
  * should be set to its nearest system equivalent depending on
  * iShape argument.
  */
  STDMETHOD (SetMouseCursor) (csMouseCursorID Shape, ITextureHandle *hBitmap) PURE;
  /// (*) Set a color index to given R,G,B (0..255) values
  STDMETHOD (SetRGB) (int i, int r, int g, int b) PURE;
  /// Write a text string into the back buffer
  STDMETHOD (Write) (int x, int y, int fg, int bg, char* str) PURE;
  /// Write a single character.
  STDMETHOD (WriteChar) (int x, int y, int fg, int bg, char c) PURE;

  /// Gets the type of the font.
  STDMETHOD (GetFontID) (int& id) PURE;
  /// Sets the type of the font.
  STDMETHOD (SetFontID) (int id) PURE;

  /// Open the device.
  STDMETHOD (Open) (char *Title) PURE;
  /// Close the device.
  STDMETHOD (Close) () PURE;

 /**
  * Perform a system specific exension.
  * This is probably not the good way to do this but I see no other
  * way currently.
  */
  STDMETHOD (PerformExtension) (char* args) PURE;
};

/**
 * This interface is used to retrieve information
 * about the current pixel format and palette
 * entries.
 */
interface IGraphicsInfo : public IUnknown
{
  /// Get active videopage number (starting from zero)
  STDMETHOD (GetPage) (int& nPage) PURE;

 /**
  * Return the number of bytes for every pixel.
  * This function is equivalent to the PixelBytes field that
  * you get from GetPixelFormat.
  */
  STDMETHOD (GetPixelBytes) (int& nPixelBytes) PURE;

 /**
  * Return information about about the pixel format.
  */
  STDMETHOD (GetPixelFormat) (csPixelFormat* PixelFormat) PURE;

  /// Get the current font ID
  STDMETHOD (GetFontID) (int& nID) PURE;

  /// Return the width of the framebuffer.
  STDMETHOD (GetWidth) (int& nWidth) PURE;
  /// Return the height of the framebuffer.
  STDMETHOD (GetHeight) (int& nHeight) PURE;
  /// Returns 'true' if the program is being run full-screen.
  STDMETHOD (GetIsFullScreen) (bool& bIsFullScreen) PURE;

 /**
  * Return the number of palette entries that can be modified.
  * This should return 0 if there is no palette (true color displays).
  * This function is equivalent to the PalEntries field that you
  * get from GetPixelFormat. It is just a little bit easier to obtain
  * this way.
  */
  STDMETHOD (GetNumPalEntries) (int& nEntries) PURE;
  /// Get the palette (if there is one)
  STDMETHOD (GetPalette) (RGBpaletteEntry** pPalette) PURE;

  /// Get the string equivilent of the HRESULT.
  STDMETHOD (GetStringError) (HRESULT hRes, char* szErrorString) PURE;

  /// Get the width of a string if it would be drawn with given font
  STDMETHOD (GetTextWidth) (int &Width, int Font, char *text) PURE;
  /// Get the height of given font
  STDMETHOD (GetTextHeight) (int &Height, int Font) PURE;
};

extern const IID IID_IGraphics2DFactory;

interface IGraphics2DFactory : public IUnknown
{
  /// Create the graphics context
  STDMETHOD (CreateInstance) (REFIID riid, ISystem* piSystem, void** ppv) PURE;

  /// Lock or unlock from memory.
  STDMETHOD (LockServer) (COMBOOL bLock) PURE;
};

#endif      // __IGRAPH2D_H__
