/*
    To define a static draw_scanline function in the 'Scan' class just
    define the following variables:
	SCANFUNC	: name of the draw_scanline function (required)
	SCANEND		: optional additional code after the function (mostly for Z fill)
	SCANLOOP	: code to draw one segment of 16 (or less) pixels
	SCANMAP		: define if the function uses the lighted textures
	SCAN8		: define if function uses 8-bit pixels
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
	shifter_h	: shift value for vv (only if SCANMAP is NOT defined)
	ander_w		: and value for uu (only if SCANMAP is NOT defined)
	ander_h		: and value for vv (only if SCANMAP is NOT defined)
    The SCANLOOP code needs to update these variables as follows:
	uu, vv, izz	: u, v, and 1/z components just after this segment (izz = 8:24)
	_dest		: _destend+1
	z_buffer	: pointer to z_buffer after end of segment
    izz and z_buffer need only be updated if the loop code is interested
    in using the Z buffer. If the code does only a Z fill, 'izz' need only
    be updated in the Z fill loop.
*/

#if defined (SCAN8)
#  define PIXTYPE UByte
#elif defined (SCAN16)
#  define PIXTYPE UShort
#elif defined (SCAN32)
#  define PIXTYPE ULong
#else
#  error "WARNING: no pixel size defined before including scanln.inc!"
#endif

void SCANFUNC (int xx, unsigned char* d, unsigned long* z_buf, float inv_z,
  float u_div_z, float v_div_z)
{
  (void)z_buf;
  if (xx <= 0) return;

  float u1, v1, z, z1, u, v;
  int uu1, vv1, duu, dvv, uu, vv;
  long izz, dzz;

  z = 1 / inv_z;
  u = u_div_z * z;
  v = v_div_z * z;
  uu = QInt16 (u);
  vv = QInt16 (v);
  izz = QInt24 (inv_z);

  // Some optimizations for processors with many registers (PowerPC, 680x0).
  // Get global variables into registers.
# ifdef SCANMAP
  unsigned long shifter = Scan.shf_u;
  PIXTYPE *srcTex = (PIXTYPE *)Scan.tmap2;
# else
  unsigned long shifter_h = Scan.shf_h;
  unsigned long ander_w = Scan.and_w;
  unsigned long ander_h = Scan.and_h;
  PIXTYPE *srcTex = (PIXTYPE *)Scan.tmap;
# endif /*SCANMAP*/
  unsigned long *z_buffer = z_buf; (void)z_buffer;
  unsigned long *lastZbuf = z_buffer + xx-1; (void)lastZbuf;
  PIXTYPE *_destend;
  PIXTYPE *_dest = (PIXTYPE *)d;

  if (xx >= Scan.InterpolStep)
    inv_z += Scan.dM;
  else
    inv_z += Scan.M * xx;
  z1 = 1 / inv_z;

  dzz = QInt24 (Scan.M);

# if STUPID_TEST && SCANMAP
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
      uu1 = QInt16 (u1);
      vv1 = QInt16 (v1);
#     if STUPID_TEST && SCANMAP
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
      uu1 = QInt16 (u1);
      vv1 = QInt16 (v1);
#     if STUPID_TEST && SCANMAP
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

    SCANLOOP;

#   if STUPID_TEST && SCANMAP
    // This almost definitively removes texture overflows
    uu = uu1;
    vv = vv1;
#   endif
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
