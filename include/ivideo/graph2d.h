/*
    Copyright (C) 2001 by Jorrit Tyberghein
    Copyright (C) 1998-2000 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CS_IVIDEO_GRAPH2D_H__
#define __CS_IVIDEO_GRAPH2D_H__

/**\file
 * 2D graphics interface
 */

/**
 * \addtogroup gfx2d
 * @{ */
 
#include "csutil/scf.h"
#include "csgfx/rgbpixel.h"
#include "ivideo/cursor.h"


struct iImage;
struct iFontServer;
struct iFont;
struct iNativeWindow;
struct iGraphics2D;

class csRect;

/// iGraphics2D::Write() flags.
enum
{
  /**
   * Write by baseline, \p x and \p y are treated as the pen position on
   * a baseline. 
   */
  CS_WRITE_BASELINE    = (1 << 0),
  /**
   * Don't use anti-aliased glyphs.
   */
  CS_WRITE_NOANTIALIAS = (1 << 1)
};

/// Simple 2D pixel coordinate
struct csPixelCoord
{
  /// X component
  int x;
  /// Y component
  int y;
};

/**
 * Structure describing the pixel format.
 */
struct csPixelFormat
{
  /**
   * The masks to extract the color information from a pixel (truecolor mode
   * only). Ignore the Mask and Shift fields of this structure if
   * PalEntries != 0.
   */
  uint32 RedMask, GreenMask, BlueMask, AlphaMask;
  /**
   * The shifts to extract the color information from a pixel (truecolor mode
   * only).
   */
  int RedShift, GreenShift, BlueShift, AlphaShift;
  /// The number of significant bits for every color.
  int RedBits, GreenBits, BlueBits, AlphaBits;

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

  /**
   * Little helper function to complete a csPixelFormat
   * structure given that the masks are correctly filled in.
   */
  void complete ()
  {
#define COMPUTE(comp)                                                   \
    {                                                                   \
      unsigned long i, tmp = comp##Mask;                                \
      for (i = 0; tmp && !(tmp & 1); tmp >>= 1, i++) {}                 \
      comp##Shift = i;                                                  \
      for (i = 0; tmp & 1; tmp >>= 1, i++) {}                           \
      comp##Bits = i;                                                   \
    }
    COMPUTE (Red);
    COMPUTE (Green);
    COMPUTE (Blue);
    COMPUTE (Alpha);
#undef COMPUTE
  }
};

/// This structure is used for saving/restoring areas of screen
struct csImageArea
{
  int x, y, w, h;
  char *data;

  inline csImageArea (int sx, int sy, int sw, int sh)
  { x = sx; y = sy; w = sw; h = sh; data = 0; }
};

SCF_VERSION (iOffscreenCanvasCallback, 1, 0, 0);

/**
 * When you create an offscreen canvas (CreateOffscreenCanvas()) then
 * you can use this callback to get informed when the texture has
 * been modified (FinishDraw() called) or a palette entry is modified.
 */
struct iOffscreenCanvasCallback : public iBase
{
  /// FinishDraw has been called.
  virtual void FinishDraw (iGraphics2D* canvas) = 0;
  /// Palette entry has been modified.
  virtual void SetRGB (iGraphics2D* canvas, int idx, int r, int g, int b) = 0;
};

SCF_VERSION (iGraphics2D, 2, 6, 0);

/**
 * This is the interface for 2D renderer. The 2D renderer is responsible
 * for all 2D operations such as creating the window, switching pages,
 * returning pixel format and so on.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>OpenGL/Windows canvas plugin (crystalspace.graphics2d.glwin32)
 *   <li>OpenGL/X11 canvas plugin (crystalspace.graphics2d.glx)
 *   <li>DirectDraw canvas plugin (crystalspace.graphics2d.directdraw)
 *   <li>X11 canvas plugin (crystalspace.graphics2d.x2d)
 *   <li>Memory canvas plugin (crystalspace.graphics2d.memory)
 *   <li>Null 2D canvas plugin (crystalspace.graphics2d.null)
 *   <li>Some others.
 *   <li>Note that it is the 3D renderer that will automatically create
 *       the right instance of the canvas that it requires.
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>CS_QUERY_REGISTRY()
 *   <li>iGraphics3D::GetDriver2D()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>3D renderers (iGraphics3D implementations)
 *   </ul>
 */
struct iGraphics2D : public virtual iBase
{
  /// Open the device.
  virtual bool Open () = 0;

  /// Close the device.
  virtual void Close () = 0;

  /// Return the width of the framebuffer.
  virtual int GetWidth () = 0;

  /// Return the height of the framebuffer.
  virtual int GetHeight () = 0;

  /// Get active videopage number (starting from zero)
  virtual int GetPage () = 0;

  /// Enable or disable double buffering; returns success status
  virtual bool DoubleBuffer (bool Enable) = 0;

  /// Get the double buffer state
  virtual bool GetDoubleBufferState () = 0;

  /// Return information about the pixel format.
  virtual csPixelFormat const* GetPixelFormat () = 0;

  /**
   * Return the number of bytes for every pixel.
   * This function is equivalent to the PixelBytes field that
   * you get from GetPixelFormat().
   */
  virtual int GetPixelBytes () = 0;

  /**
   * Return the number of palette entries that can be modified.
   * This should return 0 if there is no palette (true color displays).
   * This function is equivalent to the PalEntries field that you
   * get from GetPixelFormat(). It is just a little bit easier to obtain
   * this way.
   */
  virtual int GetPalEntryCount () = 0;

  /// Get the palette (if there is one)
  virtual csRGBpixel *GetPalette () = 0;

  /**
   * Set a color index to given R,G,B (0..255) values. Only use if there
   * is a palette.
   */
  virtual void SetRGB (int i, int r, int g, int b) = 0;
  /**
   * Find an RGB (0..255) color. If there is a palette, this returns an
   * entry index set with SetRGB(). If the returned value is -1, a
   * suitable palette entry was not found.
   * Without a palette, the actual color bytes are returned.
   * <p>
   * Use returned value for color arguments in iGraphics2D.
   */
  virtual int FindRGB (int r, int g, int b, int a = 255) = 0;

  /**
   * Retrieve the R,G,B tuple for a given color index.
   */
  virtual void GetRGB (int color, int& r, int& g, int& b) = 0;
  /**
   * Retrieve the R,G,B,A tuple for a given color index.
   */
  virtual void GetRGB (int color, int& r, int& g, int& b, int& a) = 0;
  
  /**
   * Set clipping rectangle.
   * The clipping rectangle is inclusive the top and left edges and exclusive
   * for the right and bottom borders.
   */
  virtual void SetClipRect (int nMinX, int nMinY, int nMaxX, int nMaxY) = 0;

  /// Retrieve clipping rectangle
  virtual void GetClipRect(int& nMinX, int& nMinY, int& nMaxX, int& nMaxY) = 0;

  /**
   * This routine should be called before any draw operations.
   * It should return true if graphics context is ready.
   */
  virtual bool BeginDraw () = 0;

  /// This routine should be called when you finished drawing.
  virtual void FinishDraw () = 0;

  /**
   * Flip video pages (or dump backbuffer into framebuffer). The area
   * parameter is only a hint to the canvas driver. Changes outside the
   * rectangle may or may not be printed as well.
   */
  virtual void Print (csRect const* pArea) = 0;

  /// Clear backbuffer.
  virtual void Clear (int color) = 0;

  /// Clear all video pages.
  virtual void ClearAll (int color) = 0;

  /// Draw a line.
  virtual void DrawLine(float x1, float y1, float x2, float y2, int color) = 0;

  /// Draw a box
  virtual void DrawBox (int x, int y, int w, int h, int color) = 0;

  /**
   * Clip a line against given rectangle.
   * Function returns true if line is not visible.
   */
  virtual bool ClipLine (float& x1, float& y1, float& x2, float& y2,
    int xmin, int ymin, int xmax, int ymax) = 0;

  /// Draw a pixel.
  virtual void DrawPixel (int x, int y, int color) = 0;

  /// Draw an array of pixel coordinates with the given color.
  virtual void DrawPixels(csPixelCoord const* pixels, int num_pixels,
     int color) = 0;

  /// Blit a memory block.  Format of the image is RGBA in bytes. Row by row.
  virtual void Blit (int x, int y, int width, int height,
    unsigned char const* data) = 0;

  /// Returns the address of the pixel at the specified (x, y) coordinates.
  virtual unsigned char *GetPixelAt (int x, int y) = 0;

  /// Query pixel R,G,B at given screen location
  virtual void GetPixel (int x, int y, uint8 &oR, uint8 &oG, uint8 &oB) = 0;
  /// As GetPixel() above, but with alpha
  virtual void GetPixel (int x, int y, uint8 &oR, uint8 &oG, uint8 &oB, uint8 &oA) = 0;

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

  /**
   * Write a text string into the back buffer. A value of -1 for \p bg
   * color will not draw the background.
   * \remarks \p str is expected to be UTF-8 encoded.
   * \remarks For transparent backgrounds, it is recommended to obtain a color
   *  value from FindRGB() that has the same R, G, B components as the 
   *  foreground color, but an alpha component of 0.
   */
  virtual void Write (iFont *font, int x, int y, int fg, int bg,
    const char *str, uint flags = 0) = 0;

  /**
   * Write a text string into the back buffer. A value of -1 for \p bg
   * color will not draw the background. x and y are the pen position on
   * a baseline. The actual font baseline is shifted up by the font's descent.
   * \deprecated
   * Instead, use Write() with the #CS_WRITE_BASELINE flag set.
   */
  CS_DEPRECATED_METHOD virtual void WriteBaseline (iFont *font, 
    int x, int y, int fg, int bg, const char *str) = 0;

  /// Enable/disable canvas resizing
  virtual void AllowResize (bool iAllow) = 0;

  /// Resize the canvas
  virtual bool Resize (int w, int h) = 0;

  /// Get the active font server (does not do IncRef())
  virtual iFontServer *GetFontServer () = 0;

  /**
   * Perform a system specific exension.<p>
   * The command is a string; any arguments may follow.
   * There is no way to guarantee the uniquiness of
   * commands, so please try to use descriptive command names rather
   * than "a", "b" and so on...
   */
  virtual bool PerformExtension (char const* command, ...) = 0;

  /**
   * Perform a system specific exension.<p>
   * Just like PerformExtension() except that the command arguments are passed
   * as a `va_list'.
   */
  virtual bool PerformExtensionV (char const* command, va_list) = 0;

  /// Do a screenshot: return a new iImage object
  virtual csPtr<iImage> ScreenShot () = 0;

  /**
   * Get the native window corresponding with this canvas.
   * If this is an off-screen canvas then this will return 0.
   */
  virtual iNativeWindow* GetNativeWindow () = 0;

  /// Returns 'true' if the program is being run full-screen.
  virtual bool GetFullScreen () = 0;

  /**
   * Change the fullscreen state of the canvas.
   */
  virtual void SetFullScreen (bool b) = 0;

  /// Set mouse position (relative to top-left of CS window).
  virtual bool SetMousePosition (int x, int y) = 0;

  /**
   * Set mouse cursor to one of predefined shape classes
   * (see csmcXXX enum above). If a specific mouse cursor shape
   * is not supported, return 'false'; otherwise return 'true'.
   * If system supports it the cursor should be set to its nearest
   * system equivalent depending on iShape argument and the routine
   * should return "true".
   */
  virtual bool SetMouseCursor (csMouseCursorID iShape) = 0;

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
                               csRGBcolor bg = csRGBcolor(0,0,0)) = 0;

  /**
   * Set gamma value (if supported by canvas). By default this is 1.
   * Smaller values are darker. If the canvas doesn't support gamma
   * then this function will return false.
   */
  virtual bool SetGamma (float gamma) = 0;

  /**
   * Get gamma value.
   */
  virtual float GetGamma () const = 0;

  /**
   * Create an off-screen canvas so you can render on a given memory
   * area. If depth==8 then the canvas will use palette mode. In that
   * case you can do SetRGB() to initialize the palette.
   * The callback interface (if given) is used to communicate from the
   * canvas back to the caller. You can use this to detect when the
   * texture data has changed for example.
   */
  virtual csPtr<iGraphics2D> CreateOffscreenCanvas (
  	void* memory, int width, int height, int depth,
	iOffscreenCanvasCallback* ofscb) = 0;
};

/** @} */

#endif // __CS_IVIDEO_GRAPH2D_H__

