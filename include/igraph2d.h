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

#include "csutil/scf.h"
#include "iplugin.h"
#include "itexture.h"

class csRect;

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
  ULong RedMask, GreenMask, BlueMask;
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

/// This structure is used for saving/restoring areas of screen
struct csImageArea
{
  int x, y, w, h;
  char *data;

  inline csImageArea (int sx, int sy, int sw, int sh)
  { x = sx; y = sy; w = sw; h = sh; data = NULL; }
};

SCF_VERSION (iGraphics2D, 0, 0, 2);

/**
 * This is the interface for 2D renderer. The 2D renderer is responsible
 * for all 2D operations such as creating the window, switching pages,
 * returning pixel format and so on.
 */
struct iGraphics2D : public iPlugIn
{
  /// Open the device.
  virtual bool Open (const char *Title) = 0;
  /// Close the device.
  virtual void Close () = 0;

  /// Set clipping rectangle
  virtual void SetClipRect (int nMinX, int nMinY, int nMaxX, int nMaxY) = 0;
  /// Retrieve clipping rectangle
  virtual void GetClipRect (int& nMinX, int& nMinY, int& nMaxX, int& nMaxY) = 0;

 /**
  * This routine should be called before any draw operations.
  * It should return S_OK if graphics context is ready.
  */
  virtual bool BeginDraw () = 0;
  /// This routine should be called when you finished drawing.
  virtual void FinishDraw () = 0;

  /// Flip video pages (or dump backbuffer into framebuffer).
  virtual void Print (csRect *pArea) = 0;

  /// Get active videopage number (starting from zero)
  virtual int GetPage () = 0;
  /// Enable or disable double buffering; returns success status
  virtual bool DoubleBuffer (bool Enable) = 0;
  /// Get the double buffer state
  virtual bool GetDoubleBufferState () = 0;

  /// Clear backbuffer.
  virtual void Clear (int color) = 0;
  /// Clear all video pages.
  virtual void ClearAll (int color) = 0;

  /// Draw a line.
  virtual void DrawLine (float x1, float y1, float x2, float y2, int color) = 0;
  /// Draw a box
  virtual void DrawBox (int x1, int x2, int y1, int y2, int color) = 0;
 /**
  * Clip a line against given rectangle.
  * Function returns true if line is not visible.
  */
  virtual bool ClipLine (float& x1, float& y1, float& x2, float& y2,
    int xmin, int ymin, int xmax, int ymax) = 0;
  /// Draw a pixel.
  virtual void DrawPixel (int x, int y, int color) = 0;
  /// Returns the address of the pixel at the specified (x, y) coordinates.
  virtual unsigned char *GetPixelAt (int x, int y) = 0;
  /// Draw a sprite using a rectangle from given texture
  virtual void DrawSprite (iTextureHandle *hTex, int sx, int sy, int sw, int sh,
    int tx, int ty, int tw, int th) = 0;

 /**
  * Save a subarea of screen and return a handle to saved buffer.
  * Storage is allocated in this call, you should either FreeArea()
  * the handle after usage or RestoreArea () it.
  */
  virtual csImageArea *SaveArea (int x, int y, int w, int h) = 0;
  /// Restore a subarea of screen saved with SaveArea()
  virtual void RestoreArea (csImageArea *Area, bool Free) = 0;
  /// Free storage allocated for a subarea of screen
  virtual void FreeArea (csImageArea *Area) = 0;

  /// Set mouse position (relative to top-left of CS window).
  virtual bool SetMousePosition (int x, int y) = 0;

 /**
  * Set mouse cursor to one of predefined shape classes
  * (see csmcXXX enum above). If a specific mouse cursor shape
  * is not supported, return 'false'; otherwise return 'true'.
  * If system supports it and iBitmap != NULL, shape should be
  * set to the bitmap passed as second argument; otherwise cursor
  * should be set to its nearest system equivalent depending on
  * iShape argument.
  */
  virtual bool SetMouseCursor (csMouseCursorID Shape, iTextureHandle *hBitmap) = 0;
  /// Set a color index to given R,G,B (0..255) values
  virtual void SetRGB (int i, int r, int g, int b) = 0;
  /// Write a text string into the back buffer
  virtual void Write (int x, int y, int fg, int bg, const char *str) = 0;
  /// Write a single character.
  virtual void WriteChar (int x, int y, int fg, int bg, char c) = 0;

  /// Gets the ID of current font.
  virtual int GetFontID () = 0;
  /// Sets the type of the font.
  virtual void SetFontID (int FontID) = 0;

 /**
  * Perform a system specific exension.
  * This is probably not the good way to do this but I see no other
  * way currently.
  */
  virtual bool PerformExtension (char *args) = 0;

 /**
  * Return the number of bytes for every pixel.
  * This function is equivalent to the PixelBytes field that
  * you get from GetPixelFormat.
  */
  virtual int GetPixelBytes () = 0;

 /**
  * Return information about the pixel format.
  */
  virtual csPixelFormat *GetPixelFormat () = 0;

  /// Return the width of the framebuffer.
  virtual int GetWidth () = 0;
  /// Return the height of the framebuffer.
  virtual int GetHeight () = 0;
  /// Returns 'true' if the program is being run full-screen.
  virtual bool GetFullScreen () = 0;

 /**
  * Return the number of palette entries that can be modified.
  * This should return 0 if there is no palette (true color displays).
  * This function is equivalent to the PalEntries field that you
  * get from GetPixelFormat. It is just a little bit easier to obtain
  * this way.
  */
  virtual int GetNumPalEntries () = 0;
  /// Get the palette (if there is one)
  virtual RGBpaletteEntry *GetPalette () = 0;

  /// Get the width of a string if it would be drawn with given font
  virtual int GetTextWidth (int FontID, const char *text) = 0;
  /// Get the height of given font
  virtual int GetTextHeight (int FontID) = 0;

  /// Query pixel R,G,B at given screen location
  virtual void GetPixel (int x, int y, UByte &oR, UByte &oG, UByte &oB) = 0;
};

#endif // __IGRAPH2D_H__
