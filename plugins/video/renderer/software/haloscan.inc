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
#  define HALO_PIXTYPE	unsigned int
#endif

void HALO_NAME (void *src, void *dest, int count, int delta)
{
  unsigned char *A = (unsigned char *)src;
  HALO_PIXTYPE *D = (HALO_PIXTYPE *)dest;
  HALO_PIXTYPE *E = D + count;
  if (delta == 0x10000)
    while (D < E)
    {
      unsigned char a = (*A++ * Scan.FogDensity) >> 8;
      if (a)
      {
        // Get the destination pixel from framebuffer
        unsigned int d = *D;
#if (HALO_BPP == 32) && defined (TOP8BITS_R8G8B8_USED)
        d >>= PostShift;
#endif

        // Compute destination R,G,B
        unsigned int dr = d & HALO_RM;
        unsigned int dg = d & HALO_GM;
        unsigned int db = d & HALO_BM;
        // Apply alpha
        dr = dr + (a * (Scan.FogR - dr) >> 8);
        dg = dg + (a * (Scan.FogG - dg) >> 8);
        db = db + (a * (Scan.FogB - db) >> 8);
        // Clamp the values if needed
#ifdef HALO_CLAMP
        if (dr > HALO_RM) dr = HALO_RM; else dr &= HALO_RM;
        if (dg > HALO_GM) dg = HALO_GM; else dg &= HALO_GM;
        if (db > HALO_BM) db = HALO_BM;//else db &= HALO_BM;
#else
        dr &= HALO_RM;
        dg &= HALO_GM;
      //db &= HALO_BM;
#endif

        // Build the value to write back into framebuffer
        d = dr | dg | db;

        // Write the new value back into framebuffer
#if (HALO_BPP == 32) && defined (TOP8BITS_R8G8B8_USED)
        d <<= PostShift;
#endif
        *D = d;
      }
      D++;
    }
  else
  {
    unsigned int ax = 0;
    while (D < E)
    {
      unsigned char a = (A [ax >> 16] * Scan.FogDensity) >> 8;
      if (a)
      {
        // Get the destination pixel from framebuffer
        unsigned int d = *D;
#if (HALO_BPP == 32) && defined (TOP8BITS_R8G8B8_USED)
        d >>= PostShift;
#endif

        // Compute destination R,G,B
        unsigned int dr = d & HALO_RM;
        unsigned int dg = d & HALO_GM;
        unsigned int db = d & HALO_BM;
        // Apply alpha
        dr = dr + (a * (Scan.FogR - dr) >> 8);
        dg = dg + (a * (Scan.FogG - dg) >> 8);
        db = db + (a * (Scan.FogB - db) >> 8);
        // Clamp the values if needed
#ifdef HALO_CLAMP
        if (dr > HALO_RM) dr = HALO_RM; else dr &= HALO_RM;
        if (dg > HALO_GM) dg = HALO_GM; else dg &= HALO_GM;
        if (db > HALO_BM) db = HALO_BM;//else db &= HALO_BM;
#else
        dr &= HALO_RM;
        dg &= HALO_GM;
      //db &= HALO_BM;
#endif

        // Build the value to write back into framebuffer
        d = dr | dg | db;

        // Write the new value back into framebuffer
#if (HALO_BPP == 32) && defined (TOP8BITS_R8G8B8_USED)
        d <<= PostShift;
#endif
        *D = d;
      }
      D++;
      ax += delta;
    }
  } /* endif */
}

#undef HALO_NAME
#undef HALO_BPP
#undef HALO_PIXTYPE
#undef HALO_RM
#undef HALO_GM
#undef HALO_BM
#undef HALO_CLAMP
