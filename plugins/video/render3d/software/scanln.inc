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
    To define a static draw_scanline function in the 'Scan' class just
    define the following variables:
	SCANFUNC	: name of the draw_scanline function (required)
	SCANEND		: optional additional code after the function (mostly for Z fill)
	SCANLOOP	: code to draw one segment of 16 (or less) pixels
	SCANMAP		: define if the function uses the lighted textures
	TEXCOORDS	: if SCANMAP is defined but you want/need texture coordinates, too
	SCAN16		: define if function uses 16-bit pixels
	SCAN32		: define if function uses 32-bit pixels

    The SCANLOOP code can expect the following variables to be set:
	uu, vv, izz	: u, v, and 1/z components for start of segment (izz = 8:24)
	duu, dvv, dzz	: deltas per pixel for above values (dzz = 8:24)
	_dest		: pointer to screen for start of segment
	_destend	: pointer to screen for end of segment
	z_buffer	: pointer to z_buffer for start of segment
	lastZbuf	: pointer to z_buffer for end of complete line
	srcTex		: pointer to texture map
	shifter		: shift value for vv (only if SCANMAP is defined)
	shifter_h	: shift value for vv (only if SCANMAP is NOT defined OR TEXCOORDS is defined)
	ander_w		: and value for uu (only if SCANMAP is NOT defined OR TEXCOORDS is defined)
	ander_h		: and value for vv (only if SCANMAP is NOT defined OR TEXCOORDS is defined)
    The SCANLOOP code needs to update these variables as follows:
	uu, vv, izz	: u, v, and 1/z components just after this segment (izz = 8:24)
	_dest		: _destend+1
	z_buffer	: pointer to z_buffer after end of segment
    izz and z_buffer need only be updated if the loop code is interested
    in using the Z buffer. If the code does only a Z fill, 'izz' need only
    be updated in the Z fill loop.
*/

#if defined (SCAN16)
#  define PIXTYPE uint16
#elif defined (SCAN32)
#  define PIXTYPE uint32
#else
#  error "WARNING: no pixel size defined before including scanln.inc!"
#endif

void SCANFUNC (int xx, unsigned char* d, uint32* z_buf, float inv_z,
  float u_div_z, float v_div_z)
{
  (void)z_buf;
  if (xx <= 0) return;

  float u1, v1, z, z1, u, v;
  int uu1, vv1, duu, dvv, uu, vv;
  uint32 izz, dzz;

  // does not really belong here...
  // but there doesn't seem to be a better place
#ifdef A_MAP
  int a_uofs = Scan.amap_uofs;
  int a_vofs = Scan.amap_vofs;
#endif
  z = 1 / inv_z;
  u = u_div_z * z;
  v = v_div_z * z;
  uu = csQfixed16 (u);
  vv = csQfixed16 (v);
  izz = csQfixed24 (inv_z);

  // Some optimizations for processors with many registers (PowerPC, 680x0).
  // Get global variables into registers.
# ifdef SCANMAP
  uint32 shifter = Scan.shf_u;
  PIXTYPE *srcTex = (PIXTYPE *)Scan.bitmap2;
# else
  uint8 *srcTex = Scan.bitmap;
# endif
# if !defined(SCANMAP) || defined(TEXCOORDS)
  uint32 shifter_h = Scan.shf_h;
  uint32 ander_w = Scan.and_w;
  uint32 ander_h = Scan.and_h;
# endif // SCANMAP
  uint32 *z_buffer = z_buf; (void)z_buffer;
  uint32 *lastZbuf = z_buffer + xx-1; (void)lastZbuf;
  PIXTYPE *_destend;
  PIXTYPE *_dest = (PIXTYPE *)d;

  if (xx >= Scan.InterpolStep)
    inv_z += Scan.dM;
  else
    inv_z += Scan.M * xx;
  z1 = 1 / inv_z;

  dzz = csQfixed24 (Scan.M);

# if defined (STUPID_TEST) && defined (SCANMAP)
  if (uu < 0) uu = 0; else if (uu > Scan.tw2fp) uu = Scan.tw2fp;
  if (vv < 0) vv = 0; else if (vv > Scan.th2fp) vv = Scan.th2fp;
# endif

  do
  {
    if (xx < Scan.InterpolStep)
    {
      u_div_z += Scan.J1 * xx;
      v_div_z += Scan.K1 * xx;

      _destend = _dest + xx - 1;

      u1 = u_div_z * z1;
      v1 = v_div_z * z1;
      uu1 = csQfixed16 (u1);
      vv1 = csQfixed16 (v1);
#     if defined (STUPID_TEST) && defined (SCANMAP)
      if (uu1 < 0) uu1 = 0; else if (uu1 > Scan.tw2fp) uu1 = Scan.tw2fp;
      if (vv1 < 0) vv1 = 0; else if (vv1 > Scan.th2fp) vv1 = Scan.th2fp;
#     endif

      duu = (uu1 - uu) / xx;
      dvv = (vv1 - vv) / xx;

      xx = 0;
    }
    else
    {
      u_div_z += Scan.dJ1;
      v_div_z += Scan.dK1;

      _destend = _dest + Scan.InterpolStep - 1;
      xx -= Scan.InterpolStep;

      u1 = u_div_z * z1;
      v1 = v_div_z * z1;
      uu1 = csQfixed16 (u1);
      vv1 = csQfixed16 (v1);
#     if defined (STUPID_TEST) && defined (SCANMAP)
      if (uu1 < 0) uu1 = 0; else if (uu1 > Scan.tw2fp) uu1 = Scan.tw2fp;
      if (vv1 < 0) vv1 = 0; else if (vv1 > Scan.th2fp) vv1 = Scan.th2fp;
#     endif

      if (xx >= Scan.InterpolStep)
        inv_z += Scan.dM;
      else
        inv_z += Scan.M * xx;

      duu = (uu1 - uu) >> Scan.InterpolShift;
      dvv = (vv1 - vv) >> Scan.InterpolShift;
    }

    // The following divide should overlap with integer instructions
    // below and GCC shouldn't pickup the result until the
    // end of the loop. Thus, the divide is free and the
    // routine is almost as fast as an affine mapper on
    // Pentium class processors.
    // Note that this divide should normally be inside one of the above
    // branches (it is not needed when doing the last part of the scanline)
    // but GCC is not able to correctly optimize this if we put it inside
    // the 'if' statement. That's the reason that it is put here.
    // In case of the last part of the scanline it will just calculate
    // a result that will not be needed afterwards (but it doesn't
    // hurt either).
    z1 = 1 / inv_z;

#   if !defined (SCANMAP)
    uu += Scan.min_u; vv += Scan.min_v;
#   endif

    SCANLOOP;

    // This almost definitively removes texture overflows
    uu = uu1;
    vv = vv1;
  }
  while (xx);

# ifdef SCANEND
  SCANEND;
# endif /*SCANEND*/
}

#undef SCANFUNC
#undef SCANEND
#undef SCANLOOP
#undef SCANMAP
#undef PIXTYPE
#undef TEXCOORDS

