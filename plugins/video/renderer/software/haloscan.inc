/*  -*- Mode: C++; -*- */
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
#  define HALO_PIXTYPE	unsigned short
#else
#  define HALO_PIXTYPE	unsigned long
#endif

#if (HALO_BPP == 32) && defined (TOP8BITS_R8G8B8_USED)
#define PIXEL_PREPROC(x)  ((x) >> pixel_adjust)
#define PIXEL_POSTPROC(x) ((x) << pixel_adjust)
#else
#define PIXEL_PREPROC(x)  (x)
#define PIXEL_POSTPROC(x) (x)
#endif

static void HALO_NAME(void *src, void *dest, int count, int delta,
  int pixel_adjust)
{
  (void)pixel_adjust;
  unsigned char *A = (unsigned char *)src;
  HALO_PIXTYPE *D = (HALO_PIXTYPE *)dest;
  HALO_PIXTYPE *E = D + count;
  if (delta == 0x10000)
    while (D < E)
    {
      uint8 a = (*A++ * Scan.FogDensity) >> 8;
      if (a)
      {
        // Get the destination pixel from framebuffer
        // NOTE: d, dr, dg and db SHOULD be 32-bit,
        // even for 16-bit modes due to possible overflow (HALO_CLAMP)
        uint32 d = PIXEL_PREPROC (*D);

        // Compute destination R,G,B
        uint32 dr = d & HALO_RM;
        uint32 dg = d & HALO_GM;
        uint32 db = d & HALO_BM;
        // Apply alpha
        dr = dr + (int (a * (Scan.FogR - dr)) >> 8);
        dg = dg + (int (a * (Scan.FogG - dg)) >> 8);
        db = db + (int (a * (Scan.FogB - db)) >> 8);
        // Clamp the values if needed
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
        *D = PIXEL_POSTPROC (dr | dg | db);
      }
      D++;
    }
  else
  {
    unsigned int ax = 0;
    while (D < E)
    {
      uint8 a = (A [ax >> 16] * Scan.FogDensity) >> 8;
      if (a)
      {
        // Get the destination pixel from framebuffer
        uint32 d = PIXEL_PREPROC (*D);

        // Compute destination R,G,B
        uint32 dr = d & HALO_RM;
        uint32 dg = d & HALO_GM;
        uint32 db = d & HALO_BM;
        // Apply alpha
        dr = dr + (int (a * (Scan.FogR - dr)) >> 8);
        dg = dg + (int (a * (Scan.FogG - dg)) >> 8);
        db = db + (int (a * (Scan.FogB - db)) >> 8);
        // Clamp the values if needed
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
        *D = PIXEL_POSTPROC (dr | dg | db);
      }
      D++;
      ax += delta;
    }
  } /* endif */
}

#undef PIXEL_PREPROC
#undef PIXEL_POSTPROC
#undef HALO_NAME
#undef HALO_BPP
#undef HALO_PIXTYPE
#undef HALO_RM
#undef HALO_GM
#undef HALO_BM
#undef HALO_CLAMP
