/*
    Copyright (C) 1998-2003 by Jorrit Tyberghein
		       2003 by Frank Richter

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

/**\file
 * Helper routines to pack csRGBcolor/csRGBpixel into 
 * RGB/RGBA byte arrays.
 */

#ifndef __CSGFX_PACKRGB_H__
#define __CSGFX_PACKRGB_H__

#include "csextern.h"

#include "cstypes.h"
#include "csgfx/rgbpixel.h"


/**\addtogroup gfx
 * @{
 */

/**\name Pixel packing
 * When using csRGBcolor or csRGBpixel, <b>don't</b> assume that
 * sizeof(csRGBcolor) == 3, sizeof(csRGBpixel) == 4 or that those
 * struct are stored as a packed RGB/RGBA byte array. If you need
 * such packed RGB/RGBA data (e.g. for use with external libraries)
 * use the functions here to convert from csRGBcolor/csRGBpixel to
 * RGB/RGBA and vice versa. If no copy of the data is needed, they 
 * are free on platforms where sizeof(csRGBcolor) == 3, 
 * sizeof(csRGBpixel) == 4 are true. In any other case copying/packing
 * the data is handled appropriately.
 *
 * @{
 */

/**
 * RGB data packing.
 */
struct csPackRGB
{
  static bool IsRGBcolorSane() { return (sizeof(csRGBcolor) == 3); }
  /**
   * Pack an array of csRGBcolor into a RGB byte array. 
   * \param buf Buffer to pack the data into.
   * \param pixels Source array of csRGBcolor data
   * \param numPixels Number of pixels in the array
   */
  static void PackRGBcolorToRGBBuffer (uint8* buf, 
    const csRGBcolor* pixels, size_t numPixels)
  {
    if (IsRGBcolorSane())
      memcpy (buf, pixels, numPixels * 3);
    else
    {
      uint8* bufptr = buf;
      while (numPixels--)
      {
	*bufptr++ = pixels->red;
	*bufptr++ = pixels->green;
	*bufptr++ = pixels->blue;
	pixels++; 
      }
    }
  }
  /**
   * Pack an array of csRGBcolor into a RGB byte array. 
   * \remarks
   * May return \p pixels.
   * May allocate memory. Free it using DiscardPackedRGB() when finished.
   * \param pixels Source array of csRGBcolor data
   * \param numPixels Number of pixels in the array
   * \return A byte array containing the source data packed as RGB.
   */
  static const uint8* PackRGBcolorToRGB (const csRGBcolor* pixels, 
    size_t numPixels)
  {
    if (IsRGBcolorSane())
      return (uint8*)pixels;
    else
    {
      uint8* buf = new uint8[numPixels * 3];
      PackRGBcolorToRGBBuffer (buf, pixels, numPixels);
      return buf;
    }
  }
  /**
   * Frees memory possibly allocated by PackRGBcolorToRGB().
   * \param rgb Pointer to packed RGB data returned by PackRGBcolorToRGB().
   */
  static void DiscardPackedRGB (const uint8* rgb) 
  {
    if (!IsRGBcolorSane())
      delete[] (uint8*)rgb;
  }
  /**
   * Unpack a RGB byte array into an array of csRGBcolor. 
   * \param buf Buffer to unpack the data into.
   * \param rgb Source array of RGB data
   * \param numPixels Number of pixels in the array
   */
  static void UnpackRGBtoRGBcolor (csRGBcolor* buf, const uint8* rgb, 
    size_t numPixels)
  {
    if (IsRGBcolorSane())
      memcpy (buf, rgb, numPixels * 3);
    else
    {
      csRGBcolor* bufptr = buf;
      while (numPixels--)
      {
	bufptr->red = *rgb++;
	bufptr->green = *rgb++;
	bufptr->blue = *rgb++;
	bufptr++; 
      }
    }
  }
  /**
   * Unpack a RGB byte array into an array of csRGBcolor. 
   * \remarks
   * May return \p pixels.
   * May allocate memory. Free it using sDiscardUnpackedRGBcolor() when finished.
   * \param rgb Source array of RGB data
   * \param numPixels Number of pixels in the array
   * \return An array containing the source data in csRGBcolor structs.
   */
  static const csRGBcolor* UnpackRGBtoRGBcolor (const uint8* rgb, 
    size_t numPixels)
  {
    if (IsRGBcolorSane())
      return (const csRGBcolor*)rgb;
    else
    {
      csRGBcolor* buf = new csRGBcolor[numPixels];
      UnpackRGBtoRGBcolor (buf, rgb, numPixels);
      return buf;
    }
  }
  /**
   * Frees memory possibly allocated by UnpackRGBtoRGBcolor().
   * \param pixels Pointer to csRGBcolor array returned by UnpackRGBtoRGBcolor().
   */
  static void DiscardUnpackedRGBcolor (const csRGBcolor* pixels) 
  {
    if (!IsRGBcolorSane())
      delete[] (csRGBcolor*)pixels;
  }
  /**
   * Pack an array of csRGBpixel into a RGB byte array. Alpha information 
   * is discarded!
   * \remarks
   * Allocates memory. Free it using delete[] when finished.
   * \param pixels Source array of csRGBpixel data
   * \param numPixels Number of pixels in the array
   * \return A byte array containing the source data packed as RGB.
   */
  static inline uint8* PackRGBpixelToRGB (const csRGBpixel* pixels, 
    size_t numPixels)
  {
    uint8* buf = new uint8[numPixels * 3];
    uint8* bufptr = buf;
    while (numPixels--)
    {
      *bufptr++ = pixels->red;
      *bufptr++ = pixels->green;
      *bufptr++ = pixels->blue;
      pixels++; 
    }
    return buf;
  }
  /**
   * Unpack a RGB byte array into an array of csRGBpixel. 
   * \param buf Buffer to unpack the data into.
   * \param rgb Source array of RGB data
   * \param numPixels Number of pixels in the array
   */
  static void UnpackRGBtoRGBpixelBuffer (csRGBpixel* buf, uint8* rgb,
    size_t numPixels)
  {
    csRGBpixel* bufptr = buf;
    while (numPixels--)
    {
      bufptr->red = *rgb++;
      bufptr->green = *rgb++;
      bufptr->blue = *rgb++;
      bufptr++; 
    }
  }
};

/**
 * RGBA data packing.
 */
struct csPackRGBA
{
  static bool IsRGBpixelSane() { return (sizeof(csRGBpixel) == 4); }
  /**
   * Pack an array of csRGBpixel into a RGBA byte array. 
   * \param buf Buffer to pack the data into.
   * \param pixels Source array of csRGBpixel data
   * \param numPixels Number of pixels in the array
   */
  static void PackRGBpixelToRGBA (uint8* buf, const csRGBpixel* pixels, 
    size_t numPixels)
  {
    if (IsRGBpixelSane())
      memcpy (buf, pixels, numPixels * 4);
    else
    {
      uint8* bufptr = buf;
      while (numPixels--)
      {
	*bufptr++ = pixels->red;
	*bufptr++ = pixels->green;
	*bufptr++ = pixels->blue;
	*bufptr++ = pixels->alpha;
	pixels++; 
      }
    }
  }
  /**
   * Pack an array of csRGBpixel into a RGBA byte array. 
   * \remarks
   * May return \p pixels.
   * May allocate memory. Free it using DiscardPackedRGBA() when finished.
   * \param pixels Source array of csRGBpixel data
   * \param numPixels Number of pixels in the array
   * \return A byte array containing the source data packed as RGBA.
   */
  static const uint8* PackRGBpixelToRGBA (const csRGBpixel* pixels, 
    size_t numPixels)
  {
    if (IsRGBpixelSane())
      return (uint8*)pixels;
    else
    {
      uint8* buf = new uint8[numPixels * 4];
      PackRGBpixelToRGBA (buf, pixels, numPixels);
      return buf;
    }
  }
  /**
   * Frees memory possibly allocated by PackRGBpixelToRGBA().
   * \param rgba Pointer to packed RGB data returned by PackRGBpixelToRGBA().
   */
  static void DiscardPackedRGBA (const uint8* rgba) 
  {
    if (!IsRGBpixelSane())
    {
      delete[] (uint8*)rgba;
    }
  }
  /**
   * Unpack a RGBA byte array into an array of csRGBpixel. 
   * \param buf Buffer to unpack the data into.
   * \param rgba Source array of RGBA data
   * \param numPixels Number of pixels in the array
   */
  static void UnpackRGBAtoRGBpixel (csRGBpixel* buf, const uint8* rgba, 
    size_t numPixels)
  {
    if (IsRGBpixelSane())
      memcpy (buf, rgba, numPixels * 4);
    else
    {
      csRGBpixel* bufptr = buf;
      while (numPixels--)
      {
	bufptr->red = *rgba++;
	bufptr->green = *rgba++;
	bufptr->blue = *rgba++;
	bufptr->alpha = *rgba++;
	bufptr++; 
      }
    }
  }
  /**
   * Unpack a RGBA byte array into an array of csRGBpixel. 
   * \remarks
   * May return \p pixels.
   * May allocate memory. Free it using DiscardUnpackedRGBpixel() when finished.
   * \param rgba Source array of RGBA data
   * \param numPixels Number of pixels in the array
   * \return An array containing the source data in csRGBpixel structs.
   */
  static const csRGBpixel* UnpackRGBAtoRGBpixel (const uint8* rgba, 
    size_t numPixels)
  {
    if (IsRGBpixelSane())
      return (csRGBpixel*)rgba;
    else
    {
      csRGBpixel* buf = new csRGBpixel[numPixels];
      UnpackRGBAtoRGBpixel (buf, rgba, numPixels);
      return buf;
    }
  }
  /**
   * Unpack a RGBA byte array into an array of csRGBpixel. 
   * \remarks
   * Never returns \p rgba.
   * Allocate memory. Free it using delete[] when finished.
   * \param rgba Source array of RGBA data
   * \param numPixels Number of pixels in the array
   * \return An array containing the source data in csRGBpixel structs.
   */
  static csRGBpixel* CopyUnpackRGBAtoRGBpixel (const uint8* rgba, 
    size_t numPixels)
  {
    if (IsRGBpixelSane())
    {
      csRGBpixel* buf = new csRGBpixel[numPixels];
      memcpy (buf, rgba, numPixels * sizeof(csRGBpixel));
      return buf;
    }
    else
      return (csRGBpixel*)UnpackRGBAtoRGBpixel (rgba, numPixels);
  }
  /**
   * Frees memory possibly allocated by UnpackRGBAtoRGBpixel().
   * \param pixels Pointer to csRGBpixel array returned by
   * UnpackRGBAtoRGBpixel().
   */
  static void csDiscardUnpackedRGBpixel (const csRGBpixel* pixels) 
  {
    if (!IsRGBpixelSane())
      delete[] (csRGBpixel*)pixels;
  }
  /**
   * Unpack a RGBA byte array into an array of csRGBcolor. Alpha information 
   * is discarded!
   * \remarks
   * Allocates memory. Free it using delete[] when finished.
   * \param rgba Source array of RGBA data
   * \param numPixels Number of pixels in the array
   * \return An array containing the source data in csRGBcolor structs.
   */
  static inline csRGBcolor* UnpackRGBAtoRGBcolor (const uint8* rgba, 
    size_t numPixels)
  {
    csRGBcolor* buf = new csRGBcolor[numPixels];
    csRGBcolor* bufptr = buf;
    while (numPixels--)
    {
      bufptr->red = *rgba++;
      bufptr->green = *rgba++;
      bufptr->blue = *rgba++;
      rgba++;
      bufptr++; 
    }
    return buf;
  }
};

/** @} */

/** @} */

#endif // __CSGFX_PACKRGB_H__
