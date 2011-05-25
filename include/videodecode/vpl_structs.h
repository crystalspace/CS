/*
Copyright (C) 2011 by Alin Baciu

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

#ifndef __CS_VPL_COMMON_STRUCTS_H__
#define __CS_VPL_COMMON_STRUCTS_H__

/**\file
 * Video Player: types
 */

#include "csutil/scf.h"

/**\addtogroup vpl
 * @{ */

struct csVPLvideoFormat
{
  /// a boolean I use to tell if a theora video stream was found
  bool foundVid;

  /// frame width and height
  uint32 fWidth, 
         fHeight;
  /// colorspace for the video stream
  int colorspace;
  /// the pixel format of the video stream
  int pixelfmt;

  /// picture offset
  uint32 picOffsetX,
         picOffsetY;

  /// the FPS of the video stream
  float FPS;

  /// the target bitrate of the video stream 
  int target_bitrate;
};

struct vidFrameData
{
	unsigned char * data;

	int width;
	int height;
	int stride;
};

/** @} */

#endif // __CS_VPL_COMMON_STRUCTS_H__
