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
#include "rgbpixel.h"

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

/**\fn const uint8* csPackRGBcolorToRGB (const csRGBcolor* pixels, int numPixels)
 * Pack an array of csRGBcolor into a RGB byte array. 
 * \remarks
 * May return \p pixels.
 * May allocate memory. Free it using csDiscardPackedRGB() when finished.
 * \param pixels Source array of csRGBcolor data
 * \param numPixels Number of pixels in the array
 * \return A byte array containing the source data packed as RGB.
 */

/**\fn void csDiscardPackedRGB (const uint8* rgb)
 * Frees memory possibly allocated by csPackRGBcolorToRGB().
 * \param rgb Pointer to packed RGB data returned by csPackRGBcolorToRGB().
 */

/**\fn const csRGBcolor* csUnpackRGBtoRGBcolor (const uint8* rgb, int numPixels)
 * Unpack a RGB byte array into an array of csRGBcolor. 
 * \remarks
 * May return \p pixels.
 * May allocate memory. Free it using csDiscardUnpackedRGBcolor() when finished.
 * \param rgb Source array of RGB data
 * \param numPixels Number of pixels in the array
 * \return An array containing the source data in csRGBcolor structs.
 */

/**\fn void csDiscardUnpackedRGBcolor (const csRGBcolor* pixels)
 * Frees memory possibly allocated by csUnpackRGBtoRGBcolor().
 * \param pixels Pointer to csRGBcolor array returned by csUnpackRGBtoRGBcolor().
 */

#ifdef CS_RGBCOLOR_SANE
// sizeof(csRGBcolor) == 3

inline const uint8* csPackRGBcolorToRGB (const csRGBcolor* pixels, 
					 int /*numPixels*/)
{
  return (const uint8*)pixels;
}

inline void csDiscardPackedRGB (const uint8* /*rgb*/) {}

inline const csRGBcolor* csUnpackRGBtoRGBcolor (const uint8* rgb, 
						int /*numPixels*/)
{
  return (const csRGBcolor*)rgb;
}

inline void csDiscardUnpackedRGBcolor (const csRGBcolor* /*pixels*/) {}

#else
// sizeof(csRGBcolor) != 3

inline uint8* csPackRGBcolorToRGB (const csRGBcolor* pixels, 
				   int numPixels)
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

inline void csDiscardPackedRGB (const uint8* rgb) 
{
  delete[] rgb;
}

inline const csRGBcolor* csUnpackRGBtoRGBcolor (const uint8* rgb, 
					        int numPixels)
{
  csRGBcolor* buf = new csRGBcolor[numPixels];
  csRGBcolor* bufptr = buf;
  while (numPixels--)
  {
    bufptr->red = *rgb++;
    bufptr->green = *rgb++;
    bufptr->blue = *rgb++;
    bufptr++; 
  }
  return buf;
}

inline void csDiscardUnpackedRGBcolor (const csRGBcolor* pixels) 
{
  delete[] pixels;
}

#endif // CS_RGBCOLOR_SANE

/**\fn const uint8* csPackRGBpixelToRGBA (const csRGBpixel* pixels, int numPixels)
 * Pack an array of csRGBpixel into a RGBA byte array. 
 * \remarks
 * May return \p pixels.
 * May allocate memory. Free it using csDiscardPackedRGBA() when finished.
 * \param pixels Source array of csRGBpixel data
 * \param numPixels Number of pixels in the array
 * \return A byte array containing the source data packed as RGBA.
 */

/**\fn void csDiscardPackedRGBA (const uint8* rgba)
 * Frees memory possibly allocated by csPackRGBpixelToRGBA().
 * \param rgba Pointer to packed RGB data returned by csPackRGBpixelToRGBA().
 */

/**\fn const csRGBpixel* csUnpackRGBAtoRGBpixel (const uint8* rgba, int numPixels)
 * Unpack a RGBA byte array into an array of csRGBpixel. 
 * \remarks
 * May return \p pixels.
 * May allocate memory. Free it using csDiscardUnpackedRGBpixel() when finished.
 * \param rgba Source array of RGBA data
 * \param numPixels Number of pixels in the array
 * \return An array containing the source data in csRGBpixel structs.
 */

/**\fn csRGBpixel* csCopyUnpackRGBAtoRGBpixel(const uint8*pixels,int numPixels)
 * Unpack a RGBA byte array into an array of csRGBpixel. 
 * \remarks
 * Never returns \p pixels.
 * Allocate memory. Free it using delete[] when finished.
 * \param pixels Source array of RGBA data
 * \param numPixels Number of pixels in the array
 * \return An array containing the source data in csRGBpixel structs.
 */

/**\fn void csDiscardUnpackedRGBpixel(const csRGBpixel* pixels)
 * Frees memory possibly allocated by csUnpackRGBAtoRGBpixel().
 * \param pixels Pointer to csRGBpixel array returned by
 * csUnpackRGBAtoRGBpixel().
 */

#ifdef CS_RGBPIXEL_SANE
// sizeof(csRGBpixel) == 4

inline const uint8* csPackRGBpixelToRGBA (const csRGBpixel* pixels, 
				    int /*numPixels*/)
{
  return (uint8*)pixels;
}

inline void csDiscardPackedRGBA (const uint8* /*rgba*/) {}

inline const csRGBpixel* csUnpackRGBAtoRGBpixel (const uint8* rgba, 
						 int /*numPixels*/)
{
  return (csRGBpixel*)rgba;
}

inline csRGBpixel* csCopyUnpackRGBAtoRGBpixel (const uint8* rgba, 
					       int numPixels)
{
  csRGBpixel* buf = new csRGBpixel[numPixels];
  memcpy ((void*)buf, (const void*)rgba, numPixels*  sizeof(csRGBpixel));
  return buf;
}

inline void csDiscardUnpackedRGBpixel (const csRGBpixel* /*pixels*/) {}

#else
// sizeof(csRGBpixel) != 4

inline const uint8* csPackRGBpixelToRGBA (const csRGBpixel* pixels, 
					  int numPixels)
{
  uint8* buf = new uint8[numPixels * 4];
  uint8* bufptr = buf;
  while (numPixels--)
  {
    *bufptr++ = pixels->red;
    *bufptr++ = pixels->green;
    *bufptr++ = pixels->blue;
    *bufptr++ = pixels->alpha;
    pixels++; 
  }
  return buf;
}

inline void csDiscardPackedRGBA (const uint8* rgba) 
{
  delete[] rgba;
}

inline const csRGBpixel* csUnpackRGBAtoRGBpixel (const uint8* rgba, 
						 int numPixels)
{
  csRGBpixel* buf = new csRGBpixel[numPixels];
  csRGBpixel* bufptr = buf;
  while (numPixels--)
  {
    bufptr->red = *rgba++;
    bufptr->green = *rgba++;
    bufptr->blue = *rgba++;
    bufptr->alpha = *rgba++;
    bufptr++; 
  }
  return buf;
}

inline csRGBpixel* csCopyUnpackRGBAtoRGBpixel (const uint8* rgba, 
					       int numPixels)
{
  return (csRGBpixel*)csUnpackRGBAtoRGBpixel (rgba, numPixels);
}

inline void csDiscardUnpackedRGBpixel (const csRGBpixel* pixels) 
{
  delete[] pixels;
}

#endif // CS_RGBPIXEL_SANE

/**
 * Pack an array of csRGBpixel into a RGB byte array. Alpha information 
 * is discarded!
 * \remarks
 * Allocates memory. Free it using delete[] when finished.
 * \param pixels Source array of csRGBpixel data
 * \param numPixels Number of pixels in the array
 * \return A byte array containing the source data packed as RGB.
 */
inline uint8* csPackRGBpixelToRGB (const csRGBpixel* pixels, 
				   int numPixels)
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
 * Unpack a RGBA byte array into an array of csRGBcolor. Alpha information 
 * is discarded!
 * \remarks
 * Allocates memory. Free it using delete[] when finished.
 * \param rgba Source array of RGBA data
 * \param numPixels Number of pixels in the array
 * \return An array containing the source data in csRGBcolor structs.
 */
inline csRGBcolor* csUnpackRGBAtoRGBcolor (const uint8* rgba, 
					   int numPixels)
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

/** @} */

/** @} */

#endif // __CSGFX_PACKRGB_H__
