/*  -*- Mode: C++; -*- */
/*
    Crystal Space 3D engine
    Copyright (C) 2000 by Jorrit Tyberghein

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

/*
 * Include this file to setup all blending defines.
 * It requires the correct define of PI_R?G?B? and the inclusion
 * of pixtype.inc.
 */

// Masks for accessing blending table. Since blending table expects a different
// number of bits per color component (for example, in 32bpp mode we have
// 6 bits per color component blending tables) we should strip off
// unused bits from color component. Since we allow 2*SRC lighting, we actually
// use one more bit per each component (to allow multiplication overflows).
#include "unblend.inc"

#ifdef PI_R8G8B8
#  define PI_RB_BLEND	6
#  define PI_GB_BLEND	6
#  define PI_BB_BLEND	6
#else
#  define PI_RB_BLEND	PI_RB
#  define PI_GB_BLEND	PI_GB
#  define PI_BB_BLEND	PI_BB
#endif

#define PI_RM_BLEND	(((2 << PI_RB_BLEND) - 1) << PI_RB_BLEND)
#define PI_GM_BLEND	(((2 << PI_GB_BLEND) - 1) << PI_GB_BLEND)
#define PI_BM_BLEND	(((2 << PI_BB_BLEND) - 1) << PI_BB_BLEND)

#define PI_RS_DST_BLEND	(PI_RS + (PI_RB - PI_RB_BLEND))
#define PI_GS_DST_BLEND	(PI_GS + (PI_GB - PI_GB_BLEND))
#define PI_BS_DST_BLEND	(PI_BS + (PI_BB - PI_BB_BLEND))

#define PI_RM_GOUTEX	((PI_RM >> PI_RS) << (24 - (PI_RB - PI_RB_BLEND)))
#define PI_GM_GOUTEX	((PI_GM >> PI_GS) << (24 - (PI_GB - PI_GB_BLEND)))
#define PI_BM_GOUTEX	((PI_BM >> PI_BS) << (24 - (PI_BB - PI_BB_BLEND)))

#define PI_RS_GOUTEX	(24 - (PI_RB - PI_RB_BLEND))
#define PI_GS_GOUTEX	(24 - (PI_GB - PI_GB_BLEND))
#define PI_BS_GOUTEX	(24 - (PI_BB - PI_BB_BLEND))

// In 5-6-5 mode we'll need two blending tables:
// one for 5-bit transformations and second for 6-bit transformations
#if (PI_GB == 6)
#  define PI_RBT_OFS	0
#  define PI_GBT_OFS	32*64
#  define PI_BBT_OFS	0
#else
#  define PI_RBT_OFS	0
#  define PI_GBT_OFS	0
#  define PI_BBT_OFS	0
#endif


