/*
    Copyright (C) 1998 by Jorrit Tyberghein
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

#ifndef __GRAPH2D_H__
#define __GRAPH2D_H__

#include "csutil/scf.h"
#include "igraph2d.h"

/// This structure is used to define a external list of bitmap fonts
struct FontDef
{
  /// Width of font (if IndividualWidth == NULL)
  int Width;
  /// Height of font
  int Height;
  /// Bytes per character
  int BytesPerChar;
  /// An array of character widths or NULL
  unsigned char *IndividualWidth;
  /// A pointer to font bitmap
  unsigned char *FontBitmap;
};

/// The array containing the definitions of all fonts
extern FontDef FontList[];

/**
 * This is the base class for 2D renderer. System-dependent ports
 * should derive their own SysGraphics2D class from this one and
 * implement required (marked with an asterisk (*)) functions.
 * Functions not marked with an asterisk are optional, but possibly
 * slow since they are too general.
 */
class csGraphics2D : public iGraphics2D
{
public:
  /// The clipping rectangle
  int ClipX1, ClipX2, ClipY1, ClipY2;

  /// The pixel format
  csPixelFormat pfmt;

  /// Most systems have a pointer to (real or pseudo) video RAM
  unsigned char *Memory;

  /// Keep a array of Y*width to avoid multiplications
  int *LineAddress;

  /// The system driver.
  iSystem* System;

  /// Current font number
  int Font;
  /// The width, height and depth of visual
  int Width, Height, Depth;
  /// True if visual is full-screen
  bool FullScreen;
  /// 256-color palette
  RGBpaletteEntry Palette[256];
  /// true if some palette entry is already allocated
  bool PaletteAlloc[256];
  // The counter that is incremented inside BeginDraw and decremented in FinishDraw
  int FrameBufferLocked;

public:
  /// Create csGraphics2D object
  csGraphics2D ();
  /// Destroy csGraphics2D object
  virtual ~csGraphics2D ();

  /// Initialize the plugin
  virtual bool Initialize (iSystem *pSystem);

  /// (*) Open graphics system (set videomode, open window etc)
  virtual bool Open (const char *Title);
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
  virtual void Print (csRect *area = NULL) = 0;

  /// Get active videopage number (starting from zero)
  virtual int GetPage ();
  /// Enable or disable double buffering; return TRUE if supported
  virtual bool DoubleBuffer (bool Enable);
  /// Return current double buffering state
  virtual bool GetDoubleBufferState ();

  /// Clear backbuffer
  virtual void Clear (int color);
  /// Clear all video pages
  virtual void ClearAll (int color);

  /**
   * To facilitate multiple pixel formats, the most critical drawing routines
   * are defined as pointers to functions, not as virtual methods.
   * This allows deciding at run-time which function we will choose.
   */
  /// Draw a pixel
  void (*_DrawPixel) (csGraphics2D *This, int x, int y, int color);
  /// Same but exposed through iGraphics2D interface
  virtual void DrawPixel (int x, int y, int color)
  { _DrawPixel (this, x, y, color); }
  /// Draw a line
  virtual void DrawLine (float x1, float y1, float x2, float y2, int color);
  /// Draw a box of given width and height
  virtual void DrawBox (int x, int y, int w, int h, int color);
  /// (*) Set a color index to given R,G,B (0..255) values
  virtual void SetRGB (int i, int r, int g, int b);
  /// Write a text string into the back buffer
  virtual void Write (int x, int y, int fg, int bg, const char *text);
  /// Write a single character
  void (*_WriteChar) (csGraphics2D *This, int x, int y, int fg, int bg, char c);
  /// Same but exposed through iGraphics2D interface
  virtual void WriteChar (int x, int y, int fg, int bg, char c)
  { _WriteChar (this, x, y, fg, bg, c); }
  /// Get the width of a string if it would be drawn with given font
  virtual int GetTextWidth (int Font, const char *text);
  /// Get the height of given font
  virtual int GetTextHeight (int Font);
  /**
   * Draw a sprite (possibly rescaled to given width (sw) and height (sh))
   * using given rectangle from given texture
   */
  void (*_DrawSprite) (csGraphics2D *This, iTextureHandle *hTex, int sx, int sy,
    int sw, int sh, int tx, int ty, int tw, int th);
  /// Same but exposed through iGraphics2D interface
  virtual void DrawSprite (iTextureHandle *hTex, int sx, int sy, int sw, int sh,
    int tx, int ty, int tw, int th)
  { _DrawSprite (this, hTex, sx, sy, sw, sh, tx, ty, tw, th); }
  /// (*) Get address of video RAM at given x,y coordinates
  unsigned char* (*_GetPixelAt) (csGraphics2D *This, int x, int y);
  /// Same but exposed through iGraphics2D interface
  virtual unsigned char *GetPixelAt (int x, int y)
  { return _GetPixelAt (this, x, y); }

  /**
   * Return the number of palette entries that can be modified.
   * This should return 0 if there is no palette (true color displays).
   * This function is equivalent to the PalEntries field that you
   * get from GetPixelFormat. It is just a little bit easier to obtain
   * this way.
   */
  virtual int GetNumPalEntries ()
  { return pfmt.PalEntries; }

  /**
   * Return the number of bytes for every pixel.
   * This function is equivalent to the PixelBytes field that
   * you get from GetPixelFormat.
   */
  virtual int GetPixelBytes ()
  { return pfmt.PixelBytes; }

  /**
   * Return information about about the pixel format.
   */
  virtual csPixelFormat* GetPixelFormat ()
  { return &pfmt; }

  /**
   * Save a subarea of screen area into the variable Data.
   * Storage is allocated in this call, you should either FreeArea()
   * it after usage or RestoreArea() it.
   */
  virtual csImageArea *SaveArea (int x, int y, int w, int h);
  /// Restore a subarea of screen saved with SaveArea()
  virtual void RestoreArea (csImageArea *Area, bool Free = true);
  /// Free storage allocated for a subarea of screen
  virtual void FreeArea (csImageArea *Area);

  /**
   * Clip a line against given rectangle
   * Function returns true if line is not visible
   */
  virtual bool ClipLine (float &x1, float &y1, float &x2, float &y2,
    int xmin, int ymin, int xmax, int ymax);

  /// Set mouse cursor position; return success status
  virtual bool SetMousePosition (int x, int y);

  /**
   * Set mouse cursor to one of predefined shape classes
   * (see csmcXXX enum above). If a specific mouse cursor shape
   * is not supported, return 'false'; otherwise return 'true'.
   * If system supports it and iBitmap != NULL, shape should be
   * set to the bitmap passed as second argument; otherwise cursor
   * should be set to its nearest system equivalent depending on
   * iShape argument.
   */
  virtual bool SetMouseCursor (csMouseCursorID iShape, iTextureHandle *hBitmap);

  /// Gets the ID of current font.
  virtual int GetFontID ()
  { return Font; }
  /// Sets the type of the font.
  virtual void SetFontID (int FontID)
  { Font = FontID; }

  /// Return the width of the framebuffer.
  virtual int GetWidth ()
  { return Width; }
  /// Return the height of the framebuffer.
  virtual int GetHeight ()
  { return Height; }
  /// Returns 'true' if the program is being run full-screen.
  virtual bool GetFullScreen ()
  { return FullScreen; }

  /// Get the palette (if there is one)
  virtual RGBpaletteEntry *GetPalette ()
  { return pfmt.PalEntries ? Palette : NULL; }

  /// Query pixel R,G,B at given screen location
  virtual void GetPixel (int x, int y, UByte &oR, UByte &oG, UByte &oB);

  /**
   * Perform a system specific extension. Return false if extension
   * not supported.
   */
  virtual bool PerformExtension (char* args);

protected:
  /// A replacement for ::CsPrintf() that works through system driver
  void CsPrintf(int mode, const char* text, ...);

  /**
   * Little helper function to complete a csPixelFormat
   * structure given that the masks are correctly filled in.
   */
  void complete_pixel_format ();

  /**
   * Default drawing routines for 8-bit and 16-bit modes
   * If a system port has its own routines, it should assign
   * their addresses to respective pointers.
   */

  /// Draw a pixel in 8-bit modes
  static void DrawPixel8 (csGraphics2D *This, int x, int y, int color);
  /// Write a character in 8-bit modes
  static void WriteChar8 (csGraphics2D *This, int x, int y, int fg, int bg, char c);
  /// Return address of a 8-bit pixel
  static unsigned char *GetPixelAt8 (csGraphics2D *This, int x, int y);
  /// Draw a sprite on 8-bit display using a rectangle from given texture
  static void DrawSprite8 (csGraphics2D *This, iTextureHandle *hTex, int sx, int sy, int sw, int sh,
    int tx, int ty, int tw, int th);

  /// Draw a pixel in 16-bit modes
  static void DrawPixel16 (csGraphics2D *This, int x, int y, int color);
  /// Write a character in 16-bit modes
  static void WriteChar16 (csGraphics2D *This, int x, int y, int fg, int bg, char c);
  /// Return address of a 16-bit pixel
  static unsigned char *GetPixelAt16 (csGraphics2D *This, int x, int y);
  /// Draw a sprite on 16-bit display using a rectangle from given texture
  static void DrawSprite16 (csGraphics2D *This, iTextureHandle *hTex, int sx, int sy, int sw, int sh,
    int tx, int ty, int tw, int th);

  /// Draw a pixel in 32-bit modes
  static void DrawPixel32 (csGraphics2D *This, int x, int y, int color);
  /// Write a character in 32-bit modes
  static void WriteChar32 (csGraphics2D *This, int x, int y, int fg, int bg, char c);
  /// Return address of a 32-bit pixel
  static unsigned char *GetPixelAt32 (csGraphics2D *This, int x, int y);
  /// Draw a sprite on 32-bit display using a rectangle from given texture
  static void DrawSprite32 (csGraphics2D *This, iTextureHandle *hTex, int sx, int sy, int sw, int sh,
    int tx, int ty, int tw, int th);
};

#endif // __GRAPH2D_H__
