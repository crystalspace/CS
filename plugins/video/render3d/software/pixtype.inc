/*  -*- Mode: C++; -*- */
/*
    Crystal Space 3D engine
    Copyright (C) 2000 by Andrew Zabolotny

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

#include "csplugincommon/render3d/pixfmt.h"

// Depending on the Colorformat, we define different masks and shiftvalues
// to get the accrding values for rgb; R,G,B stands for Red, Green and Blue,
// and M, S, B stands for Mask, Shift and Bits correspondingly.
#undef PI_PIXTYPE
#undef PI_RM
#undef PI_RS
#undef PI_RB
#undef PI_GM
#undef PI_GS
#undef PI_GB
#undef PI_BM
#undef PI_BS
#undef PI_BB

#if defined (PI_R5G5B5)
#  define PI_PIXTYPE	uint16
#  define PI_RB		5
#  define PI_GB		5
#  define PI_BB		5
#  define PI_RS		10
#  define PI_GS		5
#  define PI_BS		0
#elif defined (PI_R5G6B5)
#  define PI_PIXTYPE	uint16
#  define PI_RB		5
#  define PI_GB		6
#  define PI_BB		5
#  define PI_RS		11
#  define PI_GS		5
#  define PI_BS		0
#elif defined (PI_R8G8B8)
#  define PI_PIXTYPE	uint32
#  define PI_RB	8
#  define PI_GB	8
#  define PI_BB	8
#  if (CS_24BIT_PIXEL_LAYOUT == CS_24BIT_PIXEL_ABGR)
#    define PI_RS	0
#    define PI_GS	8
#    define PI_BS	16
#  else
#    define PI_RS	16
#    define PI_GS	8
#    define PI_BS	0
#  endif
#else
#  error "ERROR: Pixel format not defined before including pixtype.inc!"
#endif

#define PI_RM unsigned (((1 << PI_RB) - 1) << PI_RS)
#define PI_GM unsigned (((1 << PI_GB) - 1) << PI_GS)
#define PI_BM unsigned (((1 << PI_BB) - 1) << PI_BS)
