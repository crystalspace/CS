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
    This include file defines a single scanline drawing routine for halo,
    either scaled or not, R,G,B can be clipped to 1.0 or not and so on.

    The following macros can/should be defined before including this file:

	HALO_NAME	Routine name
	HALO_BPP	number of bits per pixel
	HALO_RM		red component mask
	HALO_GM		green component mask
	HALO_BM		blue component mask
	HALO_CLAMP	if r/g/b components should be clamped to 1.0
*/

#if (HALO_BPP == 16)
#  define HALO_PIXTYPE	uint16
#else
#  define HALO_PIXTYPE	uint32
#endif

#if (HALO_BPP == 32)
#define HALO_PIXEL_PREPROC(x)  R8G8B8_PIXEL_PREPROC(x)
#define HALO_PIXEL_POSTPROC(x) R8G8B8_PIXEL_POSTPROC(x)
#else
#define HALO_PIXEL_PREPROC(x)  (x)
#define HALO_PIXEL_POSTPROC(x) (x)
#endif

static void HALO_NAME(void *src, void *dest, int count, int delta)
{
  unsigned char *A = (unsigned char *)src;
  HALO_PIXTYPE *D = (HALO_PIXTYPE *)dest;
  HALO_PIXTYPE *E = D + count;
  if (delta == 0x10000)
    while (D < E)
    {
      int32 a = (*A++ * Scan.FogDensity) >> 8;
      if (a)
      {
        // Get the destination pixel from framebuffer
        // NOTE: d, dr, dg and db SHOULD be 32-bit,
        // even for 16-bit modes due to possible overflow (HALO_CLAMP)
        uint32 d = HALO_PIXEL_PREPROC (*D);

        // Compute destination R,G,B
        uint32 dr = d & HALO_RM;
        uint32 dg = d & HALO_GM;
        uint32 db = d & HALO_BM;
        // Apply alpha
        dr = dr + ((a * int32 (Scan.FogR - dr)) >> 6);
        dg = dg + ((a * int32 (Scan.FogG - dg)) >> 6);
        db = db + ((a * int32 (Scan.FogB - db)) >> 6);
        // Clamp the values if needed
        dr &= ~(int32 (dr) >> 31); // if (dr < 0) dr = 0;
        dg &= ~(int32 (dg) >> 31); // if (dg < 0) dg = 0;
        db &= ~(int32 (db) >> 31); // if (db < 0) db = 0;
#ifdef HALO_CLAMP
        if (dr > HALO_RM) dr = HALO_RM; else dr &= HALO_RM;
        if (dg > HALO_GM) dg = HALO_GM; else dg &= HALO_GM;
        if (db > HALO_BM) db = HALO_BM;
#  if (HALO_BM & 1) == 0
        else db &= HALO_BM;
#  endif
#else
        dr &= HALO_RM;
        dg &= HALO_GM;
#  if (HALO_BM & 1) == 0
        db &= HALO_BM;
#  endif
#endif
        // Build new value and write into framebuffer
        *D = HALO_PIXEL_POSTPROC (dr | dg | db);
      }
      D++;
    }
  else
  {
    unsigned int ax = 0;
    while (D < E)
    {
      int32 a = (A [ax >> 16] * Scan.FogDensity) >> 8;
      if (a)
      {
        // Get the destination pixel from framebuffer
        uint32 d = HALO_PIXEL_PREPROC (*D);

        // Compute destination R,G,B
        uint32 dr = d & HALO_RM;
        uint32 dg = d & HALO_GM;
        uint32 db = d & HALO_BM;
        // Apply alpha
        dr = dr + ((a * int32 (Scan.FogR - dr)) >> 6);
        dg = dg + ((a * int32 (Scan.FogG - dg)) >> 6);
        db = db + ((a * int32 (Scan.FogB - db)) >> 6);
        // Clamp the values if needed
        dr &= ~((int32)dr >> 31); // if (dr < 0) dr = 0;
        dg &= ~((int32)dg >> 31); // if (dg < 0) dg = 0;
        db &= ~((int32)db >> 31); // if (db < 0) db = 0;
#ifdef HALO_CLAMP
        if (dr > HALO_RM) dr = HALO_RM; else dr &= HALO_RM;
        if (dg > HALO_GM) dg = HALO_GM; else dg &= HALO_GM;
        if (db > HALO_BM) db = HALO_BM;
#  if (HALO_BM & 1) == 0
        else db &= HALO_BM;
#  endif
#else
        dr &= HALO_RM;
        dg &= HALO_GM;
#  if (HALO_BM & 1) == 0
        db &= HALO_BM;
#  endif
#endif
        // Build new value and write into framebuffer
        *D = HALO_PIXEL_POSTPROC (dr | dg | db);
      }
      D++;
      ax += delta;
    }
  } /* endif */
}

#undef HALO_PIXEL_PREPROC
#undef HALO_PIXEL_POSTPROC
#undef HALO_NAME
#undef HALO_BPP
#undef HALO_PIXTYPE
#undef HALO_RM
#undef HALO_GM
#undef HALO_BM
#undef HALO_CLAMP
